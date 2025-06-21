#include "orca.h"

int main(void)
{
    // platform_debug
    oc_log_set_level(OC_LOG_LEVEL_INFO);
    oc_log_set_output(OC_LOG_DEFAULT_OUTPUT);
    oc_log_info("Hello\n");
    if(0)  OC_ABORT("Testing abort");
    if(0)  OC_ASSERT(0, "Test assert");

    // TODO(pld): test platform_thread
    // TODO(pld): test platform_path
    // TODO(pld): test platform_clock
    // TODO(pld): test platform_io
    // TODO(pld): test platform_memory
    // TODO(pld): test platform_io_dialog
    // TODO(pld): test platform_io_internal

    oc_init();
    oc_clock_init();

    // TODO(pld): test app.h
    // - oc_init
    // - oc_terminate
    // - oc_should_quit
    // - oc_request_quit
    // - oc_set_cursor
    // - oc_pump_events
    // - oc_next_event
    // - oc_scancode_to_keycode
    // - oc_window_create
    // - oc_window_destroy
    // - oc_window_native_pointer
    // - oc_window_should_close
    // - oc_window_request_close
    // - oc_window_cancel_close
    // - oc_window_is_hidden
    // - oc_window_hide
    // - oc_window_show
    // - oc_window_set_title
    // - oc_window_is_minimized
    // - oc_window_is_maximized
    // - oc_window_minimize
    // - oc_window_maximize
    // - oc_window_restore
    // - oc_window_has_focus
    // - oc_window_focus
    // - oc_window_unfocus
    // - oc_window_send_to_back
    // - oc_window_send_to_front
    // - oc_window_get_frame_rect
    // - oc_window_set_frame_rect
    // - oc_window_set_frame_position
    // - oc_window_set_frame_size
    // - oc_window_get_content_rect
    // - oc_window_set_content_rect
    // - oc_window_set_content_position
    // - oc_window_set_content_size
    // - oc_window_center
    // - oc_window_content_rect_for_frame_rect
    // - oc_window_frame_rect_for_content_rect
    // - oc_vsync_init
    // - oc_vsync_wait
    // - oc_dispatch_on_main_thread_sync
    // - oc_clipboard_clear
    // - oc_clipboard_set_string
    // - oc_clipboard_get_string
    // - oc_clipboard_copy_string
    // - oc_clipboard_has_tag
    // - oc_clipboard_set_data_for_tag
    // - oc_clipboard_get_data_for_tag
    // - oc_file_dialog
    // - oc_file_dialog_for_table
    // - oc_alert_popup
    // - oc_file_move
    // - oc_file_remove
    // - oc_directory_create
    // - oc_window_set_size
    // TODO(pld): graphics
    // TODO(pld): text
    // TODO(pld): ui

    return 0;
}
