/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "app.c"
#include "platform/platform_thread.h"
#include "graphics/graphics.h"

#include <errno.h>
#include <poll.h>
#include <X11/Xlib-xcb.h>
#include <xcb/xcb.h>
#include <xcb/sync.h>

static_assert(oc_array_size_of_member(oc_linux_x11, winIdToHandle) == OC_APP_MAX_WINDOWS,
  "Must match OC_APP_MAX_WINDOWS");
static_assert(oc_array_size_of_member(oc_linux_x11, winIdToHandle) <= U32_MAX,
  "winIdToHandle's lengh must fit in winIdToHandleLen");

// TODO(pld): window bring to front
// TODO(pld): window focus
// TODO(pld): window center
// TODO(pld): window destroy
// TODO(pld): window set size
// TODO(pld): terminate
// TODO(pld): use ICCCM's recommended way of getting server timestamp
// TODO(pld): WM_CLASS, WM_CLIENT_MACHINE
// (zero-length property set)
//
// TODO(pld): get_content_size
// TODO(pld): set_content_size

// TODO(pld): minimize
// TODO(pld): maximize
// TODO(pld): center
// TODO(pld): frame rect
// TODO(pld): restore
// TODO(pld): destroy
// TODO(pld): close
// TODO(pld): surface callbacks
// TODO(pld): kb/mouse events
// TODO(pld): clipboard
// TODO(pld): file move
// TODO(pld): alert
// TODO(pld): file dialog
// TODO(pld): terminate
// TODO(pld): quit
// TODO(pld): dispatch main thread
// TODO(pld): mouse cursor
//

// NOTE(pld): Thread safety: we assume a thread is pumping events and writing
// into each window's windowData, but cannot delete or create new ones.
// Meanwhile, another thread will dequeue oc_events and read windowData and
// create/delete entries.

static inline void* memz(void* buf, size_t n)
{
    return (memset(buf, 0, n));
}

