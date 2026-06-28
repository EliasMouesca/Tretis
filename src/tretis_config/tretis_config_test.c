#include "../tretis_config.h"

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

static void testKeyNamesRoundTrip() {
    assert(parseKeyName("left", 0) == SDLK_LEFT);
    assert(parseKeyName("right", 0) == SDLK_RIGHT);
    assert(parseKeyName("down", 0) == SDLK_DOWN);
    assert(parseKeyName("up", 0) == SDLK_UP);
    assert(parseKeyName("space", 0) == SDLK_SPACE);
    assert(parseKeyName("escape", 0) == SDLK_ESCAPE);
    assert(parseKeyName("esc", 0) == SDLK_ESCAPE);
    assert(parseKeyName("z", 0) == SDLK_Z);
    assert(parseKeyName("unknown", SDLK_Q) == SDLK_Q);

    assert(keyName(SDLK_LEFT)[0] == 'l');
    assert(keyName(SDLK_Z)[0] == 'z');
}

static void testConfigSaveLoadRoundTrip() {
    char path[128];
    snprintf(path, sizeof(path), "/tmp/tretis_config_test_%ld/config", (long)getpid());

    tretis_config_t saved = defaultTretisConfig();
    saved.showHud = false;
    saved.showGhost = false;
    saved.zenMode = true;
    saved.blockSize = 23;
    saved.fallDelay = 321;
    saved.softFallDelay = 27;
    saved.speedup = false;
    saved.speedupEvery = 12;
    saved.speedupStep = 4;
    saved.minFallDelay = 111;
    saved.nextPieces = 2;
    saved.sidebarWidth = 140;
    saved.fontSize = 17;
    saved.keyLeft = SDLK_A;
    saved.keyRight = SDLK_D;
    saved.keyDown = SDLK_S;
    saved.keyRotate = SDLK_W;
    saved.keyDrop = SDLK_SPACE;
    saved.keyHold = SDLK_E;
    saved.keyRestart = SDLK_R;
    saved.keyQuit = SDLK_Q;
    saved.keyPause = SDLK_P;
    snprintf(saved.fontPath, sizeof(saved.fontPath), "/tmp/font.ttf");
    snprintf(saved.statsPath, sizeof(saved.statsPath), "/tmp/stats");

    saveTretisConfig(&saved, path);

    tretis_config_t loaded = defaultTretisConfig();
    loadTretisConfig(&loaded, path);

    assert(loaded.showHud == saved.showHud);
    assert(loaded.showGhost == saved.showGhost);
    assert(loaded.zenMode == saved.zenMode);
    assert(loaded.blockSize == saved.blockSize);
    assert(loaded.fallDelay == saved.fallDelay);
    assert(loaded.softFallDelay == saved.softFallDelay);
    assert(loaded.speedup == saved.speedup);
    assert(loaded.speedupEvery == saved.speedupEvery);
    assert(loaded.speedupStep == saved.speedupStep);
    assert(loaded.minFallDelay == saved.minFallDelay);
    assert(loaded.nextPieces == saved.nextPieces);
    assert(loaded.sidebarWidth == saved.sidebarWidth);
    assert(loaded.fontSize == saved.fontSize);
    assert(loaded.keyLeft == saved.keyLeft);
    assert(loaded.keyRight == saved.keyRight);
    assert(loaded.keyDown == saved.keyDown);
    assert(loaded.keyRotate == saved.keyRotate);
    assert(loaded.keyDrop == saved.keyDrop);
    assert(loaded.keyHold == saved.keyHold);
    assert(loaded.keyRestart == saved.keyRestart);
    assert(loaded.keyQuit == saved.keyQuit);
    assert(loaded.keyPause == saved.keyPause);
}

int main() {
    testKeyNamesRoundTrip();
    testConfigSaveLoadRoundTrip();
    return 0;
}
