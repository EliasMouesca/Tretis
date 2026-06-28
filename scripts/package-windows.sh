#!/usr/bin/env sh
set -eu

APP_NAME="Tretis"
TARGET="tretis.exe"
STAGE_DIR="dist/tretis-windows"
INSTALLER="dist/TretisSetup.exe"

copy_dependencies() {
    exe="$1"
    dest="$2"
    changed=1

    while [ "$changed" -eq 1 ]; do
        changed=0

        for binary in "$dest"/*.exe "$dest"/*.dll; do
            [ -e "$binary" ] || continue

            ldd "$binary" | awk '
                /=>/ && $3 ~ /^\// { print $3 }
                /^[[:space:]]*\// { print $1 }
            ' | while IFS= read -r dll; do
                case "$dll" in
                    /c/Windows/*|/C/Windows/*|/windows/*|/Windows/*)
                        continue
                        ;;
                esac

                [ -f "$dll" ] || continue
                name=$(basename "$dll")
                [ ! -f "$dest/$name" ] || continue

                cp "$dll" "$dest/$name"
                printf 'Copied %s\n' "$name"
                touch "$dest/.deps-changed"
            done
        done

        if [ -f "$dest/.deps-changed" ]; then
            rm "$dest/.deps-changed"
            changed=1
        fi
    done
}

case "$(uname -s)" in
    MINGW*|MSYS*)
        ;;
    *)
        printf '%s\n' "This packager must run from an MSYS2 MinGW shell on Windows." >&2
        exit 1
        ;;
esac

make -f Makefile.windows

rm -rf "$STAGE_DIR"
mkdir -p "$STAGE_DIR/fonts"

cp "$TARGET" "$STAGE_DIR/"
cp fonts/SpaceMono-Regular.ttf "$STAGE_DIR/fonts/"
copy_dependencies "$STAGE_DIR/$TARGET" "$STAGE_DIR"

printf '%s\n' "Portable bundle written to $STAGE_DIR"

if command -v makensis >/dev/null 2>&1; then
    mkdir -p dist
    makensis -DAPP_NAME="$APP_NAME" installer/tretis.nsi
    printf '%s\n' "Installer written to $INSTALLER"
else
    printf '%s\n' "makensis not found; install mingw-w64-ucrt-x86_64-nsis to build $INSTALLER."
fi
