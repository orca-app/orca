#!/bin/bash

set -exo pipefail

target="$1"

if [ -z $target ] ; then
	target='orca'
fi
target=$(echo $target | tr '[:upper:]' '[:lower:]')

if [ ! \( -e bin \) ] ; then
	mkdir ./bin
fi

if [ ! \( -e resources \) ] ; then
	mkdir ./resources
fi


if [ $target = milepost ] ; then
	echo "building milepost"
	pushd milepost > /dev/null
	./build.sh lib "$2"
	popd > /dev/null

elif [ $target = wasm3 ] ; then

	echo "building wasm3"
	mkdir ./bin/obj
	for file in ./ext/wasm3/source/*.c ; do
		name=$(basename $file)
		name=${name/.c/.o}
		clang -c -g -O2 -foptimize-sibling-calls -Wno-extern-initializer -Dd_m3VerboseErrorMessages -o ./bin/obj/$name -I./ext/wasm3/source $file
	done
	ar -rcs ./bin/libwasm3.a ./bin/obj/*.o
	rm -rf ./bin/obj

elif [ $target = orca ] ; then
	echo "building orca"

	if [ ! \( -e milepost/bin/libmilepost.dylib \) -o ! \( -e milepost/bin/mtl_renderer.metallib \) ] ; then
		./build.sh milepost
	fi

	if [ ! \( -e ./bin/libwasm3.a \) ] ; then
		./build.sh wasm3
	fi

	# copy libraries
	cp milepost/bin/mtl_renderer.metallib bin/
	cp milepost/bin/libmilepost.dylib bin/
	cp milepost/bin/libGLESv2.dylib bin/
	cp milepost/bin/libEGL.dylib bin/

	INCLUDES="-Isrc -Isdk -Imilepost/src -Imilepost/src/util -Imilepost/src/platform -Iext/wasm3/source -Imilepost/ext/"
	LIBS="-Lbin -lmilepost -lwasm3"
	FLAGS="-g -DLOG_COMPILE_DEBUG -mmacos-version-min=10.15.4 -maes"

	# generate wasm3 api bindings

	python3 ./scripts/bindgen.py core \
		src/core_api.json \
		--wasm3-bindings ./src/core_api_bind_gen.c

	python3 ./scripts/bindgen.py gles \
		src/gles_api.json \
		--wasm3-bindings ./src/gles_api_bind_gen.c

	python3 ./scripts/bindgen.py canvas \
		src/canvas_api.json \
		--guest-stubs sdk/orca_surface.c \
		--guest-include graphics.h \
		--wasm3-bindings ./src/canvas_api_bind_gen.c

	python3 ./scripts/bindgen.py clock \
		src/clock_api.json \
		--guest-stubs sdk/orca_clock.c \
		--guest-include platform_clock.h \
		--wasm3-bindings ./src/clock_api_bind_gen.c

	python3 ./scripts/bindgen.py io \
		src/io_api.json \
		--guest-stubs sdk/io_stubs.c \
		--wasm3-bindings ./src/io_api_bind_gen.c

	# compile orca
	clang $FLAGS $INCLUDES $LIBS -o bin/orca src/main.c

	# fix libs imports
	install_name_tool -change "./bin/libmilepost.dylib" "@rpath/libmilepost.dylib" bin/orca
	install_name_tool -add_rpath "@executable_path/" bin/orca


else
	echo "unknown build target $target"
fi
