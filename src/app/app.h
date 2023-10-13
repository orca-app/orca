/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#ifndef __APP_H_
#define __APP_H_

#include "util/lists.h"
#include "util/memory.h"
#include "util/typedefs.h"
#include "util/utf8.h"
#include "util/macros.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
// Typedefs, enums and constants
//--------------------------------------------------------------------

typedef struct oc_window
{
    u64 h;
} oc_window;

typedef enum
{
    OC_MOUSE_CURSOR_ARROW,
    OC_MOUSE_CURSOR_RESIZE_0,
    OC_MOUSE_CURSOR_RESIZE_90,
    OC_MOUSE_CURSOR_RESIZE_45,
    OC_MOUSE_CURSOR_RESIZE_135,
    OC_MOUSE_CURSOR_TEXT
} oc_mouse_cursor;

typedef u32 oc_window_style;

enum oc_window_style_enum
{
    OC_WINDOW_STYLE_NO_TITLE = 0x01 << 0,
    OC_WINDOW_STYLE_FIXED_SIZE = 0x01 << 1,
    OC_WINDOW_STYLE_NO_CLOSE = 0x01 << 2,
    OC_WINDOW_STYLE_NO_MINIFY = 0x01 << 3,
    OC_WINDOW_STYLE_NO_FOCUS = 0x01 << 4,
    OC_WINDOW_STYLE_FLOAT = 0x01 << 5,
    OC_WINDOW_STYLE_POPUPMENU = 0x01 << 6,
    OC_WINDOW_STYLE_NO_BUTTONS = 0x01 << 7
};

typedef enum
{
    OC_EVENT_NONE,
    OC_EVENT_KEYBOARD_MODS, //TODO: remove, keep only key?
    OC_EVENT_KEYBOARD_KEY,
    OC_EVENT_KEYBOARD_CHAR,
    OC_EVENT_MOUSE_BUTTON,
    OC_EVENT_MOUSE_MOVE,
    OC_EVENT_MOUSE_WHEEL,
    OC_EVENT_MOUSE_ENTER,
    OC_EVENT_MOUSE_LEAVE,
    OC_EVENT_CLIPBOARD_PASTE,
    OC_EVENT_WINDOW_RESIZE,
    OC_EVENT_WINDOW_MOVE,
    OC_EVENT_WINDOW_FOCUS,
    OC_EVENT_WINDOW_UNFOCUS,
    OC_EVENT_WINDOW_HIDE, // rename to minimize?
    OC_EVENT_WINDOW_SHOW, // rename to restore?
    OC_EVENT_WINDOW_CLOSE,
    OC_EVENT_PATHDROP,
    OC_EVENT_FRAME,
    OC_EVENT_QUIT
} oc_event_type;

typedef enum
{
    OC_KEY_NO_ACTION,
    OC_KEY_PRESS,
    OC_KEY_RELEASE,
    OC_KEY_REPEAT
} oc_key_action;