void oc_init(void)
{
    memset(&oc_appData, 0, sizeof(oc_appData));
    oc_linux_app_data* linux = &oc_appData.linux;

    oc_init_common();
    oc_arena_init(&linux->persistent_arena);

    oc_arena_scope scratch = oc_scratch_begin();

    oc_linux_x11* x11 = &linux->x11;

    x11->display = XOpenDisplay(NULL);
    OC_ASSERT(x11->display);

    xcb_connection_t* conn = XGetXCBConnection(x11->display);
    struct {
        const oc_str8 s;
        xcb_intern_atom_cookie_t cookie;
        xcb_atom_t* atom;
    } atoms[] =
    {
        #define DEF_ATOM(name)  \
            { .s = OC_STR8_LIT(#name), .cookie = 0, .atom = &x11->atoms. name }
        DEF_ATOM(OC_X11_CLIENT_MESSAGE),
        DEF_ATOM(UTF8_STRING),
        DEF_ATOM(WM_CHANGE_STATE),
        DEF_ATOM(WM_DELETE_WINDOW),
        DEF_ATOM(WM_PROTOCOLS),
        DEF_ATOM(WM_STATE),
        DEF_ATOM(_NET_ACTIVE_WINDOW),
        DEF_ATOM(_NET_SUPPORTED),
        DEF_ATOM(_NET_SUPPORTING_WM_CHECK),
        DEF_ATOM(_NET_WM_ICON_NAME),
        DEF_ATOM(_NET_WM_NAME),
        DEF_ATOM(_NET_WM_PID),
        DEF_ATOM(_NET_WM_PING),
        DEF_ATOM(_NET_WM_STATE),
        DEF_ATOM(_NET_WM_STATE_MAXIMIZED_HORZ),
        DEF_ATOM(_NET_WM_STATE_MAXIMIZED_VERT),
        DEF_ATOM(_NET_WM_SYNC_REQUEST),
        DEF_ATOM(_NET_WM_SYNC_REQUEST_COUNTER),
        DEF_ATOM(_NET_WM_USER_TIME),
        DEF_ATOM(_NET_WM_USER_TIME_WINDOW),
        DEF_ATOM(_NET_WM_WINDOW_TYPE),
        DEF_ATOM(_NET_WM_WINDOW_TYPE_NORMAL),
        #undef DEF_ATOM
    };
    for(u64 i = 0; i < oc_array_size(atoms); i++)
    {
        atoms[i].cookie = xcb_intern_atom(conn, false, oc_str8_lp(atoms[i].s));
    }
    xcb_flush(conn);
    for(u64 i = 0; i < oc_array_size(atoms); i++)
    {
        xcb_intern_atom_reply_t* reply = NULL;
        reply = xcb_intern_atom_reply(conn, atoms[i].cookie, NULL);
        // TODO(pld): error handling, crash?
        OC_ASSERT(reply);
        OC_ASSERT(reply->response_type == X11_RESPONSE_TYPE_REPLY);
        memcpy(atoms[i].atom, &reply->atom, sizeof(reply->atom));
        free(reply);
    }

    /* Retrieve root window */
    {
        const xcb_setup_t* setup = xcb_get_setup(conn);
        xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
        OC_ASSERT(iter.rem > 0);
        xcb_screen_t* screen = iter.data;
        x11->rootWinId = screen->root;
    }

    /* Initialize XSync extension */
    {
        OC_ASSERT(XCB_SYNC_MAJOR_VERSION == 3);
        OC_ASSERT(XCB_SYNC_MINOR_VERSION == 1);
        xcb_sync_initialize_cookie_t cookie =
            xcb_sync_initialize(conn, XCB_SYNC_MAJOR_VERSION, XCB_SYNC_MINOR_VERSION);
        xcb_flush(conn);
        xcb_sync_initialize_reply_t* reply = xcb_sync_initialize_reply(conn, cookie, NULL);
        OC_ASSERT(reply);
        OC_ASSERT(reply->response_type == X11_RESPONSE_TYPE_REPLY);
        OC_ASSERT(reply->major_version == XCB_SYNC_MAJOR_VERSION);
        OC_ASSERT(reply->minor_version >= XCB_SYNC_MINOR_VERSION);
        free(reply);
    }

    /* Check for window manager conformance to EWMH and ICCCM via
     * _NET_SUPPORTING_WM_CHECK. */
    {
        xcb_get_property_cookie_t cookie = {0};
        cookie = xcb_get_property(conn, false, x11->rootWinId,
            x11->atoms._NET_SUPPORTING_WM_CHECK, XCB_ATOM_WINDOW, 0, 1);
        xcb_flush(conn);
        xcb_get_property_reply_t* reply = NULL;
        reply = xcb_get_property_reply(conn, cookie, NULL);
        OC_ASSERT(reply);
        OC_ASSERT(reply->response_type == X11_RESPONSE_TYPE_REPLY);
        OC_ASSERT(reply->value_len == 1);
        OC_ASSERT(reply->bytes_after == 0);
        xcb_window_t child = *(xcb_window_t*)xcb_get_property_value(reply);
        OC_ASSERT(child, "Unsupported window manager");
        free(reply);
        cookie = xcb_get_property(conn, false, child,
            x11->atoms._NET_SUPPORTING_WM_CHECK, XCB_ATOM_WINDOW, 0, 1);
        xcb_flush(conn);
        reply = xcb_get_property_reply(conn, cookie, NULL);
        OC_ASSERT(reply);
        OC_ASSERT(reply->response_type == X11_RESPONSE_TYPE_REPLY);
        OC_ASSERT(reply->value_len == 1);
        OC_ASSERT(reply->bytes_after == 0);
        xcb_window_t child2 = *(xcb_window_t*)xcb_get_property_value(reply);
        OC_ASSERT(child == child2, "Unsupported window manager");
        free(reply);
    }

    /* Check whether the hints we require are present in _NET_SUPPORTED */
    {
        xcb_atom_t required[] =
        {
            x11->atoms._NET_ACTIVE_WINDOW,
            x11->atoms._NET_SUPPORTING_WM_CHECK,
            x11->atoms._NET_WM_ICON_NAME,
            x11->atoms._NET_WM_NAME,
            x11->atoms._NET_WM_PID,
            x11->atoms._NET_WM_PING,
            x11->atoms._NET_WM_STATE,
            x11->atoms._NET_WM_STATE_MAXIMIZED_HORZ,
            x11->atoms._NET_WM_STATE_MAXIMIZED_VERT,
            x11->atoms._NET_WM_SYNC_REQUEST,
            x11->atoms._NET_WM_SYNC_REQUEST_COUNTER,
            x11->atoms._NET_WM_USER_TIME,
            //x11->atoms._NET_WM_USER_TIME_WINDOW,
            x11->atoms._NET_WM_WINDOW_TYPE,
            x11->atoms._NET_WM_WINDOW_TYPE_NORMAL,
        };
        u32 required_len = oc_array_size(required);
        u32 offset = 0;
        while(true)
        {
            xcb_get_property_cookie_t cookie = {0};
            cookie = xcb_get_property(conn, false, x11->rootWinId,
                x11->atoms._NET_SUPPORTED, XCB_ATOM_ATOM, offset, 64);
            xcb_flush(conn);
            xcb_get_property_reply_t* reply = xcb_get_property_reply(conn, cookie, NULL);
            OC_ASSERT(reply);
            OC_ASSERT(reply->response_type == X11_RESPONSE_TYPE_REPLY);
            xcb_atom_t* supported = xcb_get_property_value(reply);
            OC_ASSERT(supported);
            for(u32 i = 0; i < reply->value_len; i++, supported++)
            {
                for(u32 j = 0; j < required_len; j++)
                {
                    if(required[j] == *supported)
                    {
                        memmove(&required[j], &required[j + 1], (required_len - j - 1) * sizeof(*required));
                        required_len--;
                        required[required_len] = 0;
                        break;
                    }
                }
            }
            u32 value_len = reply->value_len;
            u32 bytes_after = reply->bytes_after;
            free(reply);
            if(bytes_after == 0)
            {
                break;
            }
            offset += value_len;
        }
        OC_ASSERT(required_len == 0, "Window manager is missing required hints");
    }

    int argc = oc_get_argc();
    const char** argv = oc_get_argv();
    OC_ASSERT(argc && argv);
    oc_str8 instanceName = {0};
    {
        for(u32 i = 1; i < argc; i++)
        {
            if(!strcmp(argv[i], "-name") && i + 1 < argc)
            {
                instanceName = OC_STR8(argv[i]);
                break;
            }
        }
        if(!instanceName.ptr)
        {
            char* resourceName = getenv("RESOURCE_NAME");
            instanceName = OC_STR8(resourceName);
        }
        if(!instanceName.ptr)
        {
            oc_str8 argv0 = OC_STR8(argv[0]);
            u64 i = argv0.len;
            for(; i != U64_MAX; i--)
            {
                if(argv0.ptr[i] == '/')
                {
                    break;
                }
            }
            instanceName = oc_str8_slice(argv0, i + 1, argv0.len);
        }
    }
    oc_str8 className = OC_STR8("Orca");
    u64 wmClassLen = instanceName.len + className.len + 2;
    OC_ASSERT(wmClassLen <= U32_MAX);
    u8* wmClass = oc_arena_push(&linux->persistent_arena, wmClassLen);
    {
        u8* p = wmClass;
        memcpy(p, instanceName.ptr, instanceName.len), p += instanceName.len;
        *p++ = '\0';
        memcpy(p, className.ptr, className.len), p += className.len;
        *p++ = '\0';
        OC_ASSERT((u64)(p - wmClass) == wmClassLen);
    }
    x11->wmClass = wmClass;
    x11->wmClassLen = (u32)wmClassLen;

    x11->wmClientMachine = oc_arena_push_zero(&linux->persistent_arena, 256);
    int ok = gethostname((char*)x11->wmClientMachine, 256);
    OC_ASSERT(ok == 0);
    x11->wmClientMachineLen = strlen((char*)x11->wmClientMachine) + 1;

    // TODO(pld): aborts instead of OC_ASSERTs, with errno for relevant cases
    // TODO(pld): after the above, create window and map window
    //
    // TODO(pld): clang-format
    // TODO(pld): build w/ gcc instead of clang?
    // TODO(pld): debug why gdb gets very confused when debugging runtime, while lldb doesn't

    // TODO(pld): init keys
    //
    oc_scratch_end(scratch);
    return;
}

void oc_terminate(void)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    oc_linux_x11* x11 = &linux->x11;

    XCloseDisplay(x11->display);
    oc_arena_cleanup(&linux->persistent_arena);
    oc_terminate_common();
    return;
}

bool oc_should_quit(void)
{
    // TODO(pld)
    return (false);
}

void oc_request_quit(void)
{
    oc_unimplemented();
    return;
}

void oc_set_cursor(oc_mouse_cursor cursor)
{
    oc_unimplemented();
    return;
}

// TODO(pld): Does this lack thread safety?
static oc_window window_handle_from_x11_id(u32 winId)
{
    oc_linux_x11* x11 = &oc_appData.linux.x11;
    oc_window window = {0};
    for(u32 i = 0; i < x11->winIdToHandleLen; i++)
    {
        x11_win_id_to_handle* entry = &x11->winIdToHandle[i];
        if(entry->winId == winId)
        {
            window = entry->handle;
        }
    }
    return (window);
}

static void window_update_last_user_activity(xcb_connection_t* conn, oc_window window, xcb_timestamp_t ts)
{
    oc_linux_x11* x11 = &oc_appData.linux.x11;
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        xcb_change_property(conn, XCB_PROP_MODE_REPLACE,
            windowData->linux.x11Id, x11->atoms._NET_WM_USER_TIME,
            XCB_ATOM_CARDINAL, 32, 1, &ts);
        windowData->linux.netWmUserTime = ts;
    }
}

