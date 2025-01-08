#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <utime.h>
#include <string.h>
#include <errno.h>

void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s <file>\n", prog_name);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    const char *filename = argv[1];
    int fd;

    // Try to open the file
    fd = open(filename, O_WRONLY | O_CREAT, 0666);
    if (fd < 0) {
        fprintf(stderr, "Error opening/creating file '%s': %s\n", filename, strerror(errno));
        return EXIT_FAILURE;
    }

    // Close the file descriptor
    close(fd);

    // Update the access and modification times
    if (utime(filename, NULL) < 0) {
        fprintf(stderr, "Error updating times for file '%s': %s\n", filename, strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
