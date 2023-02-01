set INCLUDES=/I ..\..\src /I ..\..\src\util /I ..\..\src\platform /I ../../ext /I ../../ext/angle_headers
cl /we4013 /Zi /Zc:preprocessor /std:c11 %INCLUDES% main.c /link /LIBPATH:../../bin milepost.lib /LIBPATH:../../bin libEGL.dll.lib libGLESv2.dll.lib user32.lib opengl32.lib gdi32.lib /out:../../bin/example_gles_triangle.exe
