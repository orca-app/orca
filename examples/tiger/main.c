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
#include<stdio.h>
#include<errno.h>

#define _USE_MATH_DEFINES //NOTE: necessary for MSVC
#include<math.h>

#include"milepost.h"

#include"tiger.c"

mg_font create_font()
{
	//NOTE(martin): create font
	str8 fontPath = mp_app_get_resource_path(mem_scratch(), "../resources/OpenSansLatinSubset.ttf");
	char* fontPathCString = str8_to_cstring(mem_scratch(), fontPath);

	FILE* fontFile = fopen(fontPathCString, "r");
	if(!fontFile)
	{
		log_error("Could not load font file '%s': %s\n", fontPathCString, strerror(errno));
		return(mg_font_nil());
	}
	unsigned char* fontData = 0;
	fseek(fontFile, 0, SEEK_END);
	u32 fontDataSize = ftell(fontFile);
	rewind(fontFile);
	fontData = (unsigned char*)malloc(fontDataSize);
	fread(fontData, 1, fontDataSize, fontFile);
	fclose(fontFile);

	unicode_range ranges[5] = {UNICODE_RANGE_BASIC_LATIN,
	                           UNICODE_RANGE_C1_CONTROLS_AND_LATIN_1_SUPPLEMENT,
	                           UNICODE_RANGE_LATIN_EXTENDED_A,
	                           UNICODE_RANGE_LATIN_EXTENDED_B,
	                           UNICODE_RANGE_SPECIALS};

	mg_font font = mg_font_create_from_memory(fontDataSize, fontData, 5, ranges);
	free(fontData);

	return(font);
}

int main()
{
	mp_init();
	mp_clock_init(); //TODO put that in mp_init()?

	mp_rect windowRect = {.x = 100, .y = 100, .w = 810, .h = 610};
	mp_window window = mp_window_create(windowRect, "test", 0);

	mp_rect contentRect = mp_window_get_content_rect(window);

	//NOTE: create surface
	mg_surface surface = mg_surface_create_for_window(window, MG_CANVAS);
	mg_surface_swap_interval(surface, 0);

	//TODO: create canvas
	mg_canvas canvas = mg_canvas_create();

	if(mg_canvas_is_nil(canvas))
	{
		printf("Error: couldn't create canvas\n");
		return(-1);
	}

	mg_font font = create_font();

	// start app
	mp_window_bring_to_front(window);
	mp_window_focus(window);

	bool tracked = false;
	vec2 trackPoint = {0};

	f32 zoom = 1;
	f32 startX = 300, startY = 200;
	bool singlePath = false;
	int singlePathIndex = 0;

	f64 frameTime = 0;

	mp_input_state inputState = {0};

	while(!mp_should_quit())
	{
		f64 startTime = mp_get_time(MP_CLOCK_MONOTONIC);

		mp_pump_events(0);
		mp_event* event = 0;
		while((event = mp_next_event(mem_scratch())) != 0)
		{
			mp_input_process_event(&inputState, event);

			switch(event->type)
			{
				case MP_EVENT_WINDOW_CLOSE:
				{
					mp_request_quit();
				} break;

				case MP_EVENT_WINDOW_RESIZE:
				{
					mp_rect frame = {0, 0, event->frame.rect.w, event->frame.rect.h};
					mg_surface_set_frame(surface, frame);
				} break;

				case MP_EVENT_MOUSE_BUTTON:
				{
					if(event->key.code == MP_MOUSE_LEFT)
					{
						if(event->key.action == MP_KEY_PRESS)
						{
							tracked = true;
							vec2 mousePos = mp_mouse_position(&inputState);
							trackPoint.x = (mousePos.x - startX)/zoom;
							trackPoint.y = (mousePos.y - startY)/zoom;
						}
						else
						{
							tracked = false;
						}
					}
				} break;

				case MP_EVENT_MOUSE_WHEEL:
				{
					vec2 mousePos = mp_mouse_position(&inputState);
					f32 pinX = (mousePos.x - startX)/zoom;
					f32 pinY = (mousePos.y - startY)/zoom;

					zoom *= 1 + event->move.deltaY * 0.01;
					zoom = Clamp(zoom, 0.5, 5);

					startX = mousePos.x - pinX*zoom;
					startY = mousePos.y - pinY*zoom;
				} break;

				case MP_EVENT_KEYBOARD_KEY:
				{
					if(event->key.action == MP_KEY_PRESS || event->key.action == MP_KEY_REPEAT)
					{
						switch(event->key.code)
						{
							case MP_KEY_SPACE:
								singlePath = !singlePath;
								break;

							case MP_KEY_UP:
							{
								if(event->key.mods & MP_KEYMOD_SHIFT)
								{
									singlePathIndex++;
								}
								else
								{
									zoom += 0.001;
								}
							} break;

							case MP_KEY_DOWN:
							{
								if(event->key.mods & MP_KEYMOD_SHIFT)
								{
									singlePathIndex--;
								}
								else
								{
									zoom -= 0.001;
								}
							} break;
						}
					}
				} break;

				default:
					break;
			}
		}

		if(tracked)
		{
			vec2 mousePos = mp_mouse_position(&inputState);
			startX = mousePos.x - trackPoint.x*zoom;
			startY = mousePos.y - trackPoint.y*zoom;
		}

		mg_surface_prepare(surface);

		mg_set_color_rgba(1, 0, 1, 1);
		mg_clear();

		mg_matrix_push((mg_mat2x3){zoom, 0, startX,
		                           0, zoom, startY});

		draw_tiger(singlePath, singlePathIndex);

		if(singlePath)
		{
			printf("display single path %i\n", singlePathIndex);
			printf("viewpos = (%f, %f), zoom = %f\n", startX, startY, zoom);
		}

		mg_matrix_pop();

			// text
			mg_set_color_rgba(0, 0, 1, 1);
			mg_set_font(font);
			mg_set_font_size(12);
			mg_move_to(50, 600-50);

			str8 text = str8_pushf(mem_scratch(),
			                      "Milepost vector graphics test program (frame time = %fs, fps = %f)...",
			                      frameTime,
			                      1./frameTime);
			mg_text_outlines(text);
			mg_fill();

			printf("Milepost vector graphics test program (frame time = %fs, fps = %f)...\n",
			                      frameTime,
			                      1./frameTime);

		mg_render(surface, canvas);
		mg_surface_present(surface);

		mp_input_next_frame(&inputState);
		mem_arena_clear(mem_scratch());
		frameTime = mp_get_time(MP_CLOCK_MONOTONIC) - startTime;
	}

	mg_font_destroy(font);
	mg_canvas_destroy(canvas);
	mg_surface_destroy(surface);
	mp_window_destroy(window);

	mp_terminate();

	return(0);
}
