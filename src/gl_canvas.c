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
#include"gl_api.h"

typedef struct mg_gl_canvas_backend
{
	mg_canvas_backend interface;
	mg_surface surface;

	GLuint vao;
	GLuint dummyVertexBuffer;
	GLuint vertexBuffer;
	GLuint shapeBuffer;
	GLuint indexBuffer;
	GLuint tileCounterBuffer;
	GLuint tileArrayBuffer;
	GLuint clearCounterProgram;
	GLuint tileProgram;
	GLuint sortProgram;
	GLuint drawProgram;
	GLuint blitProgram;

	GLuint outTexture;

	char* indexMapping;
	char* vertexMapping;
	char* shapeMapping;

} mg_gl_canvas_backend;

typedef struct mg_gl_image
{
	mg_image_data interface;

	GLuint textureID;
} mg_gl_image;

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
	LAYOUT_MAT2x3_SIZE = sizeof(float)*6,
	LAYOUT_MAT2x3_ALIGN = 4,

	LAYOUT_CUBIC_OFFSET = 0,
	LAYOUT_POS_OFFSET = LayoutNext(CUBIC, VEC4, VEC2),
	LAYOUT_ZINDEX_OFFSET = LayoutNext(POS, VEC2, INT),
	LAYOUT_VERTEX_ALIGN = 16,
	LAYOUT_VERTEX_SIZE = LayoutNext(ZINDEX, INT, VERTEX),

	LAYOUT_COLOR_OFFSET = 0,
	LAYOUT_CLIP_OFFSET = LayoutNext(COLOR, VEC4, VEC4),
	LAYOUT_UV_TRANSFORM_OFFSET = LayoutNext(CLIP, VEC4, MAT2x3),
	LAYOUT_SHAPE_ALIGN = 16,
	LAYOUT_SHAPE_SIZE = LayoutNext(UV_TRANSFORM, MAT2x3, SHAPE),

	MG_GL_CANVAS_MAX_BUFFER_LENGTH = 1<<20,
	MG_GL_CANVAS_MAX_SHAPE_BUFFER_SIZE = LAYOUT_SHAPE_SIZE * MG_GL_CANVAS_MAX_BUFFER_LENGTH,
	MG_GL_CANVAS_MAX_VERTEX_BUFFER_SIZE = LAYOUT_VERTEX_SIZE * MG_GL_CANVAS_MAX_BUFFER_LENGTH,
	MG_GL_CANVAS_MAX_INDEX_BUFFER_SIZE = LAYOUT_INT_SIZE * MG_GL_CANVAS_MAX_BUFFER_LENGTH,

	//TODO: actually size this dynamically
	MG_GL_CANVAS_MAX_TILE_COUNT = 65536, //NOTE: this allows for 256*256 tiles (e.g. 4096*4096 pixels)
	MG_GL_CANVAS_TILE_COUNTER_BUFFER_SIZE = LAYOUT_INT_SIZE * MG_GL_CANVAS_MAX_TILE_COUNT,

	MG_GL_CANVAS_TILE_ARRAY_LENGTH = 1<<10, // max overlapping triangles per tiles
	MG_GL_CANVAS_TILE_ARRAY_BUFFER_SIZE = LAYOUT_INT_SIZE * MG_GL_CANVAS_MAX_TILE_COUNT * MG_GL_CANVAS_TILE_ARRAY_LENGTH,
};

