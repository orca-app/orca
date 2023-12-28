/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "app/osx_app.m"
#include "graphics/graphics_common.c"
#include "graphics/graphics_surface.c"

#if OC_COMPILE_METAL
    #include "graphics/mtl_surface.m"
#endif

#if OC_COMPILE_CANVAS
    #include "graphics/mtl_renderer.m"
#endif

#if OC_COMPILE_GLES
    #include "graphics/gl_loader.c"
    #include "graphics/egl_surface.c"
#endif
