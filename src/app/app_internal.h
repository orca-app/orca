/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "app.h"

#include "platform/platform.h"
#include "platform/platform_io_internal.h"
#include "util/ringbuffer.h"

#if OC_PLATFORM_WINDOWS
    #include "win32_app.h"
#elif OC_PLATFORM_MACOS
    #include "osx_app.h"
#elif OC_PLATFORM_LINUX
    #include "linux_app.h"
#else
    #error "platform not supported yet"
#endif

//---------------------------------------------------------------
// Window structure
//---------------------------------------------------------------

typedef struct oc_frame_stats
{
    f64 start;
    f64 workTime;
    f64 remainingTime;
    f64 targetFramePeriod;
} oc_frame_stats;

typedef struct oc_window_data
{
    oc_list_elt freeListElt;
    u32 generation;

    oc_window_style style;

    bool shouldClose; //TODO could be in status flags
    bool hidden;
    bool minimized;

    OC_PLATFORM_WINDOW_DATA
} oc_window_data;

//---------------------------------------------------------------
// Global App State
//---------------------------------------------------------------

enum
{
    OC_APP_MAX_WINDOWS = 128
};

typedef struct oc_app
{
    bool init;
    bool shouldQuit;
    bool minimized;

    oc_str8 pendingPathDrop;
    oc_arena eventArena;

    oc_ringbuffer eventQueue;

    oc_frame_stats frameStats;

    oc_window_data windowPool[OC_APP_MAX_WINDOWS];
    oc_list windowFreeList;

    oc_scan_code scanCodes[512];           // native virtual key code to oc_scan_code
    oc_key_code keyMap[OC_SCANCODE_COUNT]; // oc_scan_code to oc_key_code, as per current keyboard layout

    OC_PLATFORM_APP_DATA
} oc_app;

extern oc_key_code oc_defaultKeyMap[OC_SCANCODE_COUNT];
