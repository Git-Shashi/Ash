#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s [-f] [-i] [-r|-R] [-d] [-v] file...\n", prog_name);
}

int remove_file(const char *filename, int force, int interactive, int verbose) {
    if (interactive) {
        printf("rm: remove file '%s'? ", filename);
        int response = getchar();
        if (response != 'y' && response != 'Y') {
            return 0;
        }
    }

    if (unlink(filename) == 0) {
        if (verbose) {
            printf("removed '%s'\n", filename);
        }
        return 0;
    } else {
        if (!force) {
            fprintf(stderr, "rm: cannot remove '%s': %s\n", filename, strerror(errno));
        }
        return -1;
    }
}

int remove_directory(const char *dirname, int force, int interactive, int verbose, int recursive) {
    DIR *dir = opendir(dirname);
    if (!dir) {
        if (!force) {
            fprintf(stderr, "rm: cannot remove '%s': %s\n", dirname, strerror(errno));
        }
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dirname, entry->d_name);

        struct stat path_stat;
        stat(path, &path_stat);

        if (S_ISDIR(path_stat.st_mode)) {
            if (recursive) {
                remove_directory(path, force, interactive, verbose, recursive);
            } else {
                fprintf(stderr, "rm: cannot remove '%s': Is a directory\n", path);
            }
        } else {
            remove_file(path, force, interactive, verbose);
        }
    }
    closedir(dir);

    if (rmdir(dirname) == 0) {
        if (verbose) {
            printf("removed directory '%s'\n", dirname);
        }
        return 0;
    } else {
        if (!force) {
            fprintf(stderr, "rm: cannot remove directory '%s': %s\n", dirname, strerror(errno));
        }
        return -1;
    }
}

int main(int argc, char *argv[]) {
    int force = 0, interactive = 0, recursive = 0, verbose = 0, remove_empty_dirs = 0;
    int opt;

    while ((opt = getopt(argc, argv, "firRdv")) != -1) {
        switch (opt) {
            case 'f':
                force = 1;
                break;
            case 'i':
                interactive = 1;
                break;
            case 'r':
            case 'R':
                recursive = 1;
                break;
            case 'd':
                remove_empty_dirs = 1;
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
        struct stat path_stat;
        if (stat(argv[i], &path_stat) != 0) {
            if (!force) {
                fprintf(stderr, "rm: cannot remove '%s': %s\n", argv[i], strerror(errno));
            }
            continue;
        }

        if (S_ISDIR(path_stat.st_mode)) {
            if (remove_empty_dirs || recursive) {
                remove_directory(argv[i], force, interactive, verbose, recursive);
            } else {
                fprintf(stderr, "rm: cannot remove '%s': Is a directory\n", argv[i]);
            }
        } else {
            remove_file(argv[i], force, interactive, verbose);
        }
    }

    return EXIT_SUCCESS;
}
