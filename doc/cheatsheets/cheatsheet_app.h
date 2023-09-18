/************************************************************/ /**
*
*	@file: cheatsheet_app.h
*	@author: Martin Fouilleul
*	@date: 05/09/2023
*
*****************************************************************/

//----------------------------------------------------------------
// Handlers (can be defined by your app to respond to events)
//----------------------------------------------------------------
void oc_on_init(void);
void oc_on_mouse_down(oc_mouse_button button);
void oc_on_mouse_up(oc_mouse_button button);
void oc_on_mouse_enter(void);
void oc_on_mouse_leave(void);
void oc_on_mouse_move(f32 x, f32 y, f32 deltaX, f32 deltaY);
void oc_on_mouse_wheel(f32 deltaX, f32 deltaY);
void oc_on_key_down(oc_scan_code scan, oc_key_code key);
void oc_on_key_up(oc_scan_code scan, oc_key_code key);
void oc_on_frame_refresh(void);
void oc_on_resize(f32 width, f32 height);
void oc_on_raw_event(oc_event* event);
void oc_on_terminate(void);

//----------------------------------------------------------------
// Window
//----------------------------------------------------------------
void oc_window_set_title(oc_str8 title);
void oc_window_set_size(oc_vec2 size);

//----------------------------------------------------------------
// Quitting
//----------------------------------------------------------------
void oc_request_quit(void)
