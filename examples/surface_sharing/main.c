
#include<stdlib.h>
#include<string.h>

#define _USE_MATH_DEFINES //NOTE: necessary for MSVC
#include<math.h>

#include<stdlib.h>
#include<stdio.h>

#define MG_INCLUDE_GL_API 1
#include"milepost.h"

#define LOG_SUBSYSTEM "Main"

#ifdef OS_WIN64
	#include<process.h>
	#include<io.h>
	#include<fcntl.h>

	#define dup2 _dup2
	#define pipe(fds) _pipe(fds, 256, O_BINARY)
	#define read _read
	#define write _write
	#define itoa _itoa

	#define process_id HANDLE

	process_id spawn_child(char* program, char** argv)
	{
		return((process_id)_spawnv(P_NOWAIT, program, argv));
	}

	void terminate_child(process_id child)
	{
		TerminateProcess(child, 0);
	}

#elif OS_MACOS
	#include<unistd.h>

	void spwan_child(char* program, char** argv)
	{
		pid_t pid = fork();
		if(!pid)
		{
			char* envp[] = {0};
			execve(program, argv, envp);
			assert(0);
		}
	}
#endif


unsigned int program;

const char* vshaderSource =
	//"#version 320 es\n"
	"attribute vec4 vPosition;\n"
	"uniform mat4 transform;\n"
	"void main()\n"
	"{\n"
	"   gl_Position = transform*vPosition;\n"
	"}\n";

const char* fshaderSource =
	//"#version 320 es\n"
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

int child_main(int writeFd)
{
	mp_init();

	mp_rect rect = {.x = 100, .y = 100, .w = 800, .h = 600};
	mp_window window = mp_window_create(rect, "test", 0);

	//NOTE: create surface
	mg_surface surface = mg_surface_create_for_window(window, MG_BACKEND_GLES);
	mg_surface_prepare(surface);

	//NOTE: init shader and gl state
	mg_surface_prepare(surface);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer);

	GLfloat vertices[] = {
		-0.866/2, -0.5/2, 0, 0.866/2, -0.5/2, 0, 0, 0.5, 0};

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
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

	//NOTE: create surface server and start sharing surface
	mg_surface_server server = mg_surface_server_create();
	mg_surface_connection_id connectionID = mg_surface_server_start(server, surface);

	//NOTE: send context id to parent
	write(writeFd, &connectionID, sizeof(connectionID));

	//NOTE: render loop
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


		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
   		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
   		glEnableVertexAttribArray(0);

   		glDrawArrays(GL_TRIANGLES, 0, 3);

		mg_surface_present(surface);
	}

	mp_terminate();

	return(0);
}

int main(int argc, char** argv)
{
	LogLevel(LOG_LEVEL_DEBUG);

	if(argc > 1)
	{
		if(!strcmp(argv[1], "--child"))
		{
			int writeFd = atoi(argv[2]);
			printf("child process created with file desc %i\n", writeFd);
			return(child_main(writeFd));
		}
		else
		{
			return(-1);
		}
	}
//	setvbuf( stdout, NULL, _IONBF, 0 );
	mp_init();

	//NOTE: create main window
	mp_rect rect = {.x = 100, .y = 100, .w = 800, .h = 600};
	mp_window window = mp_window_create(rect, "test", 0);

	//NOTE: create surface client
	mg_surface surface = mg_surface_client_create_for_window(window);


	//NOTE setup descriptors
	int fileDesc[2];
	pipe(fileDesc);

	printf("parent process created readFd %i and writeFd %i\n", fileDesc[0], fileDesc[1]);

	char writeDescStr[64];
	itoa(fileDesc[1], writeDescStr, 10);
	char* args[] = {"bin/example_surface_sharing", "--child", writeDescStr, 0};

	process_id child = spawn_child(args[0], args);

	//NOTE: read the connection id
	mg_surface_connection_id connectionID = 0;
	read(fileDesc[0], &connectionID, sizeof(connectionID));
	printf("received child connection id %llu\n", connectionID);

	//NOTE: connect the client
	mg_surface_client_connect(surface, connectionID);

	//NOTE: show the window
	mp_window_bring_to_front(window);

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
		mg_surface_present(surface);
	}

	terminate_child(child);

	mp_terminate();
	return(0);
}
