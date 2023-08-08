#!/bin/bash

set -euo pipefail

if [[ -x /usr/local/opt/llvm/bin/clang ]]; then
  CLANG=/usr/local/opt/llvm/bin/clang
elif [[ -x /opt/homebrew/opt/llvm/bin/clang ]]; then
  CLANG=/opt/homebrew/opt/llvm/bin/clang
else
  echo "Could not find Homebrew clang; this script will probably not work."
  CLANG=clang
fi

STDLIB_DIR=../../cstdlib
ORCA_SDK_DIR=../../sdk
MILEPOST_DIR=../../milepost

wasmFlags="--target=wasm32 \
  --no-standard-libraries \
  -fno-builtin \
  -Wl,--no-entry \
  -Wl,--export-dynamic \
  -g \
  -O2 \
  -mbulk-memory \
  -D__ORCA__ \
  -I $STDLIB_DIR/include \
  -I $ORCA_SDK_DIR \
  -I $MILEPOST_DIR/ext -I $MILEPOST_DIR -I $MILEPOST_DIR/src"

$CLANG $wasmFlags -o ./module.wasm ../../sdk/orca.c ../../cstdlib/src/*.c src/main.c

orca bundle --orca-dir ../.. --name Pong --icon icon.png --resource-dir data module.wasm
