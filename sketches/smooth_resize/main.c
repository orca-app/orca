/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _USE_MATH_DEFINES //NOTE: necessary for MSVC
#include <math.h>

#define OC_INCLUDE_GL_API
#include "orca.h"

unsigned int program;

const char* vshaderSource =
    "#version 430\n"
    "in vec4 vPosition;\n"
    "uniform mat4 transform;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = transform*vPosition;\n"
    "}\n";

const char* fshaderSource =
    "#version 430\n"
    "layout(location = 0) out vec4 diffuse;"
    "precision mediump float;\n"
    "void main()\n"
    "{\n"
    "    diffuse = vec4(1.0, 0.0, 0.0, 1.0);\n"
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

GLfloat vertices[] = {
    -0.866 / 2, -0.5 / 2, 0, 0.866 / 2, -0.5 / 2, 0, 0, 0.5, 0
};

typedef struct app_data
{
    oc_window window;
    oc_surface surface;
    oc_canvas canvas;
    oc_font font;

    GLuint vertexBuffer;
} app_data;

void process_event(app_data* app, oc_event event)
{
    switch(event.type)
    {
        case OC_EVENT_WINDOW_CLOSE:
        {
            oc_request_quit();
        }
        break;

        case OC_EVENT_WINDOW_RESIZE:
        {
            oc_log_info("resizing window!\n");
        }
        break;

        default:
            break;
    }
}

void update_and_render(app_data* app)
{
    oc_surface_select(app->surface);

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

    glBindBuffer(GL_ARRAY_BUFFER, app->vertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    oc_surface_present(app->surface);
}

i32 render(void* user)
{
    app_data* app = (app_data*)user;

    //NOTE: init shader and gl state
    oc_surface_select(app->surface);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &app->vertexBuffer);

    GLfloat vertices[] = {
        -0.866 / 2, -0.5 / 2, 0, 0.866 / 2, -0.5 / 2, 0, 0, 0.5, 0
    };

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
        oc_log_error("link error: %.*s\n", size, buffer);
    }

    glUseProgram(program);

    while(!oc_should_quit())
    {
        oc_arena_scope scratch = oc_scratch_begin();

        oc_event* event = 0;

        while((event = oc_next_event(scratch.arena)) != 0)
        {
            process_event(app, *event);
        }
        update_and_render(app);
        oc_scratch_end(scratch);
    }

    return (0);
}

int main()
{
    oc_init();

    oc_rect windowRect = { .x = 100, .y = 100, .w = 810, .h = 610 };
    oc_window window = oc_window_create(windowRect, OC_STR8("test"), 0);

    //NOTE: create surface
    oc_surface surface = oc_surface_create_for_window(window, OC_GL);
    if(oc_surface_is_nil(surface))
    {
        oc_log_error("Error: couldn't create surface\n");
        return (-1);
    }

    oc_surface_swap_interval(surface, 1);
    oc_surface_deselect();

    // start app
    oc_window_bring_to_front(window);
    oc_window_focus(window);

    //TODO: start thread
    app_data app = { .window = window,
                     .surface = surface };

    oc_thread* renderThread = oc_thread_create(render, &app);

    while(!oc_should_quit())
    {
        oc_pump_events(0);
    }

    oc_thread_join(renderThread, NULL);

    oc_surface_destroy(surface);
    oc_window_destroy(window);

    oc_terminate();

    return (0);
}
