#!/bin/bash

BINDIR=bin
LIBDIR=../../zig-out/bin
RESDIR=../resources
SRCDIR=../../src

INCLUDES="-I$SRCDIR -I$SRCDIR/util -I$SRCDIR/platform -I$SRCDIR/app"
LIBS="-L$LIBDIR -lorca_platform"
FLAGS="-mmacos-version-min=10.15.4 -DOC_DEBUG -DLOG_COMPILE_DEBUG"

mkdir -p $BINDIR
clang -g $FLAGS $LIBS $INCLUDES -o $BINDIR/code_canvas main.c

cp $LIBDIR/liborca_platform.dylib $BINDIR/
cp $LIBDIR/libwebgpu.dylib $BINDIR/

install_name_tool -add_rpath "@executable_path" $BINDIR/code_canvas
