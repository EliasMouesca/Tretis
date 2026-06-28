# Tretis

Tretis is a small SDL3 Tetris-like game written in C. It is intentionally quiet:
no sound, no flashy animation, and no busy effects. The goal is a low-friction
game that can sit beside your thinking, not demand all of your attention.

## Install on Arch Linux

Clone the repo and install it with `makepkg`:

```sh
git clone https://github.com/EliasMouesca/Tretis.git
cd Tretis
makepkg -si
```

The package installs the `tretis` binary and the bundled default font. Runtime
stats and preferences are stored under:

```text
$HOME/.config/tretis/
```

## Build Manually

Install SDL3 and SDL3_ttf development packages, then run:

```sh
make
./main
```

Optional install target:

```sh
sudo make install
```

## Controls

- Left/Right or `A`/`D`: move
- Down or `S`: soft drop
- Up or `W`: rotate
- Space: hard drop
- `E`: hold/swap current piece
- `P` or Esc: pause
- `R`: restart
- `Q`: quit

Key bindings can also be edited in `$HOME/.config/tretis/config`.

## Command-Line Options

- `--help`: show all options and controls.
- `--hud` / `--no-hud`: show or hide the right HUD panel.
- `--zen` / `--no-zen`: hide or show score-heavy HUD details.
- `--ghost` / `--no-ghost`: show or hide the landing shadow.
- `--next N`: number of upcoming pieces to show, from `0` to `6`.
- `--block-size N`: pixel size of each board block.
- `--fall-delay N`: base milliseconds between automatic falls.
- `--speedup` / `--no-speedup`: enable or disable gradual speed increase.
- `--speedup-every N`: seconds between speed increases.
- `--speedup-step N`: milliseconds removed at each speed increase.
- `--min-fall-delay N`: fastest automatic fall delay.
- `--randomizer NAME`: piece generator. Options are `7bag`, `uniform`, `tgm`,
  and `35bag`.
- `--sidebar-width N`: width of the right HUD panel.
- `--stats`: print saved stats and exit.
- `--stats-file PATH`: custom stats file path.
- `--highscore-file PATH`: alias for `--stats-file`.
- `--font PATH`: custom TTF font file.
- `--font-size N`: HUD font size in pixels.

Command-line options override saved preferences for that run. Preferences are
loaded from and saved to `$HOME/.config/tretis/config`.

## Development

```sh
make
make test
make clean
```

Generated files such as `main`, `obj/`, package outputs, and runtime config are
ignored by Git.