static void log_generic_event(const char* name, xcb_generic_event_t* ev)
{
    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8_list s;
    oc_str8_list_init(&s);
    oc_str8_list_pushf(scratch.arena, &s, "EVENT:\n");
    oc_str8_list_pushf(scratch.arena, &s, "response_type: %s (%u", name, ev->response_type & 0x7f);
    if(ev->response_type & 0x80)
    {
        oc_str8_list_pushf(scratch.arena, &s, ", synthetic");
    }
    oc_str8_list_pushf(scratch.arena, &s, ")\n");
    oc_str8_list_pushf(scratch.arena, &s,
        "  pad0: %u\n"
        "  sequence: %u\n"
        "  pad[0]: %u\n"
        "  pad[1]: %u\n"
        "  pad[2]: %u\n"
        "  pad[3]: %u\n"
        "  pad[4]: %u\n"
        "  pad[5]: %u\n"
        "  pad[6]: %u\n"
        "  full_sequence: %u\n",
        (u32)ev->pad0,
        (u32)ev->sequence,
        (u32)ev->pad[0],
        (u32)ev->pad[1],
        (u32)ev->pad[2],
        (u32)ev->pad[3],
        (u32)ev->pad[4],
        (u32)ev->pad[4],
        (u32)ev->pad[5],
        (u32)ev->pad[6],
        (u32)ev->full_sequence);
    oc_str8 msg = oc_str8_list_join(scratch.arena, s);
    oc_log_info("%.*s", oc_str8_lp(msg));
    oc_arena_scope_end(scratch);
}
void oc_pump_events(f64 timeout)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);

    {
        int ok = xcb_flush(conn);
        OC_ASSERT(ok > 0);
    }

    xcb_generic_event_t* ev = NULL;
    {
        int fd = xcb_get_file_descriptor(conn);
        struct pollfd fds = { .fd = fd, .events = POLLIN };
        int ok = 0;
        f64 start = oc_clock_time(OC_CLOCK_MONOTONIC);
        f64 end = 0.0;
        if(timeout < 0.0)
        {
            end = -1.0;
        }
        else if(timeout > 0.0)
        {
            end = start + timeout * 1e9;
        }
        while(true)
        {
            int ms = 0;
            if(end < -1.0)
            {
                ms = -1;
            }
            else if(end > 0.0)
            {
                ms = (int)oc_clamp((end - start) / 1e6, 0.0, (double)INT_MAX);
            }
            ok = poll(&fds, 1, ms);
            if(ok == -1 && errno == EINTR)
            {
                start = oc_clock_time(OC_CLOCK_MONOTONIC);
            }
            else
            {
                break;
            }
        }
        OC_ASSERT(ok == 0 || ok == 1);
        if(ok == 1)
        {
            OC_ASSERT(fds.revents == POLLIN);
            ev = xcb_poll_for_event(conn);
        }
    }
    while(ev)
    {
        const char* type = "(unk)";
        bool synthetic = ev->response_type & 0x80;
        switch(ev->response_type & 0x7f)
        {
        case XCB_KEY_PRESS:
        {
            type = "KeyPress";
            xcb_key_press_event_t* noti = (xcb_key_press_event_t*)ev;
            oc_window window = window_handle_from_x11_id(noti->event);
            window_update_last_user_activity(conn, window, noti->time);
        } break;
        case XCB_KEY_RELEASE:
            type = "KeyRelease";
            break;
        case XCB_BUTTON_PRESS:
        {
            type = "ButtonPress";
            xcb_button_press_event_t* noti = (xcb_button_press_event_t*)ev;
            oc_window window = window_handle_from_x11_id(noti->event);
            window_update_last_user_activity(conn, window, noti->time);
        } break;
        case XCB_BUTTON_RELEASE:
            type = "ButtonRelease";
            break;
        case XCB_MOTION_NOTIFY:
        {
            type = "MotionNotify";
            xcb_motion_notify_event_t* noti = (xcb_motion_notify_event_t*)ev;
            oc_window window = window_handle_from_x11_id(noti->event);
            window_update_last_user_activity(conn, window, noti->time);
        } break;
        case XCB_ENTER_NOTIFY:
            type = "EnterNotify";
            break;
        case XCB_LEAVE_NOTIFY:
            type = "LeaveNotify";
            break;
        case XCB_FOCUS_IN:
        {
            type = "FocusIn";
            xcb_focus_in_event_t* noti = (xcb_focus_in_event_t*)ev;
            OC_ASSERT(noti->detail == XCB_NOTIFY_DETAIL_NONLINEAR || noti->detail == XCB_NOTIFY_DETAIL_POINTER);
            OC_ASSERT(noti->mode == XCB_NOTIFY_MODE_NORMAL);
            oc_window window = window_handle_from_x11_id(noti->event);
            oc_window_data* windowData = oc_window_ptr_from_handle(window);
            if(windowData)
            {
                windowData->linux.focus = OC_LINUX_WINDOW_FOCUSED;
            }
        } break;
        case XCB_FOCUS_OUT:
        {
            type = "FocusOut";
            xcb_focus_out_event_t* noti = (xcb_focus_out_event_t*)ev;
            OC_ASSERT(noti->detail == XCB_NOTIFY_DETAIL_NONLINEAR);
            OC_ASSERT(noti->mode == XCB_NOTIFY_MODE_NORMAL);
            oc_window window = window_handle_from_x11_id(noti->event);
            oc_window_data* windowData = oc_window_ptr_from_handle(window);
            if(windowData)
            {
                windowData->linux.focus = OC_LINUX_WINDOW_UNFOCUSED;
            }
        } break;
        case XCB_KEYMAP_NOTIFY:
            type = "KeymapNotify";
            break;
        case XCB_EXPOSE:
            type = "Expose";
            oc_notpossible();
            break;
        case XCB_GRAPHICS_EXPOSURE:
            type = "GraphicsExposure";
            oc_notpossible();
            break;
        case XCB_NO_EXPOSURE:
            type = "NoExposure";
            oc_notpossible();
            break;
        case XCB_VISIBILITY_NOTIFY:
            type = "VisiblityNotify";
            oc_notpossible();
            break;
        case XCB_CREATE_NOTIFY:
            type = "CreateNotify";
            oc_notpossible();
            break;
        case XCB_DESTROY_NOTIFY:
        {
            // TODO(pld): do not do this here
            type = "DestroyNotify";
            xcb_destroy_notify_event_t* noti = (xcb_destroy_notify_event_t*)ev;
            oc_window window = window_handle_from_x11_id(noti->window);
            oc_window_data* windowData = oc_window_ptr_from_handle(window);
            if(windowData)
            {
                // TODO(pld): clean other things before recycling
                x11_win_id_to_handle entry = {0};
                for(u32 i = linux->x11.winIdToHandleLen - 1; i < U32_MAX; i--)
                {
                  oc_swap(linux->x11.winIdToHandle[i], entry);
                  if(entry.winId == noti->window)
                  {
                    break;
                  }
                  OC_ASSERT(i > 0);
                }
                oc_window_recycle_ptr(windowData);
            }
        } break;
        case XCB_UNMAP_NOTIFY:
        {
            type = "UnmapNotify";
            xcb_unmap_notify_event_t* noti = (xcb_unmap_notify_event_t*)ev;
            oc_window window = window_handle_from_x11_id(noti->window);
            oc_window_data* windowData = oc_window_ptr_from_handle(window);
            if(windowData)
            {
                if(0) windowData->linux.state = X11_WINDOW_STATE_ICONIC;
                windowData->linux.flags &= ~OC_LINUX_WINDOW_X11_MAPPED;
            }
        } break;
        case XCB_MAP_NOTIFY:
        {
            type = "MapNotify";
            xcb_map_notify_event_t* noti = (xcb_map_notify_event_t*)ev;
            oc_window window = window_handle_from_x11_id(noti->window);
            oc_window_data* windowData = oc_window_ptr_from_handle(window);
            if(windowData)
            {
                if(0 && windowData->linux.flags & OC_LINUX_WINDOW_X11_MAP_IS_ICONIC)
                {
                    windowData->linux.state = X11_WINDOW_STATE_ICONIC;
                }
                else if(0)
                {
                    windowData->linux.state = X11_WINDOW_STATE_NORMAL;
                }
                windowData->linux.flags |= OC_LINUX_WINDOW_X11_MAPPED;
            }
        } break;
        case XCB_MAP_REQUEST:
            type = "MapRequest";
            oc_notpossible();
            break;
        case XCB_REPARENT_NOTIFY:
        {
            type = "ReparentNotify";
            xcb_reparent_notify_event_t* noti = (xcb_reparent_notify_event_t*)ev;
            oc_window window = window_handle_from_x11_id(noti->window);
            oc_window_data* windowData = oc_window_ptr_from_handle(window);
            if(windowData)
            {
                windowData->linux.posFromParent.x = noti->x;
                windowData->linux.posFromParent.y = noti->y;
            }
        } break;
        case XCB_CONFIGURE_NOTIFY:
        {
            type = "ConfigureNotify";
            xcb_configure_notify_event_t* noti = (xcb_configure_notify_event_t*)ev;
            oc_window window = window_handle_from_x11_id(noti->window);
            oc_window_data* windowData = oc_window_ptr_from_handle(window);
            if(!windowData)  break;

            //FIXME(pld): after repainting only
            if(windowData->linux.netWmSyncRequestUpdateValue)
            {
                xcb_sync_counter_t id = windowData->linux.netWmSyncRequestCounterId;
                xcb_sync_int64_t val =
                {
                    .hi = (i32)((windowData->linux.netWmSyncRequestUpdateValue >> 32) & 0xFFFFFFFF),
                    .lo = (u32)((windowData->linux.netWmSyncRequestUpdateValue >>  0) & 0xFFFFFFFF),
                };
                xcb_sync_set_counter(conn, id, val);
            }
        } break;
        case XCB_CONFIGURE_REQUEST:
            type = "ConfigureRequest";
            oc_notpossible();
            break;
        case XCB_GRAVITY_NOTIFY:
            type = "GravityNotify";
            break;
        case XCB_RESIZE_REQUEST:
        {
            type = "ResizeRequest";
            xcb_resize_request_event_t* rr = (xcb_resize_request_event_t*)ev;
            oc_log_info(
                "EVENT:\n"
                "  response_type: ResizeRequest (%u)\n"
                "  pad0: %u\n"
                "  sequence: %u\n"
                "  window: %u\n"
                "  width: %u\n"
                "  height: %u\n",
                (u32)rr->response_type,
                (u32)rr->pad0,
                (u32)rr->sequence,
                (u32)rr->window,
                (u32)rr->width,
                (u32)rr->height
            );
        } break;
        case XCB_CIRCULATE_NOTIFY:
            type = "CirculateNotify";
            break;
        case XCB_CIRCULATE_REQUEST:
            type = "CirculateRequest";
            break;
        case XCB_PROPERTY_NOTIFY:
        {
            type = "PropertyNotify";
            // TODO(pld): use timestamp?
            xcb_property_notify_event_t* noti = (xcb_property_notify_event_t*)ev;
            oc_window window = window_handle_from_x11_id(noti->window);
            oc_window_data* windowData = oc_window_ptr_from_handle(window);
            if(!windowData)  break;

            enum {
                X11_PROPERTY_NOTIFY_NEW_VALUE = 0,
                X11_PROPERTY_NOTIFY_DELETED = 1,
            };
            if(noti->atom == linux->x11.atoms.WM_STATE)
            {
                if(noti->state == X11_PROPERTY_NOTIFY_NEW_VALUE)
                {
                    xcb_get_property_cookie_t cookie = xcb_get_property(conn,
                        false, windowData->linux.x11Id, linux->x11.atoms.WM_STATE,
                        linux->x11.atoms.WM_STATE, 0, 1);
                    xcb_flush(conn);
                    xcb_get_property_reply_t* reply = xcb_get_property_reply(conn, cookie, NULL);
                    // TODO(pld): handle errors
                    OC_ASSERT(reply);
                    OC_ASSERT(reply->response_type == X11_RESPONSE_TYPE_REPLY);
                    if(reply->type == XCB_NONE)  break;
                    OC_ASSERT(reply->type == linux->x11.atoms.WM_STATE);
                    OC_ASSERT(reply->format == 32);
                    OC_ASSERT(reply->bytes_after == 4);
                    OC_ASSERT(reply->value_len == 1);
                    u32* p = xcb_get_property_value(reply);
                    OC_ASSERT(p);
                    x11_window_state state = *p;
                    if(state == X11_WINDOW_STATE_WITHDRAWN)  type = "PropertyNotify:WM_STATE:Withdrawn";
                    else if(state == X11_WINDOW_STATE_ICONIC)  type = "PropertyNotify:WM_STATE:Iconic";
                    else if(state == X11_WINDOW_STATE_NORMAL)  type = "PropertyNotify:WM_STATE:Normal";
                    else  oc_unreachable();
                    if(0 && state == X11_WINDOW_STATE_WITHDRAWN)
                    {
                        windowData->linux.state = X11_WINDOW_STATE_WITHDRAWN;
                    }
                    else if(0 && state == X11_WINDOW_STATE_ICONIC && windowData->linux.state == X11_WINDOW_STATE_WITHDRAWN)
                    {
                        windowData->linux.state = X11_WINDOW_STATE_ICONIC;
                    }
                    else if(0 && state == X11_WINDOW_STATE_ICONIC && windowData->linux.state == X11_WINDOW_STATE_NORMAL)
                    {
                        /* Mutter (GNOME) does not unmap the window when
                         * transitioning to the Iconic state, so the only we
                         * know the window has transitioned is via the WM_STATE
                         * property change. */
                        windowData->linux.state = X11_WINDOW_STATE_ICONIC;
                    }
                    else if(0 && state == X11_WINDOW_STATE_NORMAL && windowData->linux.state == X11_WINDOW_STATE_ICONIC)
                    {
                        /* We also have to handle the converse of the above as
                         * Mutter does not need to map the window when
                         * transitioning from the Iconic to Normal state. */
                        windowData->linux.state = X11_WINDOW_STATE_NORMAL;
                    }
                    else if(0)
                    {
                        OC_ASSERT(state == windowData->linux.state);
                    }
                    windowData->linux.state = state;
                    free(reply);
                }
                else if(noti->state == X11_PROPERTY_NOTIFY_DELETED)
                {
                    type = "PropertyNotify:WM_STATE:Withdrawn";
                    windowData->linux.state = X11_WINDOW_STATE_WITHDRAWN;
                }
                else
                {
                    oc_unreachable();
                }
            }
            else if(noti->atom == linux->x11.atoms._NET_WM_STATE)
            {
                if(noti->state == X11_PROPERTY_NOTIFY_NEW_VALUE)
                {
                    type = "PropertyNotify:_NET_WM_STATE:(new value)";
                    xcb_get_property_cookie_t cookie = xcb_get_property(conn,
                        false, windowData->linux.x11Id, linux->x11.atoms._NET_WM_STATE,
                        XCB_ATOM_ATOM, 0, oc_array_size(windowData->linux.netState));
                    xcb_flush(conn);
                    xcb_get_property_reply_t* reply = xcb_get_property_reply(conn, cookie, NULL);
                    // TODO(pld): handle errors
                    OC_ASSERT(reply);
                    OC_ASSERT(reply->response_type == X11_RESPONSE_TYPE_REPLY);
                    if(reply->type == XCB_NONE)  break;
                    OC_ASSERT(reply->type == XCB_ATOM_ATOM);
                    OC_ASSERT(reply->format == 32);
                    OC_ASSERT(reply->bytes_after == 0);
                    u32* p = xcb_get_property_value(reply);
                    OC_ASSERT(p);
                    xcb_atom_t* hints = p;
                    u32 hintsLen = reply->value_len;
                    OC_ASSERT(hintsLen <= oc_array_size(windowData->linux.netState));
                    memcpy(windowData->linux.netState, hints, hintsLen * 4);
                    memz(&windowData->linux.netState[hintsLen], sizeof(windowData->linux.netState) - hintsLen * 4);
                    windowData->linux.netStateLen = hintsLen;
                    free(reply);
                }
                else if(noti->state == X11_PROPERTY_NOTIFY_DELETED)
                {
                    type = "PropertyNotify:_NET_WM_STATE:Withdrawn";
                    windowData->linux.state = X11_WINDOW_STATE_WITHDRAWN;
                }
                else
                {
                    oc_unreachable();
                }
            }
            else if(noti->atom == linux->x11.atoms._NET_WM_USER_TIME)
            {
                /* We listen for changes on _NET_WM_USER_TIME to set an initial
                 * value for it when the window is shown. This allows to call
                 * oc_window_focus, which requires a recent enough timestamp,
                 * without needing any keyboard/pointer input beforehand.
                 * Keyboard and pointer input being the only ways this
                 * timestamp updates otherwise. */
                if(noti->state == X11_PROPERTY_NOTIFY_NEW_VALUE)
                {
                    type = "PropertyNotify:_NET_WM_USER_TIME:NewValue";
                    if(windowData->linux.netWmUserTime == 0)
                    {
                        window_update_last_user_activity(conn, window, noti->time);
                    }
                }
                else if(noti->state == X11_PROPERTY_NOTIFY_DELETED)
                {
                    type = "PropertyNotify:_NET_WM_USER_TIME:Withdrawn";
                    windowData->linux.netWmUserTime = 0;
                }
                else
                {
                    oc_unreachable();
                }
            }
        } break;
        case XCB_SELECTION_CLEAR:
            type = "SelectionClear";
            break;
        case XCB_SELECTION_REQUEST:
            type = "SelectionRequest";
            break;
        case XCB_SELECTION_NOTIFY:
            type = "SelectionNotify";
            break;
        case XCB_COLORMAP_NOTIFY:
            type = "ColormapNotify";
            break;
        case XCB_CLIENT_MESSAGE:
        {
            type = "ClientMessage";
            xcb_client_message_event_t* noti = (xcb_client_message_event_t*)ev;
            if(noti->type == linux->x11.atoms.OC_X11_CLIENT_MESSAGE)
            {
                OC_ASSERT(noti->format == 32);
                oc_window window = window_handle_from_x11_id(noti->window);
                oc_window_data* windowData = oc_window_ptr_from_handle(window);
                if(!windowData)  break;
                oc_x11_client_message t = noti->data.data32[0];
                if(t == OC_X11_CLIENT_MESSAGE_UNFOCUS)
                {
                    if(windowData->linux.focus == OC_LINUX_WINDOW_FOCUSED)
                    {
                        windowData->linux.focus = OC_LINUX_WINDOW_FOCUSED_IGNORE_INPUTS;
                    }
                }
                else
                {
                    oc_notpossible();
                }
            }
            else if(noti->type == linux->x11.atoms.WM_PROTOCOLS)
            {
                OC_ASSERT(noti->format == 32);
                oc_window window = window_handle_from_x11_id(noti->window);
                oc_window_data* windowData = oc_window_ptr_from_handle(window);
                if(!windowData)  break;
                xcb_atom_t protocol = noti->data.data32[0];
                xcb_timestamp_t ts = noti->data.data32[1];
                if(protocol == linux->x11.atoms.WM_DELETE_WINDOW)
                {
                    //FIXME(pld): confirm close dialog?
                    if(1)
                    {
                        oc_window_destroy(window);
                    }
                    else
                    {
                        window_update_last_user_activity(conn, window, ts);
                    }
                }
                else if(protocol == linux->x11.atoms._NET_WM_PING)
                {
                    noti->window = linux->x11.rootWinId;
                    xcb_send_event(conn, false, linux->x11.rootWinId,
                        XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
                        XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
                        (const char*)&noti);
                }
                else if(protocol == linux->x11.atoms._NET_WM_SYNC_REQUEST)
                {
                    u64 lo = (u64)noti->data.data32[2];
                    u64 hi = (u64)noti->data.data32[3];
                    windowData->linux.netWmSyncRequestUpdateValue = (hi << 32 | lo);
                }
                else
                {
                    oc_notpossible();
                }
            }
            else
            {
                oc_notpossible();
            }
        } break;
        case XCB_MAPPING_NOTIFY:
            type = "MappingNotify";
            break;
        }
        log_generic_event(type, ev);
        ev = xcb_poll_for_event(conn);
    }
    // TODO(pld): handle I/O errors
    if(!ev)  OC_ASSERT(!xcb_connection_has_error(conn));
    return;
}

