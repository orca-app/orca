#!/bin/bash

set -euxo pipefail

EXTRA_CFLAGS="${EXTRA_CFLAGS:-}"

CC='gcc'
ORCA_DIR="$PWD/../.."

CFLAGS='-x c -std=gnu11 -Wall -Wextra -Werror -Wshadow'
CFLAGS+=' -Wdouble-promotion -Wformat=2'
CFLAGS+=' -Wformat-overflow -Wformat-truncation'
CFLAGS+=' -Wundef -fno-common -Wconversion -gdwarf -masm=intel'
CFLAGS+=' -fno-omit-frame-pointer -march=native'
CFLAGS+=' -O0 -g3 -DDEBUG=1'
#CFLAGS+=' -fsanitize=address,undefined'
CFLAGS+=" -I$ORCA_DIR/src -I$ORCA_DIR/src/ext -I$ORCA_DIR/src/ext/angle/include"
CFLAGS+=" $EXTRA_CFLAGS"

LDFLAGS="-L$ORCA_DIR/build/bin -lorca"
LDFLAGS+=" -Wl,-rpath=$ORCA_DIR/build/bin"
LDFLAGS+=" -Wl,-rpath-link=$ORCA_DIR/build/bin"

BUILD_DIR='/tmp'
EXE="$BUILD_DIR/main"

mkdir -p "$BUILD_DIR"
"$CC" $CFLAGS -o "$EXE" main.c $LDFLAGS
