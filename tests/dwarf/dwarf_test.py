import subprocess
import tempfile

res = subprocess.run(["llvm-dwarfdump", "--debug-line", "test_module.wasm"], capture_output=True, check=True)
llvm_table = res.stdout.decode("utf-8")

res = subprocess.run(["./bin/test-dwarf"], capture_output=True, check=True)
test_table = res.stdout.decode("utf-8")

if llvm_table != test_table:
    with open("llvm-dwarfdump.out", "w") as f:
        print(llvm_table, file=f)
    with open("test-dwarf.out", "w") as f:
        print(test_table, file=f)

    with open("test-dwarf.diff", "w") as f:
        subprocess.run(["diff", "test-dwarf.out", "llvm-dwarfdump.out"], stdout=f)

    print("error: test-dwarf output differs from llvm-dwarfdump output (see test-dwarf.diff)")
    exit(-1)
else:
    print("OK")
    exit(0)
