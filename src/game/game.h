#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <stdint.h>

#include "../cell.h"
#include "../render_context/render_context.h"
#include "../tretis_config.h"
#include "../tretis_stats.h"

typedef struct {
    cell_color_t board[BOARD_ROWS][BOARD_COLS];
    int piece;
    int next[MAX_NEXT_PIECES];
    int bag[35];
    int bagSize;
    int bagIndex;
    int history[4];
    int generatedPieces;
    int heldPiece;
    int rotation;
    int row;
    int col;
    bool hasHeldPiece;
    bool swappedHeldThisTurn;
    int lines;
    int tetrises;
    int score;
    int lockedPieces;
    uint64_t startedAt;
    uint64_t lastFall;
    tretis_stats_t stats;
    tretis_config_t config;
    bool running;
    bool gameOver;
    bool statsSaved;
} game_t;

void initGame(game_t* game, tretis_config_t config);
void handleGameKey(game_t* game, int key);
void updateGame(game_t* game, uint64_t now);
void drawGame(const game_t* game, render_context_t* rc);
bool isGameRunning(const game_t* game);
void finalizeGame(game_t* game);

#endif
