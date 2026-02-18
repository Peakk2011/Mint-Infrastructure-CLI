@echo off
echo Compiling Mint Infrastructure...

gcc main.c ^
    src/buf/buf.c ^
    src/io/io.c ^
    src/parser/inline.c ^
    src/parser/parser.c ^
    src/path/path.c ^
    src/html/html.c ^
    -o mintif.exe -std=c99 -O2 -I.

if %ERRORLEVEL% == 0 (
    echo.
    echo   ok  mintif.exe ready!
    echo   Usage: mintif input.md -o output.html
) else (
    echo.
    echo   x   Build failed.
)