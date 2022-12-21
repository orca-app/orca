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

#include"win32_app.c"
#include"debug_log.c"
#include"memory.c"
#include"strings.c"


#include"ringbuffer.c"
#include"win32_base_allocator.c"
#include"utf8.c"

#define MG_IMPLEMENTS_BACKEND_GL
#include"graphics.c"
#include"win32_gl_surface.c"

//#define LOG_SUBSYSTEM "Main"

int main()
{
	LogLevel(LOG_LEVEL_DEBUG);

	mp_init();

	mp_rect rect = {.x = 100, .y = 100, .w = 800, .h = 600};
	mp_window window = mp_window_create(rect, "test", 0);
	//TODO: create surface
	mg_surface surface = mg_surface_create_for_window(window, MG_BACKEND_GL);

	mp_window_bring_to_front_and_focus(window);

	while(!mp_should_quit())
	{
		mp_pump_events(0);
		mp_event event = {0};
		while(mp_next_event(&event))
		{
			switch(event.type)
			{
				case MP_EVENT_KEYBOARD_CHAR:
				{
					printf("entered char %s\n", event.character.sequence);
				} break;

				case MP_EVENT_WINDOW_CLOSE:
				{
					mp_do_quit();
				} break;

				case MP_EVENT_WINDOW_RESIZE:
				{
				} break;

				default:
					break;
			}
		}

		mg_surface_prepare(surface);

		//DO stuff
		glClearColor(1, 0, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		mg_surface_present(surface);
	}

	mp_terminate();

	return(0);
}
