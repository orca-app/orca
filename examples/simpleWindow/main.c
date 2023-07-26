/************************************************************//**
*
*	@file: main.cpp
*	@author: Martin Fouilleul
*	@date: 30/07/2022
*	@revision:
*
*****************************************************************/
#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#include"milepost.h"

int main()
{
	mp_init();

	mp_rect rect = {.x = 100, .y = 100, .w = 800, .h = 600};
	mp_window window = mp_window_create(rect, "test", 0);

	mp_window_bring_to_front(window);
	mp_window_focus(window);

	while(!mp_should_quit())
	{
		mp_pump_events(0);
		mp_event *event = 0;
		while((event = mp_next_event(mem_scratch())) != 0)
		{
			switch(event->type)
			{
				case MP_EVENT_WINDOW_CLOSE:
				{
					mp_request_quit();
				} break;

				case MP_EVENT_WINDOW_RESIZE:
				{
					printf("resized, frame = {%f, %f, %f, %f}, content = {%f, %f, %f, %f}\n",
					       event->move.frame.x,
					       event->move.frame.y,
					       event->move.frame.w,
					       event->move.frame.h,
					       event->move.content.x,
					       event->move.content.y,
					       event->move.content.w,
					       event->move.content.h);
				} break;

				case MP_EVENT_WINDOW_MOVE:
				{
					printf("moved, frame = {%f, %f, %f, %f}, content = {%f, %f, %f, %f}\n",
					       event->move.frame.x,
					       event->move.frame.y,
					       event->move.frame.w,
					       event->move.frame.h,
					       event->move.content.x,
					       event->move.content.y,
					       event->move.content.w,
					       event->move.content.h);
				} break;

				case MP_EVENT_MOUSE_MOVE:
				{
					printf("mouse moved, pos = {%f, %f}, delta = {%f, %f}\n",
					       event->mouse.x,
					       event->mouse.y,
					       event->mouse.deltaX,
					       event->mouse.deltaY);
				} break;

				case MP_EVENT_MOUSE_WHEEL:
				{
					printf("mouse wheel, delta = {%f, %f}\n",
					       event->mouse.deltaX,
					       event->mouse.deltaY);
				} break;

				case MP_EVENT_MOUSE_ENTER:
				{
					printf("mouse enter\n");
				} break;

				case MP_EVENT_MOUSE_LEAVE:
				{
					printf("mouse leave\n");
				} break;

				case MP_EVENT_MOUSE_BUTTON:
				{
					printf("mouse button %i: %i\n",
					       event->key.code,
					       event->key.action == MP_KEY_PRESS ? 1 : 0);
				} break;

				case MP_EVENT_KEYBOARD_KEY:
				{
					printf("key %i: %s\n",
					        event->key.code,
					        event->key.action == MP_KEY_PRESS ? "press" : (event->key.action == MP_KEY_RELEASE ? "release" : "repeat"));
				} break;

				case MP_EVENT_KEYBOARD_CHAR:
				{
					printf("entered char %s\n", event->character.sequence);
				} break;

				default:
					break;
			}
		}
		mem_arena_clear(mem_scratch());
	}

	mp_terminate();

	return(0);
}
