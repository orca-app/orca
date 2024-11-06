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

#include "ext/fribidi/include/fribidi.h"
#include "unicode_scripts.h"

#include "graphics_common.h"
#include "platform/platform_debug.h"
#include "util/algebra.h"

enum
{
    OC_MATRIX_STACK_MAX_DEPTH = 64,
    OC_CLIP_STACK_MAX_DEPTH = 64,
    OC_MAX_PATH_ELEMENT_COUNT = 2 << 20,
    OC_MAX_PRIMITIVE_COUNT = 8 << 14
};

typedef struct oc_font_data
{
    oc_list_elt freeListElt;

    f32 unitsPerEm;
    oc_font_metrics metrics;

    oc_harfbuzz_handle harfbuzzHandle;

} oc_font_data;

typedef struct oc_canvas_context_data oc_canvas_context_data;

typedef enum oc_graphics_handle_kind
{
    OC_GRAPHICS_HANDLE_NONE = 0,
    OC_GRAPHICS_HANDLE_SURFACE,
    OC_GRAPHICS_HANDLE_CANVAS_RENDERER,
    OC_GRAPHICS_HANDLE_CANVAS_CONTEXT,
    OC_GRAPHICS_HANDLE_FONT,
    OC_GRAPHICS_HANDLE_HARFBUZZ,
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
    oc_list harfbuzzFreeList;

} oc_graphics_data;

typedef struct oc_canvas_context_data
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
    i32 msaaSampleCount;

    bool clear;
    oc_color clearColor;

    oc_vec4 shapeExtents;
    oc_vec4 shapeScreenExtents;

} oc_canvas_context_data;

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

    oc_graphics_handle_slot* slot = oc_list_pop_front_entry(&oc_graphicsData.handleFreeList, oc_graphics_handle_slot, freeListElt);
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
            oc_list_push_front(&oc_graphicsData.handleFreeList, &slot->freeListElt);
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
//NOTE(martin): nil handles
//------------------------------------------------------------------------------------------

oc_canvas_renderer oc_canvas_renderer_nil(void)
{
    return ((oc_canvas_renderer){ .h = 0 });
}

bool oc_canvas_renderer_is_nil(oc_canvas_renderer renderer)
{
    return (renderer.h == 0);
}

oc_surface oc_surface_nil(void)
{
    return ((oc_surface){ .h = 0 });
}

bool oc_surface_is_nil(oc_surface surface)
{
    return (surface.h == 0);
}

oc_canvas_context oc_canvas_context_nil()
{
    return ((oc_canvas_context){ .h = 0 });
}

bool oc_canvas_context_is_nil(oc_canvas_context context)
{
    return (context.h == 0);
}

//------------------------------------------------------------------------------------------
//NOTE(martin): canvas context internal
//------------------------------------------------------------------------------------------

oc_thread_local oc_canvas_context_data* oc_currentCanvasContext = 0;
oc_thread_local oc_canvas_context oc_currentCanvasContextHandle = { 0 };

bool oc_vec2_close(oc_vec2 p0, oc_vec2 p1, f32 tolerance)
{
    f32 norm2 = (p1.x - p0.x) * (p1.x - p0.x) + (p1.y - p0.y) * (p1.y - p0.y);
    return (fabs(norm2) < tolerance);
}

oc_mat2x3 oc_matrix_stack_top(oc_canvas_context_data* context)
{
    if(context->matrixStackSize == 0)
    {
        return ((oc_mat2x3){ 1, 0, 0,
                             0, 1, 0 });
    }
    else
    {
        return (context->matrixStack[context->matrixStackSize - 1]);
    }
}

void oc_matrix_stack_push(oc_canvas_context_data* context, oc_mat2x3 transform)
{
    if(context->matrixStackSize >= OC_MATRIX_STACK_MAX_DEPTH)
    {
        oc_log_error("matrix stack overflow\n");
    }
    else
    {
        context->matrixStack[context->matrixStackSize] = transform;
        context->matrixStackSize++;
    }
}

void oc_matrix_stack_pop(oc_canvas_context_data* context)
{
    if(context->matrixStackSize == 0)
    {
        oc_log_error("matrix stack underflow\n");
    }
    else
    {
        context->matrixStackSize--;
        oc_matrix_stack_top(context);
    }
}

oc_rect oc_clip_stack_top(oc_canvas_context_data* context)
{
    if(context->clipStackSize == 0)
    {
        return ((oc_rect){ -FLT_MAX / 2, -FLT_MAX / 2, FLT_MAX, FLT_MAX });
    }
    else
    {
        return (context->clipStack[context->clipStackSize - 1]);
    }
}

void oc_clip_stack_push(oc_canvas_context_data* context, oc_rect clip)
{
    if(context->clipStackSize >= OC_CLIP_STACK_MAX_DEPTH)
    {
        oc_log_error("clip stack overflow\n");
    }
    else
    {
        context->clipStack[context->clipStackSize] = clip;
        context->clipStackSize++;
    }
}

void oc_clip_stack_pop(oc_canvas_context_data* context)
{
    if(context->clipStackSize == 0)
    {
        oc_log_error("clip stack underflow\n");
    }
    else
    {
        context->clipStackSize--;
    }
}

