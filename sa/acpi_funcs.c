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
#include <sys/stat.h>

void acpi_event_loop(int fd) {
    char* buf = NULL;

    char* evt_toks[4];
    unsigned int i = 0;

    struct AcpiData vals = init_acpi_data();

    int nfds;
    fd_set set;
    struct timeval tv = {0, 0};

    while (1) {
        nfds = 0;
        while (!nfds) {
            update_brightness(&vals, &tv.tv_usec);
            FD_ZERO(&set);
            FD_SET(fd, &set);
            nfds = select(fd+1, &set, NULL, NULL, tv.tv_usec ? &tv : NULL);
        }

        if ((buf = read_line(fd)) == NULL)
            continue;

        for (i = 0; i != 4; ++i) { /* Assuming it's always 4 items */
            if (!i)
                evt_toks[i] = strtok(buf, " ");
            else evt_toks[i] = strtok(NULL, " ");
        }

        handle_acpi_events(&vals, evt_toks);
    }
}

void handle_acpi_events(struct AcpiData* vals, char** evt_toks) {
    /* Assuming all params are valid */
    float const MEANINGFUL_ALS_CHANGES = 1.0f;
    float als_lux = -1.0f;
    int do_update_brgt = 0;

    if (!strcmp(evt_toks[0], SONY_EVENT_CLASS) &&
        !strcmp(evt_toks[1], SONY_EVENT_TYPE)) {
        als_lux = read_float_from_file(SONY_ALS_LUX);

        if (!strcmp(evt_toks[2], SONY_EVENT_BL_BRGT)) {
            if (!strcmp(evt_toks[3], SONY_EVENT_BL_BRGT_UP) &&
                vals->current_acpi_brgt < ACPI_MAX_BRGT) {
                ++vals->current_acpi_brgt;
                do_update_brgt = 1;
            }
            else if (!strcmp(evt_toks[3], SONY_EVENT_BL_BRGT_DOWN) &&
                     vals->current_acpi_brgt > ACPI_MIN_BRGT) {
                --vals->current_acpi_brgt;
                do_update_brgt = 1;
            }
        }
        else if (!strcmp(evt_toks[2], SONY_EVENT_ALS) &&
                 !strcmp(evt_toks[3], SONY_EVENT_ALS_CHANGED)) {
            if (fabsf(als_lux-vals->prev_lux) > MEANINGFUL_ALS_CHANGES) {
                vals->prev_lux = als_lux;

                if (vals->kbd_bl) {
                    int kbd_bl = read_int_from_file(SONY_KBD_BL);

                    /* Turn on keyboard backlight in dim lighting */
                    if ((als_lux < AMBIENT_TOO_DIM)^kbd_bl)
                        write_int_to_file(SONY_KBD_BL, !kbd_bl);
                }

                do_update_brgt = 1;
            }
        }

        if (do_update_brgt) {
            vals->new_brgt = (20.24f*logf(als_lux)+17.51f)+
                       (vals->current_acpi_brgt/(float)ACPI_MAX_BRGT*vals->brgt_range);
            if (vals->new_brgt < vals->brgt_levels[ACPI_MIN_BRGT])
                vals->new_brgt = vals->brgt_levels[ACPI_MIN_BRGT];
            else if (vals->new_brgt > vals->brgt_levels[ACPI_MAX_BRGT])
                vals->new_brgt = vals->brgt_levels[ACPI_MAX_BRGT];
            printf("Target brightness: %i - ACPI brightness: %i\n", vals->new_brgt, vals->current_acpi_brgt);
        }
    }
}

struct AcpiData init_acpi_data() {
    struct AcpiData vals;
    struct stat st;

    vals.kbd_bl = !stat(SONY_KBD_BL, &st);
    vals.prev_lux = read_float_from_file(SONY_ALS_LUX);
    read_hex_from_file(SONY_ALS_PARAMS, vals.brgt_levels, ACPI_MAX_BRGT+1);
    vals.brgt_range = vals.brgt_levels[ACPI_MAX_BRGT]-vals.brgt_levels[ACPI_MIN_BRGT];
    vals.current_brgt = read_int_from_file(SONY_BL_BRGT);
    vals.new_brgt = vals.current_brgt;
    vals.current_acpi_brgt = (ACPI_MAX_BRGT-ACPI_MIN_BRGT)/2;
    write_int_to_file(SONY_ALS_MANAGED, 1);

    return vals;
}

void update_brightness(struct AcpiData* vals, long* usec) {
    float const PERCENTAGE_INCREASE = 0.01275f;
    float step = (vals->brgt_levels[ACPI_MAX_BRGT]-vals->brgt_levels[ACPI_MIN_BRGT])*PERCENTAGE_INCREASE;

    if (vals->current_brgt == vals->new_brgt) {
        *usec = 0;
        return;
    }
    else if (!*usec)
        *usec = 50*1000;

    if (vals->current_brgt < vals->new_brgt)
        vals->current_brgt = MIN(vals->current_brgt+step, vals->new_brgt);
    else
        vals->current_brgt = MAX(vals->current_brgt-step, vals->new_brgt);

    write_int_to_file(SONY_BL_BRGT, vals->current_brgt);
}
