#ifndef  HELPERS_INC
#define  HELPERS_INC

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

FILE* open_file(char const* path, char const* mode);

void acpi_event_loop(FILE* fd, void(*event_func)(char const*, char const*,
                                                 char const*, char const*));

void handle_acpi_events(char const* evt_cls, char const* evt_type,
                        char const* evt_major, char const* evt_minor);

#endif   /* ----- #ifndef HELPERS_INC  ----- */

