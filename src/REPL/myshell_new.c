#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MANUAL_DIR "/Users/shashi/Documents/shell/test/manuals"

void display_manual(const char *command) {
    char manual_path[256];
    snprintf(manual_path, sizeof(manual_path), "%s/%s.txt", MANUAL_DIR, command);

    if (access(manual_path, F_OK) == -1) {
        printf("No manual entry for %s\n", command);
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        return;
    }

    if (pid == 0) {
        execlp("less", "less", manual_path, (char *)NULL);
        perror("execlp failed");
        exit(EXIT_FAILURE);
    } else {
        wait(NULL);
    }
}

int main(void) {
    while (1) {
        printf("Myshell # ");
        size_t buff_size = 0;
        char *input_buf = NULL, *input_cpy = NULL;
        char *token = NULL;
        char *delim = " \n";
        int no_of_token = 0, i = 0;
        char **argv = NULL;

        if (getline(&input_buf, &buff_size, stdin) == -1) {
            perror("getline");
            free(input_buf);
            continue;
        }
        
        if (strncmp(input_buf, "exit", 4) == 0) {
            free(input_buf);
            break;
        }

        input_cpy = strdup(input_buf);
        if (!input_cpy) {
            perror("strdup");
            free(input_buf);
            continue;
        }

        token = strtok(input_buf, delim);
        while (token) {
            no_of_token++;
            token = strtok(NULL, delim);
        }

        argv = malloc(sizeof(char*) * (no_of_token + 1));
        if (!argv) {
            perror("malloc");
            free(input_buf);
            free(input_cpy);
            continue;
        }

        token = strtok(input_cpy, delim);
        while (token) {
            argv[i] = token;
            token = strtok(NULL, delim);
            i++;
        }
        argv[i] = NULL;

        if (strcmp(argv[0], "cd") == 0) {
            if (argv[1] == NULL) {
                fprintf(stderr, "cd: expected argument\n");
            } else {
                if (chdir(argv[1]) != 0) {
                    perror("cd failed");
                }
            }
        } else if (strcmp(argv[0], "man") == 0 && argv[1] != NULL) {
            display_manual(argv[1]);
        } else {
            // Check if the command is a direct path
            if (argv[0][0] == '/' || (argv[0][0] == '.' && argv[0][1] == '/')) {
                // Direct path execution
                if (access(argv[0], X_OK) == 0) {
                    pid_t pid = fork();
                    if (pid < 0) {
                        perror("Fork failed");
                        free(input_buf);
                        free(input_cpy);
                        free(argv);
                        continue;
                    }

                    if (pid == 0) {
                        execve(argv[0], argv, NULL);
                        perror("execve");
                        exit(EXIT_FAILURE);
                    } else {
                        wait(NULL);
                    }
                } else {
                    printf("Command not found or not executable: %s\n", argv[0]);
                }
            } else {
                // Search in PATH
                char *path_env = getenv("execpath");
                char *path = strdup(path_env);
                char *dir = strtok(path, ":");
                char command_path[256];
                int found = 0;

                while (dir != NULL) {
                    snprintf(command_path, sizeof(command_path), "%s/%s", dir, argv[0]);
                    if (access(command_path, X_OK) == 0) {
                        found = 1;
                        break;
                    }
                    dir = strtok(NULL, ":");
                }

                free(path);

                if (found) {
                    pid_t pid = fork();
                    if (pid < 0) {
                        perror("Fork failed");
                        free(input_buf);
                        free(input_cpy);
                        free(argv);
                        continue;
                    }

                    if (pid == 0) {
                        execve(command_path, argv, NULL);
                        perror("execve");
                        exit(EXIT_FAILURE);
                    } else {
                        wait(NULL);
                    }
                } else {
                    printf("Command not found: %s\n", argv[0]);
                }
            }
        }

        free(input_buf);
        free(input_cpy);
        free(argv);
    }

    return 0;
}
