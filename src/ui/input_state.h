/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "app/app.h"
#include "platform/platform.h"
#include "util/strings.h"
#include "util/typedefs.h"
#include "util/utf8.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct oc_key_state
{
    u64 lastUpdate;
    u32 transitionCount;
    u32 repeatCount;
    bool down;
    bool sysClicked;
    bool sysDoubleClicked;
    bool sysTripleClicked;

} oc_key_state;

typedef struct oc_keyboard_state
{
    oc_key_state keys[OC_KEY_COUNT];
    oc_keymod_flags mods;
} oc_keyboard_state;

typedef struct oc_mouse_state
{
    u64 lastUpdate;
    bool posValid;
    oc_vec2 pos;
    oc_vec2 delta;
    oc_vec2 wheel;

    union
    {
        oc_key_state buttons[OC_MOUSE_BUTTON_COUNT];

        struct
        {
            oc_key_state left;
            oc_key_state right;
            oc_key_state middle;
            oc_key_state ext1;
            oc_key_state ext2;
        };
    };
} oc_mouse_state;

enum oc_input_text_backing_size
{
    OC_INPUT_TEXT_BACKING_SIZE = 64
};

typedef struct oc_text_state
{
    u64 lastUpdate;
    oc_utf32 backing[OC_INPUT_TEXT_BACKING_SIZE];
    oc_str32 codePoints;
} oc_text_state;

typedef struct oc_clipboard_state
{
    u64 lastUpdate;
    oc_str8 pastedText;
} oc_clipboard_state;

typedef struct oc_input_state
{
    u64 frameCounter;
    oc_keyboard_state keyboard;
    oc_mouse_state mouse;
    oc_text_state text;
    oc_clipboard_state clipboard;
} oc_input_state;

ORCA_API void oc_input_process_event(oc_arena* arena, oc_input_state* state, oc_event* event);
ORCA_API void oc_input_next_frame(oc_input_state* state);

ORCA_API bool oc_key_down(oc_input_state* state, oc_key_code key);
ORCA_API u8 oc_key_press_count(oc_input_state* state, oc_key_code key);
ORCA_API u8 oc_key_release_count(oc_input_state* state, oc_key_code key);
ORCA_API u8 oc_key_repeat_count(oc_input_state* state, oc_key_code key);

ORCA_API bool oc_key_down_scancode(oc_input_state* state, oc_scan_code key);
ORCA_API u8 oc_key_press_count_scancode(oc_input_state* state, oc_scan_code key);
ORCA_API u8 oc_key_release_count_scancode(oc_input_state* state, oc_scan_code key);
ORCA_API u8 oc_key_repeat_count_scancode(oc_input_state* state, oc_scan_code key);

ORCA_API bool oc_mouse_down(oc_input_state* state, oc_mouse_button button);
ORCA_API u8 oc_mouse_pressed(oc_input_state* state, oc_mouse_button button);
ORCA_API u8 oc_mouse_released(oc_input_state* state, oc_mouse_button button);
ORCA_API bool oc_mouse_clicked(oc_input_state* state, oc_mouse_button button);
ORCA_API bool oc_mouse_double_clicked(oc_input_state* state, oc_mouse_button button);

ORCA_API oc_vec2 oc_mouse_position(oc_input_state* state);
ORCA_API oc_vec2 oc_mouse_delta(oc_input_state* state);
ORCA_API oc_vec2 oc_mouse_wheel(oc_input_state* state);

ORCA_API oc_str32 oc_input_text_utf32(oc_arena* arena, oc_input_state* state);
ORCA_API oc_str8 oc_input_text_utf8(oc_arena* arena, oc_input_state* state);

ORCA_API bool oc_clipboard_pasted(oc_input_state* state);
ORCA_API oc_str8 oc_clipboard_pasted_text(oc_input_state* state);

ORCA_API oc_keymod_flags oc_key_mods(oc_input_state* state);

#ifdef __cplusplus
}
#endif
