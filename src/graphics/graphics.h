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

#include"util/typedefs.h"
#include"platform/platform.h"
#include"app/mp_app.h"

//------------------------------------------------------------------------------------------
//NOTE(martin): backends selection
//------------------------------------------------------------------------------------------

typedef enum {
	MG_NONE,
	MG_METAL,
	MG_GL,
	MG_GLES,
	MG_CANVAS,
	MG_HOST } mg_surface_api;

//NOTE: these macros are used to select which backend to include when building milepost
//      they can be overridden by passing them to the compiler command line
#if PLATFORM_MACOS
	#ifndef MG_COMPILE_METAL
		#define MG_COMPILE_METAL 1
	#endif

	#ifndef MG_COMPILE_GLES
		#define MG_COMPILE_GLES 1
	#endif

	#ifndef MG_COMPILE_CANVAS
		#if !MG_COMPILE_METAL
			#error "Canvas surface requires a Metal backend on macOS. Make sure you define MG_COMPILE_METAL to 1."
		#endif
		#define MG_COMPILE_CANVAS 1
	#endif

	#define MG_COMPILE_GL 0

#elif PLATFORM_WINDOWS
	#ifndef MG_COMPILE_GL
		#define MG_COMPILE_GL 1
	#endif

	#ifndef MG_COMPILE_GLES
		#define MG_COMPILE_GLES 1
	#endif

	#ifndef MG_COMPILE_CANVAS
		#if !MG_COMPILE_GL
			#error "Canvas surface requires an OpenGL backend on Windows. Make sure you define MG_COMPILE_GL to 1."
		#endif
		#define MG_COMPILE_CANVAS 1
	#endif

#elif PLATFORM_LINUX
	#ifndef MG_COMPILE_GL
		#define MG_COMPILE_GL 1
	#endif

	#ifndef MG_COMPILE_CANVAS
		#if !MG_COMPILE_GL
			#error "Canvas surface requires an OpenGL backend on Linux. Make sure you define MG_COMPILE_GL to 1."
		#endif
		#define MG_COMPILE_CANVAS 1
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

MP_API bool mg_is_surface_api_available(mg_surface_api api);

//------------------------------------------------------------------------------------------
//NOTE(martin): graphics surface
//------------------------------------------------------------------------------------------
typedef struct mg_surface { u64 h; } mg_surface;

MP_API mg_surface mg_surface_nil(void);
MP_API bool mg_surface_is_nil(mg_surface surface);

MP_API mg_surface mg_surface_create_for_window(mp_window window, mg_surface_api api);
MP_API void mg_surface_destroy(mg_surface surface);

MP_API void mg_surface_prepare(mg_surface surface);
MP_API void mg_surface_present(mg_surface surface);
MP_API void mg_surface_deselect(void);

MP_API void mg_surface_swap_interval(mg_surface surface, int swap);
MP_API vec2 mg_surface_get_size(mg_surface surface);
MP_API vec2 mg_surface_contents_scaling(mg_surface surface);
MP_API bool mg_surface_get_hidden(mg_surface surface);
MP_API void mg_surface_set_hidden(mg_surface surface, bool hidden);

//NOTE(martin): surface sharing
typedef u64 mg_surface_id;

MP_API mg_surface mg_surface_create_remote(u32 width, u32 height, mg_surface_api api);
MP_API mg_surface mg_surface_create_host(mp_window window);
MP_API mg_surface_id mg_surface_remote_id(mg_surface surface);
MP_API void mg_surface_host_connect(mg_surface surface, mg_surface_id remoteId);

//------------------------------------------------------------------------------------------
//NOTE(martin): graphics canvas structs
//------------------------------------------------------------------------------------------
typedef struct mg_canvas { u64 h; } mg_canvas;
typedef struct mg_font { u64 h; } mg_font;
typedef struct mg_image { u64 h; } mg_image;

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
MP_API mg_canvas mg_canvas_nil(void);
MP_API bool mg_canvas_is_nil(mg_canvas canvas);

