/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "glsl_shaders.h"
#include "math.h"
#include "orca.h"

//----------------------------------------------------------------
//NOTE(martin): GL vertex struct and identifiers
//----------------------------------------------------------------
typedef struct Vertex
{
    float x, y;
} Vertex;

typedef struct advect_program
{
    GLuint prog;

    GLint pos;
    GLint src;
    GLint velocity;
    GLint delta;
    GLint dissipation;

} advect_program;

typedef struct div_program
{
    GLuint prog;
    GLint pos;
    GLint src;

} div_program;

typedef struct jacobi_program
{
    GLuint prog;
    GLint pos;
    GLint xTex;
    GLint bTex;

} jacobi_program;

typedef struct blit_residue_program
{
    GLuint prog;

    GLint pos;
    GLint mvp;
    GLint xTex;
    GLint bTex;
} blit_residue_program;

typedef struct multigrid_restrict_residual_program
{
    GLuint prog;
    GLint pos;
    GLint xTex;
    GLint bTex;

} multigrid_restrict_residual_program;

typedef struct multigrid_correct_program
{
    GLuint prog;
    GLint pos;
    GLint src;
    GLint error;
    GLint invGridSize;

} multigrid_correct_program;

typedef struct subtract_program
{
    GLuint prog;

    GLint pos;
    GLint src;
    GLint pressure;
    GLint invGridSize;

} subtract_program;

typedef struct blit_program
{
    GLuint prog;

    GLint pos;
    GLint mvp;
    GLint gridSize;
    GLint tex;
} blit_program;

typedef struct splat_program
{
    GLuint prog;

    GLint pos;
    GLint src;
    GLint splatPos;
    GLint splatColor;
    GLint radius;
    GLint additive;
    GLint blending;
    GLint randomize;

} splat_program;

typedef struct frame_buffer
{
    GLuint textures[2];
    GLuint fbos[2];
} frame_buffer;

advect_program advectProgram;
div_program divProgram;
jacobi_program jacobiProgram;
multigrid_restrict_residual_program multigridRestrictResidualProgram;
multigrid_correct_program multigridCorrectProgram;

subtract_program subtractProgram;
splat_program splatProgram;
blit_program blitProgram;
blit_program blitDivProgram;
blit_residue_program blitResidueProgram;

frame_buffer colorBuffer;
frame_buffer velocityBuffer;

const int MULTIGRID_COUNT = 4;
frame_buffer pressureBuffer[4];
frame_buffer divBuffer[4];

GLuint vertexBuffer;

oc_surface surface;

//----------------------------------------------------------------
//NOTE(martin): initialization
//----------------------------------------------------------------

GLuint compile_shader(const char* vs, const char* fs)
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vs, 0);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fs, 0);
    glCompileShader(fragmentShader);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vertexShader);
    glAttachShader(prog, fragmentShader);
    glLinkProgram(prog);

    int status = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if(status != GL_TRUE)
    {
        oc_log_error("program failed to link: ");
        int logSize = 0;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logSize);

        oc_arena_scope scratch = oc_scratch_begin();
        char* log = oc_arena_push(scratch.arena, logSize);

        glGetProgramInfoLog(prog, logSize, 0, log);
        oc_log_error("%s\n", log);

        oc_scratch_end(scratch);
    }

    int err = glGetError();
    if(err)
    {
        oc_log_error("gl error %i\n", err);
    }

    return (prog);
}

void init_advect(advect_program* program)
{
    oc_log_info("compiling advect...");
    program->prog = compile_shader(glsl_common_vertex, glsl_advect);
    program->pos = glGetAttribLocation(program->prog, "pos");
    program->src = glGetUniformLocation(program->prog, "src");
    program->velocity = glGetUniformLocation(program->prog, "velocity");
    program->delta = glGetUniformLocation(program->prog, "delta");
    program->dissipation = glGetUniformLocation(program->prog, "dissipation");
}

void init_div(div_program* program)
{
    oc_log_info("compiling div...");
    program->prog = compile_shader(glsl_common_vertex, glsl_divergence);
    program->pos = glGetAttribLocation(program->prog, "pos");
    program->src = glGetUniformLocation(program->prog, "src");
}

