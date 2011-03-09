#include <fcntl.h>
#include <stdlib.h>

#include "helpers.h"

/* File paths */
char const*const ACPI_EVENT_FILE = "/proc/acpi/event";
char const*const ACPI_BL_DIR = "/sys/class/backlight/acpi_video0/";
char const*const ACPI_BGRT = "brightness";
char const*const ACPI_BGRT_MAX = "max_brightness";
char const*const NVIDIA_BL_DIR = "/sys/class/backlight/nvidia_backlight/";
char const*const SONY_SYSFS_DIR = "/sys/devices/platform/sony-laptop/";
char const*const SONY_KBD_BACKLIGHT = "kbd_backlight";

int main(int argc, char** argv) {
    FILE* const event_fd = open_file(ACPI_EVENT_FILE, "r");

    acpi_event_loop(event_fd, &handle_acpi_events);

    /* We may never reach this line unless we handle terminate signal */
    fclose(event_fd);

    return 0;
}
