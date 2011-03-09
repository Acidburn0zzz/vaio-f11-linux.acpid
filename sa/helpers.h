#ifndef  HELPERS_INC
#define  HELPERS_INC

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

enum BacklightController {BC_ACPI, BC_NVIDIA};

static char const*const ACPI_BL_BRGT = "/sys/class/backlight/acpi_video0/brightness";
static char const*const ACPI_BL_BRGT_MAX = "/sys/class/backlight/acpi_video0/max_brightness";

static char const*const NVIDIA_BL_BRGT = "/sys/class/backlight/nvidia_backlight/brightness";
static char const*const NVIDIA_BL_BRGT_MAX = "/sys/class/backlight/nvidia_backlight/max_brightness";

static char const*const SONY_ALS_BL = "/sys/devices/platform/sony-laptop/als_backlight";
static char const*const SONY_EVENT_CLASS = "sony/hotkey";
static char const*const SONY_EVENT_TYPE = "SNC";
static char const*const SONY_EVENT_MAJOR = "00000001";
static char const*const SONY_EVENT_ALS = "00000003";

struct ConstValues {
    unsigned int max_brgt, min_brgt;
    int bl_ctrl;
};
struct ConstValues init_const_values(int bl_ctrl);

FILE* open_file(char const* path, char const* mode);

void acpi_event_loop(FILE* fd, int bl_ctrl,
                     void(*event_func)(struct ConstValues const*,
                                       char const*, char const*,
                                       char const*, char const*));

void handle_acpi_events(struct ConstValues const* vals,
                        char const* evt_cls, char const* evt_type,
                        char const* evt_major, char const* evt_minor);

int read_int_from_file(char const* path);
void write_int_to_file(char const* path, int val);

void update_brightness(char const* path, int current, int target);

#endif   /* ----- #ifndef HELPERS_INC  ----- */

