/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#ifndef __WGL_SURFACE_H_
#define __WGL_SURFACE_H_

#include "app/app.h"
#include "graphics.h"

ORCA_API oc_surface oc_gl_surface_create_for_window(oc_window window);

ORCA_API void oc_gl_surface_make_current(oc_surface surface);
ORCA_API void oc_gl_surface_swap_interval(oc_surface surface, int interval);
ORCA_API void oc_gl_surface_swap_buffers(oc_surface surface);

#endif // __WGL_SURFACE_H_
