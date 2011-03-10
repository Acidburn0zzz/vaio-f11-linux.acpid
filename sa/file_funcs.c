#include "file_funcs.h"

FILE* open_file(char const* path, char const* mode) {
    FILE* const fd = fopen(path, mode);
    if (!fd) {
        fprintf(stderr, "open_file: can't open %s: %s\n",
                path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    return fd;
}

int read_int_from_file(char const* path) {
    FILE* const fd = open_file(path, "r");
    int result = -1;
    if (fscanf(fd, "%i", &result) != 1) {
        fprintf(stderr, "read_int_from_file: fscanf() failed - %s\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    fclose(fd);

    return result;
}
void write_int_to_file(char const* path, int val) {
    FILE* const fd = open_file(path, "w");
    if (fprintf(fd, "%i", val) < 0) {
        fprintf(stderr, "write_int_to_file: fprintf failed - %s\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    fclose(fd);
}

int ud_connect(char const* name) {
    int fd = -1;
    struct sockaddr_un addr;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        fprintf(stderr, "ud_connect: can't create socket - %s\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    sprintf(addr.sun_path, "%s", name);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "ud_connect: can't connect to socket - %s\n",
                strerror(errno));
        close(fd);
        exit(EXIT_FAILURE);
    }

    return fd;
}

char* read_line(int fd) {
    unsigned int const MAX_BUFLEN = 1024;
    static char* buf = NULL;
    unsigned int buflen = 64;
    unsigned int i = 0;
    int r = -1;
    int searching = 1;

    while (searching) {
        buf = realloc(buf, buflen);
        if (!buf) {
            fprintf(stderr, "ERR: malloc(%d): %s\n",
                    buflen, strerror(errno));
            return NULL;
        }
        memset(buf+i, 0, buflen-i);

        while (i < buflen) {
            r = read(fd, buf+i, 1);
            if (r < 0 && errno != EINTR) {
                /* we should do something with the data */
                fprintf(stderr, "ERR: read(): %s\n",
                        strerror(errno));
                return NULL;
            } else if (r == 0) {
                /* signal this in an almost standard way */
                errno = EPIPE;
                return NULL;
            } else if (r == 1) {
                /* scan for a newline */
                if (buf[i] == '\n') {
                    searching = 0;
                    buf[i] = '\0';
                    break;
                }
                i++;
            }
        }
        if (buflen >= MAX_BUFLEN) {
            break;
        }
        buflen *= 2;
    }

    return buf;
}
