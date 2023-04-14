#!/bin/bash

wasmFlags="--target=wasm32 \
       --no-standard-libraries \
       -fno-builtin \
       -Wl,--no-entry \
       -Wl,--export-all \
       -Wl,--allow-undefined \
       -g \
       -D__ORCA__ \
       -I ../../sdk -I ../../milepost/src"

/usr/local/opt/llvm/bin/clang $wasmFlags -o ./module.wasm ../../sdk/graphics.c ../../sdk/orca.c main.c

#python3 ../../scripts/mkapp.py --orca-dir ../.. --name Pong --icon icon.png module.wasm
