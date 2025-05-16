#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

typedef struct UserScore {
    char username[MAX_USERNAME];
    int score;
    struct UserScore *next;
} UserScore;

void add_score(UserScore **head, const char *user, int value) {
    UserScore *cur = *head;
    while (cur) {
        if (strcmp(cur->username, user) == 0) {
            cur->score += value;
            return;
        }
        cur = cur->next;
    }
    // Si utilisateur non trouvé, on l'ajoute
    UserScore *new_node = malloc(sizeof(UserScore));
    if (!new_node) {
        perror("malloc");
        exit(1);
    }
    strncpy(new_node->username, user, MAX_USERNAME);
    new_node->score = value;
    new_node->next = *head;
    *head = new_node;
}

void free_scores(UserScore *head) {
    while (head) {
        UserScore *tmp = head;
        head = head->next;
        free(tmp);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hunt_id>\n", argv[0]);
        return 1;
    }

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "./hunts/%s/treasures.dat", argv[1]);
    FILE *f = fopen(filepath, "rb");
    if (!f) {
        perror("fopen");
        return 1;
    }

    Treasure t;
    UserScore *scores = NULL;

    while (fread(&t, sizeof(Treasure), 1, f) == 1) {
        add_score(&scores, t.username, t.value);
    }

    fclose(f);

    printf("Scores for hunt %s:\n", argv[1]);
    UserScore *cur = scores;
    while (cur) {
        printf("User: %s, Score: %d\n", cur->username, cur->score);
        cur = cur->next;
    }

    free_scores(scores);
    return 0;
}
