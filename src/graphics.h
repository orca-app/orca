/************************************************************//**
*
*	@file: graphics.h
*	@author: Martin Fouilleul
*	@date: 23/01/2023
*	@revision:
*
*****************************************************************/
#ifndef __GRAPHICS_H_
#define __GRAPHICS_H_

#include"typedefs.h"
#include"platform.h"
#include"mp_app.h"

//------------------------------------------------------------------------------------------
//NOTE(martin): backends selection
//------------------------------------------------------------------------------------------

typedef enum {
	MG_BACKEND_NONE,
	MG_BACKEND_METAL,
	MG_BACKEND_GL,
	MG_BACKEND_GLES } mg_backend_id;

//NOTE: these macros are used to select which backend to include when building milepost
//      they can be overridden by passing them to the compiler command line
#if defined(OS_MACOS)
	#ifndef MG_COMPILE_BACKEND_METAL
		#define MG_COMPILE_BACKEND_METAL 1
	#endif

	#ifndef MG_COMPILE_BACKEND_GLES
		#define MG_COMPILE_BACKEND_GLES 1
	#endif

	#define MG_COMPILE_BACKEND_GL 0

	#if MG_COMPILE_BACKEND_METAL
		#define MG_BACKEND_DEFAULT MG_BACKEND_METAL
	#elif MG_COMPILE_BACKEND_GL
		#define MG_BACKEND_DEFAULT MG_BACKEND_GL
	#else
		#define MG_BACKEND_DEFAULT MG_BACKEND_NONE
	#endif

#elif defined(OS_WIN64)
	#ifndef MG_COMPILE_BACKEND_GL
		#define MG_COMPILE_BACKEND_GL 1
	#endif

	#ifndef MG_COMPILE_BACKEND_GLES
		#define MG_COMPILE_BACKEND_GLES 1
	#endif

	#if MG_COMPILE_BACKEND_GL
		#define MG_BACKEND_DEFAULT MG_BACKEND_GL
	#else
		#define MG_BACKEND_DEFAULT MG_BACKEND_NONE
	#endif

#elif defined(OS_LINUX)
	#ifndef MG_COMPILE_BACKEND_GL
		#define MG_COMPILE_BACKEND_GL 1
	#endif

#endif

//NOTE: these macros are used to select backend-specific APIs to include when using milepost
#ifdef MG_EXPOSE_SURFACE_METAL
	#include"mtl_surface.h"
#endif

#ifdef MG_EXPOSE_SURFACE_WGL
	#include"wgl_surface.h"
#endif

//TODO: expose nsgl surface when supported, expose egl surface, etc...

//TODO: add MG_INCLUDE_OPENGL/GLES/etc, once we know how we make different gl versions co-exist

bool mg_is_surface_backend_available(mg_backend_id id);
bool mg_is_canvas_backend_available(mg_backend_id id);

//------------------------------------------------------------------------------------------
//NOTE(martin): graphics surface
//------------------------------------------------------------------------------------------
typedef struct mg_surface { u64 h; } mg_surface;

mg_surface mg_surface_nil();
bool mg_surface_is_nil(mg_surface surface);

mg_surface mg_surface_create_for_window(mp_window window, mg_backend_id backend);
void mg_surface_destroy(mg_surface surface);
void mg_surface_prepare(mg_surface surface);
void mg_surface_present(mg_surface surface);
void mg_surface_swap_interval(mg_surface surface, int swap);
vec2 mg_surface_contents_scaling(mg_surface surface);
mp_rect mg_surface_get_frame(mg_surface surface);
void mg_surface_set_frame(mg_surface surface, mp_rect frame);
bool mg_surface_get_hidden(mg_surface surface);
void mg_surface_set_hidden(mg_surface surface, bool hidden);


//------------------------------------------------------------------------------------------
//NOTE(martin): graphics canvas structs
//------------------------------------------------------------------------------------------
typedef struct mg_canvas { u64 h; } mg_canvas;
typedef struct mg_stream { u64 h; } mg_stream;
typedef struct mg_font { u64 h; } mg_font;