#define OC_KEY_TABLE(X)                                         \
    X(OC_SCANCODE_UNKNOWN, 0, OC_KEY_UNKNOWN, '\0')             \
    X(OC_SCANCODE_SPACE, 32, OC_KEY_SPACE, ' ')                 \
    X(OC_SCANCODE_APOSTROPHE, 39, OC_KEY_APOSTROPHE, '\'')      \
    X(OC_SCANCODE_COMMA, 44, OC_KEY_COMMA, ',')                 \
    X(OC_SCANCODE_MINUS, 45, OC_KEY_MINUS, '-')                 \
    X(OC_SCANCODE_PERIOD, 46, OC_KEY_PERIOD, '.')               \
    X(OC_SCANCODE_SLASH, 47, OC_KEY_SLASH, '/')                 \
    X(OC_SCANCODE_0, 48, OC_KEY_0, '0')                         \
    X(OC_SCANCODE_1, 49, OC_KEY_1, '1')                         \
    X(OC_SCANCODE_2, 50, OC_KEY_2, '2')                         \
    X(OC_SCANCODE_3, 51, OC_KEY_3, '3')                         \
    X(OC_SCANCODE_4, 52, OC_KEY_4, '4')                         \
    X(OC_SCANCODE_5, 53, OC_KEY_5, '5')                         \
    X(OC_SCANCODE_6, 54, OC_KEY_6, '6')                         \
    X(OC_SCANCODE_7, 55, OC_KEY_7, '7')                         \
    X(OC_SCANCODE_8, 56, OC_KEY_8, '8')                         \
    X(OC_SCANCODE_9, 57, OC_KEY_9, '9')                         \
    X(OC_SCANCODE_SEMICOLON, 59, OC_KEY_SEMICOLON, ';')         \
    X(OC_SCANCODE_EQUAL, 61, OC_KEY_EQUAL, '=')                 \
    X(OC_SCANCODE_LEFT_BRACKET, 91, OC_KEY_LEFT_BRACKET, '[')   \
    X(OC_SCANCODE_BACKSLASH, 92, OC_KEY_BACKSLASH, '\\')        \
    X(OC_SCANCODE_RIGHT_BRACKET, 93, OC_KEY_RIGHT_BRACKET, ']') \
    X(OC_SCANCODE_GRAVE_ACCENT, 96, OC_KEY_GRAVE_ACCENT, '`')   \
    X(OC_SCANCODE_A, 97, OC_KEY_A, 'a')                         \
    X(OC_SCANCODE_B, 98, OC_KEY_B, 'b')                         \
    X(OC_SCANCODE_C, 99, OC_KEY_C, 'c')                         \
    X(OC_SCANCODE_D, 100, OC_KEY_D, 'd')                        \
    X(OC_SCANCODE_E, 101, OC_KEY_E, 'e')                        \
    X(OC_SCANCODE_F, 102, OC_KEY_F, 'f')                        \
    X(OC_SCANCODE_G, 103, OC_KEY_G, 'g')                        \
    X(OC_SCANCODE_H, 104, OC_KEY_H, 'h')                        \
    X(OC_SCANCODE_I, 105, OC_KEY_I, 'i')                        \
    X(OC_SCANCODE_J, 106, OC_KEY_J, 'j')                        \
    X(OC_SCANCODE_K, 107, OC_KEY_K, 'k')                        \
    X(OC_SCANCODE_L, 108, OC_KEY_L, 'l')                        \
    X(OC_SCANCODE_M, 109, OC_KEY_M, 'm')                        \
    X(OC_SCANCODE_N, 110, OC_KEY_N, 'n')                        \
    X(OC_SCANCODE_O, 111, OC_KEY_O, 'o')                        \
    X(OC_SCANCODE_P, 112, OC_KEY_P, 'p')                        \
    X(OC_SCANCODE_Q, 113, OC_KEY_Q, 'q')                        \
    X(OC_SCANCODE_R, 114, OC_KEY_R, 'r')                        \
    X(OC_SCANCODE_S, 115, OC_KEY_S, 's')                        \
    X(OC_SCANCODE_T, 116, OC_KEY_T, 't')                        \
    X(OC_SCANCODE_U, 117, OC_KEY_U, 'u')                        \
    X(OC_SCANCODE_V, 118, OC_KEY_V, 'v')                        \
    X(OC_SCANCODE_W, 119, OC_KEY_W, 'w')                        \
    X(OC_SCANCODE_X, 120, OC_KEY_X, 'x')                        \
    X(OC_SCANCODE_Y, 121, OC_KEY_Y, 'y')                        \
    X(OC_SCANCODE_Z, 122, OC_KEY_Z, 'z')                        \
    X(OC_SCANCODE_WORLD_1, 161, OC_KEY_WORLD_1)                 \
    X(OC_SCANCODE_WORLD_2, 162, OC_KEY_WORLD_2)                 \
    X(OC_SCANCODE_ESCAPE, 256, OC_KEY_ESCAPE)                   \
    X(OC_SCANCODE_ENTER, 257, OC_KEY_ENTER)                     \
    X(OC_SCANCODE_TAB, 258, OC_KEY_TAB)                         \
    X(OC_SCANCODE_BACKSPACE, 259, OC_KEY_BACKSPACE)             \
    X(OC_SCANCODE_INSERT, 260, OC_KEY_INSERT)                   \
    X(OC_SCANCODE_DELETE, 261, OC_KEY_DELETE)                   \
    X(OC_SCANCODE_RIGHT, 262, OC_KEY_RIGHT)                     \
    X(OC_SCANCODE_LEFT, 263, OC_KEY_LEFT)                       \
    X(OC_SCANCODE_DOWN, 264, OC_KEY_DOWN)                       \
    X(OC_SCANCODE_UP, 265, OC_KEY_UP)                           \
    X(OC_SCANCODE_PAGE_UP, 266, OC_KEY_PAGE_UP)                 \
    X(OC_SCANCODE_PAGE_DOWN, 267, OC_KEY_PAGE_DOWN)             \
    X(OC_SCANCODE_HOME, 268, OC_KEY_HOME)                       \
    X(OC_SCANCODE_END, 269, OC_KEY_END)                         \
    X(OC_SCANCODE_CAPS_LOCK, 280, OC_KEY_CAPS_LOCK)             \
    X(OC_SCANCODE_SCROLL_LOCK, 281, OC_KEY_SCROLL_LOCK)         \
    X(OC_SCANCODE_NUM_LOCK, 282, OC_KEY_NUM_LOCK)               \
    X(OC_SCANCODE_PRINT_SCREEN, 283, OC_KEY_PRINT_SCREEN)       \
    X(OC_SCANCODE_PAUSE, 284, OC_KEY_PAUSE)                     \
    X(OC_SCANCODE_F1, 290, OC_KEY_F1)                           \
    X(OC_SCANCODE_F2, 291, OC_KEY_F2)                           \
    X(OC_SCANCODE_F3, 292, OC_KEY_F3)                           \
    X(OC_SCANCODE_F4, 293, OC_KEY_F4)                           \
    X(OC_SCANCODE_F5, 294, OC_KEY_F5)                           \
    X(OC_SCANCODE_F6, 295, OC_KEY_F6)                           \
    X(OC_SCANCODE_F7, 296, OC_KEY_F7)                           \
    X(OC_SCANCODE_F8, 297, OC_KEY_F8)                           \
    X(OC_SCANCODE_F9, 298, OC_KEY_F9)                           \
    X(OC_SCANCODE_F10, 299, OC_KEY_F10)                         \
    X(OC_SCANCODE_F11, 300, OC_KEY_F11)                         \
    X(OC_SCANCODE_F12, 301, OC_KEY_F12)                         \
    X(OC_SCANCODE_F13, 302, OC_KEY_F13)                         \
    X(OC_SCANCODE_F14, 303, OC_KEY_F14)                         \
    X(OC_SCANCODE_F15, 304, OC_KEY_F15)                         \
    X(OC_SCANCODE_F16, 305, OC_KEY_F16)                         \
    X(OC_SCANCODE_F17, 306, OC_KEY_F17)                         \
    X(OC_SCANCODE_F18, 307, OC_KEY_F18)                         \
    X(OC_SCANCODE_F19, 308, OC_KEY_F19)                         \
    X(OC_SCANCODE_F20, 309, OC_KEY_F20)                         \
    X(OC_SCANCODE_F21, 310, OC_KEY_F21)                         \
    X(OC_SCANCODE_F22, 311, OC_KEY_F22)                         \
    X(OC_SCANCODE_F23, 312, OC_KEY_F23)                         \
    X(OC_SCANCODE_F24, 313, OC_KEY_F24)                         \
    X(OC_SCANCODE_F25, 314, OC_KEY_F25)                         \
    X(OC_SCANCODE_KP_0, 320, OC_KEY_KP_0)                       \
    X(OC_SCANCODE_KP_1, 321, OC_KEY_KP_1)                       \
    X(OC_SCANCODE_KP_2, 322, OC_KEY_KP_2)                       \
    X(OC_SCANCODE_KP_3, 323, OC_KEY_KP_3)                       \
    X(OC_SCANCODE_KP_4, 324, OC_KEY_KP_4)                       \
    X(OC_SCANCODE_KP_5, 325, OC_KEY_KP_5)                       \
    X(OC_SCANCODE_KP_6, 326, OC_KEY_KP_6)                       \
    X(OC_SCANCODE_KP_7, 327, OC_KEY_KP_7)                       \
    X(OC_SCANCODE_KP_8, 328, OC_KEY_KP_8)                       \
    X(OC_SCANCODE_KP_9, 329, OC_KEY_KP_9)                       \
    X(OC_SCANCODE_KP_DECIMAL, 330, OC_KEY_KP_DECIMAL)           \
    X(OC_SCANCODE_KP_DIVIDE, 331, OC_KEY_KP_DIVIDE)             \
    X(OC_SCANCODE_KP_MULTIPLY, 332, OC_KEY_KP_MULTIPLY)         \
    X(OC_SCANCODE_KP_SUBTRACT, 333, OC_KEY_KP_SUBTRACT)         \
    X(OC_SCANCODE_KP_ADD, 334, OC_KEY_KP_ADD)                   \
    X(OC_SCANCODE_KP_ENTER, 335, OC_KEY_KP_ENTER)               \
    X(OC_SCANCODE_KP_EQUAL, 336, OC_KEY_KP_EQUAL)               \
    X(OC_SCANCODE_LEFT_SHIFT, 340, OC_KEY_LEFT_SHIFT)           \
    X(OC_SCANCODE_LEFT_CONTROL, 341, OC_KEY_LEFT_CONTROL)       \
    X(OC_SCANCODE_LEFT_ALT, 342, OC_KEY_LEFT_ALT)               \
    X(OC_SCANCODE_LEFT_SUPER, 343, OC_KEY_LEFT_SUPER)           \
    X(OC_SCANCODE_RIGHT_SHIFT, 344, OC_KEY_RIGHT_SHIFT)         \
    X(OC_SCANCODE_RIGHT_CONTROL, 345, OC_KEY_RIGHT_CONTROL)     \
    X(OC_SCANCODE_RIGHT_ALT, 346, OC_KEY_RIGHT_ALT)             \
    X(OC_SCANCODE_RIGHT_SUPER, 347, OC_KEY_RIGHT_SUPER)         \
    X(OC_SCANCODE_MENU, 348, OC_KEY_MENU)

