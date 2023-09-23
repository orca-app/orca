@echo off
setlocal enabledelayedexpansion

:: The following code checks if you have the necessary programs to compile the samples.
:: This code exists to improve the experience of first-time Orca users and can
:: be safely deleted in your own projects if you wish.
if exist "..\..\scripts\sample_build_check.py" (
       python ..\..\scripts\sample_build_check.py
       if !ERRORLEVEL! neq 0 exit /b 1
) else (
       echo Could not check if you have the necessary tools to build the Orca samples.
       echo If you have copied this script to your own project, you can delete this code.
)

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
if !ERRORLEVEL! neq 0 exit /b !ERRORLEVEL!

orca bundle --orca-dir %ORCA_DIR% --name Triangle module.wasm
