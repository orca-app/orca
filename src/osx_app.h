/************************************************************//**
*
*	@file: osx_app.h
*	@author: Martin Fouilleul
*	@date: 12/02/2021
*	@revision:
*
*****************************************************************/
#ifndef __OSX_APP_H_
#define __OSX_APP_H_

#include"mp_app.h"
#include"graphics.h"

#ifdef __OBJC__
	#import<Cocoa/Cocoa.h>
#else
	#define NSWindow void
	#define NSView void
	#define NSObject void
	#define NSTimer void
	#define NSCursor void
#endif

#include<Carbon/Carbon.h>

typedef struct osx_window_data
{
	NSWindow* nsWindow;
	NSView*   nsView;
	NSObject* nsWindowDelegate;

} osx_window_data;

#define MP_PLATFORM_WINDOW_DATA osx_window_data osx;

const u32 MP_APP_MAX_VIEWS = 128;

typedef struct osx_app_data
{
	NSTimer* frameTimer;
	NSCursor* cursor;

	TISInputSourceRef kbLayoutInputSource;
	void* kbLayoutUnicodeData;
	id kbLayoutListener;

} osx_app_data;

#define MP_PLATFORM_APP_DATA osx_app_data osx;



#endif //__OSX_APP_H_
