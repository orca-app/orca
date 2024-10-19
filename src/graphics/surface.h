/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#ifndef __GRAPHICS_SURFACE_H_
#define __GRAPHICS_SURFACE_H_

#include "platform/platform.h"

#include "backends.h"

#if OC_PLATFORM_WINDOWS
    #include "win32_surface.h"
#elif OC_PLATFORM_MACOS
    #include "osx_surface.h"
#elif OC_PLATFORM_LINUX
    #include "linux_surface.h"
#else
    #error "unsupported platform"
#endif

#include "graphics.h"

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------
// surface interface
//---------------------------------------------------------------

typedef struct oc_surface_base oc_surface_base;

typedef void (*oc_surface_destroy_proc)(oc_surface_base* surface);
typedef oc_vec2 (*oc_surface_get_size_proc)(oc_surface_base* surface);
typedef oc_vec2 (*oc_surface_contents_scaling_proc)(oc_surface_base* surface);
typedef void (*oc_surface_bring_to_front_proc)(oc_surface_base* surface);
typedef void (*oc_surface_send_to_back_proc)(oc_surface_base* surface);
typedef bool (*oc_surface_get_hidden_proc)(oc_surface_base* surface);
typedef void (*oc_surface_set_hidden_proc)(oc_surface_base* surface, bool hidden);

typedef struct oc_surface_base
{
    oc_view view;
    oc_surface_api api;

    oc_surface_destroy_proc destroy;
    oc_surface_get_size_proc getSize;
    oc_surface_contents_scaling_proc contentsScaling;
    oc_surface_bring_to_front_proc bringToFront;
    oc_surface_send_to_back_proc sendToBack;
    oc_surface_get_hidden_proc getHidden;
    oc_surface_set_hidden_proc setHidden;

} oc_surface_base;

oc_surface oc_surface_handle_alloc(oc_surface_base* surface);
oc_surface_base* oc_surface_from_handle(oc_surface handle);

void oc_surface_base_init_for_window(oc_surface_base* surface, oc_window_data* window);
void oc_surface_base_cleanup(oc_surface_base* surface);
oc_vec2 oc_surface_base_get_size(oc_surface_base* surface);
oc_vec2 oc_surface_base_contents_scaling(oc_surface_base* surface);
void oc_surface_base_bring_to_front(oc_surface_base* surface);
void oc_surface_base_send_to_back(oc_surface_base* surface);
bool oc_surface_base_get_hidden(oc_surface_base* surface);
void oc_surface_base_set_hidden(oc_surface_base* surface, bool hidden);

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__GRAPHICS_SURFACE_H_