void oc_push_command(oc_canvas_context_data* context, oc_primitive primitive)
{
    //NOTE(martin): push primitive and updates current stream, eventually patching a pending jump.
    OC_ASSERT(context->primitiveCount < OC_MAX_PRIMITIVE_COUNT);

    context->primitives[context->primitiveCount] = primitive;
    context->primitives[context->primitiveCount].attributes = context->attributes;
    context->primitives[context->primitiveCount].attributes.transform = oc_matrix_stack_top(context);
    context->primitives[context->primitiveCount].attributes.clip = oc_clip_stack_top(context);
    context->primitiveCount++;
}

void oc_new_path(oc_canvas_context_data* context)
{
    context->path.startIndex += context->path.count;
    context->path.count = 0;
    context->subPathStartPoint = context->subPathLastPoint;
    context->path.startPoint = context->subPathStartPoint;
}

void oc_path_push_elements(oc_canvas_context_data* context, u32 count, oc_path_elt* elements)
{
    OC_ASSERT(context->path.count + context->path.startIndex + count < OC_MAX_PATH_ELEMENT_COUNT);
    memcpy(context->pathElements + context->path.startIndex + context->path.count, elements, count * sizeof(oc_path_elt));
    context->path.count += count;

    OC_ASSERT(context->path.count < OC_MAX_PATH_ELEMENT_COUNT);
}

void oc_path_push_element(oc_canvas_context_data* context, oc_path_elt elt)
{
    oc_path_push_elements(context, 1, &elt);
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

oc_font oc_font_create_from_memory(oc_str8 mem)
{
    if(!oc_graphicsData.init)
    {
        oc_graphics_init();
    }
    oc_font fontHandle = oc_font_nil();

    oc_font_data* font = oc_list_pop_front_entry(&oc_graphicsData.fontFreeList, oc_font_data, freeListElt);
    if(!font)
    {
        font = oc_arena_push_type(&oc_graphicsData.resourceArena, oc_font_data);
    }
    if(font)
    {
        memset(font, 0, sizeof(oc_font_data));
        fontHandle = oc_font_handle_alloc(font);

        font->harfbuzzHandle = oc_harfbuzz_font_create(mem);
        //TODO: check handle
        font->unitsPerEm = oc_harfbuzz_font_get_upem(font->harfbuzzHandle);
        font->metrics = oc_harfbuzz_font_get_metrics(font->harfbuzzHandle);
    }

    return (fontHandle);
}

oc_font oc_font_create_from_file(oc_file file)
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
        font = oc_font_create_from_memory(oc_str8_from_buffer(size, buffer));
    }

    oc_scratch_end(scratch);
    return (font);
}

oc_font oc_font_create_from_path(oc_str8 path)
{
    oc_font font = oc_font_nil();

    oc_file file = oc_file_open(path, OC_FILE_ACCESS_READ, OC_FILE_OPEN_NONE);
    if(oc_file_last_error(file) != OC_IO_OK)
    {
        oc_log_error("Could not open file %*.s\n", oc_str8_ip(path));
    }
    else
    {
        font = oc_font_create_from_file(file);
    }
    oc_file_close(file);

    return (font);
}

void oc_font_destroy(oc_font fontHandle)
{
    oc_font_data* fontData = oc_font_data_from_handle(fontHandle);
    if(fontData)
    {
        oc_harfbuzz_font_destroy(fontData->harfbuzzHandle);
        oc_list_push_front(&oc_graphicsData.fontFreeList, &fontData->freeListElt);
        oc_graphics_handle_recycle(fontHandle.h);
    }
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
    oc_font_metrics metrics = oc_font_get_metrics_unscaled(font);
    f32 scale = oc_font_get_scale_for_em_pixels(font, emSize);

    metrics.ascent *= scale;
    metrics.descent *= scale;
    metrics.lineGap *= scale;
    metrics.xHeight *= scale;
    metrics.capHeight *= scale;
    metrics.width *= scale;

    return (metrics);
}

