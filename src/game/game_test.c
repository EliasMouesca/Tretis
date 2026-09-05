#include "game.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static tretis_config_t testConfig() {
    tretis_config_t config = defaultTretisConfig();

    snprintf(config.statsPath, sizeof(config.statsPath),
            "/tmp/tretis_game_test_%ld/stats", (long)getpid());
    config.fallDelay = 100000;
    config.speedup = false;
    config.showHud = false;

    return config;
}

static void testInitGameSetsPlayableDefaults() {
    game_t game;

    initGame(&game, testConfig());

    assert(game.running);
    assert(!game.paused);
    assert(!game.gameOver);
    assert(game.heldPiece == -1);
    assert(game.piece >= 0 && game.piece < 7);
    assert(game.col == 3);
    assert(game.row == 0);
    assert(game.config.showHud == false);
}

static void testElapsedTimeOnlyAccumulatesWhileActive() {
    game_t game;

    initGame(&game, testConfig());
    game.lastTick = 100;
    game.elapsedTime = 0;

    updateGame(&game, 250);
    assert(game.elapsedTime == 150);

    game.paused = true;
    updateGame(&game, 500);
    assert(game.elapsedTime == 150);

    game.paused = false;
    game.gameOver = true;
    updateGame(&game, 700);
    assert(game.elapsedTime == 150);
}

static void testMoveKeysSetAndReleaseHeldState() {
    game_t game;

    initGame(&game, testConfig());

    int startCol = game.col;
    handleGameKey(&game, game.config.keyRight);
    assert(game.col == startCol + 1);
    assert(game.movingRight);
    assert(!game.movingLeft);

    releaseGameKey(&game, game.config.keyRight);
    assert(!game.movingRight);

    handleGameKey(&game, game.config.keyDown);
    assert(game.softDropping);
    releaseGameKey(&game, game.config.keyDown);
    assert(!game.softDropping);
}

static void testZXCControlAliases() {
    game_t game;

    initGame(&game, testConfig());

    handleGameKey(&game, SDLK_Z);
    assert(game.rotation == 3);

    handleGameKey(&game, SDLK_X);
    assert(game.rotation == 0);

    int firstPiece = game.piece;
    handleGameKey(&game, SDLK_C);
    assert(game.hasHeldPiece);
    assert(game.heldPiece == firstPiece);
    assert(game.swappedHeldThisTurn);
}

static void testUndoRemovesLastLockAndKeepsCurrentTurn() {
    game_t game;
    cell_color_t board[BOARD_ROWS][BOARD_COLS];
    int next[MAX_NEXT_PIECES];
    int currentPiece;
    int col;
    int bagIndex;
    int lockedPieces;

    initGame(&game, testConfig());
    game.config.undoLimit = 5;
    memcpy(board, game.board, sizeof(board));
    memcpy(next, game.next, sizeof(next));
    col = game.col;
    bagIndex = game.bagIndex;
    lockedPieces = game.lockedPieces;

    handleGameKey(&game, game.config.keyDrop);
    assert(game.undoCount == 1);
    assert(game.lockedPieces == lockedPieces + 1);
    currentPiece = game.piece;
    memcpy(next, game.next, sizeof(next));
    bagIndex = game.bagIndex;

    handleGameKeyWithMod(&game, SDLK_Z, SDL_KMOD_LCTRL);

    assert(game.undoCount == 0);
    assert(memcmp(game.board, board, sizeof(board)) == 0);
    assert(memcmp(game.next, next, sizeof(next)) == 0);
    assert(game.piece == currentPiece);
    assert(game.row == 0);
    assert(game.col == col);
    assert(game.bagIndex == bagIndex);
    assert(game.lockedPieces == lockedPieces);
}

static void testUndoClearsHeldMovementState() {
    game_t game;

    initGame(&game, testConfig());
    handleGameKey(&game, game.config.keyRight);
    game.softDropping = true;
    handleGameKey(&game, game.config.keyDrop);
    assert(game.movingRight);

    handleGameKeyWithMod(&game, SDLK_Z, SDL_KMOD_LCTRL);

    assert(!game.movingLeft);
    assert(!game.movingRight);
    assert(!game.softDropping);
}

static void testUndoLimitAndUnavailableStates() {
    game_t game;

    initGame(&game, testConfig());
    game.config.undoLimit = 2;

    for (int i = 0; i < 3; i++) {
        handleGameKey(&game, game.config.keyDrop);
        memset(game.board, 0, sizeof(game.board));
        game.gameOver = false;
        game.running = true;
    }

    assert(game.undoCount == 2);
    game.paused = true;
    handleGameKeyWithMod(&game, SDLK_Z, SDL_KMOD_LCTRL);
    assert(game.undoCount == 2);

    game.paused = false;
    game.gameOver = true;
    handleGameKeyWithMod(&game, SDLK_Z, SDL_KMOD_RCTRL);
    assert(game.undoCount == 2);

    game.gameOver = false;
    handleGameKeyWithMod(&game, SDLK_Z, SDL_KMOD_LCTRL);
    handleGameKeyWithMod(&game, SDLK_Z, SDL_KMOD_LCTRL);
    handleGameKeyWithMod(&game, SDLK_Z, SDL_KMOD_LCTRL);
    assert(game.undoCount == 0);
}

int main() {
    testInitGameSetsPlayableDefaults();
    testElapsedTimeOnlyAccumulatesWhileActive();
    testMoveKeysSetAndReleaseHeldState();
    testZXCControlAliases();
    testUndoRemovesLastLockAndKeepsCurrentTurn();
    testUndoClearsHeldMovementState();
    testUndoLimitAndUnavailableStates();
    return 0;
}
