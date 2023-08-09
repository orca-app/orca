
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
       -Wl,--no-entry \
       -Wl,--export-all \
       -Wl,--allow-undefined \
       -g \
       -mbulk-memory"

$CLANG $wasmFlags -o ./module.wasm main.c
wasm2wat module.wasm > module.wat
