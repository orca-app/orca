
set INCLUDES=/I ..\..\src /I ../../src/ext /I ../../src/ext/angle/include

if not exist "bin" mkdir bin
cl /we4013 /Zi /Zc:preprocessor /std:c11 /experimental:c11atomics %INCLUDES% main.c /link /LIBPATH:../../build/bin orca.dll.lib /out:bin/example_gl_triangle.exe
copy ..\..\build\bin\orca.dll bin
