#!/bin/bash

BINDIR=../../bin
RESDIR=../../resources
SRCDIR=../../src

INCLUDES="-I$SRCDIR -I$SRCDIR/util -I$SRCDIR/platform -I$SRCDIR/app"
LIBS="-L$BINDIR -lmilepost -framework Foundation -framework Metal"
FLAGS="-mmacos-version-min=10.15.4 -DDEBUG -DLOG_COMPILE_DEBUG"

xcrun -sdk macosx metal -c -o shader.air shader.metal
xcrun -sdk macosx metallib -o shader.metallib shader.air
cp shader.metallib $BINDIR/triangle_shader.metallib

clang -g $FLAGS $LIBS $INCLUDES -o $BINDIR/example_metal_triangle main.m

install_name_tool -add_rpath "@executable_path" $BINDIR/example_metal_triangle
