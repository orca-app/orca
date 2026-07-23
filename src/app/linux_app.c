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
#include <xcb/xcbext.h>
#include <xcb/sync.h>
#include <xcb/bigreq.h>
#include <xcb/xkb.h>
#include <xcb/xtest.h>
#include <xcb/xinput.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-compose.h>
#include <xkbcommon/xkbcommon-x11.h>

static_assert(oc_array_size_of_member(oc_linux_x11, winIdToHandle) == OC_APP_MAX_WINDOWS,
  "Must match OC_APP_MAX_WINDOWS");
static_assert(oc_array_size_of_member(oc_linux_x11, winIdToHandle) <= U32_MAX,
  "winIdToHandle's length must fit in winIdToHandleLen");

// TODO(pld): surface callbacks
// TODO(pld): file move
// TODO(pld): alert
// TODO(pld): file dialog
// TODO(pld): mouse cursor

static inline void* memz(void* buf, usize n)
{
    return (memset(buf, 0, n));
}
static inline void* memdup(void* buf, usize n)
{
    void* p = malloc(n);
    memcpy(p, buf, n);
    return (p);
}

static void oc_linux_enqueue_app_cmd(oc_linux_app_cmd* cmd)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);
    cmd->user.queued++;
    if(cmd->user.queued > 1)
    {
        oc_log_warning("requeued: cmd=%d, window=%llu, queued=%llu\n", cmd->cmd, cmd->window.h, cmd->user.queued);
    }
    xcb_client_message_event_t msg =
    {
        .response_type = XCB_CLIENT_MESSAGE,
        .format = 32,
        .window = linux->x11.controlWinId,
        .type = linux->x11.atoms.OC_X11_CLIENT_MESSAGE,
        .data.data32[0] = cmd->cmd,
    };
    OC_STATIC_ASSERT(sizeof(cmd->window) == 8);
    memcpy(&msg.data.data32[1], &cmd->window, 8);
    if(cmd->cmd == OC_X11_CLIENT_MESSAGE_WINDOW_SET_TITLE ||
        cmd->cmd == OC_X11_CLIENT_MESSAGE_WINDOW_SET_FRAME_RECT ||
        cmd->cmd == OC_X11_CLIENT_MESSAGE_WINDOW_SET_CONTENT_RECT ||
        cmd->cmd == OC_X11_CLIENT_MESSAGE_WINDOW_CENTER ||
        cmd->cmd == OC_X11_CLIENT_MESSAGE_DISPATCH_ON_MAIN_THREAD_SYNC ||
        cmd->cmd == OC_X11_CLIENT_MESSAGE_GET_PROPERTY ||
        cmd->cmd == OC_X11_CLIENT_MESSAGE_TRANSLATE_COORDINATES_TO_ROOT ||
        cmd->cmd == OC_X11_CLIENT_MESSAGE_GET_CLIPBOARD ||
        cmd->cmd == OC_X11_CLIENT_MESSAGE_SET_CLIPBOARD ||
        cmd->cmd == OC_X11_CLIENT_MESSAGE_SET_CLIPBOARD_TARGET ||
        cmd->cmd == OC_X11_CLIENT_MESSAGE_GET_SELECTION_OWNER ||
        cmd->cmd == OC_X11_CLIENT_MESSAGE_INTERN_ATOM ||
        cmd->cmd == OC_X11_CLIENT_MESSAGE_INTERN_ATOM_REPLY ||
        cmd->cmd == OC_X11_CLIENT_MESSAGE_FRAME_RECT_FOR_CONTENT_RECT ||
        cmd->cmd == OC_X11_CLIENT_MESSAGE_WINDOW_GET_FRAME_RECT)
    {
        int ok = oc_mutex_lock(linux->appCmdUserPoolMutex);
        OC_ASSERT(ok == 0);
        oc_linux_app_cmd_user* u = oc_pool_alloc(&linux->appCmdUserPool);
        OC_ASSERT(u);
        ok = oc_mutex_unlock(linux->appCmdUserPoolMutex);
        OC_ASSERT(ok == 0);
        memcpy(u, &cmd->user, sizeof(*u));
        OC_STATIC_ASSERT(sizeof(u) == 8);
        memcpy(&msg.data.data32[3], &u, 8);
    }
    xcb_send_event(conn, false, linux->x11.controlWinId, 0, (const char*)&msg);
}

struct oc_linux_app_cmd_completion
{
    u64 threadId;
    oc_mutex* mutex;
    oc_condition* cond;
};

static oc_linux_app_cmd_completion oc_linux_app_cmd_completion_create(void)
{
    oc_linux_app_cmd_completion c =
    {
        .threadId = oc_thread_self_id(),
        .mutex = oc_mutex_create(),
        .cond = oc_condition_create(),
    };
    OC_ASSERT(c.mutex);
    OC_ASSERT(c.cond);
    int ok = oc_mutex_lock(c.mutex);
    OC_ASSERT(ok == 0);
    return (c);
}

static void oc_linux_app_cmd_completion_wait(oc_linux_app_cmd_completion* c)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    OC_ASSERT(c->threadId == oc_thread_self_id());
    if(c->threadId == linux->mainThreadId)
    {
        while(!linux->mainThreadAppCmdCompletionSignaled)
        {
            oc_pump_events(-1);
        }
        linux->mainThreadAppCmdCompletionSignaled = false;
    }
    else
    {
        int ok = oc_condition_wait(c->cond, c->mutex);
        OC_ASSERT(ok == 0);
    }
}

static void oc_linux_app_cmd_completion_signal(oc_linux_app_cmd_completion* c)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    OC_ASSERT(linux->mainThreadId == oc_thread_self_id());
    if(c->threadId == linux->mainThreadId)
    {
        OC_ASSERT(!linux->mainThreadAppCmdCompletionSignaled);
        linux->mainThreadAppCmdCompletionSignaled = true;
    }
    else
    {
        int ok = oc_mutex_lock(c->mutex);
        OC_ASSERT(ok == 0);
        ok = oc_condition_signal(c->cond);
        OC_ASSERT(ok == 0);
        ok = oc_mutex_unlock(c->mutex);
        OC_ASSERT(ok == 0);
    }
}

static void oc_linux_app_cmd_completion_destroy(oc_linux_app_cmd_completion* c)
{
    OC_ASSERT(c->threadId == oc_thread_self_id());
    int ok = oc_mutex_unlock(c->mutex);
    OC_ASSERT(ok == 0);
    ok = oc_mutex_destroy(c->mutex);
    OC_ASSERT(ok == 0);
    ok = oc_condition_destroy(c->cond);
    OC_ASSERT(ok == 0);
}

static void ensure_xcb_flush(xcb_connection_t* conn)
{
    int ok = xcb_flush(conn);
    OC_ASSERT(ok > 0);
}

static void oc_linux_queue_event(oc_event* event)
{
    bool doQueue = false;
    if(oc_window_is_nil(event->window))
    {
        doQueue = true;
    }
    else
    {
        oc_window_data* windowData = oc_window_ptr_from_handle(event->window);
        OC_ASSERT(windowData);
        doQueue = windowData->linux.flags & OC_LINUX_WINDOW_EMIT_EVENTS;
    }
    if(doQueue)
    {
        oc_queue_event(event);
    }
}

void oc_init(void)
{
    if(oc_appData.init)
    {
        return;
    }

    memset(&oc_appData, 0, sizeof(oc_appData));
    oc_linux_app_data* linux = &oc_appData.linux;

    oc_init_common();
    oc_arena_init(&linux->persistentArena);
    linux->mainThreadId = oc_thread_self_id();
    linux->windowPoolMutex = oc_mutex_create();
    OC_ASSERT(linux->windowPoolMutex);
    oc_pool_init(&linux->appCmdUserPool, sizeof(oc_linux_app_cmd_user));
    linux->appCmdUserPoolMutex = oc_mutex_create();
    OC_ASSERT(linux->appCmdUserPoolMutex);
    linux->pumpedEventsCond = oc_condition_create();
    OC_ASSERT(linux->pumpedEventsCond);
    linux->pumpedEventsMutex = oc_mutex_create();
    OC_ASSERT(linux->pumpedEventsMutex);

    oc_arena_scope scratch = oc_scratch_begin();

    oc_linux_x11* x11 = &linux->x11;

    x11->display = XOpenDisplay(NULL);
    OC_ASSERT(x11->display);
    XSetEventQueueOwner(x11->display, XCBOwnsEventQueue);

    xcb_connection_t* conn = XGetXCBConnection(x11->display);

    /* Prefetch extensions we need into XCB's cache */
    {
        xcb_prefetch_extension_data(conn, &xcb_big_requests_id);
        xcb_prefetch_extension_data(conn, &xcb_sync_id);
        xcb_prefetch_extension_data(conn, &xcb_input_id);
        xcb_prefetch_extension_data(conn, &xcb_xkb_id);
    }

    /* Intern atoms */
    struct {
        const oc_str8 s;
        xcb_intern_atom_cookie_t cookie;
        xcb_atom_t* atom;
    } atoms[] =
    {
        #define DEF_ATOM(name, required)  \
            { .s = OC_STR8_LIT(#name), .cookie = 0, .atom = &x11->atoms. name },
        ATOM_LIST(DEF_ATOM)
        #undef DEF_ATOM
    };
    for(u64 i = 0; i < oc_array_size(atoms); i++)
    {
        atoms[i].cookie = xcb_intern_atom(conn, false, oc_str8_lp(atoms[i].s));
    }
    ensure_xcb_flush(conn);
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
        x11->maximumRequestSize = setup->maximum_request_length * 4;
    }

    /* Enable big requests */
    {
        const xcb_query_extension_reply_t* ext = xcb_get_extension_data(conn, &xcb_big_requests_id);
        OC_ASSERT(ext);
        OC_ASSERT(ext->present);
        OC_STATIC_ASSERT(XCB_BIGREQUESTS_MAJOR_VERSION == 0);
        OC_STATIC_ASSERT(XCB_BIGREQUESTS_MINOR_VERSION == 0);
        xcb_big_requests_enable_cookie_t cookie = xcb_big_requests_enable(conn);
        ensure_xcb_flush(conn);
        xcb_big_requests_enable_reply_t* reply = xcb_big_requests_enable_reply(conn, cookie, NULL);
        OC_ASSERT(reply);
        OC_ASSERT(reply->response_type == X11_RESPONSE_TYPE_REPLY);
        u64 n = reply->maximum_request_length * 4;
        OC_ASSERT(n > x11->maximumRequestSize);
        x11->maximumRequestSize = n;
        free(reply);
    }

    /* Initialize XSync extension */
    {
        const xcb_query_extension_reply_t* ext = xcb_get_extension_data(conn, &xcb_sync_id);
        OC_ASSERT(ext);
        OC_ASSERT(ext->present);
        OC_STATIC_ASSERT(XCB_SYNC_MAJOR_VERSION == 3);
        OC_STATIC_ASSERT(XCB_SYNC_MINOR_VERSION == 1);
        xcb_sync_initialize_cookie_t cookie =
            xcb_sync_initialize(conn, XCB_SYNC_MAJOR_VERSION, XCB_SYNC_MINOR_VERSION);
        ensure_xcb_flush(conn);
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
        ensure_xcb_flush(conn);
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
        ensure_xcb_flush(conn);
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
            #define DEF_REQUIRED_ATOM(name, required)  \
                (required ? x11->atoms.name : 0),
            ATOM_LIST(DEF_REQUIRED_ATOM)
            #undef DEF_REQUIRED_ATOM
        };
        u32 required_len = oc_array_size(required);
        u32 offset = 0;
        while(true)
        {
            xcb_get_property_cookie_t cookie = {0};
            cookie = xcb_get_property(conn, false, x11->rootWinId,
                x11->atoms._NET_SUPPORTED, XCB_ATOM_ATOM, offset, 64);
            ensure_xcb_flush(conn);
            xcb_get_property_reply_t* reply = xcb_get_property_reply(conn, cookie, NULL);
            OC_ASSERT(reply);
            OC_ASSERT(reply->response_type == X11_RESPONSE_TYPE_REPLY);
            xcb_atom_t* supported = xcb_get_property_value(reply);
            OC_ASSERT(supported);
            for(u32 i = 0; i < reply->value_len; i++, supported++)
            {
                for(u32 j = 0; j < required_len; j++)
                {
                    bool found = required[j] == *supported;
                    bool hole = !required[j];
                    if(found || hole)
                    {
                        memmove(&required[j], &required[j + 1], (required_len - j - 1) * sizeof(*required));
                        required_len--;
                        required[required_len] = 0;
                    }
                    if(found)
                    {
                        break;
                    }
                    else if(hole)
                    {
                        j--;
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

    /* XSettings */
    {
        x11->xsettings.doubleClickTime = 250;
        x11->xsettings.doubleClickDistance = 5 * 5;  // squared

        // TODO(pld): https://specifications.freedesktop.org/xsettings/0.5/
    }

    /* Control window. For events that don't need to target a specific client
     * window. */
    {
        u32 winId = xcb_generate_id(conn);
        u32 parentId = x11->rootWinId;
        xcb_create_window_value_list_t cwa =
        {
            .event_mask = XCB_EVENT_MASK_PROPERTY_CHANGE,
        };
        xcb_void_cookie_t cookie =
            xcb_create_window_aux_checked(conn, 0, winId, parentId, 0, 0, 1, 1, 0,
                XCB_WINDOW_CLASS_INPUT_ONLY, XCB_COPY_FROM_PARENT, XCB_CW_EVENT_MASK, &cwa);
        ensure_xcb_flush(conn);
        xcb_generic_error_t* e = xcb_request_check(conn, cookie);
        OC_ASSERT(!e);
        x11->controlWinId = winId;
    }

    /* Listen for changes on the root window properties we're interested in,
     * and retrieve their initial values. */
    {
        xcb_change_window_attributes_value_list_t cwa =
        {
            .event_mask = XCB_EVENT_MASK_PROPERTY_CHANGE,
        };
        xcb_change_window_attributes_aux(conn, x11->rootWinId, XCB_CW_EVENT_MASK, &cwa);
        xcb_get_property_cookie_t cookie =
            xcb_get_property(conn, false, x11->rootWinId, x11->atoms._NET_NUMBER_OF_DESKTOPS, XCB_ATOM_CARDINAL, 0, 1);
        oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
            .cmd = OC_X11_CLIENT_MESSAGE_GET_PROPERTY,
            .user.getProperty.prop = x11->atoms._NET_NUMBER_OF_DESKTOPS,
            .user.getProperty.cookie = cookie,
        });
        u32 n = oc_array_size(x11->netWorkarea) * 4;
        cookie = xcb_get_property(conn, false, x11->rootWinId, x11->atoms._NET_WORKAREA, XCB_ATOM_CARDINAL, 0, n);
        oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
            .cmd = OC_X11_CLIENT_MESSAGE_GET_PROPERTY,
            .user.getProperty.prop = x11->atoms._NET_WORKAREA,
            .user.getProperty.cookie = cookie,
        });
    }

    /* Initialize X Input Extension */
    {
        const xcb_query_extension_reply_t* ext = xcb_get_extension_data(conn, &xcb_input_id);
        OC_ASSERT(ext);
        OC_ASSERT(ext->present);
        OC_STATIC_ASSERT(XCB_INPUT_MAJOR_VERSION == 2);
        OC_STATIC_ASSERT(XCB_INPUT_MINOR_VERSION == 3);
        xcb_input_xi_query_version_cookie_t cookie = xcb_input_xi_query_version(conn, XCB_INPUT_MAJOR_VERSION, XCB_INPUT_MINOR_VERSION);
        ensure_xcb_flush(conn);
        xcb_input_xi_query_version_reply_t* reply = xcb_input_xi_query_version_reply(conn, cookie, NULL);
        OC_ASSERT(reply);
        OC_ASSERT(reply->response_type == X11_RESPONSE_TYPE_REPLY);
        OC_ASSERT(reply->major_version == 2);
        OC_ASSERT(reply->minor_version == 3);
        free(reply);
    }

    /* Keyboard and mouse */
    {
        // TODO(pld): use XInput extension for key press/relase and mouse events
        const xcb_query_extension_reply_t* ext = xcb_get_extension_data(conn, &xcb_xkb_id);
        OC_ASSERT(ext);
        OC_ASSERT(ext->present);
        OC_STATIC_ASSERT(XCB_XKB_MAJOR_VERSION == 1);
        OC_STATIC_ASSERT(XCB_XKB_MINOR_VERSION == 0);
        xcb_xkb_use_extension_cookie_t useExtensionCookie = xcb_xkb_use_extension(conn, XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);
        ensure_xcb_flush(conn);
        xcb_xkb_use_extension_reply_t* useExtensionReply = xcb_xkb_use_extension_reply(conn, useExtensionCookie, NULL);
        OC_ASSERT(useExtensionReply);
        OC_ASSERT(useExtensionReply->response_type == X11_RESPONSE_TYPE_REPLY);
        OC_ASSERT(useExtensionReply->supported);
        OC_ASSERT(useExtensionReply->serverMajor == XCB_XKB_MAJOR_VERSION);
        OC_ASSERT(useExtensionReply->serverMinor == XCB_XKB_MINOR_VERSION);
        free(useExtensionReply);

        x11->keyboard.xkbFirstEventCode = ext->first_event;
        x11->keyboard.ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
        OC_ASSERT(x11->keyboard.ctx);
        x11->keyboard.deviceId = xkb_x11_get_core_keyboard_device_id(conn);
        OC_ASSERT(x11->keyboard.deviceId != -1);

        u32 change = XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT;
        u32 value = change;
        xcb_xkb_per_client_flags_cookie_t perClientFlagsCookie = xcb_xkb_per_client_flags(conn, x11->keyboard.deviceId,
            change, value, 0, 0, 0);
        /* XCB's helper for XkbSelectEvents is buggy wrt * padding. Let's
         * implement it ourselves. */
        xcb_void_cookie_t selectEventsCookie = {0};
        {
            xcb_xkb_select_events_request_t req =
            {
                .major_opcode = ext->major_opcode,
                .minor_opcode = XCB_XKB_SELECT_EVENTS,
                .length = 0,
                .deviceSpec = x11->keyboard.deviceId,
                .affectWhich =
                    XCB_XKB_EVENT_TYPE_NEW_KEYBOARD_NOTIFY |
                    XCB_XKB_EVENT_TYPE_MAP_NOTIFY |
                    XCB_XKB_EVENT_TYPE_STATE_NOTIFY |
                    XCB_XKB_EVENT_TYPE_CONTROLS_NOTIFY |
                    XCB_XKB_EVENT_TYPE_INDICATOR_MAP_NOTIFY |
                    XCB_XKB_EVENT_TYPE_NAMES_NOTIFY |
                    XCB_XKB_EVENT_TYPE_COMPAT_MAP_NOTIFY,
                .clear = 0,
                .selectAll = 0,
                .affectMap = -1,
                .map = XCB_XKB_MAP_PART_KEY_TYPES |
                    XCB_XKB_MAP_PART_KEY_SYMS |
                    XCB_XKB_MAP_PART_MODIFIER_MAP |
                    XCB_XKB_MAP_PART_EXPLICIT_COMPONENTS |
                    XCB_XKB_MAP_PART_KEY_ACTIONS |
                    XCB_XKB_MAP_PART_VIRTUAL_MODS |
                    XCB_XKB_MAP_PART_VIRTUAL_MOD_MAP,
            };
            u8 details[52] = {0};
            usize off = 0;
            union { u8 c8; u16 c16; u32 c32; } affect, values;
            usize size = 0;
            /* XkbNewKeyboardNotify */
            affect.c16 =
                XCB_XKB_NKN_DETAIL_KEYCODES |
                XCB_XKB_NKN_DETAIL_GEOMETRY |
                XCB_XKB_NKN_DETAIL_DEVICE_ID;
            values.c16 =
                XCB_XKB_NKN_DETAIL_KEYCODES |
                XCB_XKB_NKN_DETAIL_DEVICE_ID;
            size = 2;
            OC_ASSERT(size * 2 <= sizeof(details) - off);
            memcpy(&details[off], &affect.c16, size), off += size;
            memcpy(&details[off], &values.c16, size), off += size;

            /* XkbStateNotify */
            affect.c16 =
                 XCB_XKB_STATE_PART_MODIFIER_STATE |
                 XCB_XKB_STATE_PART_MODIFIER_BASE |
                 XCB_XKB_STATE_PART_MODIFIER_LATCH |
                 XCB_XKB_STATE_PART_MODIFIER_LOCK |
                 XCB_XKB_STATE_PART_GROUP_STATE |
                 XCB_XKB_STATE_PART_GROUP_BASE |
                 XCB_XKB_STATE_PART_GROUP_LATCH |
                 XCB_XKB_STATE_PART_GROUP_LOCK |
                 XCB_XKB_STATE_PART_COMPAT_STATE |
                 XCB_XKB_STATE_PART_GRAB_MODS |
                 XCB_XKB_STATE_PART_COMPAT_GRAB_MODS |
                 XCB_XKB_STATE_PART_LOOKUP_MODS |
                 XCB_XKB_STATE_PART_COMPAT_LOOKUP_MODS |
                 XCB_XKB_STATE_PART_POINTER_BUTTONS;
            values.c16 =
                XCB_XKB_STATE_PART_MODIFIER_BASE |
                XCB_XKB_STATE_PART_MODIFIER_LATCH |
                XCB_XKB_STATE_PART_MODIFIER_LOCK |
                XCB_XKB_STATE_PART_GROUP_BASE |
                XCB_XKB_STATE_PART_GROUP_LATCH |
                XCB_XKB_STATE_PART_GROUP_LOCK;
            size = 2;
            OC_ASSERT(size * 2 <= sizeof(details) - off);
            memcpy(&details[off], &affect.c16, size), off += size;
            memcpy(&details[off], &values.c16, size), off += size;

            /* XkbControlsNotify */
            affect.c32 =
                XCB_XKB_BOOL_CTRL_REPEAT_KEYS |
                XCB_XKB_BOOL_CTRL_SLOW_KEYS |
                XCB_XKB_BOOL_CTRL_BOUNCE_KEYS |
                XCB_XKB_BOOL_CTRL_STICKY_KEYS |
                XCB_XKB_BOOL_CTRL_MOUSE_KEYS |
                XCB_XKB_BOOL_CTRL_MOUSE_KEYS_ACCEL |
                XCB_XKB_BOOL_CTRL_ACCESS_X_KEYS |
                XCB_XKB_BOOL_CTRL_ACCESS_X_TIMEOUT_MASK |
                XCB_XKB_BOOL_CTRL_ACCESS_X_FEEDBACK_MASK |
                XCB_XKB_BOOL_CTRL_AUDIBLE_BELL_MASK |
                XCB_XKB_BOOL_CTRL_OVERLAY_1_MASK |
                XCB_XKB_BOOL_CTRL_OVERLAY_2_MASK |
                XCB_XKB_BOOL_CTRL_IGNORE_GROUP_LOCK_MASK |
                XCB_XKB_CONTROL_GROUPS_WRAP |
                XCB_XKB_CONTROL_INTERNAL_MODS |
                XCB_XKB_CONTROL_IGNORE_LOCK_MODS |
                XCB_XKB_CONTROL_PER_KEY_REPEAT |
                XCB_XKB_CONTROL_CONTROLS_ENABLED;
            values.c32 =
                XCB_XKB_CONTROL_PER_KEY_REPEAT |
                XCB_XKB_CONTROL_CONTROLS_ENABLED;
            size = 4;
            OC_ASSERT(size * 2 <= sizeof(details) - off);
            memcpy(&details[off], &affect.c32, size), off += size;
            memcpy(&details[off], &values.c32, size), off += size;

            /* XkbIndicatorMapNotify */
            affect.c32 = -1;
            values.c32 = -1;
            size = 4;
            OC_ASSERT(size * 2 <= sizeof(details) - off);
            memcpy(&details[off], &affect.c32, size), off += size;
            memcpy(&details[off], &values.c32, size), off += size;

            /* XkbNamesNotify */
            affect.c16 =
                XCB_XKB_NAME_DETAIL_KEYCODES |
                XCB_XKB_NAME_DETAIL_GEOMETRY |
                XCB_XKB_NAME_DETAIL_SYMBOLS |
                XCB_XKB_NAME_DETAIL_PHYS_SYMBOLS |
                XCB_XKB_NAME_DETAIL_TYPES |
                XCB_XKB_NAME_DETAIL_COMPAT |
                XCB_XKB_NAME_DETAIL_KEY_TYPE_NAMES |
                XCB_XKB_NAME_DETAIL_KT_LEVEL_NAMES |
                XCB_XKB_NAME_DETAIL_INDICATOR_NAMES |
                XCB_XKB_NAME_DETAIL_KEY_NAMES |
                XCB_XKB_NAME_DETAIL_KEY_ALIASES |
                XCB_XKB_NAME_DETAIL_VIRTUAL_MOD_NAMES |
                XCB_XKB_NAME_DETAIL_GROUP_NAMES |
                XCB_XKB_NAME_DETAIL_RG_NAMES;
            values.c16 =
                XCB_XKB_NAME_DETAIL_KEYCODES |
                XCB_XKB_NAME_DETAIL_SYMBOLS |
                XCB_XKB_NAME_DETAIL_TYPES |
                XCB_XKB_NAME_DETAIL_COMPAT |
                XCB_XKB_NAME_DETAIL_KEY_TYPE_NAMES |
                XCB_XKB_NAME_DETAIL_KT_LEVEL_NAMES |
                XCB_XKB_NAME_DETAIL_INDICATOR_NAMES |
                XCB_XKB_NAME_DETAIL_KEY_NAMES |
                XCB_XKB_NAME_DETAIL_KEY_ALIASES |
                XCB_XKB_NAME_DETAIL_VIRTUAL_MOD_NAMES |
                XCB_XKB_NAME_DETAIL_GROUP_NAMES;
            size = 2;
            OC_ASSERT(size * 2 <= sizeof(details) - off);
            memcpy(&details[off], &affect.c16, size), off += size;
            memcpy(&details[off], &values.c16, size), off += size;

            /* XkbCompatMapNotify */
            affect.c8 =
                XCB_XKB_CM_DETAIL_SYM_INTERP |
                XCB_XKB_CM_DETAIL_GROUP_COMPAT;
            values.c8 =
                XCB_XKB_CM_DETAIL_SYM_INTERP;
            size = 1;
            OC_ASSERT(size * 2 <= sizeof(details) - off);
            memcpy(&details[off], &affect.c8, size), off += size;
            memcpy(&details[off], &values.c8, size), off += size;

            u8 pad[3] = {0};
            OC_STATIC_ASSERT((-sizeof(req) & 3) == 0);
            struct iovec vec[3] =
            {
                { .iov_base = &req, .iov_len = sizeof(req) },
                { .iov_base = details, .iov_len = off },
                { .iov_base = pad, .iov_len = -off & 3 },
            };
            for(usize i = 0; i < oc_array_size(vec); i++)  req.length += vec[i].iov_len;
            req.length /= 4;
            xcb_protocol_request_t desc = { .count = oc_array_size(vec), .isvoid = true };
            int flags = XCB_REQUEST_RAW | XCB_REQUEST_CHECKED;
            u32 seq = xcb_send_request(conn, flags, vec, &desc);
            OC_ASSERT(seq);
            selectEventsCookie.sequence = seq;
        }
        ensure_xcb_flush(conn);
        xcb_xkb_per_client_flags_reply_t* perClientFlagsReply = xcb_xkb_per_client_flags_reply(conn, perClientFlagsCookie, NULL);
        OC_ASSERT(perClientFlagsReply);
        OC_ASSERT(perClientFlagsReply->response_type == X11_RESPONSE_TYPE_REPLY);
        //OC_ASSERT(perClientFlagsReply->deviceID == x11->keyboard.deviceId);
        OC_ASSERT(perClientFlagsReply->supported & value);
        OC_ASSERT(perClientFlagsReply->value & value);
        OC_ASSERT(perClientFlagsReply->autoCtrls == 0);
        OC_ASSERT(perClientFlagsReply->autoCtrlsValues == 0);
        xcb_generic_error_t* err = xcb_request_check(conn, selectEventsCookie);
        OC_ASSERT(!err);
        x11->keyboard.reloadKeymap = true;

        /* Compose table and state for dead keys support. */
        const char* locale = getenv("LC_ALL");
        if(!locale || !locale[0])  locale = getenv("LC_CTYPE");
        if(!locale || !locale[0])  locale = getenv("LANG");
        if(!locale || !locale[0])  locale = "C";
        x11->keyboard.composeTable = xkb_compose_table_new_from_locale(x11->keyboard.ctx, locale, XKB_COMPOSE_COMPILE_NO_FLAGS);
        if(x11->keyboard.composeTable)
        {
            x11->keyboard.composeState = xkb_compose_state_new(x11->keyboard.composeTable, XKB_COMPOSE_STATE_NO_FLAGS);
            OC_ASSERT(x11->keyboard.composeState);
        }
    }

    /* X Test extension */
    {
        {
            xcb_query_extension_cookie_t cookie = xcb_query_extension(conn, sizeof(X11_XTEST_NAME) - 1, X11_XTEST_NAME);
            ensure_xcb_flush(conn);
            xcb_query_extension_reply_t* reply = xcb_query_extension_reply(conn, cookie, NULL);
            OC_ASSERT(reply);
            OC_ASSERT(reply->response_type == X11_RESPONSE_TYPE_REPLY);
            OC_ASSERT(reply->present);
            x11->xtestMajorCode = reply->major_opcode;
            free(reply);
        }
        {
            OC_STATIC_ASSERT(X11_XTEST_MAJOR_VERSION == 2);
            OC_STATIC_ASSERT(X11_XTEST_MINOR_VERSION == 2);
            x11_xtest_get_version_req req =
            {
                .majorCode = x11->xtestMajorCode,
                .minorCode = X11_XTEST_REQUEST_GET_VERSION,
                .len = 2,
                .clientMajor = X11_XTEST_MAJOR_VERSION,
                .clientMinor = X11_XTEST_MINOR_VERSION,
            };
            struct iovec vec = { .iov_base = &req, .iov_len = sizeof(req) };
            xcb_protocol_request_t desc = { .count = 1 };
            u32 seq = xcb_send_request(conn, XCB_REQUEST_RAW, &vec, &desc);
            OC_ASSERT(seq);
            ensure_xcb_flush(conn);
            x11_xtest_get_version_res* reply = xcb_wait_for_reply(conn, seq, NULL);
            OC_ASSERT(reply);
            OC_ASSERT(reply->code == X11_RESPONSE_TYPE_REPLY);
            OC_ASSERT(reply->serverMajor == X11_XTEST_MAJOR_VERSION);
            OC_ASSERT(reply->serverMinor == X11_XTEST_MINOR_VERSION);
            free(reply);
        }
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
                instanceName = OC_STR8(argv[i + 1]);
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
    u8* wmClass = oc_arena_push(&linux->persistentArena, wmClassLen);
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

    x11->wmClientMachine = oc_arena_push_zero(&linux->persistentArena, 256);
    int ok = gethostname((char*)x11->wmClientMachine, 256);
    OC_ASSERT(ok == 0);
    x11->wmClientMachineLen = strlen((char*)x11->wmClientMachine) + 1;

    const char *startupId = getenv("DESKTOP_STARTUP_ID");
    if(0 && startupId)
    {
        oc_str8 needle = OC_STR8("_TIME");
        oc_str8 haystack = OC_STR8(startupId);
        if(haystack.len > needle.len)
        {
            u64 i = haystack.len - needle.len - 1;
            oc_str8 slice = {0};
            for(; i != U64_MAX; i--)
            {
                slice = oc_str8_slice(haystack, i, haystack.len);
                if(oc_str8_eq(slice, needle))  break;
            }
            if(i != U64_MAX)
            {
                OC_ASSERT(slice.ptr[slice.len] == '\0');
                char *end = NULL;
                i32 n = strtol(slice.ptr, &end, 10);
                if(n > 0 && (n < I32_MAX || errno != ERANGE) && end == slice.ptr + slice.len)
                {
                    UNUSED xcb_timestamp_t timestamp = (xcb_timestamp_t)n;
                    //TODO(pld): use as _NET_WM_USER_TIME value on first window?
                }
            }
        }
    }
    if(0)
    {
        ok = unsetenv("DESKTOP_STARTUP_ID");
        OC_ASSERT(ok == 0);
    }

    // TODO(pld): aborts instead of OC_ASSERT, with errno for relevant cases
    // TODO(pld): after the above, create window and map window
    //
    // TODO(pld): clang-format
    // TODO(pld): build w/ gcc instead of clang?

    oc_scratch_end(scratch);

    oc_appData.init = true;
    return;
}

void oc_terminate(void)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    oc_linux_x11* x11 = &linux->x11;
    int ok = 0;

    xkb_compose_state_unref(x11->keyboard.composeState);
    xkb_compose_table_unref(x11->keyboard.composeTable);
    xkb_state_unref(x11->keyboard.state);
    xkb_keymap_unref(x11->keyboard.keymap);
    xkb_context_unref(x11->keyboard.ctx);
    XCloseDisplay(x11->display);
    oc_pool_cleanup(&linux->appCmdUserPool);
    ok = oc_mutex_destroy(linux->appCmdUserPoolMutex);
    OC_ASSERT(ok == 0);
    ok = oc_mutex_destroy(linux->windowPoolMutex);
    OC_ASSERT(ok == 0);
    oc_arena_cleanup(&linux->persistentArena);
    oc_terminate_common();

    oc_appData.init = false;
    return;
}

bool oc_should_quit(void)
{
    //TODO(pld): how stale can the value be given we don't use atomics?
    return (oc_appData.shouldQuit);
}

void oc_request_quit(void)
{
    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
        .cmd = OC_X11_CLIENT_MESSAGE_REQUEST_QUIT,
    });
}

