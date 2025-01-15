/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "surface.h"

#ifdef __OBJC__
    #import <Metal/Metal.h>
#endif

oc_surface oc_mtl_surface_create_for_window(oc_window window);
void* oc_mtl_surface_layer(oc_surface surface);
