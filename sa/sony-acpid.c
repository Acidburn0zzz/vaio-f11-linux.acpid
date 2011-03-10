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

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "file_funcs.h"
#include "acpi_funcs.h"

int main(int argc, char** argv) {
    char const*const ACPID_SOCKET_FILE = "/var/run/acpid.socket";
    int const sock_fd = ud_connect(ACPID_SOCKET_FILE);
    int bl_ctrl = BC_ACPI;
    int opt = -1;

    while ((opt = getopt(argc, argv, "c:t:")) != -1) {
        switch (opt) {
            case 'c':
                if (!strcmp(optarg, "nvidia"))
                    bl_ctrl = BC_NVIDIA;
                break;
            case 't':
                write_int_to_file(SONY_ALS_LUX_THRESHOLD, atoi(optarg));
                break;
            default:
                fprintf(stderr, "Usage: %s [-c acpi|nvidia] [-t number]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    acpi_event_loop(sock_fd, bl_ctrl);

    /* We may never reach this line unless we handle terminate signal */
    close(sock_fd);

    return 0;
}