void oc_cancel_quit(void)
{
    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
        .cmd = OC_X11_CLIENT_MESSAGE_CANCEL_QUIT,
    });
}

void oc_set_cursor(oc_mouse_cursor cursor)
{
    // TODO(pld): ARROW
    // TODO(pld): TEXT
    // TODO(pld): change window attributes of all windows, use cursor on new windows, xcursor compatible
    // TODO(pld): unsupported:
    // - OC_MOUSE_CURSOR_RESIZE_0
    // - OC_MOUSE_CURSOR_RESIZE_90
    // - OC_MOUSE_CURSOR_RESIZE_45
    // - OC_MOUSE_CURSOR_RESIZE_135
    oc_unimplemented();
    return;
}

/* This should only be called from pump_events_main_thread, i.e. from a single
 * thread, so that it remains thread safe. The winIdToHandle array itself
 * should only be modified from pump_events_main_thread too. */
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

static oc_window oc_window_create_linux(oc_rect contentRect, oc_str8 title, oc_window_style style, bool emitEvents);

static void window_update_last_user_activity_x(xcb_connection_t* conn, oc_window_data* windowData, xcb_timestamp_t ts)
{
    oc_linux_x11* x11 = &oc_appData.linux.x11;
    OC_ASSERT(windowData);
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, windowData->linux.x11Id,
        x11->atoms._NET_WM_USER_TIME, XCB_ATOM_CARDINAL, 32, 1, &ts);
    windowData->linux.netWmUserTime = ts;
    if(x11->latestUserTime < ts)  x11->latestUserTime = ts;
}

static void window_update_last_user_activity(xcb_connection_t* conn, oc_window window, xcb_timestamp_t ts)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)  window_update_last_user_activity_x(conn, windowData, ts);
}

typedef struct oc_convert_x11_button_res
{
    bool ok;
    oc_mouse_button button;
} oc_convert_x11_button_res;
static oc_convert_x11_button_res oc_convert_x11_button(xcb_button_t button)
{
    OC_ASSERT(button > 0);
    // TODO(pld): EXT1, EXT2?
    static const oc_mouse_button toMouseButton[] =
    {
        [1] = OC_MOUSE_LEFT,
        [2] = OC_MOUSE_MIDDLE,
        [3] = OC_MOUSE_RIGHT,
        [8] = OC_MOUSE_EXT1,
        [9] = OC_MOUSE_EXT2,
    };
    oc_convert_x11_button_res res = {0};
    if(button < oc_array_size(toMouseButton))
    {
        res.ok = true;
        res.button = toMouseButton[button];
    }
    return (res);
}

static oc_keymod_flags oc_mods_from_xkb_state(void)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    OC_ASSERT(!linux->x11.keyboard.reloadKeymap);
    xkb_state* state = linux->x11.keyboard.state;
    xkb_mod_index_t shift = linux->x11.keyboard.shiftModIndex;
    xkb_mod_index_t ctrl = linux->x11.keyboard.ctrlModIndex;
    xkb_mod_index_t alt = linux->x11.keyboard.altModIndex;
    xkb_mod_index_t cmd = linux->x11.keyboard.cmdModIndex;
    xkb_state_component c = XKB_STATE_MODS_EFFECTIVE;
    oc_keymod_flags mods = 0;
    if(xkb_state_mod_index_is_active(state, shift, c)) mods |= OC_KEYMOD_SHIFT;
    if(xkb_state_mod_index_is_active(state, ctrl, c)) mods |= OC_KEYMOD_CTRL | OC_KEYMOD_MAIN_MODIFIER;
    if(xkb_state_mod_index_is_active(state, alt, c)) mods |= OC_KEYMOD_ALT;
    if(xkb_state_mod_index_is_active(state, cmd, c)) mods |= OC_KEYMOD_CMD;
    return (mods);
}

static oc_scan_code oc_scan_code_from_xkb_keycode(xkb_keycode_t key)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    OC_ASSERT(key >= xkb_keymap_min_keycode(linux->x11.keyboard.keymap) &&
        key <= xkb_keymap_max_keycode(linux->x11.keyboard.keymap));
    return (oc_appData.scanCodes[key]);
}

