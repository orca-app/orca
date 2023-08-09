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

int main()
{
	mp_init();
	mp_clock_init(); //TODO put that in mp_init()?

	mp_rect windowRect = {.x = 100, .y = 100, .w = 810, .h = 610};
	mp_window window = mp_window_create(windowRect, "test", 0);

	mp_rect contentRect = mp_window_get_content_rect(window);

	//NOTE: create surface
	mg_surface surface = mg_surface_create_for_window(window, MG_CANVAS);
	if(mg_surface_is_nil(surface))
	{
		log_error("couldn't create surface\n");
		return(-1);
	}
	mg_surface_swap_interval(surface, 0);

	//NOTE: create canvas
	mg_canvas canvas = mg_canvas_create();
	if(mg_canvas_is_nil(canvas))
	{
		log_error("Error: couldn't create canvas\n");
		return(-1);
	}

	//NOTE: create atlas
	mem_arena permanentArena = {0};
	mem_arena_init(&permanentArena);

	mg_rect_atlas* atlas = mg_rect_atlas_create(&permanentArena, 16000, 16000);
	mg_image atlasImage = mg_image_create(surface, 16000, 16000);

	str8 path1 = path_executable_relative(mem_scratch(), STR8("../../sketches/resources/triceratops.png"));
	str8 path2 = path_executable_relative(mem_scratch(), STR8("../../sketches/resources/Top512.png"));

	mg_image_region image1 = mg_image_atlas_alloc_from_file(atlas, atlasImage, path1, false);
	mg_image_region image2 = mg_image_atlas_alloc_from_file(atlas, atlasImage, path2, false);

	// start app
	mp_window_bring_to_front(window);
	mp_window_focus(window);

	while(!mp_should_quit())
	{
		mp_pump_events(0);
		mp_event* event = 0;
		while((event  = mp_next_event(mem_scratch())) != 0)
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

			mg_set_color_rgba(0, 1, 1, 1);
			mg_clear();

			mg_set_color_rgba(1, 1, 1, 1);

			mg_image_draw_region(image1.image, image1.rect, (mp_rect){100, 100, 300, 300});
			mg_image_draw_region(image2.image, image2.rect, (mp_rect){300, 200, 300, 300});

			mg_render(surface, canvas);
		mg_surface_present(surface);

		mem_arena_clear(mem_scratch());
	}

	mg_image_atlas_recycle(atlas, image1);
	mg_image_atlas_recycle(atlas, image2);

	mg_canvas_destroy(canvas);
	mg_surface_destroy(surface);
	mp_window_destroy(window);

	mp_terminate();

	return(0);
}
