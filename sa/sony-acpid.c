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
