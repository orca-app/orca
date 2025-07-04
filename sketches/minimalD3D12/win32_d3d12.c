// example how to set up D3D12 rendering on Windows in C

#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <dxgidebug.h>
#include <dwmapi.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <stddef.h>

// replace this with your favorite Assert() implementation
#include <intrin.h>
#define Assert(cond) do { if (!(cond)) __debugbreak(); } while (0)
#define AssertHR(hr) Assert(SUCCEEDED(hr))

#pragma comment (lib, "gdi32")
#pragma comment (lib, "user32")
#pragma comment (lib, "dxguid")
#pragma comment (lib, "dxgi")
#pragma comment (lib, "d3d12")
#pragma comment (lib, "d3dcompiler")
#pragma comment (lib, "dwmapi")

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
    // WS_EX_NOREDIRECTIONBITMAP flag here is needed to fix ugly bug with Windows 10
    // when window is resized and DXGI swap chain uses FLIP presentation model
    // DO NOT use it if you choose to use non-FLIP presentation model
    // read about the bug here: https://stackoverflow.com/q/63096226 and here: https://stackoverflow.com/q/53000291
    DWORD exstyle = WS_EX_APPWINDOW ;//| WS_EX_NOREDIRECTIONBITMAP;
    DWORD style = WS_OVERLAPPEDWINDOW;

    // uncomment in case you want fixed size window
    //style &= ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
    //RECT rect = { 0, 0, 1280, 720 };
    //AdjustWindowRectEx(&rect, style, FALSE, exstyle);
    //width = rect.right - rect.left;
    //height = rect.bottom - rect.top;

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
        /*
        // get DXGI device from D3D12 device
        IDXGIDevice* dxgiDevice;
        hr = ID3D12Device_QueryInterface(device, &IID_IDXGIDevice, (void**)&dxgiDevice);
        AssertHR(hr);


        // get DXGI adapter from DXGI device
        IDXGIAdapter* dxgiAdapter;
        hr = IDXGIDevice_GetAdapter(dxgiDevice, &dxgiAdapter);
        AssertHR(hr);
        */
        // get DXGI factory from DXGI adapter
        IDXGIFactory4* factory;
        hr = CreateDXGIFactory1(&IID_IDXGIFactory4, &factory);
        AssertHR(hr);

        DXGI_SWAP_CHAIN_DESC desc =
        {
            // default 0 value for width & height means to get it from HWND automatically
            //.Width = 0,
            //.Height = 0,

            // or use DXGI_FORMAT_R8G8B8A8_UNORM_SRGB for storing sRGB
            .BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM,

            // FLIP presentation model does not allow MSAA framebuffer
            // if you want MSAA then you'll need to render offscreen and manually
            // resolve to non-MSAA framebuffer
            .SampleDesc = { 1, 0 },

            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = FrameCount,

            // use more efficient FLIP presentation model
            // Windows 10 allows to use DXGI_SWAP_EFFECT_FLIP_DISCARD
            // for Windows 8 compatibility use DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL
            // for Windows 7 compatibility use DXGI_SWAP_EFFECT_DISCARD
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,

            .OutputWindow = window,
            .Windowed = TRUE,
        };

        hr = IDXGIFactory4_CreateSwapChain(factory, (IUnknown*)commandQueue, &desc, (IDXGISwapChain**)&swapChain);
        AssertHR(hr);

        // disable silly Alt+Enter changing monitor resolution to match window size
        IDXGIFactory_MakeWindowAssociation(factory, window, DXGI_MWA_NO_ALT_ENTER);

        IDXGIFactory4_Release(factory);


        //m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

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

    // show the window
    ShowWindow(window, SW_SHOWDEFAULT);

    LARGE_INTEGER freq, c1;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&c1);

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

        // resize swap chain if needed
        /*
        if (rtView == NULL || width != currentWidth || height != currentHeight)
        {
            if (rtView)
            {
                // release old swap chain buffers

                ID3D12RenderTargetView_Release(rtView);
                rtView = NULL;
            }

            // resize to new size for non-zero size
            if (width != 0 && height != 0)
            {
                hr = IDXGISwapChain1_ResizeBuffers(swapChain, 0, width, height, DXGI_FORMAT_UNKNOWN, 0);
                if (FAILED(hr))
                {
                    FatalError("Failed to resize swap chain!");
                }

                // create RenderTarget view for new backbuffer texture
                ID3D12Texture2D* backbuffer;
                IDXGISwapChain1_GetBuffer(swapChain, 0, &IID_ID3D12Texture2D, (void**)&backbuffer);
                ID3D12Device_CreateRenderTargetView(device, (ID3D12Resource*)backbuffer, NULL, &rtView);
                ID3D12Texture2D_Release(backbuffer);
            }

            currentWidth = width;
            currentHeight = height;
        }
        */
        // can render only if window size is non-zero - we must have backbuffer & RenderTarget view created


        ID3D12CommandAllocator_Reset(commandAllocator);
        ID3D12GraphicsCommandList_Reset(commandList, commandAllocator, NULL);

//        if (rtView)
        {
            LARGE_INTEGER c2;
            QueryPerformanceCounter(&c2);
            float delta = (float)((double)(c2.QuadPart - c1.QuadPart) / freq.QuadPart);
            c1 = c2;

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

        }

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
