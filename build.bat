
if not exist bin mkdir bin

set gles_shaders=src\gles_canvas_shaders\gles_canvas_blit_vertex.glsl src\gles_canvas_shaders\gles_canvas_blit_fragment.glsl src\gles_canvas_shaders\gles_canvas_clear_counters.glsl src\gles_canvas_shaders\gles_canvas_tile.glsl src\gles_canvas_shaders\gles_canvas_sort.glsl src\gles_canvas_shaders\gles_canvas_draw.glsl
call python scripts\embed_text.py %gles_shaders% --output src\gles_canvas_shaders.h

set INCLUDES=/I src /I src/util /I src/platform /I ext
cl /we4013 /Zi /Zc:preprocessor /DMG_IMPLEMENTS_BACKEND_GL /std:c11 %INCLUDES% /c /Fo:bin/milepost.obj src/milepost.c
lib bin/milepost.obj /OUT:bin/milepost.lib