// This is synchronous but should occur rarely enough that we don't care.
static void reload_x11_keymap(void)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);

    xkb_state_unref(linux->x11.keyboard.state), linux->x11.keyboard.state = NULL;
    xkb_keymap_unref(linux->x11.keyboard.keymap), linux->x11.keyboard.keymap = NULL;

    linux->x11.keyboard.keymap = xkb_x11_keymap_new_from_device(linux->x11.keyboard.ctx, conn,
        linux->x11.keyboard.deviceId, XKB_KEYMAP_COMPILE_NO_FLAGS);
    OC_ASSERT(linux->x11.keyboard.keymap);
    linux->x11.keyboard.state = xkb_x11_state_new_from_device(linux->x11.keyboard.keymap, conn,
        linux->x11.keyboard.deviceId);
    OC_ASSERT(linux->x11.keyboard.state);

    for(u64 i = 0; i < oc_array_size(oc_appData.scanCodes); i++)
    {
        oc_appData.scanCodes[i] = OC_SCANCODE_UNKNOWN;
    }
    static const char* xkbCodeNames[] =
    {
        [OC_SCANCODE_GRAVE_ACCENT] = "TLDE",
        [OC_SCANCODE_1] = "AE01",
        [OC_SCANCODE_2] = "AE02",
        [OC_SCANCODE_3] = "AE03",
        [OC_SCANCODE_4] = "AE04",
        [OC_SCANCODE_5] = "AE05",
        [OC_SCANCODE_6] = "AE06",
        [OC_SCANCODE_7] = "AE07",
        [OC_SCANCODE_8] = "AE08",
        [OC_SCANCODE_9] = "AE09",
        [OC_SCANCODE_0] = "AE10",
        [OC_SCANCODE_MINUS] = "AE11",
        [OC_SCANCODE_EQUAL] = "AE12",
        [OC_SCANCODE_BACKSPACE] = "BKSP",

        [OC_SCANCODE_TAB] = "TAB",
        [OC_SCANCODE_Q] = "AD01",
        [OC_SCANCODE_W] = "AD02",
        [OC_SCANCODE_E] = "AD03",
        [OC_SCANCODE_R] = "AD04",
        [OC_SCANCODE_T] = "AD05",
        [OC_SCANCODE_Y] = "AD06",
        [OC_SCANCODE_U] = "AD07",
        [OC_SCANCODE_I] = "AD08",
        [OC_SCANCODE_O] = "AD09",
        [OC_SCANCODE_P] = "AD10",
        [OC_SCANCODE_LEFT_BRACKET] = "AD11",
        [OC_SCANCODE_RIGHT_BRACKET] = "AD12",
        [OC_SCANCODE_BACKSLASH] = "BKSL",  // aka WORLD_3

        [OC_SCANCODE_CAPS_LOCK] = "CAPS",
        [OC_SCANCODE_A] = "AC01",
        [OC_SCANCODE_S] = "AC02",
        [OC_SCANCODE_D] = "AC03",
        [OC_SCANCODE_F] = "AC04",
        [OC_SCANCODE_G] = "AC05",
        [OC_SCANCODE_H] = "AC06",
        [OC_SCANCODE_J] = "AC07",
        [OC_SCANCODE_K] = "AC08",
        [OC_SCANCODE_L] = "AC09",
        [OC_SCANCODE_SEMICOLON] = "AC10",
        [OC_SCANCODE_APOSTROPHE] = "AC11",
        [OC_SCANCODE_ENTER] = "RTRN",

        [OC_SCANCODE_LEFT_SHIFT] = "LFSH",
        [OC_SCANCODE_WORLD_1] = "LSGT",
        [OC_SCANCODE_Z] = "AB01",
        [OC_SCANCODE_X] = "AB02",
        [OC_SCANCODE_C] = "AB03",
        [OC_SCANCODE_V] = "AB04",
        [OC_SCANCODE_B] = "AB05",
        [OC_SCANCODE_N] = "AB06",
        [OC_SCANCODE_M] = "AB07",
        [OC_SCANCODE_COMMA] = "AB08",
        [OC_SCANCODE_PERIOD] = "AB09",
        [OC_SCANCODE_SLASH] = "AB10",
        [OC_SCANCODE_WORLD_2] = "AB11",
        [OC_SCANCODE_RIGHT_SHIFT] = "RTSH",

        [OC_SCANCODE_LEFT_CONTROL] = "LCTL",
        [OC_SCANCODE_LEFT_SUPER] = "LWIN",
        [OC_SCANCODE_LEFT_ALT] = "LALT",
        [OC_SCANCODE_SPACE] = "SPCE",
        [OC_SCANCODE_RIGHT_ALT] = "RALT",
        [OC_SCANCODE_RIGHT_SUPER] = "RWIN",
        [OC_SCANCODE_MENU] = "MENU",
        [OC_SCANCODE_RIGHT_CONTROL] = "RCTL",

        [OC_SCANCODE_ESCAPE] = "ESC",
        [OC_SCANCODE_F1] = "FK01",
        [OC_SCANCODE_F2] = "FK02",
        [OC_SCANCODE_F3] = "FK03",
        [OC_SCANCODE_F4] = "FK04",
        [OC_SCANCODE_F5] = "FK05",
        [OC_SCANCODE_F6] = "FK06",
        [OC_SCANCODE_F7] = "FK07",
        [OC_SCANCODE_F8] = "FK08",
        [OC_SCANCODE_F9] = "FK09",
        [OC_SCANCODE_F10] = "FK10",
        [OC_SCANCODE_F11] = "FK11",
        [OC_SCANCODE_F12] = "FK12",
        [OC_SCANCODE_F13] = "FK13",
        [OC_SCANCODE_F14] = "FK14",
        [OC_SCANCODE_F15] = "FK15",
        [OC_SCANCODE_F16] = "FK16",
        [OC_SCANCODE_F17] = "FK17",
        [OC_SCANCODE_F18] = "FK18",
        [OC_SCANCODE_F19] = "FK19",
        [OC_SCANCODE_F20] = "FK20",
        [OC_SCANCODE_F21] = "FK21",
        [OC_SCANCODE_F22] = "FK22",
        [OC_SCANCODE_F23] = "FK23",
        [OC_SCANCODE_F24] = "FK24",
        [OC_SCANCODE_F25] = "FK25",

        [OC_SCANCODE_PRINT_SCREEN] = "PRSC",
        [OC_SCANCODE_SCROLL_LOCK] = "SCLK",
        [OC_SCANCODE_PAUSE] = "PAUS",

        [OC_SCANCODE_INSERT] = "INS",
        [OC_SCANCODE_HOME] = "HOME",
        [OC_SCANCODE_PAGE_UP] = "PGUP",
        [OC_SCANCODE_DELETE] = "DELE",
        [OC_SCANCODE_END] = "END",
        [OC_SCANCODE_PAGE_DOWN] = "PGDN",

        [OC_SCANCODE_UP] = "UP",
        [OC_SCANCODE_LEFT] = "LEFT",
        [OC_SCANCODE_DOWN] = "DOWN",
        [OC_SCANCODE_RIGHT] = "RGHT",

        [OC_SCANCODE_NUM_LOCK] = "NMLK",
        [OC_SCANCODE_KP_DIVIDE] = "KPDV",
        [OC_SCANCODE_KP_MULTIPLY] = "KPMU",
        [OC_SCANCODE_KP_SUBTRACT] = "KPSU",
        [OC_SCANCODE_KP_0] = "KP0",
        [OC_SCANCODE_KP_1] = "KP1",
        [OC_SCANCODE_KP_2] = "KP2",
        [OC_SCANCODE_KP_3] = "KP3",
        [OC_SCANCODE_KP_4] = "KP4",
        [OC_SCANCODE_KP_5] = "KP5",
        [OC_SCANCODE_KP_6] = "KP6",
        [OC_SCANCODE_KP_7] = "KP7",
        [OC_SCANCODE_KP_8] = "KP8",
        [OC_SCANCODE_KP_9] = "KP9",
        [OC_SCANCODE_KP_ADD] = "KPAD",
        [OC_SCANCODE_KP_DECIMAL] = "KPPT",  // or KPDC?
        [OC_SCANCODE_KP_ENTER] = "KPEN",
        [OC_SCANCODE_KP_EQUAL] = "KPEQ",
    };
    xkb_keycode_t minXkbCode = xkb_keymap_min_keycode(linux->x11.keyboard.keymap);
    xkb_keycode_t maxXkbCode = xkb_keymap_max_keycode(linux->x11.keyboard.keymap);
    OC_ASSERT(maxXkbCode < oc_array_size(oc_appData.scanCodes));
    for(oc_scan_code i = 0; i < oc_array_size(xkbCodeNames); i++)
    {
        const char* name = xkbCodeNames[i];
        if(name)
        {
            xkb_keycode_t xkbCode = xkb_keymap_key_by_name(linux->x11.keyboard.keymap, name);
            if(xkbCode != XKB_KEYCODE_INVALID)
            {
                OC_ASSERT(xkbCode > 0 && xkbCode <= maxXkbCode);
                oc_appData.scanCodes[xkbCode] = i;
            }
            else
            {
                oc_log_warning("X11 keycode not found in current keymap: %s\n", name);
            }
        }
    }

    //NOTE: default US layout
    memcpy(oc_appData.keyMap, oc_defaultKeyMap, sizeof(oc_key_code) * OC_SCANCODE_COUNT);

    static const struct { xkb_keysym_t keysym; oc_key_code keyCode; } keysymToKeyCode[] =
    {
        { XKB_KEY_space, OC_KEY_SPACE },
        { XKB_KEY_apostrophe, OC_KEY_APOSTROPHE },
        { XKB_KEY_comma, OC_KEY_COMMA },
        { XKB_KEY_minus, OC_KEY_MINUS },
        { XKB_KEY_period, OC_KEY_PERIOD },
        { XKB_KEY_slash, OC_KEY_SLASH },
        { XKB_KEY_0, OC_KEY_0 },
        { XKB_KEY_1, OC_KEY_1 },
        { XKB_KEY_2, OC_KEY_2 },
        { XKB_KEY_3, OC_KEY_3 },
        { XKB_KEY_4, OC_KEY_4 },
        { XKB_KEY_5, OC_KEY_5 },
        { XKB_KEY_6, OC_KEY_6 },
        { XKB_KEY_7, OC_KEY_7 },
        { XKB_KEY_8, OC_KEY_8 },
        { XKB_KEY_9, OC_KEY_9 },
        { XKB_KEY_semicolon, OC_KEY_SEMICOLON },
        { XKB_KEY_equal, OC_KEY_EQUAL },
        { XKB_KEY_bracketleft, OC_KEY_LEFT_BRACKET },
        { XKB_KEY_backslash, OC_KEY_BACKSLASH },
        { XKB_KEY_bracketright, OC_KEY_RIGHT_BRACKET },
        { XKB_KEY_grave, OC_KEY_GRAVE_ACCENT },
        { XKB_KEY_a, OC_KEY_A },
        { XKB_KEY_b, OC_KEY_B },
        { XKB_KEY_c, OC_KEY_C },
        { XKB_KEY_d, OC_KEY_D },
        { XKB_KEY_e, OC_KEY_E },
        { XKB_KEY_f, OC_KEY_F },
        { XKB_KEY_g, OC_KEY_G },
        { XKB_KEY_h, OC_KEY_H },
        { XKB_KEY_i, OC_KEY_I },
        { XKB_KEY_j, OC_KEY_J },
        { XKB_KEY_k, OC_KEY_K },
        { XKB_KEY_l, OC_KEY_L },
        { XKB_KEY_m, OC_KEY_M },
        { XKB_KEY_n, OC_KEY_N },
        { XKB_KEY_o, OC_KEY_O },
        { XKB_KEY_p, OC_KEY_P },
        { XKB_KEY_q, OC_KEY_Q },
        { XKB_KEY_r, OC_KEY_R },
        { XKB_KEY_s, OC_KEY_S },
        { XKB_KEY_t, OC_KEY_T },
        { XKB_KEY_u, OC_KEY_U },
        { XKB_KEY_v, OC_KEY_V },
        { XKB_KEY_w, OC_KEY_W },
        { XKB_KEY_x, OC_KEY_X },
        { XKB_KEY_y, OC_KEY_Y },
        { XKB_KEY_z, OC_KEY_Z },
        { XKB_KEY_less, OC_KEY_WORLD_1 },
        { XKB_KEY_underscore, OC_KEY_WORLD_2 },
        { XKB_KEY_Escape, OC_KEY_ESCAPE },
        { XKB_KEY_Return, OC_KEY_ENTER },
        { XKB_KEY_Tab, OC_KEY_TAB },
        { XKB_KEY_BackSpace, OC_KEY_BACKSPACE },
        { XKB_KEY_Insert, OC_KEY_INSERT },
        { XKB_KEY_Delete, OC_KEY_DELETE },
        { XKB_KEY_Right, OC_KEY_RIGHT },
        { XKB_KEY_Left, OC_KEY_LEFT },
        { XKB_KEY_Down, OC_KEY_DOWN },
        { XKB_KEY_Up, OC_KEY_UP },
        { XKB_KEY_Page_Up, OC_KEY_PAGE_UP },
        { XKB_KEY_Page_Down, OC_KEY_PAGE_DOWN },
        { XKB_KEY_Home, OC_KEY_HOME },
        { XKB_KEY_End, OC_KEY_END },
        { XKB_KEY_Caps_Lock, OC_KEY_CAPS_LOCK },
        { XKB_KEY_Scroll_Lock, OC_KEY_SCROLL_LOCK },
        { XKB_KEY_Num_Lock, OC_KEY_NUM_LOCK },
        { XKB_KEY_Print, OC_KEY_PRINT_SCREEN },
        { XKB_KEY_Pause, OC_KEY_PAUSE },
        { XKB_KEY_F1, OC_KEY_F1 },
        { XKB_KEY_F2, OC_KEY_F2 },
        { XKB_KEY_F3, OC_KEY_F3 },
        { XKB_KEY_F4, OC_KEY_F4 },
        { XKB_KEY_F5, OC_KEY_F5 },
        { XKB_KEY_F6, OC_KEY_F6 },
        { XKB_KEY_F7, OC_KEY_F7 },
        { XKB_KEY_F8, OC_KEY_F8 },
        { XKB_KEY_F9, OC_KEY_F9 },
        { XKB_KEY_F10, OC_KEY_F10 },
        { XKB_KEY_F11, OC_KEY_F11 },
        { XKB_KEY_F12, OC_KEY_F12 },
        { XKB_KEY_F13, OC_KEY_F13 },
        { XKB_KEY_F14, OC_KEY_F14 },
        { XKB_KEY_F15, OC_KEY_F15 },
        { XKB_KEY_F16, OC_KEY_F16 },
        { XKB_KEY_F17, OC_KEY_F17 },
        { XKB_KEY_F18, OC_KEY_F18 },
        { XKB_KEY_F19, OC_KEY_F19 },
        { XKB_KEY_F20, OC_KEY_F20 },
        { XKB_KEY_F21, OC_KEY_F21 },
        { XKB_KEY_F22, OC_KEY_F22 },
        { XKB_KEY_F23, OC_KEY_F23 },
        { XKB_KEY_F24, OC_KEY_F24 },
        { XKB_KEY_F25, OC_KEY_F25 },
        { XKB_KEY_KP_0, OC_KEY_KP_0 },
        { XKB_KEY_KP_1, OC_KEY_KP_1 },
        { XKB_KEY_KP_2, OC_KEY_KP_2 },
        { XKB_KEY_KP_3, OC_KEY_KP_3 },
        { XKB_KEY_KP_4, OC_KEY_KP_4 },
        { XKB_KEY_KP_5, OC_KEY_KP_5 },
        { XKB_KEY_KP_6, OC_KEY_KP_6 },
        { XKB_KEY_KP_7, OC_KEY_KP_7 },
        { XKB_KEY_KP_8, OC_KEY_KP_8 },
        { XKB_KEY_KP_9, OC_KEY_KP_9 },
        { XKB_KEY_KP_Decimal, OC_KEY_KP_DECIMAL },
        { XKB_KEY_KP_Divide, OC_KEY_KP_DIVIDE },
        { XKB_KEY_KP_Multiply, OC_KEY_KP_MULTIPLY },
        { XKB_KEY_KP_Subtract, OC_KEY_KP_SUBTRACT },
        { XKB_KEY_KP_Add, OC_KEY_KP_ADD },
        { XKB_KEY_KP_Enter, OC_KEY_KP_ENTER },
        { XKB_KEY_KP_Equal, OC_KEY_KP_EQUAL },
        { XKB_KEY_Shift_L, OC_KEY_LEFT_SHIFT },
        { XKB_KEY_Control_L, OC_KEY_LEFT_CONTROL },
        { XKB_KEY_Alt_L, OC_KEY_LEFT_ALT },
        { XKB_KEY_Super_L, OC_KEY_LEFT_SUPER },
        { XKB_KEY_Shift_R, OC_KEY_RIGHT_SHIFT },
        { XKB_KEY_Control_R, OC_KEY_RIGHT_CONTROL },
        { XKB_KEY_Alt_R, OC_KEY_RIGHT_ALT },
        { XKB_KEY_Super_R, OC_KEY_RIGHT_SUPER },
        { XKB_KEY_Menu, OC_KEY_MENU },
    };
    for(i32 xkbCode = minXkbCode; xkbCode < maxXkbCode; xkbCode++)
    {
        oc_key_code keyCode = OC_KEY_UNKNOWN;
        oc_scan_code scanCode = oc_appData.scanCodes[xkbCode];
        if(scanCode != OC_SCANCODE_UNKNOWN)
        {
            const xkb_keysym_t* syms = NULL;
            int symsLen = 0;
            symsLen = xkb_keymap_key_get_syms_by_level(linux->x11.keyboard.keymap, xkbCode, 0, 0, &syms);
            if(symsLen > 0)
            {
                OC_ASSERT(syms);
                xkb_keysym_t keysym = syms[0];
                for(usize i = 0; i < oc_array_size(keysymToKeyCode); i++)
                {
                    if(keysymToKeyCode[i].keysym == keysym)
                    {
                        oc_appData.keyMap[scanCode] = keysymToKeyCode[i].keyCode;
                    }
                }
            }
        }

        //NOTE fix digit row for azerty keyboards
        bool azerty = true;
        for(int scanCode = OC_SCANCODE_0; scanCode <= OC_SCANCODE_9; scanCode++)
        {
            if(oc_appData.keyMap[scanCode] >= OC_KEY_0 && oc_appData.keyMap[scanCode] <= OC_KEY_9)
            {
                azerty = false;
                break;
            }
        }
        if(azerty)
        {
            for(int scanCode = OC_SCANCODE_0; scanCode <= OC_SCANCODE_9; scanCode++)
            {
                oc_appData.keyMap[scanCode] = OC_KEY_0 + (scanCode - OC_SCANCODE_0);
            }
        }
    }

    linux->x11.keyboard.shiftModIndex = xkb_keymap_mod_get_index(linux->x11.keyboard.keymap, XKB_MOD_NAME_SHIFT);
    linux->x11.keyboard.ctrlModIndex = xkb_keymap_mod_get_index(linux->x11.keyboard.keymap, XKB_MOD_NAME_CTRL);
    // FIXME(pld): requires xkbcommon >=1.8
    //linux->x11.keyboard.altModIndex = xkb_keymap_mod_get_index(linux->x11.keyboard.keymap, XKB_VMOD_NAME_ALT);
    //linux->x11.keyboard.cmdModIndex = xkb_keymap_mod_get_index(linux->x11.keyboard.keymap, XKB_VMOD_NAME_SUPER);
    linux->x11.keyboard.altModIndex = xkb_keymap_mod_get_index(linux->x11.keyboard.keymap, XKB_MOD_NAME_ALT);
    linux->x11.keyboard.cmdModIndex = xkb_keymap_mod_get_index(linux->x11.keyboard.keymap, XKB_MOD_NAME_LOGO);
}

static void reload_x11_keymap_if_needed(void)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    if(linux->x11.keyboard.reloadKeymap)
    {
        reload_x11_keymap();
        linux->x11.keyboard.reloadKeymap = false;
    }
}

static bool oc_linux_is_scan_code_depressed(oc_scan_code scanCode)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    OC_ASSERT(!linux->x11.keyboard.reloadKeymap);
    OC_ASSERT(scanCode >= 0 && scanCode < OC_SCANCODE_COUNT);
    return (linux->x11.keyboard.depressedScanCodes[scanCode / 8]) & (1 << scanCode % 8);
}

static void oc_linux_set_scan_code_depressed(oc_scan_code scanCode, bool set)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    OC_ASSERT(!linux->x11.keyboard.reloadKeymap);
    OC_ASSERT(scanCode >= 0 && scanCode < OC_SCANCODE_COUNT);
    if(set)  linux->x11.keyboard.depressedScanCodes[scanCode / 8] |= (1 << scanCode % 8);
    else  linux->x11.keyboard.depressedScanCodes[scanCode / 8] &= ~(1 << scanCode % 8);
}

static void oc_linux_dispatch_sync_request_refcount_dec(oc_linux_dispatch_sync_request* req)
{
    OC_ASSERT(req);
    req->refcount--;
    if(req->refcount == 0)
    {
        /* No other user, free it. */
        oc_mutex_unlock(req->mutex);
        int ok = oc_mutex_destroy(req->mutex);
        OC_ASSERT(ok == 0);
        ok = oc_condition_destroy(req->cond);
        OC_ASSERT(ok == 0);
        free(req);
    }
    else
    {
        int ok = oc_mutex_unlock(req->mutex);
        OC_ASSERT(ok == 0);
    }
}

