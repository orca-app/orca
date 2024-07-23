
if [ ! -d bin ] ; then
    mkdir bin
fi


clang -g -I.. -I../ext -L../../build/bin -lorca -o bin/warm main.c

cp ../../build/bin/liborca.dylib ./bin

install_name_tool -add_rpath '@executable_path/' bin/warm


# compile test wasm
clang --target=wasm32 --no-standard-libraries -Wl,--no-entry -Wl,--export-all -o test/test.wasm test/wasm_test.c
wasm2wat test/test.wasm > test/test.wat

wat2wasm test/if.wat -o test/if.wasm
wat2wasm test/block.wat -o test/block.wasm
wat2wasm test/import.wat -o test/import.wasm


# quick json test
clang -g -I.. -I../ext -L../../build/bin -lorca -o bin/json_test json.c
install_name_tool -add_rpath '@executable_path/' bin/json_test
