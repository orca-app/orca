/************************************************************//**
*
*	@file: graphics_internal.h
*	@author: Martin Fouilleul
*	@date: 23/01/2023
*	@revision:
*
*****************************************************************/
#ifndef __GRAPHICS_INTERNAL_H_
#define __GRAPHICS_INTERNAL_H_

#include"graphics.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mg_surface_data mg_surface_data;

typedef void (*mg_surface_destroy_proc)(mg_surface_data* surface);
typedef void (*mg_surface_prepare_proc)(mg_surface_data* surface);
typedef void (*mg_surface_present_proc)(mg_surface_data* surface);
typedef void (*mg_surface_swap_interval_proc)(mg_surface_data* surface, int swap);
typedef vec2 (*mg_surface_contents_scaling_proc)(mg_surface_data* surface);
typedef mp_rect (*mg_surface_get_frame_proc)(mg_surface_data* surface);
typedef void (*mg_surface_set_frame_proc)(mg_surface_data* surface, mp_rect frame);
typedef bool (*mg_surface_get_hidden_proc)(mg_surface_data* surface);
typedef void (*mg_surface_set_hidden_proc)(mg_surface_data* surface, bool hidden);

typedef struct mg_surface_data
{
	mg_backend_id backend;

	mg_surface_destroy_proc destroy;
	mg_surface_prepare_proc prepare;
	mg_surface_present_proc present;
	mg_surface_swap_interval_proc swapInterval;
	mg_surface_contents_scaling_proc contentsScaling;
	mg_surface_get_frame_proc getFrame;
	mg_surface_set_frame_proc setFrame;
	mg_surface_get_hidden_proc getHidden;
	mg_surface_set_hidden_proc setHidden;

} mg_surface_data;

mg_surface mg_surface_alloc_handle(mg_surface_data* surface);
mg_surface_data* mg_surface_data_from_handle(mg_surface handle);

//---------------------------------------------------------------
// graphics structs
//---------------------------------------------------------------
typedef enum { MG_PATH_MOVE,
               MG_PATH_LINE,
	           MG_PATH_QUADRATIC,
	           MG_PATH_CUBIC } mg_path_elt_type;

typedef struct mg_path_elt
{
	mg_path_elt_type type;
	vec2 p[3];

} mg_path_elt;

typedef struct mg_path_descriptor
{
	u32 startIndex;
	u32 count;
	vec2 startPoint;

} mg_path_descriptor;

typedef struct mg_attributes
{
	f32 width;
	f32 tolerance;
	mg_color color;
	mg_joint_type joint;
	f32 maxJointExcursion;
	mg_cap_type cap;

	mg_font font;
	f32 fontSize;

	mg_image image;

	mp_rect clip;

} mg_attributes;

typedef struct mg_rounded_rect
{
	f32 x;
	f32 y;
	f32 w;
	f32 h;
	f32 r;
} mg_rounded_rect;

typedef enum { MG_CMD_CLEAR = 0,
	       MG_CMD_FILL,
	       MG_CMD_STROKE,
	       MG_CMD_RECT_FILL,
	       MG_CMD_RECT_STROKE,
	       MG_CMD_ROUND_RECT_FILL,
	       MG_CMD_ROUND_RECT_STROKE,
	       MG_CMD_ELLIPSE_FILL,
	       MG_CMD_ELLIPSE_STROKE,
	       MG_CMD_JUMP,
	       MG_CMD_MATRIX_PUSH,
	       MG_CMD_MATRIX_POP,
	       MG_CMD_CLIP_PUSH,
	       MG_CMD_CLIP_POP,
	       MG_CMD_IMAGE_DRAW,
	       MG_CMD_ROUNDED_IMAGE_DRAW,
	     } mg_primitive_cmd;

typedef struct mg_primitive
{
	mg_primitive_cmd cmd;
	mg_attributes attributes;

	union
	{
		mg_path_descriptor path;
		mp_rect rect;
		mg_rounded_rect roundedRect;
		utf32 codePoint;
		u32 jump;
		mg_mat2x3 matrix;
	};

} mg_primitive;

typedef struct mg_glyph_map_entry
{
	unicode_range range;
	u32 firstGlyphIndex;

} mg_glyph_map_entry;

