#!/usr/bin/env sh
set -eu

mkdir -p dist

cc tests/parser_tests.c \
  src/buf/buf.c \
  src/io/io.c \
  src/parser/inline.c \
  src/parser/inline_html.c \
  src/parser/parser.c \
  src/parser/parser_line.c \
  src/parser/parser_html.c \
  src/path/path.c \
  src/html/html.c \
  -o dist/parser_tests -std=c99 -O2 -I.

./dist/parser_tests