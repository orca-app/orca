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

if [[ -x /usr/local/opt/llvm/bin/clang++ ]]; then
  CLANGPP=/usr/local/opt/llvm/bin/clang++
elif [[ -x /opt/homebrew/opt/llvm/bin/clang++ ]]; then
  CLANGPP=/opt/homebrew/opt/llvm/bin/clang++
else
  echo "Could not find Homebrew clang++; this script will probably not work."
  CLANGPP=clang++
fi

ORCA_DIR=../..
STDLIB_DIR=../../src/libc-shim

wasmObjFlags="--target=wasm32 \
  -g \
  -O2 \
  -mbulk-memory \
  -D__ORCA__ \
  -I $ORCA_DIR/ext \
  -I $STDLIB_DIR/include \
  -I $ORCA_DIR/ext \
  -I $ORCA_DIR/src"

wasmFlags="--target=wasm32 \
  --no-standard-libraries \
  -Wl,--no-entry \
  -Wl,--export-dynamic \
  -g \
  -O2 \
  -mbulk-memory \
  -D__ORCA__ \
  -I $ORCA_DIR/ext \
  -I $STDLIB_DIR/include \
  -I $ORCA_DIR/ext \
  -I $ORCA_DIR/src"

if [ ! -e build ] ; then
    mkdir build
fi

$CLANG $wasmObjFlags -c -o ./build/orca.o ../../src/orca.c
for file in $STDLIB_DIR/src/*.c ; do
    name=$(basename $file)
    name=${name%.c}
    $CLANG $wasmObjFlags -c -o ./build/$name.o $file
done

$CLANGPP $wasmFlags -o ./module.wasm src/main.c ./build/*.o

orca bundle --orca-dir ../.. --name Clock --resource-dir data module.wasm
