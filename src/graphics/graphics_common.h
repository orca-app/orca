/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#ifndef __GRAPHICS_COMMON_H_
#define __GRAPHICS_COMMON_H_

#include "graphics.h"

//------------------------------------------------------------------------
// canvas structs
//------------------------------------------------------------------------

typedef struct oc_glyph_info
{
    u32 index;
    oc_vec2 offset;
    oc_text_metrics metrics;
} oc_glyph_info;

typedef struct oc_grapheme_info
{
    u64 offset;
    u64 count;
    oc_text_metrics metrics;
} oc_grapheme_info;

typedef struct oc_glyph_run
{
    oc_font font;

    u32 codepointCount;

    u32 glyphCount;
    oc_glyph_info* glyphs;

    u32 graphemeCount;
    oc_grapheme_info* graphemes;

    oc_text_metrics metrics;

} oc_glyph_run;

typedef struct oc_harfbuzz_handle
{
    u64 h;
} oc_harfbuzz_handle;

oc_rect oc_rect_combine(oc_rect a, oc_rect b);
oc_text_metrics oc_text_metrics_combine(oc_text_metrics a, oc_text_metrics b);

typedef enum
{
    OC_PATH_MOVE,
    OC_PATH_LINE,
    OC_PATH_QUADRATIC,
    OC_PATH_CUBIC
} oc_path_elt_type;

typedef struct oc_path_elt
{
    oc_path_elt_type type;
    oc_vec2 p[3];

} oc_path_elt;

typedef struct oc_path_descriptor
{
    u32 startIndex;
    u32 count;
    oc_vec2 startPoint;

} oc_path_descriptor;

typedef struct oc_attributes
{
    f32 width;
    f32 tolerance;
    bool hasGradient;
    oc_gradient_blend_space blendSpace;
    oc_color colors[4];
    oc_joint_type joint;
    f32 maxJointExcursion;
    oc_cap_type cap;
    oc_fill_rule fillRule;

    oc_image image;
    oc_rect srcRegion;

    oc_mat2x3 transform;
    oc_rect clip;

} oc_attributes;

typedef enum
{
    OC_CMD_FILL = 0,
    OC_CMD_STROKE,
    OC_CMD_JUMP
} oc_primitive_cmd;

typedef struct oc_primitive
{
    oc_primitive_cmd cmd;
    oc_attributes attributes;

    union
    {
        oc_path_descriptor path;
        oc_rect rect;
        u32 jump;
    };

} oc_primitive;

ORCA_API oc_harfbuzz_handle oc_harfbuzz_font_create(oc_str8 mem);
ORCA_API void oc_harfbuzz_font_destroy(oc_harfbuzz_handle handle);
ORCA_API f32 oc_harfbuzz_font_get_upem(oc_harfbuzz_handle handle);
ORCA_API oc_font_metrics oc_harfbuzz_font_get_metrics(oc_harfbuzz_handle handle);
ORCA_API oc_glyph_run* oc_harfbuzz_font_shape(oc_arena* arena,
                                              oc_harfbuzz_handle handle,
                                              oc_text_shape_settings* settings,
                                              oc_str32 codepoints,
                                              u64 begin,
                                              u64 end);

ORCA_API oc_path_elt* oc_harfbuzz_get_curves(oc_arena* arena,
                                             oc_harfbuzz_handle handle,
                                             oc_glyph_run* run,
                                             oc_vec2 start,
                                             f32 scale,
                                             bool flipY,
                                             u32* eltCount);

ORCA_API void oc_canvas_renderer_submit(oc_canvas_renderer renderer,
                                        oc_surface surface,
                                        u32 msaaSampleCount,
                                        bool clear,
                                        oc_color clearColor,
                                        u32 primitiveCount,
                                        oc_primitive* primitives,
                                        u32 eltCount,
                                        oc_path_elt* elements);

#endif //__GRAPHICS_COMMON_H_
