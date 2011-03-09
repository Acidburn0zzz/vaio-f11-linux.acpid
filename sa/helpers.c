#include "helpers.h"

FILE* open_file(char const* path, char const* mode) {
    FILE* const fd = fopen(path, mode);
    if (!fd) {
        fprintf(stderr, "open_file: can't open %s: %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    return fd;
}

void acpi_event_loop(FILE* fd, void(*event_func)(char const*, char const*,
                                                 char const*, char const*)) {
    size_t BUF_SIZE = 1024;
    char* buf = NULL;

    char* evt_toks[4];
    unsigned int i = 0;

    buf = malloc(BUF_SIZE);
    if (!buf) {
        fprintf(stderr, "acpi_event_loop: can't malloc buffer\n");
        exit(EXIT_FAILURE);
    }

    while (getline(&buf, &BUF_SIZE, fd) != -1) {
        for (i = 0; i != 4; ++i) { /* Assuming it's always 4 items */
            if (!i)
                evt_toks[i] = strtok(buf, " ");
            else evt_toks[i] = strtok(NULL, " ");
        }
        evt_toks[3][strlen(evt_toks[3])-1] = 0; /* Eliminate trailing newline */

        event_func(evt_toks[0], evt_toks[1], evt_toks[2], evt_toks[3]);
    }

    free(buf);
}

void handle_acpi_events(char const* evt_cls, char const* evt_type,
                        char const* evt_major, char const* evt_minor) {
    printf("%s %s %s %s\n", evt_cls, evt_type, evt_major, evt_minor);
}
