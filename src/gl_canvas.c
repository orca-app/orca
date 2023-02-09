/************************************************************//**
*
*	@file: gl_canvas.c
*	@author: Martin Fouilleul
*	@date: 29/01/2023
*	@revision:
*
*****************************************************************/
#include"graphics_internal.h"
#include"macro_helpers.h"
#include"glsl_shaders.h"

#define LOG_SUBSYSTEM "Graphics"

typedef struct mg_gl_canvas_backend
{
	mg_canvas_backend interface;
	mg_surface surface;

	GLint dummyVertexBuffer;
	GLint vertexBuffer;
	GLint shapeBuffer;
	GLint indexBuffer;
	GLint tileCounterBuffer;
	GLint tileArrayBuffer;
	GLint clearCounterProgram;
	GLint tileProgram;
	GLint sortProgram;
	GLint drawProgram;
	GLint blitProgram;

	GLint outTexture;

	char* indexMapping;
	char* vertexMapping;
	char* shapeMapping;

} mg_gl_canvas_backend;

mg_gl_surface* mg_gl_canvas_get_surface(mg_gl_canvas_backend* canvas)
{
	mg_gl_surface* res = 0;
	mg_surface_data* data = mg_surface_data_from_handle(canvas->surface);
	if(data && data->backend == MG_BACKEND_GL)
	{
		res = (mg_gl_surface*)data;
	}
	return(res);
}

//NOTE: debugger
typedef struct debug_vertex
{
	vec4 cubic;
	vec2 pos;
	int shapeIndex;
	u8 pad[4];
} debug_vertex;

typedef struct debug_shape
{
	vec4 color;
	vec4 clip;
	vec2 uv;
	u8 pad[8];
} debug_shape;

#define LayoutNext(prevName, prevType, nextType) \
	AlignUpOnPow2(_cat3_(LAYOUT_, prevName, _OFFSET)+_cat3_(LAYOUT_, prevType, _SIZE), _cat3_(LAYOUT_, nextType, _ALIGN))

enum {
	LAYOUT_VEC2_SIZE = 8,
	LAYOUT_VEC2_ALIGN = 8,
	LAYOUT_VEC4_SIZE = 16,
	LAYOUT_VEC4_ALIGN = 16,
	LAYOUT_INT_SIZE = 4,
	LAYOUT_INT_ALIGN = 4,

	LAYOUT_CUBIC_OFFSET = 0,
	LAYOUT_POS_OFFSET = LayoutNext(CUBIC, VEC4, VEC2),
	LAYOUT_ZINDEX_OFFSET = LayoutNext(POS, VEC2, INT),
	LAYOUT_VERTEX_ALIGN = 16,
	LAYOUT_VERTEX_SIZE = LayoutNext(ZINDEX, INT, VERTEX),

	LAYOUT_COLOR_OFFSET = 0,
	LAYOUT_CLIP_OFFSET = LayoutNext(COLOR, VEC4, VEC4),
	LAYOUT_UV_OFFSET = LayoutNext(CLIP, VEC4, VEC2),
	LAYOUT_SHAPE_ALIGN = 16,
	LAYOUT_SHAPE_SIZE = LayoutNext(UV, VEC2, SHAPE)
};

enum {
	MG_GL_CANVAS_DEFAULT_BUFFER_LENGTH = 1<<20,
	MG_GL_CANVAS_VERTEX_BUFFER_SIZE = MG_GL_CANVAS_DEFAULT_BUFFER_LENGTH * LAYOUT_VERTEX_SIZE,
	MG_GL_CANVAS_SHAPE_BUFFER_SIZE = MG_GL_CANVAS_DEFAULT_BUFFER_LENGTH * LAYOUT_SHAPE_SIZE,
	MG_GL_CANVAS_INDEX_BUFFER_SIZE = MG_GL_CANVAS_DEFAULT_BUFFER_LENGTH * LAYOUT_INT_SIZE,
	MG_GL_CANVAS_TILE_COUNTER_BUFFER_LENGTH = 65536,
	MG_GL_CANVAS_TILE_COUNTER_BUFFER_SIZE = sizeof(int)*MG_GL_CANVAS_TILE_COUNTER_BUFFER_LENGTH,
	MG_GL_CANVAS_TILE_ARRAY_LENGTH = 1<<10,
	MG_GL_CANVAS_TILE_ARRAY_SIZE = sizeof(int)*MG_GL_CANVAS_TILE_ARRAY_LENGTH,
	MG_GL_CANVAS_TILE_ARRAY_BUFFER_SIZE = MG_GL_CANVAS_TILE_COUNTER_BUFFER_LENGTH * MG_GL_CANVAS_TILE_ARRAY_SIZE,
};