void init_jacobi(jacobi_program* program)
{
    oc_log_info("compiling jacobi...");
    program->prog = compile_shader(glsl_common_vertex, glsl_jacobi_step);
    program->pos = glGetAttribLocation(program->prog, "pos");
    program->xTex = glGetUniformLocation(program->prog, "xTex");
    program->bTex = glGetUniformLocation(program->prog, "bTex");
}

void init_multigrid_restrict_residual(multigrid_restrict_residual_program* program)
{
    oc_log_info("compiling multigrid restrict residual...");
    program->prog = compile_shader(glsl_common_vertex, glsl_multigrid_restrict_residual);
    program->pos = glGetAttribLocation(program->prog, "pos");
    program->xTex = glGetUniformLocation(program->prog, "xTex");
    program->bTex = glGetUniformLocation(program->prog, "bTex");
}

void init_multigrid_correct(multigrid_correct_program* program)
{
    oc_log_info("compiling multigrid correct...");
    program->prog = compile_shader(glsl_common_vertex, glsl_multigrid_correct);
    program->pos = glGetAttribLocation(program->prog, "pos");
    program->src = glGetUniformLocation(program->prog, "src");
    program->error = glGetUniformLocation(program->prog, "error");
    program->invGridSize = glGetUniformLocation(program->prog, "invGridSize");
}

void init_subtract(subtract_program* program)
{
    oc_log_info("compiling subtract...");
    program->prog = compile_shader(glsl_common_vertex, glsl_subtract_pressure);
    program->pos = glGetAttribLocation(program->prog, "pos");
    program->src = glGetUniformLocation(program->prog, "src");
    program->pressure = glGetUniformLocation(program->prog, "pressure");
    program->invGridSize = glGetUniformLocation(program->prog, "invGridSize");
}

void init_splat(splat_program* program)
{
    oc_log_info("compiling splat...");
    program->prog = compile_shader(glsl_common_vertex, glsl_splat);
    program->pos = glGetAttribLocation(program->prog, "pos");
    program->src = glGetUniformLocation(program->prog, "src");
    program->splatPos = glGetUniformLocation(program->prog, "splatPos");
    program->splatColor = glGetUniformLocation(program->prog, "splatColor");
    program->radius = glGetUniformLocation(program->prog, "radius");
    program->additive = glGetUniformLocation(program->prog, "additive");
    program->blending = glGetUniformLocation(program->prog, "blending");
    program->randomize = glGetUniformLocation(program->prog, "randomize");
}

void init_blit(blit_program* program)
{
    oc_log_info("compiling blit...");
    program->prog = compile_shader(glsl_blit_vertex, glsl_blit_fragment);
    program->pos = glGetAttribLocation(program->prog, "pos");
    program->mvp = glGetUniformLocation(program->prog, "mvp");
    program->tex = glGetUniformLocation(program->prog, "tex");
    program->gridSize = glGetUniformLocation(program->prog, "gridSize");
}

void init_blit_div(blit_program* program)
{
    oc_log_info("compiling blit div...");
    program->prog = compile_shader(glsl_blit_div_vertex, glsl_blit_div_fragment);
    program->pos = glGetAttribLocation(program->prog, "pos");
    program->mvp = glGetUniformLocation(program->prog, "mvp");
    program->tex = glGetUniformLocation(program->prog, "tex");
}

void init_blit_residue(blit_residue_program* program)
{
    oc_log_info("compiling blit residue...");
    program->prog = compile_shader(glsl_blit_div_vertex, glsl_blit_residue_fragment);
    program->pos = glGetAttribLocation(program->prog, "pos");
    program->mvp = glGetUniformLocation(program->prog, "mvp");
    program->xTex = glGetUniformLocation(program->prog, "xTex");
    program->bTex = glGetUniformLocation(program->prog, "bTex");
}

GLuint create_texture(int width, int height, GLenum internalFormat, GLenum format, GLenum type, char* initData)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, initData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return (texture);
}

GLuint create_fbo(GLuint texture)
{
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindTexture(GL_TEXTURE_2D, texture);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    return (fbo);
}

