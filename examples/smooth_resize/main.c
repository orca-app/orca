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

#include<pthread.h>

#define LOG_SUBSYSTEM "Main"


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

typedef struct app_data
{
	mp_window window;
	mg_surface surface;
	mg_canvas canvas;
	mg_font font;

	f32 x;
	f32 y;
	f32 dx;
	f32 dy;
	f32 speed;
	f32 frameTime;

} app_data;

void process_event(app_data* app, mp_event event)
{
	switch(event.type)
	{
		case MP_EVENT_WINDOW_CLOSE:
		{
			mp_request_quit();
		} break;

		case MP_EVENT_WINDOW_RESIZE:
		{
			mp_rect frame = {0, 0, event.frame.rect.w, event.frame.rect.h};
			mg_surface_set_frame(app->surface, frame);
		} break;

		case MP_EVENT_KEYBOARD_KEY:
		{
			if(event.key.action == MP_KEY_PRESS || event.key.action == MP_KEY_REPEAT)
			{
				f32 factor = (event.key.mods & MP_KEYMOD_SHIFT) ? 10 : 1;

				if(event.key.code == MP_KEY_LEFT)
				{
						app->x-=0.3*factor;
				}
				else if(event.key.code == MP_KEY_RIGHT)
				{
						app->x+=0.3*factor;
				}
				else if(event.key.code == MP_KEY_UP)
				{
						app->y-=0.3*factor;
				}
				else if(event.key.code == MP_KEY_DOWN)
				{
						app->y+=0.3*factor;
				}
			}
		} break;

		default:
			break;
	}
}

void update_and_render(app_data* app)
{
	mp_rect contentRect = mp_window_get_content_rect(app->window);

	if(app->x-200 < 0)
	{
		app->x = 200;
		app->dx = app->speed;
	}
	if(app->x+200 > contentRect.w)
	{
		app->x = contentRect.w - 200;
		app->dx = -app->speed;
	}
	if(app->y-200 < 0)
	{
		app->y = 200;
		app->dy = app->speed;
	}
	if(app->y+200 > contentRect.h)
	{
		app->y = contentRect.h - 200;
		app->dy = -app->speed;
	}
	app->x += app->dx;
	app->y += app->dy;

	f64 startTime = mp_get_time(MP_CLOCK_MONOTONIC);

	mg_surface_prepare(app->surface);

		// background
		mg_set_color_rgba(0, 1, 1, 1);
		mg_clear();

		// head
		mg_set_color_rgba(1, 1, 0, 1);

		mg_circle_fill(app->x, app->y, 200);

		// smile
		f32 frown = app->frameTime > 0.033 ? -100 : 0;

		mg_set_color_rgba(0, 0, 0, 1);
		mg_set_width(20);
		mg_move_to(app->x-100, app->y+100);
		mg_cubic_to(app->x-50, app->y+150+frown, app->x+50, app->y+150+frown, app->x+100, app->y+100);
		mg_stroke();

		// eyes
		mg_ellipse_fill(app->x-70, app->y-50, 30, 50);
		mg_ellipse_fill(app->x+70, app->y-50, 30, 50);

		// text
		mg_set_color_rgba(0, 0, 1, 1);
		mg_set_font(app->font);
		mg_set_font_size(12);
		mg_move_to(50, 600-50);

		str8 text = str8_pushf(mem_scratch(),
			                     "Milepost vector graphics test program (frame time = %fs, fps = %f)...",
			                     app->frameTime,
			                     1./app->frameTime);
		mg_text_outlines(text);
		mg_fill();

		printf("Milepost vector graphics test program (frame time = %fs, fps = %f)...\n",
			                     app->frameTime,
			                     1./app->frameTime);

		mg_flush();
	mg_surface_present(app->surface);

	mem_arena_clear(mem_scratch());
	app->frameTime = mp_get_time(MP_CLOCK_MONOTONIC) - startTime;
}

void* render(void* user)
{
	app_data* app = (app_data*)user;

	mg_canvas_prepare(app->canvas);

	while(!mp_should_quit())
	{
		mp_event event = {0};
		while(mp_next_event(&event))
		{
			process_event(app, event);
		}
		update_and_render(app);
	}

	return(0);
}

int main()
{
	LogLevel(LOG_LEVEL_WARNING);

	mp_init();
	mp_clock_init(); //TODO put that in mp_init()?

	mp_rect windowRect = {.x = 100, .y = 100, .w = 810, .h = 610};
	mp_window window = mp_window_create(windowRect, "test", 0);

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

	//TODO: start thread
	app_data app = {.window = window,
	                .surface = surface,
	                .canvas = canvas,
	                .font = font,
	                .x = 400,
	                .y = 300,
	                .dx = 0,
	                .dy = 0};

	pthread_t renderThread;
	pthread_create(&renderThread, 0, render, &app);

	while(!mp_should_quit())
	{
		mp_pump_events(0);
		/*
		mp_event event = {0};
		while(mp_next_event(&event))
		{
			process_event(&app, event);
		}
		update_and_render(&app);
		//*/
	}

	void* res;
	pthread_join(renderThread, &res);

	mg_font_destroy(font);
	mg_canvas_destroy(canvas);
	mg_surface_destroy(surface);
	mp_window_destroy(window);

	mp_terminate();

	return(0);
}
