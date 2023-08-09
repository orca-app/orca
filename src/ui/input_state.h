/************************************************************//**
*
*	@file: input_state.h
*	@author: Martin Fouilleul
*	@date: 19/04/2023
*
*****************************************************************/
#ifndef __INPUT_STATE_H_
#define __INPUT_STATE_H_

#include"platform/platform.h"
#include"util/typedefs.h"
#include"util/strings.h"
#include"util/utf8.h"
#include"app/mp_app.h"

typedef struct mp_key_state
{
	u64 lastUpdate;
	u32 transitionCount;
	u32 repeatCount;
	bool down;
	bool sysClicked;
	bool sysDoubleClicked;

} mp_key_state;

typedef struct mp_keyboard_state
{
	mp_key_state keys[MP_KEY_COUNT];
	mp_keymod_flags  mods;
} mp_keyboard_state;

typedef struct mp_mouse_state
{
	u64 lastUpdate;
	bool posValid;
	vec2 pos;
	vec2 delta;
	vec2 wheel;

	union
	{
		mp_key_state buttons[MP_MOUSE_BUTTON_COUNT];
		struct
		{
			mp_key_state left;
			mp_key_state right;
			mp_key_state middle;
			mp_key_state ext1;
			mp_key_state ext2;
		};
	};
} mp_mouse_state;

enum { MP_INPUT_TEXT_BACKING_SIZE = 64 };

typedef struct mp_text_state
{
	u64 lastUpdate;
	utf32 backing[MP_INPUT_TEXT_BACKING_SIZE];
	str32 codePoints;
} mp_text_state;

typedef struct mp_input_state
{
	u64 frameCounter;
	mp_keyboard_state keyboard;
	mp_mouse_state	mouse;
	mp_text_state text;
} mp_input_state;

MP_API void mp_input_process_event(mp_input_state* state, mp_event* event);
MP_API void mp_input_next_frame(mp_input_state* state);

MP_API bool mp_key_down(mp_input_state* state, mp_key_code key);
MP_API int mp_key_pressed(mp_input_state* state, mp_key_code key);
MP_API int mp_key_released(mp_input_state* state, mp_key_code key);
MP_API int mp_key_repeated(mp_input_state* state, mp_key_code key);

MP_API bool mp_mouse_down(mp_input_state* state, mp_mouse_button button);
MP_API int mp_mouse_pressed(mp_input_state* state, mp_mouse_button button);
MP_API int mp_mouse_released(mp_input_state* state, mp_mouse_button button);
MP_API bool mp_mouse_clicked(mp_input_state* state, mp_mouse_button button);
MP_API bool mp_mouse_double_clicked(mp_input_state* state, mp_mouse_button button);

MP_API vec2 mp_mouse_position(mp_input_state* state);
MP_API vec2 mp_mouse_delta(mp_input_state* state);
MP_API vec2 mp_mouse_wheel(mp_input_state* state);

MP_API str32 mp_input_text_utf32(mp_input_state* state, mem_arena* arena);
MP_API str8 mp_input_text_utf8(mp_input_state* state, mem_arena* arena);

MP_API mp_keymod_flags mp_key_mods(mp_input_state* state);

#endif //__INPUT_STATE_H_
