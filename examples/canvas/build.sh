#!/bin/bash

BINDIR=../../bin
RESDIR=../../resources
SRCDIR=../../src

INCLUDES="-I$SRCDIR -I$SRCDIR/util -I$SRCDIR/platform -I$SRCDIR/app"
LIBS="-L$BINDIR -lmilepost -framework Carbon -framework Cocoa -framework Metal -framework QuartzCore -L$BINDIR -lEGL -lGLESv2"
FLAGS="-mmacos-version-min=10.15.4 -DDEBUG -DLOG_COMPILE_DEBUG -Wl,-dead_strip"

clang -g $FLAGS $LIBS $INCLUDES -o $BINDIR/example_canvas main.c

# change dynamic libraries install name
#TODO: this shouldn't be needed for apps using only metal backend...
install_name_tool -change "./libEGL.dylib" "@loader_path/libEGL.dylib" $BINDIR/example_canvas
install_name_tool -change "./libGLESv2.dylib" "@loader_path/libGLESv2.dylib" $BINDIR/example_canvas
