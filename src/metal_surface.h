/************************************************************//**
*
*	@file: metal_surface.h
*	@author: Martin Fouilleul
*	@date: 25/12/2022
*	@revision:
*
*****************************************************************/
#ifndef __METAL_SURFACE_H_
#define __METAL_SURFACE_H_

#ifdef __OBJC__
	#import<Metal/Metal.h>
#endif

mg_surface mg_metal_surface_create_for_window(mp_window window);
void* mg_metal_surface_render_encoder(mg_surface surface);
void* mg_metal_surface_compute_encoder(mg_surface surface);

void* mg_metal_surface_layer(mg_surface surface);
void* mg_metal_surface_drawable(mg_surface surface);
void* mg_metal_surface_command_buffer(mg_surface surface);

#endif //__METAL_SURFACE_H_
