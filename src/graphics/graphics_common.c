/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#define _USE_MATH_DEFINES //NOTE: necessary for MSVC
#include "platform/platform.h"
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#if OC_PLATFORM_ORCA
    #define STBI_NO_STDIO
    #define STBI_NO_HDR
#endif
#include "stb/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

#include "graphics_common.h"
#include "platform/platform_debug.h"
#include "util/algebra.h"

typedef struct oc_glyph_map_entry
{
    oc_unicode_range range;
    u32 firstGlyphIndex;

} oc_glyph_map_entry;

typedef struct oc_glyph_data
{
    bool exists;
    oc_utf32 codePoint;
    oc_path_descriptor pathDescriptor;
    oc_glyph_metrics metrics;
    //...

} oc_glyph_data;

enum
{
    OC_MATRIX_STACK_MAX_DEPTH = 64,
    OC_CLIP_STACK_MAX_DEPTH = 64,
    OC_MAX_PATH_ELEMENT_COUNT = 2 << 20,
    OC_MAX_PRIMITIVE_COUNT = 8 << 10
};

typedef struct oc_font_data
{
    oc_list_elt freeListElt;

    u32 rangeCount;
    u32 glyphCount;
    u32 outlineCount;
    oc_glyph_map_entry* glyphMap;
    oc_glyph_data* glyphs;
    oc_path_elt* outlines;

    f32 unitsPerEm;
    oc_font_metrics metrics;

} oc_font_data;

typedef struct oc_canvas_data oc_canvas_data;

typedef enum oc_graphics_handle_kind
{
    OC_GRAPHICS_HANDLE_NONE = 0,
    OC_GRAPHICS_HANDLE_SURFACE,
    OC_GRAPHICS_HANDLE_CANVAS,
    OC_GRAPHICS_HANDLE_FONT,
    OC_GRAPHICS_HANDLE_IMAGE,
    OC_GRAPHICS_HANDLE_SURFACE_SERVER,
} oc_graphics_handle_kind;

typedef struct oc_graphics_handle_slot
{
    oc_list_elt freeListElt;
    u32 generation;
    oc_graphics_handle_kind kind;

    void* data;

} oc_graphics_handle_slot;

enum
{
    OC_GRAPHICS_HANDLES_MAX_COUNT = 512
};

typedef struct oc_graphics_data
{
    bool init;

    oc_graphics_handle_slot handleArray[OC_GRAPHICS_HANDLES_MAX_COUNT];
    int handleNextIndex;
    oc_list handleFreeList;

    oc_arena resourceArena;
    oc_list canvasFreeList;
    oc_list fontFreeList;

} oc_graphics_data;

typedef struct oc_canvas_data
{
    oc_list_elt freeListElt;

    oc_attributes attributes;
    bool textFlip;

    oc_path_elt pathElements[OC_MAX_PATH_ELEMENT_COUNT];
    oc_path_descriptor path;
    oc_vec2 subPathStartPoint;
    oc_vec2 subPathLastPoint;

    oc_mat2x3 matrixStack[OC_MATRIX_STACK_MAX_DEPTH];
    u32 matrixStackSize;

    oc_rect clipStack[OC_CLIP_STACK_MAX_DEPTH];
    u32 clipStackSize;

    u32 primitiveCount;
    oc_primitive primitives[OC_MAX_PRIMITIVE_COUNT];

    //NOTE: these are used at render time
    oc_color clearColor;

    oc_vec4 shapeExtents;
    oc_vec4 shapeScreenExtents;

} oc_canvas_data;

static oc_graphics_data oc_graphicsData = { 0 };

void oc_graphics_init()
{
    if(!oc_graphicsData.init)
    {
        oc_graphicsData.handleNextIndex = 0;
        oc_arena_init(&oc_graphicsData.resourceArena);
        oc_graphicsData.init = true;
    }
}

//------------------------------------------------------------------------
// handle pools procedures
//------------------------------------------------------------------------

u64 oc_graphics_handle_alloc(oc_graphics_handle_kind kind, void* data)
{
    if(!oc_graphicsData.init)
    {
        oc_graphics_init();
    }

    oc_graphics_handle_slot* slot = oc_list_pop_entry(&oc_graphicsData.handleFreeList, oc_graphics_handle_slot, freeListElt);
    if(!slot && oc_graphicsData.handleNextIndex < OC_GRAPHICS_HANDLES_MAX_COUNT)
    {
        slot = &oc_graphicsData.handleArray[oc_graphicsData.handleNextIndex];
        oc_graphicsData.handleNextIndex++;

        slot->generation = 1;
    }
    u64 h = 0;
    if(slot)
    {
        slot->kind = kind;
        slot->data = data;

        h = ((u64)(slot - oc_graphicsData.handleArray)) << 32
          | ((u64)(slot->generation));
    }
    return (h);
}

void oc_graphics_handle_recycle(u64 h)
{
    OC_DEBUG_ASSERT(oc_graphicsData.init);

    u32 index = h >> 32;
    u32 generation = h & 0xffffffff;

    if(index * sizeof(oc_graphics_handle_slot) < oc_graphicsData.handleNextIndex)
    {
        oc_graphics_handle_slot* slot = &oc_graphicsData.handleArray[index];
        if(slot->generation == generation)
        {
            OC_DEBUG_ASSERT(slot->generation != UINT32_MAX, "surface slot generation wrap around\n");
            slot->generation++;
            oc_list_push(&oc_graphicsData.handleFreeList, &slot->freeListElt);
        }
    }
}

void* oc_graphics_data_from_handle(oc_graphics_handle_kind kind, u64 h)
{
    OC_DEBUG_ASSERT(oc_graphicsData.init);

    void* data = 0;

    u32 index = h >> 32;
    u32 generation = h & 0xffffffff;

    if(index < oc_graphicsData.handleNextIndex)
    {
        oc_graphics_handle_slot* slot = &oc_graphicsData.handleArray[index];
        if(slot->generation == generation
           && slot->kind == kind)
        {
            data = slot->data;
        }
    }
    return (data);
}

//------------------------------------------------------------------------------------------
//NOTE(martin): surface nil and is nil
//------------------------------------------------------------------------------------------

oc_surface oc_surface_nil() { return ((oc_surface){ .h = 0 }); }

bool oc_surface_is_nil(oc_surface surface) { return (surface.h == 0); }

//------------------------------------------------------------------------------------------
//NOTE(martin): graphics canvas internal
//------------------------------------------------------------------------------------------

oc_thread_local oc_canvas_data* __mgCurrentCanvas = 0;
oc_thread_local oc_canvas __mgCurrentCanvasHandle = { 0 };

bool oc_vec2_close(oc_vec2 p0, oc_vec2 p1, f32 tolerance)
{
    f32 norm2 = (p1.x - p0.x) * (p1.x - p0.x) + (p1.y - p0.y) * (p1.y - p0.y);
    return (fabs(norm2) < tolerance);
}

oc_mat2x3 oc_matrix_stack_top(oc_canvas_data* canvas)
{
    if(canvas->matrixStackSize == 0)
    {
        return ((oc_mat2x3){ 1, 0, 0,
                             0, 1, 0 });
    }
    else
    {
        return (canvas->matrixStack[canvas->matrixStackSize - 1]);
    }
}

void oc_matrix_stack_push(oc_canvas_data* canvas, oc_mat2x3 transform)
{
    if(canvas->matrixStackSize >= OC_MATRIX_STACK_MAX_DEPTH)
    {
        oc_log_error("matrix stack overflow\n");
    }
    else
    {
        canvas->matrixStack[canvas->matrixStackSize] = transform;
        canvas->matrixStackSize++;
    }
}

void oc_matrix_stack_pop(oc_canvas_data* canvas)
{
    if(canvas->matrixStackSize == 0)
    {
        oc_log_error("matrix stack underflow\n");
    }
    else
    {
        canvas->matrixStackSize--;
        oc_matrix_stack_top(canvas);
    }
}

