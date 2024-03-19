/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#ifndef __GRAPHICS_H_
#define __GRAPHICS_H_

#include "app/app.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------------------
//SECTION: graphics surface
//------------------------------------------------------------------------------------------
typedef struct oc_surface
{
    u64 h;
} oc_surface;

ORCA_API oc_surface oc_surface_nil(void);
ORCA_API bool oc_surface_is_nil(oc_surface surface);

ORCA_API void oc_surface_destroy(oc_surface surface);

ORCA_API oc_vec2 oc_surface_get_size(oc_surface surface);
ORCA_API oc_vec2 oc_surface_contents_scaling(oc_surface surface);
ORCA_API void oc_surface_bring_to_front(oc_surface surface);
ORCA_API void oc_surface_send_to_back(oc_surface surface);
ORCA_API bool oc_surface_get_hidden(oc_surface surface);
ORCA_API void oc_surface_set_hidden(oc_surface surface, bool hidden);

//------------------------------------------------------------------------------------------
//SECTION: graphics canvas structs
//------------------------------------------------------------------------------------------

typedef struct oc_canvas_renderer
{
    u64 h;
} oc_canvas_renderer;

typedef struct oc_canvas_context
{
    u64 h;
} oc_canvas_context;

typedef struct oc_font
{
    u64 h;
} oc_font;

typedef struct oc_image
{
    u64 h;
} oc_image;

typedef struct oc_color
{
    union
    {
        struct
        {
            f32 r;
            f32 g;
            f32 b;
            f32 a;
        };

        f32 c[4];
    };
} oc_color;

typedef enum
{
    OC_JOINT_MITER = 0,
    OC_JOINT_BEVEL,
    OC_JOINT_NONE
} oc_joint_type;

typedef enum
{
    OC_CAP_NONE = 0,
    OC_CAP_SQUARE
} oc_cap_type;

typedef struct oc_font_metrics
{
    f32 ascent;    // the extent above the baseline (by convention a positive value extends above the baseline)
    f32 descent;   // the extent below the baseline (by convention, positive value extends below the baseline)
    f32 lineGap;   // spacing between one row's descent and the next row's ascent
    f32 xHeight;   // height of the lower case letter 'x'
    f32 capHeight; // height of the upper case letter 'M'
    f32 width;     // maximum width of the font

} oc_font_metrics;

typedef struct oc_glyph_metrics
{
    oc_rect ink;
    oc_vec2 advance;
} oc_glyph_metrics;

typedef struct oc_text_metrics
{
    oc_rect ink;
    oc_rect logical;
    oc_vec2 advance;

} oc_text_metrics;

//------------------------------------------------------------------------------------------
//SECTION: canvas renderer
//------------------------------------------------------------------------------------------
ORCA_API oc_canvas_renderer oc_canvas_renderer_nil(void);
ORCA_API bool oc_canvas_renderer_is_nil(oc_canvas_renderer renderer);

ORCA_API oc_canvas_renderer oc_canvas_renderer_create(void);
ORCA_API void oc_canvas_renderer_destroy(oc_canvas_renderer renderer);

ORCA_API void oc_canvas_render(oc_canvas_renderer renderer, oc_canvas_context context, oc_surface surface);
ORCA_API void oc_canvas_present(oc_canvas_renderer renderer, oc_surface surface);

//------------------------------------------------------------------------------------------
//SECTION: canvas surface
//------------------------------------------------------------------------------------------
#if OC_PLATFORM_ORCA
ORCA_API oc_surface oc_canvas_surface_create(oc_canvas_renderer renderer);
#else
ORCA_API oc_surface oc_canvas_surface_create_for_window(oc_canvas_renderer renderer, oc_window window);
#endif

ORCA_API void oc_canvas_surface_swap_interval(oc_surface surface, int swap);

//------------------------------------------------------------------------------------------
//SECTION: canvas context
//------------------------------------------------------------------------------------------
ORCA_API oc_canvas_context oc_canvas_context_nil(void);
ORCA_API bool oc_canvas_context_is_nil(oc_canvas_context context);

ORCA_API oc_canvas_context oc_canvas_context_create(void);
ORCA_API void oc_canvas_context_destroy(oc_canvas_context context);
ORCA_API oc_canvas_context oc_canvas_context_select(oc_canvas_context context);

