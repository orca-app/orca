	/************************************************************//**
*
*	@file: graphics.h
*	@author: Martin Fouilleul
*	@date: 01/08/2022
*	@revision:
*
*****************************************************************/
#ifndef __GRAPHICS_H_
#define __GRAPHICS_H_

#include"mp_app.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------------------
//NOTE(martin): graphics surface
//------------------------------------------------------------------------------------------

typedef struct mg_surface { u64 h; } mg_surface;

typedef enum { MG_BACKEND_DUMMY,
               MG_BACKEND_METAL,
               MG_BACKEND_GLES,
               //...
             } mg_backend_id;

void mg_init();

mg_surface mg_surface_nil();
mg_surface mg_surface_create_for_window(mp_window window, mg_backend_id backend);
mg_surface mg_surface_create_for_view(mp_view view, mg_backend_id backend);
mg_surface mg_surface_create_offscreen(mg_backend_id backend, u32 width, u32 height);

void mg_surface_destroy(mg_surface surface);
void* mg_surface_get_os_resource(mg_surface surface);

void mg_surface_prepare(mg_surface surface);
void mg_surface_present(mg_surface surface);
void mg_surface_resize(mg_surface surface, int width, int height);
void mg_surface_set_hidden(mg_surface surface, bool hidden);

vec2 mg_surface_size(mg_surface surface);

//------------------------------------------------------------------------------------------
//NOTE(martin): graphics surface sharing
//------------------------------------------------------------------------------------------
typedef void* mg_surface_server_id;

typedef struct mg_surface_server { u64 h; } mg_surface_server;
typedef struct mg_surface_client { u64 h; } mg_surface_client;

mg_surface_server mg_surface_server_create(mg_surface surface);
mg_surface_server mg_surface_server_create_native(void* p);
void mg_surface_server_destroy(mg_surface_server server);
mg_surface_server_id mg_surface_server_get_id(mg_surface_server server);

mg_surface_client mg_surface_client_create(mg_surface_server_id id);
void mg_surface_client_destroy(mg_surface_client client);
void mg_surface_client_attach_to_view(mg_surface_client client, mp_view view);
void mg_surface_client_detach(mg_surface_client client);

//------------------------------------------------------------------------------------------
//NOTE(martin): canvas drawing structs
//------------------------------------------------------------------------------------------

typedef struct mg_canvas { u64 h; } mg_canvas;
typedef struct mg_stream { u64 h; } mg_stream;
typedef struct mg_font { u64 h; } mg_font;

typedef struct mg_mat2x3
{
	f32 m[6];
} mg_mat2x3;

typedef enum { MG_COORDS_2D_DISPLAY_CENTER,
               MG_COORDS_2D_DISPLAY_TOP_LEFT,
               MG_COORDS_2D_DISPLAY_BOTTOM_LEFT } mg_coordinate_system;

typedef struct mg_color
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
} mg_color;

typedef enum {MG_JOINT_MITER = 0,
              MG_JOINT_BEVEL,
	          MG_JOINT_NONE } mg_joint_type;

typedef enum {MG_CAP_NONE = 0,
              MG_CAP_SQUARE } mg_cap_type;

typedef struct mg_font_extents
{
	f32 ascent;    // the extent above the baseline (by convention a positive value extends above the baseline)
	f32 descent;   // the extent below the baseline (by convention, positive value extends below the baseline)
	f32 leading;   // spacing between one row's descent and the next row's ascent
	f32 xHeight;   // height of the lower case letter 'x'
	f32 capHeight; // height of the upper case letter 'M'
	f32 width;     // maximum width of the font

} mg_font_extents;

typedef struct mg_text_extents
{
	f32 xBearing;
	f32 yBearing;
	f32 width;
	f32 height;
	f32 xAdvance;
	f32 yAdvance;

} mg_text_extents;

//------------------------------------------------------------------------------------------
//NOTE(martin): canvas lifetime and command streams
//------------------------------------------------------------------------------------------
mg_canvas mg_canvas_create(mg_surface surface, mp_rect viewPort);
void mg_canvas_destroy(mg_canvas context);
void mg_canvas_viewport(mg_canvas, mp_rect viewPort);
void mg_canvas_flush(mg_canvas context);

mg_stream mg_stream_create(mg_canvas context);
mg_stream mg_stream_swap(mg_canvas context, mg_stream stream);
void mg_stream_append(mg_canvas context, mg_stream stream);

//------------------------------------------------------------------------------------------
//NOTE(martin): fonts management
//------------------------------------------------------------------------------------------
mg_font mg_font_nil();
mg_font mg_font_create_from_memory(u32 size, byte* buffer, u32 rangeCount, unicode_range* ranges);
void mg_font_destroy(mg_font font);

//NOTE(martin): the following int valued functions return -1 if font is invalid or codepoint is not present in font//
//TODO(martin): add enum error codes

mg_font_extents mg_font_get_extents(mg_font font);
mg_font_extents mg_font_get_scaled_extents(mg_font font, f32 emSize);
f32 mg_font_get_scale_for_em_pixels(mg_font font, f32 emSize);

//NOTE(martin): if you need to process more than one codepoint, first convert your codepoints to glyph indices, then use the
//              glyph index versions of the functions, which can take an array of glyph indices.

str32 mg_font_get_glyph_indices(mg_font font, str32 codePoints, str32 backing);
str32 mg_font_push_glyph_indices(mg_font font, mem_arena* arena, str32 codePoints);
u32 mg_font_get_glyph_index(mg_font font, utf32 codePoint);