oc_text_metrics oc_font_text_metrics_utf32(oc_font font, f32 fontSize, oc_str32 codePoints)
{
    //////////////////////////////////////////////////////
    //TODO: remove that API in favor of shaped text
    //////////////////////////////////////////////////////

    if(!codePoints.len || !codePoints.ptr)
    {
        return ((oc_text_metrics){ 0 });
    }

    oc_arena_scope scratch = oc_scratch_begin();

    oc_glyph_run* run = oc_text_shape(scratch.arena, font, 0, codePoints, 0, codePoints.len);
    oc_text_metrics metrics = run->metrics;

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

    //oc_scratch_end(scratch);
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
//NOTE(martin): canvas context API
//------------------------------------------------------------------------------------------

oc_canvas_context oc_canvas_context_handle_alloc(oc_canvas_context_data* context)
{
    oc_canvas_context handle = { .h = oc_graphics_handle_alloc(OC_GRAPHICS_HANDLE_CANVAS_CONTEXT, (void*)context) };
    return (handle);
}

oc_canvas_context_data* oc_canvas_context_from_handle(oc_canvas_context handle)
{
    oc_canvas_context_data* context = oc_graphics_data_from_handle(OC_GRAPHICS_HANDLE_CANVAS_CONTEXT, handle.h);
    return (context);
}

oc_canvas_context oc_canvas_context_create()
{
    if(!oc_graphicsData.init)
    {
        oc_graphics_init();
    }

    oc_canvas_context contextHandle = oc_canvas_context_nil();
    oc_canvas_context_data* context = oc_list_pop_front_entry(&oc_graphicsData.canvasFreeList, oc_canvas_context_data, freeListElt);
    if(!context)
    {
        context = oc_arena_push_type(&oc_graphicsData.resourceArena, oc_canvas_context_data);
    }
    if(context)
    {
        context->textFlip = false;
        context->path = (oc_path_descriptor){ 0 };
        context->matrixStackSize = 0;
        context->clipStackSize = 0;
        context->primitiveCount = 0;
        context->clearColor = (oc_color){ 0, 0, 0, 0 };
        context->msaaSampleCount = 8;

        context->attributes = (oc_attributes){ 0 };
        context->attributes.hasGradient = false;
        context->attributes.colors[0] = (oc_color){ 0, 0, 0, 1 };
        context->attributes.tolerance = 1;
        context->attributes.width = 10;
        context->attributes.clip = (oc_rect){ -FLT_MAX / 2, -FLT_MAX / 2, FLT_MAX, FLT_MAX };

        contextHandle = oc_canvas_context_handle_alloc(context);

        oc_canvas_context_select(contextHandle);
    }
    return (contextHandle);
}

void oc_canvas_context_destroy(oc_canvas_context handle)
{
    oc_canvas_context_data* context = oc_canvas_context_from_handle(handle);
    if(context)
    {
        if(oc_currentCanvasContext == context)
        {
            oc_currentCanvasContext = 0;
            oc_currentCanvasContextHandle = oc_canvas_context_nil();
        }

        oc_list_push_front(&oc_graphicsData.canvasFreeList, &context->freeListElt);
        oc_graphics_handle_recycle(handle.h);
    }
}

oc_canvas_context oc_canvas_context_select(oc_canvas_context handle)
{
    oc_canvas_context old = oc_currentCanvasContextHandle;
    oc_currentCanvasContextHandle = handle;
    oc_currentCanvasContext = oc_canvas_context_from_handle(handle);
    return (old);
}

void oc_canvas_context_set_msaa_sample_count(oc_canvas_context handle, u32 sampleCount)
{
    oc_canvas_context_data* context = oc_canvas_context_from_handle(handle);
    if(context)
    {
        context->msaaSampleCount = sampleCount;
    }
}

void oc_canvas_render(oc_canvas_renderer rendererHandle, oc_canvas_context contextHandle, oc_surface surfaceHandle)
{
    oc_canvas_context_data* context = oc_canvas_context_from_handle(contextHandle);

    if(context && !oc_canvas_renderer_is_nil(rendererHandle) && !oc_surface_is_nil(surfaceHandle))
    {
        int eltCount = context->path.startIndex + context->path.count;

        oc_canvas_renderer_submit(rendererHandle,
                                  surfaceHandle,
                                  context->msaaSampleCount,
                                  context->clear,
                                  context->clearColor,
                                  context->primitiveCount,
                                  context->primitives,
                                  eltCount,
                                  context->pathElements);

        context->primitiveCount = 0;
        context->path.startIndex = 0;
        context->path.count = 0;
        context->clear = false;
    }
}

//------------------------------------------------------------------------------------------
//NOTE(martin): transform, viewport and clipping
//------------------------------------------------------------------------------------------

void oc_matrix_push(oc_mat2x3 matrix)
{
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        oc_matrix_stack_push(context, matrix);
    }
}

void oc_matrix_multiply_push(oc_mat2x3 matrix)
{
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        oc_mat2x3 transform = oc_matrix_stack_top(context);
        oc_matrix_stack_push(context, oc_mat2x3_mul_m(transform, matrix));
    }
}

void oc_matrix_pop()
{
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        oc_matrix_stack_pop(context);
    }
}

oc_mat2x3 oc_matrix_top()
{
    oc_mat2x3 mat = {
        1, 0, 0,
        0, 1, 0
    };
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        mat = oc_matrix_stack_top(context);
    }

    return (mat);
}

void oc_clip_push(f32 x, f32 y, f32 w, f32 h)
{
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        oc_rect clip = { x, y, w, h };

        //NOTE(martin): transform clip
        oc_mat2x3 transform = oc_matrix_stack_top(context);
        oc_vec2 p0 = oc_mat2x3_mul(transform, (oc_vec2){ clip.x, clip.y });
        oc_vec2 p1 = oc_mat2x3_mul(transform, (oc_vec2){ clip.x + clip.w, clip.y });
        oc_vec2 p2 = oc_mat2x3_mul(transform, (oc_vec2){ clip.x + clip.w, clip.y + clip.h });
        oc_vec2 p3 = oc_mat2x3_mul(transform, (oc_vec2){ clip.x, clip.y + clip.h });

        f32 x0 = oc_min(p0.x, oc_min(p1.x, oc_min(p2.x, p3.x)));
        f32 y0 = oc_min(p0.y, oc_min(p1.y, oc_min(p2.y, p3.y)));
        f32 x1 = oc_max(p0.x, oc_max(p1.x, oc_max(p2.x, p3.x)));
        f32 y1 = oc_max(p0.y, oc_max(p1.y, oc_max(p2.y, p3.y)));

        oc_rect current = oc_clip_stack_top(context);

        //NOTE(martin): intersect with current clip
        x0 = oc_max(current.x, x0);
        y0 = oc_max(current.y, y0);
        x1 = oc_min(current.x + current.w, x1);
        y1 = oc_min(current.y + current.h, y1);

        oc_rect r = { x0, y0, oc_max(0, x1 - x0), oc_max(0, y1 - y0) };
        oc_clip_stack_push(context, r);

        context->attributes.clip = r;
    }
}

void oc_clip_pop()
{
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        oc_clip_stack_pop(context);
        context->attributes.clip = oc_clip_stack_top(context);
    }
}

