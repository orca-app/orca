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

#include"platform.h"
#include"ringbuffer.h"

#if defined(OS_WIN64) || defined(OS_WIN32)
	#include"win32_app.h"
#elif defined(OS_MACOS)
	#include"osx_app.h"
#else
	#error "platform not supported yet"
#endif

//---------------------------------------------------------------
// Input State
//---------------------------------------------------------------

typedef struct mp_key_utf8
{
	u8 labelLen;
	char label[8];
} mp_key_utf8;

typedef struct mp_key_state
{
	u64 lastUpdate;
	u32 transitionCounter;
	bool down;
	bool clicked;
	bool doubleClicked;

} mp_key_state;

typedef struct mp_keyboard_state
{
	mp_key_state keys[MP_KEY_COUNT];
	mp_key_mods  mods;
} mp_keyboard_state;

typedef struct mp_mouse_state
{
	u64 lastUpdate;
	bool posValid;
	vec2 pos;
	vec2 delta;
	vec2 wheel;

	union
	{
		mp_key_state buttons[MP_MOUSE_BUTTON_COUNT];
		struct
		{
			mp_key_state left;
			mp_key_state right;
			mp_key_state middle;
			mp_key_state ext1;
			mp_key_state ext2;
		};
	};
} mp_mouse_state;

enum { MP_INPUT_TEXT_BACKING_SIZE = 64 };

typedef struct mp_text_state
{
	u64 lastUpdate;
	utf32 backing[MP_INPUT_TEXT_BACKING_SIZE];
	str32 codePoints;
} mp_text_state;

typedef struct mp_input_state
{
	u64 frameCounter;
	mp_keyboard_state keyboard;
	mp_mouse_state	mouse;
	mp_text_state text;
} mp_input_state;

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

	mp_rect contentRect;
	mp_rect frameRect;
	mp_window_style	style;

	bool shouldClose; //TODO could be in status flags
	bool hidden;
	bool minimized;

	MP_PLATFORM_WINDOW_DATA
} mp_window_data;


//---------------------------------------------------------------
// Global App State
//---------------------------------------------------------------

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

	mp_input_state inputState;

	mp_key_utf8 keyLabels[512];
	int keyCodes[512];
	int nativeKeys[MP_KEY_COUNT];

	MP_PLATFORM_APP_DATA
} mp_app;


#endif // __MP_APP_INTERNAL_H_
