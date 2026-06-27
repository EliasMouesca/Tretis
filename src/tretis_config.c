#include "tretis_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

tretis_config_t defaultTretisConfig() {
    tretis_config_t config = {
        .rows = DEFAULT_BOARD_ROWS,
        .cols = DEFAULT_BOARD_COLS,
        .blockSize = DEFAULT_BLOCK_SIZE,
        .sidebarWidth = DEFAULT_SIDEBAR_WIDTH,
        .nextPieces = DEFAULT_NEXT_PIECES,
        .fallDelay = 500,
        .fontSize = 14,
        .randomizer = RANDOMIZER_7_BAG,
        .showHud = true,
        .showGhost = true
    };

    const char* home = getenv("HOME");
    if (home != NULL && home[0] != '\0')
        snprintf(config.statsPath, sizeof(config.statsPath), "%s/.config/tretis/stats", home);
    else
        strncpy(config.statsPath, ".tretis_stats", sizeof(config.statsPath) - 1);

    strncpy(config.fontPath, "./fonts/SpaceMono-Regular.ttf", sizeof(config.fontPath) - 1);

    return config;
}
