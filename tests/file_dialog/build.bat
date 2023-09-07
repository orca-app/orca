
set INCLUDES=/I ..\..\src /I ..\..\ext

mkdir bin

cl /we4013 /Zi /Zc:preprocessor /std:c11 /experimental:c11atomics %INCLUDES% main.c /link /LIBPATH:../../build/bin orca.dll.lib /out:bin/test_file_dialog.exe
copy ..\..\build\bin\orca.dll bin
