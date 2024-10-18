/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

//------------------------------------------------------------------------------------------
//graphics surface
//------------------------------------------------------------------------------------------
oc_surface oc_surface_nil(void);
bool oc_surface_is_nil(oc_surface surface);

void oc_surface_destroy(oc_surface surface);

oc_vec2 oc_surface_get_size(oc_surface surface);
oc_vec2 oc_surface_contents_scaling(oc_surface surface);
void oc_surface_bring_to_front(oc_surface surface);
void oc_surface_send_to_back(oc_surface surface);
bool oc_surface_get_hidden(oc_surface surface);
void oc_surface_set_hidden(oc_surface surface, bool hidden);

oc_surface oc_gles_surface_create(void);
void oc_gles_surface_make_current(oc_surface surface);
void oc_gles_surface_swap_interval(oc_surface surface, int interval);
void oc_gles_surface_swap_buffers(oc_surface surface);

//------------------------------------------------------------------------------------------
//color helpers
//------------------------------------------------------------------------------------------
oc_color oc_color_rgba(f32 r, f32 g, f32 b, f32 a);
oc_color oc_color_srgba(f32 r, f32 g, f32 b, f32 a);
oc_color oc_color_convert(oc_color color, oc_color_space colorSpace);

//------------------------------------------------------------------------------------------
//canvas renderer
//------------------------------------------------------------------------------------------
oc_canvas_renderer oc_canvas_renderer_nil(void);
bool oc_canvas_renderer_is_nil(oc_canvas_renderer renderer);

oc_canvas_renderer oc_canvas_renderer_create(void);
void oc_canvas_renderer_destroy(oc_canvas_renderer renderer);

void oc_canvas_render(oc_canvas_renderer renderer, oc_canvas_context context, oc_surface surface);
void oc_canvas_present(oc_canvas_renderer renderer, oc_surface surface);

//------------------------------------------------------------------------------------------
//canvas surface
//------------------------------------------------------------------------------------------
oc_surface oc_canvas_surface_create(oc_canvas_renderer renderer);
void oc_canvas_surface_swap_interval(oc_surface surface, int swap);

//------------------------------------------------------------------------------------------
//canvas context
//------------------------------------------------------------------------------------------
oc_canvas_context oc_canvas_context_nil(void);
bool oc_canvas_context_is_nil(oc_canvas_context context);

oc_canvas_context oc_canvas_context_create(void);
void oc_canvas_context_destroy(oc_canvas_context context);
oc_canvas_context oc_canvas_context_select(oc_canvas_context context);

void oc_canvas_context_set_msaa_sample_count(oc_canvas_context context, u32 sampleCount);

//------------------------------------------------------------------------------------------
//fonts
//------------------------------------------------------------------------------------------
oc_font oc_font_nil(void);
bool oc_font_is_nil(oc_font font);

oc_font oc_font_create_from_memory(oc_str8 mem, u32 rangeCount, oc_unicode_range* ranges);
oc_font oc_font_create_from_file(oc_file file, u32 rangeCount, oc_unicode_range* ranges);
oc_font oc_font_create_from_path(oc_str8 path, u32 rangeCount, oc_unicode_range* ranges);

void oc_font_destroy(oc_font font);

oc_str32 oc_font_get_glyph_indices(oc_font font, oc_str32 codePoints, oc_str32 backing);
oc_str32 oc_font_push_glyph_indices(oc_arena* arena, oc_font font, oc_str32 codePoints);
u32 oc_font_get_glyph_index(oc_font font, oc_utf32 codePoint);

// metrics
oc_font_metrics oc_font_get_metrics(oc_font font, f32 emSize);
oc_font_metrics oc_font_get_metrics_unscaled(oc_font font);
f32 oc_font_get_scale_for_em_pixels(oc_font font, f32 emSize);

oc_text_metrics oc_font_text_metrics_utf32(oc_font font, f32 fontSize, oc_str32 codepoints);
oc_text_metrics oc_font_text_metrics(oc_font font, f32 fontSize, oc_str8 text);

//------------------------------------------------------------------------------------------
//images
//------------------------------------------------------------------------------------------
oc_image oc_image_nil(void);
bool oc_image_is_nil(oc_image a);
oc_image oc_image_create(oc_canvas_renderer renderer, u32 width, u32 height);
oc_image oc_image_create_from_rgba8(oc_canvas_renderer renderer, u32 width, u32 height, u8* pixels);
oc_image oc_image_create_from_memory(oc_canvas_renderer renderer, oc_str8 mem, bool flip);
oc_image oc_image_create_from_file(oc_canvas_renderer renderer, oc_file file, bool flip);
oc_image oc_image_create_from_path(oc_canvas_renderer renderer, oc_str8 path, bool flip);
void oc_image_destroy(oc_image image);
void oc_image_upload_region_rgba8(oc_image image, oc_rect region, u8* pixels);
oc_vec2 oc_image_size(oc_image image);

