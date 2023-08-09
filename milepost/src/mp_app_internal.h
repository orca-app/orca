/************************************************************//**
*
*	@file: mp_app_internal.h
*	@author: Martin Fouilleul
*	@date: 23/12/2022
*	@revision:
*
*****************************************************************/
#ifndef __MP_APP_INTERNAL_H_
#define __MP_APP_INTERNAL_H_

#include"mp_app.h"

#include"platform.h"
#include"ringbuffer.h"

#if PLATFORM_WINDOWS
	#include"win32_app.h"
#elif PLATFORM_MACOS
	#include"osx_app.h"
#else
	#error "platform not supported yet"
#endif

//---------------------------------------------------------------
// Window structure
//---------------------------------------------------------------

typedef struct mp_frame_stats
{
	f64 start;
	f64 workTime;
	f64 remainingTime;
	f64 targetFramePeriod;
} mp_frame_stats;

typedef struct mp_window_data
{
	list_elt freeListElt;
	u32 generation;

	mp_window_style	style;

	bool shouldClose; //TODO could be in status flags
	bool hidden;
	bool minimized;

	MP_PLATFORM_WINDOW_DATA
} mp_window_data;


//---------------------------------------------------------------
// Global App State
//---------------------------------------------------------------

typedef struct mp_key_utf8
{
	u8 labelLen;
	char label[8];
} mp_key_utf8;


enum { MP_APP_MAX_WINDOWS = 128 };

typedef struct mp_app
{
	bool init;
	bool shouldQuit;
	bool minimized;

	str8 pendingPathDrop;
	mem_arena eventArena;

	ringbuffer eventQueue;

	mp_frame_stats frameStats;

	mp_window_data windowPool[MP_APP_MAX_WINDOWS];
	list_info windowFreeList;

	mp_live_resize_callback liveResizeCallback;
	void* liveResizeData;

	mp_key_utf8 keyLabels[512];
	int keyCodes[512];
	int nativeKeys[MP_KEY_COUNT];

	MP_PLATFORM_APP_DATA
} mp_app;

#endif // __MP_APP_INTERNAL_H_