void mg_gl_canvas_update_vertex_layout(mg_gl_canvas_backend* backend)
{
	backend->interface.vertexLayout = (mg_vertex_layout){
		    .maxVertexCount = MG_GL_CANVAS_DEFAULT_BUFFER_LENGTH,
	        .maxIndexCount = MG_GL_CANVAS_DEFAULT_BUFFER_LENGTH,
	        .posBuffer = backend->vertexMapping + LAYOUT_POS_OFFSET,
	        .posStride = LAYOUT_VERTEX_SIZE,
	        .cubicBuffer = backend->vertexMapping + LAYOUT_CUBIC_OFFSET,
	        .cubicStride = LAYOUT_VERTEX_SIZE,
	        .shapeIndexBuffer = backend->vertexMapping + LAYOUT_ZINDEX_OFFSET,
	        .shapeIndexStride = LAYOUT_VERTEX_SIZE,

	        .colorBuffer = backend->shapeMapping + LAYOUT_COLOR_OFFSET,
	        .colorStride = LAYOUT_SHAPE_SIZE,
	        .clipBuffer = backend->shapeMapping + LAYOUT_CLIP_OFFSET,
	        .clipStride = LAYOUT_SHAPE_SIZE,
	        .uvBuffer = backend->shapeMapping + LAYOUT_UV_OFFSET,
	        .uvStride = LAYOUT_SHAPE_SIZE,

	        .indexBuffer = backend->indexMapping,
	        .indexStride = LAYOUT_INT_SIZE};
}

void mg_gl_send_buffers(mg_gl_canvas_backend* backend, int shapeCount, int vertexCount, int indexCount)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->vertexBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, LAYOUT_VERTEX_SIZE*vertexCount, backend->vertexMapping, GL_STREAM_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->shapeBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, LAYOUT_SHAPE_SIZE*shapeCount, backend->shapeMapping, GL_STREAM_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->indexBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, LAYOUT_INT_SIZE*indexCount, backend->indexMapping, GL_STREAM_DRAW);
}

void mg_gl_canvas_begin(mg_canvas_backend* interface)
{
	mg_gl_canvas_backend* backend = (mg_gl_canvas_backend*)interface;
	mg_gl_surface* surface = mg_gl_canvas_get_surface(backend);
	if(!surface)
	{
		return;
	}
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void mg_gl_canvas_end(mg_canvas_backend* interface)
{
	//NOTE: nothing to do here...
}

void mg_gl_canvas_clear(mg_canvas_backend* interface, mg_color clearColor)
{
	mg_gl_canvas_backend* backend = (mg_gl_canvas_backend*)interface;
	mg_gl_surface* surface = mg_gl_canvas_get_surface(backend);
	if(!surface)
	{
		return;
	}

	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT);
}

