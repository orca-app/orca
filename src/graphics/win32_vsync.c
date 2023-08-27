/************************************************************/ /**
*
*	@file: win32_vsync.h
*	@author: Reuben Dunnington
*	@date: 26/08/2023
*
*****************************************************************/

// #define WIN32_LEAN_AND_MEAN
// #define WIN32_EXTRA_LEAN
// #define NOMINMAX

// #define D3D11_NO_HELPERS
// #define CINTERFACE
// #define COBJMACROS

// #define interface struct

// #include <windows.h>

#include "platform/platform_thread.h"
#include "app/win32_app.h"

// This forward declaration is needed to suppress an order of declarations bug in the d3d headers.
// d3d11shader.h(454): warning C4115: 'ID3D11ModuleInstance': named type definition in parentheses
// struct ID3D11ModuleInstance;

#define COBJMACROS
#define interface struct
#include <d3d11_1.h>
#include <dxgi1_6.h>
#undef interface

// #include <dxgidebug.h>
// #include <d3dcompiler.h>

typedef struct oc_vsync_data
{
    IDXGIFactory4* factory;
    IDXGIAdapter1* adapter;
} oc_vsync_data;

oc_vsync_data __oc_vsync_data;

typedef struct oc_vsync_thread_data
{
    IDXGIAdapter1* adapter;
    oc_window window;
    bool quit;
} oc_vsync_thread_data;

static i32 oc_window_vysnc_notification_thread(void* userPointer)
{
    oc_vsync_thread_data* threadData = (oc_vsync_thread_data*)userPointer;

    while(!threadData->quit)
    {
        oc_window_data* windowData = oc_window_ptr_from_handle(threadData->window);
        if(!windowData)
        {
            oc_log_error("Failed to get window ptr - assuming window was closed.");
            break;
        }

        RECT windowRect = { 0 };
        if(GetWindowRect(windowData->win32.hWnd, &windowRect) == FALSE)
        {
            oc_log_error("Failed to get window rect with error: %d.", GetLastError());
            return 1;
        }

        //NOTE(Reuben): Wait for VBlank on the display device with which the window has the most intersecting area
        IDXGIOutput* output = NULL;
        IDXGIOutput* selectedOutput = NULL;
        uint32_t selected_intersect_area = 0;
        for(UINT i = 0; DXGI_ERROR_NOT_FOUND != IDXGIAdapter1_EnumOutputs(threadData->adapter, i, &output); ++i)
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
                oc_log_error("Failed to get IDXGIOutput desc with error: %d", hr);
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
                oc_log_warning("Failed to wait for vblank with error: %d", hr);
            }

            SendMessage(windowData->win32.hWnd, OC_WM_USER_VBLANK, 0, 0);

            IDXGIOutput_Release(selectedOutput);
        }
        else
        {
            oc_log_warning("No outputs found. Were all monitors unplugged?");
        }
    }

    return 0;
}

void oc_vsync_init()
{
    if(__oc_vsync_data.adapter)
    {
        return;
    }

    IDXGIFactory4* factory = NULL;
    {
        UINT flags = 0;
        flags |= DXGI_CREATE_FACTORY_DEBUG; // TODO make this optional

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
            oc_log_info("Couldn't find a dedicated hardware DXGI adapater, using software fallback.");
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
        oc_log_info("Couldn't find any DXGI adapters. Vsync will be unavailable.");
        IDXGIFactory_Release(factory);
    }
}

void oc_vsync_window(oc_window window)
{
    if(__oc_vsync_data.adapter)
    {
        oc_vsync_thread_data* data = malloc(sizeof(oc_vsync_thread_data));
        *data = (oc_vsync_thread_data){
            .adapter = __oc_vsync_data.adapter,
            .window = window,
            .quit = false,
        };
        oc_thread_create_with_name(oc_window_vysnc_notification_thread, data, OC_STR8("vblank thread"));
    }
    else
    {
        oc_log_error("No DXGI adapter available - unable to start vsync notification thread.");
    }
}
