
if [ ! -d bin ] ; then
    mkdir bin
fi


clang -g -I.. -I../ext -I../ext/angle/include -L../../build/bin -lorca -o bin/waspector waspector.c

cp ../../build/bin/liborca.dylib ./bin
cp ../../build/bin/libwebgpu.dylib ./bin

install_name_tool -add_rpath '@executable_path/' bin/waspector
