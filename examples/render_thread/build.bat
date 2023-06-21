
set pthread_dir=..\..\..\..\vcpkg\packages\pthreads_x64-windows
set INCLUDES=/I ..\..\src /I ..\..\src\util /I ..\..\src\platform /I ../../ext /I ../../ext/angle_headers /I %pthread_dir%\include

cl /we4013 /Zi /Zc:preprocessor /std:c11 %INCLUDES% main.c /link /MANIFEST:EMBED /MANIFESTINPUT:../../src/win32_manifest.xml /LIBPATH:../../bin milepost.dll.lib /LIBPATH:%pthread_dir%\lib pthreadVC3.lib /out:../../bin/example_render_thread.exe

copy %pthread_dir%\bin\pthreadVC3.dll ..\..\bin
