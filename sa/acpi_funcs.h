/*
 * Display and keyboard backlight daemon for Sony laptops
 *
 * Copyright (C) 2011 Tu Vuong <vanhtu1987@gmail.com>
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

#ifndef  ACPI_FUNCS_INC
#define  ACPI_FUNCS_INC

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "file_funcs.h"

enum BacklightController {BC_ACPI, BC_NVIDIA};

static char const*const ACPI_BL_BRGT = "/sys/class/backlight/acpi_video0/brightness";
static char const*const ACPI_BL_BRGT_MAX = "/sys/class/backlight/acpi_video0/max_brightness";

static char const*const NVIDIA_BL_BRGT = "/sys/class/backlight/nvidia_backlight/brightness";
static char const*const NVIDIA_BL_BRGT_MAX = "/sys/class/backlight/nvidia_backlight/max_brightness";

static char const*const SONY_ALS_BL = "/sys/devices/platform/sony-laptop/als_backlight";
static char const*const SONY_ALS_LUX_THRESHOLD = "/sys/devices/platform/sony-laptop/als_lux_threshold";
static char const*const SONY_KBD_BL = "/sys/devices/platform/sony-laptop/kbd_backlight";
static int const AMBIENT_TOO_DIM = 25; /* If als_backlight < 25 then the environment is too dim */
static char const*const SONY_EVENT_CLASS = "sony/hotkey";
static char const*const SONY_EVENT_TYPE = "SNC";
static char const*const SONY_EVENT_MAJOR = "00000001";
static char const*const SONY_EVENT_ALS = "00000003";

struct ConstValues {
    unsigned int max_brgt, min_brgt;
    int bl_ctrl;
};
struct ConstValues init_const_values(int bl_ctrl);

void acpi_event_loop(int fd, int bl_ctrl);

void handle_acpi_events(struct ConstValues const* vals,
                        char const* evt_cls, char const* evt_type,
                        char const* evt_major, char const* evt_minor);

void update_brightness(char const* path, int current, int target);

#endif   /* ----- #ifndef ACPI_FUNCS_INC  ----- */

