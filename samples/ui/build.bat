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
       -isystem ..\..\cstdlib\include -I ..\..\sdk -I..\..\milepost\ext -I ..\..\milepost -I ..\..\milepost\src -I ..\..\milepost\src\util -I ..\..\milepost\src\platform

clang %wasmFlags% -o .\module.wasm ..\..\sdk\orca.c ..\..\cstdlib\src\*.c src\main.c

python3 ..\..\scripts\mkapp.py --orca-dir ..\.. --name UI --resource-dir data module.wasm
