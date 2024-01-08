/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#ifndef __MTL_SURFACE_H_
#define __MTL_SURFACE_H_

#include "surface.h"

#ifdef __OBJC__
    #import <Metal/Metal.h>
#endif

oc_surface oc_mtl_surface_create_for_window(oc_window window);
void* oc_mtl_surface_layer(oc_surface surface);

#endif //__MTL_SURFACE_H_
