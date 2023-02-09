
if not exist bin mkdir bin

set glsl_shaders=src\glsl_shaders\common.glsl src\glsl_shaders\blit_vertex.glsl src\glsl_shaders\blit_fragment.glsl src\glsl_shaders\clear_counters.glsl src\glsl_shaders\tile.glsl src\glsl_shaders\sort.glsl src\glsl_shaders\draw.glsl

call python3 scripts\embed_text.py %glsl_shaders% --prefix=glsl_ --output src\glsl_shaders.h

set INCLUDES=/I src /I src/util /I src/platform /I ext
cl /we4013 /Zi /Zc:preprocessor /DMG_IMPLEMENTS_BACKEND_GL /std:c11 %INCLUDES% /c /Fo:bin/milepost.obj src/milepost.c
lib bin/milepost.obj /OUT:bin/milepost.lib
