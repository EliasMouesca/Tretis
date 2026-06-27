#include <SDL3/SDL.h>
#include <stdlib.h>
#include <string.h>

#include "game/game.h"
#include "log/log.h"
#include "render_context/render_context.h"
#include "tretis_config.h"

static int readIntArg(int argc, char* argv[], int* i, int fallback) {
    if (*i + 1 >= argc)
        return fallback;

    (*i)++;
    return atoi(argv[*i]);
}

static tretis_config_t parseConfig(int argc, char* argv[]) {
    tretis_config_t config = defaultTretisConfig();

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--no-hud") == 0) {
            config.showHud = false;
        } else if (strcmp(argv[i], "--hud") == 0) {
            config.showHud = true;
        } else if (strcmp(argv[i], "--no-ghost") == 0) {
            config.showGhost = false;
        } else if (strcmp(argv[i], "--ghost") == 0) {
            config.showGhost = true;
        } else if (strcmp(argv[i], "--next") == 0) {
            config.nextPieces = readIntArg(argc, argv, &i, config.nextPieces);
        } else if (strcmp(argv[i], "--block-size") == 0) {
            config.blockSize = readIntArg(argc, argv, &i, config.blockSize);
        } else if (strcmp(argv[i], "--fall-delay") == 0) {
            config.fallDelay = readIntArg(argc, argv, &i, config.fallDelay);
        } else if (strcmp(argv[i], "--sidebar-width") == 0) {
            config.sidebarWidth = readIntArg(argc, argv, &i, config.sidebarWidth);
        } else {
            warn("Ignoring unknown option '%s'", argv[i]);
        }
    }

    if (config.nextPieces < 0)
        config.nextPieces = 0;
    if (config.nextPieces > MAX_NEXT_PIECES)
        config.nextPieces = MAX_NEXT_PIECES;
    if (config.blockSize < 8)
        config.blockSize = 8;
    if (config.fallDelay < 50)
        config.fallDelay = 50;
    if (config.sidebarWidth < 100)
        config.sidebarWidth = 100;

    return config;
}

int main(int argc, char* argv[]) {
    tretis_config_t config = parseConfig(argc, argv);
    render_context_t* rc = createRenderContext(config);
    game_t game;

    initGame(&game, config);

    while (isGameRunning(&game)) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT)
                handleGameKey(&game, SDLK_Q);

            if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat)
                handleGameKey(&game, event.key.key);
        }

        updateGame(&game, SDL_GetTicks());

        renderBegin(rc);
        drawGame(&game, rc);
        renderEnd(rc);

        SDL_Delay(16);
    }

    destroyRenderContext(&rc);

    return 0;
}
