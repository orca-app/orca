/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "app/app.h"
#include "graphics.h"

#if OC_PLATFORM_ORCA
ORCA_API oc_surface oc_gles_surface_create(void);
#else
ORCA_API oc_surface oc_gles_surface_create_for_window(oc_window window);
#endif

ORCA_API void oc_gles_surface_make_current(oc_surface surface);
ORCA_API void oc_gles_surface_swap_interval(oc_surface surface, int interval);
ORCA_API void oc_gles_surface_swap_buffers(oc_surface surface);