oc_rect oc_clip_top()
{
    oc_rect clip = { -FLT_MAX / 2, -FLT_MAX / 2, FLT_MAX, FLT_MAX };

    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        clip = oc_clip_stack_top(context);
    }

    return (clip);
}

//------------------------------------------------------------------------------------------
//NOTE(martin): color helpers
//------------------------------------------------------------------------------------------

oc_color oc_color_rgba(f32 r, f32 g, f32 b, f32 a)
{
    return ((oc_color){ r, g, b, a, OC_COLOR_SPACE_RGB });
}

oc_color oc_color_srgba(f32 r, f32 g, f32 b, f32 a)
{
    return ((oc_color){ r, g, b, a, OC_COLOR_SPACE_SRGB });
}

oc_color oc_color_convert(oc_color color, oc_color_space colorSpace)
{
    switch(colorSpace)
    {
        case OC_COLOR_SPACE_RGB:
        {
            switch(color.colorSpace)
            {
                case OC_COLOR_SPACE_RGB:
                    // No conversion
                    break;

                case OC_COLOR_SPACE_SRGB:
                {
                    for(int i = 0; i < 3; i++)
                    {
                        if(color.c[i] <= 0.04045)
                        {
                            color.c[i] = color.c[i] / 12.92;
                        }
                        else
                        {
                            color.c[i] = powf((color.c[i] + 0.055) / 1.055, 2.4);
                        }
                    }
                }
                break;
            }
        }
        break;

        case OC_COLOR_SPACE_SRGB:
        {
            switch(color.colorSpace)
            {
                case OC_COLOR_SPACE_RGB:
                {
                    // RGBA to SRGBA
                    for(int i = 0; i < 3; i++)
                    {
                        if(color.c[i] <= 0.0031308)
                        {
                            color.c[i] = color.c[i] * 12.92;
                        }
                        else
                        {
                            color.c[i] = 1.055 * powf(color.c[i], 1.0 / 2.4) - 0.055;
                        }
                    }
                }
                break;

                case OC_COLOR_SPACE_SRGB:
                    // No conversion
                    break;
            }
        }
        break;
    }
    color.colorSpace = colorSpace;
    return (color);
}

//------------------------------------------------------------------------------------------
//NOTE(martin): graphics attributes setting/getting
//------------------------------------------------------------------------------------------
void oc_set_color(oc_color color)
{
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        context->attributes.hasGradient = false;
        context->attributes.colors[0] = color;
    }
}

void oc_set_color_rgba(f32 r, f32 g, f32 b, f32 a)
{
    oc_set_color((oc_color){ r, g, b, a, .colorSpace = OC_COLOR_SPACE_RGB });
}

void oc_set_color_srgba(f32 r, f32 g, f32 b, f32 a)
{
    oc_set_color((oc_color){ r, g, b, a, .colorSpace = OC_COLOR_SPACE_SRGB });
}

void oc_set_gradient(oc_gradient_blend_space blendSpace, oc_color bottomLeft, oc_color bottomRight, oc_color topRight, oc_color topLeft)
{
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        context->attributes.hasGradient = true;
        context->attributes.blendSpace = blendSpace;
        context->attributes.colors[0] = bottomLeft;
        context->attributes.colors[1] = bottomRight;
        context->attributes.colors[2] = topRight;
        context->attributes.colors[3] = topLeft;
    }
}

void oc_set_width(f32 width)
{
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        context->attributes.width = width;
    }
}

void oc_set_tolerance(f32 tolerance)
{
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        context->attributes.tolerance = tolerance;
    }
}

void oc_set_joint(oc_joint_type joint)
{
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        context->attributes.joint = joint;
    }
}

void oc_set_max_joint_excursion(f32 maxJointExcursion)
{
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        context->attributes.maxJointExcursion = maxJointExcursion;
    }
}

void oc_set_cap(oc_cap_type cap)
{
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        context->attributes.cap = cap;
    }
}

void oc_set_fill_rule(oc_fill_rule rule)
{
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        context->attributes.fillRule = rule;
    }
}

void oc_set_text_flip(bool flip)
{
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        context->textFlip = flip;
    }
}

void oc_set_image(oc_image image)
{
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        context->attributes.image = image;
        oc_vec2 size = oc_image_size(image);
        context->attributes.srcRegion = (oc_rect){ 0, 0, size.x, size.y };
    }
}

void oc_set_image_source_region(oc_rect region)
{
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        context->attributes.srcRegion = region;
    }
}

oc_color oc_get_color()
{
    oc_color color = { 0 };
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        color = context->attributes.colors[0];
    }
    return (color);
}

f32 oc_get_width()
{
    f32 width = 0;
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        width = context->attributes.width;
    }
    return (width);
}

f32 oc_get_tolerance()
{
    f32 tolerance = 0;
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        tolerance = context->attributes.tolerance;
    }
    return (tolerance);
}

oc_joint_type oc_get_joint()
{
    oc_joint_type joint = 0;
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        joint = context->attributes.joint;
    }
    return (joint);
}

f32 oc_get_max_joint_excursion()
{
    f32 maxJointExcursion = 0;
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        maxJointExcursion = context->attributes.maxJointExcursion;
    }
    return (maxJointExcursion);
}

oc_cap_type oc_get_cap()
{
    oc_cap_type cap = 0;
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        cap = context->attributes.cap;
    }
    return (cap);
}

