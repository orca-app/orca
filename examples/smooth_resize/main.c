/************************************************************//**
*
*	@file: main.cpp
*	@author: Martin Fouilleul
*	@date: 30/07/2022
*	@revision:
*
*****************************************************************/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>

#define _USE_MATH_DEFINES //NOTE: necessary for MSVC
#include<math.h>

#define MG_INCLUDE_GL_API
#include"milepost.h"

#include<pthread.h>

unsigned int program;

const char* vshaderSource =
	"#version 430\n"
	"attribute vec4 vPosition;\n"
	"uniform mat4 transform;\n"
	"void main()\n"
	"{\n"
	"   gl_Position = transform*vPosition;\n"
	"}\n";

const char* fshaderSource =
	"#version 430\n"
	"precision mediump float;\n"
	"void main()\n"
	"{\n"
	"    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
	"}\n";

void compile_shader(GLuint shader, const char* source)
{
	glShaderSource(shader, 1, &source, 0);
	glCompileShader(shader);

	int err = glGetError();
	if(err)
	{
		printf("gl error: %i\n", err);
	}

	int status = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if(!status)
	{
		char buffer[256];
		int size = 0;
		glGetShaderInfoLog(shader, 256, &size, buffer);
		printf("shader error: %.*s\n", size, buffer);
	}
}

GLfloat vertices[] = {
		-0.866/2, -0.5/2, 0, 0.866/2, -0.5/2, 0, 0, 0.5, 0};

typedef struct app_data
{
	mp_window window;
	mg_surface surface;
	mg_canvas canvas;
	mg_font font;

	GLuint vertexBuffer;
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
			log_info("resizing window!\n");
		} break;

		default:
			break;
	}
}

void update_and_render(app_data* app)
{
	mg_surface_prepare(app->surface);

	glClearColor(0.3, 0.3, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);

 	static float alpha = 0;
	//f32 aspect = frameSize.x/frameSize.y;
	f32 aspect = 800/(f32)600;

   	GLfloat matrix[] = {cosf(alpha)/aspect, sinf(alpha), 0, 0,
    	                  	-sinf(alpha)/aspect, cosf(alpha), 0, 0,
    	                  	0, 0, 1, 0,
    	                  	0, 0, 0, 1};

   	alpha += 2*M_PI/120;

   	glUniformMatrix4fv(0, 1, false, matrix);


	glBindBuffer(GL_ARRAY_BUFFER, app->vertexBuffer);
   	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
   	glEnableVertexAttribArray(0);

   	glDrawArrays(GL_TRIANGLES, 0, 3);

	mg_surface_present(app->surface);

	mem_arena_clear(mem_scratch());
}

void* render(void* user)
{
	app_data* app = (app_data*)user;

	//NOTE: init shader and gl state
	mg_surface_prepare(app->surface);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &app->vertexBuffer);

	GLfloat vertices[] = {
		-0.866/2, -0.5/2, 0, 0.866/2, -0.5/2, 0, 0, 0.5, 0};

	glBindBuffer(GL_ARRAY_BUFFER, app->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


	unsigned int vshader = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fshader = glCreateShader(GL_FRAGMENT_SHADER);
	program = glCreateProgram();

	compile_shader(vshader, vshaderSource);
	compile_shader(fshader, fshaderSource);

	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	glLinkProgram(program);

	int status = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if(!status)
	{
		char buffer[256];
		int size = 0;
		glGetProgramInfoLog(program, 256, &size, buffer);
		printf("link error: %.*s\n", size, buffer);
 	}

	glUseProgram(program);

	while(!mp_should_quit())
	{
		mp_event* event = 0;

		while((event = mp_next_event(mem_scratch())) != 0)
		{
			process_event(app, *event);
		}
		update_and_render(app);
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

	//NOTE: create surface
	mg_surface surface = mg_surface_create_for_window(window, MG_GL);
	if(mg_surface_is_nil(surface))
	{
		printf("Error: couldn't create surface\n");
		return(-1);
	}

	mg_surface_swap_interval(surface, 1);
	mg_surface_deselect();


	// start app
	mp_window_bring_to_front(window);
	mp_window_focus(window);

	//TODO: start thread
	app_data app = {.window = window,
	                .surface = surface};

	pthread_t renderThread;
	pthread_create(&renderThread, 0, render, &app);

	while(!mp_should_quit())
	{
		mp_pump_events(0);
	}

	void* res;
	pthread_join(renderThread, &res);

	mg_surface_destroy(surface);
	mp_window_destroy(window);

	mp_terminate();

	return(0);
}
