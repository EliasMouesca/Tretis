
#include "render_context.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../log/log.h"

#define WINDOW_TITLE "TRETIS"
#define TEXT_CACHE_SIZE 64

typedef struct {
    char text[64];
    SDL_Texture* texture;
    int w;
    int h;
} text_cache_entry_t;

struct render_context_t {
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    tretis_config_t config;
    text_cache_entry_t textCache[TEXT_CACHE_SIZE];
    int textCacheNext;
};

static TTF_Font* openFont(tretis_config_t config) {
    char executableFont[FONT_PATH_MAX] = {0};
    const char* basePath = SDL_GetBasePath();

    if (basePath != NULL)
        snprintf(executableFont, sizeof(executableFont), "%sfonts/SpaceMono-Regular.ttf", basePath);

    const char* fallbacks[] = {
        config.fontPath,
        executableFont,
        "./fonts/SpaceMono-Regular.ttf",
        "/usr/share/tretis/fonts/SpaceMono-Regular.ttf",
        "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/liberation/LiberationMono-Regular.ttf"
    };

    for (size_t i = 0; i < sizeof(fallbacks) / sizeof(fallbacks[0]); i++) {
        if (fallbacks[i][0] == '\0')
            continue;

        TTF_Font* font = TTF_OpenFont(fallbacks[i], config.fontSize);
        if (font != NULL)
            return font;
    }

    critical("Could not open any configured or fallback TTF font");
    return NULL;
}

render_context_t* createRenderContext(tretis_config_t config) {
    if (!SDL_Init(SDL_INIT_VIDEO))
        critical("Could not initialize SDL: %s", SDL_GetError());

    if (!TTF_Init())
        critical("Could not initialize TTF: %s", SDL_GetError());

    render_context_t* rc = (render_context_t*) malloc(sizeof(render_context_t));
    if (rc == NULL)
        critical("Out of memory");

    rc->config = config;
    memset(rc->textCache, 0, sizeof(rc->textCache));
    rc->textCacheNext = 0;
    rc->font = openFont(config);

    int width = config.cols * config.blockSize;
    int height = config.rows * config.blockSize;
    if (config.showHud)
        width += config.sidebarWidth;

    rc->window = SDL_CreateWindow(WINDOW_TITLE, width, height, 0);
    if (rc->window == NULL) 
        critical("Could not initialize SDL window: %s", SDL_GetError());

    rc->renderer = SDL_CreateRenderer(rc->window, NULL);
    if (rc->renderer == NULL)
        critical("Could not initialize SDL renderer: %s", SDL_GetError());

    return rc;
}

void destroyRenderContext(render_context_t** rcp) {
    render_context_t* rc = *rcp;

    for (int i = 0; i < TEXT_CACHE_SIZE; i++)
        if (rc->textCache[i].texture != NULL)
            SDL_DestroyTexture(rc->textCache[i].texture);

    TTF_CloseFont(rc->font);
    SDL_DestroyRenderer(rc->renderer);
    SDL_DestroyWindow(rc->window);

    free(rc);
    *rcp = NULL;
    TTF_Quit();
    SDL_Quit();

    return;
}

void renderBegin(render_context_t* rc) {
    SDL_SetRenderDrawColor(rc->renderer, 0, 0, 0, 255);
    SDL_RenderClear(rc->renderer);

    if (rc->config.showHud) {
        SDL_FRect sidebar = {
            .x = (float)(rc->config.cols * rc->config.blockSize),
            .y = 0.0f,
            .w = (float)rc->config.sidebarWidth,
            .h = (float)(rc->config.rows * rc->config.blockSize)
        };

        SDL_SetRenderDrawColor(rc->renderer, 32, 32, 36, 255);
        SDL_RenderFillRect(rc->renderer, &sidebar);
    }
}

static void setCellColor(render_context_t* rc, cell_color_t color, Uint8 alpha) {
    switch (color) {
        case CELL_CYAN:   SDL_SetRenderDrawColor(rc->renderer, 67, 205, 230, alpha); break;
        case CELL_BLUE:   SDL_SetRenderDrawColor(rc->renderer, 55, 96, 214, alpha); break;
        case CELL_ORANGE: SDL_SetRenderDrawColor(rc->renderer, 235, 151, 45, alpha); break;
        case CELL_YELLOW: SDL_SetRenderDrawColor(rc->renderer, 235, 214, 65, alpha); break;
        case CELL_GREEN:  SDL_SetRenderDrawColor(rc->renderer, 90, 190, 85, alpha); break;
        case CELL_PURPLE: SDL_SetRenderDrawColor(rc->renderer, 165, 92, 210, alpha); break;
        case CELL_RED:    SDL_SetRenderDrawColor(rc->renderer, 220, 65, 75, alpha); break;
        case CELL_EMPTY:  SDL_SetRenderDrawColor(rc->renderer, 0, 0, 0, alpha); break;
    }
}

