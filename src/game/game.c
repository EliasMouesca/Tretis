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

static int elapsedSeconds(const game_t* game);

static void saveUndoSnapshot(game_t* game) {
    int limit = game->config.undoLimit;

    if (limit <= 0)
        return;
    if (limit > MAX_UNDO_HISTORY)
        limit = MAX_UNDO_HISTORY;

    if (game->undoCount == limit) {
        memmove(&game->undoHistory[0], &game->undoHistory[1],
                (size_t)(limit - 1) * sizeof(game->undoHistory[0]));
        game->undoCount--;
    }

    game_snapshot_t* snapshot = &game->undoHistory[game->undoCount++];
    memcpy(snapshot->board, game->board, sizeof(snapshot->board));
    memcpy(snapshot->next, game->next, sizeof(snapshot->next));
    memcpy(snapshot->bag, game->bag, sizeof(snapshot->bag));
    memcpy(snapshot->history, game->history, sizeof(snapshot->history));
    snapshot->piece = game->piece;
    snapshot->bagSize = game->bagSize;
    snapshot->bagIndex = game->bagIndex;
    snapshot->generatedPieces = game->generatedPieces;
    snapshot->heldPiece = game->heldPiece;
    snapshot->rotation = game->rotation;
    snapshot->row = game->row;
    snapshot->col = game->col;
    snapshot->hasHeldPiece = game->hasHeldPiece;
    snapshot->swappedHeldThisTurn = game->swappedHeldThisTurn;
    snapshot->lines = game->lines;
    snapshot->tretises = game->tretises;
    snapshot->score = game->score;
    snapshot->lockedPieces = game->lockedPieces;
    snapshot->startedAt = game->startedAt;
    snapshot->lastFall = game->lastFall;
    snapshot->nextMoveAt = game->nextMoveAt;
    snapshot->nextSoftFallAt = game->nextSoftFallAt;
    snapshot->lastTick = game->lastTick;
    snapshot->elapsedTime = game->elapsedTime;
    snapshot->groundedAt = game->groundedAt;
    snapshot->lastLockDelayedAt = game->lastLockDelayedAt;
    snapshot->stats = game->stats;
    snapshot->config = game->config;
    snapshot->running = game->running;
    snapshot->paused = game->paused;
    snapshot->gameOver = game->gameOver;
    snapshot->statsSaved = game->statsSaved;
    snapshot->movingLeft = game->movingLeft;
    snapshot->movingRight = game->movingRight;
    snapshot->softDropping = game->softDropping;
    snapshot->grounded = game->grounded;
}

static void restoreUndoSnapshot(game_t* game) {
    int undoCount = game->undoCount - 1;
    game_snapshot_t* snapshot;

    if (undoCount < 0)
        return;

    snapshot = &game->undoHistory[undoCount];
    memcpy(game->board, snapshot->board, sizeof(game->board));
    memcpy(game->next, snapshot->next, sizeof(game->next));
    memcpy(game->bag, snapshot->bag, sizeof(game->bag));
    memcpy(game->history, snapshot->history, sizeof(game->history));
    game->piece = snapshot->piece;
    game->bagSize = snapshot->bagSize;
    game->bagIndex = snapshot->bagIndex;
    game->generatedPieces = snapshot->generatedPieces;
    game->heldPiece = snapshot->heldPiece;
    game->rotation = snapshot->rotation;
    game->row = snapshot->row;
    game->col = snapshot->col;
    game->hasHeldPiece = snapshot->hasHeldPiece;
    game->swappedHeldThisTurn = snapshot->swappedHeldThisTurn;
    game->lines = snapshot->lines;
    game->tretises = snapshot->tretises;
    game->score = snapshot->score;
    game->lockedPieces = snapshot->lockedPieces;
    game->startedAt = snapshot->startedAt;
    game->lastFall = snapshot->lastFall;
    game->nextMoveAt = snapshot->nextMoveAt;
    game->nextSoftFallAt = snapshot->nextSoftFallAt;
    game->lastTick = snapshot->lastTick;
    game->elapsedTime = snapshot->elapsedTime;
    game->groundedAt = snapshot->groundedAt;
    game->lastLockDelayedAt = snapshot->lastLockDelayedAt;
    game->stats = snapshot->stats;
    game->config = snapshot->config;
    game->running = snapshot->running;
    game->paused = snapshot->paused;
    game->gameOver = snapshot->gameOver;
    game->statsSaved = snapshot->statsSaved;
    game->movingLeft = snapshot->movingLeft;
    game->movingRight = snapshot->movingRight;
    game->softDropping = snapshot->softDropping;
    game->grounded = snapshot->grounded;
    game->undoCount = undoCount;
}

