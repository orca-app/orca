#include "graphics/graphics.h"
#include "graphics/graphics_common.h"
#include "ext/harfbuzz/include/hb.h"
#include "ext/harfbuzz/include/hb-ot.h"

hb_draw_funcs_t* oc_hbDrawFuncs = 0;

typedef struct oc_harfbuzz_font
{
    oc_list_elt freeListElt;
    hb_face_t* hbFace;
    hb_font_t* hbFont;

    oc_font_metrics metrics;
    f32 upem;

} oc_harfbuzz_font;

oc_harfbuzz_handle oc_harfbuzz_handle_alloc(oc_harfbuzz_font* font)
{
    oc_harfbuzz_handle handle = { .h = oc_graphics_handle_alloc(OC_GRAPHICS_HANDLE_HARFBUZZ, (void*)font) };
    return (handle);
}

oc_harfbuzz_font* oc_harfbuzz_font_from_handle(oc_harfbuzz_handle handle)
{
    oc_harfbuzz_font* font = oc_graphics_data_from_handle(OC_GRAPHICS_HANDLE_HARFBUZZ, handle.h);
    return (font);
}

oc_harfbuzz_handle oc_harfbuzz_font_create(oc_str8 mem)
{
    if(!oc_graphicsData.init)
    {
        oc_graphics_init();
    }
    oc_harfbuzz_handle handle = { 0 };

    oc_harfbuzz_font* font = oc_list_pop_front_entry(&oc_graphicsData.harfbuzzFreeList, oc_harfbuzz_font, freeListElt);
    if(!font)
    {
        font = oc_arena_push_type(&oc_graphicsData.resourceArena, oc_harfbuzz_font);
    }
    if(font)
    {
        //NOTE: create harfbuzz face and font
        hb_blob_t* blob = hb_blob_create(mem.ptr, mem.len, HB_MEMORY_MODE_DUPLICATE, 0, 0);
        font->hbFace = hb_face_create(blob, 0);
        font->hbFont = hb_font_create(font->hbFace);

        font->upem = hb_face_get_upem(font->hbFace);

        //TODO: this assumes only horizontal layout
        hb_font_extents_t hbExtents;
        hb_font_get_h_extents(font->hbFont, &hbExtents);

        hb_codepoint_t xGlyph = 0;
        hb_glyph_extents_t xExtents = { 0 };
        hb_font_get_nominal_glyph(font->hbFont, 'x', &xGlyph);
        hb_font_get_glyph_extents(font->hbFont, xGlyph, &xExtents);

        hb_codepoint_t MGlyph = 0;
        hb_glyph_extents_t MExtents = { 0 };
        hb_position_t MAdvanceX = 0;
        hb_position_t MAdvanceY = 0;
        hb_font_get_nominal_glyph(font->hbFont, 'M', &MGlyph);
        hb_font_get_glyph_extents(font->hbFont, MGlyph, &MExtents);
        hb_font_get_glyph_advance_for_direction(font->hbFont,
                                                MGlyph,
                                                HB_DIRECTION_LTR,
                                                &MAdvanceX,
                                                &MAdvanceY);

        font->metrics.ascent = hbExtents.ascender;
        font->metrics.descent = -hbExtents.descender;
        font->metrics.lineGap = hbExtents.line_gap;
        font->metrics.xHeight = xExtents.height;
        font->metrics.capHeight = MExtents.height;
        font->metrics.width = MAdvanceX;

        handle = oc_harfbuzz_handle_alloc(font);
    }

    return handle;
}

void oc_harfbuzz_font_destroy(oc_harfbuzz_handle handle)
{
    oc_harfbuzz_font* font = oc_harfbuzz_font_from_handle(handle);
    if(font)
    {
        hb_font_destroy(font->hbFont);
        hb_face_destroy(font->hbFace);
    }
}

f32 oc_harfbuzz_font_get_upem(oc_harfbuzz_handle handle)
{
    f32 res = 0;
    oc_harfbuzz_font* font = oc_harfbuzz_font_from_handle(handle);
    if(font)
    {
        res = font->upem;
    }
    return (res);
}

oc_font_metrics oc_harfbuzz_font_get_metrics(oc_harfbuzz_handle handle)
{
    oc_font_metrics metrics = { 0 };
    oc_harfbuzz_font* font = oc_harfbuzz_font_from_handle(handle);
    if(font)
    {
        metrics = font->metrics;
    }
    return (metrics);
}

