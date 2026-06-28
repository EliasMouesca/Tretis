#include "grid.h"

#include <assert.h>

static void testModelGridCopiesModelToEveryCell() {
    int model = 7;
    grid_t* grid = makeModelGrid(2, 3, sizeof(int), &model);

    assert(grid != NULL);
    assert(grid->rows == 2);
    assert(grid->cols == 3);
    assert(grid->elementSize == sizeof(int));

    for (int r = 0; r < grid->rows; r++)
    for (int c = 0; c < grid->cols; c++)
        assert(*GRID_AT(int, grid, r, c) == model);

    freeGrid(&grid);
    assert(grid == NULL);
}

static void testNullModelInitializesToZero() {
    grid_t* grid = makeModelGrid(2, 2, sizeof(int), NULL);

    for (int r = 0; r < grid->rows; r++)
    for (int c = 0; c < grid->cols; c++)
        assert(*GRID_AT(int, grid, r, c) == 0);

    freeGrid(&grid);
}

static void testCopyGridOwnsIndependentCells() {
    int model = 3;
    grid_t* grid = makeModelGrid(1, 2, sizeof(int), &model);
    grid_t* copy = makeCopyGrid(grid);

    *GRID_AT(int, grid, 0, 0) = 9;

    assert(*GRID_AT(int, copy, 0, 0) == 3);
    assert(*GRID_AT(int, copy, 0, 1) == 3);

    freeGrid(&copy);
    freeGrid(&grid);
}

static void testEmptyGridCanBeFreedSafely() {
    grid_t* grid = makeEmptyGrid();

    assert(grid->rows == 0);
    assert(grid->cols == 0);
    assert(grid->cells == NULL);

    freeGrid(&grid);
    assert(grid == NULL);
    freeGrid(&grid);
    freeGrid(NULL);
}

int main() {
    testModelGridCopiesModelToEveryCell();
    testNullModelInitializesToZero();
    testCopyGridOwnsIndependentCells();
    testEmptyGridCanBeFreedSafely();
    return 0;
}
