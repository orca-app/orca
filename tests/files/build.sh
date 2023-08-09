#!/bin/bash

LIBDIR=../../bin
RESDIR=../../resources
SRCDIR=../../milepost/src

INCLUDES="-I$SRCDIR -I$SRCDIR/util -I$SRCDIR/platform -I$SRCDIR/app"
LIBS="-L$LIBDIR -lmilepost"
FLAGS="-mmacos-version-min=10.15.4 -DDEBUG -DLOG_COMPILE_DEBUG"

if [ ! \( -e bin \) ] ; then
	mkdir ./bin
fi


clang -g $FLAGS $LIBS $INCLUDES -o ./bin/test_files main.c
install_name_tool -add_rpath "@executable_path/../../../bin" ./bin/test_files