void init_frame_buffer(frame_buffer* framebuffer,
                       int width,
                       int height,
                       GLenum internalFormat,
                       GLenum format,
                       GLenum type,
                       char* initData)
{
    for(int i = 0; i < 2; i++)
    {
        framebuffer->textures[i] = create_texture(width, height, internalFormat, format, type, initData);
        framebuffer->fbos[i] = create_fbo(framebuffer->textures[i]);
    }

    GLenum err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(err != GL_FRAMEBUFFER_COMPLETE)
    {
        oc_log_info("Frame buffer incomplete, %i", err);
    }
}

void frame_buffer_swap(frame_buffer* buffer)
{
    GLuint tmp = buffer->fbos[0];
    buffer->fbos[0] = buffer->fbos[1];
    buffer->fbos[1] = tmp;

    tmp = buffer->textures[0];
    buffer->textures[0] = buffer->textures[1];
    buffer->textures[1] = tmp;
}

//----------------------------------------------------------------
//NOTE(martin): entry point
//----------------------------------------------------------------

#define texWidth (256)
#define texHeight (256)

float colorInitData[texWidth][texHeight][4] = { 0 };
float velocityInitData[texWidth][texHeight][4] = { 0 };

const float EPSILON = 1.,
            INV_GRID_SIZE = 1. / (float)texWidth,
            DELTA = 1. / 120.;

const GLenum TEX_INTERNAL_FORMAT = GL_RGBA32F;
const GLenum TEX_FORMAT = GL_RGBA;
const GLenum TEX_TYPE = GL_FLOAT;

#define square(x) ((x) * (x))

/*
void reset_texture(GLuint texture, float width, float height, char* initData)
{
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, TEX_INTERNAL_FORMAT, width, height, 0, TEX_FORMAT, TEX_TYPE, initData);
}

static bool resetCmd = false;

void reset()
{
//	resetCmd = true;
	oc_log_info("reset");

	reset_texture(colorBuffer.textures[0], texWidth, texHeight, (char*)colorInitData);
	reset_texture(colorBuffer.textures[1], texWidth, texHeight, (char*)colorInitData);
	reset_texture(velocityBuffer.textures[0], texWidth, texHeight, (char*)velocityInitData);
	reset_texture(velocityBuffer.textures[1], texWidth, texHeight, (char*)velocityInitData);

	int gridFactor = 1;
	for(int i=0; i<MULTIGRID_COUNT; i++)
	{
		reset_texture(pressureBuffer[i].textures[0], texWidth/gridFactor, texHeight/gridFactor, 0);
		reset_texture(pressureBuffer[i].textures[1], texWidth/gridFactor, texHeight/gridFactor, 0);

		gridFactor *= 2;
	}
}

*/

typedef struct mouse_input
{
    float x;
    float y;
    float deltaX;
    float deltaY;
    bool down;

} mouse_input;

mouse_input mouseInput = { 0 };

int frameWidth = 800;
int frameHeight = 600;

ORCA_EXPORT void oc_on_mouse_down(int button)
{
    mouseInput.down = true;
}

ORCA_EXPORT void oc_on_mouse_up(int button)
{
    mouseInput.down = false;
}

ORCA_EXPORT void oc_on_mouse_move(float x, float y, float dx, float dy)
{
    mouseInput.x = x;
    mouseInput.y = y;
    mouseInput.deltaX = dx;
    mouseInput.deltaY = dy;
}

void init_color_checker()
{
    for(int i = 0; i < texHeight; i++)
    {
        for(int j = 0; j < texWidth; j++)
        {
            float u = j / (float)texWidth;
            float v = i / (float)texWidth;
            float value = ((int)(u * 10) % 2) == ((int)(v * 10) % 2) ? 1. : 0.;

            for(int k = 0; k < 3; k++)
            {
                colorInitData[i][j][k] = value;
            }
            colorInitData[i][j][3] = 1;
        }
    }
}

void init_velocity_vortex()
{
    for(int i = 0; i < texHeight; i++)
    {
        for(int j = 0; j < texWidth; j++)
        {
            float x = 2 * j / (float)texWidth - 1;
            float y = 2 * i / (float)texWidth - 1;
            velocityInitData[i][j][0] = sinf(2 * M_PI * y);
            velocityInitData[i][j][1] = sinf(2 * M_PI * x);
        }
    }
}