oc_fill_rule oc_get_fill_rule()
{
    oc_fill_rule rule = 0;
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        rule = context->attributes.fillRule;
    }
    return (rule);
}

bool oc_get_text_flip()
{
    bool flip = false;
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        flip = context->textFlip;
    }
    return (flip);
}

oc_image oc_get_image()
{
    oc_image image = oc_image_nil();
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        image = context->attributes.image;
    }
    return (image);
}

oc_rect oc_get_image_source_region()
{
    oc_rect rect = { 0 };
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        rect = context->attributes.srcRegion;
    }
    return (rect);
}

//------------------------------------------------------------------------------------------
//NOTE(martin): path construction
//------------------------------------------------------------------------------------------
oc_vec2 oc_get_position()
{
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(!context)
    {
        return ((oc_vec2){ 0, 0 });
    }
    return (context->subPathLastPoint);
}

void oc_move_to(f32 x, f32 y)
{
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(!context)
    {
        return;
    }
    oc_path_push_element(context, ((oc_path_elt){ .type = OC_PATH_MOVE, .p[0] = { x, y } }));
    context->subPathStartPoint = (oc_vec2){ x, y };
    context->subPathLastPoint = (oc_vec2){ x, y };
}

void oc_line_to(f32 x, f32 y)
{
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(!context)
    {
        return;
    }
    oc_path_push_element(context, ((oc_path_elt){ .type = OC_PATH_LINE, .p[0] = { x, y } }));
    context->subPathLastPoint = (oc_vec2){ x, y };
}

void oc_quadratic_to(f32 x1, f32 y1, f32 x2, f32 y2)
{
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(!context)
    {
        return;
    }
    oc_path_push_element(context, ((oc_path_elt){ .type = OC_PATH_QUADRATIC, .p = { { x1, y1 }, { x2, y2 } } }));
    context->subPathLastPoint = (oc_vec2){ x2, y2 };
}

void oc_cubic_to(f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3)
{
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(!context)
    {
        return;
    }
    oc_path_push_element(context, ((oc_path_elt){ .type = OC_PATH_CUBIC, .p = { { x1, y1 }, { x2, y2 }, { x3, y3 } } }));
    context->subPathLastPoint = (oc_vec2){ x3, y3 };
}

void oc_close_path()
{
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(!context)
    {
        return;
    }
    if(context->subPathStartPoint.x != context->subPathLastPoint.x
       || context->subPathStartPoint.y != context->subPathLastPoint.y)
    {
        oc_line_to(context->subPathStartPoint.x, context->subPathStartPoint.y);
    }
    context->subPathStartPoint = context->subPathLastPoint;
}

oc_glyph_run* oc_text_shape(oc_arena* arena,
                            oc_font font,
                            oc_text_shape_settings* settings,
                            oc_str32 codepoints,
                            u64 begin,
                            u64 end)
{
    oc_glyph_run* run = oc_arena_push_type(arena, oc_glyph_run);
    memset(run, 0, sizeof(oc_glyph_run));

    run->font = font;

    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(!context)
    {
        return (run);
    }
    oc_font_data* fontData = oc_font_data_from_handle(run->font);
    if(!fontData)
    {
        return (run);
    }

    run = oc_harfbuzz_font_shape(arena, fontData->harfbuzzHandle, settings, codepoints, begin, end);
    run->font = font;
    return (run);
}

typedef struct oc_text_line
{
    u64 codepointCount;
    u64 runCount;
    oc_glyph_run** runs; //TODO: could be just an array of glyph_runs if we change glyph run creation to take a run struct to init..
    oc_text_attributes* attributes;
    oc_vec2* offsets;

    oc_text_metrics metrics;
} oc_text_line;

typedef struct oc_text_item
{
    oc_list_elt listElt;
    u64 start;
    u64 end;
    oc_text_direction direction;
    oc_unicode_script script;

} oc_text_item;

oc_unicode_script oc_unicode_script_for_codepoint(oc_utf32 codepoint)
{
    for(u64 i = 0; i < OC_UNICODE_SCRIPT_RANGE_COUNT; i++)
    {
        const oc_unicode_script_range* range = &OC_UNICODE_SCRIPT_RANGES[i];
        if(codepoint >= range->start && codepoint <= range->end)
        {
            return range->script;
        }
    }
    return (OC_UNICODE_SCRIPT_COMMON);
}

