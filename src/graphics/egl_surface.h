/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#ifndef __EGL_SURFACE_H_
#define __EGL_SURFACE_H_

#include "app/app.h"
#include "graphics_surface.h"

oc_surface_data* oc_egl_surface_create_for_window(oc_window window);
oc_surface_data* oc_egl_surface_create_remote(u32 width, u32 height);

#endif // __EGL_SURFACE_H_
