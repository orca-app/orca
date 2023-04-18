/************************************************************//**
*
*	@file: mp_app_internal.c
*	@author: Martin Fouilleul
*	@date: 23/12/2022
*	@revision:
*
*****************************************************************/

#include"mp_app_internal.h"

mp_app __mpApp = {0};

//---------------------------------------------------------------
// Window handles
//---------------------------------------------------------------

void mp_init_window_handles()
{
	list_init(&__mpApp.windowFreeList);
	for(int i=0; i<MP_APP_MAX_WINDOWS; i++)
	{
		__mpApp.windowPool[i].generation = 1;
		list_append(&__mpApp.windowFreeList, &__mpApp.windowPool[i].freeListElt);
	}
}

bool mp_window_handle_is_null(mp_window window)
{
	return(window.h == 0);
}

mp_window mp_window_null_handle()
{
	return((mp_window){.h = 0});
}

mp_window_data* mp_window_alloc()
{
	return(list_pop_entry(&__mpApp.windowFreeList, mp_window_data, freeListElt));
}

mp_window_data* mp_window_ptr_from_handle(mp_window handle)
{
	u32 index = handle.h>>32;
	u32 generation = handle.h & 0xffffffff;
	if(index >= MP_APP_MAX_WINDOWS)
	{
		return(0);
	}
	mp_window_data* window = &__mpApp.windowPool[index];
	if(window->generation != generation)
	{
		return(0);
	}
	else
	{
		return(window);
	}
}

mp_window mp_window_handle_from_ptr(mp_window_data* window)
{
	DEBUG_ASSERT(  (window - __mpApp.windowPool) >= 0
	            && (window - __mpApp.windowPool) < MP_APP_MAX_WINDOWS);

	u64 h = ((u64)(window - __mpApp.windowPool))<<32
	      | ((u64)window->generation);

	return((mp_window){h});
}

void mp_window_recycle_ptr(mp_window_data* window)
{
	window->generation++;
	list_push(&__mpApp.windowFreeList, &window->freeListElt);
}

//---------------------------------------------------------------
// Init
//---------------------------------------------------------------

static void mp_init_common()
{
	mp_init_window_handles();
	ringbuffer_init(&__mpApp.eventQueue, 16);
}

static void mp_terminate_common()
{
	ringbuffer_cleanup(&__mpApp.eventQueue);
}

//---------------------------------------------------------------
// Event handling
//---------------------------------------------------------------

void mp_queue_event(mp_event* event)
{
	if(ringbuffer_write_available(&__mpApp.eventQueue) < sizeof(mp_event))
	{
		log_error("event queue full\n");
	}
	else
	{
		u32 written = ringbuffer_write(&__mpApp.eventQueue, sizeof(mp_event), (u8*)event);
		DEBUG_ASSERT(written == sizeof(mp_event));
	}
}

bool mp_next_event(mp_event* event)
{
	//NOTE pop and return event from queue
	if(ringbuffer_read_available(&__mpApp.eventQueue) >= sizeof(mp_event))
	{
		u64 read = ringbuffer_read(&__mpApp.eventQueue, sizeof(mp_event), (u8*)event);
		DEBUG_ASSERT(read == sizeof(mp_event));
		return(true);
	}
	else
	{
		return(false);
	}
}

//---------------------------------------------------------------
// Input state updating
//---------------------------------------------------------------

