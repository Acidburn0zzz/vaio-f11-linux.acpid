/*
 * Display and keyboard backlight daemon for Sony laptops
 *
 * Copyright (C) 2011 Tu Vuong <vanhtu1987@gmail.com>
 *
 * ud_connect() and read_line() were copied and adapted from acpid 1.x
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

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
void read_hex_from_file(char const* path, int* array, int size) {
    FILE* const fd = open_file(path, "r");
    int result, i;
    for (i = 0; i < size; i++) {
        if (fscanf(fd, "0x%x ", &result) != 1) {
            fprintf(stderr, "read_hex_from_file: fscanf(%d) failed - %s\n", i,
                    strerror(errno));
            exit(EXIT_FAILURE);
        }
        array[i] = result;
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
