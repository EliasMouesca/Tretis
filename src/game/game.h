#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <stdint.h>

#include "../cell.h"
#include "../render_context/render_context.h"
#include "../tretis_config.h"

typedef struct {
    cell_color_t board[BOARD_ROWS][BOARD_COLS];
    int piece;
    int next[MAX_NEXT_PIECES];
    int rotation;
    int row;
    int col;
    int lines;
    int score;
    int lockedPieces;
    uint64_t startedAt;
    uint64_t lastFall;
    tretis_config_t config;
    bool running;
    bool gameOver;
} game_t;

void initGame(game_t* game, tretis_config_t config);
void handleGameKey(game_t* game, int key);
void updateGame(game_t* game, uint64_t now);
void drawGame(const game_t* game, render_context_t* rc);
bool isGameRunning(const game_t* game);

#endif