oc_rect oc_clip_stack_top(oc_canvas_data* canvas)
{
    if(canvas->clipStackSize == 0)
    {
        return ((oc_rect){ -FLT_MAX / 2, -FLT_MAX / 2, FLT_MAX, FLT_MAX });
    }
    else
    {
        return (canvas->clipStack[canvas->clipStackSize - 1]);
    }
}

void oc_clip_stack_push(oc_canvas_data* canvas, oc_rect clip)
{
    if(canvas->clipStackSize >= OC_CLIP_STACK_MAX_DEPTH)
    {
        oc_log_error("clip stack overflow\n");
    }
    else
    {
        canvas->clipStack[canvas->clipStackSize] = clip;
        canvas->clipStackSize++;
    }
}

void oc_clip_stack_pop(oc_canvas_data* canvas)
{
    if(canvas->clipStackSize == 0)
    {
        oc_log_error("clip stack underflow\n");
    }
    else
    {
        canvas->clipStackSize--;
    }
}

void oc_push_command(oc_canvas_data* canvas, oc_primitive primitive)
{
    //NOTE(martin): push primitive and updates current stream, eventually patching a pending jump.
    OC_ASSERT(canvas->primitiveCount < OC_MAX_PRIMITIVE_COUNT);

    canvas->primitives[canvas->primitiveCount] = primitive;
    canvas->primitives[canvas->primitiveCount].attributes = canvas->attributes;
    canvas->primitives[canvas->primitiveCount].attributes.transform = oc_matrix_stack_top(canvas);
    canvas->primitives[canvas->primitiveCount].attributes.clip = oc_clip_stack_top(canvas);
    canvas->primitiveCount++;
}

void oc_new_path(oc_canvas_data* canvas)
{
    canvas->path.startIndex += canvas->path.count;
    canvas->path.count = 0;
    canvas->subPathStartPoint = canvas->subPathLastPoint;
    canvas->path.startPoint = canvas->subPathStartPoint;
}

void oc_path_push_elements(oc_canvas_data* canvas, u32 count, oc_path_elt* elements)
{
    OC_ASSERT(canvas->path.count + canvas->path.startIndex + count < OC_MAX_PATH_ELEMENT_COUNT);
    memcpy(canvas->pathElements + canvas->path.startIndex + canvas->path.count, elements, count * sizeof(oc_path_elt));
    canvas->path.count += count;

    OC_ASSERT(canvas->path.count < OC_MAX_PATH_ELEMENT_COUNT);
}

void oc_path_push_element(oc_canvas_data* canvas, oc_path_elt elt)
{
    oc_path_push_elements(canvas, 1, &elt);
}

//------------------------------------------------------------------------------------------
//NOTE(martin): fonts
//------------------------------------------------------------------------------------------

oc_font oc_font_nil() { return ((oc_font){ .h = 0 }); }

bool oc_font_is_nil(oc_font font) { return (font.h == 0); }

oc_font oc_font_handle_alloc(oc_font_data* font)
{
    oc_font handle = { .h = oc_graphics_handle_alloc(OC_GRAPHICS_HANDLE_FONT, (void*)font) };
    return (handle);
}

oc_font_data* oc_font_data_from_handle(oc_font handle)
{
    oc_font_data* data = oc_graphics_data_from_handle(OC_GRAPHICS_HANDLE_FONT, handle.h);
    return (data);
}

oc_font oc_font_create_from_memory(oc_str8 mem, u32 rangeCount, oc_unicode_range* ranges)
{
    if(!oc_graphicsData.init)
    {
        oc_graphics_init();
    }
    oc_font fontHandle = oc_font_nil();

    oc_font_data* font = oc_list_pop_entry(&oc_graphicsData.fontFreeList, oc_font_data, freeListElt);
    if(!font)
    {
        font = oc_arena_push_type(&oc_graphicsData.resourceArena, oc_font_data);
    }
    if(font)
    {
        memset(font, 0, sizeof(oc_font_data));
        fontHandle = oc_font_handle_alloc(font);

        stbtt_fontinfo stbttFontInfo;
        stbtt_InitFont(&stbttFontInfo, (byte*)mem.ptr, 0);

        //NOTE(martin): load font metrics data
        font->unitsPerEm = 1. / stbtt_ScaleForMappingEmToPixels(&stbttFontInfo, 1);

        int ascent, descent, lineGap, x0, x1, y0, y1;
        stbtt_GetFontVMetrics(&stbttFontInfo, &ascent, &descent, &lineGap);
        stbtt_GetFontBoundingBox(&stbttFontInfo, &x0, &y0, &x1, &y1);

        font->metrics.ascent = ascent;
        font->metrics.descent = -descent;
        font->metrics.lineGap = lineGap;
        font->metrics.width = x1 - x0;

        stbtt_GetCodepointBox(&stbttFontInfo, 'x', &x0, &y0, &x1, &y1);
        font->metrics.xHeight = y1 - y0;

        stbtt_GetCodepointBox(&stbttFontInfo, 'M', &x0, &y0, &x1, &y1);
        font->metrics.capHeight = y1 - y0;

        //NOTE(martin): load codepoint ranges
        font->rangeCount = rangeCount;
        font->glyphMap = oc_malloc_array(oc_glyph_map_entry, rangeCount);
        font->glyphCount = 0;

        for(int i = 0; i < rangeCount; i++)
        {
            //NOTE(martin): initialize the map entry.
            //              The glyph indices are offseted by 1, to reserve 0 as an invalid glyph index.
            font->glyphMap[i].range = ranges[i];
            font->glyphMap[i].firstGlyphIndex = font->glyphCount + 1;
            font->glyphCount += ranges[i].count;
        }

        font->glyphs = oc_malloc_array(oc_glyph_data, font->glyphCount);

        //NOTE(martin): first do a count of outlines
        int outlineCount = 0;
        for(int rangeIndex = 0; rangeIndex < rangeCount; rangeIndex++)
        {
            oc_utf32 codePoint = font->glyphMap[rangeIndex].range.firstCodePoint;
            u32 firstGlyphIndex = font->glyphMap[rangeIndex].firstGlyphIndex;
            u32 endGlyphIndex = firstGlyphIndex + font->glyphMap[rangeIndex].range.count;

            for(int glyphIndex = firstGlyphIndex;
                glyphIndex < endGlyphIndex; glyphIndex++)
            {
                int stbttGlyphIndex = stbtt_FindGlyphIndex(&stbttFontInfo, codePoint);
                if(stbttGlyphIndex == 0)
                {
                    //NOTE(martin): the codepoint is not found in the font
                    codePoint++;
                    continue;
                }
                //NOTE(martin): load glyph outlines
                stbtt_vertex* vertices = 0;
                outlineCount += stbtt_GetGlyphShape(&stbttFontInfo, stbttGlyphIndex, &vertices);
                stbtt_FreeShape(&stbttFontInfo, vertices);
                codePoint++;
            }
        }
        //NOTE(martin): allocate outlines
        font->outlines = oc_malloc_array(oc_path_elt, outlineCount);
        font->outlineCount = 0;

        //NOTE(martin): load metrics and outlines
        for(int rangeIndex = 0; rangeIndex < rangeCount; rangeIndex++)
        {
            oc_utf32 codePoint = font->glyphMap[rangeIndex].range.firstCodePoint;
            u32 firstGlyphIndex = font->glyphMap[rangeIndex].firstGlyphIndex;
            u32 endGlyphIndex = firstGlyphIndex + font->glyphMap[rangeIndex].range.count;

            for(int glyphIndex = firstGlyphIndex;
                glyphIndex < endGlyphIndex; glyphIndex++)
            {
                oc_glyph_data* glyph = &(font->glyphs[glyphIndex - 1]);

                int stbttGlyphIndex = stbtt_FindGlyphIndex(&stbttFontInfo, codePoint);
                if(stbttGlyphIndex == 0)
                {
                    //NOTE(martin): the codepoint is not found in the font, we zero the glyph info
                    memset(glyph, 0, sizeof(*glyph));
                    codePoint++;
                    continue;
                }

                glyph->exists = true;
                glyph->codePoint = codePoint;

                //NOTE(martin): load glyph metric
                int xAdvance, xBearing, x0, y0, x1, y1;
                stbtt_GetGlyphHMetrics(&stbttFontInfo, stbttGlyphIndex, &xAdvance, &xBearing);
                stbtt_GetGlyphBox(&stbttFontInfo, stbttGlyphIndex, &x0, &y0, &x1, &y1);

                //NOTE(martin): stb stbtt_GetGlyphBox returns bottom left and top right corners, with y up,
                //              so we have to set .y = -y1
                glyph->metrics.ink = (oc_rect){
                    .x = x0,
                    .y = -y1,
                    .w = x1 - x0,
                    .h = y1 - y0
                };

                glyph->metrics.advance = (oc_vec2){ xAdvance, 0 };

                //NOTE(martin): load glyph outlines

                stbtt_vertex* vertices = 0;
                int vertexCount = stbtt_GetGlyphShape(&stbttFontInfo, stbttGlyphIndex, &vertices);

                glyph->pathDescriptor = (oc_path_descriptor){ .startIndex = font->outlineCount,
                                                              .count = vertexCount,
                                                              .startPoint = { 0, 0 } };

                oc_path_elt* elements = font->outlines + font->outlineCount;
                font->outlineCount += vertexCount;
                oc_vec2 currentPos = { 0, 0 };

                for(int vertIndex = 0; vertIndex < vertexCount; vertIndex++)
                {
                    f32 x = vertices[vertIndex].x;
                    f32 y = vertices[vertIndex].y;
                    f32 cx = vertices[vertIndex].cx;
                    f32 cy = vertices[vertIndex].cy;
                    f32 cx1 = vertices[vertIndex].cx1;
                    f32 cy1 = vertices[vertIndex].cy1;

                    switch(vertices[vertIndex].type)
                    {
                        case STBTT_vmove:
                            elements[vertIndex].type = OC_PATH_MOVE;
                            elements[vertIndex].p[0] = (oc_vec2){ x, y };
                            break;

                        case STBTT_vline:
                            elements[vertIndex].type = OC_PATH_LINE;
                            elements[vertIndex].p[0] = (oc_vec2){ x, y };
                            break;

                        case STBTT_vcurve:
                        {
                            elements[vertIndex].type = OC_PATH_QUADRATIC;
                            elements[vertIndex].p[0] = (oc_vec2){ cx, cy };
                            elements[vertIndex].p[1] = (oc_vec2){ x, y };
                        }
                        break;

                        case STBTT_vcubic:
                            elements[vertIndex].type = OC_PATH_CUBIC;
                            elements[vertIndex].p[0] = (oc_vec2){ cx, cy };
                            elements[vertIndex].p[1] = (oc_vec2){ cx1, cy1 };
                            elements[vertIndex].p[2] = (oc_vec2){ x, y };
                            break;
                    }
                    currentPos = (oc_vec2){ x, y };
                }
                stbtt_FreeShape(&stbttFontInfo, vertices);
                codePoint++;
            }
        }
    }
    return (fontHandle);
}