void mg_gl_canvas_draw_batch(mg_canvas_backend* interface, u32 shapeCount, u32 vertexCount, u32 indexCount)
{
	mg_gl_canvas_backend* backend = (mg_gl_canvas_backend*)interface;
	mg_gl_surface* surface = mg_gl_canvas_get_surface(backend);
	if(!surface)
	{
		return;
	}

/*NOTE: if we want debug_vertex while debugging, the following ensures the struct def doesn't get stripped away
	debug_vertex vertex;
	debug_shape shape;
	printf("foo %p, bar %p\n", &vertex, &shape);
//*/
	mg_gl_send_buffers(backend, shapeCount, vertexCount, indexCount);

	mp_rect frame = mg_surface_get_frame(backend->surface);
	vec2 contentsScaling = mg_surface_contents_scaling(backend->surface);

	const int tileSize = 16;
	const int tileCountX = (frame.w*contentsScaling.x + tileSize - 1)/tileSize;
	const int tileCountY = (frame.h*contentsScaling.y + tileSize - 1)/tileSize;
	const int tileArraySize = MG_GL_CANVAS_TILE_ARRAY_LENGTH;

	//TODO: ensure there's enough space in tile buffer

	//NOTE: first clear counters
	glUseProgram(backend->clearCounterProgram);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, backend->tileCounterBuffer);
	glDispatchCompute(tileCountX*tileCountY, 1, 1);

	//NOTE: we first distribute triangles into tiles:

	glUseProgram(backend->tileProgram);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, backend->vertexBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, backend->shapeBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, backend->indexBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, backend->tileCounterBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, backend->tileArrayBuffer);

	glUniform1ui(0, indexCount);
	glUniform2ui(1, tileCountX, tileCountY);
	glUniform1ui(2, tileSize);
	glUniform1ui(3, tileArraySize);
	glUniform2f(4, contentsScaling.x, contentsScaling.y);

	u32 threadCount = indexCount/3;
	glDispatchCompute((threadCount + 255)/256, 1, 1);

	//NOTE: next we sort triangles in each tile
	glUseProgram(backend->sortProgram);

	glUniform1ui(0, indexCount);
	glUniform2ui(1, tileCountX, tileCountY);
	glUniform1ui(2, tileSize);
	glUniform1ui(3, tileArraySize);

	glDispatchCompute(tileCountX * tileCountY, 1, 1);

	//NOTE: then we fire the drawing shader that will select only triangles in its tile
	glUseProgram(backend->drawProgram);

	glBindImageTexture(0, backend->outTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

	glUniform1ui(0, indexCount);
	glUniform2ui(1, tileCountX, tileCountY);
	glUniform1ui(2, tileSize);
	glUniform1ui(3, tileArraySize);
	glUniform2f(4, contentsScaling.x, contentsScaling.y);

	glDispatchCompute(tileCountX, tileCountY, 1);

	//NOTE: now blit out texture to surface
	glUseProgram(backend->blitProgram);
	glBindBuffer(GL_ARRAY_BUFFER, backend->dummyVertexBuffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, backend->outTexture);
	glUniform1i(0, 0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	mg_gl_canvas_update_vertex_layout(backend);
}

void mg_gl_canvas_destroy(mg_canvas_backend* interface)
{
	mg_gl_canvas_backend* backend = (mg_gl_canvas_backend*)interface;

	//TODO
}

void mg_gl_canvas_atlas_upload(mg_canvas_backend* interface, mp_rect rect, u8* bytes)
{
	//TODO
}

static int mg_gl_compile_shader(const char* name, GLuint shader, const char* source)
{
	int res = 0;

	const char* sources[3] = {"#version 430", glsl_common, source};

	glShaderSource(shader, 3, sources, 0);
	glCompileShader(shader);

	int status = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if(!status)
	{
		char buffer[256];
		int size = 0;
		glGetShaderInfoLog(shader, 256, &size, buffer);
		printf("Shader compile error (%s): %.*s\n", name, size, buffer);
		res = -1;
	}
	return(res);
}

static int mg_gl_canvas_compile_compute_program_named(const char* name, const char* source, GLuint* outProgram)
{
	int res = 0;
	*outProgram = 0;

	GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
	GLuint program = glCreateProgram();

	res |= mg_gl_compile_shader(name, shader, source);

	if(!res)
	{
		glAttachShader(program, shader);
		glLinkProgram(program);

		int status = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &status);
		if(!status)
		{
			char buffer[256];
			int size = 0;
			glGetProgramInfoLog(program, 256, &size, buffer);
			LOG_ERROR("Shader link error (%s): %.*s\n", name, size, buffer);

			res = -1;
		}
		else
		{
			*outProgram = program;
		}
	}
	return(res);
}

