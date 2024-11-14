
set INCLUDES=/I ..\..\src /I ..\..\src\util /I ..\..\src\platform /I ../../ext /I ../../ext/angle_headers

if not exist "bin" mkdir bin
cl /we4013 /Zi /Zc:preprocessor /std:c11 /experimental:c11atomics %INCLUDES% main.c /link /LIBPATH:../../build/bin orca.dll.lib /out:bin/example_ui.exe

copy ..\..\build\bin\orca.dll bin
copy ..\..\build\bin\webgpu.dll bin
copy ..\..\build\bin\libharfbuzz.dll bin
copy ..\..\build\bin\fribidi-0.dll bin
