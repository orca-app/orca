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
	#define CALayer void
	#define CAContext void
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

//-----------------------------------------------
// Surface layer
//-----------------------------------------------
#ifdef __OBJC__
	//NOTE: these private interfaces for surface sharing need to be declared explicitly here
	typedef uint32_t CGSConnectionID;
	CGSConnectionID CGSMainConnectionID(void);

	typedef uint32_t CAContextID;

	@interface CAContext : NSObject
	{
	}
	+ (id)contextWithCGSConnection:(CAContextID)contextId options:(NSDictionary*)optionsDict;
	@property(readonly) CAContextID contextId;
	@property(retain) CALayer *layer;
	@end

	@interface CALayerHost : CALayer
	{
	}
	@property CAContextID contextId;
	@end
#endif

typedef struct mp_layer
{
	CALayer* caLayer;
	CAContext* caContext;
} mp_layer;



#endif //__OSX_APP_H_
