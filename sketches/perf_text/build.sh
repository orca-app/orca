#!/bin/bash

BINDIR=../../build/bin
SRCDIR=../../src
EXTDIR=../../ext

INCLUDES="-I$SRCDIR -I$EXTDIR"
LIBS="-L$BINDIR -lmilepost"
FLAGS="-O2 -mmacos-version-min=10.15.4"

clang -g $FLAGS $LIBS $INCLUDES -o $BINDIR/perf_text main.c

install_name_tool -add_rpath "@executable_path" $BINDIR/perf_text
