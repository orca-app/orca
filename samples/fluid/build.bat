@echo off
setlocal enabledelayedexpansion

:: The following code checks if you have the necessary programs to compile the samples.
:: This code exists to improve the experience of first-time Orca users and can
:: be safely deleted in your own projects if you wish.
if exist "..\..\scripts\sample_build_check.py" (
       python ..\..\scripts\sample_build_check.py
       if !ERRORLEVEL! neq 0 exit /b 1
) else (
       echo Could not check if you have the necessary tools to build the Orca samples.
       echo If you have copied this script to your own project, you can delete this code.
)

set ORCA_DIR=..\..
set STDLIB_DIR=%ORCA_DIR%\src\libc-shim

:: common flags to build wasm modules
set wasmFlags=--target=wasm32^
       --no-standard-libraries ^
       -mbulk-memory ^
       -g -O2 ^
       -D__ORCA__ ^
       -Wl,--no-entry ^
       -Wl,--export-dynamic ^
       -isystem %STDLIB_DIR%\include ^
       -I%ORCA_DIR%\src ^
       -I%ORCA_DIR%\src\ext

:: build orca core as wasm module
clang %wasmFlags% -Wl,--relocatable -o .\liborca.a %ORCA_DIR%\src\orca.c %ORCA_DIR%\src\libc-shim\src\*.c
IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

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

:: build sample as wasm module and link it with the orca module
clang %wasmFlags% -L . -lorca -o .\module.wasm src\main.c
IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

:: create app directory and copy files into it
orca bundle --orca-dir %ORCA_DIR% --name Fluid --icon icon.png module.wasm
