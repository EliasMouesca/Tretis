#ifndef TRETIS_CONFIG_H
#define TRETIS_CONFIG_H

#include <stdbool.h>
#include <SDL3/SDL_keycode.h>

#define DEFAULT_BOARD_COLS 10
#define DEFAULT_BOARD_ROWS 20
#define DEFAULT_BLOCK_SIZE 30
#define DEFAULT_SIDEBAR_WIDTH 180
#define DEFAULT_NEXT_PIECES 4
#define MAX_NEXT_PIECES 6
#define STATS_PATH_MAX 256
#define FONT_PATH_MAX 256
#define CONFIG_PATH_MAX 256

typedef enum {
    RANDOMIZER_7_BAG,
    RANDOMIZER_UNIFORM,
    RANDOMIZER_TGM,
    RANDOMIZER_35_BAG
} randomizer_t;

typedef struct {
    int rows;
    int cols;
    int blockSize;
    int sidebarWidth;
    int nextPieces;
    int fallDelay;
    int minFallDelay;
    int speedupEvery;
    int speedupStep;
    int fontSize;
    randomizer_t randomizer;
    bool speedup;
    bool showHud;
    bool showGhost;
    bool zenMode;
    SDL_Keycode keyLeft;
    SDL_Keycode keyRight;
    SDL_Keycode keyDown;
    SDL_Keycode keyRotate;
    SDL_Keycode keyDrop;
    SDL_Keycode keyHold;
    SDL_Keycode keyRestart;
    SDL_Keycode keyQuit;
    SDL_Keycode keyPause;
    char statsPath[STATS_PATH_MAX];
    char configPath[CONFIG_PATH_MAX];
    char fontPath[FONT_PATH_MAX];
} tretis_config_t;

tretis_config_t defaultTretisConfig();
tretis_config_t sensibleTretisConfig();
void loadTretisConfig(tretis_config_t* config, const char* path);
void saveTretisConfig(const tretis_config_t* config, const char* path);
SDL_Keycode parseKeyName(const char* value, SDL_Keycode fallback);
const char* keyName(SDL_Keycode key);

#endif