oc_font oc_font_create_from_file(oc_file file, u32 rangeCount, oc_unicode_range* ranges)
{
    oc_font font = oc_font_nil();
    oc_arena_scope scratch = oc_scratch_begin();

    u64 size = oc_file_size(file);
    char* buffer = oc_arena_push(scratch.arena, size);
    u64 read = oc_file_read(file, size, buffer);

    if(read != size)
    {
        oc_log_error("Couldn't read font data\n");
    }
    else
    {
        font = oc_font_create_from_memory(oc_str8_from_buffer(size, buffer), rangeCount, ranges);
    }

    oc_scratch_end(scratch);
    return (font);
}

oc_font oc_font_create_from_path(oc_str8 path, u32 rangeCount, oc_unicode_range* ranges)
{
    oc_font font = oc_font_nil();

    oc_file file = oc_file_open(path, OC_FILE_ACCESS_READ, OC_FILE_OPEN_NONE);
    if(oc_file_last_error(file) != OC_IO_OK)
    {
        oc_log_error("Could not open file %*.s\n", oc_str8_ip(path));
    }
    else
    {
        font = oc_font_create_from_file(file, rangeCount, ranges);
    }
    oc_file_close(file);

    return (font);
}

void oc_font_destroy(oc_font fontHandle)
{
    oc_font_data* fontData = oc_font_data_from_handle(fontHandle);
    if(fontData)
    {
        free(fontData->glyphMap);
        free(fontData->glyphs);
        free(fontData->outlines);

        oc_list_push(&oc_graphicsData.fontFreeList, &fontData->freeListElt);
        oc_graphics_handle_recycle(fontHandle.h);
    }
}

oc_str32 oc_font_get_glyph_indices_from_font_data(oc_font_data* fontData, oc_str32 codePoints, oc_str32 backing)
{
    u64 count = oc_min(codePoints.len, backing.len);

    for(int i = 0; i < count; i++)
    {
        u32 glyphIndex = 0;
        for(int rangeIndex = 0; rangeIndex < fontData->rangeCount; rangeIndex++)
        {
            if(codePoints.ptr[i] >= fontData->glyphMap[rangeIndex].range.firstCodePoint
               && codePoints.ptr[i] < (fontData->glyphMap[rangeIndex].range.firstCodePoint + fontData->glyphMap[rangeIndex].range.count))
            {
                u32 rangeOffset = codePoints.ptr[i] - fontData->glyphMap[rangeIndex].range.firstCodePoint;
                glyphIndex = fontData->glyphMap[rangeIndex].firstGlyphIndex + rangeOffset;
                break;
            }
        }
        if(glyphIndex && !fontData->glyphs[glyphIndex].exists)
        {
            backing.ptr[i] = 0;
        }
        backing.ptr[i] = glyphIndex;
    }
    oc_str32 res = { .ptr = backing.ptr, .len = count };
    return (res);
}

u32 oc_font_get_glyph_index_from_font_data(oc_font_data* fontData, oc_utf32 codePoint)
{
    u32 glyphIndex = 0;
    oc_str32 codePoints = { .ptr = &codePoint, .len = 1 };
    oc_str32 backing = { .ptr = &glyphIndex, 1 };
    oc_font_get_glyph_indices_from_font_data(fontData, codePoints, backing);
    return (glyphIndex);
}

oc_str32 oc_font_get_glyph_indices(oc_font font, oc_str32 codePoints, oc_str32 backing)
{
    oc_font_data* fontData = oc_font_data_from_handle(font);
    if(!fontData)
    {
        return ((oc_str32){ 0 });
    }
    return (oc_font_get_glyph_indices_from_font_data(fontData, codePoints, backing));
}

oc_str32 oc_font_push_glyph_indices(oc_arena* arena, oc_font font, oc_str32 codePoints)
{
    u32* buffer = oc_arena_push_array(arena, u32, codePoints.len);
    oc_str32 backing = { .ptr = buffer, .len = codePoints.len };
    return (oc_font_get_glyph_indices(font, codePoints, backing));
}

u32 oc_font_get_glyph_index(oc_font font, oc_utf32 codePoint)
{
    u32 glyphIndex = 0;
    oc_str32 codePoints = { .ptr = &codePoint, .len = 1 };
    oc_str32 backing = { .ptr = &glyphIndex, .len = 1 };
    oc_font_get_glyph_indices(font, codePoints, backing);
    return (glyphIndex);
}

oc_glyph_data* oc_font_get_glyph_data(oc_font_data* fontData, u32 glyphIndex)
{
    OC_DEBUG_ASSERT(glyphIndex);
    OC_DEBUG_ASSERT(glyphIndex < fontData->glyphCount);
    return (&(fontData->glyphs[glyphIndex - 1]));
}