oc_glyph_run* oc_harfbuzz_font_shape(oc_arena* arena,
                                     oc_harfbuzz_handle handle,
                                     oc_text_shape_settings* settings,
                                     oc_str32 codepoints,
                                     u64 begin,
                                     u64 end)
{
    oc_glyph_run* run = oc_arena_push_type(arena, oc_glyph_run);
    memset(run, 0, sizeof(oc_glyph_run));

    run->codepointCount = codepoints.len;

    oc_harfbuzz_font* harfbuzzFont = oc_harfbuzz_font_from_handle(handle);
    if(!harfbuzzFont)
    {
        return (run);
    }

    hb_buffer_t* buffer = hb_buffer_create();
    u32 segmentLen = end >= begin ? end - begin : 0;
    hb_buffer_add_codepoints(buffer, codepoints.ptr, codepoints.len, begin, segmentLen);

    if(!settings
       || settings->script.len == 0
       || settings->lang.len == 0
       || settings->direction == OC_TEXT_DIRECTION_UNKNOWN)
    {
        hb_buffer_guess_segment_properties(buffer);
    }

    if(settings)
    {
        if(settings->script.len)
        {
            hb_script_t script = hb_script_from_string(settings->script.ptr, settings->script.len);
            hb_buffer_set_script(buffer, script);
        }
        if(settings->lang.len)
        {
            hb_language_t lang = hb_language_from_string(settings->lang.ptr, settings->lang.len);
            hb_buffer_set_language(buffer, lang);
        }
        switch(settings->direction)
        {
            case OC_TEXT_DIRECTION_LTR:
                hb_buffer_set_direction(buffer, HB_DIRECTION_LTR);
                break;
            case OC_TEXT_DIRECTION_RTL:
                hb_buffer_set_direction(buffer, HB_DIRECTION_RTL);
                break;
            case OC_TEXT_DIRECTION_TTB:
                hb_buffer_set_direction(buffer, HB_DIRECTION_TTB);
                break;
            case OC_TEXT_DIRECTION_BTT:
                hb_buffer_set_direction(buffer, HB_DIRECTION_BTT);
                break;
            default:
                break;
        }
    }

    hb_shape(harfbuzzFont->hbFont, buffer, NULL, 0);

    hb_glyph_info_t* glyphInfo = hb_buffer_get_glyph_infos(buffer, &run->glyphCount);
    hb_glyph_position_t* glyphPos = hb_buffer_get_glyph_positions(buffer, &run->glyphCount);

    run->glyphs = oc_arena_push_array(arena, oc_glyph_info, run->glyphCount);

    f32 lineHeight = harfbuzzFont->metrics.descent + harfbuzzFont->metrics.ascent + harfbuzzFont->metrics.lineGap;
    if(run->glyphCount)
    {
        run->metrics.logical.y = -(harfbuzzFont->metrics.ascent + harfbuzzFont->metrics.lineGap); //TODO: should we really have line gap here?
        run->metrics.logical.h += lineHeight + harfbuzzFont->metrics.lineGap;
    }

    for(u64 i = 0; i < run->glyphCount; i++)
    {
        oc_glyph_info* glyph = &run->glyphs[i];
        glyph->index = glyphInfo[i].codepoint;

        //WARN: Harfbuzz uses y-up so we negate Y advances and offsets
        glyph->offset = (oc_vec2){ glyphPos[i].x_offset, -glyphPos[i].y_offset };

        //TODO this assumes LTR
        hb_glyph_extents_t glyphExtents = { 0 };
        hb_font_get_glyph_extents(harfbuzzFont->hbFont, glyph->index, &glyphExtents);

        //WARN: Harfbuzz uses y-up, y_bearing is the top of the bbox wrt origin, and height is the _signed_
        //      distance from the top to the bottom (i.e. it is _negative_).
        glyph->metrics.ink = (oc_rect){
            run->metrics.advance.x + glyphExtents.x_bearing,
            run->metrics.advance.y - glyphExtents.y_bearing,
            glyphExtents.width,
            -glyphExtents.height,
        };

        glyph->metrics.logical = (oc_rect){
            run->metrics.logical.x + run->metrics.advance.x,
            run->metrics.logical.y + run->metrics.advance.y,
            glyphPos[i].x_advance,
            lineHeight,
        };

        glyph->metrics.advance = (oc_vec2){ glyphPos[i].x_advance, -glyphPos[i].y_advance };

        run->metrics.ink = oc_rect_combine(run->metrics.ink, glyph->metrics.ink);
        run->metrics.logical.w = oc_max(run->metrics.logical.w, run->metrics.advance.x);
        run->metrics.advance = oc_vec2_add(run->metrics.advance, glyph->metrics.advance);
    }

    //------------------------------------------------------------------------------
    //NOTE compute graphemes info
    oc_arena_scope scratch = oc_scratch_begin_next(arena);

    //TODO: detect grapheme boundaries. For now just consider all codepoints
    run->graphemeCount = segmentLen;
    run->graphemes = oc_arena_push_array(arena, oc_grapheme_info, run->graphemeCount);
    for(u64 i = 0; i < run->graphemeCount; i++)
    {
        run->graphemes[i].offset = begin + i;
        run->graphemes[i].count = 1;
        run->graphemes[i].metrics = (oc_text_metrics){ 0 };
    }

    if(run->glyphCount)
    {
        //NOTE: collate clusters and cluster widths
        u64* clusters = oc_arena_push_array(scratch.arena, u64, run->glyphCount);
        u64* clusterFirstGlyphs = oc_arena_push_array(scratch.arena, u64, run->glyphCount);
        oc_text_metrics* clusterMetrics = oc_arena_push_array(scratch.arena, oc_text_metrics, run->glyphCount);

        u64 clusterCount = 0;
        u64 currentClusterValue = 0;

        currentClusterValue = glyphInfo[0].cluster;
        clusters[clusterCount] = currentClusterValue;
        clusterFirstGlyphs[clusterCount] = 0;
        clusterMetrics[clusterCount] = run->glyphs[0].metrics;
        clusterCount++;

        for(u64 glyphIndex = 1; glyphIndex < run->glyphCount; glyphIndex++)
        {
            if(glyphInfo[glyphIndex].cluster != currentClusterValue)
            {
                currentClusterValue = glyphInfo[glyphIndex].cluster;
                clusters[clusterCount] = currentClusterValue;
                clusterFirstGlyphs[clusterCount] = glyphIndex;
                clusterMetrics[clusterCount] = run->glyphs[glyphIndex].metrics;
                clusterCount++;
            }
            clusterMetrics[clusterCount] = oc_text_metrics_combine(clusterMetrics[clusterCount],
                                                                   run->glyphs[glyphIndex].metrics);
        }

        //NOTE: bucket graphemes into clusters
        u64* clusterFirstGrapheme = oc_arena_push_array(scratch.arena, u64, clusterCount);
        u64* clusterGraphemeCount = oc_arena_push_array(scratch.arena, u64, clusterCount);
        u64 nextGrapheme = 0;

        for(u64 clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++)
        {
            clusterFirstGrapheme[clusterIndex] = nextGrapheme;
            clusterGraphemeCount[clusterIndex] = 0;
            for(u64 graphemeIndex = nextGrapheme; graphemeIndex < run->graphemeCount; graphemeIndex++)
            {
                if((clusterIndex < clusterCount - 1)
                   && run->graphemes[graphemeIndex].offset >= clusters[clusterIndex + 1])
                {
                    //NOTE: End of current cluster.
                    //      Cluster contains clusterGraphemeCount graphemes starting at clusterFirstGrapheme
                    nextGrapheme += clusterGraphemeCount[clusterIndex];
                    break;
                }
                else
                {
                    // add graphemes to current cluster
                    clusterGraphemeCount[clusterIndex]++;
                }
            }
        }

        //NOTE: compute grapheme positions
        for(u64 clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++)
        {
            u64 firstGrapheme = clusterFirstGrapheme[clusterIndex];
            u64 graphemeCount = clusterGraphemeCount[clusterIndex];

            //TODO: we should try to use ligature caret position if they exist, using
            // hb_ot_layout_get_ligature_carets(). However, all latin fonts with ligatures
            // I've tested don't even define them. Arabic fonts make more use of them,
            // but I don't know what result to expect as I don't know arabic, so I should
            // check with someone who does.

            //NOTE: Fall back to evenly subdividing the cluster size.
            //////////////////////////////////////////////////////////////////////////////////////////////
            //TODO: this works only for LTR
            //////////////////////////////////////////////////////////////////////////////////////////////

            oc_text_metrics metrics = clusterMetrics[clusterIndex];
            oc_vec2 advance = oc_vec2_mul(1. / graphemeCount, metrics.advance);

            for(u64 graphemeIndex = 0; graphemeIndex < graphemeCount; graphemeIndex++)
            {
                oc_vec2 offset = oc_vec2_mul(graphemeIndex, advance);

                oc_text_metrics* graphemeMetrics = &run->graphemes[firstGrapheme + graphemeIndex].metrics;

                graphemeMetrics->ink = (oc_rect){
                    metrics.ink.x + graphemeIndex * advance.x,
                    metrics.ink.y,
                    metrics.ink.w / graphemeCount,
                    metrics.ink.h,
                };

                graphemeMetrics->logical = (oc_rect){
                    metrics.logical.x + graphemeIndex * advance.x,
                    metrics.logical.y,
                    metrics.logical.w / graphemeCount,
                    metrics.logical.h,
                };

                graphemeMetrics->advance = advance;
            }
        }
        oc_scratch_end(scratch);
    }

    hb_buffer_destroy(buffer);

    return (run);
}

