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
#include"mp_app_internal.h"

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
typedef void* (*mg_surface_native_layer_proc)(mg_surface_data* surface);
typedef mg_surface_id (*mg_surface_remote_id_proc)(mg_surface_data* surface);
typedef void (*mg_surface_host_connect_proc)(mg_surface_data* surface, mg_surface_id remoteId);

typedef struct mg_surface_data
{
	mg_backend_id backend;
	mp_layer layer;

	mg_surface_destroy_proc destroy;
	mg_surface_prepare_proc prepare;
	mg_surface_present_proc present;
	mg_surface_swap_interval_proc swapInterval;
	mg_surface_contents_scaling_proc contentsScaling;
	mg_surface_get_frame_proc getFrame;
	mg_surface_set_frame_proc setFrame;
	mg_surface_get_hidden_proc getHidden;
	mg_surface_set_hidden_proc setHidden;
	mg_surface_native_layer_proc nativeLayer;
	mg_surface_remote_id_proc remoteID;
	mg_surface_host_connect_proc hostConnect;
} mg_surface_data;

mg_surface mg_surface_alloc_handle(mg_surface_data* surface);
mg_surface_data* mg_surface_data_from_handle(mg_surface handle);

void mg_surface_init_for_window(mg_surface_data* surface, mp_window_data* window);
void mg_surface_init_remote(mg_surface_data* surface, u32 width, u32 height);
void mg_surface_init_host(mg_surface_data* surface, mp_window_data* window);
void mg_surface_cleanup(mg_surface_data* surface);
void* mg_surface_native_layer(mg_surface surface);

//---------------------------------------------------------------
// canvas backend interface
//---------------------------------------------------------------
typedef struct mg_image_data
{
	list_elt listElt;
	u32 generation;
	vec2 size;

} mg_image_data;

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
typedef void (*mg_canvas_backend_begin_proc)(mg_canvas_backend* backend, mg_color clearColor);
typedef void (*mg_canvas_backend_end_proc)(mg_canvas_backend* backend);
typedef void (*mg_canvas_backend_draw_batch_proc)(mg_canvas_backend* backend,
                                                  mg_image_data* imageData,
                                                  u32 vertexCount,
                                                  u32 shapeCount,
                                                  u32 indexCount);


typedef mg_image_data* (*mg_canvas_backend_image_create_proc)(mg_canvas_backend* backend, vec2 size);
typedef void (*mg_canvas_backend_image_destroy_proc)(mg_canvas_backend* backend, mg_image_data* image);
typedef void (*mg_canvas_backend_image_upload_region_proc)(mg_canvas_backend* backend,
                                                           mg_image_data* image,
                                                           mp_rect region,
                                                           u8* pixels);

typedef struct mg_canvas_backend
{
	mg_vertex_layout vertexLayout;

	mg_canvas_backend_destroy_proc destroy;
	mg_canvas_backend_begin_proc begin;
	mg_canvas_backend_end_proc end;
	mg_canvas_backend_draw_batch_proc drawBatch;

	mg_canvas_backend_image_create_proc imageCreate;
	mg_canvas_backend_image_destroy_proc imageDestroy;
	mg_canvas_backend_image_upload_region_proc imageUploadRegion;
} mg_canvas_backend;

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__GRAPHICS_INTERNAL_H_