static void undoLastLock(game_t* game) {
    int highScore;
    uint64_t now;

    if (game->paused || game->gameOver || game->undoCount == 0)
        return;

    highScore = game->stats.highScore;
    restoreUndoSnapshot(game);
    if (game->stats.highScore < highScore)
        game->stats.highScore = highScore;

    // Give the restored piece a fresh chance to be moved or dropped.
    now = SDL_GetTicks();
    game->grounded = false;
    game->groundedAt = now;
    game->lastLockDelayedAt = now;
    game->lastFall = now;
    game->nextSoftFallAt = now + game->config.moveRepeatInitialDelay;
}
static void syncElapsedTime(game_t* game, uint64_t now);
static void endGame(game_t* game);

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

static int uniformPiece() {
    return rand() % 7;
}

static void shuffle(int* values, int count) {
    for (int i = count - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = values[i];
        values[i] = values[j];
        values[j] = tmp;
    }
}

static void refillBag(game_t* game) {
    if (game->config.randomizer == RANDOMIZER_35_BAG) {
        game->bagSize = 35;
        for (int i = 0; i < game->bagSize; i++)
            game->bag[i] = i % 7;
    } else {
        game->bagSize = 7;
        for (int i = 0; i < game->bagSize; i++)
            game->bag[i] = i;
    }

    shuffle(game->bag, game->bagSize);
    game->bagIndex = 0;
}

static bool inHistory(const game_t* game, int piece) {
    for (int i = 0; i < 4; i++)
        if (game->history[i] == piece)
            return true;

    return false;
}

static void pushHistory(game_t* game, int piece) {
    for (int i = 3; i > 0; i--)
        game->history[i] = game->history[i - 1];

    game->history[0] = piece;
}

static int tgmPiece(game_t* game) {
    int piece = uniformPiece();

    while (game->generatedPieces == 0 && (piece == 3 || piece == 4 || piece == 6))
        piece = uniformPiece();

    for (int i = 0; i < 4 && inHistory(game, piece); i++)
        piece = uniformPiece();

    pushHistory(game, piece);
    game->generatedPieces++;
    return piece;
}

static int nextRandomPiece(game_t* game) {
    switch (game->config.randomizer) {
        case RANDOMIZER_UNIFORM:
            return uniformPiece();
        case RANDOMIZER_TGM:
            return tgmPiece(game);
        case RANDOMIZER_35_BAG:
        case RANDOMIZER_7_BAG:
            if (game->bagIndex >= game->bagSize)
                refillBag(game);
            return game->bag[game->bagIndex++];
    }

    return uniformPiece();
}

static void refillNextQueue(game_t* game) {
    for (int i = 0; i < MAX_NEXT_PIECES; i++)
        game->next[i] = nextRandomPiece(game);
}

static void updateHighScore(game_t* game) {
    if (game->score <= game->stats.highScore)
        return;

    game->stats.highScore = game->score;
    saveTretisStats(game->config.statsPath, game->stats);
}

static void spawnPiece(game_t* game) {
    game->piece = game->next[0];

    for (int i = 0; i < MAX_NEXT_PIECES - 1; i++)
        game->next[i] = game->next[i + 1];

    game->next[MAX_NEXT_PIECES - 1] = nextRandomPiece(game);
    game->rotation = 0;
    game->row = 0;
    game->col = 3;
    game->swappedHeldThisTurn = false;
    game->grounded = false;
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

    if (cleared == 4)
        game->tretises++;

    updateHighScore(game);
}