oc_font_metrics oc_font_get_metrics_unscaled(oc_font font)
{
    oc_font_data* fontData = oc_font_data_from_handle(font);
    if(!fontData)
    {
        return ((oc_font_metrics){ 0 });
    }
    return (fontData->metrics);
}

oc_font_metrics oc_font_get_metrics(oc_font font, f32 emSize)
{
    oc_font_data* fontData = oc_font_data_from_handle(font);
    if(!fontData)
    {
        return ((oc_font_metrics){ 0 });
    }
    f32 scale = emSize / fontData->unitsPerEm;
    oc_font_metrics metrics = fontData->metrics;

    metrics.ascent *= scale;
    metrics.descent *= scale;
    metrics.lineGap *= scale;
    metrics.xHeight *= scale;
    metrics.capHeight *= scale;
    metrics.width *= scale;

    return (metrics);
}

f32 oc_font_get_scale_for_em_pixels(oc_font font, f32 emSize)
{
    oc_font_data* fontData = oc_font_data_from_handle(font);
    if(!fontData)
    {
        return (0);
    }
    return (emSize / fontData->unitsPerEm);
}

////////////////////////////////////////////////////////////////////////////////////////////::
void oc_font_get_glyph_metrics_from_font_data(oc_font_data* fontData,
                                              oc_str32 glyphIndices,
                                              oc_glyph_metrics* outMetrics)
{
    for(int i = 0; i < glyphIndices.len; i++)
    {
        if(!glyphIndices.ptr[i] || glyphIndices.ptr[i] >= fontData->glyphCount)
        {
            continue;
        }
        oc_glyph_data* glyph = oc_font_get_glyph_data(fontData, glyphIndices.ptr[i]);
        outMetrics[i] = glyph->metrics;
    }
}

/*
int oc_font_get_glyph_metrics(oc_font font, oc_str32 glyphIndices, oc_text_metrics* outMetrics)
{
    oc_font_data* fontData = oc_font_data_from_handle(font);
    if(!fontData)
    {
        return (-1);
    }
    oc_font_get_glyph_metrics_from_font_data(fontData, glyphIndices, outMetrics);
    return (0);
}

int oc_font_get_codepoint_metrics(oc_font font, oc_utf32 codePoint, oc_text_metrics* outMetrics)
{
    oc_font_data* fontData = oc_font_data_from_handle(font);
    if(!fontData)
    {
        return (-1);
    }
    u32 glyphIndex = 0;
    oc_str32 codePoints = { 1, &codePoint };
    oc_str32 backing = { 1, &glyphIndex };
    oc_str32 glyphs = oc_font_get_glyph_indices_from_font_data(fontData, codePoints, backing);
    oc_font_get_glyph_metrics_from_font_data(fontData, glyphs, outMetrics);
    return (0);
}
*/
/////////////////////////////////////////////////

oc_text_metrics oc_font_text_metrics_utf32(oc_font font, f32 fontSize, oc_str32 codePoints)
{
    if(!codePoints.len || !codePoints.ptr)
    {
        return ((oc_text_metrics){ 0 });
    }

    oc_font_data* fontData = oc_font_data_from_handle(font);
    if(!fontData)
    {
        return ((oc_text_metrics){ 0 });
    }

    oc_arena_scope scratch = oc_scratch_begin();
    oc_str32 glyphIndices = oc_font_push_glyph_indices(scratch.arena, font, codePoints);

    //NOTE(martin): find width of missing character
    //TODO(martin): should cache that at font creation...
    oc_glyph_metrics missingGlyphMetrics = { 0 };
    u32 missingGlyphIndex = oc_font_get_glyph_index_from_font_data(fontData, 0xfffd);

    if(missingGlyphIndex)
    {
        oc_font_get_glyph_metrics_from_font_data(fontData, (oc_str32){ .ptr = &missingGlyphIndex, .len = 1 }, &missingGlyphMetrics);
    }
    else
    {
        //NOTE(martin): could not find replacement glyph, try to get an 'X' to get a somewhat correct dimensions
        //              to render an empty rectangle. Otherwise just render with the max font width

        missingGlyphIndex = oc_font_get_glyph_index_from_font_data(fontData, 'X');
        if(missingGlyphIndex)
        {
            oc_font_get_glyph_metrics_from_font_data(fontData, (oc_str32){ .ptr = &missingGlyphIndex, .len = 1 }, &missingGlyphMetrics);
        }
        else
        {
            missingGlyphMetrics = (oc_glyph_metrics){
                .ink = {
                    .x = fontData->metrics.width * 0.1,
                    .y = -fontData->metrics.ascent,
                    .w = fontData->metrics.width * 0.8,
                    .h = fontData->metrics.ascent,
                },
                .advance = { .x = fontData->metrics.width, .y = 0 },
            };
        }
    }

    //NOTE(martin): accumulate text extents
    oc_text_metrics metrics = { 0 };
    f32 lineHeight = fontData->metrics.descent + fontData->metrics.ascent + fontData->metrics.lineGap;

    if(glyphIndices.len)
    {
        metrics.logical.y = -(fontData->metrics.ascent + fontData->metrics.lineGap);
        metrics.logical.h += lineHeight + fontData->metrics.lineGap;
    }

    f32 inkX0 = 0, inkX1 = 0, inkY0 = 0, inkY1 = 0;

    for(int i = 0; i < glyphIndices.len; i++)
    {
        //TODO(martin): make it failsafe for fonts that don't have a glyph for the line-feed codepoint ?

        oc_glyph_data* glyph = 0;
        oc_glyph_metrics glyphMetrics;
        if(!glyphIndices.ptr[i] || glyphIndices.ptr[i] >= fontData->glyphCount)
        {
            glyphMetrics = missingGlyphMetrics;
        }
        else
        {
            glyph = oc_font_get_glyph_data(fontData, glyphIndices.ptr[i]);
            glyphMetrics = glyph->metrics;
        }

        inkX0 = oc_min(inkX0, metrics.advance.x + glyphMetrics.ink.x);
        inkX1 = oc_max(inkX1, metrics.advance.x + glyphMetrics.ink.x + glyphMetrics.ink.w);

        inkY0 = oc_min(inkY0, metrics.advance.y + glyphMetrics.ink.y);
        inkY1 = oc_max(inkY1, metrics.advance.y + glyphMetrics.ink.y + glyphMetrics.ink.h);

        metrics.advance.x += glyphMetrics.advance.x;
        metrics.advance.y += glyphMetrics.advance.y;

        metrics.logical.w = oc_max(metrics.logical.w, metrics.advance.x);

        if(glyph && glyph->codePoint == '\n')
        {
            metrics.advance.y += lineHeight;
            metrics.advance.x = 0;

            if(i < glyphIndices.len - 1)
            {
                metrics.logical.h += lineHeight;
            }
        }
    }
    metrics.ink = (oc_rect){
        inkX0,
        inkY0,
        inkX1 - inkX0,
        inkY1 - inkY0
    };

    OC_ASSERT(metrics.ink.y <= 0);

    f32 fontScale = oc_font_get_scale_for_em_pixels(font, fontSize);

    metrics.ink.x *= fontScale;
    metrics.ink.y *= fontScale;
    metrics.ink.w *= fontScale;
    metrics.ink.h *= fontScale;
    metrics.logical.x *= fontScale;
    metrics.logical.y *= fontScale;
    metrics.logical.w *= fontScale;
    metrics.logical.h *= fontScale;
    metrics.advance.x *= fontScale;
    metrics.advance.y *= fontScale;

    oc_scratch_end(scratch);
    return (metrics);
}

