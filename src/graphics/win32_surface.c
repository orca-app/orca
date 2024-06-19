/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include <dwmapi.h>
#include "win32_surface.h"
#include "surface.h"

////////////////////////////////////////////////////////////////////////////////
//TODO: check that dcomp surface work well with dpi changes in parent window
////////////////////////////////////////////////////////////////////////////////

oc_vec2 oc_surface_base_get_size(oc_surface_base* surface)
{
    oc_vec2 size = { 0 };
    RECT rect;
    if(GetClientRect(surface->view.hWnd, &rect))
    {
        u32 dpi = GetDpiForWindow(surface->view.hWnd);
        f32 scale = (float)dpi / 96.;
        size = (oc_vec2){ (rect.right - rect.left) / scale, (rect.bottom - rect.top) / scale };
    }
    return (size);
}

oc_vec2 oc_surface_base_contents_scaling(oc_surface_base* surface)
{
    u32 dpi = GetDpiForWindow(surface->view.hWnd);
    oc_vec2 contentsScaling = (oc_vec2){ (float)dpi / 96., (float)dpi / 96. };
    return (contentsScaling);
}

void oc_surface_base_bring_to_front(oc_surface_base* surface)
{
    oc_list_remove(&surface->view.parent->win32.surfaces, &surface->view.listElt);
    oc_list_push_front(&surface->view.parent->win32.surfaces, &surface->view.listElt);

    IDCompositionVisual* rootVisual = surface->view.parent->win32.dcompRootVisual;
    rootVisual->lpVtbl->RemoveVisual(rootVisual, surface->view.dcompVisual);
    rootVisual->lpVtbl->AddVisual(rootVisual, surface->view.dcompVisual, FALSE, NULL);

    IDCompositionDevice* device = oc_appData.win32.dcompDevice;
    device->lpVtbl->Commit(device);
}

void oc_surface_base_send_to_back(oc_surface_base* surface)
{
    oc_list_remove(&surface->view.parent->win32.surfaces, &surface->view.listElt);
    oc_list_push_back(&surface->view.parent->win32.surfaces, &surface->view.listElt);

    IDCompositionVisual* rootVisual = surface->view.parent->win32.dcompRootVisual;
    rootVisual->lpVtbl->RemoveVisual(rootVisual, surface->view.dcompVisual);
    rootVisual->lpVtbl->AddVisual(rootVisual, surface->view.dcompVisual, TRUE, NULL);

    IDCompositionDevice* device = oc_appData.win32.dcompDevice;
    device->lpVtbl->Commit(device);
}

bool oc_surface_base_get_hidden(oc_surface_base* surface)
{
    bool hidden = !IsWindowVisible(surface->view.hWnd);
    return (hidden);
}

void oc_surface_base_set_hidden(oc_surface_base* surface, bool hidden)
{
    ////////////////////////////////////////////////////////////////////////////////
    //TODO: check that it works well with dcomp??
    ////////////////////////////////////////////////////////////////////////////////
    ShowWindow(surface->view.hWnd, hidden ? SW_HIDE : SW_NORMAL);
}

void oc_surface_base_cleanup(oc_surface_base* surface)
{
    oc_list_remove(&surface->view.parent->win32.surfaces, &surface->view.listElt);
    DestroyWindow(surface->view.hWnd);
}

LRESULT oc_win32_surface_window_proc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    if(message == WM_NCHITTEST)
    {
        return (HTTRANSPARENT);
    }
    else
    {
        return (DefWindowProc(windowHandle, message, wParam, lParam));
    }
}

void oc_surface_base_init_for_window(oc_surface_base* surface, oc_window_data* window)
{
    surface->getSize = oc_surface_base_get_size;
    surface->contentsScaling = oc_surface_base_contents_scaling;
    surface->bringToFront = oc_surface_base_bring_to_front;
    surface->sendToBack = oc_surface_base_send_to_back;
    surface->getHidden = oc_surface_base_get_hidden;
    surface->setHidden = oc_surface_base_set_hidden;

    //NOTE(martin): create a hidden window for the surface
    ///////////////////////////////////////////////////////////////////
    //TODO: surface-api-rework: register this only once
    ///////////////////////////////////////////////////////////////////
    WNDCLASS viewWindowClass = { .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
                                 .lpfnWndProc = oc_win32_surface_window_proc,
                                 .hInstance = GetModuleHandleW(NULL),
                                 .lpszClassName = "view_window_class",
                                 .hCursor = LoadCursor(0, IDC_ARROW) };

    RegisterClass(&viewWindowClass);

    RECT parentRect;
    GetClientRect(window->win32.hWnd, &parentRect);
    int clientWidth = parentRect.right - parentRect.left;
    int clientHeight = parentRect.bottom - parentRect.top;

    surface->view.hWnd = CreateWindowEx(
        WS_EX_TOOLWINDOW,
        viewWindowClass.lpszClassName,
        "view_window",
        WS_POPUP,
        0, 0, clientWidth, clientHeight,
        NULL,
        0,
        viewWindowClass.hInstance,
        0);

    //NOTE: make the window transparent
    {
        HRGN region = CreateRectRgn(0, 0, -1, -1);

        DWM_BLURBEHIND bb = { 0 };
        bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
        bb.hRgnBlur = region;
        bb.fEnable = TRUE;

        HRESULT res = DwmEnableBlurBehindWindow(surface->view.hWnd, &bb);

        DeleteObject(region);
        if(res != S_OK)
        {
            oc_log_error("couldn't enable DWM blur behind window\n");
        }
    }

    //NOTE: cloak and "show" the window
    BOOL cloaked = TRUE;
    DwmSetWindowAttribute(surface->view.hWnd, DWMWA_CLOAK, &cloaked, sizeof(cloaked));
    ShowWindow(surface->view.hWnd, SW_SHOW);

    //NOTE: create composition visual and inserts it in parent's window root visual
    {
        HRESULT hr = 0;
#define TRY_HR(expr)   \
    hr = expr;         \
    if(hr != S_OK)     \
    {                  \
        goto dcompEnd; \
    }

        IDCompositionDevice* device = oc_appData.win32.dcompDevice;
        TRY_HR((device->lpVtbl->CreateVisual(device, &surface->view.dcompVisual)));

        IUnknown* dcompSurface = 0;
        TRY_HR((device->lpVtbl->CreateSurfaceFromHwnd(device, surface->view.hWnd, &dcompSurface)));

        TRY_HR((surface->view.dcompVisual->lpVtbl->SetContent(surface->view.dcompVisual, dcompSurface)));

        TRY_HR((window->win32.dcompRootVisual->lpVtbl->AddVisual(
            window->win32.dcompRootVisual,
            surface->view.dcompVisual,
            FALSE,
            NULL)));

        TRY_HR((device->lpVtbl->Commit(device)));

        dcompSurface->lpVtbl->Release(dcompSurface);
#undef TRY_HR

    dcompEnd:
        if(hr != S_OK)
        {
            oc_log_error("Failed to initialize DirectComposition for surface");
        }
    }

    //NOTE(reuben): Creating the child window takes focus away from the main window, but we want to keep
    //the focus on the main window
    //TODO(martin): if the main window wasn't focused we probably don't want that either
    SetFocus(window->win32.hWnd);

    surface->view.parent = window;
    oc_list_push_front(&window->win32.surfaces, &surface->view.listElt);
}
