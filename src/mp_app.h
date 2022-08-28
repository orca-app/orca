/************************************************************//**
*
*	@file: platform_app.h
*	@author: Martin Fouilleul
*	@date: 16/05/2020
*	@revision:
*
*****************************************************************/
#ifndef __PLATFORM_APP_H_
#define __PLATFORM_APP_H_

#include"typedefs.h"
#include"utf8.h"
#include"lists.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
// Typedefs, enums and constants
//--------------------------------------------------------------------

typedef struct mp_window { u64 h; } mp_window;
typedef struct mp_view { u64 h; } mp_view;

typedef enum { MP_MOUSE_CURSOR_ARROW,
	           MP_MOUSE_CURSOR_RESIZE_0,
               MP_MOUSE_CURSOR_RESIZE_90,
               MP_MOUSE_CURSOR_RESIZE_45,
               MP_MOUSE_CURSOR_RESIZE_135,
               MP_MOUSE_CURSOR_TEXT } mp_mouse_cursor;

typedef i32 mp_window_style;
static const mp_window_style MP_WINDOW_STYLE_NO_TITLE	 = 0x01<<0,
                      MP_WINDOW_STYLE_FIXED_SIZE = 0x01<<1,
                      MP_WINDOW_STYLE_NO_CLOSE	 = 0x01<<2,
                      MP_WINDOW_STYLE_NO_MINIFY	 = 0x01<<3,
                      MP_WINDOW_STYLE_NO_FOCUS	 = 0x01<<4,
                      MP_WINDOW_STYLE_FLOAT		 = 0x01<<5,
                      MP_WINDOW_STYLE_POPUPMENU	 = 0x01<<6,
                      MP_WINDOW_STYLE_NO_BUTTONS = 0x01<<7;

typedef enum { MP_EVENT_NONE,
               MP_EVENT_KEYBOARD_MODS,
               MP_EVENT_KEYBOARD_KEY,
               MP_EVENT_KEYBOARD_CHAR,
               MP_EVENT_MOUSE_BUTTON,
               MP_EVENT_MOUSE_MOVE,
               MP_EVENT_MOUSE_WHEEL,
               MP_EVENT_MOUSE_ENTER,
               MP_EVENT_MOUSE_LEAVE,
               MP_EVENT_WINDOW_RESIZE,
               MP_EVENT_WINDOW_MOVE,
               MP_EVENT_WINDOW_FOCUS,
               MP_EVENT_WINDOW_UNFOCUS,
               MP_EVENT_WINDOW_HIDE,
               MP_EVENT_WINDOW_SHOW,
               MP_EVENT_WINDOW_CLOSE,
               MP_EVENT_CLIPBOARD,
               MP_EVENT_PATHDROP,
               MP_EVENT_FRAME,
               MP_EVENT_QUIT } mp_event_type;

typedef enum { MP_KEY_NO_ACTION,
               MP_KEY_PRESS,
               MP_KEY_RELEASE,
               MP_KEY_REPEAT } mp_key_action;

typedef i32 mp_key_code;

