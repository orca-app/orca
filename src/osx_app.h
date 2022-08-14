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

#import<Cocoa/Cocoa.h>
#import<Carbon/Carbon.h>
#include"mp_app.h"
#include"graphics.h"

struct mp_window_data
{
	list_elt freeListElt;
	u32 generation;

	NSWindow* nsWindow;
	NSView*   nsView;
	NSObject* nsWindowDelegate;

	mp_rect contentRect;
	mp_rect frameRect;
	mp_window_style	style;

	bool shouldClose; //TODO could be in status flags
	bool hidden;

	mp_view mainView;
};

struct mp_view_data
{
	list_elt freeListElt;
	u32 generation;

	mp_window window;
	NSView*   nsView;
	mg_surface surface;
};

@interface MPNativeWindow : NSWindow
{
	mp_window_data* mpWindow;
}
- (id)initWithMPWindow:(mp_window_data*) window contentRect:(NSRect) rect styleMask:(uint32) style;
@end

mp_window_data* mp_window_ptr_from_handle(mp_window handle);
mp_view_data* mp_view_ptr_from_handle(mp_view handle);



#endif //__OSX_APP_H_
