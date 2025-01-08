#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

void print_permissions(mode_t mode) {
    printf("%c", S_ISDIR(mode) ? 'd' : '-');
    printf("%c", mode & S_IRUSR ? 'r' : '-');
    printf("%c", mode & S_IWUSR ? 'w' : '-');
    printf("%c", mode & S_IXUSR ? 'x' : '-');
    printf("%c", mode & S_IRGRP ? 'r' : '-');
    printf("%c", mode & S_IWGRP ? 'w' : '-');
    printf("%c", mode & S_IXGRP ? 'x' : '-');
    printf("%c", mode & S_IROTH ? 'r' : '-');
    printf("%c", mode & S_IWOTH ? 'w' : '-');
    printf("%c", mode & S_IXOTH ? 'x' : '-');
}

void print_file_info(const char *path, const char *name, int long_format) {
    if (long_format) {
        struct stat file_stat;
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, name);
        if (stat(full_path, &file_stat) == -1) {
            perror("stat");
            return;
        }

        print_permissions(file_stat.st_mode);
        printf("%hu", file_stat.st_nlink);

        struct passwd *pw = getpwuid(file_stat.st_uid);
        struct group *gr = getgrgid(file_stat.st_gid);
        printf(" %s %s", pw->pw_name, gr->gr_name);

        printf(" %5lld", file_stat.st_size);

        char timebuf[80];
        struct tm *timeinfo = localtime(&file_stat.st_mtime);
        strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", timeinfo);
        printf(" %s", timebuf);

        printf(" %s\n", name);
    } else {
        printf("%s\n", name);
    }
}

void list_directory(const char *path, int show_all, int recursive, int long_format) {
    DIR *directory = opendir(path);
    if (directory == NULL) {
        perror("Unable to open directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL) {
        if (!show_all && entry->d_name[0] == '.') {
            continue;
        }
        print_file_info(path, entry->d_name, long_format);
    }

    if (recursive) {
        rewinddir(directory);
        while ((entry = readdir(directory)) != NULL) {
            if (entry->d_type == DT_DIR && 
                strcmp(entry->d_name, ".") != 0 && 
                strcmp(entry->d_name, "..") != 0) {
                char new_path[1024];
                snprintf(new_path, sizeof(new_path), "%s/%s", path, entry->d_name);
                printf("\n%s:\n", new_path);
                list_directory(new_path, show_all, recursive, long_format);
            }
        }
    }

    closedir(directory);
}

int main(int argc, char *argv[]) {
    int show_all = 0, recursive = 0, long_format = 0;
    const char *path = ".";

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            for (int j = 1; argv[i][j] != '\0'; j++) {
                switch (argv[i][j]) {
                    case 'a':
                        show_all = 1;
                        break;
                    case 'R':
                        recursive = 1;
                        break;
                    case 'l':
                        long_format = 1;
                        break;
                    default:
                        fprintf(stderr, "Unknown option: -%c\n", argv[i][j]);
                        return EXIT_FAILURE;
                }
            }
        } else {
            path = argv[i];
        }
    }

    list_directory(path, show_all, recursive, long_format);
    return EXIT_SUCCESS;
}