#define OC_SCANCODE_ENUM(sc, sv, ...) sc = sv,
#define OC_KEYCODE_ENUM(sc, sv, kc, ...) kc = OC_VA_NOPT(sv, ##__VA_ARGS__) __VA_ARGS__,

typedef enum
{
    OC_KEY_TABLE(OC_SCANCODE_ENUM)
        OC_SCANCODE_COUNT
} oc_scan_code;

typedef enum
{
    OC_KEY_TABLE(OC_KEYCODE_ENUM)
        OC_KEY_COUNT
} oc_key_code;

#undef OC_SCANCODE_ENUM
#undef OC_KEYCODE_ENUM

typedef enum
{
    OC_KEYMOD_NONE = 0x00,
    OC_KEYMOD_ALT = 0x01,
    OC_KEYMOD_SHIFT = 0x02,
    OC_KEYMOD_CTRL = 0x04,
    OC_KEYMOD_CMD = 0x08,
    OC_KEYMOD_MAIN_MODIFIER = 0x10 /* CMD on Mac, CTRL on Win32 */
} oc_keymod_flags;

typedef enum
{
    OC_MOUSE_LEFT = 0x00,
    OC_MOUSE_RIGHT = 0x01,
    OC_MOUSE_MIDDLE = 0x02,
    OC_MOUSE_EXT1 = 0x03,
    OC_MOUSE_EXT2 = 0x04,
    OC_MOUSE_BUTTON_COUNT
} oc_mouse_button;

