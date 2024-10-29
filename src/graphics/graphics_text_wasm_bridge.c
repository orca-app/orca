#include "graphics_common.h"
#include "runtime_memory.h"
#include "ext/harfbuzz/include/hb.h"
#include "ext/harfbuzz/include/hb-ot.h"

typedef struct oc_wasm_glyph_run
{
    oc_font font;

    u32 glyphCount;
    oc_wasm_addr glyphs;

    u32 graphemeCount;
    oc_wasm_addr graphemes;

    oc_text_metrics metrics;

} oc_wasm_glyph_run;

oc_harfbuzz_handle oc_harfbuzz_font_create_bridge(oc_wasm_str8 mem)
{
    oc_str8 nativeMem = {
        .ptr = oc_wasm_address_to_ptr(mem.ptr, mem.len),
        .len = mem.len
    };
    return (oc_harfbuzz_font_create(nativeMem));
}

oc_wasm_addr oc_harfbuzz_font_shape_bridge(oc_wasm_addr arena,
                                           oc_harfbuzz_handle handle,
                                           oc_text_shape_settings* settings,
                                           oc_wasm_str32 codepoints,
                                           u64 begin,
                                           u64 end)
{
    oc_arena_scope scratch = oc_scratch_begin();

    oc_str32 nativeCodepoints = {
        .ptr = oc_wasm_address_to_ptr(codepoints.ptr, codepoints.len * sizeof(u32)),
        .len = codepoints.len,
    };
    oc_glyph_run* nativeRun = oc_harfbuzz_font_shape(scratch.arena, handle, settings, nativeCodepoints, begin, end);

    oc_wasm_addr wasmRunAddr = oc_wasm_arena_push(arena, sizeof(oc_wasm_glyph_run));
    oc_wasm_glyph_run* wasmRun = oc_wasm_address_to_ptr(wasmRunAddr, sizeof(oc_wasm_glyph_run));

    wasmRun->glyphCount = nativeRun->glyphCount;
    wasmRun->glyphs = oc_wasm_arena_push(arena, sizeof(oc_glyph_info) * wasmRun->glyphCount);
    oc_glyph_info* glyphs = oc_wasm_address_to_ptr(wasmRun->glyphs, sizeof(oc_glyph_info) * wasmRun->glyphCount);
    memcpy(glyphs, nativeRun->glyphs, sizeof(oc_glyph_info) * wasmRun->glyphCount);

    wasmRun->graphemeCount = nativeRun->graphemeCount;
    wasmRun->graphemes = oc_wasm_arena_push(arena, sizeof(oc_grapheme_info) * wasmRun->graphemeCount);
    oc_grapheme_info* graphemes = oc_wasm_address_to_ptr(wasmRun->graphemes, sizeof(oc_grapheme_info) * wasmRun->graphemeCount);
    memcpy(graphemes, nativeRun->graphemes, sizeof(oc_grapheme_info) * wasmRun->graphemeCount);

    memcpy(&wasmRun->metrics, &nativeRun->metrics, sizeof(oc_text_metrics));

    oc_scratch_end(scratch);

    return (wasmRunAddr);
}

hb_draw_funcs_t* oc_hbDumpFuncs = 0;

typedef struct oc_hb_dump_data
{
    oc_path_elt* pathElements;
    u32 eltCap;
    u32 eltCount;

    f32 scale;
    f32 flipY;
    oc_vec2 origin;
} oc_hb_dump_data;

void oc_hb_dump_move_to_func(hb_draw_funcs_t* dfuncs,
                             void* draw_data,
                             hb_draw_state_t* st,
                             float to_x,
                             float to_y,
                             void* user_data)
{
    oc_hb_dump_data* data = (oc_hb_dump_data*)draw_data;
    if(data->eltCount < data->eltCap)
    {
        oc_path_elt* elt = &data->pathElements[data->eltCount];
        data->eltCount++;

        elt->type = OC_PATH_MOVE;
        elt->p[0].x = data->origin.x + to_x * data->scale;
        elt->p[0].y = data->origin.y - to_y * data->scale * data->flipY;
    }
}

