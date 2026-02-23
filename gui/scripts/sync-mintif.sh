#!/bin/bash
set -e

SOURCE_ROOT="${1:-../dist}"
TARGET_ROOT="${2:-vendor/mintif}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GUI_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

resolve_path() {
    local base="$1"
    local candidate="$2"

    if [[ "$candidate" = /* ]]; then
        echo "$candidate"
    else
        echo "$base/$candidate"
    fi
}

RESOLVED_SOURCE="$(resolve_path "$GUI_ROOT" "$SOURCE_ROOT")"
RESOLVED_TARGET="$(resolve_path "$GUI_ROOT" "$TARGET_ROOT")"

SOURCE_EXE="$RESOLVED_SOURCE/mintif"         # who crazy ran .exe on macOS
SOURCE_CSS="$RESOLVED_SOURCE/styles.css"

if [ ! -f "$SOURCE_EXE" ]; then
    echo "Error: mintif not found at '$SOURCE_EXE'" >&2
    exit 1
fi

if [ ! -f "$SOURCE_CSS" ]; then
    echo "Error: styles.css not found at '$SOURCE_CSS'" >&2
    exit 1
fi

mkdir -p "$RESOLVED_TARGET"

cp -f "$SOURCE_EXE" "$RESOLVED_TARGET/mintif"
cp -f "$SOURCE_CSS" "$RESOLVED_TARGET/styles.css"

echo "Synced mintif and styles.css to $RESOLVED_TARGET"