static void lockPiece(game_t* game) {
    const block_t* shape = PIECES[game->piece][game->rotation];
    uint64_t now = SDL_GetTicks();

    saveUndoSnapshot(game);

    for (int i = 0; i < 4; i++) {
        int r = game->row + shape[i].row;
        int c = game->col + shape[i].col;

        if (r >= 0 && r < BOARD_ROWS && c >= 0 && c < BOARD_COLS) {
            game->board[r][c] = pieceColor(game->piece);
        }
    }

    // So the soft fall doesn't feel like it gets directly transfered to the next piece (avoids unwanted softdrops)
    game->nextSoftFallAt = now + game->config.moveRepeatInitialDelay;
    game->lockedPieces++;
    addLineScore(game, clearLines(game));
    spawnPiece(game);

    if (collides(game, game->row, game->col, game->rotation)) {
        endGame(game);
    }

    game->grounded = false;
}

static void movePiece(game_t* game, int drow, int dcol) {
    uint64_t now = SDL_GetTicks();

    if (game->gameOver)
        return;

    if (!collides(game, game->row + drow, game->col + dcol, game->rotation)) {
        game->row += drow;
        game->col += dcol;
        return;
    }

    if (drow > 0) {
        if(!game->grounded){
            game->groundedAt = now;
            game->lastLockDelayedAt= now;
            game->grounded = true;
        }
        return;
    }
}

