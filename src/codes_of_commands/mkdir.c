#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s [-p] [-m mode] <directory_path>\n", prog_name);
}

int make_path(const char *path, mode_t mode) {
    char *temp_path = strdup(path);
    char *pos = temp_path;
    int status = 0;

    // Iterate over the path and create directories as needed
    while ((pos = strchr(pos, '/')) != NULL) {
        if (pos != temp_path) {
            *pos = '\0'; // Temporarily terminate the string at the current position
            if (mkdir(temp_path, mode) && errno != EEXIST) {
                fprintf(stderr, "Error creating directory '%s': %s\n", temp_path, strerror(errno));
                status = -1;
                break;
            }
            *pos = '/'; // Restore the slash
        }
        pos++;
    }

    // Create the final directory in the path
    if (status == 0 && mkdir(temp_path, mode) && errno != EEXIST) {
        fprintf(stderr, "Error creating directory '%s': %s\n", temp_path, strerror(errno));
        status = -1;
    }

    free(temp_path);
    return status;
}

int main(int argc, char *argv[]) {
    int opt;
    int p_flag = 0;
    mode_t mode = 0755; // Default mode

    // Parse command-line options
    while ((opt = getopt(argc, argv, "pm:")) != -1) {
        switch (opt) {
            case 'p':
                p_flag = 1;
                break;
            case 'm':
                mode = strtol(optarg, NULL, 8);
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

    const char *dir_path = argv[optind];

    if (p_flag) {
        if (make_path(dir_path, mode) == -1) {
            return EXIT_FAILURE;
        }
    } else {
        if (mkdir(dir_path, mode) && errno != EEXIST) {
            fprintf(stderr, "Error creating directory '%s': %s\n", dir_path, strerror(errno));
            return EXIT_FAILURE;
        }
    }

    printf("Directory created successfully.\n");
    return EXIT_SUCCESS;
}
 