#!/bin/bash

DEBUG_FLAGS="-g -DDEBUG -DLOG_COMPILE_DEBUG"
#DEBUG_FLAGS="-O3"

#--------------------------------------------------------------
# set target
#--------------------------------------------------------------

target="$1"
if [ -z $target ] ; then
	target='lib'
fi

shaderFlagParam="$2"
#--------------------------------------------------------------
# Detect OS and set environment variables accordingly
#--------------------------------------------------------------
OS=$(uname -s)

if [ $OS = "Darwin" ] ; then
	#echo "Target '$target' for macOS"
	CC=clang
	CXX=clang++
	DYLIB_SUFFIX='dylib'
	SYS_LIBS=''
	FLAGS="-mmacos-version-min=10.15.4 -maes"
	CFLAGS="-std=c11"

elif [ $OS = "Linux" ] ; then
	echo "Error: Linux is not supported yet"
	exit -1
else
	echo "Error: Unsupported OS $OS"
	exit -1
fi

#--------------------------------------------------------------
# Set paths
#--------------------------------------------------------------
BINDIR="./bin"
SRCDIR="./src"
EXTDIR="./ext"
RESDIR="./resources"
INCLUDES="-I$SRCDIR -I$SRCDIR/util -I$SRCDIR/platform -I$EXTDIR -I$EXTDIR/angle_headers"

#--------------------------------------------------------------
# Build
#--------------------------------------------------------------

if [ ! \( -e bin \) ] ; then
	mkdir ./bin
fi

if [ $target = 'lib' ] ; then

	# compile metal shader
	xcrun -sdk macosx metal $shaderFlagParam -c -o $BINDIR/mtl_renderer.air $SRCDIR/mtl_renderer.metal
	xcrun -sdk macosx metallib -o $RESDIR/mtl_renderer.metallib $BINDIR/mtl_renderer.air

	# compile milepost. We use one compilation unit for all C code, and one compilation
	# unit for all ObjectiveC code
	$CC $DEBUG_FLAGS -c -o $BINDIR/milepost_c.o $CFLAGS $FLAGS $INCLUDES $SRCDIR/milepost.c
	$CC $DEBUG_FLAGS -c -o $BINDIR/milepost_objc.o $FLAGS $INCLUDES $SRCDIR/milepost.m

	# build dynamic library
	ld -dylib -o $BINDIR/libmilepost.dylib $BINDIR/milepost_c.o $BINDIR/milepost_objc.o -lc -framework Carbon -framework Cocoa -framework Metal -framework QuartzCore -L$BINDIR -weak-lEGL -weak-lGLESv2

	# change dependent libs path to @rpath.
	install_name_tool -change "./libEGL.dylib" '@rpath/libEGL.dylib' $BINDIR/libmilepost.dylib
	install_name_tool -change "./libGLESv2.dylib" '@rpath/libGLESv2.dylib' $BINDIR/libmilepost.dylib

	# add executable path to rpath. Client executable can still add its own rpaths if needed, e.g. @executable_path/libs/ etc.
	install_name_tool -add_rpath "@executable_path" $BINDIR/libmilepost.dylib

else
	# additional targets
	if [ $target = 'test' ] ; then
		pushd examples/test_app
		./build.sh
		popd

	elif [ $target = 'clean' ] ; then
		rm -r ./bin
	else
		echo "unrecognized target $target"
		exit -1
	fi
fi
