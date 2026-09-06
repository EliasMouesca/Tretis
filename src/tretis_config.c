#include "tretis_config.h"

#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform/platform.h"

SDL_Keycode parseKeyName(const char* value, SDL_Keycode fallback) {
    if (strcmp(value, "left") == 0) return SDLK_LEFT;
    if (strcmp(value, "right") == 0) return SDLK_RIGHT;
    if (strcmp(value, "down") == 0) return SDLK_DOWN;
    if (strcmp(value, "up") == 0) return SDLK_UP;
    if (strcmp(value, "space") == 0) return SDLK_SPACE;
    if (strcmp(value, "escape") == 0 || strcmp(value, "esc") == 0) return SDLK_ESCAPE;
    if (strlen(value) == 1) return (SDL_Keycode)value[0];

    return fallback;
}

static SDL_Keymod parseShortcutModifier(const char* value) {
    if (strcmp(value, "ctrl") == 0 || strcmp(value, "control") == 0)
        return SDL_KMOD_CTRL;
    if (strcmp(value, "shift") == 0)
        return SDL_KMOD_SHIFT;
    if (strcmp(value, "alt") == 0)
        return SDL_KMOD_ALT;
    if (strcmp(value, "gui") == 0 || strcmp(value, "meta") == 0 || strcmp(value, "cmd") == 0)
        return SDL_KMOD_GUI;

    return SDL_KMOD_NONE;
}

static bool isShortcutKeyName(const char* value) {
    return strlen(value) == 1 ||
        strcmp(value, "left") == 0 || strcmp(value, "right") == 0 ||
        strcmp(value, "down") == 0 || strcmp(value, "up") == 0 ||
        strcmp(value, "space") == 0 || strcmp(value, "escape") == 0 ||
        strcmp(value, "esc") == 0;
}

SDL_Keycode parseShortcut(const char* value, SDL_Keycode fallbackKey,
        SDL_Keymod fallbackMod, SDL_Keymod* mod) {
    char buffer[256];
    char* token;
    SDL_Keymod parsedMod = SDL_KMOD_NONE;
    bool hasModifier = false;

    if (strlen(value) >= sizeof(buffer)) {
        *mod = fallbackMod;
        return fallbackKey;
    }

    strcpy(buffer, value);
    token = strtok(buffer, "+");

    while (token != NULL) {
        SDL_Keymod shortcutMod = parseShortcutModifier(token);

        if (shortcutMod != SDL_KMOD_NONE) {
            parsedMod |= shortcutMod;
            hasModifier = true;
            token = strtok(NULL, "+");
            continue;
        }

        if (strtok(NULL, "+") != NULL) {
            *mod = fallbackMod;
            return fallbackKey;
        }

        if (!isShortcutKeyName(token)) {
            *mod = fallbackMod;
            return fallbackKey;
        }

        *mod = hasModifier ? parsedMod : SDL_KMOD_NONE;
        return parseKeyName(token, fallbackKey);
    }

    *mod = fallbackMod;
    return fallbackKey;
}

const char* keyName(SDL_Keycode key) {
    switch (key) {
        case SDLK_LEFT: return "left";
        case SDLK_RIGHT: return "right";
        case SDLK_DOWN: return "down";
        case SDLK_UP: return "up";
        case SDLK_SPACE: return "space";
        case SDLK_ESCAPE: return "escape";
        default: break;
    }

    static char buffer[2] = {0};
    if (key >= 32 && key <= 126) {
        buffer[0] = (char)key;
        return buffer;
    }

    return "?";
}

const char* shortcutName(SDL_Keycode key, SDL_Keymod mod) {
    static char buffer[64];
    snprintf(buffer, sizeof(buffer), "%s%s%s%s%s",
            mod & SDL_KMOD_CTRL ? "ctrl+" : "",
            mod & SDL_KMOD_SHIFT ? "shift+" : "",
            mod & SDL_KMOD_ALT ? "alt+" : "",
            mod & SDL_KMOD_GUI ? "gui+" : "",
            keyName(key));
    return buffer;
}