static const mp_key_code MP_KEY_UNKNOWN    = -1,
                  MP_KEY_SPACE      = 32,
                  MP_KEY_APOSTROPHE = 39, // '
                  MP_KEY_COMMA      = 44, // ,
                  MP_KEY_MINUS      = 45, // -
                  MP_KEY_PERIOD     = 46, // .
                  MP_KEY_SLASH      = 47, // /
                  MP_KEY_0          = 48,
                  MP_KEY_1          = 49,
                  MP_KEY_2          = 50,
                  MP_KEY_3          = 51,
                  MP_KEY_4          = 52,
                  MP_KEY_5          = 53,
                  MP_KEY_6          = 54,
                  MP_KEY_7          = 55,
                  MP_KEY_8          = 56,
                  MP_KEY_9          = 57,
                  MP_KEY_SEMICOLON  = 59, // ;
                  MP_KEY_EQUAL      = 61, // =
                  MP_KEY_A          = 65,
                  MP_KEY_B          = 66,
                  MP_KEY_C          = 67,
                  MP_KEY_D          = 68,
                  MP_KEY_E          = 69,
                  MP_KEY_F          = 70,
                  MP_KEY_G          = 71,
                  MP_KEY_H          = 72,
                  MP_KEY_I          = 73,
                  MP_KEY_J          = 74,
                  MP_KEY_K          = 75,
                  MP_KEY_L          = 76,
                  MP_KEY_M          = 77,
                  MP_KEY_N          = 78,
                  MP_KEY_O          = 79,
                  MP_KEY_P          = 80,
                  MP_KEY_Q          = 81,
                  MP_KEY_R          = 82,
                  MP_KEY_S          = 83,
                  MP_KEY_T          = 84,
                  MP_KEY_U          = 85,
                  MP_KEY_V          = 86,
                  MP_KEY_W          = 87,
                  MP_KEY_X          = 88,
                  MP_KEY_Y          = 89,
                  MP_KEY_Z          = 90,
                  MP_KEY_LEFT_BRACKET = 91,	// [
                  MP_KEY_BACKSLASH  = 92,	// \ */
                  MP_KEY_RIGHT_BRACKET = 93,    // ]
                  MP_KEY_GRAVE_ACCENT = 96,	// `
                  MP_KEY_WORLD_1    = 161,	// non-US #1
                  MP_KEY_WORLD_2    = 162,	// non-US #2
                  MP_KEY_ESCAPE     = 256,
                  MP_KEY_ENTER      = 257,
                  MP_KEY_TAB   	    = 258,
                  MP_KEY_BACKSPACE  = 259,
                  MP_KEY_INSERT     = 260,
                  MP_KEY_DELETE     = 261,
                  MP_KEY_RIGHT      = 262,
                  MP_KEY_LEFT       = 263,
                  MP_KEY_DOWN       = 264,
                  MP_KEY_UP         = 265,
                  MP_KEY_PAGE_UP    = 266,
                  MP_KEY_PAGE_DOWN  = 267,
                  MP_KEY_HOME       = 268,
                  MP_KEY_END        = 269,
                  MP_KEY_CAPS_LOCK  = 280,
                  MP_KEY_SCROLL_LOCK = 281,
                  MP_KEY_NUM_LOCK   = 282,
                  MP_KEY_PRINT_SCREEN = 283,
                  MP_KEY_PAUSE      = 284,
                  MP_KEY_F1         = 290,
                  MP_KEY_F2         = 291,
                  MP_KEY_F3         = 292,
                  MP_KEY_F4         = 293,
                  MP_KEY_F5         = 294,
                  MP_KEY_F6         = 295,
                  MP_KEY_F7         = 296,
                  MP_KEY_F8         = 297,
                  MP_KEY_F9         = 298,
                  MP_KEY_F10        = 299,
                  MP_KEY_F11        = 300,
                  MP_KEY_F12        = 301,
                  MP_KEY_F13        = 302,
                  MP_KEY_F14        = 303,
                  MP_KEY_F15        = 304,
                  MP_KEY_F16        = 305,
                  MP_KEY_F17        = 306,
                  MP_KEY_F18        = 307,
                  MP_KEY_F19        = 308,
                  MP_KEY_F20        = 309,
                  MP_KEY_F21        = 310,
                  MP_KEY_F22        = 311,
                  MP_KEY_F23        = 312,
                  MP_KEY_F24        = 313,
                  MP_KEY_F25        = 314,
                  MP_KEY_KP_0       = 320,
                  MP_KEY_KP_1       = 321,
                  MP_KEY_KP_2       = 322,
                  MP_KEY_KP_3       = 323,
                  MP_KEY_KP_4       = 324,
                  MP_KEY_KP_5       = 325,
                  MP_KEY_KP_6       = 326,
                  MP_KEY_KP_7       = 327,
                  MP_KEY_KP_8       = 328,
                  MP_KEY_KP_9       = 329,
                  MP_KEY_KP_DECIMAL = 330,
                  MP_KEY_KP_DIVIDE  = 331,
                  MP_KEY_KP_MULTIPLY = 332,
                  MP_KEY_KP_SUBTRACT = 333,
                  MP_KEY_KP_ADD      = 334,
                  MP_KEY_KP_ENTER    = 335,
                  MP_KEY_KP_EQUAL    = 336,
                  MP_KEY_LEFT_SHIFT  = 340,
                  MP_KEY_LEFT_CONTROL = 341,
                  MP_KEY_LEFT_ALT    = 342,
                  MP_KEY_LEFT_SUPER  = 343,
                  MP_KEY_RIGHT_SHIFT = 344,
                  MP_KEY_RIGHT_CONTROL = 345,
                  MP_KEY_RIGHT_ALT   = 346,
                  MP_KEY_RIGHT_SUPER = 347,
                  MP_KEY_MENU        = 348;

