
#include <stdlib.h>
#include <string.h>

#define _USE_MATH_DEFINES //NOTE: necessary for MSVC
#include <math.h>

#include <stdio.h>
#include <stdlib.h>

#define OC_INCLUDE_GL_API
#include "orca.h"

#ifdef OC_PLATFORM_WINDOWS
    #include <process.h>
    #include <io.h>
    #include <fcntl.h>

    #define dup2 _dup2
    #define pipe(fds) _pipe(fds, 256, O_BINARY)
    #define read _read
    #define write _write

    #define process_id HANDLE

process_id spawn_child(char* program, char** argv)
{
    return ((process_id)_spawnv(P_NOWAIT, program, argv));
}

void terminate_child(process_id child)
{
    TerminateProcess(child, 0);
}

#elif OC_PLATFORM_MACOS
    #include <unistd.h>
    #include <signal.h>

    #define process_id pid_t

process_id spawn_child(char* program, char** argv)
{
    pid_t pid = fork();
    if(!pid)
    {
        char* envp[] = { 0 };
        execve(program, argv, envp);
        OC_ASSERT(0);
    }
    return (pid);
}

void terminate_child(process_id child)
{
    kill(child, SIGTERM);
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
        oc_log_error("gl error: %i\n", err);
    }

    int status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if(!status)
    {
        char buffer[256];
        int size = 0;
        glGetShaderInfoLog(shader, 256, &size, buffer);
        oc_log_error("shader error: %.*s\n", size, buffer);
    }
}

int child_main(int writeFd)
{
    oc_init();

    oc_rect rect = { .x = 100, .y = 100, .w = 800, .h = 600 };
    oc_window window = oc_window_create(rect, OC_STR8("test"), 0);

    //NOTE: create surface
    oc_surface surface = oc_surface_create_remote(800, 600, OC_GLES);
    oc_surface_id connectionID = oc_surface_remote_id(surface);

    oc_surface_select(surface);

    //NOTE: init shader and gl state
    oc_surface_select(surface);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);

    GLfloat vertices[] = {
        -0.866 / 2, -0.5 / 2, 0, 0.866 / 2, -0.5 / 2, 0, 0, 0.5, 0
    };

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
        oc_log_error("link error: %.*s\n", size, buffer);
    }

    glUseProgram(program);

    //NOTE: send context id to parent
    write(writeFd, &connectionID, sizeof(connectionID));

    //NOTE: render loop
    while(!oc_should_quit())
    {
        oc_pump_events(0);
        oc_event* event = 0;
        while((event = oc_next_event(oc_scratch())) != 0)
        {
            switch(event->type)
            {
                case OC_EVENT_WINDOW_CLOSE:
                {
                    oc_request_quit();
                }
                break;

                default:
                    break;
            }
        }

        oc_surface_select(surface);

        oc_vec2 size = oc_surface_get_size(surface);
        oc_vec2 scaling = oc_surface_contents_scaling(surface);

        glViewport(0, 0, size.x * scaling.x, size.y * scaling.y);
        glClearColor(0.3, 0.3, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        static float alpha = 0;
        //f32 aspect = frameSize.x/frameSize.y;
        f32 aspect = 800 / (f32)600;

        GLfloat matrix[] = { cosf(alpha) / aspect, sinf(alpha), 0, 0,
                             -sinf(alpha) / aspect, cosf(alpha), 0, 0,
                             0, 0, 1, 0,
                             0, 0, 0, 1 };

        alpha += 2 * M_PI / 120;

        glUniformMatrix4fv(0, 1, false, matrix);

        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        oc_surface_present(surface);
    }

    oc_terminate();

    return (0);
}

int main(int argc, char** argv)
{
    if(argc > 1)
    {
        if(!strcmp(argv[1], "--child"))
        {
            int writeFd = atoi(argv[2]);
            oc_log_info("child process created with file desc %i\n", writeFd);
            return (child_main(writeFd));
        }
        else
        {
            return (-1);
        }
    }
    //	setvbuf( stdout, NULL, _IONBF, 0 );
    oc_init();

    //NOTE: create main window
    oc_rect rect = { .x = 100, .y = 100, .w = 800, .h = 600 };
    oc_window window = oc_window_create(rect, OC_STR8("test"), 0);

    //NOTE: create surface client
    oc_surface surface = oc_surface_create_host(window);

    //NOTE setup descriptors
    int fileDesc[2];
    pipe(fileDesc);

    oc_log_info("parent process created readFd %i and writeFd %i\n", fileDesc[0], fileDesc[1]);

    char writeDescStr[64];
    snprintf(writeDescStr, 64, "%i", fileDesc[1]);
    char* args[] = { "bin/example_surface_sharing", "--child", writeDescStr, 0 };

    process_id child = spawn_child(args[0], args);

    //NOTE: read the connection id
    oc_surface_id connectionID = 0;
    read(fileDesc[0], &connectionID, sizeof(connectionID));
    oc_log_info("received child connection id %llu\n", connectionID);

    //NOTE: connect the client
    oc_surface_host_connect(surface, connectionID);

    //NOTE: show the window
    oc_window_bring_to_front(window);

    while(!oc_should_quit())
    {
        oc_pump_events(0);
        oc_event* event = 0;
        while((event = oc_next_event(oc_scratch())) != 0)
        {
            switch(event->type)
            {
                case OC_EVENT_WINDOW_CLOSE:
                {
                    oc_request_quit();
                }
                break;

                default:
                    break;
            }

            oc_arena_clear(oc_scratch());
        }
        oc_surface_select(surface);
        oc_surface_present(surface);
    }

    terminate_child(child);

    oc_terminate();
    return (0);
}
