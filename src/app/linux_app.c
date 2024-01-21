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
#include <xcb/xproto.h>

static inline void* memz(void* buf, size_t n)
{
    return memset(buf, 0, n);
}

void oc_init(void)
{
    memset(&oc_appData, 0, sizeof(oc_appData));
    oc_linux_app_data* linux = &oc_appData.linux;

    oc_init_common();

    oc_arena_scope scratch = oc_scratch_begin();

    oc_linux_x11* x11 = &linux->x11;

    x11->display = XOpenDisplay(NULL);
    OC_ASSERT(x11->display);

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
    oc_terminate_common();
    OC_ASSERT(0 && "Unimplemented");
    return;
}
bool oc_should_quit(void)
{
    // TODO(pld)
    return false;
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
void oc_pump_events(f64 timeout)
{
    // TODO(pld)
    return;
}

oc_window oc_window_create(oc_rect contentRect, oc_str8 title, oc_window_style style)
{
    // TODO(pld): title (requires WM integration)
    // TODO(pld): style
    oc_linux_app_data* linux = &oc_appData.linux;
    xcb_connection_t* conn = XGetXCBConnection(linux->x11.display);

    const xcb_setup_t* setup = xcb_get_setup(conn);

    u32 win_id = xcb_generate_id(conn);
    u32 parent_id = 0;
    {
        xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
        OC_ASSERT(iter.rem > 0);
        xcb_screen_t *screen = iter.data;
        parent_id = screen->root;
    }
    enum xcb_cw_t attributes_mask = XCB_CW_BACK_PIXMAP |
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
        .colormap = XCB_COPY_FROM_PARENT,
        .cursor = XCB_CURSOR_NONE,
    };

    UNUSED
    xcb_void_cookie_t cw_cookie = xcb_create_window_aux(conn, 0, win_id,
        parent_id, contentRect.x, contentRect.y, contentRect.w, contentRect.h,
        0, XCB_WINDOW_CLASS_INPUT_OUTPUT, XCB_COPY_FROM_PARENT,
        attributes_mask, &attributes);

    UNUSED
    xcb_void_cookie_t map_cookie = xcb_map_window(conn, win_id);

    xcb_flush(conn);

    oc_window_data* window_data = oc_window_alloc();
    window_data->linux.x11_id = win_id;
    oc_window window = oc_window_handle_from_ptr(window_data);

    return window;
}

void oc_window_destroy(oc_window window)
{
    OC_ASSERT(0 && "Unimplemented");
    return;
}
void* oc_window_native_pointer(oc_window window)
{
    OC_ASSERT(0 && "Unimplemented");
    return NULL;
}