oc_window oc_window_create(oc_rect contentRect, oc_str8 title, oc_window_style style)
{
    // TODO(pld): style
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);

    if(linux->x11.winIdToHandleLen == oc_array_size(linux->x11.winIdToHandle))
    {
      return (oc_window){0};
    }

    const xcb_setup_t* setup = xcb_get_setup(conn);

    u32 winId = xcb_generate_id(conn);
    u32 parentId = linux->x11.rootWinId;
    xcb_cw_t attributesMask = XCB_CW_BACK_PIXMAP |
        XCB_CW_BACK_PIXEL |
        XCB_CW_BORDER_PIXMAP |
        XCB_CW_BORDER_PIXEL |
        XCB_CW_BIT_GRAVITY |
        XCB_CW_WIN_GRAVITY |
        XCB_CW_BACKING_STORE |
        XCB_CW_BACKING_PLANES |
        XCB_CW_BACKING_PIXEL |
        XCB_CW_OVERRIDE_REDIRECT |
        XCB_CW_SAVE_UNDER |
        XCB_CW_EVENT_MASK |
        XCB_CW_DONT_PROPAGATE |
        XCB_CW_COLORMAP |
        XCB_CW_CURSOR |
        0;
    xcb_create_window_value_list_t attributes =
    {
        .background_pixmap = XCB_BACK_PIXMAP_NONE,
        .background_pixel = 0,
        .border_pixmap = XCB_COPY_FROM_PARENT,
        .border_pixel = 0,
        .bit_gravity = XCB_GRAVITY_BIT_FORGET,
        .win_gravity = XCB_GRAVITY_NORTH_WEST,
        .backing_store = XCB_BACKING_STORE_NOT_USEFUL,
        .backing_planes = 0,
        .backing_pixel = 0,
        .override_redirect = false,
        .save_under = false,
        .event_mask =
            XCB_EVENT_MASK_KEY_PRESS |
            //XCB_EVENT_MASK_KEY_RELEASE |
            XCB_EVENT_MASK_BUTTON_PRESS |
            //XCB_EVENT_MASK_BUTTON_RELEASE |
            //XCB_EVENT_MASK_ENTER_WINDOW |
            //XCB_EVENT_MASK_LEAVE_WINDOW |
            XCB_EVENT_MASK_POINTER_MOTION |
            //XCB_EVENT_MASK_POINTER_MOTION_HINT |
            //XCB_EVENT_MASK_BUTTON_1_MOTION |
            //XCB_EVENT_MASK_BUTTON_2_MOTION |
            //XCB_EVENT_MASK_BUTTON_3_MOTION |
            //XCB_EVENT_MASK_BUTTON_4_MOTION |
            //XCB_EVENT_MASK_BUTTON_5_MOTION |
            //XCB_EVENT_MASK_BUTTON_MOTION |
            //XCB_EVENT_MASK_KEYMAP_STATE |
            //XCB_EVENT_MASK_EXPOSURE |
            //XCB_EVENT_MASK_VISIBILITY_CHANGE |
            XCB_EVENT_MASK_STRUCTURE_NOTIFY |
            //XCB_EVENT_MASK_RESIZE_REDIRECT |
            //XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
            //XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
            XCB_EVENT_MASK_FOCUS_CHANGE |
            XCB_EVENT_MASK_PROPERTY_CHANGE |
            //XCB_EVENT_MASK_COLOR_MAP_CHANGE |
            //XCB_EVENT_MASK_OWNER_GRAB_BUTTON |
            0,
        .do_not_propogate_mask = 0,
        .colormap = XCB_COPY_FROM_PARENT,
        .cursor = XCB_CURSOR_NONE,
    };

    xcb_create_window_aux(conn, 0, winId, parentId,
        contentRect.x, contentRect.y, contentRect.w, contentRect.h,
        0, XCB_WINDOW_CLASS_INPUT_OUTPUT, XCB_COPY_FROM_PARENT,
        attributesMask, &attributes);
    if(OC_DEBUG)
    {
        xcb_flush(conn);
    }
    static const u32 wmHints[9] =
    {
        /* flags: InputHint, StateHint */
        1 | 2,
        /* input: Passive (as we don't add WM_TAKE_FOCUS to the WM_PROTOCOLS
         * property) */
        1,
        /* initial_state */
        X11_WINDOW_STATE_NORMAL,
    };
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE,
        winId, XCB_ATOM_WM_HINTS,
        XCB_ATOM_WM_HINTS, 32, sizeof(wmHints) / 4, &wmHints);
    static const u32 wmNormalHints[18] =
    {
        /* flags: PPosition, PSize, PResizeInc, PWinGravity */
        [0] = 4 | 8 | 64 | 512,
        /* width_inc, height_inc */
        [9] = 1,
        [10] = 1,
        /* win_gravity */
        [17] = XCB_GRAVITY_NORTH_WEST,
    };
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE,
        winId, XCB_ATOM_WM_NORMAL_HINTS,
        XCB_ATOM_WM_SIZE_HINTS, 32, sizeof(wmNormalHints) / 4, &wmNormalHints);
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, winId, XCB_ATOM_WM_CLASS,
        XCB_ATOM_STRING, 8, linux->x11.wmClassLen, linux->x11.wmClass);
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, winId, XCB_ATOM_WM_CLIENT_MACHINE,
        XCB_ATOM_STRING, 8, linux->x11.wmClientMachineLen, linux->x11.wmClientMachine);
    u32 pid = (u32)getpid();
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, winId, linux->x11.atoms._NET_WM_PID,
        XCB_ATOM_CARDINAL, 32, 1, &pid);
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, winId, linux->x11.atoms._NET_WM_WINDOW_TYPE,
        XCB_ATOM_ATOM, 32, 1, &linux->x11.atoms._NET_WM_WINDOW_TYPE_NORMAL);
    xcb_atom_t protocols[] =
    {
        linux->x11.atoms.WM_DELETE_WINDOW,
        linux->x11.atoms._NET_WM_PING,
        linux->x11.atoms._NET_WM_SYNC_REQUEST,
    };
    OC_STATIC_ASSERT(sizeof(xcb_atom_t) == 4);
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, winId, linux->x11.atoms.WM_PROTOCOLS,
        XCB_ATOM_ATOM, 32, oc_array_size(protocols), protocols);
    u32 counterId = xcb_generate_id(conn);
    xcb_sync_create_counter(conn, counterId, (xcb_sync_int64_t){0});
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, winId, linux->x11.atoms._NET_WM_SYNC_REQUEST_COUNTER,
        XCB_ATOM_CARDINAL, 32, 1, &counterId);
    // TODO(pld): test _NET_WM_ALLOWED_ACTIONS?

    oc_window_data* windowData = oc_window_alloc();
    windowData->linux.x11Id = winId;
    windowData->linux.netWmSyncRequestCounterId = counterId;
    oc_window window = oc_window_handle_from_ptr(windowData);

    linux->x11.winIdToHandle[linux->x11.winIdToHandleLen++] = (x11_win_id_to_handle){
      .winId = winId,
      .handle = window,
    };

    oc_window_set_title(window, title);

    return (window);
}

