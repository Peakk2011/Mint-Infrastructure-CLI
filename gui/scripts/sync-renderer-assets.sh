#!/bin/bash
set -e

SOURCE_DIR="${1:-src/renderer}"
TARGET_DIR="${2:-dist/renderer}"

if [ ! -d "$SOURCE_DIR" ]; then
    echo "Error: Renderer source folder not found at '$SOURCE_DIR'" >&2
    exit 1
fi

mkdir -p "$TARGET_DIR"

cp -rf "$SOURCE_DIR/." "$TARGET_DIR/"

echo "Synced renderer assets to $TARGET_DIR"