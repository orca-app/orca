/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "platform/platform_thread.h"
#include "app/win32_app.h"

#define COBJMACROS
#define interface struct
#include <d3d11_1.h>
#include <dxgi1_6.h>
#undef interface

typedef struct oc_vsync_data
{
    IDXGIFactory4* factory;
    IDXGIAdapter1* adapter;
} oc_vsync_data;

oc_vsync_data __oc_vsync_data;

void oc_vsync_init(void)
{
    if(__oc_vsync_data.adapter)
    {
        return;
    }

    IDXGIFactory4* factory = NULL;
    {
        UINT flags = 0;
        // flags |= DXGI_CREATE_FACTORY_DEBUG; // TODO make this optional

        HRESULT hr = CreateDXGIFactory2(flags, &IID_IDXGIFactory4, (void**)&factory);
        if(!SUCCEEDED(hr))
        {
            return;
        }
    }

    IDXGIAdapter1* adapter = NULL;
    IDXGIAdapter1* adapterFallback = NULL;
    IDXGIAdapter1* enumeratedAdapter = NULL;

    for(UINT i = 0; DXGI_ERROR_NOT_FOUND != IDXGIFactory1_EnumAdapters1(factory, i, &enumeratedAdapter); ++i)
    {
        DXGI_ADAPTER_DESC1 adapterDesc;
        IDXGIAdapter1_GetDesc1(enumeratedAdapter, &adapterDesc);
        if(adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            adapterFallback = enumeratedAdapter;
            continue;
        }
        else
        {
            adapter = enumeratedAdapter;
        }
        break;
    }

    if(adapter == NULL)
    {
        adapter = adapterFallback;
        if(adapter)
        {
            oc_log_info("Couldn't find a dedicated hardware DXGI adapater, using software fallback.\n");
        }
    }

    if(adapter)
    {
        __oc_vsync_data = (oc_vsync_data){
            .factory = factory,
            .adapter = adapter,
        };
    }
    else
    {
        oc_log_info("Couldn't find any DXGI adapters - vsync will be unavailable.\n");
        IDXGIFactory_Release(factory);
    }
}

void oc_vsync_wait(oc_window window)
{
    if(__oc_vsync_data.adapter)
    {
        oc_window_data* windowData = oc_window_ptr_from_handle(window);
        if(!windowData)
        {
            oc_log_error("Failed to get window ptr - assuming window was closed.\n");
            return;
        }

        RECT windowRect = { 0 };
        if(GetWindowRect(windowData->win32.hWnd, &windowRect) == FALSE)
        {
            oc_log_error("Failed to get window rect with error: %d.\n", GetLastError());
            return;
        }

        // Wait for VBlank on the display device with which the window has the most intersecting area
        IDXGIOutput* output = NULL;
        IDXGIOutput* selectedOutput = NULL;
        uint32_t selected_intersect_area = 0;
        for(UINT i = 0; DXGI_ERROR_NOT_FOUND != IDXGIAdapter1_EnumOutputs(__oc_vsync_data.adapter, i, &output); ++i)
        {
            DXGI_OUTPUT_DESC outputDesc = { 0 };
            HRESULT hr = IDXGIOutput_GetDesc(output, &outputDesc);
            if(SUCCEEDED(hr))
            {
                RECT intersectRect = { 0 };
                if(IntersectRect(&intersectRect, &windowRect, &outputDesc.DesktopCoordinates))
                {
                    uint32_t outputIntersectArea = (intersectRect.right - intersectRect.left) * (intersectRect.bottom - intersectRect.top);

                    if(selectedOutput == NULL || outputIntersectArea > selected_intersect_area)
                    {
                        selectedOutput = output;
                        selected_intersect_area = outputIntersectArea;
                    }
                }
            }
            else
            {
                oc_log_error("Failed to get IDXGIOutput desc with error: %d\n", hr);
            }

            if(selectedOutput != output)
            {
                IDXGIOutput_Release(output);
            }
        }

        if(selectedOutput)
        {
            HRESULT hr = IDXGIOutput_WaitForVBlank(selectedOutput);

            if(FAILED(hr))
            {
                // TODO(reuben) - fall back to software timer
                oc_log_warning("Failed to wait for vblank with error: %d\n", hr);
            }

            IDXGIOutput_Release(selectedOutput);
        }
        else if(output)
        {
            // just use the last output found as a fallback - the window may be minimized or not otherwise visible on any monitor
            IDXGIOutput_WaitForVBlank(output);
        }
    }
}
