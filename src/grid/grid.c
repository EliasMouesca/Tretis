#include "grid.h"

#include "../log/log.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static grid_t* allocGrid() {
    grid_t* g = malloc(sizeof(grid_t));

    if (!g)
        critical("Out of memory");

    return g;
}

static size_t gridDataSize(int rows, int cols, size_t elementSize) {
    if (rows < 0 || cols < 0)
        critical("Grid dimensions must be non-negative");
    if (elementSize == 0)
        critical("Grid element size must be greater than zero");

    size_t rowCount = (size_t)rows;
    size_t colCount = (size_t)cols;

    if (rowCount != 0 && colCount > SIZE_MAX / rowCount)
        critical("Grid dimensions are too large");

    size_t cells = rowCount * colCount;
    if (cells != 0 && elementSize > SIZE_MAX / cells)
        critical("Grid storage is too large");

    return cells * elementSize;
}

grid_t* makeCopyGrid(const grid_t* grid) {
    if (!grid)
        critical("Cannot copy null grid");

    grid_t* g = allocGrid();
    g->rows = grid->rows;
    g->cols = grid->cols;
    g->elementSize = grid->elementSize;

    size_t total = gridDataSize(g->rows, g->cols, g->elementSize);

    if (total > 0) {
        g->cells = malloc(total);
        if (!g->cells) critical("Out of memory");
        memcpy(g->cells, grid->cells, total);
    } else {
        g->cells = NULL;
    }

    return g;
}

grid_t* makeEmptyGrid() {
    grid_t* g = allocGrid();
    g->rows = 0;
    g->cols = 0;
    g->elementSize = 1;
    g->cells = NULL;

    return g;
}

grid_t* makeModelGrid(int rows, int cols, size_t elementSize, const void* model) {
    grid_t* g = allocGrid();
    g->rows = rows;
    g->cols = cols;
    g->elementSize = elementSize;

    size_t total = gridDataSize(rows, cols, elementSize);
    if (total == 0) {
        g->cells = NULL;
        return g;
    }

    g->cells = malloc(total);
    if (!g->cells) critical("Out of memory");

    if (!model) {
        memset(g->cells, 0, total);
        return g;
    }

    for (size_t i = 0; i < total; i += elementSize)
        memcpy((char*)g->cells + i, model, elementSize);

    return g;
}

void freeGrid(grid_t** grid) {
    if (!grid || !*grid)
        return;

    grid_t* g = *grid;
    if (g->cells) free(g->cells);
    free(g);
    *grid = NULL;
}
