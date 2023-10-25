#!/bin/zsh

#cfiles=(src/{complex,ctype,errno,fenv,math,prng,search,stdio,stdlib,string}/*.c)

cfiles=(src/{complex,ctype,errno,fenv,math,prng,string,stdlib,stdio,internal}/*.c)

warnings=(-Wall -Wextra -Werror -Wno-null-pointer-arithmetic -Wno-unused-parameter -Wno-sign-compare -Wno-unused-variable -Wno-unused-function -Wno-ignored-attributes -Wno-missing-braces -Wno-ignored-pragmas -Wno-unused-but-set-variable -Wno-unknown-warning-option -Wno-parentheses -Wno-shift-op-parentheses -Wno-bitwise-op-parentheses -Wno-logical-op-parentheses -Wno-string-plus-int -Wno-dangling-else -Wno-unknown-pragmas)

includes=(-isystem ./include -Iarch -Isrc/internal)

# compile dummy CRT
/usr/local/opt/llvm/bin/clang -O2 -DNDEBUG --target=wasm32 --no-standard-libraries -fno-trapping-math -mbulk-memory -DBULK_MEMORY_THRESHOLD=32 -mthread-model single -MD -MP -Wl,--relocatable $warnings $includes -o lib/crt1.o src/crt/crt1.c

# compile standard lib
/usr/local/opt/llvm/bin/clang -O2 -DNDEBUG --target=wasm32 --std=c11 --no-standard-libraries -fno-trapping-math -mbulk-memory -DBULK_MEMORY_THRESHOLD=32 -mthread-model single -MD -MP -Wl,--relocatable $warnings $includes -o lib/libc.o $cfiles

/usr/local/opt/llvm/bin/llvm-ar crs lib/libc.a lib/libc.o

# compile test
/usr/local/opt/llvm/bin/clang -O2 --target=wasm32 --sysroot=. -Wl,--no-entry -o test test.c