oc_text_metrics oc_font_text_metrics(oc_font font, f32 fontSize, oc_str8 text)
{
    if(!text.len || !text.ptr)
    {
        return ((oc_text_metrics){ 0 });
    }

    oc_arena_scope scratch = oc_scratch_begin();
    oc_str32 codePoints = oc_utf8_push_to_codepoints(scratch.arena, text);
    oc_text_metrics result = oc_font_text_metrics_utf32(font, fontSize, codePoints);
    oc_scratch_end(scratch);
    return (result);
}

//------------------------------------------------------------------------------------------
//NOTE(martin): graphics canvas API
//------------------------------------------------------------------------------------------

oc_canvas oc_canvas_nil() { return ((oc_canvas){ .h = 0 }); }

bool oc_canvas_is_nil(oc_canvas canvas) { return (canvas.h == 0); }

oc_canvas oc_canvas_handle_alloc(oc_canvas_data* canvas)
{
    oc_canvas handle = { .h = oc_graphics_handle_alloc(OC_GRAPHICS_HANDLE_CANVAS, (void*)canvas) };
    return (handle);
}

oc_canvas_data* oc_canvas_data_from_handle(oc_canvas handle)
{
    oc_canvas_data* data = oc_graphics_data_from_handle(OC_GRAPHICS_HANDLE_CANVAS, handle.h);
    return (data);
}

oc_canvas oc_canvas_create()
{
    if(!oc_graphicsData.init)
    {
        oc_graphics_init();
    }

    oc_canvas canvasHandle = oc_canvas_nil();
    oc_canvas_data* canvas = oc_list_pop_entry(&oc_graphicsData.canvasFreeList, oc_canvas_data, freeListElt);
    if(!canvas)
    {
        canvas = oc_arena_push_type(&oc_graphicsData.resourceArena, oc_canvas_data);
    }
    if(canvas)
    {
        canvas->textFlip = false;
        canvas->path = (oc_path_descriptor){ 0 };
        canvas->matrixStackSize = 0;
        canvas->clipStackSize = 0;
        canvas->primitiveCount = 0;
        canvas->clearColor = (oc_color){ 0, 0, 0, 0 };

        canvas->attributes = (oc_attributes){ 0 };
        canvas->attributes.color = (oc_color){ 0, 0, 0, 1 };
        canvas->attributes.tolerance = 1;
        canvas->attributes.width = 10;
        canvas->attributes.clip = (oc_rect){ -FLT_MAX / 2, -FLT_MAX / 2, FLT_MAX, FLT_MAX };

        canvasHandle = oc_canvas_handle_alloc(canvas);

        oc_canvas_select(canvasHandle);
    }
    return (canvasHandle);
}

void oc_canvas_destroy(oc_canvas handle)
{
    oc_canvas_data* canvas = oc_canvas_data_from_handle(handle);
    if(canvas)
    {
        if(__mgCurrentCanvas == canvas)
        {
            __mgCurrentCanvas = 0;
            __mgCurrentCanvasHandle = oc_canvas_nil();
        }
        oc_list_push(&oc_graphicsData.canvasFreeList, &canvas->freeListElt);
        oc_graphics_handle_recycle(handle.h);
    }
}

oc_canvas oc_canvas_select(oc_canvas canvas)
{
    oc_canvas old = __mgCurrentCanvasHandle;
    __mgCurrentCanvasHandle = canvas;
    __mgCurrentCanvas = oc_canvas_data_from_handle(canvas);
    return (old);
}

void oc_render(oc_canvas canvas)
{
    oc_surface selectedSurface = oc_surface_get_selected();
    oc_canvas_data* canvasData = oc_canvas_data_from_handle(canvas);
    if(canvasData && !oc_surface_is_nil(selectedSurface))
    {
        int eltCount = canvasData->path.startIndex + canvasData->path.count;
        oc_surface_render_commands(selectedSurface,
                                   canvasData->clearColor,
                                   canvasData->primitiveCount,
                                   canvasData->primitives,
                                   eltCount,
                                   canvasData->pathElements);

        canvasData->primitiveCount = 0;
        canvasData->path.startIndex = 0;
        canvasData->path.count = 0;
    }
}

//------------------------------------------------------------------------------------------
//NOTE(martin): transform, viewport and clipping
//------------------------------------------------------------------------------------------

void oc_matrix_push(oc_mat2x3 matrix)
{
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        oc_matrix_stack_push(canvas, matrix);
    }
}

void oc_matrix_multiply_push(oc_mat2x3 matrix)
{
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        oc_mat2x3 transform = oc_matrix_stack_top(canvas);
        oc_matrix_stack_push(canvas, oc_mat2x3_mul_m(transform, matrix));
    }
}

void oc_matrix_pop()
{
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        oc_matrix_stack_pop(canvas);
    }
}

oc_mat2x3 oc_matrix_top()
{
    oc_mat2x3 mat = {
        1, 0, 0,
        0, 1, 0
    };
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        mat = oc_matrix_stack_top(canvas);
    }

    return (mat);
}

void oc_clip_push(f32 x, f32 y, f32 w, f32 h)
{
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        oc_rect clip = { x, y, w, h };

        //NOTE(martin): transform clip
        oc_mat2x3 transform = oc_matrix_stack_top(canvas);
        oc_vec2 p0 = oc_mat2x3_mul(transform, (oc_vec2){ clip.x, clip.y });
        oc_vec2 p1 = oc_mat2x3_mul(transform, (oc_vec2){ clip.x + clip.w, clip.y });
        oc_vec2 p2 = oc_mat2x3_mul(transform, (oc_vec2){ clip.x + clip.w, clip.y + clip.h });
        oc_vec2 p3 = oc_mat2x3_mul(transform, (oc_vec2){ clip.x, clip.y + clip.h });

        f32 x0 = oc_min(p0.x, oc_min(p1.x, oc_min(p2.x, p3.x)));
        f32 y0 = oc_min(p0.y, oc_min(p1.y, oc_min(p2.y, p3.y)));
        f32 x1 = oc_max(p0.x, oc_max(p1.x, oc_max(p2.x, p3.x)));
        f32 y1 = oc_max(p0.y, oc_max(p1.y, oc_max(p2.y, p3.y)));

        oc_rect current = oc_clip_stack_top(canvas);

        //NOTE(martin): intersect with current clip
        x0 = oc_max(current.x, x0);
        y0 = oc_max(current.y, y0);
        x1 = oc_min(current.x + current.w, x1);
        y1 = oc_min(current.y + current.h, y1);

        oc_rect r = { x0, y0, oc_max(0, x1 - x0), oc_max(0, y1 - y0) };
        oc_clip_stack_push(canvas, r);

        canvas->attributes.clip = r;
    }
}

void oc_clip_pop()
{
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        oc_clip_stack_pop(canvas);
        canvas->attributes.clip = oc_clip_stack_top(canvas);
    }
}

oc_rect oc_clip_top()
{
    oc_rect clip = { -FLT_MAX / 2, -FLT_MAX / 2, FLT_MAX, FLT_MAX };

    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        clip = oc_clip_stack_top(canvas);
    }

    return (clip);
}

//------------------------------------------------------------------------------------------
//NOTE(martin): graphics attributes setting/getting
//------------------------------------------------------------------------------------------
void oc_set_color(oc_color color)
{
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        canvas->attributes.color = color;
    }
}

void oc_set_color_rgba(f32 r, f32 g, f32 b, f32 a)
{
    oc_set_color((oc_color){ r, g, b, a });
}

void oc_set_width(f32 width)
{
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        canvas->attributes.width = width;
    }
}

void oc_set_tolerance(f32 tolerance)
{
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        canvas->attributes.tolerance = tolerance;
    }
}

void oc_set_joint(oc_joint_type joint)
{
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        canvas->attributes.joint = joint;
    }
}

void oc_set_max_joint_excursion(f32 maxJointExcursion)
{
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        canvas->attributes.maxJointExcursion = maxJointExcursion;
    }
}

void oc_set_cap(oc_cap_type cap)
{
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        canvas->attributes.cap = cap;
    }
}

