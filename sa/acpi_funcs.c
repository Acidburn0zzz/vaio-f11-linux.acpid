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

void acpi_event_loop(int fd) {
    char* buf = NULL;

    char* evt_toks[4];
    unsigned int i = 0;

    struct AcpiData vals = init_acpi_data();

    while ((buf = read_line(fd)) != NULL) {
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
    int new_brgt = 0;
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

                /* Turn on keyboard backlight in dim lighting */
                if ((als_lux < AMBIENT_TOO_DIM)^vals->kbd_bl) {
                    vals->kbd_bl = !vals->kbd_bl;
                    write_int_to_file(SONY_KBD_BL, vals->kbd_bl);
                }

                do_update_brgt = 1;
            }
        }

        if (do_update_brgt) {
            new_brgt = (20.24f*logf(als_lux)+17.51f)+
                       (vals->current_acpi_brgt/(float)ACPI_MAX_BRGT*vals->brgt_range);
            if (new_brgt < vals->brgt_levels[ACPI_MIN_BRGT])
                new_brgt = vals->brgt_levels[ACPI_MIN_BRGT];
            else if (new_brgt > vals->brgt_levels[ACPI_MAX_BRGT])
                new_brgt = vals->brgt_levels[ACPI_MAX_BRGT];
            printf("Target brightness: %i - ACPI brightness: %i\n", new_brgt, vals->current_acpi_brgt);
            update_brightness(vals, new_brgt);
            vals->current_brgt = new_brgt;
        }
    }
}

struct AcpiData init_acpi_data() {
    struct AcpiData vals;

    vals.kbd_bl = read_int_from_file(SONY_KBD_BL);
    vals.prev_lux = read_float_from_file(SONY_ALS_LUX);
    read_hex_from_file(SONY_ALS_PARAMS, vals.brgt_levels, ACPI_MAX_BRGT+1);
    vals.brgt_range = vals.brgt_levels[ACPI_MAX_BRGT]-vals.brgt_levels[ACPI_MIN_BRGT];
    vals.current_brgt = read_int_from_file(SONY_BL_BRGT);
    vals.current_acpi_brgt = (ACPI_MAX_BRGT-ACPI_MIN_BRGT)/2;

    return vals;
}

void update_brightness(struct AcpiData const* vals, int target) {
    struct timespec const ts = {0, 50*1000*1000};
    int current = vals->current_brgt;
    float const PERCENTAGE_INCREASE = 0.01275f;
    float step = (vals->brgt_levels[ACPI_MAX_BRGT]-vals->brgt_levels[ACPI_MIN_BRGT])*PERCENTAGE_INCREASE;

    if (target == current)
        return;
    else if (target < current)
        step = -step;

    while (abs(target-current) > abs(step)) {
        current += step;
        write_int_to_file(SONY_BL_BRGT, current);
        nanosleep(&ts, NULL); /* There is a failure possibility */
    }
    write_int_to_file(SONY_BL_BRGT, target);
}
