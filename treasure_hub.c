#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define CMD_FILE "cmd.txt"

pid_t monitor_pid = -1;
int waiting_monitor_exit = 0;

void sigchld_handler(int sig) {
    int status;
    pid_t pid = waitpid(-1, &status, WNOHANG);
    if (pid == monitor_pid) {
        monitor_pid = -1;
        waiting_monitor_exit = 0;
        printf("[INFO] Monitor process terminated.\n");
    }
}

void send_command_to_monitor(const char *command) {
    FILE *f = fopen(CMD_FILE, "w");
    if (!f) {
        perror("Error writing to command file");
        return;
    }
    fprintf(f, "%s\n", command);
    fclose(f);
    kill(monitor_pid, SIGUSR1);
}

void start_monitor() {
    if (monitor_pid > 0) {
        printf("Monitor already running.\n");
        return;
    }
    
    monitor_pid = fork();
    if (monitor_pid == 0) {
        // Fils : lance treasure_manager en mode "monitor"
        execl("./treasure_manager", "treasure_manager", "monitor", NULL);
        perror("Failed to exec monitor");
        exit(1);
    } else {
        printf("Monitor started (PID %d).\n", monitor_pid);
    }
}

void stop_monitor() {
    if (monitor_pid <= 0) {
        printf("No monitor is currently running.\n");
        return;
    }
    printf("Stopping monitor...\n");
    waiting_monitor_exit = 1;
    kill(monitor_pid, SIGUSR2); // on suppose que SIGUSR2 arrête le monitor
}

int main() {
    signal(SIGCHLD, sigchld_handler);

    char input[256];

    while (1) {
        printf("hub> ");
        fflush(stdout);
        if (!fgets(input, sizeof(input), stdin)) break;

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "start_monitor") == 0) {
            start_monitor();

        } else if (strcmp(input, "list_hunts") == 0 ||
                   strncmp(input, "list_treasures", 14) == 0 ||
                   strncmp(input, "view_treasure", 13) == 0) {

            if (monitor_pid <= 0) {
                printf("Monitor not running. Use start_monitor first.\n");
                continue;
            }
            send_command_to_monitor(input);

        } else if (strcmp(input, "stop_monitor") == 0) {
            stop_monitor();

        } else if (strcmp(input, "exit") == 0) {
            if (monitor_pid > 0) {
                printf("Cannot exit while monitor is running. Use stop_monitor first.\n");
                continue;
            }
            printf("Exiting hub.\n");
            break;

        } else {
            printf("Unknown command: %s\n", input);
        }

        if (waiting_monitor_exit) {
            printf("Waiting for monitor to terminate...\n");
            while (monitor_pid > 0) {
                sleep(1);
            }
        }
    }

    return 0;
}
