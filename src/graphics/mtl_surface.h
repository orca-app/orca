/************************************************************/ /**
*
*	@file: mtl_surface.h
*	@author: Martin Fouilleul
*	@date: 25/12/2022
*	@revision:
*
*****************************************************************/
#ifndef __MTL_SURFACE_H_
#define __MTL_SURFACE_H_

#include "graphics_surface.h"

#ifdef __OBJC__
    #import <Metal/Metal.h>
#endif

oc_surface_data* oc_mtl_surface_create_for_window(oc_window window);

void* oc_mtl_surface_render_encoder(oc_surface surface);
void* oc_mtl_surface_compute_encoder(oc_surface surface);
void* oc_mtl_surface_layer(oc_surface surface);
void* oc_mtl_surface_drawable(oc_surface surface);
void* oc_mtl_surface_command_buffer(oc_surface surface);

#endif //__MTL_SURFACE_H_