//------------------------------------------------------------------------------------------
//atlasing
//------------------------------------------------------------------------------------------

oc_rect_atlas* oc_rect_atlas_create(oc_arena* arena, i32 width, i32 height);
oc_rect oc_rect_atlas_alloc(oc_rect_atlas* atlas, i32 width, i32 height);
void oc_rect_atlas_recycle(oc_rect_atlas* atlas, oc_rect rect);
oc_image_region oc_image_atlas_alloc_from_rgba8(oc_rect_atlas* atlas, oc_image backingImage, u32 width, u32 height, u8* pixels);
oc_image_region oc_image_atlas_alloc_from_memory(oc_rect_atlas* atlas, oc_image backingImage, oc_str8 mem, bool flip);
oc_image_region oc_image_atlas_alloc_from_file(oc_rect_atlas* atlas, oc_image backingImage, oc_file file, bool flip);
oc_image_region oc_image_atlas_alloc_from_path(oc_rect_atlas* atlas, oc_image backingImage, oc_str8 path, bool flip);
void oc_image_atlas_recycle(oc_rect_atlas* atlas, oc_image_region imageRgn);

//------------------------------------------------------------------------------------------
//transform, viewport and clipping
//------------------------------------------------------------------------------------------
void oc_matrix_push(oc_mat2x3 matrix);
void oc_matrix_multiply_push(oc_mat2x3 matrix);
void oc_matrix_pop(void);
oc_mat2x3 oc_matrix_top();
void oc_clip_push(f32 x, f32 y, f32 w, f32 h);
void oc_clip_pop(void);
oc_rect oc_clip_top();

//------------------------------------------------------------------------------------------
//graphics attributes setting/getting
//------------------------------------------------------------------------------------------
void oc_set_color(oc_color color);
void oc_set_color_rgba(f32 r, f32 g, f32 b, f32 a);
void oc_set_color_srgba(f32 r, f32 g, f32 b, f32 a);
void oc_set_gradient(oc_gradient_blend_space blendSpace, oc_color bottomLeft, oc_color bottomRight, oc_color topRight, oc_color topLeft);
void oc_set_width(f32 width);
void oc_set_tolerance(f32 tolerance);
void oc_set_joint(oc_joint_type joint);
void oc_set_max_joint_excursion(f32 maxJointExcursion);
void oc_set_cap(oc_cap_type cap);
void oc_set_font(oc_font font);
void oc_set_font_size(f32 size);
void oc_set_text_flip(bool flip);
void oc_set_image(oc_image image);
void oc_set_image_source_region(oc_rect region);

oc_color oc_get_color(void);
f32 oc_get_width(void);
f32 oc_get_tolerance(void);
oc_joint_type oc_get_joint(void);
f32 oc_get_max_joint_excursion(void);
oc_cap_type oc_get_cap(void);
oc_font oc_get_font(void);
f32 oc_get_font_size(void);
bool oc_get_text_flip(void);
oc_image oc_get_image();
oc_rect oc_get_image_source_region();

//------------------------------------------------------------------------------------------
//path construction
//------------------------------------------------------------------------------------------
oc_vec2 oc_get_position(void);
void oc_move_to(f32 x, f32 y);
void oc_line_to(f32 x, f32 y);
void oc_quadratic_to(f32 x1, f32 y1, f32 x2, f32 y2);
void oc_cubic_to(f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3);
void oc_close_path(void);

oc_rect oc_glyph_outlines(oc_str32 glyphIndices);
void oc_codepoints_outlines(oc_str32 string);
void oc_text_outlines(oc_str8 string);

//------------------------------------------------------------------------------------------
//clear/fill/stroke
//------------------------------------------------------------------------------------------
void oc_clear(void);
void oc_fill(void);
void oc_stroke(void);

//------------------------------------------------------------------------------------------
//shapes helpers
//------------------------------------------------------------------------------------------
void oc_rectangle_fill(f32 x, f32 y, f32 w, f32 h);
void oc_rectangle_stroke(f32 x, f32 y, f32 w, f32 h);
void oc_rounded_rectangle_fill(f32 x, f32 y, f32 w, f32 h, f32 r);
void oc_rounded_rectangle_stroke(f32 x, f32 y, f32 w, f32 h, f32 r);
void oc_ellipse_fill(f32 x, f32 y, f32 rx, f32 ry);
void oc_ellipse_stroke(f32 x, f32 y, f32 rx, f32 ry);
void oc_circle_fill(f32 x, f32 y, f32 r);
void oc_circle_stroke(f32 x, f32 y, f32 r);
void oc_arc(f32 x, f32 y, f32 r, f32 arcAngle, f32 startAngle);

void oc_text_fill(f32 x, f32 y, oc_str8 text);

//NOTE: image helpers
void oc_image_draw(oc_image image, oc_rect rect);
void oc_image_draw_region(oc_image image, oc_rect srcRegion, oc_rect dstRegion);