void oc_set_font(oc_font font)
{
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        canvas->attributes.font = font;
    }
}

void oc_set_font_size(f32 fontSize)
{
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        canvas->attributes.fontSize = fontSize;
    }
}

void oc_set_text_flip(bool flip)
{
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        canvas->textFlip = flip;
    }
}

void oc_set_image(oc_image image)
{
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        canvas->attributes.image = image;
        oc_vec2 size = oc_image_size(image);
        canvas->attributes.srcRegion = (oc_rect){ 0, 0, size.x, size.y };
    }
}

void oc_set_image_source_region(oc_rect region)
{
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        canvas->attributes.srcRegion = region;
    }
}

oc_color oc_get_color()
{
    oc_color color = { 0 };
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        color = canvas->attributes.color;
    }
    return (color);
}

f32 oc_get_width()
{
    f32 width = 0;
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        width = canvas->attributes.width;
    }
    return (width);
}

f32 oc_get_tolerance()
{
    f32 tolerance = 0;
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        tolerance = canvas->attributes.tolerance;
    }
    return (tolerance);
}

oc_joint_type oc_get_joint()
{
    oc_joint_type joint = 0;
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        joint = canvas->attributes.joint;
    }
    return (joint);
}

f32 oc_get_max_joint_excursion()
{
    f32 maxJointExcursion = 0;
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        maxJointExcursion = canvas->attributes.maxJointExcursion;
    }
    return (maxJointExcursion);
}

oc_cap_type oc_get_cap()
{
    oc_cap_type cap = 0;
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        cap = canvas->attributes.cap;
    }
    return (cap);
}

oc_font oc_get_font()
{
    oc_font font = oc_font_nil();
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        font = canvas->attributes.font;
    }
    return (font);
}

f32 oc_get_font_size()
{
    f32 fontSize = 0;
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        fontSize = canvas->attributes.fontSize;
    }
    return (fontSize);
}

bool oc_get_text_flip()
{
    bool flip = false;
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        flip = canvas->textFlip;
    }
    return (flip);
}

oc_image oc_get_image()
{
    oc_image image = oc_image_nil();
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        image = canvas->attributes.image;
    }
    return (image);
}

oc_rect oc_get_image_source_region()
{
    oc_rect rect = { 0 };
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        rect = canvas->attributes.srcRegion;
    }
    return (rect);
}

//------------------------------------------------------------------------------------------
//NOTE(martin): path construction
//------------------------------------------------------------------------------------------
oc_vec2 oc_get_position()
{
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(!canvas)
    {
        return ((oc_vec2){ 0, 0 });
    }
    return (canvas->subPathLastPoint);
}

void oc_move_to(f32 x, f32 y)
{
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(!canvas)
    {
        return;
    }
    oc_path_push_element(canvas, ((oc_path_elt){ .type = OC_PATH_MOVE, .p[0] = { x, y } }));
    canvas->subPathStartPoint = (oc_vec2){ x, y };
    canvas->subPathLastPoint = (oc_vec2){ x, y };
}

void oc_line_to(f32 x, f32 y)
{
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(!canvas)
    {
        return;
    }
    oc_path_push_element(canvas, ((oc_path_elt){ .type = OC_PATH_LINE, .p[0] = { x, y } }));
    canvas->subPathLastPoint = (oc_vec2){ x, y };
}

void oc_quadratic_to(f32 x1, f32 y1, f32 x2, f32 y2)
{
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(!canvas)
    {
        return;
    }
    oc_path_push_element(canvas, ((oc_path_elt){ .type = OC_PATH_QUADRATIC, .p = { { x1, y1 }, { x2, y2 } } }));
    canvas->subPathLastPoint = (oc_vec2){ x2, y2 };
}

void oc_cubic_to(f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3)
{
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(!canvas)
    {
        return;
    }
    oc_path_push_element(canvas, ((oc_path_elt){ .type = OC_PATH_CUBIC, .p = { { x1, y1 }, { x2, y2 }, { x3, y3 } } }));
    canvas->subPathLastPoint = (oc_vec2){ x3, y3 };
}

void oc_close_path()
{
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(!canvas)
    {
        return;
    }
    if(canvas->subPathStartPoint.x != canvas->subPathLastPoint.x
       || canvas->subPathStartPoint.y != canvas->subPathLastPoint.y)
    {
        oc_line_to(canvas->subPathStartPoint.x, canvas->subPathStartPoint.y);
    }
    canvas->subPathStartPoint = canvas->subPathLastPoint;
}

oc_rect oc_glyph_outlines_from_font_data(oc_font_data* fontData, oc_str32 glyphIndices)
{
    oc_canvas_data* canvas = __mgCurrentCanvas;

    f32 startX = canvas->subPathLastPoint.x;
    f32 startY = canvas->subPathLastPoint.y;
    f32 maxWidth = 0;

    f32 scale = canvas->attributes.fontSize / fontData->unitsPerEm;

    for(int i = 0; i < glyphIndices.len; i++)
    {
        u32 glyphIndex = glyphIndices.ptr[i];

        f32 xOffset = canvas->subPathLastPoint.x;
        f32 yOffset = canvas->subPathLastPoint.y;
        f32 flip = canvas->textFlip ? 1 : -1;

        if(!glyphIndex || glyphIndex >= fontData->glyphCount)
        {
            oc_log_warning("code point is not present in font ranges\n");
            //NOTE(martin): try to find the replacement character
            glyphIndex = oc_font_get_glyph_index_from_font_data(fontData, 0xfffd);
            if(!glyphIndex)
            {
                //NOTE(martin): could not find replacement glyph, try to get an 'X' to get a somewhat correct dimensions
                //              to render an empty rectangle. Otherwise just render with the max font width

                oc_glyph_metrics missingGlyphMetrics = { 0 };
                u32 missingGlyphIndex = oc_font_get_glyph_index_from_font_data(fontData, 'X');

                if(missingGlyphIndex)
                {
                    oc_font_get_glyph_metrics_from_font_data(fontData, (oc_str32){ .ptr = &missingGlyphIndex, .len = 1 }, &missingGlyphMetrics);
                }
                else
                {
                    missingGlyphMetrics = (oc_glyph_metrics){
                        .ink = {
                            .x = fontData->metrics.width * 0.1,
                            .y = -fontData->metrics.ascent,
                            .w = fontData->metrics.width * 0.8,
                            .h = fontData->metrics.ascent,
                        },
                        .advance = { .x = fontData->metrics.width, .y = 0 },
                    };
                }

                f32 oldStrokeWidth = canvas->attributes.width;

                oc_set_width(missingGlyphMetrics.ink.w * 0.005);
                oc_rectangle_stroke(xOffset + missingGlyphMetrics.ink.x * scale,
                                    yOffset + missingGlyphMetrics.ink.y * scale,
                                    missingGlyphMetrics.ink.w * scale * flip,
                                    missingGlyphMetrics.ink.h * scale);

                oc_set_width(oldStrokeWidth);
                oc_move_to(xOffset + missingGlyphMetrics.advance.x * scale, yOffset);
                maxWidth = oc_max(maxWidth, xOffset + missingGlyphMetrics.advance.x * scale - startX);
                continue;
            }
        }

        oc_glyph_data* glyph = oc_font_get_glyph_data(fontData, glyphIndex);

        oc_path_push_elements(canvas, glyph->pathDescriptor.count, fontData->outlines + glyph->pathDescriptor.startIndex);

        oc_path_elt* elements = canvas->pathElements + canvas->path.count + canvas->path.startIndex - glyph->pathDescriptor.count;
        for(int eltIndex = 0; eltIndex < glyph->pathDescriptor.count; eltIndex++)
        {
            for(int pIndex = 0; pIndex < 3; pIndex++)
            {
                elements[eltIndex].p[pIndex].x = elements[eltIndex].p[pIndex].x * scale + xOffset;
                elements[eltIndex].p[pIndex].y = elements[eltIndex].p[pIndex].y * scale * flip + yOffset;
            }
        }
        oc_move_to(xOffset + scale * glyph->metrics.advance.x, yOffset);

        maxWidth = oc_max(maxWidth, xOffset + scale * glyph->metrics.advance.x - startX);
    }
    f32 lineHeight = (fontData->metrics.ascent + fontData->metrics.descent) * scale;
    oc_rect box = { startX, startY, maxWidth, canvas->subPathLastPoint.y - startY + lineHeight };
    return (box);
}