oc_text_line* oc_text_line_from_utf32(oc_arena* arena, oc_str32 codepoints, oc_text_attributes* attributes)
{
    oc_text_line* line = oc_arena_push_type(arena, oc_text_line);
    memset(line, 0, sizeof(oc_text_line));

    if(codepoints.len == 0)
    {
        return (line);
    }

    line->codepointCount = codepoints.len;

    oc_arena_scope scratch = oc_scratch_begin_next(arena);

    //NOTE: use fribidi to get bidi embedding levels
    FriBidiParType baseDir = FRIBIDI_TYPE_LTR_VAL;
    FriBidiLevel* levels = oc_arena_push_array(scratch.arena, FriBidiLevel, codepoints.len);

    fribidi_log2vis(codepoints.ptr,
                    codepoints.len,
                    &baseDir,
                    0, 0, 0,
                    levels);

    //NOTE: push runs of contiguous embedding level
    oc_list dirRuns = { 0 };
    u64 runCount = 0;
    FriBidiLevel currentLevel = levels[0];
    u64 runStart = 0;

    for(u64 i = 0; i < codepoints.len; i++)
    {
        if((levels[i] != currentLevel))
        {
            oc_text_item* run = oc_arena_push_type(scratch.arena, oc_text_item);
            run->start = runStart;
            run->end = i;
            run->direction = (currentLevel & 1) ? OC_TEXT_DIRECTION_RTL : OC_TEXT_DIRECTION_LTR;
            oc_list_push_back(&dirRuns, &run->listElt);
            runCount++;

            runStart = i;
            currentLevel = levels[i];
        }
    }
    //NOTE: push last run
    oc_text_item* run = oc_arena_push_type(scratch.arena, oc_text_item);
    run->start = runStart;
    run->end = codepoints.len;
    run->direction = (currentLevel & 1) ? OC_TEXT_DIRECTION_RTL : OC_TEXT_DIRECTION_LTR;
    oc_list_push_back(&dirRuns, &run->listElt);
    runCount++;

    //NOTE: further split based on script
    //TODO: also split based on style

    oc_list scriptRuns = { 0 };
    runCount = 0;
    oc_list_for(dirRuns, dirRun, oc_text_item, listElt)
    {
        oc_unicode_script currentScript = oc_unicode_script_for_codepoint(codepoints.ptr[dirRun->start]);
        u64 runStart = dirRun->start;
        for(u64 i = dirRun->start; i < dirRun->end; i++)
        {
            oc_unicode_script script = oc_unicode_script_for_codepoint(codepoints.ptr[i]);
            if(currentScript == OC_UNICODE_SCRIPT_COMMON || currentScript == OC_UNICODE_SCRIPT_INHERITED)
            {
                currentScript = script;
            }

            if(script != currentScript
               && script != OC_UNICODE_SCRIPT_COMMON
               && script != OC_UNICODE_SCRIPT_INHERITED)
            {
                oc_text_item* item = oc_arena_push_type(scratch.arena, oc_text_item);
                item->start = runStart;
                item->end = i;
                item->direction = dirRun->direction;
                item->script = currentScript;
                oc_list_push_back(&scriptRuns, &item->listElt);
                runCount++;

                runStart = i;
                currentScript = script;
            }
        }
        //NOTE: push last script run
        oc_text_item* item = oc_arena_push_type(scratch.arena, oc_text_item);
        item->start = runStart;
        item->end = dirRun->end;
        item->direction = dirRun->direction;
        item->script = currentScript;
        oc_list_push_back(&scriptRuns, &item->listElt);
        runCount++;

        //TODO: reverse sequences of RTL runs that were split
    }

    //NOTE: allocate runs
    line->runCount = runCount;
    line->runs = oc_arena_push_array(arena, oc_glyph_run*, runCount);
    memset(line->runs, 0, sizeof(oc_glyph_run*) * runCount);

    line->offsets = oc_arena_push_array(arena, oc_vec2, runCount);
    memset(line->offsets, 0, sizeof(oc_vec2) * runCount);

    line->attributes = oc_arena_push_array(arena, oc_text_attributes, runCount);
    memset(line->attributes, 0, sizeof(oc_text_attributes) * runCount);

    //NOTE: Shape runs and compute offsets and metrics
    f32 inkX0 = 0;
    f32 inkY0 = 0;
    f32 inkX1 = 0;
    f32 inkY1 = 0;

    f32 logicalX0 = 0;
    f32 logicalY0 = 0;
    f32 logicalX1 = 0;
    f32 logicalY1 = 0;

    u64 runIndex = 0;
    oc_vec2 offset = { 0 };
    oc_list_for(scriptRuns, item, oc_text_item, listElt)
    {
        line->offsets[runIndex] = offset;
        line->attributes[runIndex] = *attributes;
        line->runs[runIndex] = oc_text_shape(arena, attributes->font, 0, codepoints, item->start, item->end);

        f32 scale = oc_font_get_scale_for_em_pixels(attributes->font, attributes->fontSize);
        oc_text_metrics runMetrics = line->runs[runIndex]->metrics;

        oc_vec2 advance = oc_vec2_mul(scale, line->runs[runIndex]->metrics.advance);
        offset = oc_vec2_add(offset, advance);

        inkX0 = oc_min(inkX0, offset.x + runMetrics.ink.x * scale);
        inkY0 = oc_min(inkY0, offset.y + runMetrics.ink.y * scale);
        inkX1 = oc_max(inkX1, offset.x + (runMetrics.ink.x + runMetrics.ink.w) * scale);
        inkY1 = oc_max(inkY1, offset.y + (runMetrics.ink.y + runMetrics.ink.h) * scale);

        logicalX0 = oc_min(logicalX0, offset.x + runMetrics.logical.x * scale);
        logicalY0 = oc_min(logicalY0, offset.y + runMetrics.logical.y * scale);
        logicalX1 = oc_max(logicalX1, offset.x + (runMetrics.logical.x + runMetrics.logical.w) * scale);
        logicalY1 = oc_max(logicalY1, offset.y + (runMetrics.logical.y + runMetrics.logical.h) * scale);

        runIndex++;
    }

    line->metrics.ink = (oc_rect){
        inkX0,
        inkY0,
        inkX1 - inkX0,
        inkY1 - inkY0,
    };

    line->metrics.logical = (oc_rect){
        logicalX0,
        logicalY0,
        logicalX1 - logicalX0,
        logicalY1 - logicalY0,
    };

    line->metrics.advance = offset;

    oc_scratch_end(scratch);

    return (line);
}

