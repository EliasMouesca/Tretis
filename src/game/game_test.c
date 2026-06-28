#include "game.h"

#include <assert.h>
#include <stdio.h>
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

int main() {
    testInitGameSetsPlayableDefaults();
    testElapsedTimeOnlyAccumulatesWhileActive();
    testMoveKeysSetAndReleaseHeldState();
    return 0;
}