void oc_window_destroy(oc_window window)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);
    oc_window_data* windowData = oc_window_ptr_from_handle(window);

    xcb_destroy_window(conn, windowData->linux.x11Id);
    oc_window_recycle_ptr(windowData);
    return;
}

void* oc_window_native_pointer(oc_window window)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    void* ptr = NULL;
    if(windowData)
    {
        ptr = (void*)(uptr)windowData->linux.x11Id;
    }
    return (ptr);
}

bool oc_window_should_close(oc_window window)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    return (windowData && windowData->shouldClose);
}

void oc_window_request_close(oc_window window)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData && !windowData->shouldClose)
    {
        windowData->shouldClose = true;
        // TODO(pld): enqueue destroy window?
        // TODO(pld): enqueue OC_EVENT_WINDOW_CLOSE
    }
    return;
}

void oc_window_cancel_close(oc_window window)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        windowData->shouldClose = false;
    }
    return;
}

void oc_window_set_title(oc_window window, oc_str8 title)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        u32 saturated_len = oc_clamp_high(title.len, UINT32_MAX);
        xcb_change_property(conn, XCB_PROP_MODE_REPLACE,
            windowData->linux.x11Id, XCB_ATOM_WM_NAME,
            XCB_ATOM_STRING, 8, saturated_len, title.ptr);
        xcb_change_property(conn, XCB_PROP_MODE_REPLACE,
            windowData->linux.x11Id, XCB_ATOM_WM_ICON_NAME,
            XCB_ATOM_STRING, 8, saturated_len, title.ptr);
        xcb_change_property(conn, XCB_PROP_MODE_REPLACE,
            windowData->linux.x11Id, linux->x11.atoms._NET_WM_NAME,
            linux->x11.atoms.UTF8_STRING, 8, saturated_len, title.ptr);
        xcb_change_property(conn, XCB_PROP_MODE_REPLACE,
            windowData->linux.x11Id, linux->x11.atoms._NET_WM_ICON_NAME,
            linux->x11.atoms.UTF8_STRING, 8, saturated_len, title.ptr);
    }
    return;
}

