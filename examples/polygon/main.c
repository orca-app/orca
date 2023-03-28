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
#include<errno.h>

#define _USE_MATH_DEFINES //NOTE: necessary for MSVC
#include<math.h>

#include"milepost.h"

#define LOG_SUBSYSTEM "Main"

int main()
{
	LogLevel(LOG_LEVEL_WARNING);

	mp_init();
	mp_clock_init(); //TODO put that in mp_init()?

	mp_rect windowRect = {.x = 100, .y = 100, .w = 810, .h = 610};
	mp_window window = mp_window_create(windowRect, "test", 0);

	mp_rect contentRect = mp_window_get_content_rect(window);

	//NOTE: create surface
	mg_surface surface = mg_surface_create_for_window(window, MG_BACKEND_DEFAULT);
	mg_surface_swap_interval(surface, 0);

	//TODO: create canvas
	mg_canvas canvas = mg_canvas_create(surface);

	if(mg_canvas_is_nil(canvas))
	{
		printf("Error: couldn't create canvas\n");
		return(-1);
	}

	// start app
	mp_window_bring_to_front(window);
	mp_window_focus(window);

	f64 frameTime = 0;

	while(!mp_should_quit())
	{
		f64 startTime = mp_get_time(MP_CLOCK_MONOTONIC);

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

				default:
					break;
			}
		}

		mg_surface_prepare(surface);

			// background
			mg_set_color_rgba(0, 1, 1, 1);
			mg_clear();

			mg_move_to(100, 100);
			mg_line_to(150, 150);
			mg_line_to(100, 200);
			mg_line_to(50, 150);
			mg_close_path();
			mg_set_color_rgba(1, 0, 0, 1);
			mg_fill();

			mg_move_to(200, 100);
			mg_line_to(410, 100);
			mg_line_to(410, 200);
			mg_line_to(200, 200);
			mg_close_path();
			mg_set_color_rgba(0, 1, 0, 1);
			mg_fill();


			printf("Milepost vector graphics test program (frame time = %fs, fps = %f)...\n",
			                      frameTime,
			                      1./frameTime);

			mg_flush();
		mg_surface_present(surface);

		mem_arena_clear(mem_scratch());
		frameTime = mp_get_time(MP_CLOCK_MONOTONIC) - startTime;
	}

	mg_canvas_destroy(canvas);
	mg_surface_destroy(surface);
	mp_window_destroy(window);

	mp_terminate();

	return(0);
}