tretis_config_t defaultTretisConfig() {
    tretis_config_t config = {
        .rows = DEFAULT_BOARD_ROWS,
        .cols = DEFAULT_BOARD_COLS,
        .blockSize = DEFAULT_BLOCK_SIZE,
        .sidebarWidth = DEFAULT_SIDEBAR_WIDTH,
        .nextPieces = DEFAULT_NEXT_PIECES,
        .undoLimit = DEFAULT_UNDO_LIMIT,
        .fallDelay = 500,
        .moveRepeatDelay = 80,
        .moveRepeatInitialDelay = 180,
        .lockDelay = 0,
        .maxLockDelay = 0,
        .softFallDelay = 45,
        .minFallDelay = 180,
        .speedupEvery = 45,
        .speedupStep = 15,
        .fontSize = 14,
        .randomizer = RANDOMIZER_7_BAG,
        .speedup = true,
        .showHud = true,
        .showGhost = true,
        .zenMode = false,
        .resumePaused = false,
        .keyLeft = SDLK_LEFT,
        .keyRight = SDLK_RIGHT,
        .keyDown = SDLK_DOWN,
        .keyRotate = SDLK_UP,
        .keyRotateLeft = SDLK_Z,
        .keyRotateRight = SDLK_X,
        .keyDrop = SDLK_SPACE,
        .keyHold = SDLK_E,
        .keyRestart = SDLK_R,
        .keyQuit = SDLK_Q,
        .keyPause = SDLK_P,
        .keyUndo = SDLK_Z,
        .keyUndoMod = SDL_KMOD_CTRL
    };

    // Sets default runtime paths in config.{statsPath; snapshotPath; configPath}.
    makeDefaultRuntimePath(config.statsPath, sizeof(config.statsPath), "stats");
    makeDefaultRuntimePath(config.snapshotPath, sizeof(config.snapshotPath), "snapshot");
    makeDefaultRuntimePath(config.configPath, sizeof(config.configPath), "config");
    snprintf(config.fontPath, sizeof(config.fontPath), "./fonts/SpaceMono-Regular.ttf");

    return config;
}

tretis_config_t sensibleTretisConfig() {
    tretis_config_t config = defaultTretisConfig();

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
        return config;

    SDL_DisplayID display = SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(display);

    if (mode != NULL && mode->w > 0 && mode->h > 0) {
        int targetHeight = mode->h * 8 / 10;
        int blockSize = targetHeight / config.rows;
        int maxBoardWidth = mode->w * 8 / 10 - config.sidebarWidth;
        int maxBlockByWidth = maxBoardWidth / config.cols;

        if (maxBlockByWidth > 0 && blockSize > maxBlockByWidth)
            blockSize = maxBlockByWidth;
        if (blockSize > DEFAULT_BLOCK_SIZE)
            blockSize = DEFAULT_BLOCK_SIZE;
        if (blockSize < 8)
            blockSize = 8;

        config.blockSize = blockSize;
        config.sidebarWidth = mode->w / 5;
        if (config.sidebarWidth < DEFAULT_SIDEBAR_WIDTH)
            config.sidebarWidth = DEFAULT_SIDEBAR_WIDTH;
        if (config.sidebarWidth > 260)
            config.sidebarWidth = 260;
    }

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    return config;
}

void loadTretisConfig(tretis_config_t* config, const char* path) {
    FILE* file = fopen(path, "r");
    char key[64];
    char value[256];

    if (!file)
        return;

    while (fscanf(file, "%63s %255s", key, value) == 2) {
        if (strcmp(key, "show_hud") == 0) config->showHud = atoi(value) != 0;
        else if (strcmp(key, "show_ghost") == 0) config->showGhost = atoi(value) != 0;
        else if (strcmp(key, "zen") == 0) config->zenMode = atoi(value) != 0;
        else if (strcmp(key, "resume_paused") == 0) config->resumePaused = atoi(value) != 0;
        else if (strcmp(key, "block_size") == 0) config->blockSize = atoi(value);
        else if (strcmp(key, "fall_delay") == 0) config->fallDelay = atoi(value);
        else if (strcmp(key, "move_repeat_delay") == 0) config->moveRepeatDelay = atoi(value);
        else if (strcmp(key, "move_repeat_initial_delay") == 0) config->moveRepeatInitialDelay = atoi(value);
        else if (strcmp(key, "soft_fall_delay") == 0) config->softFallDelay = atoi(value);
        else if (strcmp(key, "speedup") == 0) config->speedup = atoi(value) != 0;
        else if (strcmp(key, "speedup_every") == 0) config->speedupEvery = atoi(value);
        else if (strcmp(key, "speedup_step") == 0) config->speedupStep = atoi(value);
        else if (strcmp(key, "min_fall_delay") == 0) config->minFallDelay = atoi(value);
        else if (strcmp(key, "next_pieces") == 0) config->nextPieces = atoi(value);
        else if (strcmp(key, "undo_limit") == 0) config->undoLimit = atoi(value);
        else if (strcmp(key, "sidebar_width") == 0) config->sidebarWidth = atoi(value);
        else if (strcmp(key, "font_size") == 0) config->fontSize = atoi(value);
        else if (strcmp(key, "font") == 0) snprintf(config->fontPath, sizeof(config->fontPath), "%s", value);
        else if (strcmp(key, "stats_file") == 0) snprintf(config->statsPath, sizeof(config->statsPath), "%s", value);
        else if (strcmp(key, "snapshot_file") == 0) snprintf(config->snapshotPath, sizeof(config->snapshotPath), "%s", value);
        else if (strcmp(key, "key_left") == 0) config->keyLeft = parseKeyName(value, config->keyLeft);
        else if (strcmp(key, "key_right") == 0) config->keyRight = parseKeyName(value, config->keyRight);
        else if (strcmp(key, "key_down") == 0) config->keyDown = parseKeyName(value, config->keyDown);
        else if (strcmp(key, "key_rotate") == 0) config->keyRotate = parseKeyName(value, config->keyRotate);
        else if (strcmp(key, "key_rotate_left") == 0) config->keyRotateLeft = parseKeyName(value, config->keyRotateLeft);
        else if (strcmp(key, "key_rotate_right") == 0) config->keyRotateRight = parseKeyName(value, config->keyRotateRight);
        else if (strcmp(key, "key_drop") == 0) config->keyDrop = parseKeyName(value, config->keyDrop);
        else if (strcmp(key, "key_hold") == 0) config->keyHold = parseKeyName(value, config->keyHold);
        else if (strcmp(key, "key_restart") == 0) config->keyRestart = parseKeyName(value, config->keyRestart);
        else if (strcmp(key, "key_quit") == 0) config->keyQuit = parseKeyName(value, config->keyQuit);
        else if (strcmp(key, "key_pause") == 0) config->keyPause = parseKeyName(value, config->keyPause);
        else if (strcmp(key, "key_undo") == 0)
            config->keyUndo = parseShortcut(value, config->keyUndo, config->keyUndoMod, &config->keyUndoMod);
        else if (strcmp(key, "lock_delay") == 0) config->lockDelay = atoi(value);
        else if (strcmp(key, "max_lock_delay") == 0) config->maxLockDelay = atoi(value);
    }

    fclose(file);
}

