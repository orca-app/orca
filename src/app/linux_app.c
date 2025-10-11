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
#include <X11/Xlib-xcb.h>
#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>

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

static inline void* memz(void* buf, size_t n)
{
    return (memset(buf, 0, n));
}

void free(void* ptr);

void oc_init(void)
{
    memset(&oc_appData, 0, sizeof(oc_appData));
    oc_linux_app_data* linux = &oc_appData.linux;

    oc_init_common();

    oc_arena_scope scratch = oc_scratch_begin();

    oc_linux_x11* x11 = &linux->x11;

    x11->display = XOpenDisplay(NULL);
    OC_ASSERT(x11->display);

    xcb_connection_t* conn = XGetXCBConnection(x11->display);
    struct {
        const oc_str8 s;
        xcb_intern_atom_cookie_t cookie;
        xcb_atom_t* atom;
    } atoms[] = {
        #define DEF_ATOM(name)  \
            { .s = OC_STR8_LIT(#name), .cookie = 0, .atom = &x11->atoms. name }
        DEF_ATOM(_NET_WM_NAME),
        DEF_ATOM(_NET_WM_ICON_NAME),
        DEF_ATOM(UTF8_STRING),
        DEF_ATOM(WM_CHANGE_STATE),
        #undef DEF_ATOM
    };
    for(u64 i = 0; i < oc_array_size(atoms); i++)
    {
        atoms[i].cookie = xcb_intern_atom(conn, false, oc_str8_lp(atoms[i].s));
    }
    xcb_flush(conn);
    for(u64 i = 0; i < oc_array_size(atoms); i++)
    {
        xcb_intern_atom_reply_t *reply = NULL;
        reply = xcb_intern_atom_reply(conn, atoms[i].cookie, NULL);
        // TODO(pld): error handling, crash?
        OC_ASSERT(reply);
        memcpy(atoms[i].atom, &reply->atom, sizeof(reply->atom));
        free(reply);
    }

    {
        const xcb_setup_t* setup = xcb_get_setup(conn);
        xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
        OC_ASSERT(iter.rem > 0);
        xcb_screen_t* screen = iter.data;
        x11->rootWinId = screen->root;
    }


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
    OC_ASSERT(0 && "Unimplemented");
    return;
}
void oc_set_cursor(oc_mouse_cursor cursor)
{
    OC_ASSERT(0 && "Unimplemented");
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
    return window;
}
static void log_generic_event(const char* name, xcb_generic_event_t* ev)
{
    oc_log_info(
        "EVENT:\n"
        "  response_type: %s (%u)\n"
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
        name, ev->response_type,
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
        (u32)ev->full_sequence
    );
}
void oc_pump_events(f64 timeout)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);

    // TODO(pld): XCB doesn't have any helper for timed-wait event polling?
    OC_ASSERT(timeout == 0 || timeout == -1.0);

    xcb_generic_event_t* ev = NULL;
    if(timeout == 0.0)
    {
        ev = xcb_poll_for_event(conn);
    }
    else if(timeout == -1.0)
    {
        ev = xcb_wait_for_event(conn);
    }
    else
    {
        __builtin_unreachable();
    }
    while(ev)
    {
        const char* type = "(unk)";
        switch(ev->response_type)
        {
        case XCB_KEY_PRESS:
            type = "KeyPress";
            break;
        case XCB_KEY_RELEASE:
            type = "KeyRelease";
            break;
        case XCB_BUTTON_PRESS:
            type = "ButtonPress";
            break;
        case XCB_BUTTON_RELEASE:
            type = "ButtonRelease";
            break;
        case XCB_MOTION_NOTIFY:
            type = "MotionNotify";
            break;
        case XCB_ENTER_NOTIFY:
            type = "EnterNotify";
            break;
        case XCB_LEAVE_NOTIFY:
            type = "LeaveNotify";
            break;
        case XCB_FOCUS_IN:
            type = "FocusIn";
            break;
        case XCB_FOCUS_OUT:
            type = "FocusOut";
            break;
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
                windowData->linux.state = X11_WINDOW_STATE_ICONIC;
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
                windowData->linux.state = X11_WINDOW_STATE_NORMAL;
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
            type = "PropertyNotify";
            break;
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
            type = "ClientMessage";
            break;
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
    xcb_create_window_value_list_t attributes = {
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
        .event_mask = XCB_EVENT_MASK_KEY_PRESS |
            XCB_EVENT_MASK_KEY_RELEASE |
            XCB_EVENT_MASK_BUTTON_PRESS |
            XCB_EVENT_MASK_BUTTON_RELEASE |
            XCB_EVENT_MASK_ENTER_WINDOW |
            XCB_EVENT_MASK_LEAVE_WINDOW |
            XCB_EVENT_MASK_POINTER_MOTION |
            XCB_EVENT_MASK_POINTER_MOTION_HINT |
            XCB_EVENT_MASK_BUTTON_1_MOTION |
            XCB_EVENT_MASK_BUTTON_2_MOTION |
            XCB_EVENT_MASK_BUTTON_3_MOTION |
            XCB_EVENT_MASK_BUTTON_4_MOTION |
            XCB_EVENT_MASK_BUTTON_5_MOTION |
            XCB_EVENT_MASK_BUTTON_MOTION |
            XCB_EVENT_MASK_KEYMAP_STATE |
            //XCB_EVENT_MASK_EXPOSURE |
            //XCB_EVENT_MASK_VISIBILITY_CHANGE |
            XCB_EVENT_MASK_STRUCTURE_NOTIFY |
            //XCB_EVENT_MASK_RESIZE_REDIRECT |
            XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
            //XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
            XCB_EVENT_MASK_FOCUS_CHANGE |
            XCB_EVENT_MASK_PROPERTY_CHANGE |
            XCB_EVENT_MASK_COLOR_MAP_CHANGE |
            XCB_EVENT_MASK_OWNER_GRAB_BUTTON,
        .do_not_propogate_mask = 0,
        .colormap = XCB_COPY_FROM_PARENT,
        .cursor = XCB_CURSOR_NONE,
    };

    xcb_create_window_aux(conn, 0, winId, parentId,
        contentRect.x, contentRect.y, contentRect.w, contentRect.h,
        0, XCB_WINDOW_CLASS_INPUT_OUTPUT, XCB_COPY_FROM_PARENT,
        attributesMask, &attributes);
    static const u32 wmHints[9] = {
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
        XCB_ATOM_WM_HINTS, 8, sizeof(wmHints), &wmHints);
    static const u32 wmNormalHints[18] = {
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
        XCB_ATOM_WM_SIZE_HINTS, 8, sizeof(wmNormalHints), &wmNormalHints);
    // TODO(pld): WM_CLASS?
    // TODO(pld): WM_PROTOCOLS?
    // TODO(pld): CLIENT_MACHINE?
    xcb_flush(conn);

    oc_window_data* windowData = oc_window_alloc();
    windowData->linux.x11Id = winId;
    oc_window window = oc_window_handle_from_ptr(windowData);

    linux->x11.winIdToHandle[linux->x11.winIdToHandleLen++] = (x11_win_id_to_handle){
      .winId = winId,
      .handle = window,
    };

    oc_window_set_title(window, title);
    oc_window_show(window);

    return (window);
}

void oc_window_destroy(oc_window window)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);
    oc_window_data* windowData = oc_window_ptr_from_handle(window);

    xcb_destroy_window(conn, windowData->linux.x11Id);
    xcb_flush(conn);
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
    return ptr;
}