void oc_window_show(oc_window window)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData && windowData->linux.state != X11_WINDOW_STATE_NORMAL)
    {
        if(windowData->linux.state == X11_WINDOW_STATE_WITHDRAWN)
        {
            static const u32 wmHints[9] =
            {
                /* flags: InputHint, StateHint */
                1 | 2,
                /* input: Passive (as we don't add WM_TAKE_FOCUS to the WM_PROTOCOLS
                 * property) */
                1,
                /* initial_state */
                X11_WINDOW_STATE_NORMAL,
            };
            xcb_change_property(conn, XCB_PROP_MODE_REPLACE,
                windowData->linux.x11Id, XCB_ATOM_WM_HINTS,
                XCB_ATOM_WM_HINTS, 32, sizeof(wmHints) / 4, &wmHints);
            windowData->linux.flags &= ~OC_LINUX_WINDOW_X11_MAP_IS_ICONIC;
        }
        if(0 && windowData->linux.flags & OC_LINUX_WINDOW_X11_MAPPED)
        {
            /* WM (e.g. Mutter) does not unmap the window when Iconic, we need
             * to send an explicit MapRequest event to the root. */
            xcb_map_request_event_t msg =
            {
                .response_type = XCB_MAP_REQUEST,
                .parent = XCB_NONE,
                .window = windowData->linux.x11Id,
            };
            xcb_send_event(conn, false, linux->x11.rootWinId,
                XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
                XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
                (const char*)&msg);
        }
        else
        {
            xcb_delete_property(conn, windowData->linux.x11Id,
                linux->x11.atoms._NET_WM_USER_TIME);
            xcb_map_window(conn, windowData->linux.x11Id);
            xcb_change_property(conn, XCB_PROP_MODE_APPEND,
                windowData->linux.x11Id, linux->x11.atoms._NET_WM_USER_TIME,
                XCB_ATOM_CARDINAL, 32, 0, NULL);
        }
    }
    return;
}

