#ifndef GRID_H
#define GRID_H

#include <stddef.h>

typedef struct grid_t {
    void*  cells;
    size_t elementSize;
    int    rows;
    int    cols;
} grid_t;

grid_t* makeCopyGrid(const grid_t* grid);
grid_t* makeModelGrid(int rows, int cols, size_t elementSize, const void* init);
grid_t* makeEmptyGrid();
void freeGrid(grid_t** grid);

static inline void* grid_at(grid_t* g, int r, int c) {
    return (unsigned char*)g->cells + ((size_t)(r * g->cols + c) * g->elementSize);
}

static inline const void* grid_at_const(const grid_t* g, int r, int c) {
    return (const unsigned char*)g->cells + ((size_t)(r * g->cols + c) * g->elementSize);
}

#define GRID_AT(T, gptr, r, c) ((T*)grid_at((gptr), (r), (c)))

#endif
