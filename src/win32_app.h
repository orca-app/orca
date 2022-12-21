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

typedef struct mp_window_data
{
	list_elt freeListElt;
	u32 generation;

	HWND windowHandle;
} mp_window_data;

mp_window_data* mp_window_ptr_from_handle(mp_window handle);
//mp_view_data* mp_view_ptr_from_handle(mp_view handle);

#endif __WIN32_APP_H_
