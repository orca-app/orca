@echo off

:: compile wasm module
set wasmFlags=--target=wasm32^
       --no-standard-libraries ^
       -fno-builtin ^
       -Wl,--no-entry ^
       -Wl,--export-all ^
       -Wl,--allow-undefined ^
       -g ^
       -D__ORCA__ ^
       -I ..\.. -I ..\..\src -I ..\..\sdk -I..\..\milepost -I ..\..\milepost\src

clang %wasmFlags% -o .\module.wasm ..\..\sdk\orca.c src\main.c


python3 ..\..\scripts\mkapp.py --orca-dir ../.. --name Pong --icon icon.png --data-dir dir1 module.wasm
