/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#ifndef __WIN32_APP_H_
#define __WIN32_APP_H_

#include "app.h"

#define WIN32_LEAN_AND_MEAN
#define COBJMACROS
#define UNICODE
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_3.h>
#include "win32_dcomp_c_api.h"

typedef struct oc_win32_window_data
{
    HWND hWnd;

    IDCompositionTarget* dcompTarget;
    IDCompositionVisual* dcompRootVisual;
    oc_list surfaces;
} oc_win32_window_data;

typedef struct oc_window_data oc_window_data;

#define OC_PLATFORM_WINDOW_DATA oc_win32_window_data win32;

typedef struct oc_win32_app_data
{
    u32 savedConsoleCodePage;

    int mouseCaptureMask;
    bool mouseTracked;
    oc_vec2 lastMousePos;
    u32 lastClickTime[OC_MOUSE_BUTTON_COUNT];
    u32 clickCount[OC_MOUSE_BUTTON_COUNT];
    u32 wheelScrollLines;

    DWORD mainThreadID;

    IDCompositionDevice* dcompDevice;

} oc_win32_app_data;

#define OC_PLATFORM_APP_DATA oc_win32_app_data win32;

enum OC_WM_USER
{
    OC_WM_USER_DISPATCH_PROC = 0x0400, // WM_USER messages are defined from 0x400 to 0x7FFF
    OC_WM_USER_WAKEUP = 0x0401,
};

#endif __WIN32_APP_H_
