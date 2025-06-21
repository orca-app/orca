/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "platform/platform.h"

//NOTE: these macros are used to select which backend to compile when building the Orca platform layer
//      they can be overridden by passing them to the compiler command line

#if OC_PLATFORM_MACOS
    //NOTE: surface backends enabled by default on macOS are: GLES, WebGPU, and Canvas
    #ifndef OC_GRAPHICS_ENABLE_GLES
        #define OC_GRAPHICS_ENABLE_GLES 1
    #endif

    #ifndef OC_GRAPHICS_ENABLE_WEBGPU
        #define OC_GRAPHICS_ENABLE_WEBGPU 1
    #endif

    #ifndef OC_GRAPHICS_ENABLE_CANVAS
        #if !OC_GRAPHICS_ENABLE_WEBGPU
            #error "Canvas surface requires a WebGPU backend. Make sure you define OC_GRAPHICS_ENABLE_WEBGPU to 1."
        #endif
        #define OC_GRAPHICS_ENABLE_CANVAS 1
    #endif

    //NOTE: surface backends available but disabled by default on macOS are: Metal

    //NOTE: surface backends not supported on macOS are: OpenGL
    #if defined(OC_GRAPHICS_ENABLE_GL) && OC_GRAPHICS_ENABLE_GL
        #error "Desktop OpenGL backend is not supported on macOS. Make sure you let OC_GRAPHICS_ENABLE_GL undefined or set to 0"
    #endif

#elif OC_PLATFORM_WINDOWS
    //NOTE: surface backends enabled by default on Windows are: GLES, WebGPU
    #ifndef OC_GRAPHICS_ENABLE_GLES
        #define OC_GRAPHICS_ENABLE_GLES 1
    #endif

    #ifndef OC_GRAPHICS_ENABLE_WEBGPU
        #define OC_GRAPHICS_ENABLE_WEBGPU 1
    #endif

    #ifndef OC_GRAPHICS_ENABLE_CANVAS
        #if !OC_GRAPHICS_ENABLE_WEBGPU
            #error "Canvas surface requires a WebGPU backend. Make sure you define OC_GRAPHICS_ENABLE_WEBGPU to 1."
        #endif
        #define OC_GRAPHICS_ENABLE_CANVAS 1
    #endif

    //NOTE: surface backends available but disabled by default on Windows are: OpenGL

    //NOTE: surface backends not supported on Windows are: Metal
    #if defined(OC_GRAPHICS_ENABLE_METAL) && OC_GRAPHICS_ENABLE_METAL
        #error "Metal backend is not supported on Windows. Make sure you let OC_GRAPHICS_ENABLE_METAL undefined or set to 0"
    #endif

#elif OC_PLATFORM_LINUX
    #ifndef OC_GRAPHICS_ENABLE_GLES
        #define OC_GRAPHICS_ENABLE_GLES 1
    #endif

    #ifndef OC_GRAPHICS_ENABLE_WEBGPU
        #define OC_GRAPHICS_ENABLE_WEBGPU 1
    #endif

    #ifndef OC_GRAPHICS_ENABLE_CANVAS
        #if !OC_GRAPHICS_ENABLE_WEBGPU
            #error "Canvas surface requires a WebGPU backend. Make sure you define OC_GRAPHICS_ENABLE_WEBGPU to 1."
        #endif
        #define OC_GRAPHICS_ENABLE_CANVAS 1
    #endif

    //NOTE: surface backends available but disabled by default on Linux are: OpenGL

    //NOTE: surface backends not supported on Windows are: Metal
    #if defined(OC_GRAPHICS_ENABLE_METAL) && OC_GRAPHICS_ENABLE_METAL
        #error "Metal backend is not supported on Linux. Make sure you let OC_GRAPHICS_ENABLE_METAL undefined or set to 0"
    #endif

#elif OC_PLATFORM_ORCA
    //NOTE: surface backends enabled by default on Orca are: GLES, WebGPU
    #ifndef OC_GRAPHICS_ENABLE_GLES
        #define OC_GRAPHICS_ENABLE_GLES 1
    #endif

    #ifndef OC_GRAPHICS_ENABLE_WEBGPU
        #define OC_GRAPHICS_ENABLE_WEBGPU 1
    #endif

    //NOTE: surface backends not supported on Orca are: Metal, OpenGL
    #if defined(OC_GRAPHICS_ENABLE_METAL) && OC_GRAPHICS_ENABLE_METAL
        #error "Metal backend is not supported on Orca. Make sure you let OC_GRAPHICS_ENABLE_METAL undefined or set to 0"
    #endif

    #if defined(OC_GRAPHICS_ENABLE_GL) && OC_GRAPHICS_ENABLE_GL
        #error "Desktop OpenGL backend is not supported on Orca. Make sure you let OC_GRAPHICS_ENABLE_GL undefined or set to 0"
    #endif
#else
    #error "unsupported platform"
#endif

#ifndef OC_GRAPHICS_ENABLE_GLES
    #define OC_GRAPHICS_ENABLE_GLES 0
#endif
#ifndef OC_GRAPHICS_ENABLE_WEBGPU
    #define OC_GRAPHICS_ENABLE_WEBGPU 0
#endif
#ifndef OC_GRAPHICS_ENABLE_GL
    #define OC_GRAPHICS_ENABLE_GL 0
#endif
#ifndef OC_GRAPHICS_ENABLE_METAL
    #define OC_GRAPHICS_ENABLE_METAL 0
#endif
#ifndef OC_GRAPHICS_ENABLE_CANVAS
    #define OC_GRAPHICS_ENABLE_CANVAS 0
#endif

typedef enum
{
    OC_NONE,
    OC_SURFACE_METAL,
    OC_SURFACE_GL,
    OC_SURFACE_GLES,
    OC_SURFACE_WEBGPU,
    OC_SURFACE_CANVAS = 1 << 31,

} oc_surface_api;
