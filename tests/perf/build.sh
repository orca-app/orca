#!/bin/bash

BINDIR=bin
LIBDIR=../../build/bin
SRCDIR=../../src

INCLUDES="-I$SRCDIR"
LIBS="-L$LIBDIR -lorca"
FLAGS="-mmacos-version-min=13.0.0 -DOC_DEBUG -DLOG_COMPILE_DEBUG"

mkdir -p $BINDIR

clang -g -O3 $FLAGS $LIBS $INCLUDES -o $BINDIR/driver driver.c
install_name_tool -add_rpath "@executable_path" $BINDIR/driver

cp Info.plist $BINDIR/
cp $LIBDIR/liborca.dylib $BINDIR/
cp $LIBDIR/libwebgpu.dylib $BINDIR/