typedef struct oc_key_event // keyboard and mouse buttons input
{
    oc_key_action action;
    oc_scan_code scanCode;
    oc_key_code keyCode;
    oc_mouse_button button;
    oc_keymod_flags mods;
    u8 clickCount;
} oc_key_event;

typedef struct oc_char_event // character input
{
    oc_utf32 codepoint;
    char sequence[8];
    u8 seqLen;
} oc_char_event;

typedef struct oc_mouse_event // mouse move/scroll
{
    f32 x;
    f32 y;
    f32 deltaX;
    f32 deltaY;
    oc_keymod_flags mods;
} oc_mouse_event;

typedef struct oc_move_event // window resize / move
{
    oc_rect frame;
    oc_rect content;
} oc_move_event;

typedef struct oc_event
{
    //TODO clipboard and path drop
    oc_window window;
    oc_event_type type;

    union
    {
        oc_key_event key;
        oc_char_event character;
        oc_mouse_event mouse;
        oc_move_event move;
        oc_str8_list paths;
    };

} oc_event;

//NOTE: these APIs are not directly available to Orca apps
#if !defined(OC_PLATFORM_ORCA) || !(OC_PLATFORM_ORCA)
//--------------------------------------------------------------------
// app management
//--------------------------------------------------------------------

ORCA_API void oc_init(void);
ORCA_API void oc_terminate(void);

