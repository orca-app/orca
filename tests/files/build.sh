#!/bin/bash

LIBDIR=../../build/bin
SRCDIR=../../src

INCLUDES="-I$SRCDIR"
FLAGS="-DOC_DEBUG -DLOG_COMPILE_DEBUG"

if [ ! \( -e bin \) ] ; then
	mkdir ./bin
fi

clang -g $FLAGS $INCLUDES -o ./bin/test_files main.c
