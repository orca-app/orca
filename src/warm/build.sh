
if [ ! -d bin ] ; then
    mkdir bin
fi


clang -std=c11 -g -O0 -I.. -I../ext -L../../build/bin -lorca -o bin/wastep step.c
cp ../../build/bin/liborca.dylib ./bin
install_name_tool -add_rpath '@executable_path/' bin/wastep

clang -std=c11 -g -O0 -I.. -I../ext -L../../build/bin -lorca -o bin/warm main.c
cp ../../build/bin/liborca.dylib ./bin
install_name_tool -add_rpath '@executable_path/' bin/warm


clang -g -I.. -L../../build/bin -lorca dwarfdump.c -o bin/dwarfdump
install_name_tool -add_rpath '@executable_path/' bin/dwarfdump

clang --target=wasm32 --no-standard-libraries -Wl,--no-entry -Wl,--export-all -gdwarf-5 -o test/test.wasm test/wasm_test.c
