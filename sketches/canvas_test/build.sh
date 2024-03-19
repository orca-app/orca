#!/bin/bash

BINDIR=bin
LIBDIR=../../build/bin
RESDIR=../resources
SRCDIR=../../src

INCLUDES="-I$SRCDIR"
LIBS="-L$LIBDIR -lorca"
FLAGS="-mmacos-version-min=13.0.0 -DOC_DEBUG -DLOG_COMPILE_DEBUG"

mkdir -p $BINDIR
clang -g $FLAGS $LIBS $INCLUDES -o $BINDIR/example_canvas main.c

cp $LIBDIR/liborca.dylib $BINDIR/
cp $LIBDIR/libwebgpu.dylib $BINDIR/

install_name_tool -add_rpath "@executable_path" $BINDIR/example_canvas
