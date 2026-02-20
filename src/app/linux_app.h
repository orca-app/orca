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
#include <xcb/xproto.h>

typedef struct x11_win_id_to_handle
{
  u32 winId;
  oc_window handle;
} x11_win_id_to_handle;

typedef struct oc_linux_x11
{
    Display* display;
    struct {
        xcb_atom_t OC_X11_CLIENT_MESSAGE;
        xcb_atom_t UTF8_STRING;
        xcb_atom_t WM_CHANGE_STATE;
        xcb_atom_t WM_DELETE_WINDOW;
        xcb_atom_t WM_PROTOCOLS;
        xcb_atom_t WM_STATE;
        xcb_atom_t _NET_ACTIVE_WINDOW;
        xcb_atom_t _NET_CLOSE_WINDOW;
        xcb_atom_t _NET_FRAME_EXTENTS;
        xcb_atom_t _NET_REQUEST_FRAME_EXTENTS;
        xcb_atom_t _NET_SUPPORTED;
        xcb_atom_t _NET_SUPPORTING_WM_CHECK;
        xcb_atom_t _NET_WM_ICON_NAME;
        xcb_atom_t _NET_WM_NAME;
        xcb_atom_t _NET_WM_PID;
        xcb_atom_t _NET_WM_PING;
        xcb_atom_t _NET_WM_STATE;
        xcb_atom_t _NET_WM_STATE_MAXIMIZED_HORZ;
        xcb_atom_t _NET_WM_STATE_MAXIMIZED_VERT;
        xcb_atom_t _NET_WM_SYNC_REQUEST;
        xcb_atom_t _NET_WM_SYNC_REQUEST_COUNTER;
        xcb_atom_t _NET_WM_USER_TIME;
        xcb_atom_t _NET_WM_USER_TIME_WINDOW;
        xcb_atom_t _NET_WM_WINDOW_TYPE;
        xcb_atom_t _NET_WM_WINDOW_TYPE_NORMAL;
    } atoms;
    u32 rootWinId;
    u32 controlWinId;
    u32 winIdToHandleLen;
    x11_win_id_to_handle winIdToHandle[128];
    u8* wmClass;
    u32 wmClassLen;
    u8* wmClientMachine;
    u32 wmClientMachineLen;
} oc_linux_x11;

typedef struct oc_linux_app_data
{
    oc_arena persistent_arena;
    oc_linux_x11 x11;
} oc_linux_app_data;

typedef enum x11_reponse_type {
    X11_RESPONSE_TYPE_ERROR = 0,
    X11_RESPONSE_TYPE_REPLY = 1,
} x11_reponse_type;

typedef enum x11_window_state
{
    X11_WINDOW_STATE_WITHDRAWN = 0,
    X11_WINDOW_STATE_NORMAL = 1,
    X11_WINDOW_STATE_ICONIC = 3,
} x11_window_state;

typedef enum x11_net_wm_state_action
{
    X11_NET_WM_STATE_REMOVE = 0,
    X11_NET_WM_STATE_ADD = 1,
    X11_NET_WM_STATE_TOGGLE = 2,
} x11_net_wm_state_action;

typedef enum x11_ewmh_source_indication
{
    X11_EWMH_SOURCE_INDICATION_NONE = 0,
    X11_EWMH_SOURCE_INDICATION_NORMAL = 1,
    X11_EWMH_SOURCE_INDICATION_OTHER = 2,
} x11_ewmh_source_indication;

typedef enum oc_linux_window_flags
{
    OC_LINUX_WINDOW_X11_MAPPED = (1 << 0),
    OC_LINUX_WINDOW_X11_MAP_IS_ICONIC = (1 << 1),
    OC_LINUX_WINDOW_X11_REPARENTED = (1 << 2),
} oc_linux_window_flags;

typedef enum oc_linux_window_focus
{
    OC_LINUX_WINDOW_UNFOCUSED,
    OC_LINUX_WINDOW_FOCUSED_IGNORE_INPUTS,
    OC_LINUX_WINDOW_FOCUSED,
} oc_linux_window_focus;

typedef struct oc_linux_window_data
{
    u32 x11Id;
    x11_window_state state;
    xcb_atom_t netState[32];
    u32 netStateLen;
    xcb_timestamp_t netWmUserTime;
    oc_linux_window_flags flags;
    oc_linux_window_focus focus;
    /* Left-outer corner from the parent's origin. */
    oc_vec2 posFromParent;
    /* x and y are relative to the root window's origin. */
    oc_rect rect;
    /* Frame widths added by window manager. */
    f32 frameLeft, frameRight, frameTop, frameBottom;
    u32 netWmSyncRequestCounterId;
    u64 netWmSyncRequestUpdateValue;
} oc_linux_window_data;

#define OC_PLATFORM_WINDOW_DATA oc_linux_window_data linux;
#define OC_PLATFORM_APP_DATA oc_linux_app_data linux;

typedef struct oc_layer
{
    u32 x11WinId;
} oc_layer;

typedef enum oc_x11_client_message
{
    OC_X11_CLIENT_MESSAGE_UNFOCUS,
    OC_X11_CLIENT_MESSAGE_CANCEL_CLOSE,
    OC_X11_CLIENT_MESSAGE_REQUEST_QUIT,
    OC_X11_CLIENT_MESSAGE_CANCEL_QUIT,
    OC_X11_CLIENT_MESSAGE_ADD_WINDOW_ID_TO_HANDLE_ENTRY,
    OC_X11_CLIENT_MESSAGE_DISPATCH_ON_MAIN_THREAD_SYNC,
    OC_X11_CLIENT_MESSAGE_GET_PROPERTY,
} oc_x11_client_message;

typedef struct oc_linux_dispatch_sync_request
{
    oc_dispatch_proc proc;
    void* user;
    oc_condition* cond;
    oc_mutex* mutex;
    i32 retval;
} oc_linux_dispatch_sync_request;

#endif // __LINUX_APP_H_

