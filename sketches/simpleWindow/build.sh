#!/bin/bash

BINDIR=../../bin
RESDIR=../../resources
SRCDIR=../../src
EXTDIR=../../ext

INCLUDES="-I$SRCDIR -I$SRCDIR/util -I$SRCDIR/platform -I$SRCDIR/app"
LIBS="-L$BINDIR -lmilepost -framework Carbon -framework Cocoa -framework Metal -framework QuartzCore"
FLAGS="-mmacos-version-min=10.15.4 -DOC_DEBUG -DLOG_COMPILE_DEBUG"

clang -g $FLAGS -Wl,-dead_strip $LIBS $INCLUDES -o $BINDIR/example_simple_window main.c

install_name_tool -add_rpath "@executable_path" $BINDIR/example_simple_window
