/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#ifndef __LINUX_APP_H_
#define __LINUX_APP_H_

#include "app.h"

// We use libx11 here purely to get a Display structure. EGL's
// EGL_PLATFORM_XCB_EXT is actually quite recent, so not as widely supported as
// EGL_PLATFORM_X11_EXT. All other interactions with X11 go through the
// underlying XCB socket, be it from Orca or from Mesa's libegl.
#include <X11/Xlib.h>

typedef struct oc_linux_x11
{
    Display* display;
} oc_linux_x11;

typedef struct oc_linux_app_data
{
    oc_linux_x11 x11;
} oc_linux_app_data;

typedef struct oc_linux_window_data
{
    u32 x11_id;
} oc_linux_window_data;

#define OC_PLATFORM_WINDOW_DATA oc_linux_window_data linux;
#define OC_PLATFORM_APP_DATA oc_linux_app_data linux;

typedef struct oc_layer
{
    u32 x11_win_id;
} oc_layer;

#endif // __LINUX_APP_H_

