// example how to set up D3D12 rendering with a transparent window on Windows in C

#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <dxgidebug.h>
#include <dwmapi.h >

#include"dcomp_c_api.h"
#include<initguid.h>
EXTERN_C const IID IID_IDCompositionDevice;
DEFINE_GUID(IID_IDCompositionDevice, 0xC37EA93A, 0xE7AA, 0x450D, 0xB1, 0x6F, 0x97, 0x46, 0xCB, 0x04, 0x07, 0xF3);

#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>
#include <string.h>
#include <stddef.h>

#include <intrin.h>
#define Assert(cond) do { if (!(cond)) __debugbreak(); } while (0)
#define AssertHR(hr) Assert(SUCCEEDED(hr))

#pragma comment (lib, "gdi32")
#pragma comment (lib, "user32")
#pragma comment (lib, "dxguid")
#pragma comment (lib, "dxgi")
#pragma comment (lib, "d3d11")
#pragma comment (lib, "d3d12")
#pragma comment (lib, "d3dcompiler")
#pragma comment (lib, "dwmapi")
#pragma comment (lib, "dcomp")

#define STR2(x) #x
#define STR(x) STR2(x)

static void FatalError(const char* message)
{
    MessageBoxA(NULL, message, "Error", MB_ICONEXCLAMATION);
    ExitProcess(0);
}

