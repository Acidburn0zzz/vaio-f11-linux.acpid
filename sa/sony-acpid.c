#include <fcntl.h>
#include <stdlib.h>

#include "helpers.h"

/* File paths */
static char const*const ACPI_EVENT_FILE = "/proc/acpi/event";

int main(int argc, char** argv) {
    FILE* const event_fd = open_file(ACPI_EVENT_FILE, "r");
    int bl_ctrl = BC_ACPI;
    if (argc > 1) {
        if (!strcmp(argv[1], "nvidia"))
            bl_ctrl = BC_NVIDIA;
    }

    acpi_event_loop(event_fd, bl_ctrl, &handle_acpi_events);

    /* We may never reach this line unless we handle terminate signal */
    fclose(event_fd);

    return 0;
}