typedef struct oc_hb_draw_data
{
    f32 scale;
    f32 flipY;
    oc_vec2 origin;
} oc_hb_draw_data;

//WARN: font curves are passed by Harfbuzz with y-up

void oc_hb_move_to_func(hb_draw_funcs_t* dfuncs,
                        void* draw_data,
                        hb_draw_state_t* st,
                        float to_x,
                        float to_y,
                        void* user_data)
{
    oc_hb_draw_data* data = (oc_hb_draw_data*)draw_data;
    oc_move_to(data->origin.x + to_x * data->scale,
               data->origin.y - to_y * data->scale * data->flipY);
}

void oc_hb_line_to_func(hb_draw_funcs_t* dfuncs,
                        void* draw_data,
                        hb_draw_state_t* st,
                        float to_x,
                        float to_y,
                        void* user_data)
{
    oc_hb_draw_data* data = (oc_hb_draw_data*)draw_data;
    oc_line_to(data->origin.x + to_x * data->scale,
               data->origin.y - to_y * data->scale * data->flipY);
}

void oc_hb_quadratic_to_func(hb_draw_funcs_t* dfuncs,
                             void* draw_data,
                             hb_draw_state_t* st,
                             float control_x,
                             float control_y,
                             float to_x,
                             float to_y,
                             void* user_data)
{
    oc_hb_draw_data* data = (oc_hb_draw_data*)draw_data;
    oc_quadratic_to(data->origin.x + control_x * data->scale,
                    data->origin.y - control_y * data->scale * data->flipY,
                    data->origin.x + to_x * data->scale,
                    data->origin.y - to_y * data->scale * data->flipY);
}

