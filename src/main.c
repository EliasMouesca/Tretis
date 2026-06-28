#include <SDL3/SDL.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "game/game.h"
#include "log/log.h"
#include "render_context/render_context.h"
#include "tretis_config.h"
#include "tretis_stats.h"

static int readIntArg(int argc, char* argv[], int* i, int fallback) {
    if (*i + 1 >= argc)
        return fallback;

    (*i)++;
    return atoi(argv[*i]);
}

static const char* readStringArg(int argc, char* argv[], int* i, const char* fallback) {
    if (*i + 1 >= argc)
        return fallback;

    (*i)++;
    return argv[*i];
}

static const char* randomizerName(randomizer_t randomizer) {
    switch (randomizer) {
        case RANDOMIZER_7_BAG: return "7bag";
        case RANDOMIZER_UNIFORM: return "uniform";
        case RANDOMIZER_TGM: return "tgm";
        case RANDOMIZER_35_BAG: return "35bag";
    }

    return "7bag";
}

static randomizer_t parseRandomizer(const char* value, randomizer_t fallback) {
    if (strcmp(value, "7bag") == 0 || strcmp(value, "bag") == 0)
        return RANDOMIZER_7_BAG;
    if (strcmp(value, "uniform") == 0 || strcmp(value, "random") == 0)
        return RANDOMIZER_UNIFORM;
    if (strcmp(value, "tgm") == 0 || strcmp(value, "history") == 0)
        return RANDOMIZER_TGM;
    if (strcmp(value, "35bag") == 0 || strcmp(value, "tgm3") == 0)
        return RANDOMIZER_35_BAG;

    warn("Ignoring unknown randomizer '%s'", value);
    return fallback;
}

static void printHelp(const char* program) {
    tretis_config_t defaults = defaultTretisConfig();

    printf("Usage: %s [options]\n", program);
    printf("\n");
    printf("Options:\n");
    printf("  --help                 Show this help and exit\n");
    printf("  --hud                  Show the right HUD panel (default)\n");
    printf("  --no-hud               Hide the right HUD panel\n");
    printf("  --zen                  Hide score/highscore details in the HUD\n");
    printf("  --no-zen               Show full HUD stats (default)\n");
    printf("  --ghost                Show the landing shadow (default)\n");
    printf("  --no-ghost             Hide the landing shadow\n");
    printf("  --next N               Number of next pieces to show, 0-%d (default %d)\n", MAX_NEXT_PIECES, DEFAULT_NEXT_PIECES);
    printf("  --block-size N         Pixel size for each board block (default %d)\n", DEFAULT_BLOCK_SIZE);
    printf("  --fall-delay N         Milliseconds between automatic falls (default 500)\n");
    printf("  --speedup              Slowly increase fall speed over time (default)\n");
    printf("  --no-speedup           Keep fall speed constant\n");
    printf("  --speedup-every N      Seconds between tiny speed increases (default 45)\n");
    printf("  --speedup-step N       Milliseconds removed each speed increase (default 15)\n");
    printf("  --min-fall-delay N     Fastest automatic fall delay in ms (default 180)\n");
    printf("  --randomizer NAME      Piece generator: 7bag, uniform, tgm, 35bag (default %s)\n",
            randomizerName(defaults.randomizer));
    printf("  --sidebar-width N      Width of the right HUD panel (default %d)\n", DEFAULT_SIDEBAR_WIDTH);
    printf("  --stats                Print saved stats and exit\n");
#ifdef _WIN32
    printf("  --stats-file PATH      File used to load and save stats (default %%APPDATA%%\\tretis\\stats)\n");
#else
    printf("  --stats-file PATH      File used to load and save stats (default $HOME/.config/tretis/stats)\n");
#endif
    printf("  --highscore-file PATH  Alias for --stats-file\n");
    printf("  --font PATH            TTF font file for HUD text\n");
    printf("  --font-size N          HUD font size in pixels (default 14)\n");
    printf("\n");
#ifdef _WIN32
    printf("Preferences are loaded from and saved to %%APPDATA%%\\tretis\\config.\n");
#else
    printf("Preferences are loaded from and saved to $HOME/.config/tretis/config.\n");
#endif
    printf("Key bindings can be edited there with key_left, key_right, key_down,\n");
    printf("key_rotate, key_drop, key_hold, key_restart, key_quit, and key_pause.\n");
    printf("\n");
    printf("Controls:\n");
    printf("  Left/Right or A/D      Move\n");
    printf("  Down or S              Soft drop\n");
    printf("  Up or W                Rotate\n");
    printf("  Space                  Hard drop\n");
    printf("  E                      Hold/swap current piece\n");
    printf("  R                      Restart\n");
    printf("  Q                      Quit\n");
}

