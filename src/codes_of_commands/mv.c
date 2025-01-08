#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

void move_file(const char *source, const char *destination, int interactive, int no_clobber) {
    struct stat statbuf;
    if (stat(destination, &statbuf) == 0) {
        if (no_clobber) {
            printf("Skipping '%s' as it already exists.\n", destination);
            return;
        }
        if (interactive) {
            printf("Overwrite '%s'? (y/n): ", destination);
            char response = getchar();
            if (response != 'y' && response != 'Y') {
                printf("Skipping '%s'.\n", destination);
                return;
            }
        }
    }
    if (rename(source, destination) != 0) {
        perror("Error moving file");
    }
}

void move_directory(const char *source, const char *destination, int interactive, int no_clobber) {
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    char src_path[PATH_MAX];
    char dest_path[PATH_MAX];

    if (mkdir(destination, 0755) < 0 && errno != EEXIST) {
        perror("Error creating destination directory");
        return;
    }

    dir = opendir(source);
    if (!dir) {
        perror("Error opening source directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(src_path, sizeof(src_path), "%s/%s", source, entry->d_name);
        snprintf(dest_path, sizeof(dest_path), "%s/%s", destination, entry->d_name);

        if (stat(src_path, &statbuf) == 0) {
            if (S_ISDIR(statbuf.st_mode)) {
                move_directory(src_path, dest_path, interactive, no_clobber);
            } else if (S_ISREG(statbuf.st_mode)) {
                move_file(src_path, dest_path, interactive, no_clobber);
            }
        }
    }

    closedir(dir);

    // Remove the source directory after moving its contents
    if (rmdir(source) != 0) {
        perror("Error removing source directory");
    }
}

int main(int argc, char *argv[]) {
    struct stat statbuf;
    int interactive = 0;
    int no_clobber = 0;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s [-i|-f|-n] <source> <destination>\n", argv[0]);
        return 1;
    }

    int opt;
    while ((opt = getopt(argc, argv, "ifn")) != -1) {
        switch (opt) {
            case 'i':
                interactive = 1;
                break;
            case 'f':
                interactive = 0;
                no_clobber = 0;
                break;
            case 'n':
                no_clobber = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-i|-f|-n] <source> <destination>\n", argv[0]);
                return 1;
        }
    }

    if (optind + 2 != argc) {
        fprintf(stderr, "Usage: %s [-i|-f|-n] <source> <destination>\n", argv[0]);
        return 1;
    }

    char *source = argv[optind];
    char *destination = argv[optind + 1];

    if (stat(source, &statbuf) != 0) {
        perror("Error accessing source");
        return 1;
    }

    if (S_ISDIR(statbuf.st_mode)) {
        move_directory(source, destination, interactive, no_clobber);
    } else if (S_ISREG(statbuf.st_mode)) {
        move_file(source, destination, interactive, no_clobber);
    } else {
        fprintf(stderr, "Source is neither a file nor a directory\n");
        return 1;
    }

    printf("Move completed successfully.\n");
    return 0;
}