MP_API mg_canvas mg_canvas_create(void);
MP_API void mg_canvas_destroy(mg_canvas canvas);
MP_API mg_canvas mg_canvas_set_current(mg_canvas canvas);
MP_API void mg_render(mg_surface surface, mg_canvas canvas);

//------------------------------------------------------------------------------------------
//NOTE(martin): fonts
//------------------------------------------------------------------------------------------
MP_API mg_font mg_font_nil(void);
MP_API mg_font mg_font_create_from_memory(u32 size, byte* buffer, u32 rangeCount, unicode_range* ranges);
MP_API void mg_font_destroy(mg_font font);

//NOTE(martin): the following int valued functions return -1 if font is invalid or codepoint is not present in font//
//TODO(martin): add enum error codes

MP_API mg_font_extents mg_font_get_extents(mg_font font);
MP_API mg_font_extents mg_font_get_scaled_extents(mg_font font, f32 emSize);
MP_API f32 mg_font_get_scale_for_em_pixels(mg_font font, f32 emSize);

//NOTE(martin): if you need to process more than one codepoint, first convert your codepoints to glyph indices, then use the
//              glyph index versions of the functions, which can take an array of glyph indices.

MP_API str32 mg_font_get_glyph_indices(mg_font font, str32 codePoints, str32 backing);
MP_API str32 mg_font_push_glyph_indices(mg_font font, mem_arena* arena, str32 codePoints);
MP_API u32 mg_font_get_glyph_index(mg_font font, utf32 codePoint);

MP_API int mg_font_get_codepoint_extents(mg_font font, utf32 codePoint, mg_text_extents* outExtents);

MP_API int mg_font_get_glyph_extents(mg_font font, str32 glyphIndices, mg_text_extents* outExtents);

MP_API mp_rect mg_text_bounding_box_utf32(mg_font font, f32 fontSize, str32 text);
MP_API mp_rect mg_text_bounding_box(mg_font font, f32 fontSize, str8 text);

//------------------------------------------------------------------------------------------
//NOTE(martin): images
//------------------------------------------------------------------------------------------
MP_API mg_image mg_image_nil(void);
MP_API bool mg_image_is_nil(mg_image a);

MP_API mg_image mg_image_create(mg_surface surface, u32 width, u32 height);
MP_API mg_image mg_image_create_from_rgba8(mg_surface surface, u32 width, u32 height, u8* pixels);
MP_API mg_image mg_image_create_from_data(mg_surface surface, str8 data, bool flip);
MP_API mg_image mg_image_create_from_file(mg_surface surface, str8 path, bool flip);

MP_API void mg_image_destroy(mg_image image);

MP_API void mg_image_upload_region_rgba8(mg_image image, mp_rect region, u8* pixels);
MP_API vec2 mg_image_size(mg_image image);

//------------------------------------------------------------------------------------------
//NOTE(martin): atlasing
//------------------------------------------------------------------------------------------

//NOTE: rectangle allocator
typedef struct mg_rect_atlas mg_rect_atlas;

MP_API mg_rect_atlas* mg_rect_atlas_create(mem_arena* arena, i32 width, i32 height);
MP_API mp_rect mg_rect_atlas_alloc(mg_rect_atlas* atlas, i32 width, i32 height);
MP_API void mg_rect_atlas_recycle(mg_rect_atlas* atlas, mp_rect rect);

//NOTE: image atlas helpers
typedef struct mg_image_region
{
	mg_image image;
	mp_rect rect;
} mg_image_region;

MP_API mg_image_region mg_image_atlas_alloc_from_rgba8(mg_rect_atlas* atlas, mg_image backingImage, u32 width, u32 height, u8* pixels);
MP_API mg_image_region mg_image_atlas_alloc_from_data(mg_rect_atlas* atlas, mg_image backingImage, str8 data, bool flip);
MP_API mg_image_region mg_image_atlas_alloc_from_file(mg_rect_atlas* atlas, mg_image backingImage, str8 path, bool flip);
MP_API void mg_image_atlas_recycle(mg_rect_atlas* atlas, mg_image_region imageRgn);

