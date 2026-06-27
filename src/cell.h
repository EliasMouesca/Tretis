#ifndef CELL_H
#define CELL_H

#include "tretis_config.h"

#define BOARD_COLS DEFAULT_BOARD_COLS
#define BOARD_ROWS DEFAULT_BOARD_ROWS

typedef enum {
    CELL_EMPTY = 0,
    CELL_CYAN,
    CELL_BLUE,
    CELL_ORANGE,
    CELL_YELLOW,
    CELL_GREEN,
    CELL_PURPLE,
    CELL_RED
} cell_color_t;

#endif
