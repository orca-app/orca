

clang --target=wasm32 --no-standard-libraries -Wl,--no-entry -Wl,--export-all -g -gdwarf-5 -o module.wasm test_module.c

clang -g -o test main.c
