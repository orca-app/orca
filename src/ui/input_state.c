/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "input_state.h"

//---------------------------------------------------------------
// Input state updating
//---------------------------------------------------------------

static void oc_update_key_state(oc_input_state* state, oc_key_state* key, oc_key_action action)
{
    u64 frameCounter = state->frameCounter;
    if(key->lastUpdate != frameCounter)
    {
        key->transitionCount = 0;
        key->repeatCount = 0;
        key->sysClicked = false;
        key->sysDoubleClicked = false;
        key->sysTripleClicked = false;
        key->lastUpdate = frameCounter;
    }

    switch(action)
    {
        case OC_KEY_PRESS:
        {
            if(!key->down)
            {
                key->transitionCount++;
            }
            key->down = true;
        }
        break;

        case OC_KEY_REPEAT:
        {
            key->repeatCount++;
            key->down = true;
        }
        break;

        case OC_KEY_RELEASE:
        {
            if(key->down)
            {
                key->transitionCount++;
            }
            key->down = false;
        }
        break;

        default:
            break;
    }
}

static void oc_update_key_mods(oc_input_state* state, oc_keymod_flags mods)
{
    state->keyboard.mods = mods;
}

static void oc_update_clipboard_paste(oc_arena* arena, oc_input_state* state)
{
    state->clipboard.lastUpdate = state->frameCounter;
    state->clipboard.pastedText = oc_clipboard_get_string(arena);
}

static void oc_update_mouse_move(oc_input_state* state, f32 x, f32 y, f32 deltaX, f32 deltaY)
{
    u64 frameCounter = state->frameCounter;
    oc_mouse_state* mouse = &state->mouse;
    if(mouse->lastUpdate != frameCounter)
    {
        mouse->delta = (oc_vec2){ 0, 0 };
        mouse->wheel = (oc_vec2){ 0, 0 };
        mouse->lastUpdate = frameCounter;
    }
    mouse->posValid = true;
    mouse->pos = (oc_vec2){ x, y };
    mouse->delta.x += deltaX;
    mouse->delta.y += deltaY;
}

static void oc_update_mouse_leave(oc_input_state* state)
{
    u64 frameCounter = state->frameCounter;
    oc_mouse_state* mouse = &state->mouse;
    if(mouse->lastUpdate != frameCounter)
    {
        mouse->delta = (oc_vec2){ 0, 0 };
        mouse->wheel = (oc_vec2){ 0, 0 };
        mouse->lastUpdate = frameCounter;
    }
    mouse->posValid = false;
}

static void oc_update_mouse_wheel(oc_input_state* state, f32 deltaX, f32 deltaY)
{
    u64 frameCounter = state->frameCounter;
    oc_mouse_state* mouse = &state->mouse;
    if(mouse->lastUpdate != frameCounter)
    {
        mouse->delta = (oc_vec2){ 0, 0 };
        mouse->wheel = (oc_vec2){ 0, 0 };
        mouse->lastUpdate = frameCounter;
    }
    mouse->wheel.x += deltaX;
    mouse->wheel.y += deltaY;
}

static void oc_update_text(oc_input_state* state, oc_utf32 codepoint)
{
    u64 frameCounter = state->frameCounter;
    oc_text_state* text = &state->text;

    if(text->lastUpdate != frameCounter)
    {
        text->codePoints.len = 0;
        text->lastUpdate = frameCounter;
    }

    text->codePoints.ptr = text->backing;
    if(text->codePoints.len < OC_INPUT_TEXT_BACKING_SIZE)
    {
        text->codePoints.ptr[text->codePoints.len] = codepoint;
        text->codePoints.len++;
    }
    else
    {
        oc_log_warning("too many input codepoints per frame, dropping input");
    }
}

void oc_input_next_frame(oc_input_state* state)
{
    state->frameCounter++;
}

