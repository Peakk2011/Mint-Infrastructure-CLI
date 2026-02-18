@echo off
echo Compiling Mint Infrastructure...
gcc mintif.c -o mintif.exe -std=c99 -O2

if %ERRORLEVEL% == 0 (
    echo.
    echo   ok  mintif.exe ready!
    echo   Usage: mintif input.md -o output.html
) else (
    echo.
    echo   x   Build failed.
)