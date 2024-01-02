

set INCLUDES=/I ..\..\src

if not exist "bin" mkdir bin

cl /we4013 /Zi /Zc:preprocessor /std:c11 /experimental:c11atomics %INCLUDES% main.c /link /LIBPATH:../../build/bin orca.dll.lib /out:bin/example_d3d11_triangle.exe

copy ..\..\build\bin\orca.dll bin
