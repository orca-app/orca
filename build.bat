@echo off

set target=%1%
if "%~1%" == "" set target=orca

if not exist bin mkdir bin
if not exist bin\obj mkdir bin\obj

if %target% == wasm3 (
	echo building wasm3

	set wasm3_includes=/I .\ext\wasm3\source
	set wasm3_sources=/I .\ext\wasm3\source\*.c

	for %%f in ( .\ext\wasm3\source\*.c ) do (
		cl /nologo /Zi /Zc:preprocessor /c /Fo:bin\obj\%%~nf.obj %wasm3_includes% %%f
	)
	lib /nologo /out:bin\wasm3.lib bin\obj\*.obj
)

if %target% == milepost (
	echo building milepost
	cd milepost
	build.bat
	cd ..
)

if %target% == orca (
	echo building orca

	::copy libraries
	copy milepost\bin\milepost.dll bin
	copy milepost\bin\milepost.dll.lib bin

	::generate wasm3 api bindings
	python3 scripts\bindgen.py core src

	python3 scripts\bindgen.py gles src

	python3 scripts\bindgen2.py canvas src\canvas_api.json^
	        --guest-stubs sdk\orca_surface.c^
	        --guest-include graphics.h^
	        --wasm3-bindings src\canvas_api_bind_gen.c

	python3 scripts\bindgen2.py io^
	        src\io_api.json^
	        --guest-stubs sdk\io_stubs.c^
	        --wasm3-bindings src\io_api_bind_gen.c

	::compile orca
	set INCLUDES=/I src /I sdk /I ext\wasm3\source /I milepost\src /I milepost\ext /I ..\vcpkg\packages\pthreads_x64-windows\include
	set LIBS=/LIBPATH:bin /LIBPATH:..\vcpkg\packages\pthreads_x64-windows\lib milepost.dll.lib wasm3.lib pthreadVC3.lib

	cl /Zi /Zc:preprocessor /std:c11 %INCLUDES% src\main.c /link %LIBS% /out:bin\orca.exe
)