oc_rect oc_glyph_outlines(oc_str32 glyphIndices)
{
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(!canvas)
    {
        return ((oc_rect){ 0 });
    }
    oc_font_data* fontData = oc_font_data_from_handle(canvas->attributes.font);
    if(!fontData)
    {
        return ((oc_rect){ 0 });
    }
    return (oc_glyph_outlines_from_font_data(fontData, glyphIndices));
}

void oc_codepoints_outlines(oc_str32 codePoints)
{
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(!canvas)
    {
        return;
    }
    oc_font_data* fontData = oc_font_data_from_handle(canvas->attributes.font);
    if(!fontData)
    {
        return;
    }

    oc_arena_scope scratch = oc_scratch_begin();

    oc_str32 glyphIndices = oc_font_push_glyph_indices(scratch.arena, canvas->attributes.font, codePoints);
    oc_glyph_outlines_from_font_data(fontData, glyphIndices);

    oc_scratch_end(scratch);
}

void oc_text_outlines(oc_str8 text)
{
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(!canvas)
    {
        return;
    }
    oc_font_data* fontData = oc_font_data_from_handle(canvas->attributes.font);
    if(!fontData)
    {
        return;
    }

    oc_arena_scope scratch = oc_scratch_begin();
    oc_str32 codePoints = oc_utf8_push_to_codepoints(scratch.arena, text);
    oc_str32 glyphIndices = oc_font_push_glyph_indices(scratch.arena, canvas->attributes.font, codePoints);

    oc_glyph_outlines_from_font_data(fontData, glyphIndices);

    oc_scratch_end(scratch);
}

//------------------------------------------------------------------------------------------
//NOTE(martin): clear/fill/stroke
//------------------------------------------------------------------------------------------

void oc_clear()
{
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        canvas->primitiveCount = 0;
        canvas->clearColor = canvas->attributes.color;
    }
}

void oc_fill()
{
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas && canvas->path.count)
    {
        oc_push_command(canvas, (oc_primitive){ .cmd = OC_CMD_FILL, .path = canvas->path });
        oc_new_path(canvas);
    }
}

void oc_stroke()
{
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas && canvas->path.count)
    {
        oc_push_command(canvas, (oc_primitive){ .cmd = OC_CMD_STROKE, .path = canvas->path });
        oc_new_path(canvas);
    }
}

//------------------------------------------------------------------------------------------
//NOTE(martin): simple shape helpers
//------------------------------------------------------------------------------------------

void oc_rectangle_path(f32 x, f32 y, f32 w, f32 h)
{
    oc_move_to(x, y);
    oc_line_to(x + w, y);
    oc_line_to(x + w, y + h);
    oc_line_to(x, y + h);
    oc_close_path();
}

void oc_rectangle_fill(f32 x, f32 y, f32 w, f32 h)
{
    oc_rectangle_path(x, y, w, h);
    oc_fill();
}

void oc_rectangle_stroke(f32 x, f32 y, f32 w, f32 h)
{
    oc_rectangle_path(x, y, w, h);
    oc_stroke();
}

void oc_rounded_rectangle_path(f32 x, f32 y, f32 w, f32 h, f32 r)
{
    r = oc_min(r, oc_min(w / 2, h / 2));
    f32 c = r * 4 * (sqrt(2) - 1) / 3;

    oc_move_to(x + r, y);
    oc_line_to(x + w - r, y);
    oc_cubic_to(x + w - r + c, y, x + w, y + r - c, x + w, y + r);
    oc_line_to(x + w, y + h - r);
    oc_cubic_to(x + w, y + h - r + c, x + w - r + c, y + h, x + w - r, y + h);
    oc_line_to(x + r, y + h);
    oc_cubic_to(x + r - c, y + h, x, y + h - r + c, x, y + h - r);
    oc_line_to(x, y + r);
    oc_cubic_to(x, y + r - c, x + r - c, y, x + r, y);
}

void oc_rounded_rectangle_fill(f32 x, f32 y, f32 w, f32 h, f32 r)
{
    oc_rounded_rectangle_path(x, y, w, h, r);
    oc_fill();
}

void oc_rounded_rectangle_stroke(f32 x, f32 y, f32 w, f32 h, f32 r)
{
    oc_rounded_rectangle_path(x, y, w, h, r);
    oc_stroke();
}

void oc_ellipse_path(f32 x, f32 y, f32 rx, f32 ry)
{
    f32 cx = rx * 4 * (sqrt(2) - 1) / 3;
    f32 cy = ry * 4 * (sqrt(2) - 1) / 3;

    oc_move_to(x - rx, y);
    oc_cubic_to(x - rx, y + cy, x - cx, y + ry, x, y + ry);
    oc_cubic_to(x + cx, y + ry, x + rx, y + cy, x + rx, y);
    oc_cubic_to(x + rx, y - cy, x + cx, y - ry, x, y - ry);
    oc_cubic_to(x - cx, y - ry, x - rx, y - cy, x - rx, y);
}

void oc_ellipse_fill(f32 x, f32 y, f32 rx, f32 ry)
{
    oc_ellipse_path(x, y, rx, ry);
    oc_fill();
}

void oc_ellipse_stroke(f32 x, f32 y, f32 rx, f32 ry)
{
    oc_ellipse_path(x, y, rx, ry);
    oc_stroke();
}

void oc_circle_fill(f32 x, f32 y, f32 r)
{
    oc_ellipse_fill(x, y, r, r);
}

void oc_circle_stroke(f32 x, f32 y, f32 r)
{
    oc_ellipse_stroke(x, y, r, r);
}

//TODO: change to arc_to?
void oc_arc(f32 x, f32 y, f32 r, f32 arcAngle, f32 startAngle)
{
    f32 endAngle = startAngle + arcAngle;

    while(startAngle < endAngle)
    {
        f32 smallAngle = oc_min(endAngle - startAngle, M_PI / 4.);
        if(smallAngle < 0.001)
        {
            break;
        }

        oc_vec2 v0 = { cos(smallAngle / 2), sin(smallAngle / 2) };
        oc_vec2 v1 = { (4 - v0.x) / 3, (1 - v0.x) * (3 - v0.x) / (3 * v0.y) };
        oc_vec2 v2 = { v1.x, -v1.y };
        oc_vec2 v3 = { v0.x, -v0.y };

        f32 rotAngle = smallAngle / 2 + startAngle;
        f32 rotCos = cos(rotAngle);
        f32 rotSin = sin(rotAngle);

        oc_mat2x3 t = { r * rotCos, -r * rotSin, x,
                        r * rotSin, r * rotCos, y };

        v0 = oc_mat2x3_mul(t, v0);
        v1 = oc_mat2x3_mul(t, v1);
        v2 = oc_mat2x3_mul(t, v2);
        v3 = oc_mat2x3_mul(t, v3);

        oc_move_to(v0.x, v0.y);
        oc_cubic_to(v1.x, v1.y, v2.x, v2.y, v3.x, v3.y);

        startAngle += smallAngle;
    }
}

void oc_text_fill(f32 x, f32 y, oc_str8 text)
{
    oc_move_to(x, y);
    oc_text_outlines(text);
    oc_fill();
}

//------------------------------------------------------------------------------------------
//NOTE(martin): images
//------------------------------------------------------------------------------------------

oc_image oc_image_nil() { return ((oc_image){ .h = 0 }); }

