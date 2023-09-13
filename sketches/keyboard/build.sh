#!/bin/bash

BINDIR=bin
LIBDIR=../../build/bin
SRCDIR=../../src

INCLUDES="-I$SRCDIR"
LIBS="-L$LIBDIR -lorca -framework Carbon -framework Cocoa -framework Metal -framework QuartzCore"
FLAGS="-mmacos-version-min=10.15.4 -DOC_DEBUG -DLOG_COMPILE_DEBUG"

mkdir -p $BINDIR
clang -g $FLAGS $LIBS $INCLUDES -o $BINDIR/keyboard main.c

cp $LIBDIR/liborca.dylib $BINDIR/

install_name_tool -add_rpath "@executable_path" $BINDIR/keyboard
