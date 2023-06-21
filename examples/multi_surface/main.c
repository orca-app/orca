
#include<stdlib.h>
#include<stdio.h>

#define MG_INCLUDE_GL_API 1
#include"milepost.h"

int main()
{
	mp_init();
	mp_clock_init(); //TODO put that in mp_init()?

	mp_rect windowRect = {.x = 100, .y = 100, .w = 810, .h = 610};
	mp_window window = mp_window_create(windowRect, "test", 0);

	mp_rect contentRect = mp_window_get_content_rect(window);

	//NOTE: create surface
	mg_surface surface1 = mg_surface_create_for_window(window, MG_CANVAS);
	if(mg_surface_is_nil(surface1))
	{
		printf("Error: couldn't create surface 1\n");
		return(-1);
	}
	mg_surface_swap_interval(surface1, 0);
//*
	mg_surface surface2 = mg_surface_create_for_window(window, MG_CANVAS);
	if(mg_surface_is_nil(surface2))
	{
		printf("Error: couldn't create surface 2\n");
		return(-1);
	}
	mg_surface_swap_interval(surface2, 0);
//*/
	mg_canvas canvas1 = mg_canvas_create();
	if(mg_canvas_is_nil(canvas1))
	{
		printf("Error: couldn't create canvas 1\n");
		return(-1);
	}
//*
	mg_canvas canvas2 = mg_canvas_create();
	if(mg_canvas_is_nil(canvas2))
	{
		printf("Error: couldn't create canvas 2\n");
		return(-1);
	}
//*/
	// start app
	mp_window_bring_to_front(window);
	mp_window_focus(window);

	while(!mp_should_quit())
	{
		f64 startTime = mp_get_time(MP_CLOCK_MONOTONIC);

		mp_pump_events(0);
		mp_event* event = 0;
		while((event = mp_next_event(mem_scratch())) != 0)
		{
			switch(event->type)
			{
				case MP_EVENT_WINDOW_CLOSE:
				{
					mp_request_quit();
				} break;

				default:
					break;
			}
		}

		mg_surface_prepare(surface1);
		mg_canvas_set_current(canvas1);

			mg_set_color_rgba(0, 0, 0.5, 0.5);
			mg_clear();

			mg_set_color_rgba(1, 0, 0, 1);
			mg_rectangle_fill(100, 100, 300, 150);

		mg_render(surface1, canvas1);

//*
		mg_surface_prepare(surface2);
		mg_canvas_set_current(canvas2);

			mg_set_color_rgba(0, 0, 0, 0);
			mg_clear();

			mg_set_color_rgba(0, 0, 1, 1);
			mg_rectangle_fill(300, 300, 300, 200);

		mg_render(surface2, canvas2);
//*/

		mg_surface_present(surface1);
		mg_surface_present(surface2);

		mem_arena_clear(mem_scratch());
	}

	mg_canvas_destroy(canvas1);
	mg_surface_destroy(surface1);
	mg_canvas_destroy(canvas2);
	mg_surface_destroy(surface2);

	mp_window_destroy(window);

	mp_terminate();

	return(0);
}
