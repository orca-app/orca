/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

//---------------------------------------------------------------
// platform implementations
//---------------------------------------------------------------
#include "platform/platform.h"

#if OC_PLATFORM_WINDOWS
    #include "platform/native_debug.c"
    #include "platform/win32.c"
#elif OC_PLATFORM_MACOS
    #include "platform/native_debug.c"
    #include "platform/unix_memory.c"
    #include "platform/osx_clock.c"
    #include "platform/osx_path.c"
    #include "platform/posix_io.c"
    #include "platform/posix_thread.c"
    #include "platform/osx_platform.c"

/*
	#include"platform/unix_rng.c"
	#include"platform/posix_socket.c"
	*/

#elif PLATFORM_LINUX
    #include "platform/native_debug.c"
    #include "platform/unix_base_memory.c"
    #include "platform/linux_clock.c"
    #include "platform/posix_io.c"
    #include "platform/platform_io_dialog.c"
    #include "platform/posix_thread.c"
/*
	#include"platform/unix_rng.c"
	#include"platform/posix_socket.c"
	*/
#elif OC_PLATFORM_ORCA
    #include "platform/orca_debug.c"
    #include "platform/orca_clock.c"
    #include "platform/orca_memory.c"
    #include "platform/platform_io_common.c"
    #include "platform/orca_io_stubs.c"
    #include "platform/orca_platform.c"
    #include "platform/platform_path.c"
#else
    #error "Unsupported platform"
#endif

//---------------------------------------------------------------
// utilities implementations
//---------------------------------------------------------------
#include "util/algebra.c"
#include "util/hash.c"
#include "util/memory.c"
#include "util/lists.c"
#include "util/ringbuffer.c"
#include "util/strings.c"
#include "util/utf8.c"
#include "util/wrapped_types.c"
#include "util/argparse.c"

#if !defined(OC_NO_APP_LAYER)
//---------------------------------------------------------------
// app/graphics layer
//---------------------------------------------------------------
    #if OC_PLATFORM_WINDOWS
        #include "platform/platform_io_dialog.c"
        #include "app/win32_app.c"
        #include "graphics/win32_vsync.c"
        #include "graphics/graphics_common.c"
        #include "graphics/canvas_renderer.c"
        #include "graphics/surface.c"
        #include "graphics/win32_surface.c"

        #include "graphics/backends.h"

        #if OC_GRAPHICS_ENABLE_GL || OC_GRAPHICS_ENABLE_GLES
            #include "graphics/gl_loader.c"
        #endif

        #if OC_GRAPHICS_ENABLE_GL
            #include "graphics/wgl_surface.c"
        #endif

        #if OC_GRAPHICS_ENABLE_GLES
            #include "graphics/gles_surface.c"
        #endif

        #if OC_GRAPHICS_ENABLE_WEBGPU
            #include "graphics/wgpu_surface_win32.c"
        #endif

        #if OC_GRAPHICS_ENABLE_CANVAS
            #include "graphics/wgpu_renderer.c"
        #endif

    #elif OC_PLATFORM_MACOS
        #include "platform/platform_io_dialog.c"
    //NOTE: macos application layer and graphics backends are defined in orca.m
    #elif OC_PLATFORM_ORCA
        #include "app/orca_app.c"
        #include "wasmbind/core_api_stubs.c"
        #include "graphics/graphics_common.c"
        #include "graphics/orca_surface_stubs.c"
    #else
        #error "Unsupported platform"
    #endif

    #include "ui/input_state.c"
    #include "ui/widgets.c"
    #include "ui/ui.c"

#endif // !defined(OC_NO_APP_LAYER)