void apply_splat(float splatPosX, float splatPosY, float radius, float splatVelX, float splatVelY, float r, float g, float b, bool randomize)
{
    glUseProgram(splatProgram.prog);

    if(randomize)
    {
        glUniform1f(splatProgram.randomize, 1.);
    }
    else
    {
        glUniform1f(splatProgram.randomize, 0.);
    }

    // force
    glBindFramebuffer(GL_FRAMEBUFFER, velocityBuffer.fbos[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, velocityBuffer.textures[0]);
    glUniform1i(splatProgram.src, 0);

    glUniform2f(splatProgram.splatPos, splatPosX, splatPosY);
    glUniform3f(splatProgram.splatColor, splatVelX, splatVelY, 0);
    glUniform1f(splatProgram.additive, 1);
    glUniform1f(splatProgram.blending, 0);

    glUniform1f(splatProgram.radius, radius);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    frame_buffer_swap(&velocityBuffer);

    // dye
    glBindFramebuffer(GL_FRAMEBUFFER, colorBuffer.fbos[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBuffer.textures[0]);
    glUniform1i(splatProgram.src, 0);

    glUniform2f(splatProgram.splatPos, splatPosX, splatPosY);
    glUniform3f(splatProgram.splatColor, r, g, b);
    glUniform1f(splatProgram.additive, 0);
    glUniform1f(splatProgram.blending, 1);
    glUniform1f(splatProgram.radius, radius);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    frame_buffer_swap(&colorBuffer);
}

void jacobi_solve(frame_buffer* x, frame_buffer* b, float invGridSize, int iterationCount)
{
    glUseProgram(jacobiProgram.prog);

    for(int i = 0; i < iterationCount; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, x->fbos[1]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, x->textures[0]);
        glUniform1i(jacobiProgram.xTex, 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, b->textures[0]);
        glUniform1i(jacobiProgram.bTex, 1);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        frame_buffer_swap(x);
    }
}

void multigrid_coarsen_residual(frame_buffer* output, frame_buffer* x, frame_buffer* b, float invFineGridSize)
{
    //NOTE: compute residual and downsample to coarser grid, put result in coarser buffer
    glUseProgram(multigridRestrictResidualProgram.prog);
    glBindFramebuffer(GL_FRAMEBUFFER, output->fbos[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, x->textures[0]);
    glUniform1i(multigridRestrictResidualProgram.xTex, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, b->textures[0]);
    glUniform1i(multigridRestrictResidualProgram.bTex, 1);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    frame_buffer_swap(output);
}

void multigrid_prolongate_and_correct(frame_buffer* x, frame_buffer* error, float invFineGridSize)
{
    //NOTE: correct finer pressure
    glUseProgram(multigridCorrectProgram.prog);
    glBindFramebuffer(GL_FRAMEBUFFER, x->fbos[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, x->textures[0]);
    glUniform1i(multigridCorrectProgram.src, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, error->textures[0]);
    glUniform1i(multigridCorrectProgram.error, 1);

    glUniform1f(multigridCorrectProgram.invGridSize, invFineGridSize);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    frame_buffer_swap(x);
}

void multigrid_clear(frame_buffer* error)
{
    glBindFramebuffer(GL_FRAMEBUFFER, error->fbos[0]);
    glClear(GL_COLOR_BUFFER_BIT);
}

void input_splat(float t)
{
    //NOTE: apply force and dye
    if(mouseInput.down && (mouseInput.deltaX || mouseInput.deltaY))
    {
        oc_vec2 scaling = oc_surface_contents_scaling(surface);
        // account for margin
        float margin = 32;

        float offset = margin / texWidth;
        float ratio = 1 - 2 * margin / texWidth;

        float splatPosX = (mouseInput.x * scaling.x / frameWidth) * ratio + offset;
        float splatPosY = (1 - mouseInput.y * scaling.y / frameHeight) * ratio + offset;

        float splatVelX = (10000. * DELTA * mouseInput.deltaX * scaling.x / frameWidth) * ratio;
        float splatVelY = (-10000. * DELTA * mouseInput.deltaY * scaling.y / frameWidth) * ratio;

        float intensity = 100 * sqrtf(square(ratio * mouseInput.deltaX * scaling.x / frameWidth) + square(ratio * mouseInput.deltaY * scaling.y / frameHeight));

        float r = intensity * (sinf(2 * M_PI * 0.1 * t) + 1);
        float g = 0.5 * intensity * (cosf(2 * M_PI * 0.1 / M_E * t + 654) + 1);
        float b = intensity * (sinf(2 * M_PI * 0.1 / M_SQRT2 * t + 937) + 1);

        float radius = 0.005;

        apply_splat(splatPosX, splatPosY, radius, splatVelX, splatVelY, r, g, b, false);

        mouseInput.deltaX = 0;
        mouseInput.deltaY = 0;
    }
}

float testDiv[texWidth / 2][texWidth / 2][4];

ORCA_EXPORT void oc_on_init()
{
    oc_log_info("Hello, world (from C)");

    oc_window_set_title(OC_STR8("fluid"));

    surface = oc_surface_gles();
    oc_surface_select(surface);

    //	init_color_checker();
    //	init_velocity_vortex();

    // init programs
    init_advect(&advectProgram);
    init_div(&divProgram);
    init_jacobi(&jacobiProgram);
    init_multigrid_restrict_residual(&multigridRestrictResidualProgram);
    init_multigrid_correct(&multigridCorrectProgram);
    init_blit_residue(&blitResidueProgram);

    init_subtract(&subtractProgram);
    init_splat(&splatProgram);
    init_blit(&blitProgram);
    init_blit_div(&blitDivProgram);

    // init frame buffers
    oc_log_info("create color buffer");
    init_frame_buffer(&colorBuffer, texWidth, texHeight, TEX_INTERNAL_FORMAT, TEX_FORMAT, TEX_TYPE, (char*)colorInitData);
    oc_log_info("create velocity buffer");
    init_frame_buffer(&velocityBuffer, texWidth, texHeight, TEX_INTERNAL_FORMAT, TEX_FORMAT, TEX_TYPE, (char*)velocityInitData);

    int gridFactor = 1;
    for(int i = 0; i < MULTIGRID_COUNT; i++)
    {
        oc_log_info("create div buffer %i", i);
        init_frame_buffer(&divBuffer[i], texWidth / gridFactor, texHeight / gridFactor, TEX_INTERNAL_FORMAT, TEX_FORMAT, TEX_TYPE, 0);
        oc_log_info("create pressure buffer %i", i);
        init_frame_buffer(&pressureBuffer[i], texWidth / gridFactor, texHeight / gridFactor, TEX_INTERNAL_FORMAT, TEX_FORMAT, TEX_TYPE, 0);
        gridFactor *= 2;
    }

    // init vertex buffer
    static Vertex vertices[6] = {
        { -1, -1 },
        { 1, -1 },
        { 1, 1 },
        { -1, -1 },
        { 1, 1 },
        { -1, 1 }
    };

    //WARN: we assume blitProgram.pos == advectProgram.pos, is there a situation where it wouldn't be true??
    GLuint vertexBuffer = 0;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(Vertex), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(blitProgram.pos, 2, GL_FLOAT, GL_FALSE, 0, 0);

    for(int i = 0; i < texWidth / 2; i++)
    {
        for(int j = 0; j < texHeight / 2; j++)
        {
            testDiv[i][j][0] = 0.5 + 0.5 * cosf(j / 100. * 3.14159 + i / 100. * 1.2139);
        }
    }
}

ORCA_EXPORT void oc_on_resize(u32 width, u32 height)
{
    oc_vec2 scaling = oc_surface_contents_scaling(surface);
    frameWidth = width * scaling.x;
    frameHeight = height * scaling.y;
}

ORCA_EXPORT void oc_on_frame_refresh()
{
    float aspectRatio = texWidth / texHeight;

    static float t = 0;
    t += 1. / 60.;

    oc_surface_select(surface);

    glViewport(0, 0, texWidth, texHeight);

    //NOTE: advect velocity thru itself
    glUseProgram(advectProgram.prog);
    glBindFramebuffer(GL_FRAMEBUFFER, velocityBuffer.fbos[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, velocityBuffer.textures[0]);
    glUniform1i(advectProgram.src, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, velocityBuffer.textures[0]);
    glUniform1i(advectProgram.velocity, 1);

    glUniform1f(advectProgram.delta, DELTA);
    glUniform1f(advectProgram.dissipation, 0.01);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    frame_buffer_swap(&velocityBuffer);

    input_splat(t);

    //NOTE: compute divergence of advected velocity
    glUseProgram(divProgram.prog);
    glBindFramebuffer(GL_FRAMEBUFFER, divBuffer[0].fbos[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, velocityBuffer.textures[0]);
    glUniform1i(divProgram.src, 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    frame_buffer_swap(&divBuffer[0]);

    //NOTE: compute pressure
    glBindFramebuffer(GL_FRAMEBUFFER, pressureBuffer[0].fbos[1]);
    glClear(GL_COLOR_BUFFER_BIT);

#if 0
		multigrid_clear(&pressureBuffer[0]);
		jacobi_solve(&pressureBuffer[0], &divBuffer[0], INV_GRID_SIZE, texWidth*texHeight);
#else
    multigrid_clear(&pressureBuffer[0]);

    for(int i = 0; i < 1; i++)
    {
        jacobi_solve(&pressureBuffer[0], &divBuffer[0], INV_GRID_SIZE, 2);
        multigrid_coarsen_residual(&divBuffer[1], &pressureBuffer[0], &divBuffer[0], INV_GRID_SIZE);

        multigrid_clear(&pressureBuffer[1]);
        jacobi_solve(&pressureBuffer[1], &divBuffer[1], 2 * INV_GRID_SIZE, 2);
        multigrid_coarsen_residual(&divBuffer[2], &pressureBuffer[1], &divBuffer[1], 2 * INV_GRID_SIZE);

        multigrid_clear(&pressureBuffer[2]);
        jacobi_solve(&pressureBuffer[2], &divBuffer[2], 4 * INV_GRID_SIZE, 30);

        multigrid_prolongate_and_correct(&pressureBuffer[1], &pressureBuffer[2], 2 * INV_GRID_SIZE);
        jacobi_solve(&pressureBuffer[1], &divBuffer[1], 2 * INV_GRID_SIZE, 8);

        multigrid_prolongate_and_correct(&pressureBuffer[0], &pressureBuffer[1], INV_GRID_SIZE);
        jacobi_solve(&pressureBuffer[0], &divBuffer[0], INV_GRID_SIZE, 4);
    }
#endif

    //NOTE: subtract pressure gradient to advected velocity
    glUseProgram(subtractProgram.prog);
    glBindFramebuffer(GL_FRAMEBUFFER, velocityBuffer.fbos[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, velocityBuffer.textures[0]);
    glUniform1i(subtractProgram.src, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pressureBuffer[0].textures[0]);
    glUniform1i(subtractProgram.pressure, 1);

    glUniform1f(subtractProgram.invGridSize, INV_GRID_SIZE);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    frame_buffer_swap(&velocityBuffer);

    //NOTE: Advect color through corrected velocity field
    glUseProgram(advectProgram.prog);
    glBindFramebuffer(GL_FRAMEBUFFER, colorBuffer.fbos[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBuffer.textures[0]);
    glUniform1i(advectProgram.src, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, velocityBuffer.textures[0]);
    glUniform1i(advectProgram.velocity, 1);

    glUniform1f(advectProgram.delta, DELTA);

    glUniform1f(advectProgram.dissipation, 0.001);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    frame_buffer_swap(&colorBuffer);

    //NOTE: Blit color texture to screen

    glViewport(0, 0, frameWidth, frameHeight);

    float displayMatrix[16] = {
        1 / aspectRatio, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    glUseProgram(blitProgram.prog);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBuffer.textures[0]);
    glUniform1i(blitProgram.tex, 0);

    glUniform2i(blitProgram.gridSize, texWidth, texHeight);

    glUniformMatrix4fv(blitProgram.mvp, 1, GL_FALSE, displayMatrix);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    oc_surface_present(surface);
}
