#include "../tretis_stats.h"

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

static void testMissingStatsAreZero() {
    tretis_stats_t stats = loadTretisStats("/tmp/tretis_missing_stats_file");

    assert(stats.gamesPlayed == 0);
    assert(stats.highScore == 0);
    assert(stats.longestTime == 0);
    assert(stats.mostTetrises == 0);
}

static void testStatsRoundTrip() {
    char path[128];
    snprintf(path, sizeof(path), "/tmp/tretis_stats_test_%ld/stats", (long)getpid());

    tretis_stats_t saved = {
        .gamesPlayed = 5,
        .highScore = 1200,
        .longestTime = 345,
        .mostTetrises = 3
    };

    saveTretisStats(path, saved);
    tretis_stats_t loaded = loadTretisStats(path);

    assert(loaded.gamesPlayed == saved.gamesPlayed);
    assert(loaded.highScore == saved.highScore);
    assert(loaded.longestTime == saved.longestTime);
    assert(loaded.mostTetrises == saved.mostTetrises);
}

int main() {
    testMissingStatsAreZero();
    testStatsRoundTrip();
    return 0;
}
