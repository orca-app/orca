
set INCLUDES=/I ..\..\src

if not exist "bin" mkdir "bin"

cl /we4013 /Zi /DEBUG /Zc:preprocessor /std:c11 /experimental:c11atomics %INCLUDES% main.c /link /LIBPATH:../../build/bin orca.dll.lib /out:./bin/test_files.exe
copy "..\..\build\bin\orca.dll" "bin\orca.dll"
