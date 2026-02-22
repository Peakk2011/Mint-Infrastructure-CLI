@echo off
setlocal

if not exist dist (
    mkdir dist
)

gcc tests\parser_tests.c ^
    src\buf\buf.c ^
    src\io\io.c ^
    src\parser\inline.c ^
    src\parser\inline_html.c ^
    src\parser\parser.c ^
    src\parser\parser_line.c ^
    src\parser\parser_html.c ^
    src\path\path.c ^
    src\html\html.c ^
    -o dist\parser_tests.exe -std=c99 -O2 -I.
if errorlevel 1 (
    echo parser tests: build failed
    exit /b 1
)

dist\parser_tests.exe
exit /b %ERRORLEVEL%