#!/bin/bash

BINDIR=../../bin
RESDIR=../../resources
SRCDIR=../../src

INCLUDES="-I$SRCDIR -I$SRCDIR/util -I$SRCDIR/platform -I$SRCDIR/app -I$SRCDIR/../ext/angle_headers"
LIBS="-L$BINDIR -lmilepost -framework Carbon -framework Cocoa -framework Metal -framework QuartzCore -lGLESv2 -lEGL"
FLAGS="-mmacos-version-min=10.15.4 -DDEBUG -DLOG_COMPILE_DEBUG"

clang -g $FLAGS $LIBS $INCLUDES -o $BINDIR/example_gles_triangle main.c

# change dynamic libraries install name
install_name_tool -change "./libEGL.dylib" "@loader_path/libEGL.dylib" $BINDIR/example_gles_triangle
install_name_tool -change "./libGLESv2.dylib" "@loader_path/libGLESv2.dylib" $BINDIR/example_gles_triangle
