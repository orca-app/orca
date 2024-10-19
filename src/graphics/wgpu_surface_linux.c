/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "platform/platform_memory.h"
#include "surface.h"
#include "wgpu_surface.h"

typedef struct oc_wgpu_surface
{
    int TODO;
} oc_wgpu_surface;

void oc_wgpu_surface_destroy(oc_surface_base* base)
{
    oc_unimplemented();
}

oc_surface oc_wgpu_surface_create_for_window(WGPUInstance instance, oc_window window)
{
    oc_unimplemented();
    return (oc_surface){0};
}

WGPUTexture oc_wgpu_surface_get_current_texture(oc_surface handle, WGPUDevice device)
{
    oc_unimplemented();
    return NULL;
}

void oc_wgpu_surface_present(oc_surface handle)
{
    oc_unimplemented();
}
