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
#include"app/app_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------
// surface interface
//---------------------------------------------------------------
typedef struct oc_surface_data oc_surface_data;
typedef struct oc_canvas_backend oc_canvas_backend;

typedef void (*oc_surface_destroy_proc)(oc_surface_data* surface);
typedef void (*oc_surface_select_proc)(oc_surface_data* surface);
typedef void (*oc_surface_deselect_proc)(oc_surface_data* surface);
typedef void (*oc_surface_present_proc)(oc_surface_data* surface);
typedef void (*oc_surface_swap_interval_proc)(oc_surface_data* surface, int swap);
typedef oc_vec2 (*oc_surface_get_size_proc)(oc_surface_data* surface);
typedef oc_vec2 (*oc_surface_contents_scaling_proc)(oc_surface_data* surface);
typedef bool (*oc_surface_get_hidden_proc)(oc_surface_data* surface);
typedef void (*oc_surface_set_hidden_proc)(oc_surface_data* surface, bool hidden);
typedef void* (*oc_surface_native_layer_proc)(oc_surface_data* surface);
typedef oc_surface_id (*oc_surface_remote_id_proc)(oc_surface_data* surface);
typedef void (*oc_surface_host_connect_proc)(oc_surface_data* surface, oc_surface_id remoteId);

typedef struct oc_surface_data
{
	oc_surface_api api;
	oc_layer layer;

	oc_surface_destroy_proc destroy;
	oc_surface_select_proc prepare;
	oc_surface_present_proc present;
	oc_surface_deselect_proc deselect;
	oc_surface_swap_interval_proc swapInterval;
	oc_surface_get_size_proc getSize;
	oc_surface_contents_scaling_proc contentsScaling;
	oc_surface_get_hidden_proc getHidden;
	oc_surface_set_hidden_proc setHidden;
	oc_surface_native_layer_proc nativeLayer;
	oc_surface_remote_id_proc remoteID;
	oc_surface_host_connect_proc hostConnect;

	oc_canvas_backend* backend;

} oc_surface_data;

oc_surface oc_surface_handle_alloc(oc_surface_data* surface);
oc_surface_data* oc_surface_data_from_handle(oc_surface handle);

void oc_surface_init_for_window(oc_surface_data* surface, oc_window_data* window);
void oc_surface_init_remote(oc_surface_data* surface, u32 width, u32 height);
void oc_surface_init_host(oc_surface_data* surface, oc_window_data* window);
void oc_surface_cleanup(oc_surface_data* surface);
void* oc_surface_native_layer(oc_surface surface);

//---------------------------------------------------------------
// canvas backend interface
//---------------------------------------------------------------
typedef struct oc_image_data
{
	oc_list_elt listElt;
	u32 generation;
	oc_surface surface;
	oc_vec2 size;

} oc_image_data;

typedef void (*oc_canvas_backend_destroy_proc)(oc_canvas_backend* backend);

typedef oc_image_data* (*oc_canvas_backend_image_create_proc)(oc_canvas_backend* backend, oc_vec2 size);
typedef void (*oc_canvas_backend_image_destroy_proc)(oc_canvas_backend* backend, oc_image_data* image);
typedef void (*oc_canvas_backend_image_upload_region_proc)(oc_canvas_backend* backend,
                                                           oc_image_data* image,
                                                           oc_rect region,
                                                           u8* pixels);

typedef void (*oc_canvas_backend_render_proc)(oc_canvas_backend* backend,
                                              oc_color clearColor,
                                              u32 primitiveCount,
                                              oc_primitive* primitives,
                                              u32 eltCount,
                                              oc_path_elt* pathElements);

typedef struct oc_canvas_backend
{
	oc_canvas_backend_destroy_proc destroy;

	oc_canvas_backend_image_create_proc imageCreate;
	oc_canvas_backend_image_destroy_proc imageDestroy;
	oc_canvas_backend_image_upload_region_proc imageUploadRegion;

	oc_canvas_backend_render_proc render;

} oc_canvas_backend;

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__GRAPHICS_SURFACE_H_
