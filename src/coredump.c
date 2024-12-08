#include "coredump.h"
#include <limits.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "elf_utils.h"

int create_coredump(const pid_t pid) {
    maps_entry_t* pid_maps;
    char coredump_path[32];
    int coredump_fd;
    size_t phdr_count;
    int ret;

    pid_maps = parse_procfs_maps(pid);
    if (!pid_maps) {
        fprintf(stderr,
                "Error occured while parsing /proc/%d/maps\n",
                pid);
        return -1;
    }

    print_maps_list(pid_maps);

    phdr_count = count_proc_maps(pid_maps);
    printf("[DEBUG] Prog headers: %zu\n", phdr_count);

    snprintf(coredump_path, sizeof(coredump_path), "%d_coredump", pid);

    coredump_fd = open(coredump_path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (coredump_fd < 0) {
        fprintf(stderr,
                "Error occured while creating file %s\n",
                coredump_path);
        free_maps_list(pid_maps);
        return -1;
    }

    ret = write_elf_header(coredump_fd, phdr_count);
    if (ret < 0) {
        fprintf(stderr,
                "Error occured while writing to file %s\n",
                coredump_path);
        free_maps_list(pid_maps);
        close(coredump_fd);
        return -1;
    }

    ret = write_program_headers(coredump_fd, pid_maps, pid);
    if (ret < 0) {
        fprintf(stderr,
                "Error occured while writing to file %s\n",
                coredump_path);
        free_maps_list(pid_maps);
        close(coredump_fd);
        return -1;
    }

    close(coredump_fd);

    free_maps_list(pid_maps);

    return 0;
}
