#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include "parser.h"
#include "dumper.h"

static void __attribute__((noreturn)) print_usage_exit(const char* name) {
    fprintf(stderr, 
            "Usage: %s -p <PID>\n",
            name);
    exit(EXIT_FAILURE);
}

static void __attribute__((noreturn)) handle_kill_error_exit(const int pid) {
    switch (errno) {
        case EPERM:
            fprintf(stderr, 
                    "Access denied: root privileges are required to control process %d\n", 
                    pid);
            break;
        case ESRCH:
            fprintf(stderr, 
                    "Error: process %d or process group cannot be found\n", 
                    pid);
            break;
        default:
            fprintf(stderr, 
                    "Error: unknown error occurred while controlling process %d\n", 
                    pid);
            break;
    }
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
    int opt;
    int ret;
    pid_t pid = 0;

    while ((opt = getopt(argc, argv, "p:")) > 0) {
        switch (opt) {
            case 'p':
                pid = atoi(optarg);
                if (pid <= 0) {
                    print_usage_exit(argv[0]);
                }
                break;
            case '?':
            default:
                print_usage_exit(argv[0]);
        }
    }

    if (!pid) {
        print_usage_exit(argv[0]);
    }

    ret = kill(pid, SIGSTOP);
    if (ret < 0) {
        handle_kill_error_exit(pid);
    }

    printf("[DEBUG] Proccess %d has been stopped\n", pid);

    maps_entry_t* pid_maps = parse_procfs_maps(pid);
    if (!pid_maps) {
        fprintf(stderr,
                "Error occured while parsing /proc/%d/maps\n", 
                pid);
    } else {
        print_maps_list(pid_maps);
        /*
        ret = dump_procfs_mem(pid, pid_maps);
        if (ret < 0) {
            fprintf(stderr,
                    "Error occured while dumping memory of process %d\n",
                    pid);
        }
        */

        free_maps_list(pid_maps);
    }

    ret = kill(pid, SIGCONT);
    if (ret < 0) {
        handle_kill_error_exit(pid);
    }

    printf("[DEBUG] Process %d has been resumed\n", pid);

    return EXIT_SUCCESS;
}
