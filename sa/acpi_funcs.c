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

    struct AcpiData vals = init_acpi_data(bl_ctrl);

    int nfds;
    fd_set set;
    struct timeval tv = {0, 0};

    for (;;) {
        nfds = 0;
        while (!nfds) {
            update_brightness(&vals, &tv.tv_usec);
            FD_ZERO(&set);
            FD_SET(fd, &set);
            nfds = select(fd+1, &set, NULL, NULL, tv.tv_usec ? &tv : NULL);
        }

        if ((buf = read_line(fd)) == NULL)
            break;

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
    int als_lux = -1;
    int current_brgt = -1;
    int new_brgt = -1;
    int kbd_bl = 0;
    int i;

    if (!strcmp(evt_toks[0], SONY_EVENT_CLASS) &&
        !strcmp(evt_toks[1], SONY_EVENT_TYPE) &&
        !strcmp(evt_toks[2], SONY_EVENT_MAJOR)) {
        current_brgt = read_int_from_file(vals->brgt_path);
        als_lux = read_int_from_file(SONY_ALS_LUX);

        if (!als_lux)
            ++als_lux;

        if (!strcmp(evt_toks[3], SONY_EVENT_ALS)) { /* Ambient lighting changed */
            /* Turn on keyboard backlight in dim lighting */
            kbd_bl = read_int_from_file(SONY_KBD_BL);
            if ((als_lux < AMBIENT_TOO_DIM)^kbd_bl)
                write_int_to_file(SONY_KBD_BL, !kbd_bl);

            if (new_brgt > vals->max_brgt)
                new_brgt = vals->max_brgt;
            else if (new_brgt < vals->min_brgt)
                new_brgt = vals->min_brgt;

            vals->current_brgt = current_brgt;
            vals->new_brgt = new_brgt;
        }
        else if (!strcmp(evt_toks[3], SONY_EVENT_OLDALS)) { /* Ambient lighting changed */

            if (als_lux < 1000)
                new_brgt = 20.24f * log(als_lux) + 17.51f;
            else if (als_lux < 4630)
                new_brgt = 130 + als_lux * 27 / 1000;
            else
                new_brgt = vals->max_brgt;

            if (vals->table) {
                for (i = 0; i < vals->table; i++)
                    if (new_brgt <= vals->tbl_brgt[i])
                        break;

                vals->new_brgt = i;
            } else {
                vals->current_brgt = current_brgt;
                vals->new_brgt = new_brgt;
            }
        }
        else if (!strcmp(evt_toks[3], SONY_BL_BRGT_UP)) { /* User increased bl */
            if (vals->table) {
                if (vals->new_brgt < vals->table)
                    vals->new_brgt++;
            } else
                for (i = 0; i < NUMBER_BRGT; i++)
                    if (current_brgt < vals->tbl_brgt[i]) {
                        vals->current_brgt = current_brgt;
                        vals->new_brgt = vals->tbl_brgt[i];
                        break;
                    }
        }
        else if (!strcmp(evt_toks[3], SONY_BL_BRGT_DOWN)) { /* User decreased bl */
            if (vals->table) {
                if (vals->new_brgt > 0)
                    vals->new_brgt--;
            } else
                for (i = NUMBER_BRGT - 1; i >= 0; i--)
                    if (current_brgt > vals->tbl_brgt[i]) {
                        vals->current_brgt = current_brgt;
                        vals->new_brgt = vals->tbl_brgt[i];
                        break;
                    }
        }
    }
}

struct AcpiData init_acpi_data(int bl_ctrl) {
    struct AcpiData vals;
    int i;

    if (bl_ctrl == BC_ACPI) {
        vals.table = read_int_from_file(ACPI_BL_BRGT_MAX);
        vals.brgt_path = ACPI_BL_BRGT;
        read_hex_from_file(SONY_ALS_PARAM, vals.tbl_brgt, vals.table + 1);
        vals.max_brgt = vals.tbl_brgt[vals.table];
        vals.min_brgt = vals.tbl_brgt[0];
    }
    else if (bl_ctrl == BC_NVIDIA) { /* BC_NVIDIA */
        vals.table = 0;
        vals.max_brgt = read_int_from_file(NVIDIA_BL_BRGT_MAX);
        vals.min_brgt = 1500;
        vals.brgt_path = NVIDIA_BL_BRGT;
        for (i = 0; i < NUMBER_BRGT; i++)
            vals.tbl_brgt[i] = vals.min_brgt+i*(vals.max_brgt-vals.min_brgt)/NUMBER_BRGT;
    }
    else { /* BC_SONY */
        vals.table = 0;
        vals.max_brgt = read_int_from_file(SONY_BL_BRGT_MAX);
        vals.min_brgt = 0;
        vals.brgt_path = SONY_BL_BRGT;
        vals.tbl_brgt[0] = 7;
        for (i = 1; i < NUMBER_BRGT; i++)
            vals.tbl_brgt[i] = 1.585f * vals.tbl_brgt[i-1];
    }

    return vals;
}

#define DELTA 6
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))

void update_brightness(struct AcpiData *vals, long *usec) {
    if (vals->current_brgt == vals->new_brgt) {
        *usec = 0;
        return;
    }

    if (vals->current_brgt < vals->new_brgt)
        vals->current_brgt = MIN(MAX(vals->current_brgt + 1, (100 + DELTA) * vals->current_brgt/100), vals->new_brgt);
    else
        vals->current_brgt = MAX(MIN(vals->current_brgt - 1, (100 - DELTA) * vals->current_brgt/100), vals->new_brgt);

    write_int_to_file(vals->brgt_path, vals->current_brgt);

    if (vals->current_brgt == vals->new_brgt)
        *usec = 0;
    else if (!*usec)
        *usec = 50*1000;
}