static void log_event(xcb_generic_event_t* ev)
{
    if(0 && (ev->response_type & 0x7f) == XCB_CLIENT_MESSAGE)  return;
    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8_list s;
    oc_str8_list_init(&s);
    oc_str8_list_pushf(scratch.arena, &s, "EVENT:\n");
    const char* name = "(unk)";
    static const char* names[] =
    {
        "X11_RESPONSE_TYPE_ERROR",
        "X11_RESPONSE_TYPE_REPLY",
        "XCB_KEY_PRESS",
        "XCB_KEY_RELEASE",
        "XCB_BUTTON_PRESS",
        "XCB_BUTTON_RELEASE",
        "XCB_MOTION_NOTIFY",
        "XCB_ENTER_NOTIFY",
        "XCB_LEAVE_NOTIFY",
        "XCB_FOCUS_IN",
        "XCB_FOCUS_OUT",
        "XCB_KEYMAP_NOTIFY",
        "XCB_EXPOSE",
        "XCB_GRAPHICS_EXPOSURE",
        "XCB_NO_EXPOSURE",
        "XCB_VISIBILITY_NOTIFY",
        "XCB_CREATE_NOTIFY",
        "XCB_DESTROY_NOTIFY",
        "XCB_UNMAP_NOTIFY",
        "XCB_MAP_NOTIFY",
        "XCB_MAP_REQUEST",
        "XCB_REPARENT_NOTIFY",
        "XCB_CONFIGURE_NOTIFY",
        "XCB_CONFIGURE_REQUEST",
        "XCB_GRAVITY_NOTIFY",
        "XCB_RESIZE_REQUEST",
        "XCB_CIRCULATE_NOTIFY",
        "XCB_CIRCULATE_REQUEST",
        "XCB_PROPERTY_NOTIFY",
        "XCB_SELECTION_CLEAR",
        "XCB_SELECTION_REQUEST",
        "XCB_SELECTION_NOTIFY",
        "XCB_COLORMAP_NOTIFY",
        "XCB_CLIENT_MESSAGE",
        "XCB_MAPPING_NOTIFY",
    };
    if((ev->response_type & 0x7f) < oc_array_size(names)) {
        name = names[ev->response_type & 0x7f];
    }
    oc_str8_list_pushf(scratch.arena, &s, "  response_type: %s (%u", name, ev->response_type & 0x7f);
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
enum { X11_BUTTON_WHEEL_UP = 4, X11_BUTTON_WHEEL_DOWN = 5, X11_BUTTON_WHEEL_LEFT = 6, X11_BUTTON_WHEEL_RIGHT = 7 };
static void oc_pump_events_main_thread(f64 timeout)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);

    OC_ASSERT(oc_thread_self_id() == linux->mainThreadId);

    ensure_xcb_flush(conn);

    xcb_generic_event_t* ev = xcb_poll_for_event(conn);
    if(!ev && timeout != 0.0)
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
            if(end < 0.0)
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
    xcb_generic_event_t* forceNextEvent = NULL;
    while(ev)
    {
        bool synthetic = ev->response_type & 0x80;
        u8 eventCode = ev->response_type & 0x7f;
        switch(eventCode)
        {
        case XCB_KEY_PRESS:
        {
            xcb_key_press_event_t* noti = (xcb_key_press_event_t*)ev;
            oc_window window = window_handle_from_x11_id(noti->event);
            window_update_last_user_activity(conn, window, noti->time);

            if(0) oc_log_info("key press: root=%d/%d, event=%d/%d, detail=%d, state=%d, time=%d\n",
                noti->root_x, noti->root_y, noti->event_x, noti->event_y, noti->detail, noti->state, noti->time);

            reload_x11_keymap_if_needed();

            oc_key_action action = OC_KEY_PRESS;
            oc_scan_code scanCode = oc_scan_code_from_xkb_keycode(noti->detail);
            bool depressed = oc_linux_is_scan_code_depressed(scanCode);
            if(depressed && xkb_keymap_key_repeats(linux->x11.keyboard.keymap, noti->detail))
            {
                action = OC_KEY_REPEAT;
            }
            oc_event event;
            memz(&event, sizeof(event));
            event.type = OC_EVENT_KEYBOARD_KEY;
            event.window = window;
            event.key.action = action;
            event.key.scanCode = scanCode;
            event.key.keyCode = oc_scancode_to_keycode(scanCode);
            event.key.mods = oc_mods_from_xkb_state();
            oc_linux_queue_event(&event);

            // FIXME(pld): This does Control keysym transformation (e.g. Ctrl-h
            // will translate to backspace. Do we want it?
            char seqBuf[64];
            int seqLen = 0;
            bool composeDidConsume = false;
            if(linux->x11.keyboard.composeState)
            {
                xkb_keysym_t keysym = xkb_state_key_get_one_sym(linux->x11.keyboard.state, noti->detail);
                xkb_compose_feed_result feed = xkb_compose_state_feed(linux->x11.keyboard.composeState, keysym);
                xkb_compose_status status = xkb_compose_state_get_status(linux->x11.keyboard.composeState);
                if(feed == XKB_COMPOSE_FEED_ACCEPTED)
                {
                    if(status == XKB_COMPOSE_NOTHING)
                    {
                        composeDidConsume = false;
                    }
                    else if(status == XKB_COMPOSE_COMPOSING)
                    {
                        composeDidConsume = true;
                    }
                    else if(status == XKB_COMPOSE_COMPOSED)
                    {
                        seqLen = xkb_compose_state_get_utf8(linux->x11.keyboard.composeState, seqBuf, sizeof(seqBuf));
                        xkb_compose_state_reset(linux->x11.keyboard.composeState);
                        composeDidConsume = true;
                    }
                    else if(status == XKB_COMPOSE_CANCELLED)
                    {
                        /* We swallow the cancelling keysym, to match libX11's
                         * behaviour. In contrast, Windows replays the entire
                         * sequence, e.g. ^ then b will produce ^b. */
                        xkb_compose_state_reset(linux->x11.keyboard.composeState);
                        composeDidConsume = true;
                    }
                    else
                    {
                        oc_unreachable();
                    }
                }
                else if(feed == XKB_COMPOSE_FEED_IGNORED)
                {
                    OC_ASSERT(status != XKB_COMPOSE_COMPOSED && status != XKB_COMPOSE_CANCELLED);
                    composeDidConsume = status == XKB_COMPOSE_COMPOSING;
                }
                else
                {
                    oc_unreachable();
                }
            }
            if(!composeDidConsume)
            {
                seqLen = xkb_state_key_get_utf8(linux->x11.keyboard.state, noti->detail, seqBuf, sizeof(seqBuf));
            }
            OC_ASSERT(seqLen >= 0);
            OC_ASSERT(seqLen < sizeof(seqBuf));
            oc_str8 seq = oc_str8_from_buffer(seqLen, seqBuf);
            memz(&event, sizeof(event));
            event.type = OC_EVENT_KEYBOARD_CHAR;
            event.window = window;
            for(u32 off = 0; off < seq.len;)
            {
                oc_utf8_dec dec = oc_utf8_decode_at(seq, off);
                OC_ASSERT(dec.status == OC_UTF8_OK);
                OC_ASSERT(dec.size <= sizeof(event.character.sequence));

                event.character.codepoint = dec.codepoint;
                memcpy(event.character.sequence, &seq.ptr[off], dec.size);
                memz(&event.character.sequence[dec.size], sizeof(event.character.sequence) - dec.size);
                event.character.seqLen = dec.size;
                oc_linux_queue_event(&event);

                off += dec.size;
            }

            oc_linux_set_scan_code_depressed(scanCode, true);
        } break;
        case XCB_KEY_RELEASE:
        {
            xcb_key_release_event_t* noti = (xcb_key_release_event_t*)ev;
            oc_window window = window_handle_from_x11_id(noti->event);

            oc_log_info("key release: root=%d/%d, event=%d/%d, detail=%d, state=%d, time=%d\n",
                noti->root_x, noti->root_y, noti->event_x, noti->event_y, noti->detail, noti->state, noti->time);

            reload_x11_keymap_if_needed();

            oc_scan_code scanCode = oc_scan_code_from_xkb_keycode(noti->detail);
            oc_event event;
            memz(&event, sizeof(event));
            event.type = OC_EVENT_KEYBOARD_KEY;
            event.window = window;
            event.key.action = OC_KEY_RELEASE;
            event.key.scanCode = scanCode;
            event.key.keyCode = oc_scancode_to_keycode(scanCode);
            // TODO(pld): When releasing a modifier, we receive the KeyRelease
            // event before the XkbStateNotify one. The event mods will include
            // it, do we want it in or exclude it?
            event.key.mods = oc_mods_from_xkb_state();
            oc_linux_queue_event(&event);

            oc_linux_set_scan_code_depressed(scanCode, false);
        } break;
        case XCB_BUTTON_PRESS:
        {
            xcb_button_press_event_t* noti = (xcb_button_press_event_t*)ev;
            oc_window window = window_handle_from_x11_id(noti->event);
            OC_ASSERT(!oc_window_is_nil(window));
            window_update_last_user_activity(conn, window, noti->time);

            oc_log_info("button press: root=%d/%d, event=%d/%d, detail=%d, state=%d, time=%d\n",
                noti->root_x, noti->root_y, noti->event_x, noti->event_y, noti->detail, noti->state, noti->time);

            reload_x11_keymap_if_needed();

            if(noti->detail == X11_BUTTON_WHEEL_UP || noti->detail == X11_BUTTON_WHEEL_DOWN ||
                noti->detail == X11_BUTTON_WHEEL_LEFT || noti->detail == X11_BUTTON_WHEEL_RIGHT)
            {
                // See https://source.chromium.org/chromium/chromium/src/+/main:ui/events/x/events_x_utils.cc;l=40;drc=fb19e42e9bcf7a8a675e8691ee6f2de9792b8718
                oc_vec2 delta = {0};
                f32 n = 120.0f;
                if(noti->detail == X11_BUTTON_WHEEL_UP)  delta.y = -n;
                else if(noti->detail == X11_BUTTON_WHEEL_DOWN)  delta.y = n;
                else if(noti->detail == X11_BUTTON_WHEEL_LEFT)  delta.x = -n;
                else if(noti->detail == X11_BUTTON_WHEEL_RIGHT)  delta.x = n;
                else  oc_unreachable();

                oc_event event;
                memz(&event, sizeof(event));
                event.type = OC_EVENT_MOUSE_WHEEL;
                event.window = window;
                event.mouse.deltaX = delta.x;
                event.mouse.deltaY = delta.y;
                event.mouse.mods = oc_mods_from_xkb_state();
                oc_linux_queue_event(&event);
                break;
            }

            oc_convert_x11_button_res converted = oc_convert_x11_button(noti->detail);
            if(!converted.ok)  break;

            OC_ASSERT(noti->same_screen);
            oc_vec2 pos = { .x = (f32)noti->event_x, .y = (f32)noti->event_y };

            xcb_timestamp_t elapsed = noti->time - linux->x11.mouse.lastClickTime;
            f32 distance = 0.0;
            {
                f32 x = linux->x11.mouse.lastClickPos.x - pos.x;
                f32 y = linux->x11.mouse.lastClickPos.y - pos.y;
                distance = (x * x) + (y * y);
                if(0) oc_log_info("button press: lastClickPos=%f/%f, pos=%f/%f, distance=%f\n",
                    (double)linux->x11.mouse.lastClickPos.x,
                    (double)linux->x11.mouse.lastClickPos.y,
                    (double)pos.x, (double)pos.y, (double)distance);
            }
            if(noti->event != linux->x11.mouse.lastClickWinId ||
                noti->detail != linux->x11.mouse.lastClickButton ||
                elapsed > linux->x11.xsettings.doubleClickTime ||
                distance > linux->x11.xsettings.doubleClickDistance)
            {
                if(0) oc_log_info("button press: reset clickCount\n");
                linux->x11.mouse.clickCount = 0;
            }
            linux->x11.mouse.lastClickWinId = noti->event;
            linux->x11.mouse.lastClickButton = noti->detail;
            linux->x11.mouse.lastClickTime = noti->time;
            linux->x11.mouse.lastClickPos = pos;
            linux->x11.mouse.clickCount++;

            oc_event event;
            memz(&event, sizeof(event));
            event.type = OC_EVENT_MOUSE_BUTTON;
            event.window = window;
            event.key.action = OC_KEY_PRESS;
            event.key.button = converted.button;
            event.key.mods = oc_mods_from_xkb_state();
            event.key.clickCount = linux->x11.mouse.clickCount;
            oc_linux_queue_event(&event);
        } break;
        case XCB_BUTTON_RELEASE:
        {
            xcb_button_release_event_t* noti = (xcb_button_release_event_t*)ev;
            oc_window window = window_handle_from_x11_id(noti->event);
            OC_ASSERT(!oc_window_is_nil(window));

            oc_log_info("button release: root=%d/%d, event=%d/%d, detail=%d, state=%d, time=%d\n",
                noti->root_x, noti->root_y, noti->event_x, noti->event_y, noti->detail, noti->state, noti->time);

            reload_x11_keymap_if_needed();

            if(noti->detail == X11_BUTTON_WHEEL_UP || noti->detail == X11_BUTTON_WHEEL_DOWN ||
                noti->detail == X11_BUTTON_WHEEL_LEFT || noti->detail == X11_BUTTON_WHEEL_RIGHT)
            {
                break;
            }

            oc_convert_x11_button_res converted = oc_convert_x11_button(noti->detail);
            if(!converted.ok)  break;

            OC_ASSERT(noti->same_screen);
            oc_vec2 pos = { .x = (f32)noti->event_x, .y = (f32)noti->event_y };

            xcb_timestamp_t elapsed = noti->time - linux->x11.mouse.lastClickTime;
            f32 distance = 0.0;
            {
                f32 x = linux->x11.mouse.lastClickPos.x - pos.x;
                f32 y = linux->x11.mouse.lastClickPos.y - pos.y;
                distance = (x * x) + (y * y);
                if(0) oc_log_info("button release: lastClickPos=%f/%f, pos=%f/%f, distance=%f\n",
                    (double)linux->x11.mouse.lastClickPos.x,
                    (double)linux->x11.mouse.lastClickPos.y,
                    (double)pos.x, (double)pos.y, (double)distance);
            }
            OC_ASSERT(noti->detail == linux->x11.mouse.lastClickButton);
            if(noti->event != linux->x11.mouse.lastClickWinId ||
                elapsed > linux->x11.xsettings.doubleClickTime ||
                distance > linux->x11.xsettings.doubleClickDistance)
            {
                if(0) oc_log_info("button release: reset clickCount\n");
                linux->x11.mouse.clickCount = 0;
            }

            oc_event event;
            memz(&event, sizeof(event));
            event.type = OC_EVENT_MOUSE_BUTTON;
            event.window = window;
            event.key.action = OC_KEY_RELEASE;
            event.key.button = converted.button;
            event.key.mods = oc_mods_from_xkb_state();
            event.key.clickCount = linux->x11.mouse.clickCount;
            oc_linux_queue_event(&event);
        } break;
        case XCB_MOTION_NOTIFY:
        {
            xcb_motion_notify_event_t* noti = (xcb_motion_notify_event_t*)ev;
            OC_ASSERT(noti->root == linux->x11.rootWinId);
            OC_ASSERT(noti->detail == XCB_MOTION_NORMAL);
            oc_window window = window_handle_from_x11_id(noti->event);
            oc_window_data* windowData = oc_window_ptr_from_handle(window);
            OC_ASSERT(windowData);
            window_update_last_user_activity_x(conn, windowData, noti->time);

            if(0) oc_log_info("motion: root=%d/%d, event=%d/%d, detail=%d, state=%d, time=%d\n",
                noti->root_x, noti->root_y, noti->event_x, noti->event_y, noti->detail, noti->state, noti->time);

            reload_x11_keymap_if_needed();

            OC_ASSERT(noti->same_screen);
            oc_vec2 pos = { .x = (f32)noti->event_x, .y = (f32)noti->event_y };

            oc_event event;
            memz(&event, sizeof(event));
            event.type = OC_EVENT_MOUSE_MOVE;
            event.window = window;
            event.mouse.x = pos.x;
            event.mouse.y = pos.y;
            event.mouse.deltaX = pos.x - windowData->linux.pointerPos.x;
            event.mouse.deltaY = pos.y - windowData->linux.pointerPos.y;
            event.mouse.mods = oc_mods_from_xkb_state();
            oc_linux_queue_event(&event);

            windowData->linux.pointerPos = pos;
        } break;
        case XCB_ENTER_NOTIFY:
        {
            xcb_enter_notify_event_t* noti = (xcb_enter_notify_event_t*)ev;
            OC_ASSERT(noti->root == linux->x11.rootWinId);
            oc_window window = window_handle_from_x11_id(noti->event);
            oc_window_data* windowData = oc_window_ptr_from_handle(window);
            OC_ASSERT(windowData);
            window_update_last_user_activity_x(conn, windowData, noti->time);

            oc_log_info("enter: root=%d/%d, event=%d/%d, detail=%d, state=%d, time=%d\n",
                noti->root_x, noti->root_y, noti->event_x, noti->event_y, noti->detail, noti->state, noti->time);

            OC_ASSERT(noti->same_screen_focus);
            oc_vec2 pos = { .x = (f32)noti->event_x, .y = (f32)noti->event_y };

            oc_event event;
            memz(&event, sizeof(event));
            event.type = OC_EVENT_MOUSE_ENTER;
            event.window = window;
            event.mouse.x = pos.x;
            event.mouse.y = pos.y;
            oc_linux_queue_event(&event);

            windowData->linux.pointerPos = pos;
        } break;
        case XCB_LEAVE_NOTIFY:
        {
            xcb_leave_notify_event_t* noti = (xcb_leave_notify_event_t*)ev;
            OC_ASSERT(noti->root == linux->x11.rootWinId);
            oc_window window = window_handle_from_x11_id(noti->event);
            OC_ASSERT(!oc_window_is_nil(window));
            window_update_last_user_activity(conn, window, noti->time);

            oc_event event;
            memz(&event, sizeof(event));
            event.type = OC_EVENT_MOUSE_LEAVE;
            event.window = window;
            oc_linux_queue_event(&event);

            oc_log_info("leave: root=%d/%d, event=%d/%d, detail=%d, state=%d, time=%d\n",
                noti->root_x, noti->root_y, noti->event_x, noti->event_y, noti->detail, noti->state, noti->time);
        } break;
        case XCB_FOCUS_IN:
        {
            xcb_focus_in_event_t* noti = (xcb_focus_in_event_t*)ev;
            //OC_ASSERT(noti->detail == XCB_NOTIFY_DETAIL_NONLINEAR || noti->detail == XCB_NOTIFY_DETAIL_POINTER);
            //OC_ASSERT(noti->mode == XCB_NOTIFY_MODE_NORMAL);
            oc_window window = window_handle_from_x11_id(noti->event);
            oc_window_data* windowData = oc_window_ptr_from_handle(window);
            if(windowData)
            {
                if(windowData->style & OC_WINDOW_STYLE_NO_FOCUS)
                {
                    windowData->linux.focus = OC_LINUX_WINDOW_FOCUSED_IGNORE_INPUTS;
                }
                else
                {
                    windowData->linux.focus = OC_LINUX_WINDOW_FOCUSED;
                    oc_event event;
                    memz(&event, sizeof(event));
                    event.type = OC_EVENT_WINDOW_FOCUS;
                    event.window = window;
                    oc_linux_queue_event(&event);
                }
            }
        } break;
        case XCB_FOCUS_OUT:
        {
            xcb_focus_out_event_t* noti = (xcb_focus_out_event_t*)ev;
            //OC_ASSERT(noti->detail == XCB_NOTIFY_DETAIL_NONLINEAR);
            //OC_ASSERT(noti->mode == XCB_NOTIFY_MODE_NORMAL);
            oc_window window = window_handle_from_x11_id(noti->event);
            oc_window_data* windowData = oc_window_ptr_from_handle(window);
            if(windowData)
            {
                oc_linux_window_focus oldFocus = windowData->linux.focus;
                windowData->linux.focus = OC_LINUX_WINDOW_UNFOCUSED;
                if(oldFocus == OC_LINUX_WINDOW_FOCUSED)
                {
                    oc_event event;
                    memz(&event, sizeof(event));
                    event.type = OC_EVENT_WINDOW_UNFOCUS;
                    event.window = window;
                    oc_linux_queue_event(&event);
                }
            }
        } break;
        case XCB_KEYMAP_NOTIFY:
            oc_notpossible();
            break;
        case XCB_EXPOSE:
            oc_notpossible();
            break;
        case XCB_GRAPHICS_EXPOSURE:
            oc_notpossible();
            break;
        case XCB_NO_EXPOSURE:
            oc_notpossible();
            break;
        case XCB_VISIBILITY_NOTIFY:
            oc_notpossible();
            break;
        case XCB_CREATE_NOTIFY:
            oc_notpossible();
            break;
        case XCB_DESTROY_NOTIFY:
        {
            xcb_destroy_notify_event_t* noti = (xcb_destroy_notify_event_t*)ev;
            oc_window window = window_handle_from_x11_id(noti->window);
            oc_window_data* windowData = oc_window_ptr_from_handle(window);
            if(windowData)
            {
                free(windowData->linux.pendingConfigureNotify);
                OC_ASSERT(!windowData->linux.queued.frameRectForContentRect.queued);
                if(!oc_list_empty(windowData->linux.queued.getFrameRectQueue))
                {
                    int ok = oc_mutex_lock(linux->appCmdUserPoolMutex);
                    OC_ASSERT(ok == 0);
                    while(!oc_list_empty(windowData->linux.queued.getFrameRectQueue))
                    {
                        oc_linux_app_cmd_user* queued = oc_list_pop_front_entry(&windowData->linux.queued.getFrameRectQueue, __typeof__(*queued), listElt);
                        oc_pool_recycle(&linux->appCmdUserPool, queued);
                    }
                    ok = oc_mutex_unlock(linux->appCmdUserPoolMutex);
                    OC_ASSERT(ok == 0);
                }
                //TODO(pld): clean up the window's other resources
                int ok = oc_mutex_lock(linux->windowPoolMutex);
                OC_ASSERT(ok == 0);
                x11_win_id_to_handle entry = {0};
                linux->x11.winIdToHandleLen--;
                for(u32 i = linux->x11.winIdToHandleLen; i != U32_MAX; i--)
                {
                  oc_swap(linux->x11.winIdToHandle[i], entry);
                  if(entry.winId == windowData->linux.x11Id)
                  {
                    break;
                  }
                  OC_ASSERT(i > 0);
                }
                oc_window_recycle_ptr(windowData);
                ok = oc_mutex_unlock(linux->windowPoolMutex);
                OC_ASSERT(ok == 0);

                //FIXME(pld): We currently assume that having no more window
                //means the app should quit. We might not want it so if Orca
                //supports at some point terminal applications or whatever that
                //don't require windows per se. Is that a goal of Orca even?
                if(linux->x11.winIdToHandleLen == 0)
                {
                    /* All windows destroyed (except for control window which
                     * doesn't count as an application window), app should
                     * terminate. */
                    oc_event event;
                    memz(&event, sizeof(event));
                    event.type = OC_EVENT_QUIT;
                    oc_linux_queue_event(&event);
                }
            }
        } break;
        case XCB_UNMAP_NOTIFY:
            /* ignored */
            break;
        case XCB_MAP_NOTIFY:
            /* ignored */
            break;
        case XCB_MAP_REQUEST:
            oc_notpossible();
            break;
        case XCB_REPARENT_NOTIFY:
        {
            xcb_reparent_notify_event_t* noti = (xcb_reparent_notify_event_t*)ev;
            oc_window window = window_handle_from_x11_id(noti->window);
            oc_window_data* windowData = oc_window_ptr_from_handle(window);
            if(windowData)
            {
                if(noti->parent == linux->x11.rootWinId)
                {
                    windowData->linux.posFromParent.x = 0;
                    windowData->linux.posFromParent.y = 0;
                    windowData->linux.flags &= ~OC_LINUX_WINDOW_X11_REPARENTED;
                }
                else
                {
                    windowData->linux.posFromParent.x = noti->x;
                    windowData->linux.posFromParent.y = noti->y;
                    windowData->linux.flags |= OC_LINUX_WINDOW_X11_REPARENTED;
                }
            }
        } break;
        case XCB_CONFIGURE_NOTIFY:
        {
            xcb_configure_notify_event_t* noti = (xcb_configure_notify_event_t*)ev;
            oc_window window = window_handle_from_x11_id(noti->window);
            oc_window_data* windowData = oc_window_ptr_from_handle(window);
            if(!windowData)  break;
            if(!(windowData->linux.flags & OC_LINUX_WINDOW_X11_FRAME_EXTENTS))
            {
                free(windowData->linux.pendingConfigureNotify);
                windowData->linux.pendingConfigureNotify = ev;
                ev = NULL;
                break;
            }
            OC_ASSERT(!windowData->linux.pendingConfigureNotify);
            oc_rect oldRect = windowData->linux.rect;
            bool isWithdrawn = windowData->linux.state == X11_WINDOW_STATE_WITHDRAWN;

            if((!isWithdrawn && (synthetic || !(windowData->linux.flags & OC_LINUX_WINDOW_X11_REPARENTED))) || (isWithdrawn && !synthetic))
            {
                windowData->linux.rect.x = noti->x;
                windowData->linux.rect.y = noti->y;
                windowData->linux.flags |= OC_LINUX_WINDOW_X11_POS_KNOWN;
            }
            else
            {
                windowData->linux.posFromParent.x = noti->x;
                windowData->linux.posFromParent.y = noti->y;
                windowData->linux.flags &= ~OC_LINUX_WINDOW_X11_POS_KNOWN;
                xcb_translate_coordinates_cookie_t cookie =
                    xcb_translate_coordinates(conn, windowData->linux.x11Id, linux->x11.rootWinId, 0, 0);
                oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
                    .cmd = OC_X11_CLIENT_MESSAGE_TRANSLATE_COORDINATES_TO_ROOT,
                    .window = window,
                    .user.translateCoordinatesToRoot.cookie = cookie,
                    .user.translateCoordinatesToRoot.since = ev->sequence,
                });
                OC_STATIC_ASSERT(sizeof(ev->sequence) == sizeof(u16));
            }
            windowData->linux.rect.w = noti->width;
            windowData->linux.rect.h = noti->height;
            windowData->linux.rectSince = ev->sequence;
            OC_ASSERT(noti->border_width == 0);
            // TODO: queued get frame rect

            if(windowData->linux.flags & OC_LINUX_WINDOW_X11_POS_KNOWN)
            {
                bool didMove = !oc_vec2_equal(oldRect.xy, windowData->linux.rect.xy);
                bool didResize = !oc_vec2_equal(oldRect.wh, windowData->linux.rect.wh);
                oc_event event;
                if(didMove || didResize)
                {
                    memz(&event, sizeof(event));
                    event.window = window;
                    event.move.content = windowData->linux.rect;
                    event.move.frame = event.move.content;
                    event.move.frame.x -= windowData->linux.frameLeft;
                    event.move.frame.y -= windowData->linux.frameTop;
                    event.move.frame.w += windowData->linux.frameLeft + windowData->linux.frameRight;
                    event.move.frame.h += windowData->linux.frameTop + windowData->linux.frameBottom;
                }
                if(didResize)
                {
                    event.type = OC_EVENT_WINDOW_RESIZE;
                    oc_linux_queue_event(&event);
                }
                else if(didMove)
                {
                    event.type = OC_EVENT_WINDOW_MOVE;
                    oc_linux_queue_event(&event);
                }
                //FIXME(pld): after repainting only
                if(windowData->linux.netWmSyncRequestUpdateValue)
                {
                    xcb_sync_counter_t id = windowData->linux.netWmSyncRequestCounterId;
                    xcb_sync_int64_t val =
                    {
                        .lo = (u32)(windowData->linux.netWmSyncRequestUpdateValue >>  0 & 0xFFFFFFFF),
                        .hi = (i32)(windowData->linux.netWmSyncRequestUpdateValue >> 32 & 0xFFFFFFFF),
                    };
                    xcb_sync_set_counter(conn, id, val);
                }
                while(!oc_list_empty(windowData->linux.queued.getFrameRectQueue))
                {
                    oc_linux_app_cmd_user* queued = oc_list_pop_front_entry(&windowData->linux.queued.getFrameRectQueue, __typeof__(*queued), listElt);
                    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
                        .cmd = OC_X11_CLIENT_MESSAGE_WINDOW_GET_FRAME_RECT,
                        .window = window,
                        .user = *queued,
                    });
                    int ok = oc_mutex_lock(linux->appCmdUserPoolMutex);
                    OC_ASSERT(ok == 0);
                    oc_pool_recycle(&linux->appCmdUserPool, queued);
                    ok = oc_mutex_unlock(linux->appCmdUserPoolMutex);
                    OC_ASSERT(ok == 0);
                }
            }
        } break;
        case XCB_CONFIGURE_REQUEST:
            oc_notpossible();
            break;
        case XCB_GRAVITY_NOTIFY:
            /* ignored */
            break;
        case XCB_RESIZE_REQUEST:
            oc_notpossible();
            break;
        case XCB_CIRCULATE_NOTIFY:
            /* ignored */
            break;
        case XCB_CIRCULATE_REQUEST:
            oc_notpossible();
            break;
        case XCB_PROPERTY_NOTIFY:
        {
            // TODO(pld): use timestamp?
            xcb_property_notify_event_t* noti = (xcb_property_notify_event_t*)ev;
            u64 clipboardRequestorIndex = 0;
            for(; clipboardRequestorIndex < linux->x11.ownClipboard.requestorsLen; clipboardRequestorIndex++)
            {
                if(linux->x11.ownClipboard.requestors[clipboardRequestorIndex].window == noti->window &&
                    linux->x11.ownClipboard.requestors[clipboardRequestorIndex].property == noti->atom)
                {
                    break;
                }
            }
            if(clipboardRequestorIndex < linux->x11.ownClipboard.requestorsLen)
            {
                bool completed = false;
                if(noti->state == XCB_PROPERTY_DELETE)
                {
                    oc_x11_clipboard_requestor* requestor = &linux->x11.ownClipboard.requestors[clipboardRequestorIndex];
                    if(requestor->incr)
                    {
                        u64 len = 0;
                        for(u64 i = 0; i < requestor->contentVecLen; i++)  len += requestor->contentVec[i].len;
                        u64 rem = len - requestor->incrOff;
                        if(rem > 0)
                        {
                            u64 chunkSize = linux->x11.maximumRequestSize - sizeof(xcb_change_property_request_t) - 4;
                            if(rem < chunkSize)  chunkSize = rem;
                            u64 i = 0;
                            u64 off = requestor->incrOff;
                            u64 chunkRem = chunkSize;
                            while(i < requestor->contentVecLen && off > requestor->contentVec[i].len)
                            {
                                off -= requestor->contentVec[i].len, i++;
                            }
                            xcb_prop_mode_t mode = XCB_PROP_MODE_REPLACE;
                            while(i < requestor->contentVecLen && chunkRem > 0)
                            {
                                u64 n = oc_min(requestor->contentVec[i].len - off, chunkRem);
                                xcb_change_property(conn, mode, requestor->window,
                                    requestor->property, requestor->type,
                                    requestor->format, n / (requestor->format / 8),
                                    &requestor->contentVec[i].ptr[off]);
                                mode = XCB_PROP_MODE_APPEND;
                                off = 0;
                                chunkRem -= n;
                                i++;
                            }
                            requestor->incrOff += chunkSize;
                        }
                        else
                        {
                            xcb_change_property(conn, XCB_PROP_MODE_REPLACE,
                                requestor->window, requestor->property,
                                requestor->type, requestor->format, 0, NULL);
                            completed = true;
                        }
                    }
                    else
                    {
                        completed = true;
                    }
                }
                if(completed)
                {
                    /* Transfer is complete, remove requestor. */
                    memmove(&linux->x11.ownClipboard.requestors[clipboardRequestorIndex],
                        &linux->x11.ownClipboard.requestors[clipboardRequestorIndex + 1],
                        linux->x11.ownClipboard.requestorsLen - clipboardRequestorIndex - 1);
                    linux->x11.ownClipboard.requestorsLen--;

                    if(noti->window != linux->x11.controlWinId)
                    {
                        xcb_change_window_attributes_value_list_t cwa = {0};
                        xcb_change_window_attributes_aux(conn, noti->window, XCB_CW_EVENT_MASK, &cwa);
                    }

                    if(linux->x11.ownClipboard.requestorsLen == 0 &&
                        linux->x11.ownClipboard.hasPendingContent)
                    {
                        /* No more requestors, requeue pending set-clipboard action. */
                        oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
                            .cmd = OC_X11_CLIENT_MESSAGE_SET_CLIPBOARD,
                            .user.setClipboard.content = linux->x11.ownClipboard.pendingContent,
                        });
                        linux->x11.ownClipboard.pendingContent = (oc_str8){0};
                        linux->x11.ownClipboard.hasPendingContent = false;
                    }
                }
                if(noti->window != linux->x11.controlWinId)
                {
                    break;
                }
            }
            oc_window window = {0};
            oc_window_data* windowData = NULL;
            if(noti->window != linux->x11.rootWinId && noti->window != linux->x11.controlWinId)
            {
                window = window_handle_from_x11_id(noti->window);
                windowData = oc_window_ptr_from_handle(window);
                if(!windowData)  break;
            }
            if(noti->atom == linux->x11.atoms.WM_STATE)
            {
                OC_ASSERT(windowData);
                if(noti->state == XCB_PROPERTY_NEW_VALUE)
                {
                    xcb_get_property_cookie_t cookie = xcb_get_property(conn,
                        false, windowData->linux.x11Id, linux->x11.atoms.WM_STATE,
                        linux->x11.atoms.WM_STATE, 0, sizeof(x11_wm_state) / 4);
                    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
                        .cmd = OC_X11_CLIENT_MESSAGE_GET_PROPERTY,
                        .window = window,
                        .user.getProperty.prop = linux->x11.atoms.WM_STATE,
                        .user.getProperty.cookie = cookie,
                    });
                }
                else if(noti->state == XCB_PROPERTY_DELETE)
                {
                    x11_window_state oldState = windowData->linux.state;
                    windowData->linux.state = X11_WINDOW_STATE_WITHDRAWN;
                    if(oldState == X11_WINDOW_STATE_NORMAL)
                    {
                        oc_event event;
                        memz(&event, sizeof(event));
                        event.type = OC_EVENT_WINDOW_HIDE;
                        event.window = window;
                        oc_linux_queue_event(&event);
                    }
                }
                else
                {
                    oc_unreachable();
                }
            }
            else if(noti->atom == linux->x11.atoms._NET_WM_STATE)
            {
                OC_ASSERT(windowData);
                if(noti->state == XCB_PROPERTY_NEW_VALUE)
                {
                    xcb_get_property_cookie_t cookie = xcb_get_property(conn,
                        false, windowData->linux.x11Id, linux->x11.atoms._NET_WM_STATE,
                        XCB_ATOM_ATOM, 0, oc_array_size(windowData->linux.netState));
                    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
                        .cmd = OC_X11_CLIENT_MESSAGE_GET_PROPERTY,
                        .window = window,
                        .user.getProperty.prop = linux->x11.atoms._NET_WM_STATE,
                        .user.getProperty.cookie = cookie,
                    });
                }
                else if(noti->state == XCB_PROPERTY_DELETE)
                {
                    /* ignored */
                }
                else
                {
                    oc_unreachable();
                }
            }
            else if(noti->atom == linux->x11.atoms._NET_WM_USER_TIME)
            {
                OC_ASSERT(windowData);
                /* We listen for changes on _NET_WM_USER_TIME to set an initial
                 * value for it when the window is shown. This allows to call
                 * oc_window_focus, which requires a recent enough timestamp,
                 * without needing any keyboard/pointer input beforehand.
                 * Keyboard and pointer input being the only ways this
                 * timestamp updates otherwise. */
                if(noti->state == XCB_PROPERTY_NEW_VALUE)
                {
                    if(windowData->linux.netWmUserTime == 0)
                    {
                        window_update_last_user_activity_x(conn, windowData, noti->time);
                    }
                }
                else if(noti->state == XCB_PROPERTY_DELETE)
                {
                    windowData->linux.netWmUserTime = 0;
                }
                else
                {
                    oc_unreachable();
                }
            }
            else if(noti->atom == linux->x11.atoms._NET_FRAME_EXTENTS)
            {
                OC_ASSERT(windowData);
                if(noti->state == XCB_PROPERTY_NEW_VALUE)
                {
                    xcb_get_property_cookie_t cookie = {0};
                    cookie = xcb_get_property(conn, false, windowData->linux.x11Id,
                        linux->x11.atoms._NET_FRAME_EXTENTS, XCB_ATOM_CARDINAL, 0, 4);
                    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
                        .cmd = OC_X11_CLIENT_MESSAGE_GET_PROPERTY,
                        .window = window,
                        .user.getProperty.prop = linux->x11.atoms._NET_FRAME_EXTENTS,
                        .user.getProperty.cookie = cookie,
                    });
                }
                else
                {
                    oc_notpossible();
                }
            }
            else if(noti->atom == linux->x11.atoms._NET_NUMBER_OF_DESKTOPS)
            {
                OC_ASSERT(!windowData);
                OC_ASSERT(noti->window == linux->x11.rootWinId);
                if(noti->state == XCB_PROPERTY_NEW_VALUE)
                {
                    xcb_get_property_cookie_t cookie = {0};
                    cookie = xcb_get_property(conn, false, linux->x11.rootWinId,
                        linux->x11.atoms._NET_NUMBER_OF_DESKTOPS, XCB_ATOM_CARDINAL, 0, 1);
                    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
                        .cmd = OC_X11_CLIENT_MESSAGE_GET_PROPERTY,
                        .user.getProperty.prop = linux->x11.atoms._NET_NUMBER_OF_DESKTOPS,
                        .user.getProperty.cookie = cookie,
                    });
                }
                else
                {
                    oc_notpossible();
                }
            }
            else if(noti->atom == linux->x11.atoms._NET_WORKAREA)
            {
                OC_ASSERT(!windowData);
                OC_ASSERT(noti->window == linux->x11.rootWinId);
                if(noti->state == XCB_PROPERTY_NEW_VALUE)
                {
                    xcb_get_property_cookie_t cookie = {0};
                    u32 n = oc_array_size(linux->x11.netWorkarea) * 4;
                    cookie = xcb_get_property(conn, false, linux->x11.rootWinId,
                        linux->x11.atoms._NET_WORKAREA, XCB_ATOM_CARDINAL, 0, n);
                    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
                        .cmd = OC_X11_CLIENT_MESSAGE_GET_PROPERTY,
                        .user.getProperty.prop = linux->x11.atoms._NET_WORKAREA,
                        .user.getProperty.cookie = cookie,
                    });
                }
                else
                {
                    oc_notpossible();
                }
            }
            else if(noti->atom == linux->x11.atoms._NET_WM_DESKTOP)
            {
                OC_ASSERT(windowData);
                if(noti->state == XCB_PROPERTY_NEW_VALUE)
                {
                    xcb_get_property_cookie_t cookie = {0};
                    cookie = xcb_get_property(conn, false, windowData->linux.x11Id,
                        linux->x11.atoms._NET_WM_DESKTOP, XCB_ATOM_CARDINAL, 0, 1);
                    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
                        .cmd = OC_X11_CLIENT_MESSAGE_GET_PROPERTY,
                        .window = window,
                        .user.getProperty.prop = linux->x11.atoms._NET_WM_DESKTOP,
                        .user.getProperty.cookie = cookie,
                    });
                }
            }
            else if(noti->atom == linux->x11.atoms.OC_X11_CLIPBOARD_DEST)
            {
                OC_ASSERT(!windowData);
                OC_ASSERT(noti->window == linux->x11.controlWinId);
                if(noti->state == XCB_PROPERTY_NEW_VALUE && linux->x11.getClipboard.init && linux->x11.getClipboard.incr)
                {
                    xcb_get_property_cookie_t cookie = {0};
                    cookie = xcb_get_property(conn, true, linux->x11.controlWinId,
                        linux->x11.atoms.OC_X11_CLIPBOARD_DEST, XCB_ATOM_ANY, 0, UINT32_MAX);
                    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
                        .cmd = OC_X11_CLIENT_MESSAGE_GET_PROPERTY,
                        .user.getProperty.prop = linux->x11.atoms.OC_X11_CLIPBOARD_DEST,
                        .user.getProperty.cookie = cookie,
                    });
                }
            }
        } break;
        case XCB_SELECTION_CLEAR:
        {
            xcb_selection_clear_event_t* noti = (xcb_selection_clear_event_t*)ev;
            OC_ASSERT(noti->selection == linux->x11.atoms.CLIPBOARD);
            OC_ASSERT(noti->owner == linux->x11.controlWinId);
            OC_ASSERT(linux->x11.ownClipboard.status == OC_X11_CLIPBOARD_STATUS_ACQUIRING ||
                linux->x11.ownClipboard.status == OC_X11_CLIPBOARD_STATUS_OWNED);
            linux->x11.ownClipboard.relinquishedAt = noti->time;
            linux->x11.ownClipboard.status = OC_X11_CLIPBOARD_STATUS_NOT_OWNED;
        } break;
        case XCB_SELECTION_REQUEST:
        {
            xcb_selection_request_event_t* noti = (xcb_selection_request_event_t*)ev;
            OC_ASSERT(noti->selection == linux->x11.atoms.CLIPBOARD);
            OC_ASSERT(noti->owner == linux->x11.controlWinId);
            bool afterAcquired = linux->x11.ownClipboard.acquiredAt <= noti->time || noti->time == XCB_CURRENT_TIME;
            bool beforeRelinquished = noti->time < linux->x11.ownClipboard.relinquishedAt || noti->time == XCB_CURRENT_TIME;
            bool owning =
                (linux->x11.ownClipboard.status == OC_X11_CLIPBOARD_STATUS_ACQUIRING ||
                 linux->x11.ownClipboard.status == OC_X11_CLIPBOARD_STATUS_OWNED) &&
                afterAcquired;
            bool owned =
                linux->x11.ownClipboard.status == OC_X11_CLIPBOARD_STATUS_NOT_OWNED &&
                afterAcquired && beforeRelinquished;
            xcb_selection_notify_event_t reply =
            {
                .response_type = XCB_SELECTION_NOTIFY,
                .time = noti->time,
                .requestor = noti->requestor,
                .selection = noti->selection,
                .target = noti->target,
                .property = XCB_ATOM_NONE,
            };
            if(owning || owned)
            {
                xcb_atom_t prop = noti->property;
                if(prop == XCB_ATOM_NONE)  prop = noti->target;
                bool supportedTarget = true;
                oc_str8 contentVec[2] = {0};
                u64 contentVecLen = 0;
                xcb_atom_t type = XCB_ATOM_NONE;
                u8 format = 0;
                if(noti->target == linux->x11.atoms.TEXT || noti->target == linux->x11.atoms.UTF8_STRING)
                {
                    contentVec[contentVecLen++] = linux->x11.ownClipboard.content;
                    type = linux->x11.atoms.UTF8_STRING;
                    format = 8;
                }
                else if(noti->target == linux->x11.atoms.TIMESTAMP)
                {
                    contentVec[contentVecLen++] =
                    (oc_str8){
                        .ptr = (char*)&linux->x11.ownClipboard.acquiredAt,
                        .len = sizeof(linux->x11.ownClipboard.acquiredAt),
                    };
                    type = XCB_ATOM_INTEGER;
                    format = 32;
                }
                else if(noti->target == linux->x11.atoms.TARGETS)
                {
                    static xcb_atom_t builtInTargets[4] = {0};
                    if(builtInTargets[0] == XCB_ATOM_NONE)
                    {
                        u64 i = 0;
                        builtInTargets[i++] = linux->x11.atoms.TARGETS;
                        builtInTargets[i++] = linux->x11.atoms.TEXT;
                        builtInTargets[i++] = linux->x11.atoms.TIMESTAMP;
                        builtInTargets[i++] = linux->x11.atoms.UTF8_STRING;
                        OC_ASSERT(i == oc_array_size(builtInTargets));
                    }
                    OC_STATIC_ASSERT(sizeof(xcb_atom_t) == sizeof(u32));
                    OC_STATIC_ASSERT(sizeof(xcb_atom_t) == sizeof(*linux->x11.ownClipboard.targets));
                    contentVec[contentVecLen++] =
                    (oc_str8){
                        .ptr = (char*)builtInTargets,
                        .len = sizeof(builtInTargets),
                    };
                    contentVec[contentVecLen++] =
                    (oc_str8){
                        .ptr = (char*)linux->x11.ownClipboard.targets,
                        .len = linux->x11.ownClipboard.targetsLen * sizeof(*linux->x11.ownClipboard.targets),
                    };
                    type = XCB_ATOM_ATOM;
                    format = 32;
                }
                else
                {
                    usize i = 0;
                    for(; i < linux->x11.ownClipboard.targetsLen; i++)
                    {
                        if(linux->x11.ownClipboard.targets[i] == noti->target)  break;
                    }
                    if(i < linux->x11.ownClipboard.targetsLen)
                    {
                        contentVec[contentVecLen++] = linux->x11.ownClipboard.targetData[i];
                        /* Chromium reuses the target as the property type,
                         * let's do the same and not bother guessing further. */
                        type = noti->target;
                        format = 8;
                    }
                    else
                    {
                        supportedTarget = false;
                    }
                }
                if(supportedTarget)
                {
                    OC_STATIC_ASSERT(sizeof(xcb_change_property_request_t) == sizeof(xcb_get_property_request_t));
                    u64 maximumPayloadSize = linux->x11.maximumRequestSize - sizeof(xcb_change_property_request_t) - 4;
                    u64 totalLen = 0;
                    for(u64 i = 0; i < contentVecLen; i++)  totalLen += contentVec[i].len;
                    bool isLarge = totalLen > maximumPayloadSize;
                    oc_x11_clipboard_requestor requestor =
                    {
                        .window = noti->requestor,
                        .property = prop,
                        .incr = isLarge,
                    };
                    if(requestor.incr)
                    {
                        u32 n = linux->x11.ownClipboard.content.len;
                        xcb_change_property(conn, XCB_PROP_MODE_REPLACE,
                            noti->requestor, prop, linux->x11.atoms.INCR, 32, 1, &n);

                        OC_STATIC_ASSERT(sizeof(requestor.contentVec) == sizeof(contentVec));
                        memcpy(requestor.contentVec, contentVec, contentVecLen * sizeof(*contentVec));
                        requestor.contentVecLen = contentVecLen;
                        requestor.type = type;
                        requestor.format = format;
                    }
                    else
                    {
                        xcb_prop_mode_t mode = XCB_PROP_MODE_REPLACE;
                        for(u64 i = 0; i < contentVecLen; i++)
                        {
                            xcb_change_property(conn, mode, noti->requestor,
                                prop, type, format, contentVec[i].len / (format / 8),
                                contentVec[i].ptr);
                            mode = XCB_PROP_MODE_APPEND;
                        }
                    }
                    if(noti->requestor != linux->x11.controlWinId)
                    {
                        xcb_change_window_attributes_value_list_t cwa =
                        {
                            .event_mask = XCB_EVENT_MASK_PROPERTY_CHANGE,
                        };
                        xcb_change_window_attributes_aux(conn, noti->requestor, XCB_CW_EVENT_MASK, &cwa);
                    }
                    usize i = linux->x11.ownClipboard.requestorsLen;
                    OC_ASSERT(i < oc_array_size(linux->x11.ownClipboard.requestors));
                    linux->x11.ownClipboard.requestors[i] = requestor;
                    linux->x11.ownClipboard.requestorsLen++;

                    reply.property = prop;
                }
                else
                {
                    oc_log_info("Failed to convert clipboard to target %d,"
                        " either clipboard tag is missing or not a built-in"
                        " target\n", noti->target);
                }
            }
            xcb_send_event(conn, false, noti->requestor, 0, (const char*)&reply);
        } break;
        case XCB_SELECTION_NOTIFY:
        {
            xcb_selection_notify_event_t* noti = (xcb_selection_notify_event_t*)ev;
            OC_ASSERT(noti->requestor == linux->x11.controlWinId);
            OC_ASSERT(noti->selection == linux->x11.atoms.CLIPBOARD);
            OC_ASSERT(linux->x11.getClipboard.init);
            OC_ASSERT(noti->target == linux->x11.getClipboard.target);
            OC_ASSERT(noti->time == linux->x11.getClipboard.time);
            if(noti->property == linux->x11.atoms.OC_X11_CLIPBOARD_DEST)
            {
                xcb_get_property_cookie_t cookie = {0};
                cookie = xcb_get_property(conn, true, linux->x11.controlWinId,
                    linux->x11.atoms.OC_X11_CLIPBOARD_DEST, XCB_ATOM_ANY, 0, UINT32_MAX);
                oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
                    .cmd = OC_X11_CLIENT_MESSAGE_GET_PROPERTY,
                    .user.getProperty.prop = linux->x11.atoms.OC_X11_CLIPBOARD_DEST,
                    .user.getProperty.cookie = cookie,
                });
            }
            else if(noti->property == XCB_ATOM_NONE)
            {
                *linux->x11.getClipboard.result = OC_STR8("");
                oc_linux_app_cmd_completion_signal(linux->x11.getClipboard.completion);
                memz(&linux->x11.getClipboard, sizeof(linux->x11.getClipboard));
                oc_linux_app_cmd_user* queued = oc_list_pop_front_entry(&linux->x11.getClipboardQueue, __typeof__(*queued), listElt);
                if(queued)
                {
                    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
                        .cmd = OC_X11_CLIENT_MESSAGE_GET_CLIPBOARD,
                        .user = *queued,
                    });
                    int ok = oc_mutex_lock(linux->appCmdUserPoolMutex);
                    OC_ASSERT(ok == 0);
                    oc_pool_recycle(&linux->appCmdUserPool, queued);
                    ok = oc_mutex_unlock(linux->appCmdUserPoolMutex);
                    OC_ASSERT(ok == 0);
                }
            }
            else
            {
                oc_notpossible();
            }
        } break;
        case XCB_COLORMAP_NOTIFY:
            oc_notpossible();
            break;
        case XCB_CLIENT_MESSAGE:
        {
            xcb_client_message_event_t* noti = (xcb_client_message_event_t*)ev;
            if(noti->type == linux->x11.atoms.OC_X11_CLIENT_MESSAGE)
            {
                OC_ASSERT(noti->format == 32);
                OC_ASSERT(noti->window == linux->x11.controlWinId);
                oc_x11_client_message m = noti->data.data32[0];
                oc_window window = {0};
                memcpy(&window, &noti->data.data32[1], sizeof(window));
                oc_window_data* windowData = oc_window_ptr_from_handle(window);
                oc_linux_app_cmd_user* u = NULL;
                memcpy(&u, &noti->data.data32[3], sizeof(u));
                switch(m)
                {
                case OC_X11_CLIENT_MESSAGE_REQUEST_QUIT:
                {
                    OC_ASSERT(!windowData);
                    oc_appData.shouldQuit = true;
                } break;
                case OC_X11_CLIENT_MESSAGE_CANCEL_QUIT:
                {
                    OC_ASSERT(!windowData);
                    oc_appData.shouldQuit = false;
                } break;
                case OC_X11_CLIENT_MESSAGE_WINDOW_DESTROY:
                {
                    OC_ASSERT(windowData);
                    xcb_destroy_window(conn, windowData->linux.x11Id);
                } break;
                case OC_X11_CLIENT_MESSAGE_WINDOW_REQUEST_CLOSE:
                {
                    OC_ASSERT(windowData);
                    xcb_client_message_event_t msg =
                    {
                        .response_type = XCB_CLIENT_MESSAGE,
                        .format = 32,
                        .window = windowData->linux.x11Id,
                        .type = linux->x11.atoms._NET_CLOSE_WINDOW,
                        //FIXME(pld): take most recent timestamp across all window instead
                        //of only the one from the window we're attempting to close?
                        .data.data32[0] = windowData->linux.netWmUserTime,
                        .data.data32[1] = X11_EWMH_SOURCE_INDICATION_NORMAL,
                    };
                    xcb_send_event(conn, false, linux->x11.rootWinId,
                        XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
                        XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
                        (const char*)&msg);
                } break;
                case OC_X11_CLIENT_MESSAGE_WINDOW_CANCEL_CLOSE:
                {
                    OC_ASSERT(windowData);
                    windowData->shouldClose = false;
                } break;
                case OC_X11_CLIENT_MESSAGE_WINDOW_SET_TITLE:
                {
                    OC_ASSERT(windowData);
                    oc_str8 title = u->setTitle.title;
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
                } break;
                case OC_X11_CLIENT_MESSAGE_WINDOW_SHOW:
                {
                    OC_ASSERT(windowData);
                    if(windowData->linux.state == X11_WINDOW_STATE_WITHDRAWN)
                    {
                        x11_wm_hints wmHints =
                        {
                            .flags = X11_WM_HINTS_INPUT_HINT | X11_WM_HINTS_STATE_HINT,
                            .input = !(windowData->style & OC_WINDOW_STYLE_NO_FOCUS),
                            .initialState = X11_WINDOW_STATE_NORMAL,
                        };
                        xcb_change_property(conn, XCB_PROP_MODE_REPLACE,
                            windowData->linux.x11Id, XCB_ATOM_WM_HINTS,
                            XCB_ATOM_WM_HINTS, 32, sizeof(wmHints) / 4, &wmHints);
                    }
                    xcb_delete_property(conn, windowData->linux.x11Id,
                        linux->x11.atoms._NET_WM_USER_TIME);
                    xcb_map_window(conn, windowData->linux.x11Id);
                    if(windowData->style & OC_WINDOW_STYLE_FLOAT)
                    {
                        xcb_client_message_event_t msg =
                        {
                            .response_type = XCB_CLIENT_MESSAGE,
                            .format = 32,
                            .window = windowData->linux.x11Id,
                            .type = linux->x11.atoms._NET_WM_STATE,
                            .data.data32[0] = X11_NET_WM_STATE_ADD,
                            .data.data32[1] = linux->x11.atoms._NET_WM_STATE_ABOVE,
                            .data.data32[2] = 0,
                            .data.data32[3] = X11_EWMH_SOURCE_INDICATION_NORMAL,
                        };
                        xcb_send_event(conn, false, linux->x11.rootWinId,
                            XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
                            XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
                            (const char *)&msg);
                    }
                    xcb_change_property(conn, XCB_PROP_MODE_APPEND,
                        windowData->linux.x11Id, linux->x11.atoms._NET_WM_USER_TIME,
                        XCB_ATOM_CARDINAL, 32, 0, NULL);
                } break;
                case OC_X11_CLIENT_MESSAGE_WINDOW_HIDE:
                {
                    OC_ASSERT(windowData);
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
                } break;
                case OC_X11_CLIENT_MESSAGE_WINDOW_MINIMIZE:
                {
                    OC_ASSERT(windowData);
                    if(windowData->linux.state == X11_WINDOW_STATE_WITHDRAWN)
                    {
                        x11_wm_hints wmHints =
                        {
                            .flags = X11_WM_HINTS_INPUT_HINT | X11_WM_HINTS_STATE_HINT,
                            .input = !(windowData->style & OC_WINDOW_STYLE_NO_FOCUS),
                            .initialState = X11_WINDOW_STATE_ICONIC,
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
                        OC_ASSERT(windowData->linux.state == X11_WINDOW_STATE_ICONIC);
                    }
                } break;
                case OC_X11_CLIENT_MESSAGE_WINDOW_MAXIMIZE:
                {
                    OC_ASSERT(windowData);
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
                } break;
                case OC_X11_CLIENT_MESSAGE_WINDOW_RESTORE:
                {
                    OC_ASSERT(windowData);
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
                } break;
                case OC_X11_CLIENT_MESSAGE_WINDOW_FOCUS:
                {
                    OC_ASSERT(windowData);
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
                } break;
                case OC_X11_CLIENT_MESSAGE_WINDOW_UNFOCUS:
                {
                    OC_ASSERT(windowData);
                    if(windowData->linux.focus == OC_LINUX_WINDOW_FOCUSED)
                    {
                        windowData->linux.focus = OC_LINUX_WINDOW_FOCUSED_IGNORE_INPUTS;
                        oc_event event;
                        memz(&event, sizeof(event));
                        event.type = OC_EVENT_WINDOW_UNFOCUS;
                        event.window = window;
                        oc_linux_queue_event(&event);
                    }
                } break;
                case OC_X11_CLIENT_MESSAGE_WINDOW_SEND_TO_BACK:
                {
                    OC_ASSERT(windowData);
                    xcb_config_window_t config_mask = XCB_CONFIG_WINDOW_STACK_MODE;
                    xcb_configure_window_value_list_t config =
                    {
                        .stack_mode = XCB_STACK_MODE_BELOW,
                    };
                    xcb_configure_window_aux(conn, windowData->linux.x11Id, config_mask, &config);
                } break;
                case OC_X11_CLIENT_MESSAGE_WINDOW_BRING_TO_FRONT:
                {
                    OC_ASSERT(windowData);
                    xcb_config_window_t config_mask = XCB_CONFIG_WINDOW_STACK_MODE;
                    xcb_configure_window_value_list_t config =
                    {
                        .stack_mode = XCB_STACK_MODE_ABOVE,
                    };
                    xcb_configure_window_aux(conn, windowData->linux.x11Id, config_mask, &config);
                } break;
                case OC_X11_CLIENT_MESSAGE_WINDOW_SET_FRAME_RECT:
                {
                    OC_ASSERT(windowData);
                    if(windowData->linux.flags & OC_LINUX_WINDOW_X11_FRAME_EXTENTS)
                    {
                        oc_rect rect = u->setFrameRect.rect;
                        rect.w -= windowData->linux.frameLeft + windowData->linux.frameRight;
                        rect.h -= windowData->linux.frameTop + windowData->linux.frameBottom;
                        xcb_config_window_t config_mask = XCB_CONFIG_WINDOW_X |
                            XCB_CONFIG_WINDOW_Y |
                            XCB_CONFIG_WINDOW_WIDTH |
                            XCB_CONFIG_WINDOW_HEIGHT |
                            0;
                        xcb_configure_window_value_list_t config =
                        {
                            .x = (i32)rect.x,
                            .y = (i32)rect.y,
                            .width = (u32)rect.w,
                            .height = (u32)rect.h,
                        };
                        xcb_void_cookie_t cookie =
                            xcb_configure_window_aux(conn, windowData->linux.x11Id, config_mask, &config);
                        windowData->linux.rectNext = (u16)cookie.sequence;
                    }
                    else
                    {
                        // TODO(pld): do not requeue spuriously?
                        /* Requeue until we get the frame extents. */
                        oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
                            .cmd = OC_X11_CLIENT_MESSAGE_WINDOW_SET_FRAME_RECT,
                            .window = window,
                            .user = *u,
                        });
                    }
                } break;
                case OC_X11_CLIENT_MESSAGE_WINDOW_SET_CONTENT_RECT:
                {
                    OC_ASSERT(windowData);
                    if(windowData->linux.flags & OC_LINUX_WINDOW_X11_FRAME_EXTENTS)
                    {
                        oc_rect rect = u->setContentRect.rect;
                        rect.x -= windowData->linux.frameLeft;
                        rect.y -= windowData->linux.frameTop;
                        xcb_config_window_t config_mask = XCB_CONFIG_WINDOW_X |
                            XCB_CONFIG_WINDOW_Y |
                            XCB_CONFIG_WINDOW_WIDTH |
                            XCB_CONFIG_WINDOW_HEIGHT |
                            0;
                        xcb_configure_window_value_list_t config =
                        {
                            .x = (i32)rect.x,
                            .y = (i32)rect.y,
                            .width = (u32)rect.w,
                            .height = (u32)rect.h,
                        };
                        xcb_void_cookie_t cookie =
                            xcb_configure_window_aux(conn, windowData->linux.x11Id, config_mask, &config);
                        windowData->linux.rectNext = (u16)cookie.sequence;
                    }
                    else
                    {
                        // TODO(pld): do not requeue spuriously?
                        /* Requeue until we get the frame extents. */
                        oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
                            .cmd = OC_X11_CLIENT_MESSAGE_WINDOW_SET_CONTENT_RECT,
                            .window = window,
                            .user = *u,
                        });
                    }
                } break;
                case OC_X11_CLIENT_MESSAGE_WINDOW_CENTER:
                {
                    OC_ASSERT(windowData);
                    OC_ASSERT(windowData->linux.netWmDesktop < linux->x11.netNumberOfDesktops);
                    OC_ASSERT(linux->x11.netWorkareaLen == linux->x11.netNumberOfDesktops);
                    if(windowData->linux.flags & OC_LINUX_WINDOW_X11_FRAME_EXTENTS)
                    {
                        u32 desktop = windowData->linux.netWmDesktop;
                        oc_rect workarea = linux->x11.netWorkarea[desktop];
                        oc_rect rect = {0};
                        rect.wh = u->center.contentWh;
                        rect.xy = oc_vec2_add(workarea.xy, oc_vec2_mul(0.5, workarea.wh));
                        rect.xy = oc_vec2_add(rect.xy, oc_vec2_mul(-0.5, rect.wh));
                        //FIXME(pld): Do something similar as on MacOS, "somewhat above center vertically"?
                        //rect.y -= workarea.y * 0.1;
                        rect.x -= (windowData->linux.frameLeft + windowData->linux.frameRight) / 2;
                        rect.y -= (windowData->linux.frameTop + windowData->linux.frameBottom) / 2;
                        xcb_config_window_t config_mask = XCB_CONFIG_WINDOW_X |
                            XCB_CONFIG_WINDOW_Y |
                            XCB_CONFIG_WINDOW_WIDTH |
                            XCB_CONFIG_WINDOW_HEIGHT |
                            0;
                        xcb_configure_window_value_list_t config =
                        {
                            .x = (i32)rect.x,
                            .y = (i32)rect.y,
                            .width = (u32)rect.w,
                            .height = (u32)rect.h,
                        };
                        xcb_void_cookie_t cookie =
                            xcb_configure_window_aux(conn, windowData->linux.x11Id, config_mask, &config);
                        windowData->linux.rectNext = (u16)cookie.sequence;
                    }
                    else
                    {
                        // TODO(pld): do not requeue spuriously?
                        /* Requeue until we get the frame extents. */
                        oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
                            .cmd = OC_X11_CLIENT_MESSAGE_WINDOW_CENTER,
                            .window = window,
                            .user = *u,
                        });
                    }
                } break;
                case OC_X11_CLIENT_MESSAGE_DISPATCH_ON_MAIN_THREAD_SYNC:
                {
                    OC_ASSERT(!windowData);
                    oc_linux_dispatch_sync_request* req = u->dispatchOnMainThreadSync.req;
                    OC_ASSERT(req);
                    u64 reqId = u->dispatchOnMainThreadSync.reqId;
                    /* Dispatcher incremented the refcount for us. */
                    int ok = oc_mutex_lock(req->mutex);
                    OC_ASSERT(ok == 0);
                    OC_ASSERT(req->refcount > 0);
                    if(reqId == req->reqId)
                    {
                        /* Dispatcher's still waiting. */
                        req->result.retVal = req->proc(req->user);
                        req->result.didRun = true;
                        ok = oc_condition_signal(req->cond);
                        OC_ASSERT(ok == 0);
                    }
                    else
                    {
                        /* Dispatcher's timed out, skip. */
                    }
                    oc_linux_dispatch_sync_request_refcount_dec(req);
                } break;
                case OC_X11_CLIENT_MESSAGE_GET_PROPERTY:
                {
                    xcb_atom_t prop = u->getProperty.prop;
                    xcb_get_property_cookie_t cookie = u->getProperty.cookie;
                    xcb_get_property_reply_t* reply = NULL;
                    reply = xcb_get_property_reply(conn, cookie, NULL);
                    OC_ASSERT(reply);
                    OC_ASSERT(reply->response_type == X11_RESPONSE_TYPE_REPLY);
                    void* p = xcb_get_property_value(reply);
                    OC_ASSERT(p);
                    if(prop == linux->x11.atoms._NET_FRAME_EXTENTS)
                    {
                        OC_ASSERT(windowData);
                        OC_ASSERT(reply->type == XCB_ATOM_CARDINAL);
                        OC_ASSERT(reply->format == 32);
                        OC_ASSERT(reply->value_len == 4);
                        OC_ASSERT(reply->bytes_after == 0);
                        u32* widths = p;
                        windowData->linux.frameLeft = (f32)widths[0];
                        windowData->linux.frameRight = (f32)widths[1];
                        windowData->linux.frameTop = (f32)widths[2];
                        windowData->linux.frameBottom = (f32)widths[3];
                        windowData->linux.flags |= OC_LINUX_WINDOW_X11_FRAME_EXTENTS;
                        if(windowData->linux.queued.frameRectForContentRect.queued)
                        {
                            f32 c = windowData->linux.queued.frameRectForContentRect.user.frameRectForContentRect.c;
                            if(c == 0.0f)  c = 1.0f;
                            oc_rect contentRect = windowData->linux.queued.frameRectForContentRect.user.frameRectForContentRect.contentRect;
                            oc_rect* frameRect = windowData->linux.queued.frameRectForContentRect.user.frameRectForContentRect.frameRect;
                            oc_linux_app_cmd_completion* completion = windowData->linux.queued.frameRectForContentRect.user.frameRectForContentRect.completion;
                            frameRect->x = contentRect.x - windowData->linux.frameLeft * c;
                            frameRect->y = contentRect.y - windowData->linux.frameTop * c;
                            frameRect->w = contentRect.w + (windowData->linux.frameLeft + windowData->linux.frameRight) * c;
                            frameRect->h = contentRect.h + (windowData->linux.frameTop + windowData->linux.frameBottom) * c;
                            oc_linux_app_cmd_completion_signal(completion);
                            windowData->linux.queued.frameRectForContentRect.queued = false;
                            oc_window_destroy(window);
                            /* We most surely have a pendingConfigureNotify, which will be freed upon destroy. */
                        }
                        else
                        {
                            forceNextEvent = windowData->linux.pendingConfigureNotify;
                            windowData->linux.pendingConfigureNotify = NULL;
                        }
                    }
                    else if(prop == linux->x11.atoms._NET_NUMBER_OF_DESKTOPS)
                    {
                        OC_ASSERT(!windowData);
                        OC_ASSERT(reply->type == XCB_ATOM_CARDINAL);
                        OC_ASSERT(reply->format == 32);
                        OC_ASSERT(reply->bytes_after == 0);
                        OC_ASSERT(reply->value_len == 1);
                        linux->x11.netNumberOfDesktops = *(u32*)p;
                        OC_ASSERT(linux->x11.netNumberOfDesktops > 0);
                        OC_ASSERT(linux->x11.netNumberOfDesktops <= oc_array_size(linux->x11.netWorkarea));
                    }
                    else if(prop == linux->x11.atoms._NET_WORKAREA)
                    {
                        OC_ASSERT(!windowData);
                        OC_ASSERT(reply->type == XCB_ATOM_CARDINAL);
                        OC_ASSERT(reply->format == 32);
                        OC_ASSERT(reply->bytes_after == 0);
                        OC_ASSERT(reply->value_len > 0);
                        OC_ASSERT(reply->value_len % 4 == 0);
                        u32* workarea = p;
                        u32 len = reply->value_len / 4;
                        OC_ASSERT(len <= oc_array_size(linux->x11.netWorkarea));
                        for(u32 i = 0; i < len; i++)
                        {
                            linux->x11.netWorkarea[i] = (oc_rect){
                                .x = (f32)workarea[i * 4 + 0],
                                .y = (f32)workarea[i * 4 + 1],
                                .w = (f32)workarea[i * 4 + 2],
                                .h = (f32)workarea[i * 4 + 3],
                            };
                        }
                        linux->x11.netWorkareaLen = len;
                    }
                    else if(prop == linux->x11.atoms._NET_WM_DESKTOP)
                    {
                        OC_ASSERT(windowData);
                        OC_ASSERT(reply->type == XCB_ATOM_CARDINAL);
                        OC_ASSERT(reply->format == 32);
                        OC_ASSERT(reply->bytes_after == 0);
                        OC_ASSERT(reply->value_len == 1);
                        windowData->linux.netWmDesktop = *(u32*)p;
                        OC_ASSERT(windowData->linux.netWmDesktop < linux->x11.netNumberOfDesktops);
                    }
                    else if(prop == linux->x11.atoms.WM_STATE)
                    {
                        OC_ASSERT(windowData);
                        OC_ASSERT(reply->type == linux->x11.atoms.WM_STATE);
                        OC_ASSERT(reply->format == 32);
                        OC_ASSERT(reply->bytes_after == 0);
                        OC_ASSERT(reply->value_len == sizeof(x11_wm_state) / 4);
                        x11_wm_state* wmState = p;
                        x11_window_state oldState = windowData->linux.state;
                        windowData->linux.state = wmState->state;
                        if(oldState != wmState->state)
                        {
                            if(wmState->state == X11_WINDOW_STATE_NORMAL)
                            {
                                oc_event event;
                                memz(&event, sizeof(event));
                                event.type = OC_EVENT_WINDOW_SHOW;
                                event.window = window;
                                oc_linux_queue_event(&event);
                            }
                            else if(oldState == X11_WINDOW_STATE_NORMAL)
                            {
                                oc_event event;
                                memz(&event, sizeof(event));
                                event.type = OC_EVENT_WINDOW_HIDE;
                                event.window = window;
                                oc_linux_queue_event(&event);
                            }
                        }
                    }
                    else if(prop == linux->x11.atoms._NET_WM_STATE)
                    {
                        OC_ASSERT(windowData);
                        OC_ASSERT(reply->type == XCB_ATOM_ATOM);
                        OC_ASSERT(reply->format == 32);
                        OC_ASSERT(reply->bytes_after == 0);
                        xcb_atom_t* hints = p;
                        u32 hintsLen = reply->value_len;
                        OC_ASSERT(hintsLen <= oc_array_size(windowData->linux.netState));
                        memcpy(windowData->linux.netState, hints, hintsLen * 4);
                        memz(&windowData->linux.netState[hintsLen], sizeof(windowData->linux.netState) - hintsLen * 4);
                        windowData->linux.netStateLen = hintsLen;
                    }
                    else if(prop == linux->x11.atoms.OC_X11_CLIPBOARD_DEST)
                    {
                        OC_ASSERT(!windowData);
                        if(reply->type == linux->x11.atoms.UTF8_STRING)
                        {
                            OC_ASSERT(reply->format == 8);
                        }
                        else if(reply->type == XCB_ATOM_INTEGER || reply->type == XCB_ATOM_ATOM)
                        {
                            // FIXME(pld): This ends up with the oc_clipboard
                            // API overloading strings as integer arrays...
                            OC_ASSERT(reply->format == 32);
                        }
                        else if(reply->type == linux->x11.atoms.INCR)
                        {
                            OC_ASSERT(reply->format == 32);
                            /* ICCCM, page 20:
                             * > The contents of the INCR property will be an
                             * > integer, which represents a lower bound on the
                             * > number of bytes of data in the selection.
                             * At least xclip stores an empty property instead,
                             * so test for both. */
                            OC_ASSERT(reply->value_len == 1 || reply->value_len == 0);
                        }
                        else
                        {
                            oc_log_warning("Unsupported clipboard content type: %d\n", reply->type);
                            OC_ASSERT(reply->format == 8 || reply->format == 16 || reply->format == 32);
                        }
                        OC_ASSERT(reply->bytes_after == 0);
                        u32 clipboardLen = reply->value_len * (reply->format / 8);
                        OC_ASSERT(linux->x11.getClipboard.init);
                        u64 resultLen = 0;
                        if(reply->type == linux->x11.atoms.INCR)
                        {
                            OC_ASSERT(!linux->x11.getClipboard.incr);
                            linux->x11.getClipboard.incr = true;
                            oc_arena_init(&linux->x11.getClipboard.incrArena);
                            OC_ASSERT(oc_str8_list_empty(linux->x11.getClipboard.incrParts));
                        }
                        else if(linux->x11.getClipboard.incr)
                        {
                            if(oc_str8_list_empty(linux->x11.getClipboard.incrParts))
                            {
                                OC_ASSERT(linux->x11.getClipboard.incrType == XCB_ATOM_NONE);
                                linux->x11.getClipboard.incrType = reply->type;
                            }
                            else
                            {
                                OC_ASSERT(reply->type == linux->x11.getClipboard.incrType);
                            }
                            if(clipboardLen > 0)
                            {
                                oc_str8 part = oc_str8_push_buffer(&linux->x11.getClipboard.incrArena, clipboardLen, p);
                                oc_str8_list_push(&linux->x11.getClipboard.incrArena, &linux->x11.getClipboard.incrParts, part);
                            }
                            else
                            {
                                resultLen = linux->x11.getClipboard.incrParts.len;
                            }
                        }
                        else
                        {
                            resultLen = clipboardLen;
                        }

                        if(resultLen)
                        {
                            if(linux->x11.getClipboard.arena)
                            {
                                if(linux->x11.getClipboard.incr)
                                {
                                    *linux->x11.getClipboard.result = oc_str8_list_join(linux->x11.getClipboard.arena, linux->x11.getClipboard.incrParts);
                                }
                                else
                                {
                                    *linux->x11.getClipboard.result = oc_str8_push_buffer(linux->x11.getClipboard.arena, resultLen, p);
                                }
                            }
                            else
                            {
                                OC_ASSERT(linux->x11.getClipboard.result->len > 0);
                                OC_ASSERT(linux->x11.getClipboard.result->ptr);
                                usize backingLen = linux->x11.getClipboard.result->len - 1;
                                backingLen = oc_min(backingLen, resultLen);
                                if(linux->x11.getClipboard.incr)
                                {
                                    usize off = 0;
                                    oc_str8_list_for(linux->x11.getClipboard.incrParts, part)
                                    {
                                        usize n = part->string.len;
                                        usize rem = backingLen - off;
                                        if(n > rem)  n = rem;
                                        memcpy(&linux->x11.getClipboard.result->ptr[off], part->string.ptr, n);
                                        off += n;
                                        if(off == backingLen)  break;
                                    }
                                }
                                else
                                {
                                    memcpy(linux->x11.getClipboard.result->ptr, p, backingLen);
                                }
                                linux->x11.getClipboard.result->ptr[backingLen] = '\0';
                                linux->x11.getClipboard.result->len = backingLen;
                            }
                            oc_linux_app_cmd_completion_signal(linux->x11.getClipboard.completion);
                            if(linux->x11.getClipboard.incr)
                            {
                                oc_arena_cleanup(&linux->x11.getClipboard.incrArena);
                            }
                            memz(&linux->x11.getClipboard, sizeof(linux->x11.getClipboard));
                            oc_linux_app_cmd_user* queued = oc_list_pop_front_entry(&linux->x11.getClipboardQueue, __typeof__(*queued), listElt);
                            if(queued)
                            {
                                oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
                                    .cmd = OC_X11_CLIENT_MESSAGE_GET_CLIPBOARD,
                                    .user = *queued,
                                });
                                int ok = oc_mutex_lock(linux->appCmdUserPoolMutex);
                                OC_ASSERT(ok == 0);
                                oc_pool_recycle(&linux->appCmdUserPool, queued);
                                ok = oc_mutex_unlock(linux->appCmdUserPoolMutex);
                                OC_ASSERT(ok == 0);
                            }
                        }
                    }
                    else
                    {
                        oc_notpossible();
                    }
                    free(reply);
                } break;
                case OC_X11_CLIENT_MESSAGE_TRANSLATE_COORDINATES_TO_ROOT:
                {
                    OC_ASSERT(windowData);
                    if(windowData->linux.flags & OC_LINUX_WINDOW_X11_FRAME_EXTENTS)
                    {
                        xcb_translate_coordinates_cookie_t cookie = u->translateCoordinatesToRoot.cookie;
                        u16 since = u->translateCoordinatesToRoot.since;
                        xcb_translate_coordinates_reply_t* reply = NULL;
                        reply = xcb_translate_coordinates_reply(conn, cookie, NULL);
                        OC_ASSERT(reply);
                        OC_ASSERT(reply->response_type == X11_RESPONSE_TYPE_REPLY);
                        OC_ASSERT(reply->same_screen);
                        OC_ASSERT(reply->child != XCB_NONE);
                        if(since == windowData->linux.rectSince)
                        {
                            windowData->linux.rect.x = (f32)reply->dst_x;
                            windowData->linux.rect.y = (f32)reply->dst_y;
                            windowData->linux.flags |= OC_LINUX_WINDOW_X11_POS_KNOWN;

                            oc_event event;
                            memz(&event, sizeof(event));
                            event.type = OC_EVENT_WINDOW_RESIZE;
                            event.window = window;
                            event.move.content = windowData->linux.rect;
                            event.move.frame = event.move.content;
                            event.move.frame.x -= windowData->linux.frameLeft;
                            event.move.frame.y -= windowData->linux.frameTop;
                            event.move.frame.w += windowData->linux.frameLeft + windowData->linux.frameRight;
                            event.move.frame.h += windowData->linux.frameTop + windowData->linux.frameBottom;
                            oc_linux_queue_event(&event);
                            //FIXME(pld): after repainting only
                            if(windowData->linux.netWmSyncRequestUpdateValue)
                            {
                                xcb_sync_counter_t id = windowData->linux.netWmSyncRequestCounterId;
                                xcb_sync_int64_t val =
                                {
                                    .lo = (u32)(windowData->linux.netWmSyncRequestUpdateValue >>  0 & 0xFFFFFFFF),
                                    .hi = (i32)(windowData->linux.netWmSyncRequestUpdateValue >> 32 & 0xFFFFFFFF),
                                };
                                xcb_sync_set_counter(conn, id, val);
                            }
                            while(!oc_list_empty(windowData->linux.queued.getFrameRectQueue))
                            {
                                oc_linux_app_cmd_user* queued = oc_list_pop_front_entry(&windowData->linux.queued.getFrameRectQueue, __typeof__(*queued), listElt);
                                oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
                                    .cmd = OC_X11_CLIENT_MESSAGE_WINDOW_GET_FRAME_RECT,
                                    .window = window,
                                    .user = *queued,
                                });
                                int ok = oc_mutex_lock(linux->appCmdUserPoolMutex);
                                OC_ASSERT(ok == 0);
                                oc_pool_recycle(&linux->appCmdUserPool, queued);
                                ok = oc_mutex_unlock(linux->appCmdUserPoolMutex);
                                OC_ASSERT(ok == 0);
                            }
                        }
                        free(reply);
                    }
                    else
                    {
                        // TODO(pld): do not requeue spuriously?
                        /* Requeue until we get the frame extents. */
                        oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
                            .cmd = OC_X11_CLIENT_MESSAGE_TRANSLATE_COORDINATES_TO_ROOT,
                            .window = window,
                            .user = *u,
                        });
                    }
                } break;
                case OC_X11_CLIENT_MESSAGE_GET_CLIPBOARD:
                {
                    OC_ASSERT(!windowData);
                    if(linux->x11.getClipboard.init)
                    {
                        oc_list_push_back(&linux->x11.getClipboardQueue, &u->listElt);
                        u = NULL;
                        break;
                    }
                    OC_ASSERT(!linux->x11.getClipboard.init);
                    xcb_timestamp_t ts = linux->x11.latestUserTime;
                    xcb_atom_t target = u->getClipboard.target;
                    linux->x11.getClipboard.result = u->getClipboard.result;
                    linux->x11.getClipboard.arena = u->getClipboard.arena;
                    linux->x11.getClipboard.target = target;
                    linux->x11.getClipboard.completion = u->getClipboard.completion;
                    linux->x11.getClipboard.time = ts;
                    linux->x11.getClipboard.init = true;
                    xcb_convert_selection(conn, linux->x11.controlWinId,
                        linux->x11.atoms.CLIPBOARD, target,
                        linux->x11.atoms.OC_X11_CLIPBOARD_DEST, ts);
                } break;
                case OC_X11_CLIENT_MESSAGE_SET_CLIPBOARD:
                {
                    OC_ASSERT(!windowData);
                    if(linux->x11.ownClipboard.requestorsLen > 0)
                    {
                        /* Transfers in progress, wait for them to complete. */
                        linux->x11.ownClipboard.pendingContent = u->setClipboard.content;
                        linux->x11.ownClipboard.hasPendingContent = true;
                        break;
                    }
                    if(linux->x11.ownClipboard.status == OC_X11_CLIPBOARD_STATUS_OWNED &&
                        oc_str8_eq(linux->x11.ownClipboard.content, u->setClipboard.content))
                    {
                        /* Same content and owning clipboard, leave as is. From
                         * ICCCM, page 9:
                         * > If the selection value is modified, but can still
                         * > reasonably be viewed as the same selected object,
                         * > the owner should take no action. */
                        free(u->setClipboard.content.ptr);
                        u->setClipboard.content = (oc_str8){0};
                        break;
                    }
                    xcb_timestamp_t ts = linux->x11.latestUserTime;
                    free(linux->x11.ownClipboard.content.ptr);
                    for(usize i = 0; i < linux->x11.ownClipboard.targetsLen; i++)
                    {
                        free(linux->x11.ownClipboard.targetData[i].ptr);
                    }
                    linux->x11.ownClipboard.targetsLen = 0;
                    memz(linux->x11.ownClipboard.targets, sizeof(linux->x11.ownClipboard.targets));
                    memz(linux->x11.ownClipboard.targetData, sizeof(linux->x11.ownClipboard.targetData));
                    linux->x11.ownClipboard.content = u->setClipboard.content;
                    linux->x11.ownClipboard.acquiredAt = ts;
                    linux->x11.ownClipboard.relinquishedAt = UINT32_MAX;
                    linux->x11.ownClipboard.status = OC_X11_CLIPBOARD_STATUS_ACQUIRING;
                    xcb_set_selection_owner(conn, linux->x11.controlWinId, linux->x11.atoms.CLIPBOARD, ts);
                    xcb_get_selection_owner_cookie_t cookie = {0};
                    cookie = xcb_get_selection_owner(conn, linux->x11.atoms.CLIPBOARD);
                    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
                        .cmd = OC_X11_CLIENT_MESSAGE_GET_SELECTION_OWNER,
                        .user.getSelectionOwner.selection = linux->x11.atoms.CLIPBOARD,
                        .user.getSelectionOwner.cookie = cookie,
                    });
                } break;
                case OC_X11_CLIENT_MESSAGE_SET_CLIPBOARD_TARGET:
                {
                    OC_ASSERT(!windowData);
                    usize i = 0;
                    for(; i < linux->x11.ownClipboard.targetsLen; i++)
                    {
                        if(linux->x11.ownClipboard.targets[i] == u->setClipboardTarget.target)  break;
                    }
                    OC_ASSERT(i < oc_array_size(linux->x11.ownClipboard.targets));
                    free(linux->x11.ownClipboard.targetData[i].ptr);
                    linux->x11.ownClipboard.targets[i] = u->setClipboardTarget.target;
                    linux->x11.ownClipboard.targetData[i] = u->setClipboardTarget.data;
                    if(i == linux->x11.ownClipboard.targetsLen)  linux->x11.ownClipboard.targetsLen++;
                } break;
                case OC_X11_CLIENT_MESSAGE_GET_SELECTION_OWNER:
                {
                    OC_ASSERT(!windowData);
                    xcb_atom_t selection = u->getSelectionOwner.selection;
                    xcb_get_selection_owner_cookie_t cookie = u->getSelectionOwner.cookie;
                    xcb_get_selection_owner_reply_t* reply = NULL;
                    reply = xcb_get_selection_owner_reply(conn, cookie, NULL);
                    OC_ASSERT(reply);
                    OC_ASSERT(reply->response_type == X11_RESPONSE_TYPE_REPLY);
                    if(selection == linux->x11.atoms.CLIPBOARD)
                    {
                        if(linux->x11.ownClipboard.status == OC_X11_CLIPBOARD_STATUS_ACQUIRING)
                        {
                            if(reply->owner == linux->x11.controlWinId)
                            {
                                linux->x11.ownClipboard.status = OC_X11_CLIPBOARD_STATUS_OWNED;
                            }
                            else
                            {
                                linux->x11.ownClipboard.status = OC_X11_CLIPBOARD_STATUS_NOT_OWNED;
                                linux->x11.ownClipboard.relinquishedAt = linux->x11.ownClipboard.acquiredAt;
                            }
                        }
                    }
                    else
                    {
                        oc_notpossible();
                    }
                    free(reply);
                } break;
                case OC_X11_CLIENT_MESSAGE_CLIPBOARD_CLEAR:
                {
                    OC_ASSERT(!windowData);
                    if(linux->x11.ownClipboard.status == OC_X11_CLIPBOARD_STATUS_ACQUIRING ||
                        linux->x11.ownClipboard.status == OC_X11_CLIPBOARD_STATUS_OWNED)
                    {
                        xcb_set_selection_owner(conn, XCB_WINDOW_NONE, linux->x11.atoms.CLIPBOARD,
                            linux->x11.ownClipboard.acquiredAt);
                    }
                } break;
                case OC_X11_CLIENT_MESSAGE_INTERN_ATOM:
                {
                    OC_ASSERT(!windowData);
                    xcb_intern_atom_cookie_t cookie = {0};
                    cookie = xcb_intern_atom(conn, u->internAtom.onlyIfExists,
                        u->internAtom.name.len, u->internAtom.name.ptr);
                    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
                        .cmd = OC_X11_CLIENT_MESSAGE_INTERN_ATOM_REPLY,
                        .user.internAtomReply.cookie = cookie,
                        .user.internAtomReply.atom = u->internAtom.atom,
                        .user.internAtomReply.completion = u->internAtom.completion,
                    });
                } break;
                case OC_X11_CLIENT_MESSAGE_INTERN_ATOM_REPLY:
                {
                    OC_ASSERT(!windowData);
                    xcb_intern_atom_reply_t* reply = NULL;
                    reply = xcb_intern_atom_reply(conn, u->internAtomReply.cookie, NULL);
                    OC_ASSERT(reply);
                    OC_ASSERT(reply->response_type == X11_RESPONSE_TYPE_REPLY);
                    *u->internAtomReply.atom = reply->atom;
                    oc_linux_app_cmd_completion_signal(u->internAtomReply.completion);
                    free(reply);
                } break;
                case OC_X11_CLIENT_MESSAGE_FRAME_RECT_FOR_CONTENT_RECT:
                {
                    OC_ASSERT(!windowData);
                    window = oc_window_create_linux((oc_rect){0,0,1,1}, OC_STR8("Orca - Measuring window"), u->frameRectForContentRect.style, false);
                    windowData = oc_window_ptr_from_handle(window);
                    windowData->linux.queued.frameRectForContentRect.user = *u;
                    windowData->linux.queued.frameRectForContentRect.queued = true;
                } break;
                case OC_X11_CLIENT_MESSAGE_WINDOW_GET_FRAME_RECT:
                {
                    // FIXME(pld): what if window handle is invalid?
                    OC_ASSERT(windowData);
                    if(windowData->linux.rectSince >= windowData->linux.rectNext &&
                        windowData->linux.flags & OC_LINUX_WINDOW_X11_POS_KNOWN &&
                        windowData->linux.flags & OC_LINUX_WINDOW_X11_FRAME_EXTENTS)
                    {
                        OC_ASSERT(!windowData->linux.pendingConfigureNotify);
                        oc_rect rect = windowData->linux.rect;
                        f32 c = u->getFrameRect.c;
                        rect.x -= windowData->linux.frameLeft * c;
                        rect.y -= windowData->linux.frameTop * c;
                        rect.w += (windowData->linux.frameLeft + windowData->linux.frameRight) * c;
                        rect.h += (windowData->linux.frameTop + windowData->linux.frameBottom) * c;
                        *u->getFrameRect.rect = rect;
                        oc_linux_app_cmd_completion_signal(u->getFrameRect.completion);
                    }
                    else
                    {
                        /* Requeue to run once pre-conditions are fulfilled. */
                        oc_list_push_back(&windowData->linux.queued.getFrameRectQueue, &u->listElt);
                        u = NULL;
                    }
                } break;
                default:
                {
                    oc_notpossible();
                } break;
                }
                if(u)
                {
                    int ok = oc_mutex_lock(linux->appCmdUserPoolMutex);
                    OC_ASSERT(ok == 0);
                    oc_pool_recycle(&linux->appCmdUserPool, u);
                    ok = oc_mutex_unlock(linux->appCmdUserPoolMutex);
                    OC_ASSERT(ok == 0);
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
                    window_update_last_user_activity_x(conn, windowData, ts);
                    windowData->shouldClose = true;
                    oc_event event;
                    memz(&event, sizeof(event));
                    event.type = OC_EVENT_WINDOW_CLOSE;
                    event.window = window;
                    oc_linux_queue_event(&event);
                }
                else if(protocol == linux->x11.atoms._NET_WM_PING)
                {
                    noti->window = linux->x11.rootWinId;
                    xcb_send_event(conn, false, linux->x11.rootWinId,
                        XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
                        XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
                        (const char*)noti);
                }
                else if(protocol == linux->x11.atoms._NET_WM_SYNC_REQUEST)
                {
                    u64 lo = (u64)noti->data.data32[2];
                    u64 hi = (u64)noti->data.data32[3];
                    windowData->linux.netWmSyncRequestUpdateValue = (hi << 32) + lo;
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
            /* ignored */
            break;
        default:
        if(eventCode == linux->x11.keyboard.xkbFirstEventCode)
        {
            u32 xkbEventCode = ev->pad0;
            switch(xkbEventCode)
            {
            case XCB_XKB_NEW_KEYBOARD_NOTIFY:
            {
                xcb_xkb_new_keyboard_notify_event_t* noti = (xcb_xkb_new_keyboard_notify_event_t*)ev;
                oc_log_info("xkb new keyboard: dev=%hhu->%hhu, changed=%hu, minkeycode=%hhu->%hhu, maxkeycode=%hhu->%hhu, req=%hhu/%hhu, time=%u\n",
                    noti->oldDeviceID, noti->deviceID, noti->changed, noti->oldMinKeyCode, noti->minKeyCode,
                    noti->oldMaxKeyCode, noti->maxKeyCode, noti->requestMajor, noti->requestMinor, noti->time);
                if(noti->oldDeviceID == linux->x11.keyboard.deviceId)
                {
                    linux->x11.keyboard.reloadKeymap = true;
                }
            } break;
            case XCB_XKB_MAP_NOTIFY:
            {
                xcb_xkb_map_notify_event_t* noti = (xcb_xkb_map_notify_event_t*)ev;
                oc_log_info("xkb map: dev=%hhu, changed=%hu, time=%u\n", noti->deviceID, noti->changed, noti->time);
                if(noti->deviceID == linux->x11.keyboard.deviceId)
                {
                    linux->x11.keyboard.reloadKeymap = true;
                }
            } break;
            case XCB_XKB_STATE_NOTIFY:
            {
                xcb_xkb_state_notify_event_t* noti = (xcb_xkb_state_notify_event_t*)ev;
                oc_log_info("xkb state: dev=%hhu, changed=%hu, ptr=%hu, time=%u\n",
                    noti->deviceID, noti->changed, noti->ptrBtnState, noti->time);
                reload_x11_keymap_if_needed();
                xkb_state_update_mask(linux->x11.keyboard.state,
                    noti->baseMods, noti->latchedMods, noti->lockedMods,
                    noti->baseGroup, noti->latchedGroup, noti->lockedGroup);
            } break;
            case XCB_XKB_CONTROLS_NOTIFY:
            {
                xcb_xkb_controls_notify_event_t* noti = (xcb_xkb_controls_notify_event_t*)ev;
                oc_log_info("xkb controls: dev=%hhu, numGroups=%hhu, changedControls=%u, enabledControls=%u,"
                    " enabledControlChanges=%u, keycode=%hhu, eventType=%hhu, requestMajor=%hhu, requestMinor=%hhu, time=%u\n",
                    noti->deviceID, noti->numGroups, noti->changedControls, noti->enabledControls, noti->enabledControlChanges,
                    noti->keycode, noti->eventType, noti->requestMajor, noti->requestMinor, noti->time);
                if(noti->deviceID == linux->x11.keyboard.deviceId)
                {
                    linux->x11.keyboard.reloadKeymap = true;
                }
            } break;
            case XCB_XKB_INDICATOR_MAP_NOTIFY:
            {
                xcb_xkb_indicator_map_notify_event_t* noti = (xcb_xkb_indicator_map_notify_event_t*)ev;
                oc_log_info("xkb indicator map: dev=%hhu, state=%u, mapChanged=%u, time=%u\n",
                    noti->deviceID, noti->state, noti->mapChanged, noti->time);
                if(noti->deviceID == linux->x11.keyboard.deviceId)
                {
                    linux->x11.keyboard.reloadKeymap = true;
                }
            } break;
            case XCB_XKB_NAMES_NOTIFY:
            {
                xcb_xkb_names_notify_event_t* noti = (xcb_xkb_names_notify_event_t*)ev;
                oc_log_info("xkb names: dev=%hhu, changed=%hu, time=%u\n",
                    noti->deviceID, noti->changed, noti->time);
                if(noti->deviceID == linux->x11.keyboard.deviceId)
                {
                    linux->x11.keyboard.reloadKeymap = true;
                }
            } break;
            case XCB_XKB_COMPAT_MAP_NOTIFY:
            {
                xcb_xkb_compat_map_notify_event_t* noti = (xcb_xkb_compat_map_notify_event_t*)ev;
                oc_log_info("xkb compat map: dev=%hhu, changedGroups=%hhu, firstSI=%hu, nSI=%hu, nTotalSI=%hu\n",
                    noti->deviceID, noti->changedGroups, noti->firstSI, noti->nSI, noti->nTotalSI);
                if(noti->deviceID == linux->x11.keyboard.deviceId)
                {
                    linux->x11.keyboard.reloadKeymap = true;
                }
            } break;
            default:
                oc_notpossible();
                break;
            }
        }
        break;
        }
        if(0 && ev)  log_event(ev);
        free(ev);
        if(linux->mainThreadAppCmdCompletionSignaled)
        {
            break;
        }
        else if(forceNextEvent)
        {
            ev = forceNextEvent, forceNextEvent = NULL;
        }
        else
        {
            ev = xcb_poll_for_event(conn);
        }
    }
    // TODO(pld): handle I/O errors
    if(!ev)  OC_ASSERT(!xcb_connection_has_error(conn));
    {
        int ok = oc_mutex_lock(linux->pumpedEventsMutex);
        OC_ASSERT(ok == 0);
        ok = oc_condition_broadcast(linux->pumpedEventsCond);
        OC_ASSERT(ok == 0);
        ok = oc_mutex_unlock(linux->pumpedEventsMutex);
        OC_ASSERT(ok == 0);
    }
    return;
}

