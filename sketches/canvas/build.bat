
set INCLUDES=/I ..\..\src
if not exist "bin" mkdir bin
cl /we4013 /Zi /Zc:preprocessor /std:c11 /experimental:c11atomics %INCLUDES% main.c /link /LIBPATH:../../build/bin orca.dll.lib /out:bin/example_canvas.exe

copy ..\..\build\bin\orca.dll bin
copy ..\..\build\bin\webgpu.dll bin
