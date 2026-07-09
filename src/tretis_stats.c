#include "tretis_stats.h"

#include <stdio.h>
#include <string.h>

#include "platform/platform.h"

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
        else if (strcmp(key, "most_tretises") == 0)
            stats.mostTretises = value;
    }

    fclose(file);
    return stats;
}

void saveTretisStats(const char* path, tretis_stats_t stats) {
    ensureParentDir(path);

    FILE* file = fopen(path, "w");

    if (!file)
        return;

    fprintf(file, "games_played %d\n", stats.gamesPlayed);
    fprintf(file, "high_score %d\n", stats.highScore);
    fprintf(file, "longest_time %d\n", stats.longestTime);
    fprintf(file, "most_tretises %d\n", stats.mostTretises);
    fclose(file);
}

void printTretisStats(const char* path) {
    tretis_stats_t stats = loadTretisStats(path);

    printf("Stats from %s\n", path);
    printf("  Games played: %d\n", stats.gamesPlayed);
    printf("  High score: %d\n", stats.highScore);
    printf("  Longest game: %02d:%02d\n", stats.longestTime / 60, stats.longestTime % 60);
    printf("  Most tretises in one game: %d\n", stats.mostTretises);
}
