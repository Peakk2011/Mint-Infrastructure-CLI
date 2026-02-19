@echo off
echo Compiling Mint Infrastructure...

if not exist dist (
    mkdir dist
)

gcc main.c ^
    src/buf/buf.c ^
    src/io/io.c ^
    src/parser/inline.c ^
    src/parser/parser.c ^
    src/path/path.c ^
    src/html/html.c ^
    -o dist\mintif.exe -std=c99 -O2 -I.

if %ERRORLEVEL% == 0 (
    copy /Y styles.css dist\styles.css >nul
    echo.
    echo   ok  dist\mintif.exe ready!
    echo   ok  dist\styles.css copied.
    echo   Usage: dist\mintif.exe input.md -o output.html
) else (
    echo.
    echo   x   Build failed.
)
