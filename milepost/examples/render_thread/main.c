
#include<stdlib.h>
#include<stdio.h>

#define MG_INCLUDE_GL_API 1
#include"milepost.h"

mg_surface surface = {0};
mg_canvas canvas = {0};

i32 render_thread(void* user)
{
	while(!mp_should_quit())
	{
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

		mg_surface_prepare(surface);
		mg_canvas_set_current(canvas);

			mg_set_color_rgba(0, 0, 0.5, 0.5);
			mg_clear();

			mg_set_color_rgba(1, 0, 0, 1);
			mg_rectangle_fill(100, 100, 300, 150);

		mg_render(surface, canvas);

		mg_surface_present(surface);

		mem_arena_clear(mem_scratch());
	}
	return(0);
}

int main()
{
	mp_init();
	mp_clock_init(); //TODO put that in mp_init()?

	mp_rect windowRect = {.x = 100, .y = 100, .w = 810, .h = 610};
	mp_window window = mp_window_create(windowRect, "test", 0);

	mp_rect contentRect = mp_window_get_content_rect(window);

	//NOTE: create surface
	surface = mg_surface_create_for_window(window, MG_CANVAS);
	if(mg_surface_is_nil(surface))
	{
		printf("Error: couldn't create surface 1\n");
		return(-1);
	}
	mg_surface_swap_interval(surface, 0);

	canvas = mg_canvas_create();
	if(mg_canvas_is_nil(canvas))
	{
		printf("Error: couldn't create canvas 1\n");
		return(-1);
	}

	mg_surface dummy = mg_surface_create_for_window(window, MG_CANVAS);

	// start app
	mp_window_center(window);
	mp_window_bring_to_front(window);
	mp_window_focus(window);

	mp_thread* renderThread = mp_thread_create(render_thread, NULL);

	while(!mp_should_quit())
	{
		mp_pump_events(0);
	}

	mp_thread_join(renderThread, NULL);

	mg_canvas_destroy(canvas);
	mg_surface_destroy(surface);

	mp_window_destroy(window);

	mp_terminate();

	return(0);
}