ORCA_API bool oc_should_quit(void);
ORCA_API void oc_request_quit(void);

ORCA_API void oc_set_cursor(oc_mouse_cursor cursor);

//--------------------------------------------------------------------
// Main loop and events handling
//--------------------------------------------------------------------
/*NOTE:
	oc_pump_events() pumps system events into the event queue. A timeout of 0 polls for events,
	while a timeout of -1 blocks for events. A timeout > 0 blocks until new events are available
	or the timeout elapses.

	oc_next_event() get the next event from the event queue, allocating from the passed arena
*/
ORCA_API void oc_pump_events(f64 timeout);
ORCA_API oc_event* oc_next_event(oc_arena* arena);

ORCA_API oc_key_code oc_scancode_to_keycode(oc_scan_code scanCode);

//--------------------------------------------------------------------
// window management
//--------------------------------------------------------------------

ORCA_API bool oc_window_handle_is_null(oc_window window);
ORCA_API oc_window oc_window_null_handle(void);

ORCA_API oc_window oc_window_create(oc_rect contentRect, oc_str8 title, oc_window_style style);
ORCA_API void oc_window_destroy(oc_window window);
ORCA_API void* oc_window_native_pointer(oc_window window);

ORCA_API bool oc_window_should_close(oc_window window);
ORCA_API void oc_window_request_close(oc_window window);
ORCA_API void oc_window_cancel_close(oc_window window);

ORCA_API bool oc_window_is_hidden(oc_window window);
ORCA_API void oc_window_hide(oc_window window);
ORCA_API void oc_window_show(oc_window window);

ORCA_API void oc_window_set_title(oc_window window, oc_str8 title);

ORCA_API bool oc_window_is_minimized(oc_window window);
ORCA_API bool oc_window_is_maximized(oc_window window);
ORCA_API void oc_window_minimize(oc_window window);
ORCA_API void oc_window_maximize(oc_window window);
ORCA_API void oc_window_restore(oc_window window);

ORCA_API bool oc_window_has_focus(oc_window window);
ORCA_API void oc_window_focus(oc_window window);
ORCA_API void oc_window_unfocus(oc_window window);

ORCA_API void oc_window_send_to_back(oc_window window);
ORCA_API void oc_window_bring_to_front(oc_window window);

ORCA_API oc_rect oc_window_get_frame_rect(oc_window window);
ORCA_API void oc_window_set_frame_rect(oc_window window, oc_rect rect);
ORCA_API void oc_window_set_frame_position(oc_window window, oc_vec2 position);
ORCA_API void oc_window_set_frame_size(oc_window window, oc_vec2 size);

ORCA_API oc_rect oc_window_get_content_rect(oc_window window);
ORCA_API void oc_window_set_content_rect(oc_window window, oc_rect rect);
ORCA_API void oc_window_set_content_position(oc_window window, oc_vec2 position);
ORCA_API void oc_window_set_content_size(oc_window window, oc_vec2 size);

