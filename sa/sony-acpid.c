#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "helpers.h"

/* File paths */
static char const*const ACPI_EVENT_FILE = "/proc/acpi/event";

int main(int argc, char** argv) {
    FILE* const event_fd = open_file(ACPI_EVENT_FILE, "r");
    int bl_ctrl = BC_ACPI;
    int opt = -1;

    while ((opt = getopt(argc, argv, "c:t:")) != -1) {
        switch (opt) {
            case 'c':
                if (!strcmp(optarg, "nvidia"))
                    bl_ctrl = BC_NVIDIA;
                break;
            case 't':
                write_int_to_file(SONY_ALS_LUX_THRESHOLD, atoi(optarg));
                break;
            default:
                fprintf(stderr, "Usage: %s [-c acpi|nvidia] [-t number]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    acpi_event_loop(event_fd, bl_ctrl, &handle_acpi_events);

    /* We may never reach this line unless we handle terminate signal */
    fclose(event_fd);

    return 0;
}
