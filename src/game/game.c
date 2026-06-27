#include "game.h"

#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_timer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    int col;
    int row;
} block_t;

static const block_t PIECES[7][4][4] = {
    {
        {{0, 1}, {1, 1}, {2, 1}, {3, 1}},
        {{2, 0}, {2, 1}, {2, 2}, {2, 3}},
        {{0, 2}, {1, 2}, {2, 2}, {3, 2}},
        {{1, 0}, {1, 1}, {1, 2}, {1, 3}}
    },
    {
        {{0, 0}, {0, 1}, {1, 1}, {2, 1}},
        {{1, 0}, {2, 0}, {1, 1}, {1, 2}},
        {{0, 1}, {1, 1}, {2, 1}, {2, 2}},
        {{1, 0}, {1, 1}, {0, 2}, {1, 2}}
    },
    {
        {{2, 0}, {0, 1}, {1, 1}, {2, 1}},
        {{1, 0}, {1, 1}, {1, 2}, {2, 2}},
        {{0, 1}, {1, 1}, {2, 1}, {0, 2}},
        {{0, 0}, {1, 0}, {1, 1}, {1, 2}}
    },
    {
        {{1, 0}, {2, 0}, {1, 1}, {2, 1}},
        {{1, 0}, {2, 0}, {1, 1}, {2, 1}},
        {{1, 0}, {2, 0}, {1, 1}, {2, 1}},
        {{1, 0}, {2, 0}, {1, 1}, {2, 1}}
    },
    {
        {{1, 0}, {2, 0}, {0, 1}, {1, 1}},
        {{1, 0}, {1, 1}, {2, 1}, {2, 2}},
        {{1, 1}, {2, 1}, {0, 2}, {1, 2}},
        {{0, 0}, {0, 1}, {1, 1}, {1, 2}}
    },
    {
        {{1, 0}, {0, 1}, {1, 1}, {2, 1}},
        {{1, 0}, {1, 1}, {2, 1}, {1, 2}},
        {{0, 1}, {1, 1}, {2, 1}, {1, 2}},
        {{1, 0}, {0, 1}, {1, 1}, {1, 2}}
    },
    {
        {{0, 0}, {1, 0}, {1, 1}, {2, 1}},
        {{2, 0}, {1, 1}, {2, 1}, {1, 2}},
        {{0, 1}, {1, 1}, {1, 2}, {2, 2}},
        {{1, 0}, {0, 1}, {1, 1}, {0, 2}}
    }
};

static cell_color_t pieceColor(int piece) {
    return (cell_color_t)(piece + 1);
}

static int randomPiece() {
    return rand() % 7;
}

static void refillNextQueue(game_t* game) {
    for (int i = 0; i < MAX_NEXT_PIECES; i++)
        game->next[i] = randomPiece();
}

static void spawnPiece(game_t* game) {
    game->piece = game->next[0];

    for (int i = 0; i < MAX_NEXT_PIECES - 1; i++)
        game->next[i] = game->next[i + 1];

    game->next[MAX_NEXT_PIECES - 1] = randomPiece();
    game->rotation = 0;
    game->row = 0;
    game->col = 3;
}

static bool collides(const game_t* game, int row, int col, int rotation) {
    const block_t* shape = PIECES[game->piece][rotation];

    for (int i = 0; i < 4; i++) {
        int r = row + shape[i].row;
        int c = col + shape[i].col;

        if (c < 0 || c >= BOARD_COLS || r >= BOARD_ROWS)
            return true;
        if (r >= 0 && game->board[r][c] != CELL_EMPTY)
            return true;
    }

    return false;
}

static int clearLines(game_t* game) {
    int cleared = 0;

    for (int r = BOARD_ROWS - 1; r >= 0; r--) {
        bool full = true;

        for (int c = 0; c < BOARD_COLS; c++)
            if (game->board[r][c] == CELL_EMPTY)
                full = false;

        if (!full)
            continue;

        for (int rr = r; rr > 0; rr--)
            memcpy(game->board[rr], game->board[rr - 1], sizeof(game->board[rr]));

        memset(game->board[0], 0, sizeof(game->board[0]));
        game->lines++;
        cleared++;
        r++;
    }

    return cleared;
}

static void addLineScore(game_t* game, int cleared) {
    switch (cleared) {
        case 1: game->score += 100; break;
        case 2: game->score += 300; break;
        case 3: game->score += 500; break;
        case 4: game->score += 800; break;
        default: break;
    }
}

static void lockPiece(game_t* game) {
    const block_t* shape = PIECES[game->piece][game->rotation];

    for (int i = 0; i < 4; i++) {
        int r = game->row + shape[i].row;
        int c = game->col + shape[i].col;

        if (r >= 0 && r < BOARD_ROWS && c >= 0 && c < BOARD_COLS) {
            game->board[r][c] = pieceColor(game->piece);
        }
    }

    game->lockedPieces++;
    addLineScore(game, clearLines(game));
    spawnPiece(game);

    if (collides(game, game->row, game->col, game->rotation))
        game->gameOver = true;
}

static void movePiece(game_t* game, int drow, int dcol) {
    if (game->gameOver)
        return;

    if (!collides(game, game->row + drow, game->col + dcol, game->rotation)) {
        game->row += drow;
        game->col += dcol;
        return;
    }

    if (drow > 0)
        lockPiece(game);
}

