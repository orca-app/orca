

clang --target=wasm32 --no-standard-libraries -Wl,--no-entry -Wl,--export-all -g -gdwarf-5 -o module.wasm test_module.c

clang -g -I../../src -L../../build/bin -lorca -o test main.c

install_name_tool -add_rpath '@executable_path/' test
cp ../../build/bin/liborca.dylib .
