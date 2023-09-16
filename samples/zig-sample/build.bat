@echo off

:: compile wasm module
rem set wasmFlags=--target=wasm32^
rem        --no-standard-libraries ^
rem        -fno-builtin ^
rem        -Wl,--no-entry ^
rem        -Wl,--export-dynamic ^
rem        -Wl,--relocatable ^
rem        -g ^
rem        -O2 ^
rem        -mbulk-memory ^
rem        -D__ORCA__ ^
rem        -isystem ..\..\src\libc-shim\include ^
rem        -I..\..\ext -I ..\..\src

set ORCA_DIR=..\..
set STDLIB_DIR=%ORCA_DIR%\src\libc-shim

set wasmFlags=--target=wasm32^
       --no-standard-libraries ^
       -mbulk-memory ^
       -g -O2 ^
       -D__ORCA__ ^
       -Wl,--no-entry ^
       -Wl,--export-dynamic ^
       -Wl,--relocatable ^
       -isystem %STDLIB_DIR%\include ^
       -I%ORCA_DIR%\src ^
       -I%ORCA_DIR%\src\ext

clang %wasmFlags% -o .\liborca.a ..\..\src\orca.c ..\..\src\libc-shim\src\*.c
IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

zig build bundle
