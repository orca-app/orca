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

typedef struct oc_linux_dispatch_sync_result
{
    bool didRun;
    i32 retVal;
} oc_linux_dispatch_sync_result;
typedef struct oc_linux_dispatch_sync_request
{
    oc_dispatch_proc proc;
    void* user;
    oc_condition* cond;
    oc_mutex* mutex;
    u64 refcount;
    u64 reqId;
    oc_linux_dispatch_sync_result result;
} oc_linux_dispatch_sync_request;

typedef struct x11_win_id_to_handle
{
  xcb_window_t winId;
  oc_window handle;
} x11_win_id_to_handle;

typedef struct oc_linux_x11
{
    Display* display;
    struct {
        /* name, required */
        #define ATOM_LIST(V)  \
            V(OC_X11_CLIENT_MESSAGE, 0)  \
            V(UTF8_STRING, 0)  \
            V(WM_CHANGE_STATE, 0)  \
            V(WM_DELETE_WINDOW, 0)  \
            V(WM_PROTOCOLS, 0)  \
            V(WM_STATE, 0)  \
            V(_MOTIF_WM_HINTS, 0)  \
            V(_NET_ACTIVE_WINDOW, 1)  \
            V(_NET_CLOSE_WINDOW, 1)  \
            V(_NET_FRAME_EXTENTS, 1)  \
            V(_NET_NUMBER_OF_DESKTOPS, 1)  \
            V(_NET_REQUEST_FRAME_EXTENTS, 1)  \
            V(_NET_SUPPORTED, 0)  \
            V(_NET_SUPPORTING_WM_CHECK, 1)  \
            V(_NET_WM_DESKTOP, 1)  \
            V(_NET_WM_ICON_NAME, 1)  \
            V(_NET_WM_NAME, 1)  \
            V(_NET_WM_PID, 1)  \
            V(_NET_WM_PING, 1)  \
            V(_NET_WM_STATE, 1)  \
            V(_NET_WM_STATE_ABOVE, 1)  \
            V(_NET_WM_STATE_MAXIMIZED_HORZ, 1)  \
            V(_NET_WM_STATE_MAXIMIZED_VERT, 1)  \
            V(_NET_WM_SYNC_REQUEST, 1)  \
            V(_NET_WM_SYNC_REQUEST_COUNTER, 1)  \
            V(_NET_WM_USER_TIME, 1)  \
            V(_NET_WM_USER_TIME_WINDOW, 0)  \
            V(_NET_WM_WINDOW_TYPE, 1)  \
            V(_NET_WM_WINDOW_TYPE_NORMAL, 1)  \
            V(_NET_WORKAREA, 1)  \

        #define DECL_ATOM(name, required)  xcb_atom_t name;
        ATOM_LIST(DECL_ATOM)
        #undef DECL_ATOM
    } atoms;
    xcb_window_t rootWinId;
    xcb_window_t controlWinId;
    u32 winIdToHandleLen;
    x11_win_id_to_handle winIdToHandle[128];
    u8* wmClass;
    u32 wmClassLen;
    u8* wmClientMachine;
    u32 wmClientMachineLen;
    u32 netNumberOfDesktops;
    oc_rect netWorkarea[16];
    u64 netWorkareaLen;
} oc_linux_x11;

typedef enum oc_x11_client_message
{
  OC_X11_CLIENT_MESSAGE_REQUEST_QUIT,
  OC_X11_CLIENT_MESSAGE_CANCEL_QUIT,
  OC_X11_CLIENT_MESSAGE_WINDOW_DESTROY,
  OC_X11_CLIENT_MESSAGE_WINDOW_REQUEST_CLOSE,
  OC_X11_CLIENT_MESSAGE_WINDOW_CANCEL_CLOSE,
  OC_X11_CLIENT_MESSAGE_WINDOW_SET_TITLE,
  OC_X11_CLIENT_MESSAGE_WINDOW_SHOW,
  OC_X11_CLIENT_MESSAGE_WINDOW_HIDE,
  OC_X11_CLIENT_MESSAGE_WINDOW_MINIMIZE,
  OC_X11_CLIENT_MESSAGE_WINDOW_MAXIMIZE,
  OC_X11_CLIENT_MESSAGE_WINDOW_RESTORE,
  OC_X11_CLIENT_MESSAGE_WINDOW_FOCUS,
  OC_X11_CLIENT_MESSAGE_WINDOW_UNFOCUS,
  OC_X11_CLIENT_MESSAGE_WINDOW_SEND_TO_BACK,
  OC_X11_CLIENT_MESSAGE_WINDOW_BRING_TO_FRONT,
  OC_X11_CLIENT_MESSAGE_WINDOW_SET_FRAME_RECT,
  OC_X11_CLIENT_MESSAGE_WINDOW_SET_CONTENT_RECT,
  OC_X11_CLIENT_MESSAGE_WINDOW_CENTER,
  OC_X11_CLIENT_MESSAGE_DISPATCH_ON_MAIN_THREAD_SYNC,
  OC_X11_CLIENT_MESSAGE_GET_PROPERTY,
  OC_X11_CLIENT_MESSAGE_TRANSLATE_COORDINATES_TO_ROOT,

  OC_X11_CLIENT_MESSAGE_MAX,
} oc_x11_client_message;

