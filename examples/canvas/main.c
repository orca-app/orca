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


mg_font create_font()
{
	//NOTE(martin): create font
	str8 fontPath = mp_app_get_resource_path(mem_scratch(), "../resources/OpenSansLatinSubset.ttf");
	char* fontPathCString = str8_to_cstring(mem_scratch(), fontPath);

	FILE* fontFile = fopen(fontPathCString, "r");
	if(!fontFile)
	{
		LOG_ERROR("Could not load font file '%s': %s\n", fontPathCString, strerror(errno));
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

	mg_font font = create_font();

	// start app
	mp_window_bring_to_front(window);
	mp_window_focus(window);

	f32 x = 400, y = 300;
	f32 speed = 0;
	f32 dx = speed, dy = speed;
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

				case MP_EVENT_KEYBOARD_KEY:
				{
					if(event.key.action == MP_KEY_PRESS || event.key.action == MP_KEY_REPEAT)
					{
						f32 factor = (event.key.mods & MP_KEYMOD_SHIFT) ? 10 : 1;

						if(event.key.code == MP_KEY_LEFT)
						{
								x-=0.3*factor;
						}
						else if(event.key.code == MP_KEY_RIGHT)
						{
								x+=0.3*factor;
						}
						else if(event.key.code == MP_KEY_UP)
						{
								y-=0.3*factor;
						}
						else if(event.key.code == MP_KEY_DOWN)
						{
								y+=0.3*factor;
						}
					}
				} break;

				default:
					break;
			}
		}

		if(x-200 < 0)
		{
			x = 200;
			dx = speed;
		}
		if(x+200 > contentRect.w)
		{
			x = contentRect.w - 200;
			dx = -speed;
		}
		if(y-200 < 0)
		{
			y = 200;
			dy = speed;
		}
		if(y+200 > contentRect.h)
		{
			y = contentRect.h - 200;
			dy = -speed;
		}
		x += dx;
		y += dy;

		mg_surface_prepare(surface);

			// background
			mg_set_color_rgba(0, 1, 1, 1);
			mg_clear();

			// head
			mg_set_color_rgba(1, 1, 0, 1);

			mg_circle_fill(x, y, 200);

			// smile
			f32 frown = frameTime > 0.033 ? -100 : 0;

			mg_set_color_rgba(0, 0, 0, 1);
			mg_set_width(20);
			mg_move_to(x-100, y+100);
			mg_cubic_to(x-50, y+150+frown, x+50, y+150+frown, x+100, y+100);
			mg_stroke();

			// eyes
			mg_ellipse_fill(x-70, y-50, 30, 50);
			mg_ellipse_fill(x+70, y-50, 30, 50);

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

			mg_flush();
		mg_surface_present(surface);

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