void oc_hb_cubic_to_func(hb_draw_funcs_t* dfuncs,
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
    oc_hb_draw_data* data = (oc_hb_draw_data*)draw_data;
    oc_cubic_to(data->origin.x + control1_x * data->scale,
                data->origin.y - control1_y * data->scale * data->flipY,
                data->origin.x + control2_x * data->scale,
                data->origin.y - control2_y * data->scale * data->flipY,
                data->origin.x + to_x * data->scale,
                data->origin.y - to_y * data->scale * data->flipY);
}

void oc_text_draw_run(oc_glyph_run* run, f32 fontSize)
{
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(!context)
    {
        return;
    }
    oc_font_data* fontData = oc_font_data_from_handle(run->font);
    if(!fontData)
    {
        return;
    }
    oc_harfbuzz_font* harfbuzzFont = oc_harfbuzz_font_from_handle(fontData->harfbuzzHandle);
    if(!harfbuzzFont)
    {
        return;
    }

    if(!oc_hbDrawFuncs)
    {
        oc_hbDrawFuncs = hb_draw_funcs_create();

        hb_draw_funcs_set_move_to_func(oc_hbDrawFuncs, oc_hb_move_to_func, 0, 0);
        hb_draw_funcs_set_line_to_func(oc_hbDrawFuncs, oc_hb_line_to_func, 0, 0);
        hb_draw_funcs_set_quadratic_to_func(oc_hbDrawFuncs, oc_hb_quadratic_to_func, 0, 0);
        hb_draw_funcs_set_cubic_to_func(oc_hbDrawFuncs, oc_hb_cubic_to_func, 0, 0);
    }

    f32 scale = oc_font_get_scale_for_em_pixels(run->font, fontSize);
    oc_vec2 pos = context->subPathLastPoint;

    f32 flipY = (context->textFlip) ? -1 : 1;

    for(u64 i = 0; i < run->glyphCount; i++)
    {
        oc_glyph_info* glyph = &run->glyphs[i];

        oc_hb_draw_data data = {
            .scale = scale,
            .flipY = flipY,
            .origin = {
                pos.x + glyph->offset.x * scale,
                pos.y + glyph->offset.y * scale * flipY },
        };

        hb_font_draw_glyph(harfbuzzFont->hbFont, glyph->index, oc_hbDrawFuncs, &data);

        pos.x += glyph->metrics.advance.x * scale;
        pos.y += glyph->metrics.advance.y * scale * flipY;
    }

    oc_move_to(pos.x, pos.y);

    oc_fill_rule oldRule = context->attributes.fillRule;
    context->attributes.fillRule = OC_FILL_NON_ZERO;
    oc_fill();
    context->attributes.fillRule = oldRule;
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

        elt->type = OC_PATH_LINE;
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

        elt->type = OC_PATH_QUADRATIC;

        elt->p[0].x = data->origin.x + control_x * data->scale;
        elt->p[0].y = data->origin.y - control_y * data->scale * data->flipY;
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

        elt->type = OC_PATH_CUBIC;
        elt->p[0].x = data->origin.x + control1_x * data->scale;
        elt->p[0].y = data->origin.y - control1_y * data->scale * data->flipY;
        elt->p[1].x = data->origin.x + control2_x * data->scale;
        elt->p[1].y = data->origin.y - control2_y * data->scale * data->flipY;
        elt->p[2].x = data->origin.x + to_x * data->scale;
        elt->p[2].y = data->origin.y - to_y * data->scale * data->flipY;
    }
}

