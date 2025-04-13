#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>

#define MAX_USERNAME 50
#define MAX_CLUE 100
#define DATA_FILE "treasures.dat"
#define LOG_FILE "logged_hunt"
#define MAX_PATH 256

typedef struct {
    int treasure_id;
    char username[MAX_USERNAME];
    double latitude;
    double longitude;
    char clue[MAX_CLUE];
    int value;
} Treasure;

void log_action(const char *hunt_dir, const char *action) {
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%s/%s", hunt_dir, LOG_FILE);

    FILE *f = fopen(path, "a");
    if (!f) return;

    time_t now = time(NULL);
    fprintf(f, "[%s] %s\n", ctime(&now), action);
    fclose(f);

    // créer symlink s’il n’existe pas
    char symlink_name[MAX_PATH];
    snprintf(symlink_name, sizeof(symlink_name), "logged_hunt-%s", hunt_dir);
    symlink(path, symlink_name);
}

void add_treasure(const char *hunt_id) {
    char dir[MAX_PATH];
    snprintf(dir, sizeof(dir), "%s", hunt_id);

    mkdir(dir, 0777); // crée le dossier s’il n’existe pas

    char file_path[MAX_PATH];
    snprintf(file_path, sizeof(file_path), "%s/%s", dir, DATA_FILE);

    Treasure t;
    printf("Treasure ID: "); scanf("%d", &t.treasure_id);
    printf("Username: "); scanf("%s", t.username);
    printf("Latitude: "); scanf("%lf", &t.latitude);
    printf("Longitude: "); scanf("%lf", &t.longitude);
    printf("Clue: "); scanf(" %[^\n]", t.clue);
    printf("Value: "); scanf("%d", &t.value);

    FILE *f = fopen(file_path, "ab");
    if (!f) {
        perror("Error writing file");
        return;
    }

    fwrite(&t, sizeof(Treasure), 1, f);
    fclose(f);

    char log_msg[200];
    snprintf(log_msg, sizeof(log_msg), "Added treasure ID %d", t.treasure_id);
    log_action(dir, log_msg);
}

void list_treasures(const char *hunt_id) {
    char file_path[MAX_PATH];
    snprintf(file_path, sizeof(file_path), "%s/%s", hunt_id, DATA_FILE);

    struct stat st;
    if (stat(file_path, &st) != 0) {
        perror("Error getting file info");
        return;
    }

    printf("File: %s, Size: %ld bytes, Last Modified: %ld\n", file_path, st.st_size, st.st_mtime);

    FILE *f = fopen(file_path, "rb");
    if (!f) {
        perror("Error reading file");
        return;
    }

    Treasure t;
    while (fread(&t, sizeof(Treasure), 1, f)) {
        printf("ID: %d, User: %s, Coords: (%.2f, %.2f), Clue: %s, Value: %d\n",
               t.treasure_id, t.username, t.latitude, t.longitude, t.clue, t.value);
    }

    fclose(f);
}

void view_treasure(const char *hunt_id, int target_id) {
    char file_path[MAX_PATH];
    snprintf(file_path, sizeof(file_path), "%s/%s", hunt_id, DATA_FILE);

    FILE *f = fopen(file_path, "rb");
    if (!f) {
        perror("Error reading file");
        return;
    }

    Treasure t;
    while (fread(&t, sizeof(Treasure), 1, f)) {
        if (t.treasure_id == target_id) {
            printf("Treasure ID: %d\nUser: %s\nLatitude: %.2f\nLongitude: %.2f\nClue: %s\nValue: %d\n",
                   t.treasure_id, t.username, t.latitude, t.longitude, t.clue, t.value);
            fclose(f);
            return;
        }
    }

    printf("Treasure ID %d not found.\n", target_id);
    fclose(f);
}

void remove_treasure(const char *hunt_id, int target_id) {
    char file_path[MAX_PATH];
    snprintf(file_path, sizeof(file_path), "%s/%s", hunt_id, DATA_FILE);

    FILE *f = fopen(file_path, "rb");
    if (!f) {
        perror("Error opening file");
        return;
    }

    FILE *temp = fopen("temp.dat", "wb");
    if (!temp) {
        perror("Error creating temp file");
        fclose(f);
        return;
    }

    Treasure t;
    int found = 0;

    while (fread(&t, sizeof(Treasure), 1, f)) {
        if (t.treasure_id == target_id) {
            found = 1;
            continue; // skip this one
        }
        fwrite(&t, sizeof(Treasure), 1, temp);
    }

    fclose(f);
    fclose(temp);

    remove(file_path);
    rename("temp.dat", file_path);

    if (found) {
        char log_msg[200];
        snprintf(log_msg, sizeof(log_msg), "Removed treasure ID %d", target_id);
        log_action(hunt_id, log_msg);
    } else {
        printf("Treasure ID %d not found.\n", target_id);
    }
}

void remove_hunt(const char *hunt_id) {
    char file_path[MAX_PATH];
    snprintf(file_path, sizeof(file_path), "%s/%s", hunt_id, DATA_FILE);
    remove(file_path);

    char log_path[MAX_PATH];
    snprintf(log_path, sizeof(log_path), "%s/%s", hunt_id, LOG_FILE);
    remove(log_path);

    rmdir(hunt_id);

    printf("Hunt %s removed.\n", hunt_id);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: ./treasure_manager <operation> <hunt_id> [<id>]\n");
        return 1;
    }

    char *operation = argv[1];
    char *hunt_id = argv[2];

    if (strcmp(operation, "add") == 0) {
        add_treasure(hunt_id);
    } else if (strcmp(operation, "list") == 0) {
        list_treasures(hunt_id);
    } else if (strcmp(operation, "view") == 0 && argc == 4) {
        view_treasure(hunt_id, atoi(argv[3]));
    } else if (strcmp(operation, "remove_treasure") == 0 && argc == 4) {
        remove_treasure(hunt_id, atoi(argv[3]));
    } else if (strcmp(operation, "remove_hunt") == 0) {
        remove_hunt(hunt_id);
    } else {
        printf("Invalid operation or missing arguments.\n");
    }

    return 0;
}
