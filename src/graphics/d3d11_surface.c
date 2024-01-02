/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "app/app_internal.h"
#include "graphics_surface.h"
#include "d3d11_surface.h"

#pragma comment(lib, "dxguid")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

typedef struct oc_d3d11_surface
{
    oc_surface_data interface;

    ID3D11Device* device;
    ID3D11DeviceContext* context;
    IDXGISwapChain1* swapChain;
    ID3D11RenderTargetView* renderTargetView;
    oc_vec2 swapChainSize;
    bool vsync;

} oc_d3d11_surface;

void oc_d3d11_surface_destroy(oc_surface_data* interface)
{
    oc_d3d11_surface* surface = (oc_d3d11_surface*)interface;

    //TODO

    oc_surface_cleanup((oc_surface_data*)surface);
    free(surface);
}

void oc_d3d11_surface_prepare(oc_surface_data* interface)
{
    oc_d3d11_surface* surface = (oc_d3d11_surface*)interface;

    //TODO: resize swapchain buffers if needed, recreate rtView if needed

    // resize swap chain if needed
    oc_vec2 layerSize = oc_win32_surface_get_size(interface);
    oc_vec2 layerScaling = oc_win32_surface_contents_scaling(interface);
    layerSize.x *= layerScaling.x;
    layerSize.y *= layerScaling.y;

    if(surface->renderTargetView == NULL
       || surface->swapChainSize.x != layerSize.x
       || surface->swapChainSize.y != layerSize.y)
    {
        if(surface->renderTargetView)
        {
            // release old swap chain buffers
            ID3D11DeviceContext_ClearState(surface->context);
            ID3D11RenderTargetView_Release(surface->renderTargetView);
            surface->renderTargetView = NULL;
        }

        // resize to new size for non-zero size
        if(layerSize.x != 0 && layerSize.y != 0)
        {
            HRESULT hr = IDXGISwapChain1_ResizeBuffers(surface->swapChain, 0, layerSize.x, layerSize.y, DXGI_FORMAT_UNKNOWN, 0);
            if(FAILED(hr))
            {
                OC_ASSERT(0, "Failed to resize swap chain!");
            }

            // create RenderTarget view for new backbuffer texture
            ID3D11Texture2D* backbuffer;
            IDXGISwapChain1_GetBuffer(surface->swapChain, 0, &IID_ID3D11Texture2D, (void**)&backbuffer);
            ID3D11Device_CreateRenderTargetView(surface->device, (ID3D11Resource*)backbuffer, NULL, &surface->renderTargetView);
            ID3D11Texture2D_Release(backbuffer);
        }
    }
    surface->swapChainSize = layerSize;
}

void oc_d3d11_surface_present(oc_surface_data* interface)
{
    oc_d3d11_surface* surface = (oc_d3d11_surface*)interface;

    HRESULT hr = IDXGISwapChain1_Present(surface->swapChain, surface->vsync ? 1 : 0, 0);
    if(hr == DXGI_STATUS_OCCLUDED)
    {
        // window is minimized, cannot vsync - instead sleep a bit
        if(surface->vsync)
        {
            Sleep(10);
        }
    }
    else if(FAILED(hr))
    {
        //TODO: actually log an error
        OC_ASSERT(0, "Failed to present swap chain! Device lost?");
    }
}

void oc_d3d11_surface_deselect(oc_surface_data* interface)
{
    oc_d3d11_surface* surface = (oc_d3d11_surface*)interface;
    //TODO
}

void oc_d3d11_surface_swap_interval(oc_surface_data* interface, int swap)
{
    oc_d3d11_surface* surface = (oc_d3d11_surface*)interface;
    surface->vsync = (swap != 0);
}

void oc_d3d11_surface_init(oc_d3d11_surface* surface)
{
    surface->interface.api = OC_D3D11;

    surface->interface.destroy = oc_d3d11_surface_destroy;
    surface->interface.prepare = oc_d3d11_surface_prepare;
    surface->interface.present = oc_d3d11_surface_present;
    surface->interface.deselect = oc_d3d11_surface_deselect;
    surface->interface.swapInterval = oc_d3d11_surface_swap_interval;

    HWND window = (HWND)surface->interface.nativeLayer((oc_surface_data*)surface);
    HRESULT hr = 0;

    // create D3D11 device & context
    {
        UINT flags = 0;
#ifndef NDEBUG
        // this enables VERY USEFUL debug messages in debugger output
        flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };
        hr = D3D11CreateDevice(
            NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, levels, ARRAYSIZE(levels),
            D3D11_SDK_VERSION, &surface->device, NULL, &surface->context);
        // make sure device creation succeeeds before continuing
        // for simple applciation you could retry device creation with
        // D3D_DRIVER_TYPE_WARP driver type which enables software rendering
        // (could be useful on broken drivers or remote desktop situations)
        //TODO: log and return an error
        OC_ASSERT(hr == 0);
    }

