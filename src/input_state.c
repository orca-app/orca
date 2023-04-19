/************************************************************//**
*
*	@file: input_state.c
*	@author: Martin Fouilleul
*	@date: 19/04/2023
*
*****************************************************************/
#include"input_state.h"

//---------------------------------------------------------------
// Input state updating
//---------------------------------------------------------------

static void mp_update_key_state(mp_input_state* state, mp_key_state* key, mp_key_action action)
{
	u64 frameCounter = state->frameCounter;
	if(key->lastUpdate != frameCounter)
	{
		key->transitionCount = 0;
		key->repeatCount = 0;
		key->sysClicked = false;
		key->sysDoubleClicked = false;
		key->lastUpdate = frameCounter;
	}

	switch(action)
	{
		case MP_KEY_PRESS:
		{
			if(!key->down)
			{
				key->transitionCount++;
			}
			key->down = true;
		} break;

		case MP_KEY_REPEAT:
		{
			key->repeatCount++;
			key->down = true;
		} break;

		case MP_KEY_RELEASE:
		{
			if(key->down)
			{
				key->transitionCount++;
			}
			key->down = false;
		} break;

		default:
			break;
	}
}

static void mp_update_key_mods(mp_input_state* state, mp_keymod_flags mods)
{
	state->keyboard.mods = mods;
}

static void mp_update_mouse_move(mp_input_state* state, f32 x, f32 y, f32 deltaX, f32 deltaY)
{
	u64 frameCounter = state->frameCounter;
	mp_mouse_state* mouse = &state->mouse;
	if(mouse->lastUpdate != frameCounter)
	{
		mouse->delta = (vec2){0, 0};
		mouse->wheel = (vec2){0, 0};
		mouse->lastUpdate = frameCounter;
	}
	mouse->pos = (vec2){x, y};
	mouse->delta.x += deltaX;
	mouse->delta.y += deltaY;
}

static void mp_update_mouse_wheel(mp_input_state* state, f32 deltaX, f32 deltaY)
{
	u64 frameCounter = state->frameCounter;
	mp_mouse_state* mouse = &state->mouse;
	if(mouse->lastUpdate != frameCounter)
	{
		mouse->delta = (vec2){0, 0};
		mouse->wheel = (vec2){0, 0};
		mouse->lastUpdate = frameCounter;
	}
	mouse->wheel.x += deltaX;
	mouse->wheel.y += deltaY;
}

static void mp_update_text(mp_input_state* state, utf32 codepoint)
{
	u64 frameCounter = state->frameCounter;
	mp_text_state* text = &state->text;

	if(text->lastUpdate != frameCounter)
	{
		text->codePoints.len = 0;
		text->lastUpdate = frameCounter;
	}

	text->codePoints.ptr = text->backing;
	if(text->codePoints.len < MP_INPUT_TEXT_BACKING_SIZE)
	{
		text->codePoints.ptr[text->codePoints.len] = codepoint;
		text->codePoints.len++;
	}
	else
	{
		log_warning("too many input codepoints per frame, dropping input");
	}
}

void mp_input_next_frame(mp_input_state* state)
{
	state->frameCounter++;
}

void mp_input_process_event(mp_input_state* state, mp_event* event)
{
	switch(event->type)
	{
		case MP_EVENT_KEYBOARD_KEY:
		{
			mp_key_state* key = &state->keyboard.keys[event->key.code];
			mp_update_key_state(state, key, event->key.action);
			mp_update_key_mods(state, event->key.mods);
		} break;

		case MP_EVENT_KEYBOARD_CHAR:
			mp_update_text(state, event->character.codepoint);
			break;

		case MP_EVENT_KEYBOARD_MODS:
			mp_update_key_mods(state, event->key.mods);
			break;

		case MP_EVENT_MOUSE_MOVE:
			mp_update_mouse_move(state, event->move.x, event->move.y, event->move.deltaX, event->move.deltaY);
			break;

		case MP_EVENT_MOUSE_WHEEL:
			mp_update_mouse_wheel(state, event->move.deltaX, event->move.deltaY);
			break;

		case MP_EVENT_MOUSE_BUTTON:
		{
			mp_key_state* key = &state->mouse.buttons[event->key.code];
			mp_update_key_state(state, key, event->key.action);

			if(event->key.action == MP_KEY_PRESS)
			{
				if(event->key.clickCount >= 1)
				{
					key->sysClicked = true;
				}
				if(event->key.clickCount >= 2)
				{
					key->sysDoubleClicked = true;
				}
			}

			mp_update_key_mods(state, event->key.mods);
		} break;

		default:
			break;
	}
}

//--------------------------------------------------------------------
// Input state polling
//--------------------------------------------------------------------

