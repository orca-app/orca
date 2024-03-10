#!/bin/zsh

set -euo pipefail

ORCA_DIR=$(orca sdk-path)
STDLIB_DIR=$ORCA_DIR/src/libc-shim

# common flags to build wasm modules
wasmFlags=(--target=wasm32 \
  --no-standard-libraries \
  -mbulk-memory \
  -g -O2 \
  -D__ORCA__ \
  -Wl,--no-entry \
  -Wl,--export-dynamic \
  -isystem "$STDLIB_DIR"/include \
  -I "$ORCA_DIR"/src \
  -I "$ORCA_DIR"/src/ext)

# build sample as wasm module and link it with the orca module
clang "${wasmFlags[@]}" -L "$ORCA_DIR"/bin -lorca_wasm -o module.wasm src/main.c

# create app directory and copy files into it
orca bundle --name Breakout --icon icon.png --resource-dir data module.wasm
