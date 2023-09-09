//*****************************************************************
//
//	$file: win32_app.h $
//	$author: Martin Fouilleul $
//	$date: 20/12/2022 $
//	$revision: $
//	$note: (C) 2022 by Martin Fouilleul - all rights reserved $
//
//*****************************************************************
#ifndef __WIN32_APP_H_
#define __WIN32_APP_H_

#include "app.h"

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#include <windows.h>

typedef struct oc_win32_window_data
{
    HWND hWnd;
    oc_list layers;
} oc_win32_window_data;

typedef struct oc_window_data oc_window_data;

typedef struct oc_layer
{
    oc_window_data* parent;
    oc_list_elt listElt;
    HWND hWnd;
} oc_layer;

#define OC_PLATFORM_WINDOW_DATA oc_win32_window_data win32;

typedef struct oc_win32_app_data
{
    u32 savedConsoleCodePage;

    int mouseCaptureMask;
    bool mouseTracked;
    oc_vec2 lastMousePos;
    u32 wheelScrollLines;

} oc_win32_app_data;

#define OC_PLATFORM_APP_DATA oc_win32_app_data win32;

enum OC_WM_USER
{
    OC_WM_USER_DISPATCH_PROC = 0x0400, // WM_USER messages are defined from 0x400 to 0x7FFF
};

#endif __WIN32_APP_H_
