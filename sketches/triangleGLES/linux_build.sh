#!/bin/bash

set -euxo pipefail

BUILD_DIR='./bin'
ORCA_ROOT='../..'

CFLAGS=""
CFLAGS+=" -I$ORCA_ROOT/src"
CFLAGS+=" -I$ORCA_ROOT/src/util"
CFLAGS+=" -I$ORCA_ROOT/src/platform"
CFLAGS+=" -I$ORCA_ROOT/src/app"
CFLAGS+=" -L$ORCA_ROOT/src/ext"
CFLAGS+=" -L$ORCA_ROOT/build/lib -lorca"
CFLAGS+=" -lc -lm -lpthread"
CFLAGS+=" -lX11 -lX11-xcb -lxcb -lEGL"
CFLAGS+=" -DOC_DEBUG -DOC_LOG_COMPILE_DEBUG"
CFLAGS+=" -O0 -g -gdwarf"
CFLAGS+=" -fuse-ld=lld"

mkdir -p "$BUILD_DIR"
clang $CFLAGS -o "$BUILD_DIR/example_gles_triangle" main.c