void oc_input_process_event(oc_arena* arena, oc_input_state* state, oc_event* event)
{
    switch(event->type)
    {
        case OC_EVENT_KEYBOARD_KEY:
        {
            oc_key_state* key = &state->keyboard.keys[event->key.keyCode];
            oc_update_key_state(state, key, event->key.action);
            oc_update_key_mods(state, event->key.mods);
        }
        break;

        case OC_EVENT_KEYBOARD_CHAR:
            oc_update_text(state, event->character.codepoint);
            break;

        case OC_EVENT_KEYBOARD_MODS:
            oc_update_key_mods(state, event->key.mods);
            break;

        case OC_EVENT_CLIPBOARD_PASTE:
            oc_update_clipboard_paste(arena, state);
            break;

        case OC_EVENT_MOUSE_MOVE:
            oc_update_mouse_move(state, event->mouse.x, event->mouse.y, event->mouse.deltaX, event->mouse.deltaY);
            break;

        case OC_EVENT_MOUSE_LEAVE:
            oc_update_mouse_leave(state);
            break;

        case OC_EVENT_MOUSE_WHEEL:
            oc_update_mouse_wheel(state, event->mouse.deltaX, event->mouse.deltaY);
            break;

        case OC_EVENT_MOUSE_BUTTON:
        {
            oc_key_state* key = &state->mouse.buttons[event->key.button];
            oc_update_key_state(state, key, event->key.action);

            if(event->key.action == OC_KEY_PRESS)
            {
                if(event->key.clickCount >= 1)
                {
                    key->sysClicked = true;
                }

                if(event->key.clickCount % 2 == 0)
                {
                    key->sysDoubleClicked = true;
                }
                else if(event->key.clickCount > 1)
                {
                    key->sysTripleClicked = true;
                }
            }

            oc_update_key_mods(state, event->key.mods);
        }
        break;

        default:
            break;
    }
}

//--------------------------------------------------------------------
// Input state polling
//--------------------------------------------------------------------

oc_key_state oc_key_get_state(oc_input_state* input, oc_key_code key)
{
    oc_key_state state = { 0 };
    if(key <= OC_KEY_COUNT)
    {
        state = input->keyboard.keys[key];
    }
    return (state);
}

oc_key_state oc_mouse_button_get_state(oc_input_state* input, oc_mouse_button button)
{
    oc_key_state state = { 0 };
    if(button <= OC_MOUSE_BUTTON_COUNT)
    {
        state = input->mouse.buttons[button];
    }
    return (state);
}

u8 oc_key_state_press_count(oc_input_state* input, oc_key_state* key)
{
    u8 count = 0;
    if(key->lastUpdate == input->frameCounter)
    {
        count = key->transitionCount / 2;
        if(key->down)
        {
            //NOTE: add one if state is down transition count is odd
            count += (key->transitionCount & 0x01);
        }
    }
    return (count);
}

u8 oc_key_state_release_count(oc_input_state* input, oc_key_state* key)
{
    u8 count = 0;
    if(key->lastUpdate == input->frameCounter)
    {
        count = key->transitionCount / 2;
        if(!key->down)
        {
            //NOTE: add one if state is up and transition count is odd
            count += (key->transitionCount & 0x01);
        }
    }
    return (count);
}

u8 oc_key_state_repeat_count(oc_input_state* input, oc_key_state* key)
{
    u8 count = 0;
    if(key->lastUpdate == input->frameCounter)
    {
        count = key->repeatCount;
    }
    return (count);
}

bool oc_key_down(oc_input_state* input, oc_key_code key)
{
    oc_key_state state = oc_key_get_state(input, key);
    return (state.down);
}

u8 oc_key_press_count(oc_input_state* input, oc_key_code key)
{
    oc_key_state state = oc_key_get_state(input, key);
    return (oc_key_state_press_count(input, &state));
}

u8 oc_key_release_count(oc_input_state* input, oc_key_code key)
{
    oc_key_state state = oc_key_get_state(input, key);
    return (oc_key_state_release_count(input, &state));
}

u8 oc_key_repeat_count(oc_input_state* input, oc_key_code key)
{
    oc_key_state state = oc_key_get_state(input, key);
    return (oc_key_state_repeat_count(input, &state));
}

bool oc_key_down_scancode(oc_input_state* state, oc_scan_code scanCode)
{
    oc_key_code key = oc_scancode_to_keycode(scanCode);
    return (oc_key_down(state, key));
}

