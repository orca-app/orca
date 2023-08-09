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
       -isystem ..\..\src\libc-shim\include ^
       -I..\..\ext -I ..\..\src

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


call python3 ../../scripts/embed_text_files.py --prefix=glsl_ --output src/glsl_shaders.h %shaders%
IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

clang %wasmFlags% -o .\module.wasm ..\..\src\orca.c ..\..\src\libc-shim\src\*.c src\main.c
IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

orca bundle --orca-dir ..\.. --icon icon.png --name Fluid module.wasm
