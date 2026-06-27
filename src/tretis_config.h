#ifndef TRETIS_CONFIG_H
#define TRETIS_CONFIG_H

#include <stdbool.h>

#define DEFAULT_BOARD_COLS 10
#define DEFAULT_BOARD_ROWS 20
#define DEFAULT_BLOCK_SIZE 30
#define DEFAULT_SIDEBAR_WIDTH 180
#define DEFAULT_NEXT_PIECES 4
#define MAX_NEXT_PIECES 6

typedef struct {
    int rows;
    int cols;
    int blockSize;
    int sidebarWidth;
    int nextPieces;
    int fallDelay;
    bool showHud;
    bool showGhost;
} tretis_config_t;

tretis_config_t defaultTretisConfig();

#endif
