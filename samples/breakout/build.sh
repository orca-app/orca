#!/bin/bash

set -euo pipefail

# The following code checks if you have the necessary programs to compile the samples.
# This code exists to improve the experience of first-time Orca users and can
# be safely deleted in your own projects if you wish.
if [ -f ../../scripts/sample_build_check.py ]; then
  python3 ../../scripts/sample_build_check.py
else
  echo "Could not check if you have the necessary tools to build the Orca samples."
  echo "If you have copied this script to your own project, you can delete this code."
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

clang $wasmFlags -o ./module.wasm $ORCA_DIR/src/orca.c $STDLIB_DIR/src/*.c src/main.c

orca bundle --orca-dir $ORCA_DIR --name Breakout --icon icon.png --resource-dir data module.wasm
