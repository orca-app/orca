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

//---------------------------------------------------------------
// surface interface
//---------------------------------------------------------------
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
// canvas backend interface
//---------------------------------------------------------------
typedef struct mg_texture_data
{
	list_elt listElt;
	u32 generation;
	vec2 size;

} mg_texture_data;

typedef struct mg_vertex_layout
{
	u32 maxVertexCount;
	u32 maxIndexCount;

	char* posBuffer;
	u32 posStride;

	char* cubicBuffer;
	u32 cubicStride;

	char* uvTransformBuffer;
	u32 uvTransformStride;

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
typedef void (*mg_canvas_backend_draw_batch_proc)(mg_canvas_backend* backend,
                                                  mg_texture_data* textureData,
                                                  u32 vertexCount,
                                                  u32 shapeCount,
                                                  u32 indexCount);


typedef mg_texture_data* (*mg_canvas_backend_texture_create_proc)(mg_canvas_backend* backend, vec2 size);
typedef void (*mg_canvas_backend_texture_destroy_proc)(mg_canvas_backend* backend, mg_texture_data* texture);
typedef void (*mg_canvas_backend_texture_upload_region_proc)(mg_canvas_backend* backend,
                                                           mg_texture_data* texture,
                                                           mp_rect region,
                                                           u8* pixels);

typedef struct mg_canvas_backend
{
	mg_vertex_layout vertexLayout;

	mg_canvas_backend_destroy_proc destroy;
	mg_canvas_backend_begin_proc begin;
	mg_canvas_backend_end_proc end;
	mg_canvas_backend_clear_proc clear;
	mg_canvas_backend_draw_batch_proc drawBatch;

	mg_canvas_backend_texture_create_proc textureCreate;
	mg_canvas_backend_texture_destroy_proc textureDestroy;
	mg_canvas_backend_texture_upload_region_proc textureUploadRegion;
} mg_canvas_backend;

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__GRAPHICS_INTERNAL_H_