ORCA_API void oc_canvas_context_set_msaa_sample_count(oc_canvas_context context, u32 sampleCount);
//------------------------------------------------------------------------------------------
//SECTION: fonts
//------------------------------------------------------------------------------------------
ORCA_API oc_font oc_font_nil(void);
ORCA_API bool oc_font_is_nil(oc_font font);

ORCA_API oc_font oc_font_create_from_memory(oc_str8 mem, u32 rangeCount, oc_unicode_range* ranges);
ORCA_API oc_font oc_font_create_from_file(oc_file file, u32 rangeCount, oc_unicode_range* ranges);
ORCA_API oc_font oc_font_create_from_path(oc_str8 path, u32 rangeCount, oc_unicode_range* ranges);

ORCA_API void oc_font_destroy(oc_font font);

ORCA_API oc_str32 oc_font_get_glyph_indices(oc_font font, oc_str32 codePoints, oc_str32 backing);
ORCA_API oc_str32 oc_font_push_glyph_indices(oc_arena* arena, oc_font font, oc_str32 codePoints);
ORCA_API u32 oc_font_get_glyph_index(oc_font font, oc_utf32 codePoint);

// metrics
ORCA_API oc_font_metrics oc_font_get_metrics(oc_font font, f32 emSize);
ORCA_API oc_font_metrics oc_font_get_metrics_unscaled(oc_font font);
ORCA_API f32 oc_font_get_scale_for_em_pixels(oc_font font, f32 emSize);

ORCA_API oc_text_metrics oc_font_text_metrics_utf32(oc_font font, f32 fontSize, oc_str32 codepoints);
ORCA_API oc_text_metrics oc_font_text_metrics(oc_font font, f32 fontSize, oc_str8 text);

//------------------------------------------------------------------------------------------
//SECTION: images
//------------------------------------------------------------------------------------------
ORCA_API oc_image oc_image_nil(void);
ORCA_API bool oc_image_is_nil(oc_image a);

ORCA_API oc_image oc_image_create(oc_canvas_renderer renderer, u32 width, u32 height);
ORCA_API oc_image oc_image_create_from_rgba8(oc_canvas_renderer renderer, u32 width, u32 height, u8* pixels);
ORCA_API oc_image oc_image_create_from_memory(oc_canvas_renderer renderer, oc_str8 mem, bool flip);
ORCA_API oc_image oc_image_create_from_file(oc_canvas_renderer renderer, oc_file file, bool flip);
ORCA_API oc_image oc_image_create_from_path(oc_canvas_renderer renderer, oc_str8 path, bool flip);

ORCA_API void oc_image_destroy(oc_image image);

ORCA_API void oc_image_upload_region_rgba8(oc_image image, oc_rect region, u8* pixels);
ORCA_API oc_vec2 oc_image_size(oc_image image);

//------------------------------------------------------------------------------------------
//SECTION: atlasing
//------------------------------------------------------------------------------------------

//NOTE: rectangle allocator
typedef struct oc_rect_atlas oc_rect_atlas;

ORCA_API oc_rect_atlas* oc_rect_atlas_create(oc_arena* arena, i32 width, i32 height);
ORCA_API oc_rect oc_rect_atlas_alloc(oc_rect_atlas* atlas, i32 width, i32 height);
ORCA_API void oc_rect_atlas_recycle(oc_rect_atlas* atlas, oc_rect rect);

//NOTE: image atlas helpers
typedef struct oc_image_region
{
    oc_image image;
    oc_rect rect;
} oc_image_region;

ORCA_API oc_image_region oc_image_atlas_alloc_from_rgba8(oc_rect_atlas* atlas, oc_image backingImage, u32 width, u32 height, u8* pixels);
ORCA_API oc_image_region oc_image_atlas_alloc_from_memory(oc_rect_atlas* atlas, oc_image backingImage, oc_str8 mem, bool flip);
ORCA_API oc_image_region oc_image_atlas_alloc_from_file(oc_rect_atlas* atlas, oc_image backingImage, oc_file file, bool flip);
ORCA_API oc_image_region oc_image_atlas_alloc_from_path(oc_rect_atlas* atlas, oc_image backingImage, oc_str8 path, bool flip);
ORCA_API void oc_image_atlas_recycle(oc_rect_atlas* atlas, oc_image_region imageRgn);

//------------------------------------------------------------------------------------------
//SECTION: transform, viewport and clipping
//------------------------------------------------------------------------------------------
ORCA_API void oc_matrix_push(oc_mat2x3 matrix);
ORCA_API void oc_matrix_multiply_push(oc_mat2x3 matrix);
ORCA_API void oc_matrix_pop(void);
ORCA_API oc_mat2x3 oc_matrix_top();

