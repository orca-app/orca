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

	//NOTE: create canvas
	mg_canvas canvas = mg_canvas_create(surface);
	if(mg_canvas_is_nil(canvas))
	{
		printf("Error: couldn't create canvas\n");
		return(-1);
	}

	//NOTE: create image
	str8 imagePath = mp_app_get_resource_path(mem_scratch(), "../resources/triceratops.png");
	mg_image image = mg_image_create_from_file(imagePath, true);
	vec2 imageSize = mg_image_size(image);

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

				default:
					break;
			}
		}

		mg_surface_prepare(surface);

			// background
			mg_set_color_rgba(0, 1, 1, 1);
			mg_clear();

			mg_set_color_rgba(1, 1, 1, 1);

			mg_matrix_push((mg_mat2x3){0.707, -0.707, 200,
			                           0.707, 0.707, 100});

			mg_set_image(image);
			mg_set_image_source_region((mp_rect){500, 500, 2000, 1400});

//			mg_rectangle_fill(100, 100, imageSize.x/8, imageSize.y/8);

			mg_move_to(0, 0);
			mg_line_to(200, 0);
			mg_line_to(300, 100);
			mg_line_to(200, 200);
			mg_line_to(0, 200);
			mg_line_to(100, 100);
			mg_fill();

			//mg_image_draw_rounded(image, (mp_rect){0, 0, imageSize.x/8, imageSize.y/8}, 40.);

			mg_matrix_pop();

			mg_flush();
		mg_surface_present(surface);

		mem_arena_clear(mem_scratch());
	}

	mg_image_destroy(image);
	mg_canvas_destroy(canvas);
	mg_surface_destroy(surface);
	mp_window_destroy(window);

	mp_terminate();

	return(0);
}