typedef struct oc_linux_app_cmd_user
{
    oc_list_elt listElt;
    union {
        struct { oc_str8 title; } setTitle;
        struct { oc_rect rect; } setFrameRect;
        struct { oc_rect rect; } setContentRect;
        struct { oc_vec2 contentWh; } center;
        struct { oc_linux_dispatch_sync_request* req; u64 reqId; } dispatchOnMainThreadSync;
        struct { xcb_atom_t prop; xcb_get_property_cookie_t cookie; } getProperty;
        struct { xcb_translate_coordinates_cookie_t cookie; u16 since; } translateCoordinatesToRoot;
    };
} oc_linux_app_cmd_user;
typedef struct oc_linux_app_cmd
{
    oc_x11_client_message cmd;
    oc_window window;
    oc_linux_app_cmd_user user;
} oc_linux_app_cmd;

typedef struct oc_linux_app_data
{
    oc_arena persistentArena;
    u64 mainThreadId;
    oc_mutex* windowPoolMutex;
    oc_linux_x11 x11;
    oc_pool appCmdUserPool;
    oc_mutex* appCmdUserPoolMutex;
    oc_condition* pumpedEventsCond;
    oc_mutex* pumpedEventsMutex;
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

typedef enum x11_wm_hints_flags
{
    X11_WM_HINTS_INPUT_HINT = (1 << 0),
    X11_WM_HINTS_STATE_HINT = (1 << 1),
    X11_WM_HINTS_ICON_PIXMAP_HINT = (1 << 2),
    X11_WM_HINTS_ICON_WINDOW_HINT = (1 << 3),
    X11_WM_HINTS_ICON_POSITION_HINT = (1 << 4),
    X11_WM_HINTS_ICON_MASK_HINT = (1 << 5),
    X11_WM_HINTS_WINDOW_GROUP_HINT = (1 << 6),
    //X11_WM_HINTS_MESSAGE_HINT = (1 << 7),
    X11_WM_HINTS_URGENCY_HINT = (1 << 8),
} x11_wm_hints_flags;

typedef struct x11_wm_hints
{
    x11_wm_hints_flags flags;
    u32 input;
    x11_window_state initialState;
    xcb_pixmap_t iconPixmap;
    xcb_window_t iconWindow;
    u32 icon_x;
    u32 icon_y;
    xcb_pixmap_t iconMask;
    xcb_window_t windowGroup;
} x11_wm_hints;

typedef enum x11_wm_normal_hints_flags
{
    X11_WM_NORMAL_HINTS_USPOSITION = (1 << 0),
    X11_WM_NORMAL_HINTS_USSIZE = (1 << 1),
    X11_WM_NORMAL_HINTS_PPOSITION = (1 << 2),
    X11_WM_NORMAL_HINTS_PSIZE = (1 << 3),
    X11_WM_NORMAL_HINTS_PMINSIZE = (1 << 4),
    X11_WM_NORMAL_HINTS_PMAXSIZE = (1 << 5),
    X11_WM_NORMAL_HINTS_PRESIZEINC = (1 << 6),
    X11_WM_NORMAL_HINTS_PASPECT = (1 << 7),
    X11_WM_NORMAL_HINTS_PBASESIZE = (1 << 8),
    X11_WM_NORMAL_HINTS_PWINGRAVITY = (1 << 9),
} x11_wm_normal_hints_flags;

typedef struct x11_wm_normal_hints
{
    u32 flags;
    u32 pad[4];
    i32 min_width;
    i32 min_height;
    i32 max_width;
    i32 max_height;
    i32 width_inc;
    i32 height_inc;
    i32 min_aspect[2];
    i32 max_aspect[2];
    i32 base_width;
    i32 base_height;
    i32 win_gravity;
} x11_wm_normal_hints;

typedef enum x11_motif_wm_hints_flags
{
    X11_MOTIF_WM_HINTS_FUNCTIONS = (1 << 0),
    X11_MOTIF_WM_HINTS_DECORATIONS = (1 << 1),
} x11_motif_wm_hints_flags;

typedef enum x11_motif_wm_hints_functions
{
    X11_MOTIF_WM_HINTS_FUNC_ALL = (1 << 0),
    #if 0
    X11_MOTIF_WM_HINTS_FUNC_RESIZE = (1 << 1),
    X11_MOTIF_WM_HINTS_FUNC_MOVE = (1 << 2),
    X11_MOTIF_WM_HINTS_FUNC_ICONIFY = (1 << 3),
    X11_MOTIF_WM_HINTS_FUNC_MAXIMIZE = (1 << 4),
    X11_MOTIF_WM_HINTS_FUNC_CLOSE = (1 << 5),
    #endif
} x11_motif_wm_hints_functions;

typedef enum x11_motif_wm_hints_decorations
{
    X11_MOTIF_WM_HINTS_DECOR_ALL = (1 << 0),
    X11_MOTIF_WM_HINTS_DECOR_BORDER = (1 << 1),
    #if 0
    X11_MOTIF_WM_HINTS_DECOR_HANDLE = (1 << 2),
    X11_MOTIF_WM_HINTS_DECOR_TITLE = (1 << 3),
    X11_MOTIF_WM_HINTS_DECOR_MENU = (1 << 4),
    X11_MOTIF_WM_HINTS_DECOR_ICONIFY = (1 << 5),
    X11_MOTIF_WM_HINTS_DECOR_MAXIMIZE = (1 << 6),
    #endif
} x11_motif_wm_hints_decorations;

typedef struct x11_motif_wm_hints
{
    u32 flags;
    u32 functions;
    u32 decorations;
    #if 0
    i32 input_mode;
    u32 status;
    #endif
} x11_motif_wm_hints;

typedef struct x11_wm_state
{
    x11_window_state state;
    xcb_window_t icon;
} x11_wm_state;

typedef enum oc_linux_window_flags
{
    OC_LINUX_WINDOW_X11_REPARENTED = (1 << 0),
    OC_LINUX_WINDOW_X11_POS_KNOWN = (1 << 1),
    OC_LINUX_WINDOW_X11_FRAME_EXTENTS = (1 << 2),
    OC_LINUX_WINDOW_EMIT_EVENTS = (1 << 3),
} oc_linux_window_flags;

typedef enum oc_linux_window_focus
{
    OC_LINUX_WINDOW_UNFOCUSED,
    OC_LINUX_WINDOW_FOCUSED_IGNORE_INPUTS,
    OC_LINUX_WINDOW_FOCUSED,
} oc_linux_window_focus;

typedef struct oc_linux_window_data
{
    xcb_window_t x11Id;
    x11_window_state state;
    xcb_atom_t netState[32];
    u32 netStateLen;
    xcb_timestamp_t netWmUserTime;
    oc_linux_window_flags flags;
    oc_linux_window_focus focus;
    /* Top left-outer corner from the parent's origin. */
    oc_vec2 posFromParent;
    /* x and y are the top left-inner corner relative to the root window's
     * origin. */
    oc_rect rect;
    u16 rectSince;
    u16 rectNext;
    /* Frame widths added by window manager. */
    f32 frameLeft, frameRight, frameTop, frameBottom;
    u32 netWmSyncRequestCounterId;
    u64 netWmSyncRequestUpdateValue;
    u32 netWmDesktop;
    xcb_generic_event_t* pendingConfigureNotify;
} oc_linux_window_data;

#define OC_PLATFORM_WINDOW_DATA oc_linux_window_data linux;
#define OC_PLATFORM_APP_DATA oc_linux_app_data linux;

typedef struct oc_layer
{
    u32 x11WinId;
} oc_layer;

#endif // __LINUX_APP_H_

