#!/bin/bash

BINDIR=bin
LIBDIR=../../build/bin
RESDIR=../resources
SRCDIR=../../src
EXTDIR=../../ext
ANGLEDIR=../../ext/angle/include

INCLUDES="-I$SRCDIR -I$SRCDIR/util -I$SRCDIR/platform -I$SRCDIR/app -I$EXTDIR/ -I$ANGLEDIR/"
LIBS="-L$LIBDIR -lorca"
FLAGS="-mmacos-version-min=10.15.4 -DOC_DEBUG -DLOG_COMPILE_DEBUG"

mkdir -p $BINDIR
clang -g $FLAGS $LIBS $INCLUDES -o $BINDIR/example_smooth_resize main.c

cp $LIBDIR/liborca.dylib $BINDIR/
cp $LIBDIR/mtl_renderer.metallib $BINDIR/

install_name_tool -add_rpath "@executable_path" $BINDIR/example_smooth_resize
