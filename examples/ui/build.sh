#!/bin/bash

BINDIR=../../bin
RESDIR=../../resources
SRCDIR=../../src

INCLUDES="-I$SRCDIR -I$SRCDIR/util -I$SRCDIR/platform -I$SRCDIR/app"
LIBS="-L$BINDIR -lmilepost"
FLAGS="-mmacos-version-min=10.15.4 -DDEBUG -DLOG_COMPILE_DEBUG"

clang -g $FLAGS $LIBS $INCLUDES -o $BINDIR/example_ui main.c

install_name_tool -add_rpath "@executable_path" $BINDIR/example_ui
