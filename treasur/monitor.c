#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define BASE_PATH "./hunts/"
#define MAX_USERNAME 32
#define MAX_CLUE 128

typedef struct {
    int id;
    char username[MAX_USERNAME];
    float latitude;
    float longitude;
    char clue[MAX_CLUE];
    int value;
} Treasure;

volatile sig_atomic_t sigusr1_received = 0;
volatile sig_atomic_t sigterm_received = 0;

void handle_sigusr1(int signo) {
    sigusr1_received = 1;
}

void handle_sigterm(int signo) {
    sigterm_received = 1;
}

void print_hunts(int out_fd) {
    DIR *dir = opendir(BASE_PATH);
    if (!dir) {
        dprintf(out_fd, "Could not open hunts directory\n");
        return;
    }

    struct dirent *entry;
    dprintf(out_fd, "Monitor: Listing hunts\n");
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR &&
            strcmp(entry->d_name, ".") != 0 &&
            strcmp(entry->d_name, "..") != 0) {

            char treasure_file[512];
            snprintf(treasure_file, sizeof(treasure_file), "%s%s/treasures.dat", BASE_PATH, entry->d_name);

            int fd = open(treasure_file, O_RDONLY);
            if (fd < 0) {
                dprintf(out_fd, "Hunt %s: no treasures.dat file\n", entry->d_name);
                continue;
            }

            struct stat st;
            if (fstat(fd, &st) < 0) {
                dprintf(out_fd, "Error reading stats for %s\n", treasure_file);
                close(fd);
                continue;
            }

            int count = st.st_size / sizeof(Treasure);
            dprintf(out_fd, "Hunt %s : %d treasures\n", entry->d_name, count);
            close(fd);
        }
    }
    closedir(dir);
}

void print_treasures(int out_fd, const char *hunt_id) {
    char file_path[512];
    snprintf(file_path, sizeof(file_path), "%s%s/treasures.dat", BASE_PATH, hunt_id);

    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        dprintf(out_fd, "No treasures for hunt %s\n", hunt_id);
        return;
    }

    Treasure t;
    dprintf(out_fd, "Treasures in hunt %s:\n", hunt_id);
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        dprintf(out_fd, "ID: %d, User: %s, Location: (%.2f, %.2f), Value: %d\n",
                t.id, t.username, t.latitude, t.longitude, t.value);
    }
    close(fd);
}

int main(int argc, char *argv[]) {
    int out_fd = STDOUT_FILENO;
    if (argc > 1) {
        out_fd = atoi(argv[1]);
    }

    struct sigaction sa_usr1, sa_term;
    sa_usr1.sa_handler = handle_sigusr1;
    sigemptyset(&sa_usr1.sa_mask);
    sa_usr1.sa_flags = 0;
    sigaction(SIGUSR1, &sa_usr1, NULL);

    sa_term.sa_handler = handle_sigterm;
    sigemptyset(&sa_term.sa_mask);
    sa_term.sa_flags = 0;
    sigaction(SIGTERM, &sa_term, NULL);

    dprintf(out_fd, "Monitor started with pid %d\n", getpid());

    while (!sigterm_received) {
        pause();

        if (sigusr1_received) {
            char cmd[128] = {0};
            int fd = open("monitor_command", O_RDONLY);
            if (fd >= 0) {
                read(fd, cmd, sizeof(cmd) - 1);
                close(fd);
            }

            if (strncmp(cmd, "list_hunts", 10) == 0) {
                print_hunts(out_fd);
            } else if (strncmp(cmd, "list_treasures", 14) == 0) {
                char hunt_id[64];
                if (sscanf(cmd, "list_treasures %63s", hunt_id) == 1) {
                    print_treasures(out_fd, hunt_id);
                } else {
                    dprintf(out_fd, "Invalid list_treasures command format.\n");
                }
            } else {
                dprintf(out_fd, "Unknown command: %s\n", cmd);
            }

            sigusr1_received = 0;
        }
    }

    dprintf(out_fd, "Monitor terminating.\n");
    return 0;
}