//FIXME(pld): contentRect.wh can't be zero
static oc_window oc_window_create_linux(oc_rect contentRect, oc_str8 title, oc_window_style style, bool emitEvents)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);

    if(style & OC_WINDOW_STYLE_NO_CLOSE)
    {
        oc_log_warning("Unsupported window style: OC_WINDOW_STYLE_NO_CLOSE\n");
    }
    if(style & OC_WINDOW_STYLE_NO_MINIFY)
    {
        oc_log_warning("Unsupported window style: OC_WINDOW_STYLE_NO_MINIFY\n");
    }
    if(style & OC_WINDOW_STYLE_NO_BUTTONS)
    {
        oc_log_warning("Unsupported window style: OC_WINDOW_STYLE_NO_BUTTONS\n");
    }
    if(style & OC_WINDOW_STYLE_POPUPMENU)
    {
        // set _NET_WM_WINDOW_TYPE_POPUP_MENU, override-redirect, setinputfocus to menu when other windows get focus-in?
        oc_log_warning("Unsupported window style: OC_WINDOW_STYLE_POPUPMENU\n");
    }

    u32 winId = xcb_generate_id(conn);
    int ok = oc_mutex_lock(linux->windowPoolMutex);
    OC_ASSERT(ok == 0);
    oc_window_data* windowData = oc_window_alloc();
    OC_ASSERT(windowData);
    oc_window window = oc_window_handle_from_ptr(windowData);
    OC_ASSERT(linux->x11.winIdToHandleLen < oc_array_size(linux->x11.winIdToHandle));
    linux->x11.winIdToHandle[linux->x11.winIdToHandleLen++] = (x11_win_id_to_handle){
      .winId = winId,
      .handle = window,
    };
    ok = oc_mutex_unlock(linux->windowPoolMutex);
    OC_ASSERT(ok == 0);
    memz(((u8*)windowData) + offsetof(__typeof__(*windowData), end_of_internal_data),
        sizeof(*windowData) - offsetof(__typeof__(*windowData), end_of_internal_data));

    const xcb_setup_t* setup = xcb_get_setup(conn);

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
            XCB_EVENT_MASK_KEY_RELEASE |
            XCB_EVENT_MASK_BUTTON_PRESS |
            XCB_EVENT_MASK_BUTTON_RELEASE |
            XCB_EVENT_MASK_ENTER_WINDOW |
            XCB_EVENT_MASK_LEAVE_WINDOW |
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
    x11_wm_hints wmHints =
    {
        .flags = X11_WM_HINTS_INPUT_HINT | X11_WM_HINTS_STATE_HINT,
        .input = !(style & OC_WINDOW_STYLE_NO_FOCUS),
        .initialState = X11_WINDOW_STATE_NORMAL,
    };
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, winId, XCB_ATOM_WM_HINTS,
        XCB_ATOM_WM_HINTS, 32, sizeof(wmHints) / 4, &wmHints);
    x11_wm_normal_hints wmNormalHints =
    {
        .flags = X11_WM_NORMAL_HINTS_PPOSITION | X11_WM_NORMAL_HINTS_PSIZE |
            X11_WM_NORMAL_HINTS_PRESIZEINC | X11_WM_NORMAL_HINTS_PWINGRAVITY,
        .width_inc = 1,
        .height_inc = 1,
        /* The window manager will interpret configure requests' x and y as the
         * frame top-left coordinates, width and height as the content
         * dimensions. */
        .win_gravity = XCB_GRAVITY_NORTH_WEST,
    };
    if(style & OC_WINDOW_STYLE_FIXED_SIZE)
    {
        wmNormalHints.flags |= X11_WM_NORMAL_HINTS_PMINSIZE | X11_WM_NORMAL_HINTS_PMAXSIZE;
        wmNormalHints.min_width = (i32)contentRect.w;
        wmNormalHints.min_height = (i32)contentRect.h;
        wmNormalHints.max_width = (i32)contentRect.w;
        wmNormalHints.max_height = (i32)contentRect.h;
    }
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, winId, XCB_ATOM_WM_NORMAL_HINTS,
        XCB_ATOM_WM_SIZE_HINTS, 32, sizeof(wmNormalHints) / 4, &wmNormalHints);
    if(style & OC_WINDOW_STYLE_NO_TITLE)
    {
        static const x11_motif_wm_hints motifWmHints =
        {
            .flags = X11_MOTIF_WM_HINTS_DECORATIONS,
            .decorations = X11_MOTIF_WM_HINTS_DECOR_BORDER,
        };
        xcb_change_property(conn, XCB_PROP_MODE_REPLACE, winId,
            linux->x11.atoms._MOTIF_WM_HINTS, linux->x11.atoms._MOTIF_WM_HINTS,
            32, sizeof(motifWmHints) / 4, &motifWmHints);
    }
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
    xcb_sync_counter_t counterId = xcb_generate_id(conn);
    xcb_sync_create_counter(conn, counterId, (xcb_sync_int64_t){0});
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, winId, linux->x11.atoms._NET_WM_SYNC_REQUEST_COUNTER,
        XCB_ATOM_CARDINAL, 32, 1, &counterId);
    // TODO(pld): test _NET_WM_ALLOWED_ACTIONS?
    xcb_client_message_event_t msg =
    {
        .response_type = XCB_CLIENT_MESSAGE,
        .format = 32,
        .window = winId,
        .type = linux->x11.atoms._NET_REQUEST_FRAME_EXTENTS,
    };
    xcb_send_event(conn, false, linux->x11.rootWinId,
        XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
        XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
        (const char*)&msg);

    windowData->linux.x11Id = winId;
    windowData->linux.netWmSyncRequestCounterId = counterId;
    windowData->linux.rect = contentRect;
    windowData->linux.flags |= OC_LINUX_WINDOW_X11_POS_KNOWN;
    if(emitEvents)  windowData->linux.flags |= OC_LINUX_WINDOW_EMIT_EVENTS;
    windowData->style = style;

    oc_window_set_title(window, title);

    return (window);
}
oc_window oc_window_create(oc_rect contentRect, oc_str8 title, oc_window_style style)
{
    return (oc_window_create_linux(contentRect, title, style, true));
}