void oc_hb_dump_line_to_func(hb_draw_funcs_t* dfuncs,
                             void* draw_data,
                             hb_draw_state_t* st,
                             float to_x,
                             float to_y,
                             void* user_data)
{
    oc_hb_dump_data* data = (oc_hb_dump_data*)draw_data;
    if(data->eltCount < data->eltCap)
    {
        oc_path_elt* elt = &data->pathElements[data->eltCount];
        data->eltCount++;

        elt->type = OC_PATH_MOVE;
        elt->p[0].x = data->origin.x + to_x * data->scale;
        elt->p[0].y = data->origin.y - to_y * data->scale * data->flipY;
    }
}

void oc_hb_dump_quadratic_to_func(hb_draw_funcs_t* dfuncs,
                                  void* draw_data,
                                  hb_draw_state_t* st,
                                  float control_x,
                                  float control_y,
                                  float to_x,
                                  float to_y,
                                  void* user_data)
{
    oc_hb_dump_data* data = (oc_hb_dump_data*)draw_data;
    if(data->eltCount < data->eltCap)
    {
        oc_path_elt* elt = &data->pathElements[data->eltCount];
        data->eltCount++;

        elt->type = OC_PATH_MOVE;
        elt->p[0].x = data->origin.x + to_x * data->scale;
        elt->p[0].y = data->origin.y - to_y * data->scale * data->flipY;
        elt->p[1].x = data->origin.x + to_x * data->scale;
        elt->p[1].y = data->origin.y - to_y * data->scale * data->flipY;
    }
}

void oc_hb_dump_cubic_to_func(hb_draw_funcs_t* dfuncs,
                              void* draw_data,
                              hb_draw_state_t* st,
                              float control1_x,
                              float control1_y,
                              float control2_x,
                              float control2_y,
                              float to_x,
                              float to_y,
                              void* user_data)
{
    oc_hb_dump_data* data = (oc_hb_dump_data*)draw_data;
    if(data->eltCount < data->eltCap)
    {
        oc_path_elt* elt = &data->pathElements[data->eltCount];
        data->eltCount++;

        elt->type = OC_PATH_MOVE;
        elt->p[0].x = data->origin.x + to_x * data->scale;
        elt->p[0].y = data->origin.y - to_y * data->scale * data->flipY;
        elt->p[1].x = data->origin.x + control2_x * data->scale;
        elt->p[1].y = data->origin.y - control2_y * data->scale * data->flipY;
        elt->p[2].x = data->origin.x + to_x * data->scale;
        elt->p[2].y = data->origin.y - to_y * data->scale * data->flipY;
    }
}

oc_wasm_addr oc_harfbuzz_get_curves_bridge(oc_wasm_addr arena,
                                           oc_harfbuzz_handle handle,
                                           oc_wasm_glyph_run* run,
                                           oc_vec2 start,
                                           f32 scale,
                                           bool flipY,
                                           u32* eltCount)
{
    *eltCount = 0;
    oc_glyph_run nativeRun = {
        .glyphCount = run->glyphCount,
        .glyphs = oc_wasm_address_to_ptr(run->glyphs, run->glyphCount * sizeof(oc_glyph_info)),
        //NOTE: the rest isn't used by oc_harfbuzz_get_curves
    };

    oc_arena_scope scratch = oc_scratch_begin();

    u32 nativeElementCount = 0;
    oc_path_elt* nativeElements = oc_harfbuzz_get_curves(scratch.arena, handle, &nativeRun, start, scale, flipY, &nativeElementCount);

    oc_wasm_addr wasmElements = oc_wasm_arena_push(arena, sizeof(oc_path_elt) * nativeElementCount);
    oc_path_elt* elements = oc_wasm_address_to_ptr(wasmElements, sizeof(oc_path_elt) * nativeElementCount);

    memcpy(elements, nativeElements, nativeElementCount * sizeof(oc_path_elt));
    *eltCount = nativeElementCount;

    oc_scratch_end(scratch);

    return (wasmElements);
}
