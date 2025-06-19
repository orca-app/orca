#!/bin/zsh

LIBDIR=../../build/bin
SRCDIR=../../src

INCLUDES="-I$SRCDIR"
LIBS=(-L$LIBDIR -lorca)
FLAGS=(-mmacos-version-min=10.15.4 -DOC_DEBUG -DLOG_COMPILE_DEBUG)

if [ ! \( -e bin \) ] ; then
	mkdir ./bin
fi

clang -g ${FLAGS[@]} ${LIBS[@]} $INCLUDES -o ./bin/test_file_dialog main.c

cp $LIBDIR/liborca.dylib ./bin/

install_name_tool -add_rpath "@executable_path" ./bin/test_file_dialog


# build orca version

ORCA_DIR=$(orca sdk-path)

# common flags to build wasm modules
wasmFlags=(--target=wasm32 \
  -mbulk-memory \
  -g -O2 \
  -Wl,--no-entry \
  -Wl,--export-dynamic \
  --sysroot "$ORCA_DIR"/orca-libc \
  -I "$ORCA_DIR"/src \
  -I "$ORCA_DIR"/src/ext)

# build sample as wasm module and link it with the orca module
clang "${wasmFlags[@]}" -L "$ORCA_DIR"/bin -lorca_wasm -o module.wasm main_wasm.c

# create app directory and copy files into it
orca bundle --name test_file_dialog module.wasm