void oc_window_destroy(oc_window window)
{
    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
        .cmd = OC_X11_CLIENT_MESSAGE_WINDOW_DESTROY,
        .window = window,
    });
}

typedef struct oc_window_native_pointer_dispatched_user
{
    oc_window window;
    void* ptr;
} oc_window_native_pointer_dispatched_user;
static i32 oc_window_native_pointer_dispatched(void* user)
{
    oc_window_native_pointer_dispatched_user *u = user;
    OC_ASSERT(u);
    oc_window_data* windowData = oc_window_ptr_from_handle(u->window);
    if(windowData)
    {
        u->ptr = (void*)(uptr)windowData->linux.x11Id;
    }
    return (0);
}
void* oc_window_native_pointer(oc_window window)
{
    oc_window_native_pointer_dispatched_user u = { .window = window };
    oc_dispatch_on_main_thread_sync(oc_window_native_pointer_dispatched, &u);
    return (u.ptr);
}

typedef struct oc_window_should_close_dispatched_user
{
    oc_window window;
    bool shouldClose;
} oc_window_should_close_dispatched_user;
static i32 oc_window_should_close_dispatched(void* user)
{
    oc_window_should_close_dispatched_user* u = user;
    OC_ASSERT(u);
    oc_window_data* windowData = oc_window_ptr_from_handle(u->window);
    u->shouldClose = windowData && windowData->shouldClose;
    return (0);
}
bool oc_window_should_close(oc_window window)
{
    oc_window_should_close_dispatched_user u = { .window = window };
    oc_dispatch_on_main_thread_sync(oc_window_should_close_dispatched, &u);
    return (u.shouldClose);
}

