
set INCLUDES=/I ..\..\src

if not exist "bin" mkdir bin
cl /we4013 /Zi /Zc:preprocessor /std:c11 /experimental:c11atomics %INCLUDES% main.c /link /LIBPATH:../../build/bin orca.dll.lib /out:bin/color_test.exe

copy ..\..\build\bin\orca.dll bin
copy ..\..\src\ext\dawn\bin\webgpu.dll bin
