/************************************************************/ /**
*
*	@file: cheatsheet_graphics.h
*	@author: Martin Fouilleul
*	@date: 05/09/2023
*
*****************************************************************/

//------------------------------------------------------------------------------------------
// graphics surface
//------------------------------------------------------------------------------------------
oc_surface oc_surface_nil(void);
bool oc_surface_is_nil(oc_surface surface);
oc_surface oc_surface_canvas();
oc_surface oc_surface_gles();
void oc_surface_destroy(oc_surface surface);

void oc_surface_select(oc_surface surface);
void oc_surface_deselect(void);
void oc_surface_present(oc_surface surface);

oc_vec2 oc_surface_get_size(oc_surface surface);
oc_vec2 oc_surface_contents_scaling(oc_surface surface);
void oc_surface_bring_to_front(oc_surface surface);
void oc_surface_send_to_back(oc_surface surface);

//------------------------------------------------------------------------------------------
// 2D canvas command buffer
//------------------------------------------------------------------------------------------
oc_canvas oc_canvas_nil(void);
bool oc_canvas_is_nil(oc_canvas canvas);
oc_canvas oc_canvas_create(void);
void oc_canvas_destroy(oc_canvas canvas);
oc_canvas oc_canvas_set_current(oc_canvas canvas);
void oc_render(oc_canvas canvas);

//------------------------------------------------------------------------------------------
// transform and clipping
//------------------------------------------------------------------------------------------

void oc_matrix_push(oc_mat2x3 matrix);
void oc_matrix_pop(void);
oc_mat2x3 oc_matrix_top();

void oc_clip_push(f32 x, f32 y, f32 w, f32 h);
void oc_clip_pop(void);
oc_rect oc_clip_top();

//------------------------------------------------------------------------------------------
// graphics attributes setting/getting
//------------------------------------------------------------------------------------------
void oc_set_color(oc_color color);
void oc_set_color_rgba(f32 r, f32 g, f32 b, f32 a);
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

//------------------------------------------------------------------------------------------
// path construction
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
// clear/fill/stroke
//------------------------------------------------------------------------------------------
void oc_clear(void);
void oc_fill(void);
void oc_stroke(void);

//------------------------------------------------------------------------------------------
// shapes helpers
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
void oc_image_draw(oc_image image, oc_rect rect);
void oc_image_draw_region(oc_image image, oc_rect srcRegion, oc_rect dstRegion);

//------------------------------------------------------------------------------------------
// fonts
//------------------------------------------------------------------------------------------
oc_font oc_font_nil(void);
bool oc_font_is_nil(oc_font font);

oc_font oc_font_create_from_memory(oc_str8 mem, u32 rangeCount, oc_unicode_range* ranges);
void oc_font_destroy(oc_font font);

oc_font_extents oc_font_get_extents(oc_font font);
oc_font_extents oc_font_get_scaled_extents(oc_font font, f32 emSize);
f32 oc_font_get_scale_for_em_pixels(oc_font font, f32 emSize);

u32 oc_font_get_glyph_index(oc_font font, oc_utf32 codePoint);
oc_str32 oc_font_get_glyph_indices(oc_font font, oc_str32 codePoints, oc_str32 backing);
oc_str32 oc_font_push_glyph_indices(oc_font font, oc_arena* arena, oc_str32 codePoints);

int oc_font_get_codepoint_extents(oc_font font, oc_utf32 codePoint, oc_text_extents* outExtents);
int oc_font_get_glyph_extents(oc_font font, oc_str32 glyphIndices, oc_text_extents* outExtents);

oc_rect oc_text_bounding_box_utf32(oc_font font, f32 fontSize, oc_str32 text);
oc_rect oc_text_bounding_box(oc_font font, f32 fontSize, oc_str8 text);

//------------------------------------------------------------------------------------------
// images
//------------------------------------------------------------------------------------------
oc_image oc_image_nil(void);
bool oc_image_is_nil(oc_image a);

oc_image oc_image_create(oc_surface surface, u32 width, u32 height);
oc_image oc_image_create_from_rgba8(oc_surface surface, u32 width, u32 height, u8* pixels);
oc_image oc_image_create_from_memory(oc_surface surface, oc_str8 mem, bool flip);
oc_image oc_image_create_from_file(oc_surface surface, oc_str8 path, bool flip);

void oc_image_destroy(oc_image image);

void oc_image_upload_region_rgba8(oc_image image, oc_rect region, u8* pixels);
oc_vec2 oc_image_size(oc_image image);

//------------------------------------------------------------------------------------------
// image atlas
//------------------------------------------------------------------------------------------

oc_rect_atlas* oc_rect_atlas_create(oc_arena* arena, i32 width, i32 height);
oc_rect oc_rect_atlas_alloc(oc_rect_atlas* atlas, i32 width, i32 height);
void oc_rect_atlas_recycle(oc_rect_atlas* atlas, oc_rect rect);

oc_image_region oc_image_atlas_alloc_from_rgba8(oc_rect_atlas* atlas, oc_image backingImage, u32 width, u32 height, u8* pixels);
oc_image_region oc_image_atlas_alloc_from_data(oc_rect_atlas* atlas, oc_image backingImage, oc_str8 data, bool flip);
oc_image_region oc_image_atlas_alloc_from_file(oc_rect_atlas* atlas, oc_image backingImage, oc_str8 path, bool flip);
void oc_image_atlas_recycle(oc_rect_atlas* atlas, oc_image_region imageRgn);
