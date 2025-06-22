#include "orca.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
typedef ssize_t isize;
typedef size_t usize;

// FIXME(pld): thread name: 64 on MacOS, 20 on FreeBSD, 16 on Linux (with NUL)
// FIXME(pld): clock meanings?

static void print_file(const char *pathname)
{
    int fd = open(pathname, O_RDONLY);
    OC_ASSERT(fd != -1);
    u8 buf[8192];
    isize n = 0;
    do {
        n = read(fd, buf, oc_array_size(buf));
        OC_ASSERT(n >= 0);
        dprintf(2, "%.*s", (int)n, buf);
    } while(n > 0);
    close(fd);
}

int main(void)
{
    // platform_debug
    {
        oc_log_set_level(OC_LOG_LEVEL_INFO);
        oc_log_set_output(OC_LOG_DEFAULT_OUTPUT);
        oc_log_info("Logs work\n");
        if(0)  OC_ABORT("Testing abort");
        if(0)  OC_ASSERT(0, "Test assert");
    }

    // platform_clock
    {
        oc_clock_init();
        f64 a = 0.0, b = 0.0;
        a = oc_clock_time(OC_CLOCK_MONOTONIC);
        b = oc_clock_time(OC_CLOCK_MONOTONIC);
        OC_ASSERT(a < b);
        oc_log_info("monotonic: %f\n", a);
        a = oc_clock_time(OC_CLOCK_UPTIME);
        b = oc_clock_time(OC_CLOCK_UPTIME);
        OC_ASSERT(a < b);
        oc_log_info("uptime: %f\n", a);
        a = oc_clock_time(OC_CLOCK_DATE);
        b = oc_clock_time(OC_CLOCK_DATE);
        OC_ASSERT(a < b);
        oc_log_info("date: %f\n", a);
    }

    // platform_memory
    {
        const char *statm = "/proc/self/statm";
        oc_base_allocator* base = oc_base_allocator_default();
        oc_log_info("memory: initial:         "), print_file(statm);
        u8* p = oc_base_reserve(base, 1 << 30);
        oc_log_info("memory: reserved:        "), print_file(statm);
        oc_base_commit(base, p, 1 << 30);
        oc_log_info("memory: committed:       "), print_file(statm);
        p[0] = 1;
        oc_log_info("memory: wrote a page:    "), print_file(statm);
        oc_base_decommit(base, p, 1 << 30);
        oc_log_info("memory: decommitted:     "), print_file(statm);
        oc_base_commit(base, p, 1 << 30);
        oc_log_info("memory: committed:       "), print_file(statm);
        OC_ASSERT(p[0] == 0);
        for(usize n = 0; n < (1 << 30); n += 4096)  p[n] = 1;
        oc_log_info("memory: wrote all pages: "), print_file(statm);
        oc_base_decommit(base, p, 1 << 30);
        oc_log_info("memory: decommitted:     "), print_file(statm);
        oc_base_decommit(base, p, 1 << 30);
        oc_base_commit(base, p, 1 << 30);
        OC_ASSERT(p[0] == 0);
        oc_base_release(base, p, 1 << 30);
        oc_log_info("memory: released:        "), print_file(statm);
    }

    // TODO(pld): test platform_path
    {

    }

    // TODO(pld): test platform_thread
    // TODO(pld): test platform_io
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
