#ifndef TRETIS_STATS_H
#define TRETIS_STATS_H

typedef struct {
    int gamesPlayed;
    int highScore;
    int longestTime;
    int mostTetrises;
} tretis_stats_t;

tretis_stats_t loadTretisStats(const char* path);
void saveTretisStats(const char* path, tretis_stats_t stats);
void printTretisStats(const char* path);

#endif
