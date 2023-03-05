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

	ui_init();

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

	mem_arena textArena = {0};
	mem_arena_init(&textArena);

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

		if(mp_key_pressed(MP_KEY_C))
		{
			printf("pressed C!\n");
		}

		mg_surface_prepare(surface);

		mp_rect frame = mp_window_get_content_rect(window);

		ui_style defaultStyle = {.bgColor = {0.9, 0.9, 0.9, 1},
		                         .borderSize = 2,
		                         .borderColor = {0, 0, 1, 1},
		                         .fontColor = {0, 0, 0, 1},
		                         .font = font,
		                         .fontSize = 32};

		ui_begin_frame(frame.w, frame.h, defaultStyle);
		{
			ui_push_size(UI_AXIS_X, UI_SIZE_CHILDREN, 10, 0);
			ui_push_size(UI_AXIS_Y, UI_SIZE_CHILDREN, 10, 0);
			ui_panel("buttons")
			{
				ui_push_size(UI_AXIS_X, UI_SIZE_TEXT, 5, 0);
				ui_push_size(UI_AXIS_Y, UI_SIZE_TEXT, 5, 0);
				if(ui_button("1: Click me!").clicked)
				{
					printf("Clicked button 1!\n");
				}
				if(ui_button("2: Click me!").clicked)
				{
					printf("Clicked button 2!\n");
				}
				ui_pop_size(UI_AXIS_X);
				ui_pop_size(UI_AXIS_Y);
			}

			ui_panel("sliders")
			{
				static float scrollValue1 = 0.;
				static float scrollValue2 = 0.;

				ui_push_size(UI_AXIS_X, UI_SIZE_PIXELS, 300, 1);
				ui_push_size(UI_AXIS_Y, UI_SIZE_PIXELS, 50, 1);
				ui_scrollbar("scroll1", 0.5, &scrollValue1);
				ui_scrollbar("scroll2", 0.5, &scrollValue2);
				ui_pop_size(UI_AXIS_X);
				ui_pop_size(UI_AXIS_Y);
			}
			ui_panel("textbox")
			{
				ui_push_size(UI_AXIS_X, UI_SIZE_PIXELS, 300, 0);
				ui_push_size(UI_AXIS_Y, UI_SIZE_PIXELS, 50, 0);

				static str8 text = {};
				ui_text_box_result res = ui_text_box("textbox", mem_scratch(), text);
				if(res.changed)
				{
					mem_arena_clear(&textArena);
					text = str8_push_copy(&textArena, res.text);
				}

				ui_pop_size(UI_AXIS_X);
				ui_pop_size(UI_AXIS_Y);
			}
			ui_pop_size(UI_AXIS_X);
			ui_pop_size(UI_AXIS_Y);
		}
		ui_end_frame();
		ui_draw();

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
