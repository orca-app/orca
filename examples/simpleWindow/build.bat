set INCLUDES=/I ..\..\src /I ..\..\src\util /I ..\..\src\platform /I ../../ext

cl main.c %INCLUDES% /Zi /Zc:preprocessor /std:c11 /link user32.lib opengl32.lib gdi32.lib /out:test.exe
