#!/bin/bash

BINDIR=../../build/bin
SRCDIR=../../src
EXTDIR=../../ext

INCLUDES="-I$SRCDIR -I$EXTDIR"
LIBS="-L$BINDIR -lmilepost"
FLAGS="-mmacos-version-min=10.15.4 -DDEBUG -DLOG_COMPILE_DEBUG"

clang -g $FLAGS $LIBS $INCLUDES -o $BINDIR/example_tiger main.c

install_name_tool -add_rpath "@executable_path" $BINDIR/example_tiger