bool oc_window_should_close(oc_window window)
{
    OC_ASSERT(0 && "Unimplemented");
    return false;
}
void oc_window_request_close(oc_window window)
{
    OC_ASSERT(0 && "Unimplemented");
    return;
}
void oc_window_cancel_close(oc_window window)
{
    OC_ASSERT(0 && "Unimplemented");
    return;
}
bool oc_window_is_hidden(oc_window window)
{
    OC_ASSERT(0 && "Unimplemented");
    return false;
}
void oc_window_hide(oc_window window)
{
    OC_ASSERT(0 && "Unimplemented");
    return;
}
void oc_window_show(oc_window window)
{
    OC_ASSERT(0 && "Unimplemented");
    return;
}
void oc_window_set_title(oc_window window, oc_str8 title)
{
    OC_ASSERT(0 && "Unimplemented");
    return;
}
bool oc_window_is_minimized(oc_window window)
{
    OC_ASSERT(0 && "Unimplemented");
    return false;
}
bool oc_window_is_maximized(oc_window window)
{
    OC_ASSERT(0 && "Unimplemented");
    return false;
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
    OC_ASSERT(0 && "Unimplemented");
    return false;
}
void oc_window_focus(oc_window window)
{
    OC_ASSERT(0 && "Unimplemented");
    return;
}
void oc_window_unfocus(oc_window window)
{
    OC_ASSERT(0 && "Unimplemented");
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
    return (oc_rect){0};
}
void oc_window_set_frame_rect(oc_window window, oc_rect rect)
{
    OC_ASSERT(0 && "Unimplemented");
    return;
}
oc_rect oc_window_get_content_rect(oc_window window)
{
    OC_ASSERT(0 && "Unimplemented");
    return (oc_rect){0};
}
void oc_window_set_content_rect(oc_window window, oc_rect rect)
{
    OC_ASSERT(0 && "Unimplemented");
    return;
}
void oc_window_center(oc_window window)
{
    OC_ASSERT(0 && "Unimplemented");
    return;
}
oc_rect oc_window_content_rect_for_frame_rect(oc_rect frameRect, oc_window_style style)
{
    OC_ASSERT(0 && "Unimplemented");
    return (oc_rect){0};
}
oc_rect oc_window_frame_rect_for_content_rect(oc_rect contentRect, oc_window_style style)
{
    OC_ASSERT(0 && "Unimplemented");
    return (oc_rect){0};
}
i32 oc_dispatch_on_main_thread_sync(oc_window main_window, oc_dispatch_proc proc, void* user)
{
    OC_ASSERT(0 && "Unimplemented");
    return -1;
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
    return (oc_str8){0};
}
oc_str8 oc_clipboard_copy_string(oc_str8 backing)
{
    OC_ASSERT(0 && "Unimplemented");
    return (oc_str8){0};
}
bool oc_clipboard_has_tag(const char* tag)
{
    OC_ASSERT(0 && "Unimplemented");
    return false;
}
void oc_clipboard_set_data_for_tag(const char* tag, oc_str8 data)
{
    OC_ASSERT(0 && "Unimplemented");
    return;
}
oc_str8 oc_clipboard_get_data_for_tag(oc_arena* arena, const char* tag)
{
    OC_ASSERT(0 && "Unimplemented");
    return (oc_str8){0};
}
oc_file_dialog_result oc_file_dialog_for_table(oc_arena* arena, oc_file_dialog_desc* desc, oc_file_table* table)
{
    OC_ASSERT(0 && "Unimplemented");
    return (oc_file_dialog_result){0};
}
int oc_alert_popup(oc_str8 title, oc_str8 message, oc_str8_list options)
{
    OC_ASSERT(0 && "Unimplemented");
    return -1;
}
int oc_file_move(oc_str8 from, oc_str8 to)
{
    OC_ASSERT(0 && "Unimplemented");
    return -1;
}
int oc_file_remove(oc_str8 path)
{
    OC_ASSERT(0 && "Unimplemented");
    return -1;
}
int oc_directory_create(oc_str8 path)
{
    OC_ASSERT(0 && "Unimplemented");
    return -1;
}

#include "graphics/graphics_surface.h"

void oc_surface_cleanup(oc_surface_data* surface)
{
    OC_ASSERT(0 && "Unimplemented");
    return;
}

oc_vec2 oc_linux_surface_get_size(oc_surface_data* surface)
{
    OC_ASSERT(0 && "Unimplemented");
    return (oc_vec2){0};
}

oc_vec2 oc_linux_surface_contents_scaling(oc_surface_data* surface)
{
    OC_ASSERT(0 && "Unimplemented");
    return (oc_vec2){0};
}

bool oc_linux_surface_get_hidden(oc_surface_data* surface)
{
    OC_ASSERT(0 && "Unimplemented");
    return false;
}

void oc_linux_surface_set_hidden(oc_surface_data* surface, bool hidden)
{
    OC_ASSERT(0 && "Unimplemented");
    return;
}

void* oc_linux_surface_native_layer(oc_surface_data* surface)
{
    return (void *)(uintptr_t)surface->layer.x11_win_id;
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

    surface->layer.x11_win_id = window->linux.x11_id;
}
