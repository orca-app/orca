@echo off
setlocal enabledelayedexpansion

for /f %%i in ('orca sdk-path') do set ORCA_DIR=%%i

:: common flags to build wasm modules
set wasmFlags=--target=wasm32^
       -mbulk-memory ^
       -g -O2 ^
       -Wl,--no-entry ^
       -Wl,--export-dynamic ^
       --sysroot %ORCA_DIR%/orca-libc ^
       -I%ORCA_DIR%/src ^
       -I%ORCA_DIR%/src/ext

:: build sample as wasm module and link it with the orca module
clang %wasmFlags% -L %ORCA_DIR%/bin -lorca_wasm -o module.wasm src/main.c
IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

:: create app directory and copy files into it
orca bundle --name Breakout --icon icon.png --resource-dir data module.wasm
