//*****************************************************************
//
//	$file: graphics.h $
//	$author: Martin Fouilleul $
//	$date: 23/36/2015 $
//	$revision: $
//
//*****************************************************************
#ifndef __GRAPHICS_H_
#define __GRAPHICS_H_

#include"typedefs.h"
#include"strings.h"

typedef struct g_mat2x3
{
	f32 m[6];
} g_mat2x3;

typedef struct g_color
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
} g_color;

typedef enum {G_JOINT_MITER = 0,
              G_JOINT_BEVEL,
	          G_JOINT_NONE } g_joint_type;

typedef enum {G_CAP_NONE = 0,
              G_CAP_SQUARE } g_cap_type;

typedef struct g_font { u64 h; } g_font;

typedef struct g_font_extents
{
	f32 ascent;    // the extent above the baseline (by convention a positive value extends above the baseline)
	f32 descent;   // the extent below the baseline (by convention, positive value extends below the baseline)
	f32 leading;   // spacing between one row's descent and the next row's ascent
	f32 xHeight;   // height of the lower case letter 'x'
	f32 capHeight; // height of the upper case letter 'M'
	f32 width;     // maximum width of the font

} g_font_extents;

typedef struct g_text_extents
{
	f32 xBearing;
	f32 yBearing;
	f32 width;
	f32 height;
	f32 xAdvance;
	f32 yAdvance;

} g_text_extents;

//------------------------------------------------------------------------------------------
//NOTE(martin): fonts
//------------------------------------------------------------------------------------------

g_font g_font_nil();
g_font g_font_create_default();
//g_font g_font_create_from_memory(u32 size, byte* buffer, u32 rangeCount, unicode_range* ranges);
void g_font_destroy(g_font font);

//NOTE(martin): the following int valued functions return -1 if font is invalid or codepoint is not present in font//
//TODO(martin): add enum error codes
/*
g_font_extents g_font_get_extents(g_font font);
g_font_extents g_font_get_scaled_extents(g_font font, f32 emSize);
f32 g_font_get_scale_for_em_pixels(g_font font, f32 emSize);

//NOTE(martin): if you need to process more than one codepoint, first convert your codepoints to glyph indices, then use the
//              glyph index versions of the functions, which can take an array of glyph indices.

str32 g_font_get_glyph_indices(g_font font, str32 codePoints, str32 backing);
str32 g_font_push_glyph_indices(g_font font, mem_arena* arena, str32 codePoints);
u32 g_font_get_glyph_index(g_font font, utf32 codePoint);

int g_font_get_codepoint_extents(g_font font, utf32 codePoint, g_text_extents* outExtents);

int g_font_get_glyph_extents(g_font font, str32 glyphIndices, g_text_extents* outExtents);

mp_rect g_text_bounding_box_utf32(g_font font, f32 fontSize, str32 text);
mp_rect g_text_bounding_box(g_font font, f32 fontSize, str8 text);
*/
//------------------------------------------------------------------------------------------
//NOTE(martin): images
//------------------------------------------------------------------------------------------
/*
g_image g_image_nil();
bool g_image_is_nil(g_image a);

g_image g_image_create(u32 width, u32 height);
g_image g_image_create_from_rgba8(u32 width, u32 height, u8* pixels);
g_image g_image_create_from_data(str8 data, bool flip);
g_image g_image_create_from_file(str8 path, bool flip);

void g_image_destroy(g_image image);

void g_image_upload_region_rgba8(g_image image, mp_rect region, u8* pixels);
vec2 g_image_size(g_image image);
*/
//------------------------------------------------------------------------------------------
//NOTE(martin): atlasing
//------------------------------------------------------------------------------------------
/*
//NOTE: rectangle allocator
typedef struct g_rect_atlas g_rect_atlas;

g_rect_atlas* g_rect_atlas_create(mem_arena* arena, i32 width, i32 height);
mp_rect g_rect_atlas_alloc(g_rect_atlas* atlas, i32 width, i32 height);
void g_rect_atlas_recycle(g_rect_atlas* atlas, mp_rect rect);

//NOTE: image atlas helpers
typedef struct g_image_region
{
	g_image image;
	mp_rect rect;
} g_image_region;

g_image_region g_image_atlas_alloc_from_rgba8(g_rect_atlas* atlas, g_image backingImage, u32 width, u32 height, u8* pixels);
g_image_region g_image_atlas_alloc_from_data(g_rect_atlas* atlas, g_image backingImage, str8 data, bool flip);
g_image_region g_image_atlas_alloc_from_file(g_rect_atlas* atlas, g_image backingImage, str8 path, bool flip);
void g_image_atlas_recycle(g_rect_atlas* atlas, g_image_region imageRgn);
*/
//------------------------------------------------------------------------------------------
//NOTE(martin): transform, viewport and clipping
//------------------------------------------------------------------------------------------
void g_viewport(mp_rect viewPort);