ORCA_API void oc_clip_push(f32 x, f32 y, f32 w, f32 h);
ORCA_API void oc_clip_pop(void);
ORCA_API oc_rect oc_clip_top();

//------------------------------------------------------------------------------------------
//SECTION: graphics attributes setting/getting
//------------------------------------------------------------------------------------------
ORCA_API void oc_set_color(oc_color color);
ORCA_API void oc_set_color_rgba(f32 r, f32 g, f32 b, f32 a);
ORCA_API void oc_set_gradient(oc_color bottomLeft, oc_color bottomRight, oc_color topRight, oc_color topLeft);
ORCA_API void oc_set_width(f32 width);
ORCA_API void oc_set_tolerance(f32 tolerance);
ORCA_API void oc_set_joint(oc_joint_type joint);
ORCA_API void oc_set_max_joint_excursion(f32 maxJointExcursion);
ORCA_API void oc_set_cap(oc_cap_type cap);
ORCA_API void oc_set_font(oc_font font);
ORCA_API void oc_set_font_size(f32 size);
ORCA_API void oc_set_text_flip(bool flip);
ORCA_API void oc_set_image(oc_image image);
ORCA_API void oc_set_image_source_region(oc_rect region);

ORCA_API oc_color oc_get_color(void);
ORCA_API f32 oc_get_width(void);
ORCA_API f32 oc_get_tolerance(void);
ORCA_API oc_joint_type oc_get_joint(void);
ORCA_API f32 oc_get_max_joint_excursion(void);
ORCA_API oc_cap_type oc_get_cap(void);
ORCA_API oc_font oc_get_font(void);
ORCA_API f32 oc_get_font_size(void);
ORCA_API bool oc_get_text_flip(void);
ORCA_API oc_image oc_get_image();
ORCA_API oc_rect oc_get_image_source_region();

//------------------------------------------------------------------------------------------
//SECTION: path construction
//------------------------------------------------------------------------------------------
ORCA_API oc_vec2 oc_get_position(void);
ORCA_API void oc_move_to(f32 x, f32 y);
ORCA_API void oc_line_to(f32 x, f32 y);
ORCA_API void oc_quadratic_to(f32 x1, f32 y1, f32 x2, f32 y2);
ORCA_API void oc_cubic_to(f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3);
ORCA_API void oc_close_path(void);

ORCA_API oc_rect oc_glyph_outlines(oc_str32 glyphIndices);
ORCA_API void oc_codepoints_outlines(oc_str32 string);
ORCA_API void oc_text_outlines(oc_str8 string);

//------------------------------------------------------------------------------------------
//SECTION: clear/fill/stroke
//------------------------------------------------------------------------------------------
ORCA_API void oc_clear(void);
ORCA_API void oc_fill(void);
ORCA_API void oc_stroke(void);

//------------------------------------------------------------------------------------------
//SECTION: shapes helpers
//------------------------------------------------------------------------------------------
ORCA_API void oc_rectangle_fill(f32 x, f32 y, f32 w, f32 h);
ORCA_API void oc_rectangle_stroke(f32 x, f32 y, f32 w, f32 h);
ORCA_API void oc_rounded_rectangle_fill(f32 x, f32 y, f32 w, f32 h, f32 r);
ORCA_API void oc_rounded_rectangle_stroke(f32 x, f32 y, f32 w, f32 h, f32 r);
ORCA_API void oc_ellipse_fill(f32 x, f32 y, f32 rx, f32 ry);
ORCA_API void oc_ellipse_stroke(f32 x, f32 y, f32 rx, f32 ry);
ORCA_API void oc_circle_fill(f32 x, f32 y, f32 r);
ORCA_API void oc_circle_stroke(f32 x, f32 y, f32 r);
ORCA_API void oc_arc(f32 x, f32 y, f32 r, f32 arcAngle, f32 startAngle);

ORCA_API void oc_text_fill(f32 x, f32 y, oc_str8 text);

//NOTE: image helpers
ORCA_API void oc_image_draw(oc_image image, oc_rect rect);
ORCA_API void oc_image_draw_region(oc_image image, oc_rect srcRegion, oc_rect dstRegion);

#ifdef __cplusplus
}
#endif

#endif //__GRAPHICS_H_
