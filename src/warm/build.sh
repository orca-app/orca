
if [ ! -d bin ] ; then
    mkdir bin
fi

clang -g -I.. -o bin/warm -DOC_NO_APP_LAYER ../orca.c main.c

# compile test wasm
clang --target=wasm32 --no-standard-libraries -Wl,--no-entry -Wl,--export-all -o test/test.wasm test/wasm_test.c
wasm2wat test/test.wasm > test/test.wat

wat2wasm test/if.wat -o test/if.wasm