typedef struct mg_glyph_data
{
	bool exists;
	utf32 codePoint;
	mg_path_descriptor pathDescriptor;
	mg_text_extents extents;
	//...

} mg_glyph_data;

enum
{
	MG_STREAM_MAX_COUNT = 128,
	MG_IMAGE_MAX_COUNT = 128
};

typedef struct mg_image_data
{
	list_elt listElt;
	u32 generation;

	mp_rect rect;

} mg_image_data;

enum
{
	MG_MATRIX_STACK_MAX_DEPTH = 64,
	MG_CLIP_STACK_MAX_DEPTH = 64,
	MG_MAX_PATH_ELEMENT_COUNT = 2<<20,
	MG_MAX_PRIMITIVE_COUNT = 8<<10
};

typedef struct mg_font_data
{
	list_elt freeListElt;
	u32 generation;

	u32 rangeCount;
	u32 glyphCount;
	u32 outlineCount;
	mg_glyph_map_entry* glyphMap;
	mg_glyph_data*      glyphs;
	mg_path_elt* outlines;

	f32 unitsPerEm;
	mg_font_extents extents;

} mg_font_data;

typedef struct mg_vertex_layout
{
	u32 maxVertexCount;
	u32 maxIndexCount;

	char* posBuffer;
	u32 posStride;

	char* cubicBuffer;
	u32 cubicStride;

	char* uvBuffer;
	u32 uvStride;

	char* colorBuffer;
	u32 colorStride;

	char* shapeIndexBuffer;
	u32 shapeIndexStride;

	char* clipBuffer;
	u32 clipStride;

	char* indexBuffer;
	u32 indexStride;

} mg_vertex_layout;

typedef struct mg_canvas_backend mg_canvas_backend;

typedef void (*mg_canvas_backend_destroy_proc)(mg_canvas_backend* backend);
typedef void (*mg_canvas_backend_begin_proc)(mg_canvas_backend* backend);
typedef void (*mg_canvas_backend_end_proc)(mg_canvas_backend* backend);
typedef void (*mg_canvas_backend_clear_proc)(mg_canvas_backend* backend, mg_color clearColor);
typedef void (*mg_canvas_backend_draw_batch_proc)(mg_canvas_backend* backend, u32 vertexCount, u32 shapeCount, u32 indexCount);
typedef void (*mg_canvas_backend_atlas_upload_proc)(mg_canvas_backend* backend, mp_rect rect, u8* bytes);

typedef struct mg_canvas_backend
{
	mg_vertex_layout vertexLayout;

	mg_canvas_backend_destroy_proc destroy;
	mg_canvas_backend_begin_proc begin;
	mg_canvas_backend_end_proc end;
	mg_canvas_backend_clear_proc clear;
	mg_canvas_backend_draw_batch_proc drawBatch;
	mg_canvas_backend_atlas_upload_proc atlasUpload;

} mg_canvas_backend;

typedef struct mg_canvas_data
{
	list_elt freeListElt;
	u32 generation;

	u64 frameCounter;

	mg_mat2x3 transform;
	mp_rect clip;
	mg_attributes attributes;
	bool textFlip;

	mg_path_elt pathElements[MG_MAX_PATH_ELEMENT_COUNT];
	mg_path_descriptor path;
	vec2 subPathStartPoint;
	vec2 subPathLastPoint;

	mg_mat2x3 matrixStack[MG_MATRIX_STACK_MAX_DEPTH];
	u32 matrixStackSize;

	mp_rect clipStack[MG_CLIP_STACK_MAX_DEPTH];
	u32 clipStackSize;

	u32 nextShapeIndex;
	u32 primitiveCount;
	mg_primitive primitives[MG_MAX_PRIMITIVE_COUNT];

	u32 vertexCount;
	u32 indexCount;

	mg_image_data images[MG_IMAGE_MAX_COUNT];
	u32 imageNextIndex;
	list_info imageFreeList;

	vec2 atlasPos;
	u32 atlasLineHeight;
	mg_image blankImage;

	mg_canvas_backend* backend;

} mg_canvas_data;

enum
{
	MG_ATLAS_SIZE = 8192,
};


#ifdef __cplusplus
} // extern "C"
#endif

#endif //__GRAPHICS_INTERNAL_H_
