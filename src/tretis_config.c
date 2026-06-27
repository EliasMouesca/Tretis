#include "tretis_config.h"

tretis_config_t defaultTretisConfig() {
    return (tretis_config_t){
        .rows = DEFAULT_BOARD_ROWS,
        .cols = DEFAULT_BOARD_COLS,
        .blockSize = DEFAULT_BLOCK_SIZE,
        .sidebarWidth = DEFAULT_SIDEBAR_WIDTH,
        .nextPieces = DEFAULT_NEXT_PIECES,
        .fallDelay = 500,
        .showHud = true,
        .showGhost = true
    };
}