void oc_window_request_close(oc_window window)
{
    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
        .cmd = OC_X11_CLIENT_MESSAGE_WINDOW_REQUEST_CLOSE,
        .window = window,
    });
}

void oc_window_cancel_close(oc_window window)
{
    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
        .cmd = OC_X11_CLIENT_MESSAGE_WINDOW_CANCEL_CLOSE,
        .window = window,
    });
}

void oc_window_set_title(oc_window window, oc_str8 title)
{
    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
        .cmd = OC_X11_CLIENT_MESSAGE_WINDOW_SET_TITLE,
        .window = window,
        .user.setTitle.title = title,
    });
}

void oc_window_show(oc_window window)
{
    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
        .cmd = OC_X11_CLIENT_MESSAGE_WINDOW_SHOW,
        .window = window,
    });
}

typedef struct oc_window_is_hidden_dispatched_user
{
    oc_window window;
    bool isHidden;
} oc_window_is_hidden_dispatched_user;
static i32 oc_window_is_hidden_dispatched(void* user)
{
    oc_window_is_hidden_dispatched_user* u = user;
    oc_window_data* windowData = oc_window_ptr_from_handle(u->window);
    u->isHidden = windowData && windowData->linux.state == X11_WINDOW_STATE_WITHDRAWN;
    return (0);
}
bool oc_window_is_hidden(oc_window window)
{
    oc_window_is_hidden_dispatched_user u = { .window = window };
    oc_dispatch_on_main_thread_sync(oc_window_is_hidden_dispatched, &u);
    return (u.isHidden);
}