bool oc_window_should_close(oc_window window)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    return (windowData && windowData->shouldClose);
}
void oc_window_request_close(oc_window window)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData && !windowData->shouldClose) {
        windowData->shouldClose = true;
        // TODO(pld): enqueue destroy window?
        // TODO(pld): enqueue OC_EVENT_WINDOW_CLOSE
    }
    return;
}
void oc_window_cancel_close(oc_window window)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData) {
        windowData->shouldClose = false;
    }
    return;
}
// TODO(pld): thread safety? I mean, AFAICT this is run on the WASM thread,
// while pump_events is on the main thread. If the main thread destroy the
// window in the middle of this function, we would crash...
bool oc_window_is_hidden(oc_window window)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    return (windowData && windowData->linux.state != X11_WINDOW_STATE_NORMAL);
}
void oc_window_hide(oc_window window)
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
            .type = linux->x11.atoms.WM_CHANGE_STATE,
            .data.data32[0] = X11_WINDOW_STATE_ICONIC,
        };
        xcb_send_event(conn, false, linux->x11.rootWinId,
            XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
            XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
            (const char*)&msg);
        xcb_flush(conn);
    }
    return;
}
void oc_window_show(oc_window window)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        xcb_map_window(conn, windowData->linux.x11Id);
        xcb_flush(conn);
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
        uint32_t saturated_len = oc_clamp_high(title.len, UINT32_MAX);
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
        xcb_flush(conn);
    }
    return;
}
bool oc_window_is_minimized(oc_window window)
{
    OC_ASSERT(0 && "Unimplemented");
    return (false);
}
bool oc_window_is_maximized(oc_window window)
{
    OC_ASSERT(0 && "Unimplemented");
    return (false);
}
void oc_window_minimize(oc_window window)
{
    OC_ASSERT(0 && "Unimplemented");
    return;
}
void oc_window_maximize(oc_window window)
{
    OC_ASSERT(0 && "Unimplemented");
    return;
}
void oc_window_restore(oc_window window)
{
    OC_ASSERT(0 && "Unimplemented");
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
        xcb_get_input_focus_cookie_t cookie = xcb_get_input_focus(conn);
        xcb_flush(conn);
        xcb_get_input_focus_reply_t* reply = NULL;
        reply = xcb_get_input_focus_reply(conn, cookie, NULL);
        u32 focused_win_id = XCB_NONE;
        if(reply)
        {
            focused_win_id = reply->focus;
            free(reply);
        }
        focused = focused_win_id == windowData->linux.x11Id;
    }
    return (focused);
}
void oc_window_focus(oc_window window)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        xcb_set_input_focus(conn, XCB_NONE, windowData->linux.x11Id, XCB_CURRENT_TIME);
        xcb_flush(conn);
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
        xcb_set_input_focus(conn, XCB_NONE, XCB_NONE, XCB_CURRENT_TIME);
        xcb_flush(conn);
    }
    return;
}
void oc_window_send_to_back(oc_window window)
{
    OC_ASSERT(0 && "Unimplemented");
    return;
}
void oc_window_bring_to_front(oc_window window)
{
    // TODO(pld)
    return;
}
oc_rect oc_window_get_frame_rect(oc_window window)
{
    OC_ASSERT(0 && "Unimplemented");
    return ((oc_rect){0});
}
void oc_window_set_frame_rect(oc_window window, oc_rect rect)
{
    OC_ASSERT(0 && "Unimplemented");
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
        xcb_configure_window_value_list_t config = {
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
    OC_ASSERT(0 && "Unimplemented");
    return ((oc_rect){0});
}
oc_rect oc_window_frame_rect_for_content_rect(oc_rect contentRect, oc_window_style style)
{
    OC_ASSERT(0 && "Unimplemented");
    return ((oc_rect){0});
}
i32 oc_dispatch_on_main_thread_sync(oc_dispatch_proc proc, void* user)
{
    OC_ASSERT(0 && "Unimplemented");
    return (-1);
}
void oc_clipboard_clear(void)
{
    OC_ASSERT(0 && "Unimplemented");
    return;
}
void oc_clipboard_set_string(oc_str8 string)
{
    OC_ASSERT(0 && "Unimplemented");
    return;
}
oc_str8 oc_clipboard_get_string(oc_arena* arena)
{
    OC_ASSERT(0 && "Unimplemented");
    return ((oc_str8){0});
}
oc_str8 oc_clipboard_copy_string(oc_str8 backing)
{
    OC_ASSERT(0 && "Unimplemented");
    return ((oc_str8){0});
}
bool oc_clipboard_has_tag(const char* tag)
{
    OC_ASSERT(0 && "Unimplemented");
    return (false);
}
void oc_clipboard_set_data_for_tag(const char* tag, oc_str8 data)
{
    OC_ASSERT(0 && "Unimplemented");
    return;
}
oc_str8 oc_clipboard_get_data_for_tag(oc_arena* arena, const char* tag)
{
    OC_ASSERT(0 && "Unimplemented");
    return ((oc_str8){0});
}
oc_file_dialog_result oc_file_dialog_for_table(oc_arena* arena, oc_file_dialog_desc* desc, oc_file_table* table)
{
    OC_ASSERT(0 && "Unimplemented");
    return ((oc_file_dialog_result){0});
}
int oc_alert_popup(oc_str8 title, oc_str8 message, oc_str8_list options)
{
    OC_ASSERT(0 && "Unimplemented");
    return (-1);
}
int oc_file_move(oc_str8 from, oc_str8 to)
{
    OC_ASSERT(0 && "Unimplemented");
    return (-1);
}
int oc_file_remove(oc_str8 path)
{
    OC_ASSERT(0 && "Unimplemented");
    return (-1);
}
int oc_directory_create(oc_str8 path)
{
    OC_ASSERT(0 && "Unimplemented");
    return (-1);
}

/*
void oc_surface_cleanup(oc_surface_data* surface)
{
    OC_ASSERT(0 && "Unimplemented");
    return;
}

oc_vec2 oc_linux_surface_get_size(oc_surface_data* surface)
{
    OC_ASSERT(0 && "Unimplemented");
    return ((oc_vec2){0});
}

oc_vec2 oc_linux_surface_contents_scaling(oc_surface_data* surface)
{
    OC_ASSERT(0 && "Unimplemented");
    return ((oc_vec2){0});
}

bool oc_linux_surface_get_hidden(oc_surface_data* surface)
{
    OC_ASSERT(0 && "Unimplemented");
    return (false);
}

void oc_linux_surface_set_hidden(oc_surface_data* surface, bool hidden)
{
    OC_ASSERT(0 && "Unimplemented");
    return;
}

void* oc_linux_surface_native_layer(oc_surface_data* surface)
{
    return ((void*)(uintptr_t)surface->layer.x11WinId);
}

void oc_linux_surface_bring_to_front(oc_surface_data* surface)
{
    OC_ASSERT(0 && "Unimplemented");
    return;
}

void oc_linux_surface_send_to_back(oc_surface_data* surface)
{
    OC_ASSERT(0 && "Unimplemented");
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