bool oc_window_is_hidden(oc_window window)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    return (windowData && windowData->linux.state == X11_WINDOW_STATE_WITHDRAWN);
}

void oc_window_hide(oc_window window)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    // TODO(pld): can't test any window value from the window state before
    // we've flushed and pumped events? e.g. show, hide, show, flush: second
    // show is ignored
    if(windowData && windowData->linux.state != X11_WINDOW_STATE_WITHDRAWN)
    {
        xcb_unmap_window(conn, windowData->linux.x11Id);
        xcb_unmap_notify_event_t msg =
        {
            .response_type = XCB_UNMAP_NOTIFY,
            .event = linux->x11.rootWinId,
            .window = windowData->linux.x11Id,
            .from_configure = false,
        };
        xcb_send_event(conn, false, linux->x11.rootWinId,
            XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
            XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
            (const char*)&msg);
    }
}

bool oc_window_is_minimized(oc_window window)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    return (windowData && windowData->linux.state == X11_WINDOW_STATE_ICONIC);
}

void oc_window_minimize(oc_window window)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData && windowData->linux.state != X11_WINDOW_STATE_ICONIC)
    {
        if(windowData->linux.state == X11_WINDOW_STATE_WITHDRAWN)
        {
            static const u32 wmHints[9] =
            {
                /* flags: InputHint, StateHint */
                1 | 2,
                /* input: Passive (as we don't add WM_TAKE_FOCUS to the WM_PROTOCOLS
                 * property) */
                1,
                /* initial_state */
                X11_WINDOW_STATE_ICONIC,
            };
            xcb_change_property(conn, XCB_PROP_MODE_REPLACE,
                windowData->linux.x11Id, XCB_ATOM_WM_HINTS,
                XCB_ATOM_WM_HINTS, 32, sizeof(wmHints) / 4, &wmHints);
            xcb_delete_property(conn, windowData->linux.x11Id,
                linux->x11.atoms._NET_WM_USER_TIME);
            xcb_map_window(conn, windowData->linux.x11Id);
            xcb_change_property(conn, XCB_PROP_MODE_APPEND,
                windowData->linux.x11Id, linux->x11.atoms._NET_WM_USER_TIME,
                XCB_ATOM_CARDINAL, 32, 0, NULL);
            windowData->linux.flags |= OC_LINUX_WINDOW_X11_MAP_IS_ICONIC;
        }
        else if(windowData->linux.state == X11_WINDOW_STATE_NORMAL)
        {
            xcb_client_message_event_t msg =
            {
                .response_type = XCB_CLIENT_MESSAGE,
                .format = 32,
                .window = windowData->linux.x11Id,
                .type = linux->x11.atoms.WM_CHANGE_STATE,
                .data.data32[0] = X11_WINDOW_STATE_ICONIC,
            };
            xcb_send_event(conn, false, linux->x11.rootWinId,
                XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
                XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
                (const char*)&msg);
            }
        else
        {
            oc_unreachable();
        }
    }
    return;
}

bool oc_window_is_maximized(oc_window window)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData && windowData->linux.state == X11_WINDOW_STATE_NORMAL)
    {
        bool horz = false, vert = false;
        for(u32 i = 0; i < windowData->linux.netStateLen; i++)
        {
            if(windowData->linux.netState[i] == linux->x11.atoms._NET_WM_STATE_MAXIMIZED_HORZ)
            {
                horz = true;
            }
            else if(windowData->linux.netState[i] == linux->x11.atoms._NET_WM_STATE_MAXIMIZED_VERT)
            {
                vert = true;
            }
        }
        return (horz && vert);
    }
    else
    {
        return (false);
    }
}

void oc_window_maximize(oc_window window)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        oc_window_show(window);

        xcb_client_message_event_t msg =
        {
            .response_type = XCB_CLIENT_MESSAGE,
            .format = 32,
            .window = windowData->linux.x11Id,
            .type = linux->x11.atoms._NET_WM_STATE,
            .data.data32[0] = X11_NET_WM_STATE_ADD,
            .data.data32[1] = linux->x11.atoms._NET_WM_STATE_MAXIMIZED_HORZ,
            .data.data32[2] = linux->x11.atoms._NET_WM_STATE_MAXIMIZED_VERT,
            .data.data32[3] = X11_EWMH_SOURCE_INDICATION_NORMAL,
        };
        xcb_send_event(conn, false, linux->x11.rootWinId,
            XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
            XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
            (const char*)&msg);
    }
    return;
}

void oc_window_restore(oc_window window)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);
    oc_window_data* windowData = oc_window_ptr_from_handle(window);

    if(windowData)
    {
        oc_window_show(window);

        xcb_client_message_event_t msg =
        {
            .response_type = XCB_CLIENT_MESSAGE,
            .format = 32,
            .window = windowData->linux.x11Id,
            .type = linux->x11.atoms._NET_WM_STATE,
            .data.data32[0] = X11_NET_WM_STATE_REMOVE,
            .data.data32[1] = linux->x11.atoms._NET_WM_STATE_MAXIMIZED_HORZ,
            .data.data32[2] = linux->x11.atoms._NET_WM_STATE_MAXIMIZED_VERT,
            .data.data32[3] = X11_EWMH_SOURCE_INDICATION_NORMAL,
        };
        xcb_send_event(conn, false, linux->x11.rootWinId,
            XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
            XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
            (const char*)&msg);
    }
    return;
}

bool oc_window_has_focus(oc_window window)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    bool focused = false;
    if(windowData)
    {
        focused = windowData->linux.focus == OC_LINUX_WINDOW_FOCUSED;
    }
    return (focused);
}

//TODO(pld): test assumed _NET_SUPPORTED features in init
void oc_window_focus(oc_window window)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        xcb_client_message_event_t msg =
        {
            .response_type = XCB_CLIENT_MESSAGE,
            .format = 32,
            .window = windowData->linux.x11Id,
            .type = linux->x11.atoms._NET_ACTIVE_WINDOW,
            .data.data32[0] = X11_EWMH_SOURCE_INDICATION_NORMAL,
            .data.data32[1] = windowData->linux.netWmUserTime,
            .data.data32[2] = 0,
        };
        xcb_send_event(conn, false, linux->x11.rootWinId,
            XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
            XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
            (const char*)&msg);
    }
    return;
}

void oc_window_unfocus(oc_window window)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        xcb_client_message_event_t msg =
        {
            .response_type = XCB_CLIENT_MESSAGE,
            .format = 32,
            .window = windowData->linux.x11Id,
            .type = linux->x11.atoms.OC_X11_CLIENT_MESSAGE,
            .data.data32[0] = OC_X11_CLIENT_MESSAGE_UNFOCUS,
        };
        xcb_void_cookie_t cookie =
        xcb_send_event(conn, false, windowData->linux.x11Id,
            XCB_EVENT_MASK_FOCUS_CHANGE,
            (const char*)&msg);
        oc_log_info("cookie=%d\n", cookie.sequence);
    }
    return;
}

