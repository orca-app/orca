/************************************************************//**
*
*	@file: main.cpp
*	@author: Martin Fouilleul
*	@date: 30/07/2022
*	@revision:
*
*****************************************************************/
#include<stdlib.h>
#include<string.h>

#define _USE_MATH_DEFINES //NOTE: necessary for MSVC
#include<math.h>

#include"milepost.h"
#include"metal_surface.h"

#define LOG_SUBSYSTEM "Main"

int main()
{
	LogLevel(LOG_LEVEL_DEBUG);

	mp_init();

	mp_rect rect = {.x = 100, .y = 100, .w = 800, .h = 600};
	mp_window window = mp_window_create(rect, "test", 0);

	//NOTE: create surface
	mg_surface surface = mg_metal_surface_create_for_window(window);

	//TODO: create canvas
	mg_canvas canvas = mg_canvas_create(surface);

	// start app
	mp_window_bring_to_front(window);
	mp_window_focus(window);

	while(!mp_should_quit())
	{
		mp_pump_events(0);
		mp_event event = {0};
		while(mp_next_event(&event))
		{
			switch(event.type)
			{
				case MP_EVENT_WINDOW_CLOSE:
				{
					mp_request_quit();
				} break;

				case MP_EVENT_WINDOW_RESIZE:
				{
					printf("resized, rect = {%f, %f, %f, %f}\n",
					       event.frame.rect.x,
					       event.frame.rect.y,
					       event.frame.rect.w,
					       event.frame.rect.h);
				} break;

				case MP_EVENT_WINDOW_MOVE:
				{
					printf("moved, rect = {%f, %f, %f, %f}\n",
					       event.frame.rect.x,
					       event.frame.rect.y,
					       event.frame.rect.w,
					       event.frame.rect.h);
				} break;

				case MP_EVENT_MOUSE_MOVE:
				{
					printf("mouse moved, pos = {%f, %f}, delta = {%f, %f}\n",
					       event.move.x,
					       event.move.y,
					       event.move.deltaX,
					       event.move.deltaY);
				} break;

				case MP_EVENT_MOUSE_WHEEL:
				{
					printf("mouse wheel, delta = {%f, %f}\n",
					       event.move.deltaX,
					       event.move.deltaY);
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
					       event.key.code,
					       event.key.action == MP_KEY_PRESS ? 1 : 0);
				} break;

				case MP_EVENT_KEYBOARD_KEY:
				{
					printf("key %i: %s\n",
					        event.key.code,
					        event.key.action == MP_KEY_PRESS ? "press" : (event.key.action == MP_KEY_RELEASE ? "release" : "repeat"));
				} break;

				case MP_EVENT_KEYBOARD_CHAR:
				{
					printf("entered char %s\n", event.character.sequence);
				} break;

				default:
					break;
			}
		}

		mg_surface_prepare(surface);
			mg_set_color_rgba(1, 0, 1, 1);
			mg_clear();

			mg_set_color_rgba(1, 1, 0, 1);
			mg_circle_fill(400, 300, 200);

			mg_set_color_rgba(0, 0, 0, 1);
			mg_set_width(20);

			mg_move_to(300, 200);
			mg_cubic_to(350, 150, 450, 150, 500, 200);
			mg_stroke();

			mg_ellipse_fill(330, 350, 30, 50);
			mg_ellipse_fill(470, 350, 30, 50);

			mg_flush();
		mg_surface_present(surface);
	}

	mp_terminate();

	return(0);
}