oc_path_elt* oc_harfbuzz_get_curves(oc_arena* arena,
                                    oc_harfbuzz_handle handle,
                                    oc_glyph_run* run,
                                    oc_vec2 start,
                                    f32 scale,
                                    bool textFlip,
                                    u32* eltCount)
{
    *eltCount = 0;

    oc_harfbuzz_font* harfbuzzFont = oc_harfbuzz_font_from_handle(handle);
    if(!harfbuzzFont)
    {
        return 0;
    }

    if(!oc_hbDumpFuncs)
    {
        oc_hbDumpFuncs = hb_draw_funcs_create();

        hb_draw_funcs_set_move_to_func(oc_hbDumpFuncs, oc_hb_dump_move_to_func, 0, 0);
        hb_draw_funcs_set_line_to_func(oc_hbDumpFuncs, oc_hb_dump_line_to_func, 0, 0);
        hb_draw_funcs_set_quadratic_to_func(oc_hbDumpFuncs, oc_hb_dump_quadratic_to_func, 0, 0);
        hb_draw_funcs_set_cubic_to_func(oc_hbDumpFuncs, oc_hb_dump_cubic_to_func, 0, 0);
    }

    oc_arena_scope scratch = oc_scratch_begin_next(arena);

    f32 flipY = textFlip ? -1 : 1;

    oc_hb_dump_data data = {
        .pathElements = oc_arena_push_array(scratch.arena, oc_path_elt, 2 << 20),
        .eltCount = 0,
        .eltCap = 2 << 20,
        .scale = scale,
        .flipY = flipY,
    };

    oc_vec2 pos = start;

    for(u64 i = 0; i < run->glyphCount; i++)
    {
        oc_glyph_info* glyph = &run->glyphs[i];

        data.origin = (oc_vec2){
            pos.x + glyph->offset.x * scale,
            pos.y + glyph->offset.y * scale * flipY,
        };

        hb_font_draw_glyph(harfbuzzFont->hbFont, glyph->index, oc_hbDumpFuncs, &data);

        pos.x += glyph->metrics.advance.x * scale;
        pos.y += glyph->metrics.advance.y * scale * flipY;
    }

    oc_path_elt* elements = oc_arena_push_array(arena, oc_path_elt, data.eltCount);
    memcpy(elements, data.pathElements, data.eltCount * sizeof(oc_path_elt));
    *eltCount = data.eltCount;

    oc_scratch_end(scratch);

    return (elements);
}
