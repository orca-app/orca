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

wasmFlags="--target=wasm32 \
  --no-standard-libraries \
  -fno-builtin \
  -Wl,--no-entry \
  -Wl,--export-dynamic \
  -g \
  -O2 \
  -mbulk-memory \
  -D__ORCA__ \
  -isystem ../../cstdlib/include -I ../../sdk -I../../milepost/ext -I ../../milepost -I ../../milepost/src"

$CLANG $wasmFlags -o ./module.wasm ../../sdk/orca.c ../../cstdlib/src/*.c src/main.c

orca bundle --orca-dir ../.. --name UI --resource-dir data module.wasm
