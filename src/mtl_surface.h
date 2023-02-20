/************************************************************//**
*
*	@file: mtl_surface.h
*	@author: Martin Fouilleul
*	@date: 25/12/2022
*	@revision:
*
*****************************************************************/
#ifndef __MTL_SURFACE_H_
#define __MTL_SURFACE_H_

#include"graphics.h"

#ifdef __OBJC__
	#import<Metal/Metal.h>
#endif

mg_surface mg_mtl_surface_create_for_window(mp_window window);

void* mg_mtl_surface_render_encoder(mg_surface surface);
void* mg_mtl_surface_compute_encoder(mg_surface surface);
void* mg_mtl_surface_layer(mg_surface surface);
void* mg_mtl_surface_drawable(mg_surface surface);
void* mg_mtl_surface_command_buffer(mg_surface surface);

#endif //__MTL_SURFACE_H_
