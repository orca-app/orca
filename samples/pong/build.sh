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
       -D__ORCA__ \
	   -isystem ../../cstdlib/include -I ../../sdk -I../../milepost/ext -I ../../milepost -I ../../milepost/src -I ../../milepost/src/util -I ../../milepost/src/platform -I../.."

$CLANG $wasmFlags -o ./module.wasm ../../cstdlib/src/*.c ../../sdk/orca.c src/main.c

python3 ../../scripts/mkapp.py --orca-dir ../.. --name Pong --icon icon.png --data-file data/ball.png module.wasm