mp_key_state mp_key_get_state(mp_input_state* input, mp_key_code key)
{
	mp_key_state state = {0};
	if(key <= MP_KEY_COUNT)
	{
		state = input->keyboard.keys[key];
	}
	return(state);
}

mp_key_state mp_mouse_button_get_state(mp_input_state* input, mp_mouse_button button)
{
	mp_key_state state = {0};
	if(button <= MP_MOUSE_BUTTON_COUNT)
	{
		state = input->mouse.buttons[button];
	}
	return(state);
}

int mp_key_state_press_count(mp_input_state* input, mp_key_state* key)
{
	int count = 0;
	if(key->lastUpdate == input->frameCounter)
	{
		count = key->transitionCount / 2;
		if(key->down)
		{
			//NOTE: add one if state is down transition count is odd
			count += (key->transitionCount & 0x01);
		}
	}
	return(count);
}

int mp_key_state_release_count(mp_input_state* input, mp_key_state* key)
{
	int count = 0;
	if(key->lastUpdate == input->frameCounter)
	{
		count = key->transitionCount / 2;
		if(!key->down)
		{
			//NOTE: add one if state is up and transition count is odd
			count += (key->transitionCount & 0x01);
		}
	}
	return(count);
}

int mp_key_state_repeat_count(mp_input_state* input, mp_key_state* key)
{
	int count = 0;
	if(key->lastUpdate == input->frameCounter)
	{
		count = key->repeatCount;
	}
	return(count);
}

bool mp_key_down(mp_input_state* input, mp_key_code key)
{
	mp_key_state state = mp_key_get_state(input, key);
	return(state.down);
}

int mp_key_pressed(mp_input_state* input, mp_key_code key)
{
	mp_key_state state = mp_key_get_state(input, key);
	int res = mp_key_state_press_count(input, &state);
	return(res);
}

int mp_key_released(mp_input_state* input, mp_key_code key)
{
	mp_key_state state = mp_key_get_state(input, key);
	int res = mp_key_state_release_count(input, &state);
	return(res);
}

int mp_key_repeated(mp_input_state* input, mp_key_code key)
{
	mp_key_state state = mp_key_get_state(input, key);
	int res = mp_key_state_repeat_count(input, &state);
	return(res);
}

bool mp_mouse_down(mp_input_state* input, mp_mouse_button button)
{
	mp_key_state state = mp_mouse_button_get_state(input, button);
	return(state.down);
}

int mp_mouse_pressed(mp_input_state* input, mp_mouse_button button)
{
	mp_key_state state = mp_mouse_button_get_state(input, button);
	int res = mp_key_state_press_count(input, &state);
	return(res);
}

int mp_mouse_released(mp_input_state* input, mp_mouse_button button)
{
	mp_key_state state = mp_mouse_button_get_state(input, button);
	int res = mp_key_state_release_count(input, &state);
	return(res);
}

bool mp_mouse_clicked(mp_input_state* input, mp_mouse_button button)
{
	mp_key_state state = mp_mouse_button_get_state(input, button);
	bool clicked = state.sysClicked && (state.lastUpdate == input->frameCounter);
	return(clicked);
}

bool mp_mouse_double_clicked(mp_input_state* input, mp_mouse_button button)
{
	mp_key_state state = mp_mouse_button_get_state(input, button);
	bool doubleClicked = state.sysClicked && (state.lastUpdate == input->frameCounter);
	return(doubleClicked);
}

mp_keymod_flags mp_key_mods(mp_input_state* input)
{
	return(input->keyboard.mods);
}

vec2 mp_mouse_position(mp_input_state* input)
{
	return(input->mouse.pos);
}

vec2 mp_mouse_delta(mp_input_state* input)
{
	if(input->mouse.lastUpdate == input->frameCounter)
	{
		return(input->mouse.delta);
	}
	else
	{
		return((vec2){0, 0});
	}
}

vec2 mp_mouse_wheel(mp_input_state* input)
{
	if(input->mouse.lastUpdate == input->frameCounter)
	{
		return(input->mouse.wheel);
	}
	else
	{
		return((vec2){0, 0});
	}
}

str32 mp_input_text_utf32(mp_input_state* input, mem_arena* arena)
{
	str32 res = {0};
	if(input->text.lastUpdate == input->frameCounter)
	{
		res = str32_push_copy(arena, input->text.codePoints);
	}
	return(res);
}

str8 mp_input_text_utf8(mp_input_state* input, mem_arena* arena)
{
	str8 res = {0};
	if(input->text.lastUpdate == input->frameCounter)
	{
		res = utf8_push_from_codepoints(arena, input->text.codePoints);
	}
	return(res);
}
