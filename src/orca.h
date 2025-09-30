/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "util/algebra.h"
#include "util/debug.h"
#include "util/hash.h"
#include "util/lists.h"
#include "util/macros.h"
#include "util/memory.h"
#include "util/strings.h"
#include "util/typedefs.h"
#include "util/utf8.h"
#include "util/wrapped_types.h"

#include "platform/platform.h"
#include "platform/platform_clock.h"
#include "platform/io.h"
#include "platform/platform_path.h"

#if !defined(OC_PLATFORM_ORCA) || !(OC_PLATFORM_ORCA)
    #include "platform/platform_thread.h"
#endif

#if defined(OC_NO_APP_LAYER)
    #if OC_PLATFORM_ORCA
        #error "OC_NO_APP_LAYER can't be used with OC_PLATFORM_ORCA"
    #endif
#else

    #include "app/app.h"
    #include "platform/platform_io_dialog.h"

    //----------------------------------------------------------------
    // graphics
    //----------------------------------------------------------------
    #include "graphics/graphics.h"
    #include "graphics/backends.h"

    #if OC_GRAPHICS_ENABLE_METAL
        #include "graphics/mtl_surface.h"
    #endif

    #if OC_GRAPHICS_ENABLE_GL
        #include "graphics/gl_surface.h"
    #endif

    #if OC_GRAPHICS_ENABLE_GLES
        #include "graphics/gles_surface.h"
    #endif

    #if OC_PLATFORM_ORCA
        #include "graphics/orca_gl31.h"
    #else
        #if OC_GRAPHICS_INCLUDE_GL_API
            #include "graphics/gl_api.h"
        #endif
    #endif

    #if OC_GRAPHICS_ENABLE_WEBGPU
        #include "graphics/wgpu_surface.h"
    #endif

    //----------------------------------------------------------------
    // UI
    //----------------------------------------------------------------
    #include "ui/input_state.h"
    #include "ui/ui.h"
    #include "ui/widgets.h"

#endif // defined(OC_NO_APP_LAYER)
