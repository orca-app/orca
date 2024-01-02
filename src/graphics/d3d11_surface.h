/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#ifndef __D3D11_SURFACE_H_
#define __D3D11_SURFACE_H_

#include "app/app.h"
#include "graphics_surface.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define COBJMACROS
#define interface struct
#include <d3d11.h>
#include <dxgi1_3.h>
#include <d3dcompiler.h>
#include <dxgidebug.h>
#undef interface

oc_surface_data* oc_d3d11_surface_create_for_window(oc_window window);

ORCA_API ID3D11Device* oc_d3d11_surface_device(oc_surface surface);
ORCA_API ID3D11DeviceContext* oc_d3d11_surface_context(oc_surface surface);
ORCA_API ID3D11RenderTargetView* oc_d3d11_surface_get_render_target_view(oc_surface surface);

#endif // __D3D11_SURFACE_H_