static LRESULT CALLBACK WindowProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(wnd, msg, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previnstance, LPSTR cmdline, int cmdshow)
{
    // register window class to have custom WindowProc callback
    WNDCLASSEXW wc =
    {
        .cbSize = sizeof(wc),
        .lpfnWndProc = WindowProc,
        .hInstance = instance,
        .hIcon = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor = LoadCursor(NULL, IDC_ARROW),
        .lpszClassName = L"d3d12_window_class",
    };
    ATOM atom = RegisterClassExW(&wc);
    Assert(atom && "Failed to register window class");

    // window properties - width, height and style
    int width = CW_USEDEFAULT;
    int height = CW_USEDEFAULT;
    DWORD exstyle = WS_EX_APPWINDOW | WS_EX_NOREDIRECTIONBITMAP;
    DWORD style = WS_OVERLAPPEDWINDOW;

    // create window
    HWND window = CreateWindowExW(
        exstyle, wc.lpszClassName, L"D3D12 Window", style,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        NULL, NULL, wc.hInstance, NULL);
    Assert(window && "Failed to create window");

    //NOTE: make the window transparent
    {
        HRGN region = CreateRectRgn(0, 0, -1, -1);

        DWM_BLURBEHIND bb = { 0 };
        bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
        bb.hRgnBlur = region;
        bb.fEnable = TRUE;

        HRESULT res = DwmEnableBlurBehindWindow(window, &bb);

        DeleteObject(region);
        if(res != S_OK)
        {
            printf("couldn't enable DWM blur behind window\n");
        }
    }

    RECT rect;
    GetClientRect(window, &rect);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    const int FrameCount = 2;

    HRESULT hr;
    ID3D12Debug* debug;
    ID3D12Device* device;
    IDXGISwapChain3* swapChain;
    ID3D12CommandQueue* commandQueue;
    ID3D12DescriptorHeap* rtvHeap;
    ID3D12Resource* renderTargets[2];
    ID3D12CommandAllocator* commandAllocator;
    ID3D12GraphicsCommandList* commandList;

    hr = D3D12GetDebugInterface(&IID_ID3D12Debug, &debug);
    AssertHR(hr);

    IDXGIFactory4* factory;
    hr = CreateDXGIFactory1(&IID_IDXGIFactory4, &factory);
    AssertHR(hr);

    hr = D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device, &device);
    AssertHR(hr);

    D3D12_COMMAND_QUEUE_DESC queueDesc = {
        .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
        .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
    };

    hr = ID3D12Device_CreateCommandQueue(device, &queueDesc, &IID_ID3D12CommandQueue, &commandQueue);
    AssertHR(hr);

    // create DXGI swap chain
    {
        DXGI_SWAP_CHAIN_DESC1 desc =
        {
            .Width = width,
            .Height = height,
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .SampleDesc = { 1, 0 },
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = FrameCount,
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
            .AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED,
        };

        hr = IDXGIFactory4_CreateSwapChainForComposition(factory, (IUnknown*)commandQueue, &desc, NULL, (IDXGISwapChain1**)&swapChain);
        AssertHR(hr);

        // disable silly Alt+Enter changing monitor resolution to match window size
        IDXGIFactory_MakeWindowAssociation(factory, window, DXGI_MWA_NO_ALT_ENTER);
        IDXGIFactory4_Release(factory);

        // Create descriptor heaps.
        {
            // Describe and create a render target view (RTV) descriptor heap.
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
            rtvHeapDesc.NumDescriptors = FrameCount;
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            hr = ID3D12Device_CreateDescriptorHeap(device, &rtvHeapDesc, &IID_ID3D12DescriptorHeap, &rtvHeap);
        }

        // Create frame resources.
        {
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
            ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart(rtvHeap, &rtvHandle);

            int descriptorIncrementSize = ID3D12Device_GetDescriptorHandleIncrementSize(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            // Create a RTV for each frame.
            for (UINT n = 0; n < FrameCount; n++)
            {
                hr = IDXGISwapChain3_GetBuffer(swapChain, n, &IID_ID3D12Resource , &renderTargets[n]);
                AssertHR(hr);
                ID3D12Device_CreateRenderTargetView(device, renderTargets[n], NULL, rtvHandle);

                rtvHandle.ptr += descriptorIncrementSize;
            }
        }

        hr = ID3D12Device_CreateCommandAllocator(device, D3D12_COMMAND_LIST_TYPE_DIRECT, &IID_ID3D12CommandAllocator, &commandAllocator);
        AssertHR(hr);

        hr = ID3D12Device_CreateCommandList(device, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, NULL, &IID_ID3D12GraphicsCommandList, &commandList);
        AssertHR(hr);
        ID3D12GraphicsCommandList_Close(commandList);
    }

    // Create composition
    IDCompositionDevice *compositeDevice = NULL;
    {
        //NOTE: we create a D3D11Device only to get the DXGIDevice, since getting it from
        //      a D3D12Device is not exposed anymore.
        ID3D11Device *d3d11Device;
        D3D_FEATURE_LEVEL featureLevelSupported;

        hr = D3D11CreateDevice(
            NULL,
            D3D_DRIVER_TYPE_HARDWARE,
            NULL,
            D3D11_CREATE_DEVICE_BGRA_SUPPORT,
            NULL,
            0,
            D3D11_SDK_VERSION,
            &d3d11Device,
            &featureLevelSupported,
            NULL);
        AssertHR(hr);

        IDXGIDevice* dxgiDevice;
        hr = ID3D11Device_QueryInterface(d3d11Device, &IID_IDXGIDevice, (void**)&dxgiDevice);
        AssertHR(hr);

        hr = DCompositionCreateDevice(dxgiDevice, &IID_IDCompositionDevice, &compositeDevice);
        AssertHR(hr);

        ID3D11Device_Release(d3d11Device);
    }
    IDCompositionTarget *compositeTarget = NULL;
    compositeDevice->lpVtbl->CreateTargetForHwnd(compositeDevice, window, TRUE, &compositeTarget);

    IDCompositionVisual *compositeVisual = NULL;
    compositeDevice->lpVtbl->CreateVisual(compositeDevice, &compositeVisual);

    compositeVisual->lpVtbl->SetContent(compositeVisual, (IUnknown*)swapChain);
    compositeTarget->lpVtbl->SetRoot(compositeTarget, compositeVisual);
    compositeDevice->lpVtbl->Commit(compositeDevice);

    // show the window
    ShowWindow(window, SW_SHOWDEFAULT);

    DWORD currentWidth = 0;
    DWORD currentHeight = 0;

    int counter = 0;
    for (;;)
    {
        // process all incoming Windows messages
        MSG msg;
        if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            continue;
        }

        if(counter > 0)
        {
            continue;
        }
        counter++;

        // get current size for window client area
        RECT rect;
        GetClientRect(window, &rect);
        width = rect.right - rect.left;
        height = rect.bottom - rect.top;

        ID3D12CommandAllocator_Reset(commandAllocator);
        ID3D12GraphicsCommandList_Reset(commandList, commandAllocator, NULL);

        // output viewport covering all client area of window
        D3D12_VIEWPORT viewport =
        {
            .TopLeftX = 0,
            .TopLeftY = 0,
            .Width = (FLOAT)width,
            .Height = (FLOAT)height,
            .MinDepth = 0,
            .MaxDepth = 1,
        };
        ID3D12GraphicsCommandList_RSSetViewports(commandList, 1, &viewport);

        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
        ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart(rtvHeap, &rtvHandle);
        ID3D12GraphicsCommandList_OMSetRenderTargets(commandList, 1, &rtvHandle, FALSE, NULL);

        // clear screen
        FLOAT color[] = { 0.392f, 0.584f, 0.929f, 0.5f };
        ID3D12GraphicsCommandList_ClearRenderTargetView(commandList, rtvHandle, color, 0, NULL);

        ID3D12GraphicsCommandList_Close(commandList);

        ID3D12CommandList* ppCommandLists[] = { (ID3D12CommandList*)commandList };
        ID3D12CommandQueue_ExecuteCommandLists(commandQueue, _countof(ppCommandLists), ppCommandLists);

        // change to FALSE to disable vsync
        BOOL vsync = TRUE;
        hr = IDXGISwapChain1_Present(swapChain, vsync ? 1 : 0, 0);
        if (hr == DXGI_STATUS_OCCLUDED)
        {
            // window is minimized, cannot vsync - instead sleep a bit
            if (vsync)
            {
                Sleep(10);
            }
        }
        else if (FAILED(hr))
        {
            FatalError("Failed to present swap chain! Device lost?");
        }
    }
}
