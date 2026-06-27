# Repository Guidelines

## Project Structure & Module Organization

This is a small C/SDL3 project. Source files live under `src/`, with
`src/main.c` as the entry point. Reusable modules are grouped by directory, for
example `src/grid/grid.c` with `src/grid/grid.h`, `src/log/log.c`, and
`src/render_context/render_context.c`.

Build artifacts are generated into `obj/`, and the executable is `main`. These
are disposable outputs and should not be committed.

## Build, Test, and Development Commands

- `make`: compiles all `.c` files under `src/` into `main` using `gcc`,
  `pkg-config`, SDL3, and SDL3_ttf.
- `make test`: builds and runs module tests discovered as
  `src/<module>/<module>_test.c`, wrapping each test with Valgrind.
- `make test-run`: runs `main` under Valgrind with `--block-size 7`.
- `make test-stress`: runs the app with fullscreen, delay, FPS, and party-mode
  flags for manual stress testing.
- `make clean`: removes `obj/`, `main`, and generated test binaries.

Install SDL3 and SDL3_ttf development packages so the `pkg-config` calls in the
`Makefile` resolve before building.

## Coding Style & Naming Conventions

Use C style consistent with the existing code: 4-space indentation and braces on
function definitions.
Headers use include guards like `GRID_H`. Public module APIs currently use
camelCase names such as `makeModelGrid()` and `destroyRenderContext()`, while
types use snake_case with `_t`, such as `grid_t` and `render_context_t`.

Keep module-local details in `.c` files; expose only needed declarations through
matching headers. Prefer `critical()` for unrecoverable allocation or SDL
initialization failures.

## Testing Guidelines

There are no checked-in tests yet, but the `Makefile` expects one test file per
module named `src/<module>/<module>_test.c`. For example, grid tests should live
in `src/grid/grid_test.c` and will be run by `make test` as `test-grid`.

Tests should be deterministic and suitable for Valgrind. Cover allocation,
ownership, and bounds-sensitive behavior where practical. Run `make test` before
submitting changes; run `make test-run` when touching runtime or SDL-facing code.

## Commit & Pull Request Guidelines

The current history only contains `Initial commit`, so no detailed convention is
established. Use short imperative commit subjects, for example
`Add grid copy tests` or `Fix renderer cleanup order`.

Pull requests should include a brief summary, testing performed, and any manual
runtime checks. Link related issues when available. Include screenshots or short
recordings for visible SDL/rendering changes.

## Agent-Specific Instructions

Before editing, check for uncommitted user work and avoid reverting unrelated
changes. Keep generated outputs out of commits, and prefer scoped changes that
match the existing module layout.