//------------------------------------------------------------------------------------------
//NOTE(martin): transform, viewport and clipping
//------------------------------------------------------------------------------------------
MP_API void mg_viewport(mp_rect viewPort);

MP_API void mg_matrix_push(mg_mat2x3 matrix);
MP_API void mg_matrix_pop(void);

MP_API void mg_clip_push(f32 x, f32 y, f32 w, f32 h);
MP_API void mg_clip_pop(void);

//------------------------------------------------------------------------------------------
//NOTE(martin): graphics attributes setting/getting
//------------------------------------------------------------------------------------------
MP_API void mg_set_color(mg_color color);
MP_API void mg_set_color_rgba(f32 r, f32 g, f32 b, f32 a);
MP_API void mg_set_width(f32 width);
MP_API void mg_set_tolerance(f32 tolerance);
MP_API void mg_set_joint(mg_joint_type joint);
MP_API void mg_set_max_joint_excursion(f32 maxJointExcursion);
MP_API void mg_set_cap(mg_cap_type cap);
MP_API void mg_set_font(mg_font font);
MP_API void mg_set_font_size(f32 size);
MP_API void mg_set_text_flip(bool flip);
MP_API void mg_set_image(mg_image image);
MP_API void mg_set_image_source_region(mp_rect region);

MP_API mg_color mg_get_color(void);
MP_API f32 mg_get_width(void);
MP_API f32 mg_get_tolerance(void);
MP_API mg_joint_type mg_get_joint(void);
MP_API f32 mg_get_max_joint_excursion(void);
MP_API mg_cap_type mg_get_cap(void);
MP_API mg_font mg_get_font(void);
MP_API f32 mg_get_font_size(void);
MP_API bool mg_get_text_flip(void);

//------------------------------------------------------------------------------------------
//NOTE(martin): path construction
//------------------------------------------------------------------------------------------
MP_API vec2 mg_get_position(void);
MP_API void mg_move_to(f32 x, f32 y);
MP_API void mg_line_to(f32 x, f32 y);
MP_API void mg_quadratic_to(f32 x1, f32 y1, f32 x2, f32 y2);
MP_API void mg_cubic_to(f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3);
MP_API void mg_close_path(void);

MP_API mp_rect mg_glyph_outlines(str32 glyphIndices);
MP_API void mg_codepoints_outlines(str32 string);
MP_API void mg_text_outlines(str8 string);

//------------------------------------------------------------------------------------------
//NOTE(martin): clear/fill/stroke
//------------------------------------------------------------------------------------------
MP_API void mg_clear(void);
MP_API void mg_fill(void);
MP_API void mg_stroke(void);

//------------------------------------------------------------------------------------------
//NOTE(martin): 'fast' shapes primitives
//------------------------------------------------------------------------------------------
MP_API void mg_rectangle_fill(f32 x, f32 y, f32 w, f32 h);
MP_API void mg_rectangle_stroke(f32 x, f32 y, f32 w, f32 h);
MP_API void mg_rounded_rectangle_fill(f32 x, f32 y, f32 w, f32 h, f32 r);
MP_API void mg_rounded_rectangle_stroke(f32 x, f32 y, f32 w, f32 h, f32 r);
MP_API void mg_ellipse_fill(f32 x, f32 y, f32 rx, f32 ry);
MP_API void mg_ellipse_stroke(f32 x, f32 y, f32 rx, f32 ry);
MP_API void mg_circle_fill(f32 x, f32 y, f32 r);
MP_API void mg_circle_stroke(f32 x, f32 y, f32 r);
MP_API void mg_arc(f32 x, f32 y, f32 r, f32 arcAngle, f32 startAngle);

//NOTE: image helpers
MP_API void mg_image_draw(mg_image image, mp_rect rect);
MP_API void mg_image_draw_region(mg_image image, mp_rect srcRegion, mp_rect dstRegion);

#endif //__GRAPHICS_H_
