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

ORCA_DIR=../..
STDLIB_DIR=$ORCA_DIR/src/libc-shim

wasmFlags="--target=wasm32 \
  --no-standard-libraries \
  -mbulk-memory \
  -g -O2 \
  -D__ORCA__ \
  -Wl,--no-entry \
  -Wl,--export-dynamic \
  -isystem $STDLIB_DIR/include \
  -I $ORCA_DIR/src \
  -I $ORCA_DIR/src/ext"

$CLANG $wasmFlags -o ./module.wasm $ORCA_DIR/src/orca.c $STDLIB_DIR/src/*.c src/main.c

orca bundle --orca-dir $ORCA_DIR --name Clock --icon icon.png --resource-dir data module.wasm