void oc_window_hide(oc_window window)
{
    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
        .cmd = OC_X11_CLIENT_MESSAGE_WINDOW_HIDE,
        .window = window,
    });
}

typedef struct oc_window_is_minimized_dispatched_user
{
    oc_window window;
    bool isMinimized;
} oc_window_is_minimized_dispatched_user;
static i32 oc_window_is_minimized_dispatched(void* user)
{
    oc_window_is_minimized_dispatched_user* u = user;
    oc_window_data* windowData = oc_window_ptr_from_handle(u->window);
    u->isMinimized = windowData && windowData->linux.state == X11_WINDOW_STATE_ICONIC;
    return (0);
}
bool oc_window_is_minimized(oc_window window)
{
    oc_window_is_minimized_dispatched_user u = { .window = window };
    oc_dispatch_on_main_thread_sync(oc_window_is_minimized_dispatched, &u);
    return (u.isMinimized);
}

void oc_window_minimize(oc_window window)
{
    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
        .cmd = OC_X11_CLIENT_MESSAGE_WINDOW_MINIMIZE,
        .window = window,
    });
}

typedef struct oc_window_is_maximized_dispatched_user
{
    oc_window window;
    bool isMaximized;
} oc_window_is_maximized_dispatched_user;
static i32 oc_window_is_maximized_dispatched(void* user)
{
    oc_window_is_maximized_dispatched_user* u = user;
    oc_linux_app_data* linux = &oc_appData.linux;
    oc_window_data* windowData = oc_window_ptr_from_handle(u->window);
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
        u->isMaximized = horz && vert;
    }
    return (0);
}
bool oc_window_is_maximized(oc_window window)
{
    oc_window_is_maximized_dispatched_user u = { .window = window };
    oc_dispatch_on_main_thread_sync(oc_window_is_maximized_dispatched, &u);
    return (u.isMaximized);
}

void oc_window_maximize(oc_window window)
{
    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
        .cmd = OC_X11_CLIENT_MESSAGE_WINDOW_SHOW,
        .window = window,
    });
    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
        .cmd = OC_X11_CLIENT_MESSAGE_WINDOW_MAXIMIZE,
        .window = window,
    });
}

void oc_window_restore(oc_window window)
{
    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
        .cmd = OC_X11_CLIENT_MESSAGE_WINDOW_SHOW,
        .window = window,
    });
    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
        .cmd = OC_X11_CLIENT_MESSAGE_WINDOW_RESTORE,
        .window = window,
    });
}

typedef struct oc_window_has_focus_dispatched_user
{
    oc_window window;
    bool hasFocus;
} oc_window_has_focus_dispatched_user;
static i32 oc_window_has_focus_dispatched(void* user)
{
    oc_window_has_focus_dispatched_user* u = user;
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);
    oc_window_data* windowData = oc_window_ptr_from_handle(u->window);
    u->hasFocus = windowData && windowData->linux.focus == OC_LINUX_WINDOW_FOCUSED;
    return (0);
}
bool oc_window_has_focus(oc_window window)
{
    oc_window_has_focus_dispatched_user u = { .window = window };
    oc_dispatch_on_main_thread_sync(oc_window_has_focus_dispatched, &u);
    return (u.hasFocus);
}

void oc_window_focus(oc_window window)
{
    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
        .cmd = OC_X11_CLIENT_MESSAGE_WINDOW_FOCUS,
        .window = window,
    });
}

void oc_window_unfocus(oc_window window)
{
    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
        .cmd = OC_X11_CLIENT_MESSAGE_WINDOW_UNFOCUS,
        .window = window,
    });
}

void oc_window_send_to_back(oc_window window)
{
    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
        .cmd = OC_X11_CLIENT_MESSAGE_WINDOW_SEND_TO_BACK,
        .window = window,
    });
}

void oc_window_bring_to_front(oc_window window)
{
    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
        .cmd = OC_X11_CLIENT_MESSAGE_WINDOW_BRING_TO_FRONT,
        .window = window,
    });
}

u64 oc_linux_debug_window_stack_pos(oc_window window)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    OC_ASSERT(windowData);

    xcb_query_tree_cookie_t cookie = xcb_query_tree(conn, windowData->linux.x11Id);
    ensure_xcb_flush(conn);
    xcb_query_tree_reply_t* reply = NULL;
    reply = xcb_query_tree_reply(conn, cookie, NULL);
    OC_ASSERT(reply);
    OC_ASSERT(reply->response_type == X11_RESPONSE_TYPE_REPLY);
    OC_ASSERT(reply->root == linux->x11.rootWinId);
    OC_ASSERT(reply->parent != XCB_NONE);
    xcb_window_t parent = reply->parent;
    free(reply);

    cookie = xcb_query_tree(conn, linux->x11.rootWinId);
    ensure_xcb_flush(conn);
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
    free(reply);
    return (i);
}

oc_rect oc_linux_debug_window_workarea(oc_window window)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    OC_ASSERT(windowData);
    OC_ASSERT(windowData->linux.netWmDesktop < linux->x11.netNumberOfDesktops);
    OC_ASSERT(windowData->linux.netWmDesktop < linux->x11.netWorkareaLen);

    return (linux->x11.netWorkarea[windowData->linux.netWmDesktop]);
}

static oc_rect oc_linux_window_get_frame_rect(oc_window window, f32 c)
{
    oc_linux_app_cmd_completion completion = oc_linux_app_cmd_completion_create();
    oc_rect rect = {0};
    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
        .cmd = OC_X11_CLIENT_MESSAGE_WINDOW_GET_FRAME_RECT,
        .window = window,
        .user.getFrameRect.rect = &rect,
        .user.getFrameRect.c = c,
        .user.getFrameRect.completion = &completion,
    });
    oc_linux_app_cmd_completion_wait(&completion);
    oc_linux_app_cmd_completion_destroy(&completion);
    return (rect);
}

oc_rect oc_window_get_frame_rect(oc_window window)
{
    return (oc_linux_window_get_frame_rect(window, 1.0f));
}

void oc_window_set_frame_rect(oc_window window, oc_rect rect)
{
    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
        .cmd = OC_X11_CLIENT_MESSAGE_WINDOW_SET_FRAME_RECT,
        .window = window,
        .user.setFrameRect.rect = rect,
    });
}

oc_rect oc_window_get_content_rect(oc_window window)
{
    return (oc_linux_window_get_frame_rect(window, 0.0f));
}

void oc_window_set_content_rect(oc_window window, oc_rect rect)
{
    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
        .cmd = OC_X11_CLIENT_MESSAGE_WINDOW_SET_CONTENT_RECT,
        .window = window,
        .user.setContentRect.rect = rect,
    });
}

void oc_window_center(oc_window window)
{
    oc_rect rect = oc_window_get_content_rect(window);
    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
        .cmd = OC_X11_CLIENT_MESSAGE_WINDOW_CENTER,
        .window = window,
        .user.center.contentWh = rect.wh,
    });
}

oc_rect oc_window_frame_rect_for_content_rect(oc_rect contentRect, oc_window_style style)
{
    oc_linux_app_cmd_completion completion = oc_linux_app_cmd_completion_create();
    oc_rect frameRect = {0};
    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
        .cmd = OC_X11_CLIENT_MESSAGE_FRAME_RECT_FOR_CONTENT_RECT,
        .user.frameRectForContentRect.style = style,
        .user.frameRectForContentRect.contentRect = contentRect,
        .user.frameRectForContentRect.frameRect = &frameRect,
        .user.frameRectForContentRect.completion = &completion,
    });
    oc_linux_app_cmd_completion_wait(&completion);
    oc_linux_app_cmd_completion_destroy(&completion);
    return (frameRect);
}
oc_rect oc_window_content_rect_for_frame_rect(oc_rect frameRect, oc_window_style style)
{
    oc_linux_app_cmd_completion completion = oc_linux_app_cmd_completion_create();
    oc_rect contentRect = {0};
    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
        .cmd = OC_X11_CLIENT_MESSAGE_FRAME_RECT_FOR_CONTENT_RECT,
        .user.frameRectForContentRect.style = style,
        .user.frameRectForContentRect.contentRect = frameRect,
        .user.frameRectForContentRect.frameRect = &contentRect,
        .user.frameRectForContentRect.c = -1.0f,
        .user.frameRectForContentRect.completion = &completion,
    });
    oc_linux_app_cmd_completion_wait(&completion);
    oc_linux_app_cmd_completion_destroy(&completion);
    return (contentRect);
}

static void oc_linux_dispatch_sync_request_tls_destructor(void* user)
{
    oc_linux_dispatch_sync_request* req = user;
    int ok = oc_mutex_lock(req->mutex);
    OC_ASSERT(ok == 0);
    oc_linux_dispatch_sync_request_refcount_dec(req);
}
static oc_linux_dispatch_sync_result oc_dispatch_on_main_thread_sync_timed(oc_dispatch_proc proc, void* user, f64 timeout)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);
    static oc_thread_local oc_linux_dispatch_sync_request* threadReq = NULL;
    static oc_thread_local bool threadInit = false;
    if(!threadInit)
    {
        threadReq = calloc(1, sizeof(*threadReq));
        OC_ASSERT(threadReq);
        threadReq->cond = oc_condition_create();
        OC_ASSERT(threadReq->cond);
        threadReq->mutex = oc_mutex_create();
        OC_ASSERT(threadReq->mutex);
        threadReq->refcount = 1;
        oc_add_tls_destructor(oc_linux_dispatch_sync_request_tls_destructor, threadReq);
        threadInit = true;
    }
    int ok = oc_mutex_lock(threadReq->mutex);
    OC_ASSERT(ok == 0);
    threadReq->proc = proc;
    threadReq->user = user;
    threadReq->reqId++;
    threadReq->refcount++;
    threadReq->result.didRun = false;
    OC_STATIC_ASSERT(sizeof(&threadReq) == sizeof(u64));
    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
        .cmd = OC_X11_CLIENT_MESSAGE_DISPATCH_ON_MAIN_THREAD_SYNC,
        .user.dispatchOnMainThreadSync.req = threadReq,
        .user.dispatchOnMainThreadSync.reqId = threadReq->reqId,
    });

    oc_linux_dispatch_sync_result res = {0};
    if(oc_thread_self_id() == linux->mainThreadId)
    {
        ok = oc_mutex_unlock(threadReq->mutex);
        OC_ASSERT(ok == 0);
        if(timeout < 0.0)
        {
            while(!threadReq->result.didRun)  oc_pump_events_main_thread(-1.0);
        }
        else
        {
            while(!threadReq->result.didRun && timeout >= 0.0)
            {
                f64 start = oc_clock_time(OC_CLOCK_MONOTONIC);
                oc_pump_events_main_thread(timeout);
                f64 elapsed = oc_clock_time(OC_CLOCK_MONOTONIC) - start;
                timeout -= elapsed;
            }
        }
        res = threadReq->result;
    }
    else
    {
        if(timeout < 0.0)
        {
            ok = oc_condition_wait(threadReq->cond, threadReq->mutex);
            OC_ASSERT(ok == 0);
            OC_ASSERT(threadReq->result.didRun);
        }
        else
        {
            while(!threadReq->result.didRun && timeout >= 0.0)
            {
                f64 start = oc_clock_time(OC_CLOCK_MONOTONIC);
                ok = oc_condition_timedwait(threadReq->cond, threadReq->mutex, timeout);
                OC_ASSERT(ok == 0 || ok == EINTR || ok == ETIMEDOUT);
                f64 elapsed = oc_clock_time(OC_CLOCK_MONOTONIC) - start;
                timeout -= elapsed;
            }
        }
        res = threadReq->result;
        ok = oc_mutex_unlock(threadReq->mutex);
        OC_ASSERT(ok == 0);
    }

    return (res);
}
i32 oc_dispatch_on_main_thread_sync(oc_dispatch_proc proc, void* user)
{
    oc_linux_dispatch_sync_result res = oc_dispatch_on_main_thread_sync_timed(proc, user, -1.0);
    OC_ASSERT(res.didRun);
    return (res.retVal);
}

void oc_pump_events(f64 timeout)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    if(oc_thread_self_id() == linux->mainThreadId)
    {
        oc_pump_events_main_thread(timeout);
    }
    else
    {
        int ok = oc_mutex_lock(linux->pumpedEventsMutex);
        OC_ASSERT(ok == 0);
        if(timeout < 0.0)
        {
            ok = oc_condition_wait(linux->pumpedEventsCond, linux->pumpedEventsMutex);
            OC_ASSERT(ok == 0);
        }
        else
        {
            do
            {
                f64 start = oc_clock_time(OC_CLOCK_MONOTONIC);
                ok = oc_condition_timedwait(linux->pumpedEventsCond, linux->pumpedEventsMutex, timeout);
                f64 end = oc_clock_time(OC_CLOCK_MONOTONIC);
                timeout = oc_max(timeout - (end - start), 0.0);
            }
            while(ok == EINTR);
            OC_ASSERT(ok == 0 || ok == ETIMEDOUT);
        }
        ok = oc_mutex_unlock(linux->pumpedEventsMutex);
        OC_ASSERT(ok == 0);
    }
}

static xcb_atom_t oc_linux_x11_intern_atom(oc_str8 name, bool onlyIfExists)
{
    xcb_atom_t atom = XCB_ATOM_NONE;
    oc_linux_app_cmd_completion completion = oc_linux_app_cmd_completion_create();
    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
        .cmd = OC_X11_CLIENT_MESSAGE_INTERN_ATOM,
        .user.internAtom.name = name,
        .user.internAtom.onlyIfExists = onlyIfExists,
        .user.internAtom.atom = &atom,
        .user.internAtom.completion = &completion,
    });
    oc_linux_app_cmd_completion_wait(&completion);
    oc_linux_app_cmd_completion_destroy(&completion);
    return (atom);
}

// TODO(pld): clipboard: history of selections to handle late requestors?
// TODO(pld): clipboard: handle X11 Alloc errors?
// TODO(pld): clipboard: other built-in targets to support?
// - CLASS
// - CHARACTER_POSITION
// - COLUMN_NUMBER
// - CLIENT_WINDOW
// - HOST_NAME
// - LINE_NUMBER
// - MULTIPLE
// - NAME
// - OWNER_OS
// - PROCESS
// - TASK
// - USER
// - DELETE
// - INSERT_PROPERTY
// - INSERT_SELECTION
// - text/plain
// - text/plain;charset=utf-8
// - other mime types
// won't support:
// - ADOBE_PORTABLE_DOCUMENT_FORMAT
// - APPLE_PICT
// - BACKGROUND
// - BITMAP
// - COLORMAP
// - DRAWABLE
// - ENCAPSULATED_POSTSCRIPT
// - ENCAPSULATED_POSTSCRIPT_INTERCHANGE
// - FILE_NAME
// - FOREGROUND
// - LIST_LENGTH
// - MODULE
// - ODIF
// - PIXMAP
// - POSTSCRIPT
// - PROCEDURE
// - STRING
// - COMPOUND_TEXT
// - LENGTH (deprecated)
// to read up:
// - TEXT
// - INCR
// - DRAWABLE
// - SPAN
// - manager selections
void oc_clipboard_clear(void)
{
    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
        .cmd = OC_X11_CLIENT_MESSAGE_CLIPBOARD_CLEAR,
    });
}

static oc_str8 oc_linux_get_clipboard(oc_arena* arena, oc_str8 backing, xcb_atom_t target)
{
    if(arena)  OC_ASSERT(!backing.ptr && backing.len == 0);
    else  OC_ASSERT(backing.ptr && backing.len > 0);
    OC_ASSERT(target != XCB_ATOM_NONE);
    oc_str8 s = {0};
    if(!arena)  s = backing;
    oc_linux_app_cmd_completion completion = oc_linux_app_cmd_completion_create();
    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
        .cmd = OC_X11_CLIENT_MESSAGE_GET_CLIPBOARD,
        .user.getClipboard.result = &s,
        .user.getClipboard.arena = arena,
        .user.getClipboard.target = target,
        .user.getClipboard.completion = &completion,
    });
    oc_linux_app_cmd_completion_wait(&completion);
    oc_linux_app_cmd_completion_destroy(&completion);
    return (s);
}

oc_str8 oc_clipboard_get_string(oc_arena* arena)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    return (oc_linux_get_clipboard(arena, (oc_str8){0}, linux->x11.atoms.TEXT));
}

/* Backing comprises a NUL terminator */
// TODO(pld): what if backing.len is 0, we can't guarantee the nul terminator?
oc_str8 oc_clipboard_copy_string(oc_str8 backing)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    return (oc_linux_get_clipboard(NULL, backing, linux->x11.atoms.TEXT));
}

oc_str8 oc_clipboard_get_data_for_tag(oc_arena* arena, const char* tag)
{
    OC_ASSERT(tag);
    xcb_atom_t target = oc_linux_x11_intern_atom(OC_STR8(tag), true);
    if(target == XCB_ATOM_NONE)  return (OC_STR8(""));
    return (oc_linux_get_clipboard(arena, (oc_str8){0}, target));
}

bool oc_clipboard_has_tag(const char* tag)
{
    OC_ASSERT(tag);
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_atom_t target = oc_linux_x11_intern_atom(OC_STR8(tag), true);
    if(target == XCB_ATOM_NONE)  return (false);
    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8 s = oc_linux_get_clipboard(scratch.arena, (oc_str8){0}, linux->x11.atoms.TARGETS);
    OC_ASSERT(s.len % sizeof(xcb_atom_t) == 0);
    bool found = false;
    for(usize i = 0; i < s.len; i += sizeof(xcb_atom_t))
    {
        /* May be misaligned (triggers UBSAN), we need to load the value via memcpy. */
        xcb_atom_t target0;
        memcpy(&target0, &s.ptr[i], sizeof(target0));
        found = target0 == target;
        if(found)  break;
    }
    oc_scratch_end(scratch);
    return (found);
}

void oc_clipboard_set_string(oc_str8 string)
{
    OC_ASSERT(string.ptr && string.len > 0);
    char* buf = memdup(string.ptr, string.len);
    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
        .cmd = OC_X11_CLIENT_MESSAGE_SET_CLIPBOARD,
        .user.setClipboard.content = oc_str8_from_buffer(string.len, buf),
    });
}

void oc_clipboard_set_data_for_tag(const char* tag, oc_str8 data)
{
    OC_ASSERT(tag);
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_atom_t target = oc_linux_x11_intern_atom(OC_STR8(tag), false);
    OC_ASSERT(target != XCB_ATOM_NONE);
    if(target == linux->x11.atoms.TARGETS ||
        target == linux->x11.atoms.TEXT ||
        target == linux->x11.atoms.UTF8_STRING ||
        target == linux->x11.atoms.TIMESTAMP)
    {
        oc_log_error("Clipboard tag \"%s\" cannot be set on Linux hosts, it is already implicitly set via oc_clipboard_set_string\n", tag);
        return;
    }
    char* buf = memdup(data.ptr, data.len);
    oc_linux_enqueue_app_cmd(&(oc_linux_app_cmd){
        .cmd = OC_X11_CLIENT_MESSAGE_SET_CLIPBOARD_TARGET,
        .user.setClipboardTarget.target = target,
        .user.setClipboardTarget.data = oc_str8_from_buffer(data.len, buf),
    });
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

static void x11_xtest_send_void_request(void* req, usize len)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);
    struct iovec vec = { .iov_base = req, .iov_len = len };
    xcb_protocol_request_t desc = { .count = 1, .isvoid = true };
    int flags = XCB_REQUEST_RAW | XCB_REQUEST_CHECKED;
    u32 seq = xcb_send_request(conn, flags, &vec, &desc);
    OC_ASSERT(seq);
    ensure_xcb_flush(conn);
    xcb_generic_error_t* e = xcb_request_check(conn, (xcb_void_cookie_t){ seq });
    OC_ASSERT(!e);
}

void oc_linux_debug_fake_key(oc_scan_code scanCode, bool depressed)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_keycode_t kc = 0;
    for (usize i = 0; i < oc_array_size(oc_appData.scanCodes); i++)
    {
        if(oc_appData.scanCodes[i] == scanCode)
        {
            kc = i;
            break;
        }
    }
    OC_ASSERT(kc);
    x11_xtest_fake_input_req req =
    {
        .majorCode = linux->x11.xtestMajorCode,
        .minorCode = X11_XTEST_REQUEST_FAKE_INPUT,
        .len = sizeof(req) / 4,
        .eventType = depressed ? X11_XTEST_EVENT_KEY_PRESS : X11_XTEST_EVENT_KEY_RELEASE,
        .detail = kc,
    };
    x11_xtest_send_void_request(&req, sizeof(req));
}

void oc_linux_debug_fake_mouse_move(i16 x, i16 y, bool absolute)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    x11_xtest_fake_input_req req =
    {
        .majorCode = linux->x11.xtestMajorCode,
        .minorCode = X11_XTEST_REQUEST_FAKE_INPUT,
        .len = sizeof(req) / 4,
        .eventType = X11_XTEST_EVENT_MOTION_NOTIFY,
        .detail = !absolute,
        .motionWindow = linux->x11.rootWinId,
        .motionX = x,
        .motionY = y,
    };
    x11_xtest_send_void_request(&req, sizeof(req));
}

void oc_linux_debug_fake_mouse_button(oc_mouse_button button, bool depressed)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_button_t buttons[] =
    {
        [OC_MOUSE_LEFT] = 1,
        [OC_MOUSE_MIDDLE] = 2,
        [OC_MOUSE_RIGHT] = 3,
        [OC_MOUSE_EXT1] = 8,
        [OC_MOUSE_EXT2] = 9,
    };
    OC_ASSERT(button >= 0 && button < OC_MOUSE_BUTTON_COUNT);
    x11_xtest_fake_input_req req =
    {
        .majorCode = linux->x11.xtestMajorCode,
        .minorCode = X11_XTEST_REQUEST_FAKE_INPUT,
        .len = sizeof(req) / 4,
        .eventType = depressed ? X11_XTEST_EVENT_BUTTON_PRESS : X11_XTEST_EVENT_BUTTON_RELEASE,
        .detail = buttons[button],
    };
    x11_xtest_send_void_request(&req, sizeof(req));
}

void oc_linux_debug_fake_mouse_wheel(oc_linux_debug_wheel_direction direction, usize n)
{
    oc_linux_app_data* linux = &oc_appData.linux;
    u8 wheelDirections[] =
    {
        [OC_LINUX_DEBUG_WHEEL_UP] = X11_BUTTON_WHEEL_UP,
        [OC_LINUX_DEBUG_WHEEL_DOWN] = X11_BUTTON_WHEEL_DOWN,
        [OC_LINUX_DEBUG_WHEEL_LEFT] = X11_BUTTON_WHEEL_LEFT,
        [OC_LINUX_DEBUG_WHEEL_RIGHT] = X11_BUTTON_WHEEL_RIGHT,
    };
    OC_ASSERT(direction >= 0 && direction < oc_array_size(wheelDirections));
    x11_xtest_fake_input_req req =
    {
        .majorCode = linux->x11.xtestMajorCode,
        .minorCode = X11_XTEST_REQUEST_FAKE_INPUT,
        .len = sizeof(req) / 4,
        .detail = wheelDirections[direction],
    };
    int seq = 0;
    for(usize i = 0; i < n; i++)
    {
        req.eventType = X11_XTEST_EVENT_BUTTON_PRESS;
        x11_xtest_send_void_request(&req, sizeof(req));
        req.eventType = X11_XTEST_EVENT_BUTTON_RELEASE;
        x11_xtest_send_void_request(&req, sizeof(req));
    }
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
