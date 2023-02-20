#!/bin/bash

BINDIR=../../bin
RESDIR=../../resources
SRCDIR=../../src

INCLUDES="-I$SRCDIR -I$SRCDIR/util -I$SRCDIR/platform -I$SRCDIR/app"
LIBS="-L$BINDIR -lmilepost -framework Carbon -framework Cocoa -framework Metal -framework QuartzCore -L$BINDIR -lEGL -lGLESv2"
FLAGS="-mmacos-version-min=10.15.4 -DDEBUG -DLOG_COMPILE_DEBUG -Wl,-dead_strip"

xcrun -sdk macosx metal -c -o shader.air shader.metal
xcrun -sdk macosx metallib -o shader.metallib shader.air
cp shader.metallib $BINDIR/triangle_shader.metallib

clang -g $FLAGS $LIBS $INCLUDES -o $BINDIR/example_metal_triangle main.m

# change dynamic libraries install name
#TODO: this shouldn't be needed for apps using only metal backend...
install_name_tool -change "./libEGL.dylib" "@loader_path/libEGL.dylib" $BINDIR/example_metal_triangle
install_name_tool -change "./libGLESv2.dylib" "@loader_path/libGLESv2.dylib" $BINDIR/example_metal_triangle
