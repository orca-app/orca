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
       -isystem ..\..\cstdlib\include -I ..\..\sdk -I..\..\milepost\ext -I ..\..\milepost -I ..\..\milepost\src

clang %wasmFlags% -o .\module.wasm ..\..\sdk\orca.c ..\..\cstdlib\src\*.c src\main.c
IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

orca bundle --orca-dir ..\.. --name Pong --icon icon.png --resource-dir data module.wasm
