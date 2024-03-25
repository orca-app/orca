#!/bin/bash

BINDIR=bin
LIBDIR=../../build/bin
RESDIR=../resources
SRCDIR=../../src
EXTDIR=../../src/ext
DAWNDIR=../../src/ext/dawn

INCLUDES="-I$SRCDIR -I$EXTDIR -I$DAWNDIR/include"
LIBS="-L$LIBDIR -lorca -L$DAWNDIR/bin -lwebgpu"
FLAGS="-mmacos-version-min=13.0.0 -DOC_DEBUG -DLOG_COMPILE_DEBUG"

mkdir -p $BINDIR
clang -g $FLAGS $LIBS $INCLUDES -o $BINDIR/example_wgpu_triangle main.c

cp $LIBDIR/liborca.dylib $BINDIR/
cp $DAWNDIR/bin/libwebgpu.dylib $BINDIR/

install_name_tool -add_rpath "@executable_path" $BINDIR/example_wgpu_triangle
