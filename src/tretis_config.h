#ifndef TRETIS_CONFIG_H
#define TRETIS_CONFIG_H

#include <stdbool.h>

#define DEFAULT_BOARD_COLS 10
#define DEFAULT_BOARD_ROWS 20
#define DEFAULT_BLOCK_SIZE 30
#define DEFAULT_SIDEBAR_WIDTH 180
#define DEFAULT_NEXT_PIECES 4
#define MAX_NEXT_PIECES 6
#define STATS_PATH_MAX 256
#define FONT_PATH_MAX 256

typedef enum {
    RANDOMIZER_7_BAG,
    RANDOMIZER_UNIFORM,
    RANDOMIZER_TGM,
    RANDOMIZER_35_BAG
} randomizer_t;

typedef struct {
    int rows;
    int cols;
    int blockSize;
    int sidebarWidth;
    int nextPieces;
    int fallDelay;
    int fontSize;
    randomizer_t randomizer;
    bool showHud;
    bool showGhost;
    char statsPath[STATS_PATH_MAX];
    char fontPath[FONT_PATH_MAX];
} tretis_config_t;

tretis_config_t defaultTretisConfig();

#endif
