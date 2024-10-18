/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#ifndef __WGPU_SURFACE_H_
#define __WGPU_SURFACE_H_

#include "app/app.h"
#include "ext/dawn/include/webgpu.h"

ORCA_API oc_surface oc_wgpu_surface_create_for_window(WGPUInstance instance, oc_window window);
ORCA_API WGPUTexture oc_wgpu_surface_get_current_texture(oc_surface handle, WGPUDevice device);
ORCA_API void oc_wgpu_surface_present(oc_surface handle);

#endif //__WGPU_SURFACE_H_
