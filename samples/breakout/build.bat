@echo off

set ORCA_DIR=..\..
set STDLIB_DIR=%ORCA_DIR%\src\libc-shim

:: compile wasm module
set wasmFlags=--target=wasm32^
       --no-standard-libraries ^
       -mbulk-memory ^
       -g -O2 ^
       -D__ORCA__ ^
       -Wl,--no-entry ^
       -Wl,--export-dynamic ^
       -isystem %STDLIB_DIR%\include ^
       -I%ORCA_DIR%\src ^
       -I%ORCA_DIR%\src\ext

clang %wasmFlags% -o .\module.wasm %ORCA_DIR%\src\orca.c %STDLIB_DIR%\src\*.c src\main.c
IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

orca bundle --orca-dir %ORCA_DIR% --name Breakout --icon icon.png --resource-dir data module.wasm