oc_text_line* oc_text_line_from_utf8(oc_arena* arena, oc_str8 string, oc_text_attributes* attributes)
{
    oc_arena_scope scratch = oc_scratch_begin_next(arena);
    oc_str32 codepoints = oc_utf8_push_to_codepoints(scratch.arena, string);
    oc_text_line* line = oc_text_line_from_utf32(arena, codepoints, attributes);
    oc_scratch_end(scratch);
    return (line);
}

oc_text_metrics oc_text_line_get_metrics(oc_text_line* line)
{
    return (line->metrics);
}

oc_rect oc_rect_combine(oc_rect a, oc_rect b)
{
    f32 x0 = oc_min(a.x, b.x);
    f32 x1 = oc_max(a.x + a.w, b.x + b.w);
    f32 y0 = oc_min(a.y, b.y);
    f32 y1 = oc_max(a.y + a.h, b.y + b.h);

    oc_rect r = { x0, y0, x1 - x0, y1 - y0 };
    return r;
}

oc_text_metrics oc_text_metrics_combine(oc_text_metrics a, oc_text_metrics b)
{
    oc_text_metrics r = {
        .ink = oc_rect_combine(a.ink, b.ink),
        .logical = oc_rect_combine(a.logical, b.logical),
        .advance = oc_vec2_add(a.advance, b.advance),
    };
    return r;
}

oc_text_metrics oc_text_line_get_metrics_for_range(oc_text_line* line, u64 start, u64 end)
{
    oc_text_metrics result = { 0 };
    bool rangeStarted = false;

    for(u64 runIndex = 0; runIndex < line->runCount; runIndex++)
    {
        oc_glyph_run* run = line->runs[runIndex];
        oc_vec2 runOffset = line->offsets[runIndex];

        f32 scale = oc_font_get_scale_for_em_pixels(line->attributes[runIndex].font,
                                                    line->attributes[runIndex].fontSize);

        for(u64 graphemeIndex = 0; graphemeIndex < run->graphemeCount; graphemeIndex++)
        {
            oc_grapheme_info* grapheme = &run->graphemes[graphemeIndex];
            if(grapheme->offset + grapheme->count <= start)
            {
                continue;
            }
            else if(grapheme->offset >= end)
            {
                break;
            }
            else
            {
                oc_text_metrics graphemeMetrics = grapheme->metrics;
                graphemeMetrics.ink.x *= scale;
                graphemeMetrics.ink.y *= scale;
                graphemeMetrics.ink.w *= scale;
                graphemeMetrics.ink.h *= scale;
                graphemeMetrics.logical.x *= scale;
                graphemeMetrics.logical.y *= scale;
                graphemeMetrics.logical.w *= scale;
                graphemeMetrics.logical.h *= scale;
                graphemeMetrics.advance.x *= scale;
                graphemeMetrics.advance.y *= scale;

                graphemeMetrics.ink.x += runOffset.x;
                graphemeMetrics.ink.y += runOffset.y;
                graphemeMetrics.logical.x += runOffset.x;
                graphemeMetrics.logical.y += runOffset.y;

                if(!rangeStarted)
                {
                    rangeStarted = true;
                    result = graphemeMetrics;
                }
                else
                {
                    result = oc_text_metrics_combine(result, graphemeMetrics);
                }
            }
        }
    }
    return (result);
}

u64 oc_text_line_codepoint_index_for_position(oc_text_line* line, oc_vec2 position)
{
    ////////////////////////////////////////////////
    //TODO: this assumes LTR for now
    ////////////////////////////////////////////////
    oc_vec2 graphemePos = { 0 };
    bool found = false;
    u64 codepointIndex = 0;

    for(u64 runIndex = 0; runIndex < line->runCount; runIndex++)
    {
        oc_glyph_run* run = line->runs[runIndex];

        f32 scale = oc_font_get_scale_for_em_pixels(line->attributes[runIndex].font,
                                                    line->attributes[runIndex].fontSize);

        u64 graphemeIndex = 0;
        for(; graphemeIndex < run->graphemeCount; graphemeIndex++)
        {
            oc_grapheme_info* grapheme = &run->graphemes[graphemeIndex];

            oc_vec2 graphemeAdvance = oc_vec2_mul(scale, grapheme->metrics.advance);
            if(position.x < graphemePos.x + graphemeAdvance.x)
            {
                codepointIndex = grapheme->offset;
                if(position.x > graphemePos.x + graphemeAdvance.x / 2)
                {
                    codepointIndex += grapheme->count;
                }
                found = true;
                break;
            }
            graphemePos = oc_vec2_add(graphemePos, graphemeAdvance);
        }
        if(found)
        {
            break;
        }
    }
    if(!found)
    {
        codepointIndex = line->codepointCount;
    }
    return codepointIndex;
}

oc_vec2 oc_text_line_position_for_codepoint_index(oc_text_line* line, u64 codepointIndex)
{
    oc_vec2 graphemePos = { 0 };
    for(u64 runIndex = 0; runIndex < line->runCount; runIndex++)
    {
        oc_glyph_run* run = line->runs[runIndex];

        f32 scale = oc_font_get_scale_for_em_pixels(line->attributes[runIndex].font,
                                                    line->attributes[runIndex].fontSize);

        for(u64 graphemeIndex = 0; graphemeIndex < run->graphemeCount; graphemeIndex++)
        {
            oc_grapheme_info* grapheme = &run->graphemes[graphemeIndex];

            if(grapheme->offset >= codepointIndex)
            {
                break;
            }
            oc_vec2 advance = oc_vec2_mul(scale, grapheme->metrics.advance);
            graphemePos = oc_vec2_add(graphemePos, advance);
        }
    }
    return graphemePos;
}