bool oc_image_is_nil(oc_image image) { return (image.h == 0); }

oc_image oc_image_create_from_rgba8(oc_surface surface, u32 width, u32 height, u8* pixels)
{
    oc_image image = oc_image_create(surface, width, height);
    if(!oc_image_is_nil(image))
    {
        oc_image_upload_region_rgba8(image, (oc_rect){ 0, 0, width, height }, pixels);
    }
    return (image);
}

oc_image oc_image_create_from_memory(oc_surface surface, oc_str8 mem, bool flip)
{
    oc_image image = oc_image_nil();
    int width, height, channels;

    stbi_set_flip_vertically_on_load(flip ? 1 : 0);
    u8* pixels = stbi_load_from_memory((u8*)mem.ptr, mem.len, &width, &height, &channels, 4);

    if(pixels)
    {
        image = oc_image_create_from_rgba8(surface, width, height, pixels);
        free(pixels);
    }
    else
    {
        oc_log_error("stbi_load_from_memory() failed: %s\n", stbi_failure_reason());
    }
    return (image);
}

oc_image oc_image_create_from_file(oc_surface surface, oc_file file, bool flip)
{
    oc_image image = oc_image_nil();
    oc_arena_scope scratch = oc_scratch_begin();

    u64 size = oc_file_size(file);
    char* buffer = oc_arena_push(scratch.arena, size);
    u64 read = oc_file_read(file, size, buffer);

    if(read != size)
    {
        oc_log_error("Couldn't read image data\n");
    }
    else
    {
        image = oc_image_create_from_memory(surface, oc_str8_from_buffer(size, buffer), flip);
    }

    oc_scratch_end(scratch);
    return (image);
}

oc_image oc_image_create_from_path(oc_surface surface, oc_str8 path, bool flip)
{
    oc_image image = oc_image_nil();

    oc_file file = oc_file_open(path, OC_FILE_ACCESS_READ, OC_FILE_OPEN_NONE);
    if(oc_file_last_error(file) != OC_IO_OK)
    {
        oc_log_error("Could not open file %*.s\n", oc_str8_ip(path));
    }
    else
    {
        image = oc_image_create_from_file(surface, file, flip);
    }
    oc_file_close(file);
    return (image);
}

void oc_image_draw_region(oc_image image, oc_rect srcRegion, oc_rect dstRegion)
{
    oc_canvas_data* canvas = __mgCurrentCanvas;
    if(canvas)
    {
        oc_image oldImage = canvas->attributes.image;
        oc_rect oldSrcRegion = canvas->attributes.srcRegion;
        oc_color oldColor = canvas->attributes.color;

        canvas->attributes.image = image;
        canvas->attributes.srcRegion = srcRegion;
        canvas->attributes.color = (oc_color){ 1, 1, 1, 1 };

        oc_move_to(dstRegion.x, dstRegion.y);
        oc_line_to(dstRegion.x + dstRegion.w, dstRegion.y);
        oc_line_to(dstRegion.x + dstRegion.w, dstRegion.y + dstRegion.h);
        oc_line_to(dstRegion.x, dstRegion.y + dstRegion.h);
        oc_close_path();

        oc_fill();

        canvas->attributes.image = oldImage;
        canvas->attributes.srcRegion = oldSrcRegion;
        canvas->attributes.color = oldColor;
    }
}

void oc_image_draw(oc_image image, oc_rect rect)
{
    oc_vec2 size = oc_image_size(image);
    oc_image_draw_region(image, (oc_rect){ 0, 0, size.x, size.y }, rect);
}

//------------------------------------------------------------------------------------------
//NOTE(martin): atlasing
//------------------------------------------------------------------------------------------

//NOTE: rectangle allocator
typedef struct oc_rect_atlas
{
    oc_arena* arena;
    oc_vec2i size;
    oc_vec2i pos;
    u32 lineHeight;

} oc_rect_atlas;

oc_rect_atlas* oc_rect_atlas_create(oc_arena* arena, i32 width, i32 height)
{
    oc_rect_atlas* atlas = oc_arena_push_type(arena, oc_rect_atlas);
    memset(atlas, 0, sizeof(oc_rect_atlas));
    atlas->arena = arena;
    atlas->size = (oc_vec2i){ width, height };
    return (atlas);
}

oc_rect oc_rect_atlas_alloc(oc_rect_atlas* atlas, i32 width, i32 height)
{
    oc_rect rect = { 0, 0, 0, 0 };
    if(width > 0 && height > 0)
    {
        if(atlas->pos.x + width >= atlas->size.x)
        {
            atlas->pos.x = 0;
            atlas->pos.y += (atlas->lineHeight + 1);
            atlas->lineHeight = 0;
        }
        if(atlas->pos.x + width < atlas->size.x
           && atlas->pos.y + height < atlas->size.y)
        {
            rect = (oc_rect){ atlas->pos.x, atlas->pos.y, width, height };

            atlas->pos.x += (width + 1);
            atlas->lineHeight = oc_max(atlas->lineHeight, height);
        }
    }
    return (rect);
}

void oc_rect_atlas_recycle(oc_rect_atlas* atlas, oc_rect rect)
{
    //TODO
}

oc_image_region oc_image_atlas_alloc_from_rgba8(oc_rect_atlas* atlas, oc_image backingImage, u32 width, u32 height, u8* pixels)
{
    oc_image_region imageRgn = { 0 };

    oc_rect rect = oc_rect_atlas_alloc(atlas, width, height);
    if(rect.w == width && rect.h == height)
    {
        oc_image_upload_region_rgba8(backingImage, rect, pixels);
        imageRgn.rect = rect;
        imageRgn.image = backingImage;
    }
    return (imageRgn);
}

oc_image_region oc_image_atlas_alloc_from_memory(oc_rect_atlas* atlas, oc_image backingImage, oc_str8 mem, bool flip)
{
    oc_image_region imageRgn = { 0 };

    int width, height, channels;

    stbi_set_flip_vertically_on_load(flip ? 1 : 0);
    u8* pixels = stbi_load_from_memory((u8*)mem.ptr, mem.len, &width, &height, &channels, 4);
    if(pixels)
    {
        imageRgn = oc_image_atlas_alloc_from_rgba8(atlas, backingImage, width, height, pixels);
        free(pixels);
    }
    return (imageRgn);
}

oc_image_region oc_image_atlas_alloc_from_file(oc_rect_atlas* atlas, oc_image backingImage, oc_file file, bool flip)
{
    oc_image_region imageRgn = { 0 };

    oc_arena_scope scratch = oc_scratch_begin();

    u64 size = oc_file_size(file);
    char* buffer = oc_arena_push(scratch.arena, size);
    u64 read = oc_file_read(file, size, buffer);

    if(read != size)
    {
        oc_log_error("Couldn't read image data\n");
    }
    else
    {
        imageRgn = oc_image_atlas_alloc_from_memory(atlas, backingImage, oc_str8_from_buffer(size, buffer), flip);
    }

    oc_scratch_end(scratch);
    return (imageRgn);
}

oc_image_region oc_image_atlas_alloc_from_path(oc_rect_atlas* atlas, oc_image backingImage, oc_str8 path, bool flip)
{
    oc_image_region imageRgn = { 0 };

    oc_file file = oc_file_open(path, OC_FILE_ACCESS_READ, OC_FILE_OPEN_NONE);
    if(oc_file_last_error(file) != OC_IO_OK)
    {
        oc_log_error("Could not open file %*.s\n", oc_str8_ip(path));
    }
    else
    {
        imageRgn = oc_image_atlas_alloc_from_file(atlas, backingImage, file, flip);
    }
    oc_file_close(file);
    return (imageRgn);
}

void oc_image_atlas_recycle(oc_rect_atlas* atlas, oc_image_region imageRgn)
{
    oc_rect_atlas_recycle(atlas, imageRgn.rect);
}
