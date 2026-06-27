#include "tretis_stats.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static void ensureRuntimeDir(const char* path) {
    char buffer[512];
    char* slash;

    if (strlen(path) >= sizeof(buffer))
        return;

    strcpy(buffer, path);
    slash = strrchr(buffer, '/');

    if (!slash)
        return;

    *slash = '\0';

    for (char* p = buffer + 1; *p; p++) {
        if (*p != '/')
            continue;

        *p = '\0';
        if (mkdir(buffer, 0700) != 0 && errno != EEXIST)
            return;
        *p = '/';
    }

    mkdir(buffer, 0700);
}

tretis_stats_t loadTretisStats(const char* path) {
    tretis_stats_t stats = {0};
    FILE* file = fopen(path, "r");

    if (!file)
        return stats;

    char key[64];
    int value;

    while (fscanf(file, "%63s %d", key, &value) == 2) {
        if (strcmp(key, "games_played") == 0)
            stats.gamesPlayed = value;
        else if (strcmp(key, "high_score") == 0)
            stats.highScore = value;
        else if (strcmp(key, "longest_time") == 0)
            stats.longestTime = value;
        else if (strcmp(key, "most_tetrises") == 0)
            stats.mostTetrises = value;
    }

    fclose(file);
    return stats;
}

void saveTretisStats(const char* path, tretis_stats_t stats) {
    ensureRuntimeDir(path);

    FILE* file = fopen(path, "w");

    if (!file)
        return;

    fprintf(file, "games_played %d\n", stats.gamesPlayed);
    fprintf(file, "high_score %d\n", stats.highScore);
    fprintf(file, "longest_time %d\n", stats.longestTime);
    fprintf(file, "most_tetrises %d\n", stats.mostTetrises);
    fclose(file);
}

void printTretisStats(const char* path) {
    tretis_stats_t stats = loadTretisStats(path);

    printf("Stats from %s\n", path);
    printf("  Games played: %d\n", stats.gamesPlayed);
    printf("  High score: %d\n", stats.highScore);
    printf("  Longest game: %02d:%02d\n", stats.longestTime / 60, stats.longestTime % 60);
    printf("  Most tetrises in one game: %d\n", stats.mostTetrises);
}
