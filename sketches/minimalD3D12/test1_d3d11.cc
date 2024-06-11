#define WIN32_LEAN_AND_MEAN 1
#define UNICODE 1
#define NOMINMAX 1
#include <Windows.h>
#include <d3d11.h>
#include <dcomp.h>
#include <dxgi1_3.h>

#pragma comment(lib, "USER32")
#pragma comment(lib, "OLE32")
#pragma comment(lib, "D3D11")
#pragma comment(lib, "DXGI")
#pragma comment(lib, "DCOMP")

int width = 0, height = 0;

bool should_continue = true;

LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wParam,
                             LPARAM lParam) {
  if (message == WM_CLOSE) {
    should_continue = false;
    PostQuitMessage(0);
    return 0;
  } else if (message == WM_SIZE) {
    width = LOWORD(lParam);
    height = HIWORD(lParam);
  }

  return DefWindowProcW(window, message, wParam, lParam);
}

int wWinMain(HINSTANCE selfInstance, HINSTANCE prevInstance, LPWSTR commandLine,
             int showCommand) {

  WNDCLASSEXW windowClass = {
      .cbSize = sizeof(windowClass),
      .style = 0,
      .lpfnWndProc = DefWindowProcW,
      .cbClsExtra = 0,
      .cbWndExtra = 0,
      .hInstance = selfInstance,
      .hIcon = LoadIconW(NULL, IDI_APPLICATION),
      .hCursor = LoadCursor(NULL, IDC_ARROW),
      .hbrBackground = NULL,
      .lpszMenuName = NULL,
      .lpszClassName = L"main_window",
      .hIconSm = NULL,
  };

  RegisterClassExW(&windowClass);

  HWND window = CreateWindowEx(
      WS_EX_NOREDIRECTIONBITMAP | WS_EX_LAYERED, windowClass.lpszClassName,
      L"DirectComposite Test", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
      CW_USEDEFAULT, 640, 480, NULL, NULL, selfInstance, NULL);

  SetLayeredWindowAttributes(window, 0, 255, LWA_ALPHA);

  CoInitialize(NULL);

  ID3D11Device *device = NULL;
  ID3D11DeviceContext *context = NULL;

  UINT d3dFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG;

  D3D_FEATURE_LEVEL featureLevels[] = {
      D3D_FEATURE_LEVEL_11_0,
  };
  D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, d3dFlags,
                    featureLevels, 1, D3D11_SDK_VERSION, &device, NULL,
                    &context);

  ID3D11RenderTargetView *targetView = NULL;

  DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {
      .Width = 1,
      .Height = 1,
      .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
      .Stereo = FALSE,
      .SampleDesc =
          {
              1,
              0,
          },
      .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
      .BufferCount = 2,
      .Scaling = DXGI_SCALING_STRETCH,
      .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
      .AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED,
      .Flags = 0,
  };

  IDXGIDevice *dxgiDevice;
  device->QueryInterface(__uuidof(dxgiDevice), (void **)&dxgiDevice);

  IDXGIFactory2 *factory = NULL;
  CreateDXGIFactory1(__uuidof(factory), (void **)&factory);

  IDXGISwapChain1 *swapChain;
  factory->CreateSwapChainForComposition(device, &swapChainDesc, NULL,
                                         &swapChain);

  IDCompositionDevice *compositeDevice = NULL;
  DCompositionCreateDevice(dxgiDevice, __uuidof(compositeDevice),
                           (void **)&compositeDevice);

  IDCompositionTarget *compositeTarget = NULL;
  compositeDevice->CreateTargetForHwnd(window, true, &compositeTarget);

  IDCompositionVisual *compositeVisual = NULL;
  compositeDevice->CreateVisual(&compositeVisual);

  compositeVisual->SetContent(swapChain);
  compositeTarget->SetRoot(compositeVisual);
  compositeDevice->Commit();

  ShowWindow(window, SW_SHOWNORMAL);
  UpdateWindow(window);

  int bufferWidth = 0;
  int bufferHeight = 0;

  MSG msg;
  while (should_continue) {
    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) {
        return 0;
      }

      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }

    RECT cr;
    GetClientRect(window, &cr);

    width = cr.right - cr.left;
    height = cr.bottom - cr.top;

    if (bufferWidth != width || bufferHeight != height) {
      bufferWidth = width;
      bufferHeight = height;

      context->ClearState();

      if (targetView) {
        targetView->Release();
        targetView = NULL;
      }

      if (width > 0 && height > 0) {
        swapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_UNKNOWN, 0);

        ID3D11Texture2D *color = NULL;
        swapChain->GetBuffer(0, __uuidof(color), (void **)&color);
        if (color) {
          device->CreateRenderTargetView(color, NULL, &targetView);
        }
        if (color) {
          color->Release();
        }
      }
    }

    if (context && targetView) {
      context->ClearRenderTargetView(targetView, ((FLOAT[4]){0, 0, 0, 0.5}));
    }
    swapChain->Present(1, 0);
  }

  return 0;
}