u8 oc_key_press_count_scancode(oc_input_state* state, oc_scan_code scanCode)
{
    oc_key_code key = oc_scancode_to_keycode(scanCode);
    return (oc_key_press_count(state, key));
}

u8 oc_key_release_count_scancode(oc_input_state* state, oc_scan_code scanCode)
{
    oc_key_code key = oc_scancode_to_keycode(scanCode);
    return (oc_key_release_count(state, key));
}

u8 oc_key_repeat_count_scancode(oc_input_state* state, oc_scan_code scanCode)
{
    oc_key_code key = oc_scancode_to_keycode(scanCode);
    return (oc_key_repeat_count(state, key));
}

bool oc_mouse_down(oc_input_state* input, oc_mouse_button button)
{
    oc_key_state state = oc_mouse_button_get_state(input, button);
    return (state.down);
}

u8 oc_mouse_pressed(oc_input_state* input, oc_mouse_button button)
{
    oc_key_state state = oc_mouse_button_get_state(input, button);
    return (oc_key_state_press_count(input, &state));
}

u8 oc_mouse_released(oc_input_state* input, oc_mouse_button button)
{
    oc_key_state state = oc_mouse_button_get_state(input, button);
    return (oc_key_state_release_count(input, &state));
}

bool oc_mouse_clicked(oc_input_state* input, oc_mouse_button button)
{
    oc_key_state state = oc_mouse_button_get_state(input, button);
    bool clicked = state.sysClicked && (state.lastUpdate == input->frameCounter);
    return (clicked);
}

bool oc_mouse_double_clicked(oc_input_state* input, oc_mouse_button button)
{
    oc_key_state state = oc_mouse_button_get_state(input, button);
    bool doubleClicked = state.sysDoubleClicked && (state.lastUpdate == input->frameCounter);
    return (doubleClicked);
}

bool oc_mouse_triple_clicked(oc_input_state* input, oc_mouse_button button)
{
    oc_key_state state = oc_mouse_button_get_state(input, button);
    bool tripleClicked = state.sysTripleClicked && (state.lastUpdate == input->frameCounter);
    return (tripleClicked);
}

oc_keymod_flags oc_key_mods(oc_input_state* input)
{
    return (input->keyboard.mods);
}

oc_vec2 oc_mouse_position(oc_input_state* input)
{
    if(input->mouse.posValid)
    {
        return (input->mouse.pos);
    }
    else
    {
        return ((oc_vec2){ -1, -1 });
    }
}

oc_vec2 oc_mouse_delta(oc_input_state* input)
{
    if(input->mouse.lastUpdate == input->frameCounter)
    {
        return (input->mouse.delta);
    }
    else
    {
        return ((oc_vec2){ 0, 0 });
    }
}

oc_vec2 oc_mouse_wheel(oc_input_state* input)
{
    if(input->mouse.lastUpdate == input->frameCounter)
    {
        return (input->mouse.wheel);
    }
    else
    {
        return ((oc_vec2){ 0, 0 });
    }
}

oc_str32 oc_input_text_utf32(oc_arena* arena, oc_input_state* input)
{
    oc_str32 res = { 0 };
    if(input->text.lastUpdate == input->frameCounter)
    {
        res = oc_str32_push_copy(arena, input->text.codePoints);
    }
    return (res);
}

oc_str8 oc_input_text_utf8(oc_arena* arena, oc_input_state* input)
{
    oc_str8 res = { 0 };
    if(input->text.lastUpdate == input->frameCounter)
    {
        res = oc_utf8_push_from_codepoints(arena, input->text.codePoints);
    }
    return (res);
}

bool oc_clipboard_pasted(oc_input_state* input)
{
    return input->clipboard.lastUpdate == input->frameCounter;
}

oc_str8 oc_clipboard_pasted_text(oc_input_state* input)
{
    oc_str8 res = { 0 };
    if(input->clipboard.lastUpdate == input->frameCounter)
    {
        res = input->clipboard.pastedText;
    }
    return (res);
}
