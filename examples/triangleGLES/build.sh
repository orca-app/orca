#!/bin/bash

BINDIR=../../bin
RESDIR=../../resources
SRCDIR=../../src
EXTDIR=../../ext

INCLUDES="-I$SRCDIR -I$SRCDIR/util -I$SRCDIR/platform -I$SRCDIR/app -I$EXTDIR"
LIBS="-L$BINDIR -lmilepost"
FLAGS="-mmacos-version-min=10.15.4 -DDEBUG -DLOG_COMPILE_DEBUG"

clang -g $FLAGS $LIBS $INCLUDES -o $BINDIR/example_gles_triangle main.c
