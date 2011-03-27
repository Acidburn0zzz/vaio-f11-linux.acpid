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
#include <math.h>

#include "file_funcs.h"

enum BacklightController {BC_ACPI, BC_NVIDIA, BC_SONY};

static char const*const ACPI_BL_BRGT = "/sys/class/backlight/acpi_video0/brightness";
static char const*const ACPI_BL_BRGT_MAX = "/sys/class/backlight/acpi_video0/max_brightness";

static char const*const NVIDIA_BL_BRGT = "/sys/class/backlight/nvidia_backlight/brightness";
static char const*const NVIDIA_BL_BRGT_MAX = "/sys/class/backlight/nvidia_backlight/max_brightness";

static char const*const SONY_BL_BRGT = "/sys/class/backlight/sony/brightness";
static char const*const SONY_BL_BRGT_MAX = "/sys/class/backlight/sony/max_brightness";

static char const*const SONY_ALS_LUX = "/sys/devices/platform/sony-laptop/als_lux";
static char const*const SONY_ALS_PARAM = "/sys/devices/platform/sony-laptop/als_parameters";
static char const*const SONY_KBD_BL = "/sys/devices/platform/sony-laptop/kbd_backlight";
static int const AMBIENT_TOO_DIM = 7; /* If als_lux < 7 then the environment is too dim */
static char const*const SONY_EVENT_CLASS = "sony/hotkey";
static char const*const SONY_EVENT_TYPE = "SNC";
static char const*const SONY_EVENT_MAJOR = "00000001";
static char const*const SONY_EVENT_ALS = "00000003";
static char const*const SONY_EVENT_OLDALS = "0000012f";
static char const*const SONY_BL_BRGT_UP = "00000011";
static char const*const SONY_BL_BRGT_DOWN = "00000010";

#define NUMBER_BRGT 9

struct AcpiData {
    int max_brgt, min_brgt;
    int table;	/* Values aren't linear. Are table-based */
    int tbl_brgt[NUMBER_BRGT];
    int current_brgt, new_brgt;
    char const* brgt_path;
};
struct AcpiData init_acpi_data(int bl_ctrl);

void acpi_event_loop(int fd, int bl_ctrl);

void handle_acpi_events(struct AcpiData* vals, char** evt_toks);

void update_brightness(struct AcpiData* vals, long *usec);

#endif   /* ----- #ifndef ACPI_FUNCS_INC  ----- */

