/************************************************************//**
*
*	@file: egl_surface.h
*	@author: Martin Fouilleul
*	@date: 28/01/2023
*	@revision:
*
*****************************************************************/
#ifndef __EGL_SURFACE_H_
#define __EGL_SURFACE_H_

#include"graphics_internal.h"
#include"mp_app.h"

mg_surface_data* mg_egl_surface_create_for_window(mp_window window);
mg_surface_data* mg_egl_surface_create_for_sharing(u32 width, u32 height);

#endif // __EGL_SURFACE_H_
