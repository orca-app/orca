/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#ifndef __OSX_APP_H_
#define __OSX_APP_H_

#include "app.h"
#include "graphics/graphics.h"

#import <Cocoa/Cocoa.h>
#include <Carbon/Carbon.h>

typedef struct oc_osx_window_data
{
    NSWindow* nsWindow;
    NSView* nsView;
    NSObject* nsWindowDelegate;

    oc_list surfaces;

} oc_osx_window_data;

#define OC_PLATFORM_WINDOW_DATA oc_osx_window_data osx;

const u32 OC_APP_MAX_VIEWS = 128;

typedef struct oc_osx_app_data
{
    NSTimer* frameTimer;
    NSCursor* cursor;
    id kbLayoutListener;

} oc_osx_app_data;

#define OC_PLATFORM_APP_DATA oc_osx_app_data osx;

#endif //__OSX_APP_H_