int mg_font_get_codepoint_extents(mg_font font, utf32 codePoint, mg_text_extents* outExtents);

int mg_font_get_glyph_extents(mg_font font, str32 glyphIndices, mg_text_extents* outExtents);

mp_rect mg_text_bounding_box_utf32(mg_font font, f32 fontSize, str32 text);
mp_rect mg_text_bounding_box(mg_font font, f32 fontSize, str8 text);

//------------------------------------------------------------------------------------------
//NOTE(martin): matrix settings
//------------------------------------------------------------------------------------------
void mg_matrix_push(mg_canvas context, mg_mat2x3 matrix);
void mg_matrix_pop(mg_canvas context);

//------------------------------------------------------------------------------------------
//NOTE(martin): clipping
//------------------------------------------------------------------------------------------
void mg_clip_push(mg_canvas context, f32 x, f32 y, f32 w, f32 h);
void mg_clip_pop(mg_canvas context);

//------------------------------------------------------------------------------------------
//NOTE(martin): graphics attributes setting/getting
//------------------------------------------------------------------------------------------
void mg_set_clear_color(mg_canvas context, mg_color color);
void mg_set_clear_color_rgba(mg_canvas context, f32 r, f32 g, f32 b, f32 a);
void mg_set_color(mg_canvas context, mg_color color);
void mg_set_color_rgba(mg_canvas context, f32 r, f32 g, f32 b, f32 a);
void mg_set_width(mg_canvas context, f32 width);
void mg_set_tolerance(mg_canvas context, f32 tolerance);
void mg_set_joint(mg_canvas context, mg_joint_type joint);
void mg_set_max_joint_excursion(mg_canvas context, f32 maxJointExcursion);
void mg_set_cap(mg_canvas context, mg_cap_type cap);
void mg_set_font(mg_canvas context, mg_font font);
void mg_set_font_size(mg_canvas context, f32 size);
void mg_set_text_flip(mg_canvas context, bool flip);

mg_color mg_get_clear_color(mg_canvas context);
mg_color mg_get_color(mg_canvas context);
f32 mg_get_width(mg_canvas context);
f32 mg_get_tolerance(mg_canvas context);
mg_joint_type mg_get_joint(mg_canvas context);
f32 mg_get_max_joint_excursion(mg_canvas context);
mg_cap_type mg_get_cap(mg_canvas context);
mg_font mg_get_font(mg_canvas context);
f32 mg_get_font_size(mg_canvas context);
bool mg_get_text_flip(mg_canvas context);
//------------------------------------------------------------------------------------------
//NOTE(martin): path construction
//------------------------------------------------------------------------------------------
vec2 mg_get_position(mg_canvas context);
void mg_move_to(mg_canvas context, f32 x, f32 y);
void mg_line_to(mg_canvas context, f32 x, f32 y);
void mg_quadratic_to(mg_canvas context, f32 x1, f32 y1, f32 x2, f32 y2);
void mg_cubic_to(mg_canvas context, f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3);
void mg_close_path(mg_canvas context);

mp_rect mg_glyph_outlines(mg_canvas context, str32 glyphIndices);
void mg_codepoints_outlines(mg_canvas context, str32 string);
void mg_text_outlines(mg_canvas context, str8 string);

//------------------------------------------------------------------------------------------
//NOTE(martin): clear/fill/stroke
//------------------------------------------------------------------------------------------
void mg_clear(mg_canvas context);
void mg_fill(mg_canvas context);
void mg_stroke(mg_canvas context);

//------------------------------------------------------------------------------------------
//NOTE(martin): 'fast' shapes primitives
//------------------------------------------------------------------------------------------
void mg_rectangle_fill(mg_canvas context, f32 x, f32 y, f32 w, f32 h);
void mg_rectangle_stroke(mg_canvas context, f32 x, f32 y, f32 w, f32 h);
void mg_rounded_rectangle_fill(mg_canvas context, f32 x, f32 y, f32 w, f32 h, f32 r);
void mg_rounded_rectangle_stroke(mg_canvas context, f32 x, f32 y, f32 w, f32 h, f32 r);
void mg_ellipse_fill(mg_canvas context, f32 x, f32 y, f32 rx, f32 ry);
void mg_ellipse_stroke(mg_canvas context, f32 x, f32 y, f32 rx, f32 ry);
void mg_circle_fill(mg_canvas context, f32 x, f32 y, f32 r);
void mg_circle_stroke(mg_canvas context, f32 x, f32 y, f32 r);
void mg_arc(mg_canvas handle, f32 x, f32 y, f32 r, f32 arcAngle, f32 startAngle);

//------------------------------------------------------------------------------------------
//NOTE(martin): images
//------------------------------------------------------------------------------------------
typedef struct mg_image { u64 h; } mg_image;

mg_image mg_image_nil();
bool mg_image_equal(mg_image a, mg_image b);

mg_image mg_image_create_from_rgba8(mg_canvas canvas, u32 width, u32 height, u8* pixels);
mg_image mg_image_create_from_data(mg_canvas canvas, str8 data, bool flip);
mg_image mg_image_create_from_file(mg_canvas canvas, str8 path, bool flip);

void mg_image_drestroy(mg_canvas canvas, mg_image image);

vec2 mg_image_size(mg_canvas canvas, mg_image image);
void mg_image_draw(mg_canvas canvas, mg_image image, mp_rect rect);
void mg_rounded_image_draw(mg_canvas handle, mg_image image, mp_rect rect, f32 roundness);

#ifdef __cplusplus
} // extern "C"
#endif
#endif //__GRAPHICS_H_
