/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "app/osx_app.m"
#include "graphics/graphics_common.c"
#include "graphics/graphics_text_native.c"
#include "graphics/canvas_renderer.c"
#include "graphics/surface.c"
#include "graphics/osx_surface.m"

#include "graphics/backends.h"

#if OC_GRAPHICS_ENABLE_METAL
    #include "graphics/mtl_surface.m"
#endif

#if OC_GRAPHICS_ENABLE_GLES
    #include "graphics/gl_loader.c"
    #include "graphics/gles_surface.c"
#endif

#if OC_GRAPHICS_ENABLE_WEBGPU
    #include "graphics/wgpu_surface_osx.m"
#endif

#if OC_GRAPHICS_ENABLE_CANVAS
    #include "graphics/wgpu_renderer.c"
#endif