void mg_gl_canvas_update_vertex_layout(mg_gl_canvas_backend* backend)
{
	backend->interface.vertexLayout = (mg_vertex_layout){
		    .maxVertexCount = MG_GL_CANVAS_MAX_BUFFER_LENGTH,
	        .maxIndexCount = MG_GL_CANVAS_MAX_BUFFER_LENGTH,
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
	        .uvTransformBuffer = backend->shapeMapping + LAYOUT_UV_TRANSFORM_OFFSET,
	        .uvTransformStride = LAYOUT_SHAPE_SIZE,

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
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

void mg_gl_canvas_end(mg_canvas_backend* interface)
{
	//NOTE: nothing to do here...
}

void mg_gl_canvas_clear(mg_canvas_backend* interface, mg_color clearColor)
{
	mg_gl_canvas_backend* backend = (mg_gl_canvas_backend*)interface;

	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT);
}

void mg_gl_canvas_draw_batch(mg_canvas_backend* interface, mg_image_data* imageInterface, u32 shapeCount, u32 vertexCount, u32 indexCount)
{
	mg_gl_canvas_backend* backend = (mg_gl_canvas_backend*)interface;

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
	const int tileArrayLength = MG_GL_CANVAS_TILE_ARRAY_LENGTH;

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
	glUniform1ui(3, tileArrayLength);
	glUniform2f(4, contentsScaling.x, contentsScaling.y);

	u32 threadCount = indexCount/3;
	glDispatchCompute((threadCount + 255)/256, 1, 1);

	//NOTE: next we sort triangles in each tile
	glUseProgram(backend->sortProgram);

	glUniform1ui(0, indexCount);
	glUniform2ui(1, tileCountX, tileCountY);
	glUniform1ui(2, tileSize);
	glUniform1ui(3, tileArrayLength);

	glDispatchCompute(tileCountX * tileCountY, 1, 1);

	//NOTE: then we fire the drawing shader that will select only triangles in its tile
	glUseProgram(backend->drawProgram);

	glBindImageTexture(0, backend->outTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

	glUniform1ui(0, indexCount);
	glUniform2ui(1, tileCountX, tileCountY);
	glUniform1ui(2, tileSize);
	glUniform1ui(3, tileArrayLength);
	glUniform2f(4, contentsScaling.x, contentsScaling.y);

	if(imageInterface)
	{
		//TODO: make sure this image belongs to that context
		mg_gl_image* image = (mg_gl_image*)imageInterface;
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, image->textureID);
		glUniform1ui(5, 1);
	}
	else
	{
		glUniform1ui(5, 0);
	}

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

	glDeleteTextures(1, &backend->outTexture);

	glDeleteBuffers(1, &backend->dummyVertexBuffer);
	glDeleteBuffers(1, &backend->vertexBuffer);
	glDeleteBuffers(1, &backend->shapeBuffer);
	glDeleteBuffers(1, &backend->indexBuffer);
	glDeleteBuffers(1, &backend->tileCounterBuffer);
	glDeleteBuffers(1, &backend->tileArrayBuffer);

	glDeleteVertexArrays(1, &backend->vao);

	free(backend->shapeMapping);
	free(backend->vertexMapping);
	free(backend->indexMapping);
	free(backend);
}

mg_image_data* mg_gl_canvas_image_create(mg_canvas_backend* interface, vec2 size)
{
	mg_gl_image* image = 0;

	image = malloc_type(mg_gl_image);
	if(image)
	{
		glGenTextures(1, &image->textureID);
		glBindTexture(GL_TEXTURE_2D, image->textureID);
//		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, size.x, size.y);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		image->interface.size = size;
	}
	return((mg_image_data*)image);
}

void mg_gl_canvas_image_destroy(mg_canvas_backend* interface, mg_image_data* imageInterface)
{
	//TODO: check that this image belongs to this context
	mg_gl_image* image = (mg_gl_image*)imageInterface;
	glDeleteTextures(1, &image->textureID);
	free(image);
}

void mg_gl_canvas_image_upload_region(mg_canvas_backend* interface,
                                      mg_image_data* imageInterface,
                                      mp_rect region,
                                      u8* pixels)
{
	//TODO: check that this image belongs to this context
	mg_gl_image* image = (mg_gl_image*)imageInterface;
	glBindTexture(GL_TEXTURE_2D, image->textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, region.w, region.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
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

	int err = 0;

	if(surfaceData && surfaceData->backend == MG_BACKEND_GL)
	{
		backend = malloc_type(mg_gl_canvas_backend);
		memset(backend, 0, sizeof(mg_gl_canvas_backend));
		backend->surface = surface;

		//NOTE(martin): setup interface functions
		backend->interface.destroy = mg_gl_canvas_destroy;
		backend->interface.begin = mg_gl_canvas_begin;
		backend->interface.end = mg_gl_canvas_end;
		backend->interface.clear = mg_gl_canvas_clear;
		backend->interface.drawBatch = mg_gl_canvas_draw_batch;
		backend->interface.imageCreate = mg_gl_canvas_image_create;
		backend->interface.imageDestroy = mg_gl_canvas_image_destroy;
		backend->interface.imageUploadRegion = mg_gl_canvas_image_upload_region;

		mg_surface_prepare(surface);

		glGenVertexArrays(1, &backend->vao);
		glBindVertexArray(backend->vao);

		glGenBuffers(1, &backend->dummyVertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, backend->dummyVertexBuffer);

		glGenBuffers(1, &backend->vertexBuffer);
		glGenBuffers(1, &backend->shapeBuffer);
		glGenBuffers(1, &backend->indexBuffer);

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
		err |= mg_gl_canvas_compile_compute_program(glsl_clear_counters, &backend->clearCounterProgram);
		err |= mg_gl_canvas_compile_compute_program(glsl_tile, &backend->tileProgram);
		err |= mg_gl_canvas_compile_compute_program(glsl_sort, &backend->sortProgram);
		err |= mg_gl_canvas_compile_compute_program(glsl_draw, &backend->drawProgram);
		err |= mg_gl_canvas_compile_render_program("blit", glsl_blit_vertex, glsl_blit_fragment, &backend->blitProgram);

		if(glGetError() != GL_NO_ERROR)
		{
			err |= -1;
		}

		backend->shapeMapping = malloc_array(char, MG_GL_CANVAS_MAX_SHAPE_BUFFER_SIZE);
		backend->vertexMapping = malloc_array(char, MG_GL_CANVAS_MAX_VERTEX_BUFFER_SIZE);
		backend->indexMapping = malloc_array(char, MG_GL_CANVAS_MAX_INDEX_BUFFER_SIZE);

		if(  !backend->shapeMapping
		  || !backend->shapeMapping
		  || !backend->shapeMapping)
		{
			err |= -1;
		}

		if(err)
		{
			mg_gl_canvas_destroy((mg_canvas_backend*)backend);
			backend = 0;
		}
		else
		{
			mg_gl_canvas_update_vertex_layout(backend);
		}
	}

	return((mg_canvas_backend*)backend);
}