static tretis_config_t parseConfig(int argc, char* argv[], bool* printStats) {
    tretis_config_t config = sensibleTretisConfig();
    *printStats = false;

    loadTretisConfig(&config, config.configPath);

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            printHelp(argv[0]);
            exit(0);
        } else if (strcmp(argv[i], "--stats") == 0) {
            *printStats = true;
        } else if (strcmp(argv[i], "--no-hud") == 0) {
            config.showHud = false;
        } else if (strcmp(argv[i], "--hud") == 0) {
            config.showHud = true;
        } else if (strcmp(argv[i], "--zen") == 0) {
            config.zenMode = true;
        } else if (strcmp(argv[i], "--no-zen") == 0) {
            config.zenMode = false;
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
        } else if (strcmp(argv[i], "--speedup") == 0) {
            config.speedup = true;
        } else if (strcmp(argv[i], "--no-speedup") == 0) {
            config.speedup = false;
        } else if (strcmp(argv[i], "--speedup-every") == 0) {
            config.speedupEvery = readIntArg(argc, argv, &i, config.speedupEvery);
        } else if (strcmp(argv[i], "--speedup-step") == 0) {
            config.speedupStep = readIntArg(argc, argv, &i, config.speedupStep);
        } else if (strcmp(argv[i], "--min-fall-delay") == 0) {
            config.minFallDelay = readIntArg(argc, argv, &i, config.minFallDelay);
        } else if (strcmp(argv[i], "--randomizer") == 0) {
            const char* value = readStringArg(argc, argv, &i, randomizerName(config.randomizer));
            config.randomizer = parseRandomizer(value, config.randomizer);
        } else if (strcmp(argv[i], "--sidebar-width") == 0) {
            config.sidebarWidth = readIntArg(argc, argv, &i, config.sidebarWidth);
        } else if (strcmp(argv[i], "--stats-file") == 0 || strcmp(argv[i], "--highscore-file") == 0) {
            const char* path = readStringArg(argc, argv, &i, config.statsPath);
            strncpy(config.statsPath, path, sizeof(config.statsPath) - 1);
            config.statsPath[sizeof(config.statsPath) - 1] = '\0';
        } else if (strcmp(argv[i], "--font") == 0) {
            const char* path = readStringArg(argc, argv, &i, config.fontPath);
            strncpy(config.fontPath, path, sizeof(config.fontPath) - 1);
            config.fontPath[sizeof(config.fontPath) - 1] = '\0';
        } else if (strcmp(argv[i], "--font-size") == 0) {
            config.fontSize = readIntArg(argc, argv, &i, config.fontSize);
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
    if (config.minFallDelay < 50)
        config.minFallDelay = 50;
    if (config.minFallDelay > config.fallDelay)
        config.minFallDelay = config.fallDelay;
    if (config.speedupEvery < 1)
        config.speedupEvery = 1;
    if (config.speedupStep < 0)
        config.speedupStep = 0;
    if (config.sidebarWidth < 100)
        config.sidebarWidth = 100;
    if (config.fontSize < 8)
        config.fontSize = 8;

    return config;
}

int main(int argc, char* argv[]) {
    bool printStats = false;
    tretis_config_t config = parseConfig(argc, argv, &printStats);

    if (printStats) {
        printTretisStats(config.statsPath);
        return 0;
    }

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

    finalizeGame(&game);
    saveTretisConfig(&config, config.configPath);
    destroyRenderContext(&rc);

    return 0;
}
