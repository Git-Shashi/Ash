#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>

void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s [-p] [-v] directory...\n", prog_name);
}

int remove_directory(const char *dirname, int verbose) {
    if (rmdir(dirname) == 0) {
        if (verbose) {
            printf("removed directory '%s'\n", dirname);
        }
        return 0;
    } else {
        fprintf(stderr, "rmdir: failed to remove '%s': %s\n", dirname, strerror(errno));
        return -1;
    }
}

int remove_parent_directories(const char *dirname, int verbose) {
    char *path = strdup(dirname);
    if (!path) {
        perror("strdup");
        return -1;
    }

    int result = 0;
    while (1) {
        result = remove_directory(path, verbose);
        if (result != 0) {
            break;
        }

        char *last_slash = strrchr(path, '/');
        if (!last_slash) {
            break;
        }

        *last_slash = '\0';
        if (strlen(path) == 0) {
            break;
        }
    }

    free(path);
    return result;
}

int main(int argc, char *argv[]) {
    int opt;
    int remove_parents = 0;
    int verbose = 0;

    while ((opt = getopt(argc, argv, "pv")) != -1) {
        switch (opt) {
            case 'p':
                remove_parents = 1;
                break;
            case 'v':
                verbose = 1;
                break;
            default:
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    if (optind >= argc) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    for (int i = optind; i < argc; i++) {
        if (remove_parents) {
            remove_parent_directories(argv[i], verbose);
        } else {
            remove_directory(argv[i], verbose);
        }
    }

    return EXIT_SUCCESS;
}
