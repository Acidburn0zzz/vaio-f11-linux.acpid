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

#include "acpi_funcs.h"

void acpi_event_loop(int fd, int bl_ctrl) {
    char* buf = NULL;

    char* evt_toks[4];
    unsigned int i = 0;

    struct AcpiData const vals = init_acpi_data(bl_ctrl);

    while ((buf = read_line(fd)) != NULL) {
        for (i = 0; i != 4; ++i) { /* Assuming it's always 4 items */
            if (!i)
                evt_toks[i] = strtok(buf, " ");
            else evt_toks[i] = strtok(NULL, " ");
        }

        handle_acpi_events(&vals, evt_toks);
    }
}

void handle_acpi_events(struct AcpiData const* vals, char const* evt_toks[4]) {
    /* Assuming all params are valid */
    int als_brgt = -1;
    int current_brgt = -1;
    int new_brgt = -1;
    int kbd_bl = 0;

    if (!strcmp(evt_toks[0], SONY_EVENT_CLASS) &&
        !strcmp(evt_toks[1], SONY_EVENT_TYPE) &&
        !strcmp(evt_toks[2], SONY_EVENT_MAJOR)) {
        /* Ambient lighting changed */
        if (!strcmp(evt_toks[3], SONY_EVENT_ALS)) {
            als_brgt = read_int_from_file(SONY_ALS_BL);
            current_brgt = read_int_from_file(vals->bl_ctrl == BC_ACPI ?
                                              ACPI_BL_BRGT : NVIDIA_BL_BRGT);
            new_brgt = als_brgt/100.0f*(vals->max_brgt-vals->min_brgt)+
                       vals->min_brgt;

            update_brightness(vals->bl_ctrl == BC_ACPI ? ACPI_BL_BRGT : NVIDIA_BL_BRGT,
                              current_brgt, new_brgt);

            /* Turn on keyboard backlight in dim lighting */
            kbd_bl = read_int_from_file(SONY_KBD_BL);
            if ((als_brgt < AMBIENT_TOO_DIM)^kbd_bl)
                write_int_to_file(SONY_KBD_BL, !kbd_bl);
        }

        /* Backlight changed by user */
    }
}

struct AcpiData init_acpi_data(int bl_ctrl) {
    struct AcpiData vals;

    vals.bl_ctrl = bl_ctrl;

    if (bl_ctrl == BC_ACPI) {
        vals.max_brgt = read_int_from_file(ACPI_BL_BRGT_MAX);
        vals.min_brgt = 0;
        vals.brightness = read_int_from_file(ACPI_BL_BRGT);
    }
    else { /* BC_NVIDIA */
        vals.max_brgt = read_int_from_file(NVIDIA_BL_BRGT_MAX);
        vals.min_brgt = 1500;
        vals.brightness = read_int_from_file(NVIDIA_BL_BRGT);
    }

    return vals;
}

void update_brightness(char const* path, int current, int target) {
    struct timespec const ts = {0, 50*1000*1000};
    float const step = (target-current)/10.0;
    unsigned int i = 1;

    while (i != 11) {
        write_int_to_file(path, current+step*i);
        nanosleep(&ts, NULL); /* There is a failure possibility */
        ++i;
    }
}