static void rotatePiece(game_t* game, int direction) {
    if (game->gameOver)
        return;

    int next = (game->rotation + direction + 4) % 4;

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

static void holdPiece(game_t* game) {
    if (game->gameOver || game->swappedHeldThisTurn)
        return;

    if (!game->hasHeldPiece) {
        game->heldPiece = game->piece;
        game->hasHeldPiece = true;
        spawnPiece(game);
    } else {
        int piece = game->piece;
        game->piece = game->heldPiece;
        game->heldPiece = piece;
        game->rotation = 0;
        game->row = 0;
        game->col = 3;

        if (collides(game, game->row, game->col, game->rotation)) {
            endGame(game);
        }
    }

    game->swappedHeldThisTurn = true;
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
    game->heldPiece = -1;
    game->bagIndex = 0;
    game->bagSize = 0;
    for (int i = 0; i < 4; i++)
        game->history[i] = 6;
    game->generatedPieces = 0;
    game->stats = loadTretisStats(config.statsPath);
    game->startedAt = SDL_GetTicks();
    game->lastTick = game->startedAt;
    refillNextQueue(game);
    spawnPiece(game);
}

void handleGameKey(game_t* game, SDL_Keycode key) {
    handleGameKeyWithMod(game, key, SDL_KMOD_NONE);
}

static bool matchesUndoShortcut(const game_t* game, SDL_Keycode key, SDL_Keymod mod) {
    const SDL_Keymod relevant = SDL_KMOD_CTRL | SDL_KMOD_SHIFT | SDL_KMOD_ALT | SDL_KMOD_GUI;

    return key == game->config.keyUndo &&
        (mod & relevant) == game->config.keyUndoMod;
}

void handleGameKeyWithMod(game_t* game, SDL_Keycode key, SDL_Keymod mod) {
    uint64_t now = SDL_GetTicks();

    if (key == game->config.keyQuit) {
        game->running = false;
        return;
    }

    if (key == game->config.keyPause || key == SDLK_ESCAPE) {
        bool wasPaused = game->paused;

        syncElapsedTime(game, now);
        game->paused = !game->paused;
        if (wasPaused) {
            game->lastFall = now;
            game->nextMoveAt = now + game->config.moveRepeatInitialDelay;
            game->nextSoftFallAt = now + game->config.moveRepeatDelay;
        }

        return;
    }

    if (key == game->config.keyRestart) {
        finalizeGame(game);
        initGame(game, game->config);
        return;
    }

    if (matchesUndoShortcut(game, key, mod)) {
        undoLastLock(game);
        return;
    }

    if (game->paused)
        return;


    if (key == game->config.keyHold || key == SDLK_C)
        holdPiece(game);
    else if (key == game->config.keyLeft || key == SDLK_A) {
        game->movingLeft = true;
        game->movingRight = false;
        game->nextMoveAt = SDL_GetTicks() + game->config.moveRepeatInitialDelay;
        movePiece(game, 0, -1);
    }
    else if (key == game->config.keyRight || key == SDLK_D) {
        game->movingRight = true;
        game->movingLeft = false;
        game->nextMoveAt = SDL_GetTicks() + game->config.moveRepeatInitialDelay;
        movePiece(game, 0, 1);
    }
    else if (key == game->config.keyDown || key == SDLK_S) {
        game->softDropping = true;

        game->nextSoftFallAt = now + game->config.moveRepeatDelay;
        game->lastFall = now;
        movePiece(game, 1, 0);
    }
    else if (key == game->config.keyRotateLeft) {
        game->lastLockDelayedAt = now;
        rotatePiece(game, -1);
    }
    else if (key == game->config.keyRotate || key == SDLK_W || key == game->config.keyRotateRight) {
        game->lastLockDelayedAt = now;
        rotatePiece(game, 1);
    }
    else if (key == game->config.keyDrop)
        hardDrop(game);
}

void releaseGameKey(game_t* game, SDL_Keycode key) {
    if (key == game->config.keyLeft || key == SDLK_A)
        game->movingLeft = false;
    if (key == game->config.keyRight || key == SDLK_D)
        game->movingRight = false;
    if (key == game->config.keyDown || key == SDLK_S)
        game->softDropping = false;
}

void updateGame(game_t* game, uint64_t now) {
    syncElapsedTime(game, now);

    int delay = game->config.fallDelay;
    int lockDelay = game->config.lockDelay;
    int maxLockDelay = game->config.maxLockDelay;

    if (game->gameOver || game->paused)
        return;

    if ((game->movingLeft || game->movingRight) && now >= game->nextMoveAt) {
        game->nextMoveAt = now + game->config.moveRepeatDelay;
        movePiece(game, 0, game->movingRight ? 1 : -1);
    }

    if (game->softDropping && now >= game->nextSoftFallAt) {
        game->nextSoftFallAt = now + (uint64_t)game->config.softFallDelay;
        game->lastFall = now;
        movePiece(game, 1, 0);
        if (game->gameOver)
            return;
    }


    if (game->config.speedup && game->config.speedupEvery > 0) {
        int elapsed = elapsedSeconds(game);
        delay -= (elapsed / game->config.speedupEvery) * game->config.speedupStep;

        if (delay < game->config.minFallDelay)
            delay = game->config.minFallDelay;
    }

    if (game->grounded && 
        (now - game->lastLockDelayedAt > (uint64_t)lockDelay || now - game->groundedAt > (uint64_t)maxLockDelay)){
        lockPiece(game);
        return;
    }

    if (now - game->lastFall > (uint64_t)delay){
        game->lastFall = now;
        movePiece(game, 1, 0);
        return;
    }

    return;
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

static void drawMiniPieceCentered(render_context_t* rc, int piece, int x, int y,
        int boxSize, int cellSize)
{
    const block_t* shape = PIECES[piece][0];
    int minCol = shape[0].col;
    int maxCol = shape[0].col;
    int minRow = shape[0].row;
    int maxRow = shape[0].row;

    for (int i = 1; i < 4; i++) {
        if (shape[i].col < minCol) minCol = shape[i].col;
        if (shape[i].col > maxCol) maxCol = shape[i].col;
        if (shape[i].row < minRow) minRow = shape[i].row;
        if (shape[i].row > maxRow) maxRow = shape[i].row;
    }

    int width = (maxCol - minCol + 1) * cellSize;
    int height = (maxRow - minRow + 1) * cellSize;
    int originX = x + (boxSize - width) / 2 - minCol * cellSize;
    int originY = y + (boxSize - height) / 2 - minRow * cellSize;

    drawMiniPiece(rc, piece, originX, originY, cellSize);
}

static int scaledHudValue(const game_t* game, int value) {
    return value * game->config.blockSize / DEFAULT_BLOCK_SIZE;
}

static int elapsedSeconds(const game_t* game) {
    return (int)(game->elapsedTime / 1000);
}

static void syncElapsedTime(game_t* game, uint64_t now) {
    if (now < game->lastTick) {
        game->lastTick = now;
        return;
    }

    if (!game->paused && !game->gameOver)
        game->elapsedTime += now - game->lastTick;

    game->lastTick = now;
}

static void endGame(game_t* game) {
    syncElapsedTime(game, SDL_GetTicks());
    game->gameOver = true;
    finalizeGame(game);
}

void finalizeGame(game_t* game) {
    int elapsed = elapsedSeconds(game);

    if (game->statsSaved)
        return;

    game->stats.gamesPlayed++;

    if (game->score > game->stats.highScore)
        game->stats.highScore = game->score;
    if (elapsed > game->stats.longestTime)
        game->stats.longestTime = elapsed;
    if (game->tretises > game->stats.mostTretises)
        game->stats.mostTretises = game->tretises;

    saveTretisStats(game->config.statsPath, game->stats);
    game->statsSaved = true;
}

static void drawHud(const game_t* game, render_context_t* rc) {
    if (!game->config.showHud)
        return;

    int line = renderTextLineSkip(rc);
    int margin = scaledHudValue(game, 16);
    int smallGap = scaledHudValue(game, 4);
    int mediumGap = scaledHudValue(game, 10);
    int miniCell = scaledHudValue(game, 12);
    int nextBox = miniCell * 4;
    int holdBox = nextBox + scaledHudValue(game, 8);
    int x = game->config.cols * game->config.blockSize + margin;
    int y = scaledHudValue(game, 18);
    uint64_t elapsed = (uint64_t)elapsedSeconds(game);
    char buffer[64];

    renderText(rc, x, y, "TRETIS");
    y += line + mediumGap;

    if (!game->config.zenMode) {
        snprintf(buffer, sizeof(buffer), "score %d", game->score);
        renderText(rc, x, y, buffer);
        y += line + smallGap;

        snprintf(buffer, sizeof(buffer), "high %d", game->stats.highScore);
        renderText(rc, x, y, buffer);
        y += line + smallGap;

        snprintf(buffer, sizeof(buffer), "lines %d", game->lines);
        renderText(rc, x, y, buffer);
        y += line + smallGap;

        snprintf(buffer, sizeof(buffer), "tretises %d", game->tretises);
        renderText(rc, x, y, buffer);
        y += line + smallGap;

        snprintf(buffer, sizeof(buffer), "pieces %d", game->lockedPieces);
        renderText(rc, x, y, buffer);
        y += line + smallGap;
    }

    snprintf(buffer, sizeof(buffer), "time %02llu:%02llu",
            (unsigned long long)(elapsed / 60),
            (unsigned long long)(elapsed % 60));
    renderText(rc, x, y, buffer);
    y += line + mediumGap;

    renderText(rc, x, y, "next");
    y += line + smallGap;

    int count = game->config.nextPieces;
    if (count > MAX_NEXT_PIECES)
        count = MAX_NEXT_PIECES;

    for (int i = 0; i < count; i++) {
        drawMiniPieceCentered(rc, game->next[i], x, y, nextBox, miniCell);
        y += nextBox + scaledHudValue(game, 6);
    }

    y += mediumGap;
    renderText(rc, x, y, "hold");
    y += line + smallGap;

    renderHudBox(rc, x, y, holdBox, holdBox);

    if (game->hasHeldPiece)
        drawMiniPieceCentered(rc, game->heldPiece, x, y, holdBox, miniCell);

    if (game->gameOver)
        renderText(rc, x, game->config.rows * game->config.blockSize -
                line - mediumGap, "R restart");
}

void drawGame(const game_t* game, render_context_t* rc) {
    for (int r = 0; r < BOARD_ROWS; r++)
    for (int c = 0; c < BOARD_COLS; c++)
        if (game->board[r][c] != CELL_EMPTY)
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

    if (game->paused)
        renderPauseOverlay(rc);
}

bool isGameRunning(const game_t* game) {
    return game->running;
}
