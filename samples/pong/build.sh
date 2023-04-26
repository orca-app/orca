#!/bin/bash

wasmFlags="--target=wasm32 \
       --no-standard-libraries \
       -fno-builtin \
       -Wl,--no-entry \
       -Wl,--export-all \
       -Wl,--allow-undefined \
       -g \
       -D__ORCA__ \
       -I ../../sdk -I../../milepost/ext -I ../../milepost -I ../../milepost/src -I ../../milepost/src/util -I ../../milepost/src/platform -I../.."

/usr/local/opt/llvm/bin/clang $wasmFlags -o ./module.wasm ../../sdk/orca.c src/main.c

python3 ../../scripts/mkapp.py --orca-dir ../.. --name Pong --icon icon.png module.wasm