#ifndef NDEBUG
    // for debug builds enable VERY USEFUL debug break on API errors
    {
        ID3D11InfoQueue* info;
        ID3D11Device_QueryInterface(surface->device, &IID_ID3D11InfoQueue, (void**)&info);
        ID3D11InfoQueue_SetBreakOnSeverity(info, D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        ID3D11InfoQueue_SetBreakOnSeverity(info, D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
        ID3D11InfoQueue_Release(info);
    }

    // enable debug break for DXGI too
    {
        IDXGIInfoQueue* dxgiInfo;
        hr = DXGIGetDebugInterface1(0, &IID_IDXGIInfoQueue, (void**)&dxgiInfo);
        OC_ASSERT(hr == 0);
        IDXGIInfoQueue_SetBreakOnSeverity(dxgiInfo, DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        IDXGIInfoQueue_SetBreakOnSeverity(dxgiInfo, DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE);
        IDXGIInfoQueue_Release(dxgiInfo);
    }

    // after this there's no need to check for any errors on device functions manually
    // so all HRESULT return  values in this code will be ignored
    // debugger will break on errors anyway
#endif

    // create DXGI swap chain
    {
        // get DXGI device from D3D11 device
        IDXGIDevice* dxgiDevice;
        hr = ID3D11Device_QueryInterface(surface->device, &IID_IDXGIDevice, (void**)&dxgiDevice);
        OC_ASSERT(hr == 0);

        // get DXGI adapter from DXGI device
        IDXGIAdapter* dxgiAdapter;
        hr = IDXGIDevice_GetAdapter(dxgiDevice, &dxgiAdapter);
        OC_ASSERT(hr == 0);

        // get DXGI factory from DXGI adapter
        IDXGIFactory2* factory;
        hr = IDXGIAdapter_GetParent(dxgiAdapter, &IID_IDXGIFactory2, (void**)&factory);
        OC_ASSERT(hr == 0);

        DXGI_SWAP_CHAIN_DESC1 desc = {
            // default 0 value for width & height means to get it from HWND automatically
            //.Width = 0,
            //.Height = 0,

            // or use DXGI_FORMAT_R8G8B8A8_UNORM_SRGB for storing sRGB
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,

            // FLIP presentation model does not allow MSAA framebuffer
            // if you want MSAA then you'll need to render offscreen and manually
            // resolve to non-MSAA framebuffer
            .SampleDesc = { 1, 0 },

            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = 2,

            // we don't want any automatic scaling of window content
            // this is supported only on FLIP presentation model
            .Scaling = DXGI_SCALING_NONE,

            // use more efficient FLIP presentation model
            // Windows 10 allows to use DXGI_SWAP_EFFECT_FLIP_DISCARD
            // for Windows 8 compatibility use DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL
            // for Windows 7 compatibility use DXGI_SWAP_EFFECT_DISCARD
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
        };

        hr = IDXGIFactory2_CreateSwapChainForHwnd(factory, (IUnknown*)surface->device, window, &desc, NULL, NULL, &surface->swapChain);
        OC_ASSERT(hr == 0);

        // disable silly Alt+Enter changing monitor resolution to match window size
        IDXGIFactory_MakeWindowAssociation(factory, window, DXGI_MWA_NO_ALT_ENTER);

        IDXGIFactory2_Release(factory);
        IDXGIAdapter_Release(dxgiAdapter);
        IDXGIDevice_Release(dxgiDevice);
    }
}

oc_surface_data* oc_d3d11_surface_create_for_window(oc_window window)
{
    oc_d3d11_surface* surface = 0;
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        surface = oc_malloc_type(oc_d3d11_surface);
        if(surface)
        {
            memset(surface, 0, sizeof(oc_d3d11_surface));

            oc_surface_init_for_window((oc_surface_data*)surface, windowData);
            oc_d3d11_surface_init(surface);
        }
    }
    return ((oc_surface_data*)surface);
}

ID3D11Device* oc_d3d11_surface_device(oc_surface surface)
{
    ID3D11Device* device = 0;
    oc_surface_data* surfaceData = oc_surface_data_from_handle(surface);
    if(surfaceData && surfaceData->api == OC_D3D11)
    {
        oc_d3d11_surface* d3dSurface = (oc_d3d11_surface*)surfaceData;
        device = d3dSurface->device;
    }
    return (device);
}

ID3D11DeviceContext* oc_d3d11_surface_context(oc_surface surface)
{
    ID3D11DeviceContext* context = 0;
    oc_surface_data* surfaceData = oc_surface_data_from_handle(surface);
    if(surfaceData && surfaceData->api == OC_D3D11)
    {
        oc_d3d11_surface* d3dSurface = (oc_d3d11_surface*)surfaceData;
        context = d3dSurface->context;
    }
    return (context);
}

ID3D11RenderTargetView* oc_d3d11_surface_get_render_target_view(oc_surface surface)
{
    ID3D11RenderTargetView* renderTargetView = 0;
    oc_surface_data* surfaceData = oc_surface_data_from_handle(surface);
    if(surfaceData && surfaceData->api == OC_D3D11)
    {
        oc_d3d11_surface* d3dSurface = (oc_d3d11_surface*)surfaceData;
        renderTargetView = d3dSurface->renderTargetView;
    }
    return (renderTargetView);
}
