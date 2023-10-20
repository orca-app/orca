#!/bin/zsh

#cfiles=(src/{complex,ctype,errno,fenv,math,prng,search,stdio,stdlib,string}/*.c)

cfiles=(src/{complex,ctype,errno,fenv,math,prng,string,stdlib}/*.c)

warnings=(-Wall -Wextra -Werror -Wno-null-pointer-arithmetic -Wno-unused-parameter -Wno-sign-compare -Wno-unused-variable -Wno-unused-function -Wno-ignored-attributes -Wno-missing-braces -Wno-ignored-pragmas -Wno-unused-but-set-variable -Wno-unknown-warning-option -Wno-parentheses -Wno-shift-op-parentheses -Wno-bitwise-op-parentheses -Wno-logical-op-parentheses -Wno-string-plus-int -Wno-dangling-else -Wno-unknown-pragmas)

includes=(-isystem ./include -Iarch -Isrc/internal)

/usr/local/opt/llvm/bin/clang -O2 -DNDEBUG --target=wasm32 --no-standard-libraries -fno-trapping-math -mbulk-memory -DBULK_MEMORY_THRESHOLD=32 -mthread-model single -MD -MP -Wl,--relocatable $warnings $includes -o stdlib.o $cfiles