void oc_window_send_to_back(oc_window window)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        xcb_config_window_t config_mask = XCB_CONFIG_WINDOW_STACK_MODE;
        xcb_configure_window_value_list_t config =
        {
            .stack_mode = XCB_STACK_MODE_BELOW,
        };
        xcb_configure_window_aux(conn, windowData->linux.x11Id, config_mask, &config);
    }
    return;
}

void oc_window_bring_to_front(oc_window window)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        xcb_config_window_t config_mask = XCB_CONFIG_WINDOW_STACK_MODE;
        xcb_configure_window_value_list_t config =
        {
            .stack_mode = XCB_STACK_MODE_ABOVE,
        };
        xcb_configure_window_aux(conn, windowData->linux.x11Id, config_mask, &config);
    }
    return;
}

u64 oc_window_debug_stack_pos(oc_window window)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);
    oc_window_data* windowData = oc_window_ptr_from_handle(window);

    xcb_query_tree_cookie_t cookie = xcb_query_tree(conn, windowData->linux.x11Id);
    xcb_flush(conn);
    xcb_query_tree_reply_t* reply = NULL;
    reply = xcb_query_tree_reply(conn, cookie, NULL);
    OC_ASSERT(reply);
    OC_ASSERT(reply->response_type == X11_RESPONSE_TYPE_REPLY);
    OC_ASSERT(reply->root == linux->x11.rootWinId);
    OC_ASSERT(reply->parent != XCB_NONE);
    xcb_window_t parent = reply->parent;
    free(reply);

    cookie = xcb_query_tree(conn, linux->x11.rootWinId);
    xcb_flush(conn);
    reply = xcb_query_tree_reply(conn, cookie, NULL);
    OC_ASSERT(reply);
    OC_ASSERT(reply->response_type == X11_RESPONSE_TYPE_REPLY);
    OC_ASSERT(reply->root == linux->x11.rootWinId);
    OC_ASSERT(reply->parent == XCB_NONE);
    OC_ASSERT(reply->children_len >= 1);
    xcb_window_t* children = xcb_query_tree_children(reply);
    OC_ASSERT(children);

    u64 i = 0;
    for(; i < reply->children_len; i++)
    {
        if(children[i] == windowData->linux.x11Id || children[i] == parent)  break;
    }
    return (i);
}

oc_rect oc_window_get_frame_rect(oc_window window)
{
    oc_unimplemented();
    return ((oc_rect){0});
}
void oc_window_set_frame_rect(oc_window window, oc_rect rect)
{
    oc_unimplemented();
    return;
}
oc_rect oc_window_get_content_rect(oc_window window)
{
    // TODO(pld)
    return ((oc_rect){0});
}
void oc_window_set_content_rect(oc_window window, oc_rect rect)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        // TODO(pld): border?
        // TODO(pld): should offset x/y by border?
        xcb_config_window_t config_mask = XCB_CONFIG_WINDOW_X |
            XCB_CONFIG_WINDOW_Y |
            XCB_CONFIG_WINDOW_WIDTH |
            XCB_CONFIG_WINDOW_HEIGHT |
            0;
        xcb_configure_window_value_list_t config =
        {
            .x = rect.x,
            .y = rect.y,
            .width = rect.w,
            .height = rect.h,
        };
        xcb_configure_window_aux(conn, windowData->linux.x11Id, config_mask, &config);
        xcb_flush(conn);
    }
}
void oc_window_center(oc_window window)
{
    // TODO(pld): window center
    return;
}
oc_rect oc_window_content_rect_for_frame_rect(oc_rect frameRect, oc_window_style style)
{
    oc_unimplemented();
    return ((oc_rect){0});
}
oc_rect oc_window_frame_rect_for_content_rect(oc_rect contentRect, oc_window_style style)
{
    oc_unimplemented();
    return ((oc_rect){0});
}
i32 oc_dispatch_on_main_thread_sync(oc_dispatch_proc proc, void* user)
{
    oc_unimplemented();
    return (-1);
}
void oc_clipboard_clear(void)
{
    oc_unimplemented();
    return;
}
void oc_clipboard_set_string(oc_str8 string)
{
    oc_unimplemented();
    return;
}
oc_str8 oc_clipboard_get_string(oc_arena* arena)
{
    oc_unimplemented();
    return ((oc_str8){0});
}
oc_str8 oc_clipboard_copy_string(oc_str8 backing)
{
    oc_unimplemented();
    return ((oc_str8){0});
}
bool oc_clipboard_has_tag(const char* tag)
{
    oc_unimplemented();
    return (false);
}
void oc_clipboard_set_data_for_tag(const char* tag, oc_str8 data)
{
    oc_unimplemented();
    return;
}
oc_str8 oc_clipboard_get_data_for_tag(oc_arena* arena, const char* tag)
{
    oc_unimplemented();
    return ((oc_str8){0});
}
oc_file_dialog_result oc_file_dialog_for_table(oc_arena* arena, oc_file_dialog_desc* desc, oc_file_table* table)
{
    oc_unimplemented();
    return ((oc_file_dialog_result){0});
}
int oc_alert_popup(oc_str8 title, oc_str8 message, oc_str8_list options)
{
    oc_unimplemented();
    return (-1);
}
int oc_file_move(oc_str8 from, oc_str8 to)
{
    oc_unimplemented();
    return (-1);
}
int oc_file_remove(oc_str8 path)
{
    oc_unimplemented();
    return (-1);
}
int oc_directory_create(oc_str8 path)
{
    oc_unimplemented();
    return (-1);
}

/*
void oc_surface_cleanup(oc_surface_data* surface)
{
    oc_unimplemented();
    return;
}

oc_vec2 oc_linux_surface_get_size(oc_surface_data* surface)
{
    oc_unimplemented();
    return ((oc_vec2){0});
}

oc_vec2 oc_linux_surface_contents_scaling(oc_surface_data* surface)
{
    oc_unimplemented();
    return ((oc_vec2){0});
}

bool oc_linux_surface_get_hidden(oc_surface_data* surface)
{
    oc_unimplemented();
    return (false);
}

void oc_linux_surface_set_hidden(oc_surface_data* surface, bool hidden)
{
    oc_unimplemented();
    return;
}

void* oc_linux_surface_native_layer(oc_surface_data* surface)
{
    return ((void*)(uintptr_t)surface->layer.x11WinId);
}

void oc_linux_surface_bring_to_front(oc_surface_data* surface)
{
    oc_unimplemented();
    return;
}

void oc_linux_surface_send_to_back(oc_surface_data* surface)
{
    oc_unimplemented();
    return;
}

void oc_surface_init_for_window(oc_surface_data* surface, oc_window_data* window)
{
    surface->getSize = oc_linux_surface_get_size;
    surface->contentsScaling = oc_linux_surface_contents_scaling;
    surface->getHidden = oc_linux_surface_get_hidden;
    surface->setHidden = oc_linux_surface_set_hidden;
    surface->nativeLayer = oc_linux_surface_native_layer;
    surface->bringToFront = oc_linux_surface_bring_to_front;
    surface->sendToBack = oc_linux_surface_send_to_back;

    surface->layer.x11WinId = window->linux.x11Id;
}

*/
