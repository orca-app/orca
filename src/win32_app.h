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

#define WIN32_LEAN_AND_MEAN
#include<windows.h>
#include"mp_app.h"

typedef struct win32_window_data
{
	HWND hWnd;

} win32_window_data;

#define MP_PLATFORM_WINDOW_DATA win32_window_data win32;

typedef struct win32_app_data
{
	u32 savedConsoleCodePage;

	int mouseCaptureMask;
	bool mouseTracked;

} win32_app_data;

#define MP_PLATFORM_APP_DATA win32_app_data win32;

#endif __WIN32_APP_H_
