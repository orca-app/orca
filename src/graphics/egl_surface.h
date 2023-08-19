/************************************************************/ /**
*
*	@file: egl_surface.h
*	@author: Martin Fouilleul
*	@date: 28/01/2023
*	@revision:
*
*****************************************************************/
#ifndef __EGL_SURFACE_H_
#define __EGL_SURFACE_H_

#include "app/app.h"
#include "graphics_surface.h"

oc_surface_data* oc_egl_surface_create_for_window(oc_window window);
oc_surface_data* oc_egl_surface_create_remote(u32 width, u32 height);

#endif // __EGL_SURFACE_H_
