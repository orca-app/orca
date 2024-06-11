
#include "webgpu.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include<dwmapi.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>
#include <stddef.h>

// replace this with your favorite Assert() implementation
#include <intrin.h>
#define Assert(cond) do { if (!(cond)) __debugbreak(); } while (0)

#pragma comment (lib, "gdi32")
#pragma comment (lib, "user32")
#pragma comment (lib, "webgpu")
#pragma comment (lib, "dwmapi")

static void FatalError(const char* message)
{
    MessageBoxA(NULL, message, "Error", MB_ICONEXCLAMATION);
    ExitProcess(0);
}

static void OnDeviceError(WGPUErrorType type, const char* message, void* userdata)
{
    FatalError(message);
}

static void OnAdapterRequestEnded(WGPURequestAdapterStatus status, WGPUAdapter adapter, const char* message, void* userdata)
{
    if (status != WGPURequestAdapterStatus_Success)
    {
        // cannot find adapter?
        FatalError(message);
    }
    else
    {
        // use first adapter provided
        WGPUAdapter* result = userdata;
        if (*result == NULL)
        {
            *result = adapter;
        }
    }
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

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE previnstance, LPSTR cmdline, int cmdshow)
{
    // register window class to have custom WindowProc callback
    WNDCLASSEXW wc =
    {
        .cbSize = sizeof(wc),
        .lpfnWndProc = &WindowProc,
        .hInstance = hinstance,
        .hIcon = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor = LoadCursor(NULL, IDC_ARROW),
        .lpszClassName = L"webgpu_window_class",
    };
    ATOM atom = RegisterClassExW(&wc);
    Assert(atom && "Failed to register window class");

    // window properties - width, height and style
    int width = CW_USEDEFAULT;
    int height = CW_USEDEFAULT;
    DWORD exstyle = WS_EX_APPWINDOW | WS_EX_NOREDIRECTIONBITMAP;
    DWORD style = WS_OVERLAPPEDWINDOW;

    // uncomment in case you want fixed size window
    //style &= ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
    //RECT rect = { 0, 0, 1280, 720 };
    //AdjustWindowRectEx(&rect, style, FALSE, exstyle);
    //width = rect.right - rect.left;
    //height = rect.bottom - rect.top;

    // create window
    HWND window = CreateWindowExW(
        exstyle, wc.lpszClassName, L"WebGPU Window", style,
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

    const WGPUTextureFormat kSwapChainFormat = WGPUTextureFormat_BGRA8Unorm;

    // optionally use WGPUInstanceDescriptor::nextInChain for WGPUDawnTogglesDescriptor
    // with various toggles enabled or disabled: https://dawn.googlesource.com/dawn/+/refs/heads/main/src/dawn/native/Toggles.cpp
    WGPUInstance instance = wgpuCreateInstance(NULL);
    Assert(instance && "Failed to create WebGPU instance");

    WGPUSurface surface;
    {
        WGPUSurfaceDescriptor desc =
        {
            .nextInChain = &((WGPUSurfaceDescriptorFromWindowsHWND)
            {
                .chain.sType = WGPUSType_SurfaceDescriptorFromWindowsHWND,
                .hinstance = wc.hInstance,
                .hwnd = window,
            }).chain,
        };
        surface = wgpuInstanceCreateSurface(instance, &desc);
        Assert(surface && "Failed to create WebGPU surface");
    }

    WGPUAdapter adapter = NULL;
    {
        WGPURequestAdapterOptions options =
        {
            .compatibleSurface = surface,
            // .powerPreference = WGPUPowerPreference_HighPerformance,
        };
        wgpuInstanceRequestAdapter(instance, &options, &OnAdapterRequestEnded, &adapter);
        Assert(adapter && "Failed to get WebGPU adapter");

        // can query extra details on what adapter supports:
        // wgpuAdapterEnumerateFeatures
        // wgpuAdapterGetLimits
        // wgpuAdapterGetProperties
        // wgpuAdapterHasFeature

        {
            WGPUAdapterProperties properties = { 0 };
            wgpuAdapterGetProperties(adapter, &properties);

            const char* adapter_types[] =
            {
                [WGPUAdapterType_DiscreteGPU] = "Discrete GPU",
                [WGPUAdapterType_IntegratedGPU] = "Integrated GPU",
                [WGPUAdapterType_CPU] = "CPU",
                [WGPUAdapterType_Unknown] = "unknown",
            };

            char temp[1024];
            wsprintfA(temp,
                "Adapter       = %s\n"
                "Driver        = %s\n"
                "Vendor        = %s\n"
                "Architecture  = %s\n"
                "Adapter Type  = %s\n",
                properties.name,
                properties.driverDescription,
                properties.vendorName,
                properties.architecture,
                adapter_types[properties.adapterType]);

            OutputDebugStringA(temp);
        }
    }

    WGPUDevice device = NULL;
    {
        WGPUSupportedLimits supported = { 0 };
        wgpuAdapterGetLimits(adapter, &supported);

        WGPUDeviceDescriptor desc =
        {
            // extra features: https://dawn.googlesource.com/dawn/+/refs/heads/main/src/dawn/native/Features.cpp
            //.requiredFeaturesCount
            //.requiredFeatures = (WGPUFeatureName[]) { ... }
            .requiredLimits = &(WGPURequiredLimits) { .limits = supported.limits },
        };

        device = wgpuAdapterCreateDevice(adapter, &desc);
        Assert(device && "Failed to create WebGPU device");
    }

    // notify on errors
    wgpuDeviceSetUncapturedErrorCallback(device, &OnDeviceError, NULL);

    // default device queue
    WGPUQueue queue = wgpuDeviceGetQueue(device);

    // show the window
    ShowWindow(window, SW_SHOWDEFAULT);

    LARGE_INTEGER freq, c1;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&c1);

    float angle = 0;
    int current_width = 0;
    int current_height = 0;

    // ============================================================================================
    // swap chain will be created when window will be resized
    WGPUSwapChain swap_chain = NULL;

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

        // get current size for window client area
        RECT rect;
        GetClientRect(window, &rect);
        width = rect.right - rect.left;
        height = rect.bottom - rect.top;

        // process all internal async events or error callbacks
        // this is dawn specific functionality, because in browser's WebGPU everything works with JS event loop
        wgpuDeviceTick(device);

        // resize swap chain if needed
        if (swap_chain == NULL || width != current_width || height != current_height)
        {
            if (swap_chain)
            {
                // release old swap chain
                wgpuSwapChainRelease(swap_chain);
                swap_chain = NULL;
            }

            // resize to new size for non-zero window size
            if (width != 0 && height != 0)
            {
                WGPUSwapChainDescriptor desc =
                {
                    .width = width,
                    .height = height,
                    .usage = WGPUTextureUsage_RenderAttachment,
                    .format = kSwapChainFormat,
                    .presentMode = WGPUPresentMode_Fifo, // WGPUPresentMode_Mailbox // WGPUPresentMode_Immediate
                };
                swap_chain = wgpuDeviceCreateSwapChain(device, surface, &desc);
                Assert(surface && "Failed to create WebGPU swap chain");
            }

            current_width = width;
            current_height = height;
        }

        LARGE_INTEGER c2;
        QueryPerformanceCounter(&c2);
        float delta = (float)((double)(c2.QuadPart - c1.QuadPart) / freq.QuadPart);
        c1 = c2;

        // render only if window size is non-zero, which means swap chain is created
        if (swap_chain)
        {
            WGPUTextureView next = wgpuSwapChainGetCurrentTextureView(swap_chain);
            if (!next)
            {
                FatalError("Cannot acquire next swap chain texture!");
            }

            WGPURenderPassDescriptor desc =
            {
                .colorAttachmentCount = 1,
                .colorAttachments = (WGPURenderPassColorAttachment[])
                {
                    // one output to write to, initially cleared with background color
                    {
                        .view = next,
                        .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
                        .loadOp = WGPULoadOp_Clear,
                        .storeOp = WGPUStoreOp_Store,
                        .clearValue = { 0.392, 0.584, 0.929, 0.5 }, // r,g,b,a
                    },
                },
            };

            WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, NULL);

            // encode render pass
            WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &desc);
            wgpuRenderPassEncoderEnd(pass);

            // submit to queue
            WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, NULL);
            wgpuQueueSubmit(queue, 1, &command);

            wgpuCommandBufferRelease(command);
            wgpuCommandEncoderRelease(encoder);
            wgpuTextureViewRelease(next);

            // present to output
            wgpuSwapChainPresent(swap_chain);
        }
    }
}