typedef struct mg_mat2x3
{
	f32 m[6];
} mg_mat2x3;

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
//NOTE(martin): graphics canvas
//------------------------------------------------------------------------------------------
mg_canvas mg_canvas_nil();
bool mg_canvas_is_nil(mg_canvas canvas);

mg_canvas mg_canvas_create(mg_surface surface);
void mg_canvas_destroy(mg_canvas canvas);
mg_canvas mg_canvas_set_current(mg_canvas canvas);

void mg_flush();

//------------------------------------------------------------------------------------------
//NOTE(martin): transform, viewport and clipping
//------------------------------------------------------------------------------------------
void mg_viewport(mp_rect viewPort);

void mg_matrix_push(mg_mat2x3 matrix);
void mg_matrix_pop();

void mg_clip_push(f32 x, f32 y, f32 w, f32 h);
void mg_clip_pop();

//------------------------------------------------------------------------------------------
//NOTE(martin): graphics attributes setting/getting
//------------------------------------------------------------------------------------------
void mg_set_color(mg_color color);
void mg_set_color_rgba(f32 r, f32 g, f32 b, f32 a);
void mg_set_width(f32 width);
void mg_set_tolerance(f32 tolerance);
void mg_set_joint(mg_joint_type joint);
void mg_set_max_joint_excursion(f32 maxJointExcursion);
void mg_set_cap(mg_cap_type cap);
void mg_set_font(mg_font font);
void mg_set_font_size(f32 size);
void mg_set_text_flip(bool flip);

mg_color mg_get_color();
f32 mg_get_width();
f32 mg_get_tolerance();
mg_joint_type mg_get_joint();
f32 mg_get_max_joint_excursion();
mg_cap_type mg_get_cap();
mg_font mg_get_font();
f32 mg_get_font_size();
bool mg_get_text_flip();

//------------------------------------------------------------------------------------------
//NOTE(martin): path construction
//------------------------------------------------------------------------------------------
vec2 mg_get_position();
void mg_move_to(f32 x, f32 y);
void mg_line_to(f32 x, f32 y);
void mg_quadratic_to(f32 x1, f32 y1, f32 x2, f32 y2);
void mg_cubic_to(f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3);
void mg_close_path();

mp_rect mg_glyph_outlines(str32 glyphIndices);
void mg_codepoints_outlines(str32 string);
void mg_text_outlines(str8 string);

//------------------------------------------------------------------------------------------
//NOTE(martin): clear/fill/stroke
//------------------------------------------------------------------------------------------
void mg_clear();
void mg_fill();
void mg_stroke();

//------------------------------------------------------------------------------------------
//NOTE(martin): 'fast' shapes primitives
//------------------------------------------------------------------------------------------
void mg_rectangle_fill(f32 x, f32 y, f32 w, f32 h);
void mg_rectangle_stroke(f32 x, f32 y, f32 w, f32 h);
void mg_rounded_rectangle_fill(f32 x, f32 y, f32 w, f32 h, f32 r);
void mg_rounded_rectangle_stroke(f32 x, f32 y, f32 w, f32 h, f32 r);
void mg_ellipse_fill(f32 x, f32 y, f32 rx, f32 ry);
void mg_ellipse_stroke(f32 x, f32 y, f32 rx, f32 ry);
void mg_circle_fill(f32 x, f32 y, f32 r);
void mg_circle_stroke(f32 x, f32 y, f32 r);
void mg_arc(f32 x, f32 y, f32 r, f32 arcAngle, f32 startAngle);

//------------------------------------------------------------------------------------------
//NOTE(martin): fonts
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
//NOTE(martin): images
//------------------------------------------------------------------------------------------
typedef struct mg_image { u64 h; } mg_image;

mg_image mg_image_nil();
bool mg_image_equal(mg_image a, mg_image b);

mg_image mg_image_create_from_rgba8(u32 width, u32 height, u8* pixels);
mg_image mg_image_create_from_data(str8 data, bool flip);
mg_image mg_image_create_from_file(str8 path, bool flip);

void mg_image_drestroy(mg_image image);

vec2 mg_image_size(mg_image image);
void mg_image_draw(mg_image image, mp_rect rect);
void mg_rounded_image_draw(mg_image image, mp_rect rect, f32 roundness);

#endif //__GRAPHICS_H_
