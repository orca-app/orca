/************************************************************/ /**
*
*	@file: app_internal.h
*	@author: Martin Fouilleul
*	@date: 23/12/2022
*	@revision:
*
*****************************************************************/
#ifndef __APP_INTERNAL_H_
#define __APP_INTERNAL_H_

#include "app.h"

#include "platform/platform.h"
#include "util/ringbuffer.h"

#if OC_PLATFORM_WINDOWS
    #include "win32_app.h"
#elif OC_PLATFORM_MACOS
    #include "osx_app.h"
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

typedef struct oc_key_utf8
{
    u8 labelLen;
    char label[8];
} oc_key_utf8;

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

    oc_live_resize_callback liveResizeCallback;
    void* liveResizeData;

    oc_key_utf8 keyLabels[512];
    int keyCodes[512];
    int nativeKeys[OC_KEY_COUNT];

    OC_PLATFORM_APP_DATA
} oc_app;

#endif // __APP_INTERNAL_H_
