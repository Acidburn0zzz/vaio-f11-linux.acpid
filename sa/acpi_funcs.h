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

#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))

static char const*const SONY_BL_BRGT = "/sys/devices/platform/sony-laptop/als_backlight";
static char const*const SONY_ALS_MANAGED = "/sys/devices/platform/sony-laptop/als_managed";
static char const*const SONY_ALS_LEVELS = "/sys/devices/platform/sony-laptop/als_backlight_levels";
static char const*const SONY_ALS_PARAMS = "/sys/devices/platform/sony-laptop/als_defaults";
static char const*const ACPI_VIDEO_BRGT = "/sys/class/backlight/acpi_video0/brightness";
static int const ACPI_MIN_BRGT = 0;
static int const ACPI_MAX_BRGT = 15;
#define ALS_NUM_PARAM 13

static char const*const SONY_ALS_LUX = "/sys/devices/platform/sony-laptop/als_lux";
static char const*const SONY_KBD_BL = "/sys/devices/platform/sony-laptop/kbd_backlight";
static unsigned int const AMBIENT_TOO_DIM = 7; /* If als_lux < 7 then the environment is too dim */

static char const*const SONY_EVENT_CLASS = "sony/hotkey";
static char const*const SONY_EVENT_TYPE = "SNC";

#define SONY_EVENT_KEYPRESS     1
#define SONY_EVENT_RFKILL       2
#define SONY_EVENT_ALS          3
#define SONY_EVENT_FREEFALL     4
#define SONY_KEY_BRGT_DOWN      0x10
#define SONY_KEY_BRGT_UP        0x11
#define SONY_KEY_EXTVID         0x12
#define SONY_KEY_ZOOM_OUT       0x14
#define SONY_KEY_ZOOM_IN        0x15
#define SONY_KEY_SUSPEND        0x17
#define SONY_KEY_S1             0x20
#define SONY_KEY_EJECT          0x40
#define SONY_EVENT_ALS_CHANGED  0x1
#define SONY_EVENT_ALS_ACPI_VIDEO 0x2

struct AcpiData {
    unsigned int kbd_bl;
    /* Assuming levels in als_parameters correspond to ACPI brightness levels */
    int brgt_num;
    int brgt_levels[16];
    int brgt_params[ALS_NUM_PARAM];
    int is_event;
    int brgt_range;
    int current_brgt, new_brgt;
    int current_acpi_brgt;
};
struct AcpiData init_acpi_data();

void acpi_event_loop(int fd);

void handle_acpi_events(struct AcpiData* vals, char** evt_toks);

void update_brightness(struct AcpiData* vals, long* usec);

#endif   /* ----- #ifndef ACPI_FUNCS_INC  ----- */