void saveTretisConfig(const tretis_config_t* config, const char* path) {
    ensureParentDir(path);

    FILE* file = fopen(path, "w");
    if (!file)
        return;

    fprintf(file, "show_hud %d\n", config->showHud);
    fprintf(file, "show_ghost %d\n", config->showGhost);
    fprintf(file, "zen %d\n", config->zenMode);
    fprintf(file, "resume_paused %d\n", config->resumePaused);
    fprintf(file, "block_size %d\n", config->blockSize);
    fprintf(file, "fall_delay %d\n", config->fallDelay);
    fprintf(file, "move_repeat_delay %d\n", config->moveRepeatDelay);
    fprintf(file, "move_repeat_initial_delay %d\n", config->moveRepeatInitialDelay);
    fprintf(file, "soft_fall_delay %d\n", config->softFallDelay);
    fprintf(file, "speedup %d\n", config->speedup);
    fprintf(file, "speedup_every %d\n", config->speedupEvery);
    fprintf(file, "speedup_step %d\n", config->speedupStep);
    fprintf(file, "min_fall_delay %d\n", config->minFallDelay);
    fprintf(file, "next_pieces %d\n", config->nextPieces);
    fprintf(file, "undo_limit %d\n", config->undoLimit);
    fprintf(file, "sidebar_width %d\n", config->sidebarWidth);
    fprintf(file, "font_size %d\n", config->fontSize);
    fprintf(file, "font %s\n", config->fontPath);
    fprintf(file, "stats_file %s\n", config->statsPath);
    fprintf(file, "snapshot_file %s\n", config->snapshotPath);
    fprintf(file, "key_left %s\n", keyName(config->keyLeft));
    fprintf(file, "key_right %s\n", keyName(config->keyRight));
    fprintf(file, "key_down %s\n", keyName(config->keyDown));
    fprintf(file, "key_rotate %s\n", keyName(config->keyRotate));
    fprintf(file, "key_rotate_left %s\n", keyName(config->keyRotateLeft));
    fprintf(file, "key_rotate_right %s\n", keyName(config->keyRotateRight));
    fprintf(file, "key_drop %s\n", keyName(config->keyDrop));
    fprintf(file, "key_hold %s\n", keyName(config->keyHold));
    fprintf(file, "key_restart %s\n", keyName(config->keyRestart));
    fprintf(file, "key_quit %s\n", keyName(config->keyQuit));
    fprintf(file, "key_pause %s\n", keyName(config->keyPause));
    fprintf(file, "key_undo %s\n", shortcutName(config->keyUndo, config->keyUndoMod));
    fprintf(file, "lock_delay %d\n", config->lockDelay);
    fprintf(file, "max_lock_delay %d\n", config->maxLockDelay);
    fclose(file);
}
