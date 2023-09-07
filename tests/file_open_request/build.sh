#!/bin/bash

LIBDIR=../../build/bin
SRCDIR=../../src

INCLUDES="-I$SRCDIR"
LIBS="-L$LIBDIR -lorca"
FLAGS="-mmacos-version-min=10.15.4 -DOC_DEBUG -DLOG_COMPILE_DEBUG"

if [ ! \( -e bin \) ] ; then
	mkdir ./bin
fi

clang -g $FLAGS $LIBS $INCLUDES -o ./bin/test_open_request main.c

cp $LIBDIR/liborca.dylib ./bin/

install_name_tool -add_rpath "@executable_path" ./bin/test_open_request
