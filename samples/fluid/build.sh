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

python3 ../../scripts/embed_text_files.py --prefix=glsl_ --output src/glsl_shaders.h src/shaders/*.glsl

# common flags to build wasm modules
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

# build orca core as wasm module
clang $wasmFlags -Wl,--relocatable -o ./liborca.a $ORCA_DIR/src/orca.c $STDLIB_DIR/src/*.c

# build sample as wasm module and link it with the orca module
clang $wasmFlags -L . -lorca -o module.wasm src/main.c

# create app directory and copy files into it
orca bundle --orca-dir $ORCA_DIR --name Fluid --icon icon.png module.wasm
