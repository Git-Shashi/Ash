
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

void print_usage(const char *prog_name) {
    printf("Usage: %s <octal-permissions> <file>\n", prog_name);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    // Convert the permissions from a string to an octal number
    mode_t mode = strtol(argv[1], NULL, 8);
    const char *file = argv[2];

    // Change the file permissions
    if (chmod(file, mode) == -1) {
        perror("chmod");
        return EXIT_FAILURE;
    }

    printf("Permissions of '%s' changed to '%o'\n", file, mode);
    return EXIT_SUCCESS;
}
