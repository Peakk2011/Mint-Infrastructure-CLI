#!/usr/bin/env bash
set -euo pipefail
echo "Compiling Mint Infrastructure..."
mkdir -p dist

CC_BIN=""
if command -v clang >/dev/null 2>&1; then
    CC_BIN="clang"
elif command -v gcc >/dev/null 2>&1; then
    CC_BIN="gcc"
else
    echo "No C compiler found (clang/gcc)."
    exit 1
fi

"$CC_BIN" main.c \
    src/buf/buf.c \
    src/io/io.c \
    src/parser/inline.c \
    src/parser/inline_html.c \
    src/parser/parser.c \
    src/parser/parser_line.c \
    src/parser/parser_html.c \
    src/path/path.c \
    src/html/html.c \
    -o dist/mintif -std=c99 -O2 -I.

cp styles.css dist/styles.css

echo
echo "  ok  dist/mintif ready!"
echo "  ok  dist/styles.css copied."
echo "  Usage: ./dist/mintif input.md -o output.html"