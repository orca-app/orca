
if [ ! -d bin ] ; then
    mkdir bin
fi

clang -g -O0 -DOC_DEBUG -I../../src -L../../zig-out/bin -lorca_platform -o bin/test ../../zig-out/bin/libwarm.a main.c
cp ../../zig-out/bin/liborca_platform.dylib ./bin
install_name_tool -add_rpath '@executable_path/' bin/test
