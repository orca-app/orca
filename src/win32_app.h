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

#include"mp_app.h"

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#include<windows.h>


typedef struct win32_window_data
{
	HWND hWnd;
	list_info layers;
} win32_window_data;

typedef struct mp_window_data mp_window_data;
typedef struct mp_layer
{
	mp_window_data* parent;
	list_elt listElt;
	HWND hWnd;
} mp_layer;

#define MP_PLATFORM_WINDOW_DATA win32_window_data win32;

typedef struct win32_app_data
{
	u32 savedConsoleCodePage;

	int mouseCaptureMask;
	bool mouseTracked;
	vec2 lastMousePos;

} win32_app_data;

#define MP_PLATFORM_APP_DATA win32_app_data win32;

enum MP_WM_USER
{
	MP_WM_USER_DISPATCH_PROC = 0x0400, // WM_USER messages are defined from 0x400 to 0x7FFF
};

#endif __WIN32_APP_H_
