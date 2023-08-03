@echo off

:: compile wasm module
set wasmFlags=--target=wasm32^
       --no-standard-libraries ^
       -fno-builtin ^
       -Wl,--no-entry ^
       -Wl,--export-dynamic ^
       -g ^
       -O2 ^
       -mbulk-memory ^
       -D__ORCA__ ^
       -isystem ..\..\cstdlib\include -I ..\..\sdk -I..\..\milepost\ext -I ..\..\milepost -I ..\..\milepost\src

set shaders=src/shaders/advect.glsl^
	src/shaders/blit_div_fragment.glsl^
	src/shaders/blit_div_vertex.glsl^
	src/shaders/blit_fragment.glsl^
	src/shaders/blit_residue_fragment.glsl^
	src/shaders/blit_vertex.glsl^
	src/shaders/common_vertex.glsl^
	src/shaders/divergence.glsl^
	src/shaders/jacobi_step.glsl^
	src/shaders/multigrid_correct.glsl^
	src/shaders/multigrid_restrict_residual.glsl^
	src/shaders/splat.glsl^
	src/shaders/subtract_pressure.glsl

call python3 ../../milepost/scripts/embed_text.py --prefix=glsl_ --output src/glsl_shaders.h %shaders%

clang %wasmFlags% -o .\module.wasm ..\..\sdk\orca.c ..\..\cstdlib\src\*.c src\main.c

python3 ..\..\scripts\mkapp.py --orca-dir ..\.. --icon icon.png --name Fluid module.wasm
