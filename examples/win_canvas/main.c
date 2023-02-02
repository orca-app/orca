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

#define LOG_SUBSYSTEM "Main"

int main()
{
	LogLevel(LOG_LEVEL_DEBUG);

	mp_init();

	mp_rect rect = {.x = 100, .y = 100, .w = 800, .h = 600};
	mp_window window = mp_window_create(rect, "test", 0);

	//NOTE: create surface
#if defined(OS_MACOS)
	mg_surface surface = mg_metal_surface_create_for_window(window);
#elif defined(OS_WIN64)
	mg_surface surface = mg_gles_surface_create_for_window(window);
#else
	#error "unsupported OS"
#endif

	//TODO: create canvas
	mg_canvas canvas = mg_canvas_create(surface);

	// start app
	mp_window_bring_to_front(window);
	mp_window_focus(window);

	f32 dx = 0, dy = 0;

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

				case MP_EVENT_KEYBOARD_KEY:
				{
					if(event.key.action == MP_KEY_PRESS || event.key.action == MP_KEY_REPEAT)
					{
						if(event.key.code == MP_KEY_LEFT)
						{
							dx-=5.1;
						}
						else if(event.key.code == MP_KEY_RIGHT)
						{
							dx+=5.1;
						}
						else if(event.key.code == MP_KEY_UP)
						{
							dy+=5.1;
						}
						else if(event.key.code == MP_KEY_DOWN)
						{
							dy-=5.1;
						}
					}
				} break;

				default:
					break;
			}
		}

		mg_surface_prepare(surface);

			// background
			mg_set_color_rgba(0, 1, 1, 1);
			mg_clear();

			// head
			mg_set_color_rgba(1, 1, 0, 1);
			mg_circle_fill(dx+400, dy+300, 200);

			// smile
			mg_set_color_rgba(0, 0, 0, 1);

			mg_set_width(20);
			mg_move_to(dx+300, dy+200);
			mg_cubic_to(dx+350, dy+150, dx+450, dy+150, dx+500, dy+200);
			mg_stroke();

			// eyes
			mg_ellipse_fill(dx+330, dy+350, 30, 50);
			mg_ellipse_fill(dx+470, dy+350, 30, 50);

			mg_flush();
		mg_surface_present(surface);
	}

	mp_terminate();

	return(0);
}
