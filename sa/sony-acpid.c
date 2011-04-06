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
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>

#include "file_funcs.h"
#include "acpi_funcs.h"

int sock_fd = -1;

void sig_handler(int signum) {
    (void)signum; /* Shut up the compiler */

    write_int_to_file(SONY_ALS_MANAGED, 0);
    write_int_to_file(SONY_ALS_POWER, 0);

    close(sock_fd);

    exit(EXIT_SUCCESS);
}

int main() {
    char const*const SONY_LAPTOP_DIR = "/sys/devices/platform/sony-laptop";
    char const*const ACPID_SOCKET_FILE = "/var/run/acpid.socket";
    struct stat st;
    struct sigaction act;

    if (stat(SONY_LAPTOP_DIR, &st)) {
        printf("%s not found, is sony-laptop module loaded?\n", SONY_LAPTOP_DIR);
        exit(EXIT_FAILURE);
    }

    sock_fd = ud_connect(ACPID_SOCKET_FILE);

    memset(&act, 0, sizeof(act));
    act.sa_handler = sig_handler;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGHUP, &act, NULL);

    write_int_to_file(SONY_ALS_POWER, 1);

    acpi_event_loop(sock_fd);

    return 0;
}
