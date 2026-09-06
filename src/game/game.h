#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <stdint.h>
#include <SDL3/SDL_keycode.h>

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
    int tretises;
    int score;
    int lockedPieces;
    uint64_t startedAt;
    uint64_t lastFall;
    uint64_t nextMoveAt;
    uint64_t nextSoftFallAt;
    uint64_t lastTick;
    uint64_t elapsedTime;
    uint64_t groundedAt;
    uint64_t lastLockDelayedAt;
    tretis_stats_t stats;
    tretis_config_t config;
    bool running;
    bool paused;
    bool gameOver;
    bool statsSaved;
    bool movingLeft;
    bool movingRight;
    bool softDropping;
    bool grounded;
} game_snapshot_t;

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
    int tretises;
    int score;
    int lockedPieces;
    uint64_t startedAt;
    uint64_t lastFall;
    uint64_t nextMoveAt;
    uint64_t nextSoftFallAt;
    uint64_t lastTick;
    uint64_t elapsedTime;
    uint64_t groundedAt;            // When did the piece touch the ground?
    uint64_t lastLockDelayedAt;     // When was the last time a key was press and the lock got delayed?
    tretis_stats_t stats;
    tretis_config_t config;
    bool running;
    bool paused;
    bool gameOver;
    bool statsSaved;
    bool movingLeft;
    bool movingRight;
    bool softDropping;
    bool grounded; 
    game_snapshot_t undoHistory[MAX_UNDO_HISTORY];
    int undoCount;
    game_snapshot_t turnStart;
    bool hasTurnSnapshot;
} game_t;

void initGame(game_t* game, tretis_config_t config);
bool saveGameSnapshot(game_t* game, const char* path);
bool loadGameSnapshot(game_t* game, const char* path);
void handleGameKey(game_t* game, SDL_Keycode key);
void handleGameKeyWithMod(game_t* game, SDL_Keycode key, SDL_Keymod mod);
void releaseGameKey(game_t* game, SDL_Keycode key);
void updateGame(game_t* game, uint64_t now);
void drawGame(const game_t* game, render_context_t* rc);
bool isGameRunning(const game_t* game);
void finalizeGame(game_t* game);

#endif
