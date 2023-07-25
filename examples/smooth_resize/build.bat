
set INCLUDES=/I ..\..\src /I ..\..\src\util /I ..\..\src\platform /I ../../ext /I ../../ext/angle_headers /I ..\..\..\..\vcpkg\packages\pthreads_x64-windows\include

cl /we4013 /Zi /Zc:preprocessor /std:c11 %INCLUDES% main.c /link /LIBPATH:../../bin milepost.dll.lib /LIBPATH:..\..\..\..\vcpkg\packages\pthreads_x64-windows\lib pthreadVC3.lib /out:../../bin/example_smooth_resize.exe