static void mp_update_key_state(mp_key_state* key, mp_key_action action)
{
	u64 frameCounter = __mpApp.inputState.frameCounter;
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

static void mp_update_key_mods(mp_keymod_flags mods)
{
	__mpApp.inputState.keyboard.mods = mods;
}

static void mp_update_mouse_move(f32 x, f32 y, f32 deltaX, f32 deltaY)
{
	u64 frameCounter = __mpApp.inputState.frameCounter;
	mp_mouse_state* mouse = &__mpApp.inputState.mouse;
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

static void mp_update_mouse_wheel(f32 deltaX, f32 deltaY)
{
	u64 frameCounter = __mpApp.inputState.frameCounter;
	mp_mouse_state* mouse = &__mpApp.inputState.mouse;
	if(mouse->lastUpdate != frameCounter)
	{
		mouse->delta = (vec2){0, 0};
		mouse->wheel = (vec2){0, 0};
		mouse->lastUpdate = frameCounter;
	}
	mouse->wheel.x += deltaX;
	mouse->wheel.y += deltaY;
}

static void mp_update_text(utf32 codepoint)
{
	u64 frameCounter = __mpApp.inputState.frameCounter;
	mp_text_state* text = &__mpApp.inputState.text;

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

//--------------------------------------------------------------------
// Input state polling
//--------------------------------------------------------------------

mp_key_state mp_key_get_state(mp_key_code key)
{
	mp_key_state state = {0};
	if(key <= MP_KEY_COUNT)
	{
		state = __mpApp.inputState.keyboard.keys[key];
	}
	return(state);
}

mp_key_state mp_mouse_button_get_state(mp_mouse_button button)
{
	mp_key_state state = {0};
	if(button <= MP_MOUSE_BUTTON_COUNT)
	{
		state = __mpApp.inputState.mouse.buttons[button];
	}
	return(state);
}

int mp_key_state_press_count(mp_key_state* key)
{
	int count = 0;
	if(key->lastUpdate == __mpApp.inputState.frameCounter)
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

int mp_key_state_release_count(mp_key_state* key)
{
	int count = 0;
	if(key->lastUpdate == __mpApp.inputState.frameCounter)
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

int mp_key_state_repeat_count(mp_key_state* key)
{
	int count = 0;
	if(key->lastUpdate == __mpApp.inputState.frameCounter)
	{
		count = key->repeatCount;
	}
	return(count);
}

bool mp_key_down(mp_key_code key)
{
	mp_key_state state = mp_key_get_state(key);
	return(state.down);
}

int mp_key_pressed(mp_key_code key)
{
	mp_key_state state = mp_key_get_state(key);
	int res = mp_key_state_press_count(&state);
	return(res);
}

int mp_key_released(mp_key_code key)
{
	mp_key_state state = mp_key_get_state(key);
	int res = mp_key_state_release_count(&state);
	return(res);
}

int mp_key_repeated(mp_key_code key)
{
	mp_key_state state = mp_key_get_state(key);
	int res = mp_key_state_repeat_count(&state);
	return(res);
}

bool mp_mouse_down(mp_mouse_button button)
{
	mp_key_state state = mp_mouse_button_get_state(button);
	return(state.down);
}

int mp_mouse_pressed(mp_mouse_button button)
{
	mp_key_state state = mp_mouse_button_get_state(button);
	int res = mp_key_state_press_count(&state);
	return(res);
}

int mp_mouse_released(mp_mouse_button button)
{
	mp_key_state state = mp_mouse_button_get_state(button);
	int res = mp_key_state_release_count(&state);
	return(res);
}

bool mp_mouse_clicked(mp_mouse_button button)
{
	mp_key_state state = mp_mouse_button_get_state(button);
	bool clicked = state.sysClicked && (state.lastUpdate == __mpApp.inputState.frameCounter);
	return(clicked);
}

bool mp_mouse_double_clicked(mp_mouse_button button)
{
	mp_key_state state = mp_mouse_button_get_state(button);
	bool doubleClicked = state.sysClicked && (state.lastUpdate == __mpApp.inputState.frameCounter);
	return(doubleClicked);
}

mp_keymod_flags mp_key_mods()
{
	return(__mpApp.inputState.keyboard.mods);
}

vec2 mp_mouse_position()
{
	return(__mpApp.inputState.mouse.pos);
}

vec2 mp_mouse_delta()
{
	if(__mpApp.inputState.mouse.lastUpdate == __mpApp.inputState.frameCounter)
	{
		return(__mpApp.inputState.mouse.delta);
	}
	else
	{
		return((vec2){0, 0});
	}
}

vec2 mp_mouse_wheel()
{
	if(__mpApp.inputState.mouse.lastUpdate == __mpApp.inputState.frameCounter)
	{
		return(__mpApp.inputState.mouse.wheel);
	}
	else
	{
		return((vec2){0, 0});
	}
}

str32 mp_input_text_utf32(mem_arena* arena)
{
	str32 res = {0};
	if(__mpApp.inputState.text.lastUpdate == __mpApp.inputState.frameCounter)
	{
		res = str32_push_copy(arena, __mpApp.inputState.text.codePoints);
	}
	return(res);
}

str8 mp_input_text_utf8(mem_arena* arena)
{
	str8 res = {0};
	if(__mpApp.inputState.text.lastUpdate == __mpApp.inputState.frameCounter)
	{
		res = utf8_push_from_codepoints(arena, __mpApp.inputState.text.codePoints);
	}
	return(res);
}
