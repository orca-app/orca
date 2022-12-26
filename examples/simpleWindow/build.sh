#!/bin/bash

BINDIR=../../bin
RESDIR=../../resources
SRCDIR=../../src
EXTDIR=../../ext

INCLUDES="-I$SRCDIR -I$SRCDIR/util -I$SRCDIR/platform -I$SRCDIR/app"
LIBS="-L$BINDIR -lmilepost -framework Carbon -framework Cocoa -framework Metal -framework QuartzCore"
FLAGS="-mmacos-version-min=10.15.4 -DDEBUG -DLOG_COMPILE_DEBUG"

clang -g $FLAGS -Wl,-dead_strip $LIBS $INCLUDES -o test main.c
