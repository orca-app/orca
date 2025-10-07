#!/bin/zsh

set -euo pipefail

ORCA_DIR=$(orca sdk-path)

# common flags to build wasm modules
wasmFlags=(--target=wasm32 \
  -mbulk-memory \
  -g -O2 \
  -Wl,--no-entry \
  -Wl,--export-dynamic \
  --sysroot "$ORCA_DIR"/orca-libc \
  -I "$ORCA_DIR"/src \
  -I "$ORCA_DIR"/src/ext)

# build sample as wasm module and link it with the orca module
clang "${wasmFlags[@]}" -L "$ORCA_DIR"/lib -lorca_wasm -o main.wasm src/main.c

# create app directory and copy files into it
orca bundle --name Clock --icon icon.png --resource-dir data main.wasm
