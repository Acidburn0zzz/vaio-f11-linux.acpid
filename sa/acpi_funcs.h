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

static char const*const SONY_BL_BRGT = "/sys/devices/platform/sony-laptop/als_backlight";
static char const*const SONY_ALS_MANAGED = "/sys/devices/platform/sony-laptop/als_managed";
static char const*const SONY_ALS_PARAMS = "/sys/devices/platform/sony-laptop/als_parameters";
static int const ACPI_MIN_BRGT = 0;
static int const ACPI_MAX_BRGT = 8;

static char const*const SONY_ALS_LUX = "/sys/devices/platform/sony-laptop/als_lux";
static char const*const SONY_ALS_POWER = "/sys/devices/platform/sony-laptop/als_power";
static char const*const SONY_KBD_BL = "/sys/devices/platform/sony-laptop/kbd_backlight";
static unsigned int const AMBIENT_TOO_DIM = 7; /* If als_lux < 7 then the environment is too dim */

static char const*const SONY_EVENT_CLASS = "sony/hotkey";
static char const*const SONY_EVENT_TYPE = "SNC";

static char const*const SONY_EVENT_BL_BRGT = "00000092";
static char const*const SONY_EVENT_BL_BRGT_UP = "00000011";
static char const*const SONY_EVENT_BL_BRGT_DOWN = "00000010";

static char const*const SONY_EVENT_ALS = "00000093";
static char const*const SONY_EVENT_ALS_CHANGED = "00000001";

struct AcpiData {
    unsigned int kbd_bl;
    float prev_lux;
    /* Assuming levels in als_parameters correspond to ACPI brightness levels */
    int brgt_levels[9];
    int brgt_range;
    int current_brgt, current_acpi_brgt;
};
struct AcpiData init_acpi_data();

void acpi_event_loop(int fd);

void handle_acpi_events(struct AcpiData* vals, char** evt_toks);

void update_brightness(struct AcpiData const* vals, int target);

#endif   /* ----- #ifndef ACPI_FUNCS_INC  ----- */

