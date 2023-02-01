
if not exist bin mkdir bin

call python scripts\embed_text.py src\gles_canvas_shaders\gles_canvas_fragment.glsl src\gles_canvas_shaders\gles_canvas_vertex.glsl --output src\gles_canvas_shaders.h

set INCLUDES=/I src /I src/util /I src/platform /I ext /I ext/angle_headers
cl /we4013 /Zi /Zc:preprocessor /DMG_IMPLEMENTS_BACKEND_GLES /std:c11 %INCLUDES% /c /Fo:bin/milepost.obj src/milepost.c
lib bin/milepost.obj /OUT:bin/milepost.lib