int mg_gl_canvas_compile_render_program_named(const char* progName,
                                              const char* vertexName,
                                              const char* fragmentName,
                                              const char* vertexSrc,
                                              const char* fragmentSrc,
                                              GLuint* outProgram)
{
	int res = 0;
	*outProgram = 0;

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint program = glCreateProgram();

	res |= mg_gl_compile_shader(vertexName, vertexShader, vertexSrc);
	res |= mg_gl_compile_shader(fragmentName, fragmentShader, fragmentSrc);

	if(!res)
	{
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
		glLinkProgram(program);

		int status = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &status);
		if(!status)
		{
			char buffer[256];
			int size = 0;
			glGetProgramInfoLog(program, 256, &size, buffer);
			LOG_ERROR("Shader link error (%s): %.*s\n", progName, size, buffer);
			res = -1;
 		}
 		else
 		{
			*outProgram = program;
 		}
 	}
 	return(res);
}

#define mg_gl_canvas_compile_compute_program(src, out) \
	mg_gl_canvas_compile_compute_program_named(#src, src, out)

#define mg_gl_canvas_compile_render_program(progName, shaderSrc, vertexSrc, out) \
	mg_gl_canvas_compile_render_program_named(progName, #shaderSrc, #vertexSrc, shaderSrc, vertexSrc, out)

mg_canvas_backend* mg_gl_canvas_create(mg_surface surface)
{
	mg_gl_canvas_backend* backend = 0;
	mg_surface_data* surfaceData = mg_surface_data_from_handle(surface);
	if(surfaceData && surfaceData->backend == MG_BACKEND_GL)
	{
		mg_gl_surface* glSurface = (mg_gl_surface*)surfaceData;

		backend = malloc_type(mg_gl_canvas_backend);
		memset(backend, 0, sizeof(mg_gl_canvas_backend));
		backend->surface = surface;

		//NOTE(martin): setup interface functions
		backend->interface.destroy = mg_gl_canvas_destroy;
		backend->interface.begin = mg_gl_canvas_begin;
		backend->interface.end = mg_gl_canvas_end;
		backend->interface.clear = mg_gl_canvas_clear;
		backend->interface.drawBatch = mg_gl_canvas_draw_batch;
		backend->interface.atlasUpload = mg_gl_canvas_atlas_upload;

		mg_surface_prepare(surface);

		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &backend->dummyVertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, backend->dummyVertexBuffer);

		glGenBuffers(1, &backend->vertexBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->vertexBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MG_GL_CANVAS_VERTEX_BUFFER_SIZE, 0, GL_STREAM_DRAW);

		glGenBuffers(1, &backend->shapeBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->shapeBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MG_GL_CANVAS_SHAPE_BUFFER_SIZE, 0, GL_STREAM_DRAW);

		glGenBuffers(1, &backend->indexBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->indexBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MG_GL_CANVAS_INDEX_BUFFER_SIZE, 0, GL_STREAM_DRAW);

		glGenBuffers(1, &backend->tileCounterBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->tileCounterBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MG_GL_CANVAS_TILE_COUNTER_BUFFER_SIZE, 0, GL_DYNAMIC_COPY);

		glGenBuffers(1, &backend->tileArrayBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->tileArrayBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MG_GL_CANVAS_TILE_ARRAY_BUFFER_SIZE, 0, GL_DYNAMIC_COPY);

		mp_rect frame = mg_surface_get_frame(backend->surface);
		vec2 contentsScaling = mg_surface_contents_scaling(backend->surface);

		glGenTextures(1, &backend->outTexture);
		glBindTexture(GL_TEXTURE_2D, backend->outTexture);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, frame.w*contentsScaling.x, frame.h*contentsScaling.y);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//NOTE: create programs
		mg_gl_canvas_compile_compute_program(glsl_clear_counters, &backend->clearCounterProgram);
		mg_gl_canvas_compile_compute_program(glsl_tile, &backend->tileProgram);
		mg_gl_canvas_compile_compute_program(glsl_sort, &backend->sortProgram);
		mg_gl_canvas_compile_compute_program(glsl_draw, &backend->drawProgram);
		mg_gl_canvas_compile_render_program("blit", glsl_blit_vertex, glsl_blit_fragment, &backend->blitProgram);

		backend->vertexMapping = malloc_array(char, 1<<30);
		backend->shapeMapping = malloc_array(char, 1<<30);
		backend->indexMapping = malloc_array(char, 1<<30);

		mg_gl_canvas_update_vertex_layout(backend);
	}

	return((mg_canvas_backend*)backend);
}


#undef LOG_SUBSYSTEM