ORCA_API void oc_window_center(oc_window window);

ORCA_API oc_rect oc_window_content_rect_for_frame_rect(oc_rect frameRect, oc_window_style style);
ORCA_API oc_rect oc_window_frame_rect_for_content_rect(oc_rect contentRect, oc_window_style style);

//---------------------------------------------------------------
// Dispatching stuff to the main thread
//---------------------------------------------------------------

typedef i32 (*oc_dispatch_proc)(void* user);

ORCA_API i32 oc_dispatch_on_main_thread_sync(oc_window main_window, oc_dispatch_proc proc, void* user);

//--------------------------------------------------------------------
// Clipboard
//--------------------------------------------------------------------
ORCA_API void oc_clipboard_clear(void);

ORCA_API void oc_clipboard_set_string(oc_str8 string);
ORCA_API oc_str8 oc_clipboard_get_string(oc_arena* arena);
ORCA_API oc_str8 oc_clipboard_copy_string(oc_str8 backing);

ORCA_API bool oc_clipboard_has_tag(const char* tag);
ORCA_API void oc_clipboard_set_data_for_tag(const char* tag, oc_str8 data);
ORCA_API oc_str8 oc_clipboard_get_data_for_tag(oc_arena* arena, const char* tag);

#endif // !defined(OC_PLATFORM_ORCA) || !(OC_PLATFORM_ORCA)

//--------------------------------------------------------------------
// native open/save/alert windows
//--------------------------------------------------------------------

#include "platform/platform_io.h"

typedef enum
{
    OC_FILE_DIALOG_SAVE,
    OC_FILE_DIALOG_OPEN,
} oc_file_dialog_kind;

typedef u32 oc_file_dialog_flags;

enum _oc_file_dialog_flags
{
    OC_FILE_DIALOG_FILES = 1,
    OC_FILE_DIALOG_DIRECTORIES = 1 << 1,
    OC_FILE_DIALOG_MULTIPLE = 1 << 2,
    OC_FILE_DIALOG_CREATE_DIRECTORIES = 1 << 3,
};

typedef struct oc_file_dialog_desc
{
    oc_file_dialog_kind kind;
    oc_file_dialog_flags flags;
    oc_str8 title;
    oc_str8 okLabel;
    oc_file startAt;
    oc_str8 startPath;
    oc_str8_list filters;

    //... later customization options with checkboxes / radiobuttons
} oc_file_dialog_desc;

typedef enum
{
    OC_FILE_DIALOG_CANCEL = 0,
    OC_FILE_DIALOG_OK,
} oc_file_dialog_button;

typedef struct oc_file_dialog_result
{
    oc_file_dialog_button button;
    oc_str8 path;
    oc_str8_list selection;

} oc_file_dialog_result;

#if !defined(OC_PLATFORM_ORCA) || !(OC_PLATFORM_ORCA)

ORCA_API oc_file_dialog_result oc_file_dialog(oc_arena* arena, oc_file_dialog_desc* desc);

typedef struct oc_file_table oc_file_table;
ORCA_API oc_file_dialog_result oc_file_dialog_for_table(oc_arena* arena, oc_file_dialog_desc* desc, oc_file_table* table);

ORCA_API int oc_alert_popup(oc_str8 title,
                            oc_str8 message,
                            oc_str8_list options);

//--------------------------------------------------------------------
// file system stuff... //TODO: move elsewhere
//--------------------------------------------------------------------
ORCA_API int oc_file_move(oc_str8 from, oc_str8 to);
ORCA_API int oc_file_remove(oc_str8 path);

ORCA_API int oc_directory_create(oc_str8 path);

#else

void oc_window_set_title(oc_str8 title);
void oc_window_set_size(oc_vec2 size);

void ORCA_IMPORT(oc_request_quit)(void);
oc_key_code ORCA_IMPORT(oc_scancode_to_keycode)(oc_scan_code scanCode);

void oc_clipboard_set_string(oc_str8 string);

#endif // !defined(OC_PLATFORM_ORCA) || !(OC_PLATFORM_ORCA)

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__APP_H_