static const mp_key_code MP_KEY_MAX = MP_KEY_MENU;

typedef u8 mp_key_mods;
static const mp_key_mods MP_KEYMOD_NONE  = 0x00,
                  MP_KEYMOD_ALT   = 0x01,
                  MP_KEYMOD_SHIFT = 0x02,
                  MP_KEYMOD_CTRL  = 0x04,
                  MP_KEYMOD_CMD   = 0x08;

typedef i32 mp_mouse_button;
static const mp_mouse_button MP_MOUSE_LEFT	  = 0x00,
                      MP_MOUSE_RIGHT  = 0x01,
                      MP_MOUSE_MIDDLE = 0x02,
                      MP_MOUSE_EXT1   = 0x03,
                      MP_MOUSE_EXT2   = 0x04;

static const u32 MP_KEY_COUNT          = MP_KEY_MAX+1,
          MP_MOUSE_BUTTON_COUNT = 5;

typedef struct mp_key_event		// keyboard and mouse buttons input
{
	mp_key_action action;
	mp_key_code	code;
	mp_key_mods	mods;
	char label[8];
	u8 labelLen;
	int clickCount;
} mp_key_event;

typedef struct mp_char_event		// character input
{
	utf32 codepoint;
	char  sequence[8];
	u8	  seqLen;
} mp_char_event;

typedef struct mp_move_event		// mouse move/scroll
{
	f32 x;
	f32 y;
	f32 deltaX;
	f32 deltaY;
	mp_key_mods mods;
} mp_move_event;

typedef struct mp_frame_event		// window resize / move
{
	mp_rect rect;
} mp_frame_event;

typedef struct mp_event
{
	//TODO clipboard and path drop

	mp_window window;
	mp_event_type type;

	union
	{
		mp_key_event   key;
		mp_char_event  character;
		mp_move_event  move;
		mp_frame_event frame;
		str8 path;
	};

	//TODO(martin): chain externally ?
	list_elt list;
} mp_event;

//--------------------------------------------------------------------
// app management
//--------------------------------------------------------------------

void mp_init();
void mp_terminate();

bool mp_should_quit();
void mp_do_quit();
void mp_request_quit();
void mp_cancel_quit();

void mp_set_cursor(mp_mouse_cursor cursor);

//--------------------------------------------------------------------
// window management
//--------------------------------------------------------------------

//#include"graphics.h"

bool mp_window_handle_is_null(mp_window window);
mp_window mp_window_null_handle();

mp_window mp_window_create(mp_rect contentRect, const char* title, mp_window_style style);
void mp_window_destroy(mp_window window);

bool mp_window_should_close(mp_window window);
void mp_window_request_close(mp_window window);
void mp_window_cancel_close(mp_window window);

void* mp_window_native_pointer(mp_window window);

void mp_window_center(mp_window window);

bool mp_window_is_hidden(mp_window window);
bool mp_window_is_focused(mp_window window);

void mp_window_hide(mp_window window);
void mp_window_focus(mp_window window);
void mp_window_send_to_back(mp_window window);
void mp_window_bring_to_front(mp_window window);

void mp_window_bring_to_front_and_focus(mp_window window);

mp_rect mp_window_content_rect_for_frame_rect(mp_rect frameRect, mp_window_style style);
mp_rect mp_window_frame_rect_for_content_rect(mp_rect contentRect, mp_window_style style);

