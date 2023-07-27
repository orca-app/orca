
@echo off
setlocal EnableDelayedExpansion

if not exist bin mkdir bin

set glsl_shaders=src\glsl_shaders\common.glsl src\glsl_shaders\blit_vertex.glsl src\glsl_shaders\blit_fragment.glsl src\glsl_shaders\path_setup.glsl src\glsl_shaders\segment_setup.glsl src\glsl_shaders\backprop.glsl src\glsl_shaders\merge.glsl src\glsl_shaders\raster.glsl src\glsl_shaders\balance_workgroups.glsl

call python3 scripts\embed_text.py %glsl_shaders% --prefix=glsl_ --output src\glsl_shaders.h

set INCLUDES=/I src /I src/util /I src/platform /I ext /I ext/angle_headers
set LIBS=user32.lib opengl32.lib gdi32.lib shcore.lib delayimp.lib dwmapi.lib comctl32.lib ole32.lib shell32.lib shlwapi.lib /LIBPATH:./bin libEGL.dll.lib libGLESv2.dll.lib  /DELAYLOAD:libEGL.dll /DELAYLOAD:libGLESv2.dll

cl /we4013 /Zi /Zc:preprocessor /DMP_BUILD_DLL /std:c11 %INCLUDES% src/milepost.c /Fo:bin/milepost.o /LD /link /MANIFEST:EMBED /MANIFESTINPUT:src/win32_manifest.xml %LIBS% /OUT:bin/milepost.dll /IMPLIB:bin/milepost.dll.lib