static void rotatePiece(game_t* game) {
    if (game->gameOver)
        return;

    int next = (game->rotation + 1) % 4;

    if (!collides(game, game->row, game->col, next)) {
        game->rotation = next;
        return;
    }

    if (!collides(game, game->row, game->col - 1, next)) {
        game->col--;
        game->rotation = next;
        return;
    }

    if (!collides(game, game->row, game->col + 1, next)) {
        game->col++;
        game->rotation = next;
    }
}

static void hardDrop(game_t* game) {
    if (game->gameOver)
        return;

    while (!collides(game, game->row + 1, game->col, game->rotation))
        game->row++;

    lockPiece(game);
}

void initGame(game_t* game, tretis_config_t config) {
    static bool seeded = false;

    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = true;
    }

    memset(game, 0, sizeof(*game));
    game->config = config;
    game->running = true;
    game->startedAt = SDL_GetTicks();
    refillNextQueue(game);
    spawnPiece(game);
}

void handleGameKey(game_t* game, int key) {
    switch (key) {
        case SDLK_ESCAPE:
        case SDLK_Q:
            game->running = false;
            break;
        case SDLK_R:
            initGame(game, game->config);
            break;
        case SDLK_LEFT:
        case SDLK_A:
            movePiece(game, 0, -1);
            break;
        case SDLK_RIGHT:
        case SDLK_D:
            movePiece(game, 0, 1);
            break;
        case SDLK_DOWN:
        case SDLK_S:
            movePiece(game, 1, 0);
            break;
        case SDLK_UP:
        case SDLK_W:
            rotatePiece(game);
            break;
        case SDLK_SPACE:
            hardDrop(game);
            break;
        default:
            break;
    }
}

void updateGame(game_t* game, uint64_t now) {
    if (game->gameOver)
        return;

    if (now - game->lastFall < (uint64_t)game->config.fallDelay)
        return;

    game->lastFall = now;
    movePiece(game, 1, 0);
}

static int ghostRow(const game_t* game) {
    int row = game->row;

    while (!collides(game, row + 1, game->col, game->rotation))
        row++;

    return row;
}

static void drawPiece(render_context_t* rc, int piece, int rotation, int row, int col, bool ghost)
{
    const block_t* shape = PIECES[piece][rotation];

    for (int i = 0; i < 4; i++) {
        int r = row + shape[i].row;
        int c = col + shape[i].col;

        if (r >= 0 && r < BOARD_ROWS && c >= 0 && c < BOARD_COLS) {
            if (ghost)
                renderGhostCell(rc, r, c, pieceColor(piece));
            else
                renderCell(rc, r, c, pieceColor(piece));
        }
    }
}

static void drawMiniPiece(render_context_t* rc, int piece, int x, int y, int size) {
    const block_t* shape = PIECES[piece][0];

    for (int i = 0; i < 4; i++) {
        int px = x + shape[i].col * size;
        int py = y + shape[i].row * size;
        renderMiniCell(rc, px, py, size, pieceColor(piece));
    }
}

static void drawHud(const game_t* game, render_context_t* rc) {
    if (!game->config.showHud)
        return;

    int x = game->config.cols * game->config.blockSize + 16;
    int y = 18;
    uint64_t elapsed = (SDL_GetTicks() - game->startedAt) / 1000;
    char buffer[64];

    renderText(rc, x, y, "TRETIS");
    y += 28;

    snprintf(buffer, sizeof(buffer), "score %d", game->score);
    renderText(rc, x, y, buffer);
    y += 16;

    snprintf(buffer, sizeof(buffer), "lines %d", game->lines);
    renderText(rc, x, y, buffer);
    y += 16;

    snprintf(buffer, sizeof(buffer), "pieces %d", game->lockedPieces);
    renderText(rc, x, y, buffer);
    y += 16;

    snprintf(buffer, sizeof(buffer), "time %02llu:%02llu",
            (unsigned long long)(elapsed / 60),
            (unsigned long long)(elapsed % 60));
    renderText(rc, x, y, buffer);
    y += 34;

    renderText(rc, x, y, "next");
    y += 18;

    int count = game->config.nextPieces;
    if (count > MAX_NEXT_PIECES)
        count = MAX_NEXT_PIECES;

    for (int i = 0; i < count; i++) {
        drawMiniPiece(rc, game->next[i], x, y, 12);
        y += 54;
    }

    if (game->gameOver)
        renderText(rc, x, game->config.rows * game->config.blockSize - 28, "R restart");
}

void drawGame(const game_t* game, render_context_t* rc) {
    for (int r = 0; r < BOARD_ROWS; r++)
    for (int c = 0; c < BOARD_COLS; c++)
        renderCell(rc, r, c, game->board[r][c]);

    if (!game->gameOver) {
        if (game->config.showGhost)
            drawPiece(rc, game->piece, game->rotation, ghostRow(game), game->col, true);

        drawPiece(rc, game->piece, game->rotation, game->row, game->col, false);
    }

    if (game->gameOver) {
        for (int r = 7; r < 13; r++)
        for (int c = 2; c < 8; c++)
            renderCell(rc, r, c, CELL_RED);
    }

    drawHud(game, rc);
}

bool isGameRunning(const game_t* game) {
    return game->running;
}