mp_rect mp_window_get_content_rect(mp_window window);
mp_rect mp_window_get_absolute_content_rect(mp_window window);
mp_rect mp_window_get_frame_rect(mp_window window);

void mp_window_set_content_rect(mp_window window, mp_rect contentRect);
void mp_window_set_frame_rect(mp_window window, mp_rect frameRect);
void mp_window_set_frame_size(mp_window window, int width, int height);
void mp_window_set_content_size(mp_window window, int width, int height);

//--------------------------------------------------------------------
// View management
//--------------------------------------------------------------------

mp_view mp_view_nil();
bool mp_view_is_nil(mp_view view);
mp_view mp_view_create(mp_window window, mp_rect frame);
void mp_view_destroy(mp_view view);
void mp_view_set_frame(mp_view view, mp_rect frame);

/*TODO
mp_view mp_view_bring_to_front(mp_view view);
mp_view mp_view_send_to_back(mp_view view);
*/
//--------------------------------------------------------------------
// Main loop and events handling
//--------------------------------------------------------------------

typedef void(*mp_event_callback)(mp_event event, void* data);
void mp_set_event_callback(mp_event_callback callback, void* data);
void mp_set_target_fps(u32 fps);
void mp_run_loop();
void mp_end_input_frame();

void mp_pump_events(f64 timeout);
bool mp_next_event(mp_event* event);

typedef void(*mp_live_resize_callback)(mp_event event, void* data);
void mp_set_live_resize_callback(mp_live_resize_callback callback, void* data);

//--------------------------------------------------------------------
// Input state polling
//--------------------------------------------------------------------
bool mp_input_key_down(mp_key_code key);
bool mp_input_key_pressed(mp_key_code key);
bool mp_input_key_released(mp_key_code key);
mp_key_mods mp_input_key_mods();

str8 mp_key_to_label(mp_key_code key);
mp_key_code mp_label_to_key(str8 label);

bool mp_input_mouse_down(mp_mouse_button button);
bool mp_input_mouse_pressed(mp_mouse_button button);
bool mp_input_mouse_released(mp_mouse_button button);
bool mp_input_mouse_clicked(mp_mouse_button button);
bool mp_input_mouse_double_clicked(mp_mouse_button button);

vec2 mp_input_mouse_position();
vec2 mp_input_mouse_delta();
vec2 mp_input_mouse_wheel();

str32 mp_input_text_utf32(mem_arena* arena);
str8 mp_input_text_utf8(mem_arena* arena);
//--------------------------------------------------------------------
// app resources
//--------------------------------------------------------------------
str8 mp_app_get_resource_path(mem_arena* arena, const char* name);
str8 mp_app_get_executable_path(mem_arena* arena);

//--------------------------------------------------------------------
// Clipboard
//--------------------------------------------------------------------
void mp_clipboard_clear();

void mp_clipboard_set_string(str8 string);
str8 mp_clipboard_get_string(mem_arena* arena);
str8 mp_clipboard_copy_string(str8 backing);

bool mp_clipboard_has_tag(const char* tag);
void mp_clipboard_set_data_for_tag(const char* tag, str8 data);
str8 mp_clipboard_get_data_for_tag(mem_arena* arena, const char* tag);

//--------------------------------------------------------------------
// native open/save/alert windows
//--------------------------------------------------------------------

str8 mp_open_dialog(mem_arena* arena,
                           const char* title,
                           const char* defaultPath,
                           int filterCount,
                           const char** filters,
                           bool directory);

str8 mp_save_dialog(mem_arena* arena,
                           const char* title,
                           const char* defaultPath,
                           int filterCount,
                           const char** filters);

int mp_alert_popup(const char* title,
                   const char* message,
                   u32 count,
                   const char** options);


//--------------------------------------------------------------------
// file system stuff... //TODO: move elsewhere
//--------------------------------------------------------------------
int mp_file_move(str8 from, str8 to);
int mp_file_remove(str8 path);

int mp_directory_create(str8 path);

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__PLATFORM_APP_H_