void g_matrix_push(g_mat2x3 matrix);
void g_matrix_pop();

void g_clip_push(f32 x, f32 y, f32 w, f32 h);
void g_clip_pop();

//------------------------------------------------------------------------------------------
//NOTE(martin): graphics attributes setting/getting
//------------------------------------------------------------------------------------------
void g_set_color(g_color color);
void g_set_color_rgba(f32 r, f32 g, f32 b, f32 a);
void g_set_width(f32 width);
void g_set_tolerance(f32 tolerance);
void g_set_joint(g_joint_type joint);
void g_set_max_joint_excursion(f32 maxJointExcursion);
void g_set_cap(g_cap_type cap);
void g_set_font(g_font font);
void g_set_font_size(f32 size);
void g_set_text_flip(bool flip);

/*
void g_set_image(g_image image);
void g_set_image_source_region(mp_rect region);
*/

g_color g_get_color();
f32 g_get_width();
f32 g_get_tolerance();
g_joint_type g_get_joint();
f32 g_get_max_joint_excursion();
g_cap_type g_get_cap();
g_font g_get_font();
f32 g_get_font_size();
bool g_get_text_flip();

//------------------------------------------------------------------------------------------
//NOTE(martin): path construction
//------------------------------------------------------------------------------------------
vec2 g_get_position();
void g_move_to(f32 x, f32 y);
void g_line_to(f32 x, f32 y);
void g_quadratic_to(f32 x1, f32 y1, f32 x2, f32 y2);
void g_cubic_to(f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3);
void g_close_path();

/*
mp_rect g_glyph_outlines(str32 glyphIndices);
void g_codepoints_outlines(str32 string);
*/

void g_text_outlines(str8 string);

//------------------------------------------------------------------------------------------
//NOTE(martin): clear/fill/stroke
//------------------------------------------------------------------------------------------
void g_clear();
void g_fill();
void g_stroke();

//------------------------------------------------------------------------------------------
//NOTE(martin): simple shapes helpers
//------------------------------------------------------------------------------------------
void g_rectangle_fill(f32 x, f32 y, f32 w, f32 h);
void g_rectangle_stroke(f32 x, f32 y, f32 w, f32 h);
void g_rounded_rectangle_fill(f32 x, f32 y, f32 w, f32 h, f32 r);
void g_rounded_rectangle_stroke(f32 x, f32 y, f32 w, f32 h, f32 r);
void g_ellipse_fill(f32 x, f32 y, f32 rx, f32 ry);
void g_ellipse_stroke(f32 x, f32 y, f32 rx, f32 ry);
void g_circle_fill(f32 x, f32 y, f32 r);
void g_circle_stroke(f32 x, f32 y, f32 r);
void g_arc(f32 x, f32 y, f32 r, f32 arcAngle, f32 startAngle);

//NOTE: image helpers
/*
void g_image_draw(g_image image, mp_rect rect);
void g_image_draw_region(g_image image, mp_rect srcRegion, mp_rect dstRegion);
*/
#endif //__GRAPHICS_H_