void renderCell(render_context_t* rc, int row, int col, cell_color_t color) {
    SDL_FRect rect = {
        .x = (float)(col * rc->config.blockSize),
        .y = (float)(row * rc->config.blockSize),
        .w = (float)rc->config.blockSize,
        .h = (float)rc->config.blockSize
    };

    setCellColor(rc, color, 255);
    SDL_SetRenderDrawBlendMode(rc->renderer, SDL_BLENDMODE_NONE);
    SDL_RenderFillRect(rc->renderer, &rect);
}

void renderGhostCell(render_context_t* rc, int row, int col, cell_color_t color) {
    SDL_FRect rect = {
        .x = (float)(col * rc->config.blockSize),
        .y = (float)(row * rc->config.blockSize),
        .w = (float)rc->config.blockSize,
        .h = (float)rc->config.blockSize
    };

    SDL_SetRenderDrawBlendMode(rc->renderer, SDL_BLENDMODE_BLEND);
    setCellColor(rc, color, 80);
    SDL_RenderFillRect(rc->renderer, &rect);
    SDL_SetRenderDrawBlendMode(rc->renderer, SDL_BLENDMODE_NONE);
}

void renderMiniCell(render_context_t* rc, int x, int y, int size, cell_color_t color) {
    SDL_FRect rect = {
        .x = (float)x,
        .y = (float)y,
        .w = (float)size,
        .h = (float)size
    };

    setCellColor(rc, color, 255);
    SDL_RenderFillRect(rc->renderer, &rect);
}

void renderHudBox(render_context_t* rc, int x, int y, int w, int h) {
    SDL_FRect rect = {
        .x = (float)x,
        .y = (float)y,
        .w = (float)w,
        .h = (float)h
    };

    SDL_SetRenderDrawColor(rc->renderer, 22, 22, 25, 255);
    SDL_RenderFillRect(rc->renderer, &rect);
    SDL_SetRenderDrawColor(rc->renderer, 70, 70, 78, 255);
    SDL_RenderRect(rc->renderer, &rect);
}

void renderText(render_context_t* rc, int x, int y, const char* text) {
    text_cache_entry_t* entry = NULL;

    for (int i = 0; i < TEXT_CACHE_SIZE; i++) {
        if (rc->textCache[i].texture != NULL && strcmp(rc->textCache[i].text, text) == 0) {
            entry = &rc->textCache[i];
            break;
        }
    }

    if (entry == NULL) {
        entry = &rc->textCache[rc->textCacheNext];
        rc->textCacheNext = (rc->textCacheNext + 1) % TEXT_CACHE_SIZE;

        if (entry->texture != NULL)
            SDL_DestroyTexture(entry->texture);

        entry->texture = NULL;
        entry->w = 0;
        entry->h = 0;
        snprintf(entry->text, sizeof(entry->text), "%s", text);
    }

    if (entry->texture == NULL) {
        SDL_Color color = {255, 255, 255, 255};
        SDL_Surface* surface = TTF_RenderText_Blended(rc->font, entry->text,
                strlen(entry->text), color);
        if (surface == NULL)
            return;

        entry->texture = SDL_CreateTextureFromSurface(rc->renderer, surface);
        entry->w = surface->w;
        entry->h = surface->h;
        SDL_DestroySurface(surface);

        if (entry->texture == NULL)
            return;
    }

    SDL_FRect dst = {
        .x = (float)x,
        .y = (float)y,
        .w = (float)entry->w,
        .h = (float)entry->h
    };

    SDL_RenderTexture(rc->renderer, entry->texture, NULL, &dst);
}

void renderPauseOverlay(render_context_t* rc) {
    int boardW = rc->config.cols * rc->config.blockSize;
    int boardH = rc->config.rows * rc->config.blockSize;
    SDL_FRect shade = {
        .x = 0.0f,
        .y = 0.0f,
        .w = (float)boardW,
        .h = (float)boardH
    };
    SDL_FRect box = {
        .x = (float)(boardW / 2 - 58),
        .y = (float)(boardH / 2 - 24),
        .w = 116.0f,
        .h = 48.0f
    };

    SDL_SetRenderDrawBlendMode(rc->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(rc->renderer, 150, 150, 150, 110);
    SDL_RenderFillRect(rc->renderer, &shade);

    SDL_SetRenderDrawColor(rc->renderer, 28, 28, 32, 230);
    SDL_RenderFillRect(rc->renderer, &box);
    SDL_SetRenderDrawColor(rc->renderer, 95, 95, 105, 255);
    SDL_RenderRect(rc->renderer, &box);
    SDL_SetRenderDrawBlendMode(rc->renderer, SDL_BLENDMODE_NONE);

    renderText(rc, (int)box.x + 31, (int)box.y + 14, "PAUSE");
}

void renderEnd(render_context_t* rc) {
    SDL_RenderPresent(rc->renderer);
}
