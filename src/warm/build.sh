
if [ ! -d bin ] ; then
    mkdir bin
fi


clang -std=c11 -g -O0 -I.. -I../ext -L../../build/bin -lorca -o bin/wastep step.c
cp ../../build/bin/liborca.dylib ./bin
install_name_tool -add_rpath '@executable_path/' bin/wastep
