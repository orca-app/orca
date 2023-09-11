@echo off

:: compile wasm module
set wasmFlags=--target=wasm32^
       --no-standard-libraries ^
       -fno-builtin ^
       -Wl,--no-entry ^
       -Wl,--export-dynamic ^
       -g ^
       -O2 ^
       -mbulk-memory ^
       -D__ORCA__ ^
       -isystem ..\..\src\libc-shim\include ^
       -I..\..\ext -I ..\..\src

clang %wasmFlags% -o .\liborca.a ..\..\src\orca.c ..\..\src\libc-shim\src\*.c
IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

zig build bundle
