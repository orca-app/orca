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


mg_font create_font()
{
	//NOTE(martin): create font
/*	str8 fontPath = mp_app_get_resource_path(mem_scratch(), "../resources/OpenSansLatinSubset.ttf");
	char* fontPathCString = str8_to_cstring(mem_scratch(), fontPath);
*/
	char* fontPathCString = "resources/OpenSansLatinSubset.ttf";

	FILE* fontFile = fopen(fontPathCString, "r");
	if(!fontFile)
	{
		LOG_ERROR("Could not load font file '%s'\n", fontPathCString);
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
	LogLevel(LOG_LEVEL_DEBUG);

	mp_init();
	mp_clock_init(); //TODO put that in mp_init()?

	mp_rect rect = {.x = 100, .y = 100, .w = 810, .h = 610};
	mp_window window = mp_window_create(rect, "test", 0);

	//NOTE: create surface
#if defined(OS_MACOS)
	mg_surface surface = mg_metal_surface_create_for_window(window);
#elif defined(OS_WIN64)
	mg_surface surface = mg_gl_surface_create_for_window(window);
#else
	#error "unsupported OS"
#endif

	//TODO: create canvas
	mg_canvas canvas = mg_canvas_create(surface);
	mg_font font = create_font();

	// start app
	mp_window_bring_to_front(window);
	mp_window_focus(window);

	f32 x = 400, y = 300;
	f32 dx = 5, dy = 5;
//	f32 dx = 0, dy = 0;

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
						/*
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
						*/
					}
				} break;

				default:
					break;
			}
		}

		if(x-200 < 0)
		{
			dx = 5;
		}
		if(x+200 > 800)
		{
			dx = -5;
		}
		if(y-200 < 0)
		{
			dy = 5;
		}
		if(y+200 > 550)
		{
			dy = -5;
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

			f32 frown = frameTime > 0.033 ? 100 : 0;

			mg_set_color_rgba(0, 0, 0, 1);

			mg_set_width(20);
			mg_move_to(x-100, y-100);
			mg_cubic_to(x-50, y-150+frown, x+50, y-150+frown, x+100, y-100);
			mg_stroke();

			// eyes
			mg_ellipse_fill(x-70, y+50, 30, 50);
			mg_ellipse_fill(x+70, y+50, 30, 50);

			// text
//*
			mg_set_color_rgba(0, 0, 1, 1);
			mg_set_font(font);
			mg_set_font_size(12);
			mg_move_to(50, 50);

			str8 text = str8_pushf(mem_scratch(),
			                      "Milepost vector graphics test program (frame time = %fs, fps = %f)...",
			                      frameTime,
			                      1./frameTime);
			mg_text_outlines(text);
			mg_fill();
//*/

			printf("Milepost vector graphics test program (frame time = %fs, fps = %f)...\n",
			                      frameTime,
			                      1./frameTime);

			mg_flush();
		mg_surface_present(surface);

		mem_arena_clear(mem_scratch());
		frameTime = mp_get_time(MP_CLOCK_MONOTONIC) - startTime;
	}

	mp_terminate();

	return(0);
}
