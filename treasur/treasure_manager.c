#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>

#define MAX_USERNAME 32
#define MAX_CLUE     128
#define BASE_PATH    "./hunts/"

typedef struct {
    int id;
    char username[MAX_USERNAME];
    float latitude;
    float longitude;
    char clue[MAX_CLUE];
    int value;
} Treasure;

void log_action(const char *hunt_id, const char *action) {
    char log_path[256];
    snprintf(log_path, sizeof(log_path), BASE_PATH"%s/logged_hunt", hunt_id);
    int log_fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (log_fd < 0) {
        perror("open log");
        return;
    }

    time_t now = time(NULL);
    dprintf(log_fd, "[%s] %s\n", ctime(&now), action);
    close(log_fd);

    // Création du lien symbolique
    char symlink_name[256];
    snprintf(symlink_name, sizeof(symlink_name), "logged_hunt-%s", hunt_id);
    unlink(symlink_name); // Supprime ancien lien s’il existe
    symlink(log_path, symlink_name);
}

void add_treasure(const char *hunt_id) {
    char dir_path[256];
    snprintf(dir_path, sizeof(dir_path), BASE_PATH"%s", hunt_id);
    mkdir(BASE_PATH, 0755); // s'assure que ./hunts existe
    mkdir(dir_path, 0755);

    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s/treasures.dat", dir_path);
    int fd = open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        perror("open");
        return;
    }

    Treasure t;
    printf("ID: "); scanf("%d", &t.id);
    printf("Username: "); scanf("%s", t.username);
    printf("Latitude: "); scanf("%f", &t.latitude);
    printf("Longitude: "); scanf("%f", &t.longitude);
    printf("Clue: "); getchar(); fgets(t.clue, MAX_CLUE, stdin);
    t.clue[strcspn(t.clue, "\n")] = 0;
    printf("Value: "); scanf("%d", &t.value);

    write(fd, &t, sizeof(Treasure));
    close(fd);

    char log_msg[128];
    snprintf(log_msg, sizeof(log_msg), "Added treasure ID %d", t.id);
    log_action(hunt_id, log_msg);
}

void list_treasures(const char *hunt_id) {
    char file_path[256];
    snprintf(file_path, sizeof(file_path), BASE_PATH"%s/treasures.dat", hunt_id);
    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return;
    }

    struct stat st;
    stat(file_path, &st);
    printf("Hunt: %s\nSize: %ld bytes\nLast modified: %s\n", hunt_id, st.st_size, ctime(&st.st_mtime));

    Treasure t;
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        printf("ID: %d, User: %s, Coords: (%.2f, %.2f), Value: %d\n",
               t.id, t.username, t.latitude, t.longitude, t.value);
    }

    close(fd);
    log_action(hunt_id, "Listed all treasures");
}

void view_treasure(const char *hunt_id, int id) {
    char file_path[256];
    snprintf(file_path, sizeof(file_path), BASE_PATH"%s/treasures.dat", hunt_id);
    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return;
    }

    Treasure t;
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        if (t.id == id) {
            printf("ID: %d\nUser: %s\nCoords: %.2f, %.2f\nClue: %s\nValue: %d\n",
                   t.id, t.username, t.latitude, t.longitude, t.clue, t.value);
            log_action(hunt_id, "Viewed a treasure");
            close(fd);
            return;
        }
    }

    printf("Treasure not found.\n");
    close(fd);
}

void remove_treasure(const char *hunt_id, int id) {
    char file_path[256], temp_path[256];
    snprintf(file_path, sizeof(file_path), BASE_PATH"%s/treasures.dat", hunt_id);
    snprintf(temp_path, sizeof(temp_path), BASE_PATH"%s/temp.dat", hunt_id);

    int fd = open(file_path, O_RDONLY);
    int temp_fd = open(temp_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0 || temp_fd < 0) {
        perror("open");
        return;
    }

    Treasure t;
    int found = 0;
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        if (t.id != id) {
            write(temp_fd, &t, sizeof(Treasure));
        } else {
            found = 1;
        }
    }

    close(fd);
    close(temp_fd);
    rename(temp_path, file_path);

    if (found) {
        log_action(hunt_id, "Removed a treasure");
    } else {
        printf("Treasure not found.\n");
    }
}

void remove_hunt(const char *hunt_id) {
    char dir_path[256];
    snprintf(dir_path, sizeof(dir_path), BASE_PATH"%s", hunt_id);

    char treasure_file[256];
    snprintf(treasure_file, sizeof(treasure_file), "%s/treasures.dat", dir_path);
    char log_file[256];
    snprintf(log_file, sizeof(log_file), "%s/logged_hunt", dir_path);

    unlink(treasure_file);
    unlink(log_file);
    rmdir(dir_path);

    char symlink_name[256];
    snprintf(symlink_name, sizeof(symlink_name), "logged_hunt-%s", hunt_id);
    unlink(symlink_name);

    printf("Hunt '%s' removed.\n", hunt_id);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s --<command> <hunt_id> [<id>]\n", argv[0]);
        return 1;
    }

    const char *cmd = argv[1];
    const char *hunt_id = argv[2];

    if (strcmp(cmd, "--add") == 0) {
        add_treasure(hunt_id);
    } else if (strcmp(cmd, "--list") == 0) {
        list_treasures(hunt_id);
    } else if (strcmp(cmd, "--view") == 0 && argc >= 4) {
        view_treasure(hunt_id, atoi(argv[3]));
    } else if (strcmp(cmd, "--remove_treasure") == 0 && argc >= 4) {
        remove_treasure(hunt_id, atoi(argv[3]));
    } else if (strcmp(cmd, "--remove") == 0) {
        remove_hunt(hunt_id);
    } else {
        fprintf(stderr, "Unknown or incomplete command.\n");
    }

    return 0;
}
