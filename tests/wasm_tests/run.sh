#!/bin/bash

set -e

ORCA_DIR=../..

wasmFlags="--target=wasm32 \
  -mbulk-memory \
  -g -O2 \
  -D__ORCA__ \
  -Wl,--no-entry \
  -Wl,--export-dynamic \
  --sysroot $ORCA_DIR/build/orca-libc \
  -I $ORCA_DIR/src \
  -I $ORCA_DIR/src/ext"

if [ ! \( -e bin \) ] ; then
	mkdir ./bin
fi

clang $wasmFlags -L $ORCA_DIR/build/bin -lorca_wasm -o files.wasm files.c

orca bundle --orca-dir $ORCA_DIR --name Tests --resource-dir data files.wasm

./Tests.app/Contents/macOS/orca_runtime --test=files.wasm
