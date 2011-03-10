#ifndef  FILE_FUNCS_INC
#define  FILE_FUNCS_INC

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

FILE* open_file(char const* path, char const* mode);

int read_int_from_file(char const* path);
void write_int_to_file(char const* path, int val);

int ud_connect(char const* name);

char* read_line(int fd);

#endif   /* ----- #ifndef FILE_FUNCS_INC  ----- */

