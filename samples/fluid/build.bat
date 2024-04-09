@echo off
setlocal enabledelayedexpansion

for /f %%i in ('orca sdk-path') do set ORCA_DIR=%%i

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

call python ../../scripts/embed_text_files.py --prefix=glsl_ --output src/glsl_shaders.h %shaders%
if !ERRORLEVEL! neq 0 exit /b !ERRORLEVEL!

:: common flags to build wasm modules
set wasmFlags=--target=wasm32^
       -mbulk-memory ^
       -g -O2 ^
       -Wl,--no-entry ^
       -Wl,--export-dynamic ^
       --sysroot %ORCA_DIR%/orca-libc ^
       -I%ORCA_DIR%/src ^
       -I%ORCA_DIR%/src/ext

:: build sample as wasm module and link it with the orca module
clang %wasmFlags% -L %ORCA_DIR%/bin -lorca_wasm -o module.wasm src/main.c
IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

orca bundle --name Fluid --icon icon.png module.wasm
