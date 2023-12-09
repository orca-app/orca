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

void oc_init(void)
{
  assert(0 && "Unimplemented");
  return;
}
void oc_terminate(void)
{
  assert(0 && "Unimplemented");
  return;
}
bool oc_should_quit(void)
{
  assert(0 && "Unimplemented");
  return false;
}
void oc_request_quit(void)
{
  assert(0 && "Unimplemented");
  return;
}
void oc_set_cursor(oc_mouse_cursor cursor)
{
  assert(0 && "Unimplemented");
  return;
}
void oc_pump_events(f64 timeout)
{
  assert(0 && "Unimplemented");
  return;
}
oc_window oc_window_create(oc_rect contentRect, oc_str8 title, oc_window_style style)
{
  assert(0 && "Unimplemented");
  return (oc_window){0};
}
void oc_window_destroy(oc_window window)
{
  assert(0 && "Unimplemented");
  return;
}
void* oc_window_native_pointer(oc_window window)
{
  assert(0 && "Unimplemented");
  return NULL;
}

bool oc_window_should_close(oc_window window)
{
  assert(0 && "Unimplemented");
  return false;
}
void oc_window_request_close(oc_window window)
{
  assert(0 && "Unimplemented");
  return;
}
void oc_window_cancel_close(oc_window window)
{
  assert(0 && "Unimplemented");
  return;
}
bool oc_window_is_hidden(oc_window window)
{
  assert(0 && "Unimplemented");
  return false;
}
void oc_window_hide(oc_window window)
{
  assert(0 && "Unimplemented");
  return;
}
void oc_window_show(oc_window window)
{
  assert(0 && "Unimplemented");
  return;
}
void oc_window_set_title(oc_window window, oc_str8 title)
{
  assert(0 && "Unimplemented");
  return;
}
bool oc_window_is_minimized(oc_window window)
{
  assert(0 && "Unimplemented");
  return false;
}
bool oc_window_is_maximized(oc_window window)
{
  assert(0 && "Unimplemented");
  return false;
}
void oc_window_minimize(oc_window window)
{
  assert(0 && "Unimplemented");
  return;
}
void oc_window_maximize(oc_window window)
{
  assert(0 && "Unimplemented");
  return;
}
void oc_window_restore(oc_window window)
{
  assert(0 && "Unimplemented");
  return;
}
bool oc_window_has_focus(oc_window window)
{
  assert(0 && "Unimplemented");
  return false;
}
void oc_window_focus(oc_window window)
{
  assert(0 && "Unimplemented");
  return;
}
void oc_window_unfocus(oc_window window)
{
  assert(0 && "Unimplemented");
  return;
}
void oc_window_send_to_back(oc_window window)
{
  assert(0 && "Unimplemented");
  return;
}
void oc_window_bring_to_front(oc_window window)
{
  assert(0 && "Unimplemented");
  return;
}
oc_rect oc_window_get_frame_rect(oc_window window)
{
  assert(0 && "Unimplemented");
  return (oc_rect){0};
}
void oc_window_set_frame_rect(oc_window window, oc_rect rect)
{
  assert(0 && "Unimplemented");
  return;
}
oc_rect oc_window_get_content_rect(oc_window window)
{
  assert(0 && "Unimplemented");
  return (oc_rect){0};
}
void oc_window_set_content_rect(oc_window window, oc_rect rect)
{
  assert(0 && "Unimplemented");
  return;
}
void oc_window_center(oc_window window)
{
  assert(0 && "Unimplemented");
  return;
}
oc_rect oc_window_content_rect_for_frame_rect(oc_rect frameRect, oc_window_style style)
{
  assert(0 && "Unimplemented");
  return (oc_rect){0};
}
oc_rect oc_window_frame_rect_for_content_rect(oc_rect contentRect, oc_window_style style)
{
  assert(0 && "Unimplemented");
  return (oc_rect){0};
}
i32 oc_dispatch_on_main_thread_sync(oc_window main_window, oc_dispatch_proc proc, void* user)
{
  assert(0 && "Unimplemented");
  return -1;
}
void oc_clipboard_clear(void)
{
  assert(0 && "Unimplemented");
  return;
}
void oc_clipboard_set_string(oc_str8 string)
{
  assert(0 && "Unimplemented");
  return;
}
oc_str8 oc_clipboard_get_string(oc_arena* arena)
{
  assert(0 && "Unimplemented");
  return (oc_str8){0};
}
oc_str8 oc_clipboard_copy_string(oc_str8 backing)
{
  assert(0 && "Unimplemented");
  return (oc_str8){0};
}
bool oc_clipboard_has_tag(const char* tag)
{
  assert(0 && "Unimplemented");
  return false;
}
void oc_clipboard_set_data_for_tag(const char* tag, oc_str8 data)
{
  assert(0 && "Unimplemented");
  return;
}
oc_str8 oc_clipboard_get_data_for_tag(oc_arena* arena, const char* tag)
{
  assert(0 && "Unimplemented");
  return (oc_str8){0};
}
oc_file_dialog_result oc_file_dialog_for_table(oc_arena* arena, oc_file_dialog_desc* desc, oc_file_table* table)
{
  assert(0 && "Unimplemented");
  return (oc_file_dialog_result){0};
}
int oc_alert_popup(oc_str8 title, oc_str8 message, oc_str8_list options)
{
  assert(0 && "Unimplemented");
  return -1;
}
int oc_file_move(oc_str8 from, oc_str8 to)
{
  assert(0 && "Unimplemented");
  return -1;
}
int oc_file_remove(oc_str8 path)
{
  assert(0 && "Unimplemented");
  return -1;
}
int oc_directory_create(oc_str8 path)
{
  assert(0 && "Unimplemented");
  return -1;
}
