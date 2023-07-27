/************************************************************//**
*
*	@file: graphics_surface.h
*	@author: Martin Fouilleul
*	@date: 26/04/2023
*
*****************************************************************/
#ifndef __GRAPHICS_SURFACE_H_
#define __GRAPHICS_SURFACE_H_

#include"graphics_common.h"
#include"mp_app_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------
// surface interface
//---------------------------------------------------------------
typedef struct mg_surface_data mg_surface_data;
typedef struct mg_canvas_backend mg_canvas_backend;

typedef void (*mg_surface_destroy_proc)(mg_surface_data* surface);
typedef void (*mg_surface_prepare_proc)(mg_surface_data* surface);
typedef void (*mg_surface_deselect_proc)(mg_surface_data* surface);
typedef void (*mg_surface_present_proc)(mg_surface_data* surface);
typedef void (*mg_surface_swap_interval_proc)(mg_surface_data* surface, int swap);
typedef vec2 (*mg_surface_get_size_proc)(mg_surface_data* surface);
typedef vec2 (*mg_surface_contents_scaling_proc)(mg_surface_data* surface);
typedef bool (*mg_surface_get_hidden_proc)(mg_surface_data* surface);
typedef void (*mg_surface_set_hidden_proc)(mg_surface_data* surface, bool hidden);
typedef void* (*mg_surface_native_layer_proc)(mg_surface_data* surface);
typedef mg_surface_id (*mg_surface_remote_id_proc)(mg_surface_data* surface);
typedef void (*mg_surface_host_connect_proc)(mg_surface_data* surface, mg_surface_id remoteId);

typedef struct mg_surface_data
{
	mg_surface_api api;
	mp_layer layer;

	mg_surface_destroy_proc destroy;
	mg_surface_prepare_proc prepare;
	mg_surface_present_proc present;
	mg_surface_deselect_proc deselect;
	mg_surface_swap_interval_proc swapInterval;
	mg_surface_get_size_proc getSize;
	mg_surface_contents_scaling_proc contentsScaling;
	mg_surface_get_hidden_proc getHidden;
	mg_surface_set_hidden_proc setHidden;
	mg_surface_native_layer_proc nativeLayer;
	mg_surface_remote_id_proc remoteID;
	mg_surface_host_connect_proc hostConnect;

	mg_canvas_backend* backend;

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
	mg_surface surface;
	vec2 size;

} mg_image_data;

typedef void (*mg_canvas_backend_destroy_proc)(mg_canvas_backend* backend);

typedef mg_image_data* (*mg_canvas_backend_image_create_proc)(mg_canvas_backend* backend, vec2 size);
typedef void (*mg_canvas_backend_image_destroy_proc)(mg_canvas_backend* backend, mg_image_data* image);
typedef void (*mg_canvas_backend_image_upload_region_proc)(mg_canvas_backend* backend,
                                                           mg_image_data* image,
                                                           mp_rect region,
                                                           u8* pixels);

typedef void (*mg_canvas_backend_render_proc)(mg_canvas_backend* backend,
                                              mg_color clearColor,
                                              u32 primitiveCount,
                                              mg_primitive* primitives,
                                              u32 eltCount,
                                              mg_path_elt* pathElements);

typedef struct mg_canvas_backend
{
	mg_canvas_backend_destroy_proc destroy;

	mg_canvas_backend_image_create_proc imageCreate;
	mg_canvas_backend_image_destroy_proc imageDestroy;
	mg_canvas_backend_image_upload_region_proc imageUploadRegion;

	mg_canvas_backend_render_proc render;

} mg_canvas_backend;

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__GRAPHICS_SURFACE_H_
