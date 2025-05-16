#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <dirent.h>

pid_t monitor_pid = -1;
int pipe_fds[2];

void send_command_to_monitor(const char *cmd) {
    int fd = open("monitor_command", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open monitor_command");
        return;
    }
    write(fd, cmd, strlen(cmd));
    close(fd);

    if (monitor_pid != -1) {
        kill(monitor_pid, SIGUSR1);
    }
}

void read_monitor_output() {
    char buffer[1024];
    ssize_t count;
    usleep(100000); // petite pause pour laisser le monitor écrire
    while ((count = read(pipe_fds[0], buffer, sizeof(buffer)-1)) > 0) {
        buffer[count] = '\0';
        printf("%s", buffer);
        if (count < sizeof(buffer) - 1) break; // Stop si tout est lu
    }
}

void start_monitor() {
    if (monitor_pid != -1) {
        printf("Monitor already running.\n");
        return;
    }

    if (pipe(pipe_fds) == -1) {
        perror("pipe");
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
        close(pipe_fds[0]); // fermer lecture dans l’enfant
        dup2(pipe_fds[1], STDOUT_FILENO);
        close(pipe_fds[1]);

        char arg[16];
        snprintf(arg, sizeof(arg), "%d", STDOUT_FILENO);
        execl("./monitor", "monitor", arg, NULL);
        perror("execl");
        exit(1);
    } else if (pid > 0) {
        close(pipe_fds[1]); // fermer écriture dans le parent
        monitor_pid = pid;
        printf("Monitor started with pid %d\n", monitor_pid);
    } else {
        perror("fork");
    }
}

void stop_monitor() {
    if (monitor_pid == -1) {
        printf("Monitor is not running.\n");
        return;
    }
    kill(monitor_pid, SIGTERM);
    waitpid(monitor_pid, NULL, 0);
    close(pipe_fds[0]);
    monitor_pid = -1;
    printf("Monitor stopped.\n");
}

void calculate_score() {
    DIR *dir = opendir("./hunts");
    if (!dir) {
        perror("opendir");
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR &&
            strcmp(entry->d_name, ".") != 0 &&
            strcmp(entry->d_name, "..") != 0) {

            pid_t pid = fork();
            if (pid == 0) {
                execl("./score_calculator", "score_calculator", entry->d_name, NULL);
                perror("execl");
                exit(1);
            } else if (pid > 0) {
                waitpid(pid, NULL, 0);
            }
        }
    }
    closedir(dir);
}

int main() {
    char command[128];
    while (1) {
        printf("treasure_hub> ");
        if (!fgets(command, sizeof(command), stdin)) break;
        command[strcspn(command, "\n")] = 0;

        if (strncmp(command, "start_monitor", 13) == 0) {
            start_monitor();
        } else if (strncmp(command, "stop_monitor", 12) == 0) {
            stop_monitor();
        } else if (strncmp(command, "list_hunts", 10) == 0) {
            send_command_to_monitor("list_hunts");
            read_monitor_output();
        } else if (strncmp(command, "list_treasures", 14) == 0) {
            char hunt_id[64];
            if (sscanf(command, "list_treasures %63s", hunt_id) == 1) {
                char buf[128];
                snprintf(buf, sizeof(buf), "list_treasures %s", hunt_id);
                send_command_to_monitor(buf);
                read_monitor_output();
            } else {
                printf("Usage: list_treasures <hunt_id>\n");
            }
        } else if (strncmp(command, "calculate_score", 15) == 0) {
            calculate_score();
        } else if (strncmp(command, "exit", 4) == 0) {
            if (monitor_pid != -1) {
                printf("Please stop monitor first.\n");
            } else {
                break;
            }
        } else {
            printf("Unknown command\n");
        }
    }

    return 0;
}