void oc_text_line_draw(oc_text_line* line)
{
    for(u64 i = 0; i < line->runCount; i++)
    {
        oc_glyph_run* run = line->runs[i];
        oc_text_attributes* attributes = &line->attributes[i];

        oc_set_color(attributes->color);
        oc_text_draw_run(run, line->attributes[i].fontSize);
    }
}

void oc_text_draw_utf32(oc_str32 codepoints, oc_font font, f32 fontSize)
{
    oc_arena_scope scratch = oc_scratch_begin();

    oc_text_line* line = oc_text_line_from_utf32(scratch.arena,
                                                 codepoints,
                                                 &(oc_text_attributes){
                                                     .font = font,
                                                     .fontSize = fontSize,
                                                     .color = oc_get_color(),
                                                 });
    oc_text_line_draw(line);
    oc_scratch_end(scratch);
}

void oc_text_draw_utf8(oc_str8 text, oc_font font, f32 fontSize)
{
    oc_arena_scope scratch = oc_scratch_begin();

    oc_str32 codepoints = oc_utf8_push_to_codepoints(scratch.arena, text);
    oc_text_draw_utf32(codepoints, font, fontSize);

    oc_scratch_end(scratch);
}

//------------------------------------------------------------------------------------------
//NOTE(martin): clear/fill/stroke
//------------------------------------------------------------------------------------------

void oc_clear()
{
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        context->primitiveCount = 0;
        context->clearColor = context->attributes.colors[0];
        context->clear = true;
    }
}

void oc_fill()
{
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context && context->path.count)
    {
        oc_push_command(context, (oc_primitive){ .cmd = OC_CMD_FILL, .path = context->path });
        oc_new_path(context);
    }
}

void oc_stroke()
{
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context && context->path.count)
    {
        oc_push_command(context, (oc_primitive){ .cmd = OC_CMD_STROKE, .path = context->path });
        oc_new_path(context);
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
    oc_close_path();
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

//------------------------------------------------------------------------------------------
//NOTE(martin): images
//------------------------------------------------------------------------------------------

oc_image oc_image_nil()
{
    return ((oc_image){ .h = 0 });
}

bool oc_image_is_nil(oc_image image)
{
    return (image.h == 0);
}

oc_image oc_image_create_from_rgba8(oc_canvas_renderer renderer, u32 width, u32 height, u8* pixels)
{
    oc_image image = oc_image_create(renderer, width, height);
    if(!oc_image_is_nil(image))
    {
        oc_image_upload_region_rgba8(image, (oc_rect){ 0, 0, width, height }, pixels);
    }
    return (image);
}

oc_image oc_image_create_from_memory(oc_canvas_renderer renderer, oc_str8 mem, bool flip)
{
    oc_image image = oc_image_nil();
    int width, height, channels;

    stbi_set_flip_vertically_on_load(flip ? 1 : 0);
    u8* pixels = stbi_load_from_memory((u8*)mem.ptr, mem.len, &width, &height, &channels, 4);

    if(pixels)
    {
        image = oc_image_create_from_rgba8(renderer, width, height, pixels);
        free(pixels);
    }
    else
    {
        oc_log_error("stbi_load_from_memory() failed: %s\n", stbi_failure_reason());
    }
    return (image);
}

oc_image oc_image_create_from_file(oc_canvas_renderer renderer, oc_file file, bool flip)
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
        image = oc_image_create_from_memory(renderer, oc_str8_from_buffer(size, buffer), flip);
    }

    oc_scratch_end(scratch);
    return (image);
}

oc_image oc_image_create_from_path(oc_canvas_renderer renderer, oc_str8 path, bool flip)
{
    oc_image image = oc_image_nil();

    oc_file file = oc_file_open(path, OC_FILE_ACCESS_READ, OC_FILE_OPEN_NONE);
    if(oc_file_last_error(file) != OC_IO_OK)
    {
        oc_log_error("Could not open file %*.s\n", oc_str8_ip(path));
    }
    else
    {
        image = oc_image_create_from_file(renderer, file, flip);
    }
    oc_file_close(file);
    return (image);
}

void oc_image_draw_region(oc_image image, oc_rect srcRegion, oc_rect dstRegion)
{
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(context)
    {
        oc_image oldImage = context->attributes.image;
        oc_rect oldSrcRegion = context->attributes.srcRegion;
        oc_color oldColors[4];
        memcpy(oldColors, context->attributes.colors, 4 * sizeof(oc_color));
        bool oldHasGradient = context->attributes.hasGradient;

        context->attributes.image = image;
        context->attributes.srcRegion = srcRegion;
        context->attributes.colors[0] = (oc_color){ 1, 1, 1, 1 };
        context->attributes.hasGradient = false;

        oc_move_to(dstRegion.x, dstRegion.y);
        oc_line_to(dstRegion.x + dstRegion.w, dstRegion.y);
        oc_line_to(dstRegion.x + dstRegion.w, dstRegion.y + dstRegion.h);
        oc_line_to(dstRegion.x, dstRegion.y + dstRegion.h);
        oc_close_path();

        oc_fill();

        context->attributes.image = oldImage;
        context->attributes.srcRegion = oldSrcRegion;
        memcpy(context->attributes.colors, oldColors, 4 * sizeof(oc_color));
        context->attributes.hasGradient = oldHasGradient;
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
