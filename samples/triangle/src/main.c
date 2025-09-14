/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include <math.h>

#include <orca.h>

oc_vec2 frameSize = { 100, 100 };

oc_surface surface;

unsigned int program;

const char* vshaderSource =
    "attribute vec4 vPosition;\n"
    "uniform mat4 transform;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = transform*vPosition;\n"
    "}\n";

const char* fshaderSource =
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
        oc_log_info("gl error");
    }
}

ORCA_EXPORT void oc_on_init(void)
{
    oc_window_set_title(OC_STR8("triangle"));

    surface = oc_gles_surface_create();
    oc_gles_surface_make_current(surface);

    /*
    const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
    oc_log_info("GLES extensions: %s\n", extensions);

    int extensionCount = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &extensionCount);
    for(int i = 0; i < extensionCount; i++)
    {
        const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
        oc_log_info("GLES extension %i: %s\n", i, extension);
    }
    */

    unsigned int vshader = glCreateShader(GL_VERTEX_SHADER);
    unsigned int fshader = glCreateShader(GL_FRAGMENT_SHADER);
    program = glCreateProgram();

    compile_shader(vshader, vshaderSource);
    compile_shader(fshader, fshaderSource);

    glAttachShader(program, vshader);
    glAttachShader(program, fshader);
    glLinkProgram(program);
    glUseProgram(program);

    GLfloat vertices[] = {
        -0.866 / 2, -0.5 / 2, 0, 0.866 / 2, -0.5 / 2, 0, 0, 0.5, 0
    };

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
}

ORCA_EXPORT void oc_on_resize(u32 width, u32 height)
{
    oc_log_info("frame resize %u, %u", width, height);
    frameSize.x = width;
    frameSize.y = height;
}

ORCA_EXPORT void oc_on_frame_refresh(void)
{
    f32 aspect = frameSize.x / frameSize.y;

    oc_gles_surface_make_current(surface);

    glClearColor(0, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    static float alpha = 0;

    oc_vec2 scaling = oc_surface_contents_scaling(surface);

    oc_vec2* scalingPtr = &scaling;

    glViewport(0, 0, frameSize.x * scaling.x, frameSize.y * scaling.y);

    GLfloat matrix[] = { cosf(alpha) / aspect, sinf(alpha), 0, 0,
                         -sinf(alpha) / aspect, cosf(alpha), 0, 0,
                         0, 0, 1, 0,
                         0, 0, 0, 1 };

    {
        f32 scaling = 1;
        alpha += 2 * M_PI / 120 * scaling;
    }

    glUniformMatrix4fv(0, 1, false, matrix);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    oc_gles_surface_swap_buffers(surface);
}
