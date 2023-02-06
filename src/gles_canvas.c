/************************************************************//**
*
*	@file: gles_canvas.c
*	@author: Martin Fouilleul
*	@date: 29/01/2023
*	@revision:
*
*****************************************************************/
#include"graphics_internal.h"
#include"macro_helpers.h"
#include"gles_canvas_shaders.h"

#define LOG_SUBSYSTEM "Graphics"

typedef struct mg_gles_canvas_backend
{
	mg_canvas_backend interface;
	mg_surface surface;

	GLint dummyVertexBuffer;
	GLint vertexBuffer;
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

} mg_gles_canvas_backend;

mg_gles_surface* mg_gles_canvas_get_surface(mg_gles_canvas_backend* canvas)
{
	mg_gles_surface* res = 0;
	mg_surface_data* data = mg_surface_data_from_handle(canvas->surface);
	if(data && data->backend == MG_BACKEND_GLES)
	{
		res = (mg_gles_surface*)data;
	}
	return(res);
}

//NOTE: debugger
typedef struct debug_vertex
{
	vec2 pos;
	u8 align0[8];
	vec4 cubic;
	vec2 uv;
	u8 align1[8];
	vec4 color;
	vec4 clip;
	int zIndex;
	u8 align2[12];
} debug_vertex;

#define LayoutNext(prevName, prevType, nextType) \
	AlignUpOnPow2(_cat3_(LAYOUT_, prevName, _OFFSET)+_cat3_(LAYOUT_, prevType, _SIZE), _cat3_(LAYOUT_, nextType, _ALIGN))

enum {
	LAYOUT_VEC2_SIZE = 8,
	LAYOUT_VEC2_ALIGN = 8,
	LAYOUT_VEC4_SIZE = 16,
	LAYOUT_VEC4_ALIGN = 16,
	LAYOUT_INT_SIZE = 4,
	LAYOUT_INT_ALIGN = 4,

	LAYOUT_POS_OFFSET = 0,
	LAYOUT_CUBIC_OFFSET = LayoutNext(POS, VEC2, VEC4),
	LAYOUT_UV_OFFSET = LayoutNext(CUBIC, VEC4, VEC2),
	LAYOUT_COLOR_OFFSET = LayoutNext(UV, VEC2, VEC4),
	LAYOUT_CLIP_OFFSET = LayoutNext(COLOR, VEC4, VEC4),
	LAYOUT_ZINDEX_OFFSET = LayoutNext(CLIP, VEC4, INT),

	LAYOUT_VERTEX_ALIGN = 16,
	LAYOUT_VERTEX_SIZE = LayoutNext(ZINDEX, INT, VERTEX),
};

enum {
	MG_GLES_CANVAS_DEFAULT_BUFFER_LENGTH = 8<<10,
	MG_GLES_CANVAS_VERTEX_BUFFER_SIZE = MG_GLES_CANVAS_DEFAULT_BUFFER_LENGTH * LAYOUT_VERTEX_SIZE,
	MG_GLES_CANVAS_INDEX_BUFFER_SIZE = MG_GLES_CANVAS_DEFAULT_BUFFER_LENGTH * LAYOUT_INT_SIZE,
	MG_GLES_CANVAS_TILE_COUNTER_BUFFER_SIZE = 65536,
	MG_GLES_CANVAS_TILE_ARRAY_SIZE = sizeof(int)*4096,
	MG_GLES_CANVAS_TILE_ARRAY_BUFFER_SIZE = MG_GLES_CANVAS_TILE_COUNTER_BUFFER_SIZE * MG_GLES_CANVAS_TILE_ARRAY_SIZE,
};

void mg_gles_canvas_update_vertex_layout(mg_gles_canvas_backend* backend)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->vertexBuffer);
	backend->vertexMapping = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, MG_GLES_CANVAS_VERTEX_BUFFER_SIZE, GL_MAP_WRITE_BIT);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->indexBuffer);
	backend->indexMapping = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, MG_GLES_CANVAS_INDEX_BUFFER_SIZE, GL_MAP_WRITE_BIT);

	backend->interface.vertexLayout = (mg_vertex_layout){
		    .maxVertexCount = MG_GLES_CANVAS_DEFAULT_BUFFER_LENGTH,
	        .maxIndexCount = MG_GLES_CANVAS_DEFAULT_BUFFER_LENGTH,
	        .posBuffer = backend->vertexMapping + LAYOUT_POS_OFFSET,
	        .posStride = LAYOUT_VERTEX_SIZE,
	        .cubicBuffer = backend->vertexMapping + LAYOUT_CUBIC_OFFSET,
	        .cubicStride = LAYOUT_VERTEX_SIZE,
	        .uvBuffer = backend->vertexMapping + LAYOUT_UV_OFFSET,
	        .uvStride = LAYOUT_VERTEX_SIZE,
	        .colorBuffer = backend->vertexMapping + LAYOUT_COLOR_OFFSET,
	        .colorStride = LAYOUT_VERTEX_SIZE,
	        .clipBuffer = backend->vertexMapping + LAYOUT_CLIP_OFFSET,
	        .clipStride = LAYOUT_VERTEX_SIZE,
	        .zIndexBuffer = backend->vertexMapping + LAYOUT_ZINDEX_OFFSET,
	        .zIndexStride = LAYOUT_VERTEX_SIZE,
	        .indexBuffer = backend->indexMapping,
	        .indexStride = LAYOUT_INT_SIZE};
}

void mg_gles_canvas_begin(mg_canvas_backend* interface)
{
	mg_gles_canvas_backend* backend = (mg_gles_canvas_backend*)interface;
	mg_gles_surface* surface = mg_gles_canvas_get_surface(backend);
	if(!surface)
	{
		return;
	}
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void mg_gles_canvas_end(mg_canvas_backend* interface)
{
	//NOTE: nothing to do here...
}

void mg_gles_canvas_clear(mg_canvas_backend* interface, mg_color clearColor)
{
	mg_gles_canvas_backend* backend = (mg_gles_canvas_backend*)interface;
	mg_gles_surface* surface = mg_gles_canvas_get_surface(backend);
	if(!surface)
	{
		return;
	}

	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT);
}

void mg_gles_canvas_draw_batch(mg_canvas_backend* interface, u32 vertexCount, u32 indexCount)
{
	mg_gles_canvas_backend* backend = (mg_gles_canvas_backend*)interface;
	mg_gles_surface* surface = mg_gles_canvas_get_surface(backend);
	if(!surface)
	{
		return;
	}

/*NOTE: if we want debug_vertex while debugging, the following ensures the struct def doesn't get stripped away
	debug_vertex vertex;
	printf("foo %p\n", &vertex);
//*/
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->vertexBuffer);
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->indexBuffer);
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	mp_rect frame = mg_surface_get_frame(backend->surface);

	const int tileSize = 16;
	const int tileCountX = (frame.w + tileSize - 1)/tileSize;
	const int tileCountY = (frame.h + tileSize - 1)/tileSize;
	const int tileArraySize = MG_GLES_CANVAS_TILE_ARRAY_SIZE;

	//TODO: ensure there's enough space in tile buffer

	//NOTE: first clear counters
	glUseProgram(backend->clearCounterProgram);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, backend->tileCounterBuffer);
	glDispatchCompute(tileCountX*tileCountY, 1, 1);

	//NOTE: we first distribute triangles into tiles:

	glUseProgram(backend->tileProgram);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, backend->vertexBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, backend->indexBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, backend->tileCounterBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, backend->tileArrayBuffer);

	glUniform1ui(0, indexCount);
	glUniform2ui(1, tileCountX, tileCountY);
	glUniform1ui(2, tileSize);
	glUniform1ui(3, tileArraySize);

	glDispatchCompute(indexCount/3, 1, 1);

	//NOTE: next we sort triangles in each tile
	glUseProgram(backend->sortProgram);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, backend->vertexBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, backend->indexBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, backend->tileCounterBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, backend->tileArrayBuffer);

	glUniform1ui(0, indexCount);
	glUniform2ui(1, tileCountX, tileCountY);
	glUniform1ui(2, tileSize);
	glUniform1ui(3, tileArraySize);

	glDispatchCompute(tileCountX * tileCountY, 1, 1);

	//TODO: then we fire the fragment shader that will select only triangles in its tile
//	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glUseProgram(backend->drawProgram);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, backend->vertexBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, backend->indexBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, backend->tileCounterBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, backend->tileArrayBuffer);

	glBindImageTexture(0, backend->outTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

	glUniform1ui(0, indexCount);
	glUniform2ui(1, tileCountX, tileCountY);
	glUniform1ui(2, tileSize);
	glUniform1ui(3, tileArraySize);

	glDispatchCompute(tileCountX, tileCountY, 1);

	//NOTE: now blit out texture to surface
	glUseProgram(backend->blitProgram);
	glBindBuffer(GL_ARRAY_BUFFER, backend->dummyVertexBuffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, backend->outTexture);
	glUniform1i(0, 0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	mg_gles_canvas_update_vertex_layout(backend);
}

void mg_gles_canvas_destroy(mg_canvas_backend* interface)
{
	mg_gles_canvas_backend* backend = (mg_gles_canvas_backend*)interface;

	//TODO
}

void mg_gles_canvas_atlas_upload(mg_canvas_backend* interface, mp_rect rect, u8* bytes)
{
	//TODO
}

static void mg_gles_compile_shader(GLuint shader, const char* source)
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

mg_canvas_backend* mg_gles_canvas_create(mg_surface surface)
{
	mg_gles_canvas_backend* backend = 0;
	mg_surface_data* surfaceData = mg_surface_data_from_handle(surface);
	if(surfaceData && surfaceData->backend == MG_BACKEND_GLES)
	{
		mg_gles_surface* glesSurface = (mg_gles_surface*)surfaceData;

		backend = malloc_type(mg_gles_canvas_backend);
		memset(backend, 0, sizeof(mg_gles_canvas_backend));
		backend->surface = surface;

		//NOTE(martin): setup interface functions
		backend->interface.destroy = mg_gles_canvas_destroy;
		backend->interface.begin = mg_gles_canvas_begin;
		backend->interface.end = mg_gles_canvas_end;
		backend->interface.clear = mg_gles_canvas_clear;
		backend->interface.drawBatch = mg_gles_canvas_draw_batch;
		backend->interface.atlasUpload = mg_gles_canvas_atlas_upload;

		mg_surface_prepare(surface);

		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &backend->vertexBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->vertexBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MG_GLES_CANVAS_VERTEX_BUFFER_SIZE, 0, GL_DYNAMIC_DRAW);

		glGenBuffers(1, &backend->indexBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->indexBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MG_GLES_CANVAS_INDEX_BUFFER_SIZE, 0, GL_DYNAMIC_DRAW);

		glGenBuffers(1, &backend->tileCounterBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->tileCounterBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MG_GLES_CANVAS_TILE_COUNTER_BUFFER_SIZE, 0, GL_DYNAMIC_COPY);

		glGenBuffers(1, &backend->tileArrayBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->tileArrayBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MG_GLES_CANVAS_TILE_ARRAY_BUFFER_SIZE, 0, GL_DYNAMIC_COPY);

		glGenBuffers(1, &backend->dummyVertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, backend->dummyVertexBuffer);

		mp_rect frame = mg_surface_get_frame(backend->surface);
		glGenTextures(1, &backend->outTexture);
		glBindTexture(GL_TEXTURE_2D, backend->outTexture);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, frame.w, frame.h);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//NOTE: create clear program
		{
			GLuint clearShader = glCreateShader(GL_COMPUTE_SHADER);
			backend->clearCounterProgram = glCreateProgram();

			mg_gles_compile_shader(clearShader, gles_canvas_clear_counters);

			glAttachShader(backend->clearCounterProgram, clearShader);
			glLinkProgram(backend->clearCounterProgram);

			int status = 0;
			glGetProgramiv(backend->clearCounterProgram, GL_LINK_STATUS, &status);
			if(!status)
			{
				char buffer[256];
				int size = 0;
				glGetProgramInfoLog(backend->clearCounterProgram, 256, &size, buffer);
				printf("link error in gl_canvas_clear_counters: %.*s\n", size, buffer);
				exit(-1);
 			}
		}
		//NOTE: create tile program
		{
			GLuint tileShader = glCreateShader(GL_COMPUTE_SHADER);
			backend->tileProgram = glCreateProgram();

			mg_gles_compile_shader(tileShader, gles_canvas_tile);

			glAttachShader(backend->tileProgram, tileShader);
			glLinkProgram(backend->tileProgram);

			int status = 0;
			glGetProgramiv(backend->tileProgram, GL_LINK_STATUS, &status);
			if(!status)
			{
				char buffer[256];
				int size = 0;
				glGetProgramInfoLog(backend->tileProgram, 256, &size, buffer);
				printf("link error in gl_canvas_tile: %.*s\n", size, buffer);
				exit(-1);
 			}
		}
		//NOTE: create sort program
		{
			GLuint sortShader = glCreateShader(GL_COMPUTE_SHADER);
			backend->sortProgram = glCreateProgram();

			mg_gles_compile_shader(sortShader, gles_canvas_sort);

			glAttachShader(backend->sortProgram, sortShader);
			glLinkProgram(backend->sortProgram);

			int status = 0;
			glGetProgramiv(backend->sortProgram, GL_LINK_STATUS, &status);
			if(!status)
			{
				char buffer[256];
				int size = 0;
				glGetProgramInfoLog(backend->sortProgram, 256, &size, buffer);
				printf("link error gl_canvas_sort: %.*s\n", size, buffer);
				exit(-1);
 			}
		}

		//NOTE: create draw program
		{
			GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
			backend->drawProgram = glCreateProgram();

			mg_gles_compile_shader(shader, gles_canvas_draw);

			glAttachShader(backend->drawProgram, shader);
			glLinkProgram(backend->drawProgram);

			int status = 0;
			glGetProgramiv(backend->drawProgram, GL_LINK_STATUS, &status);
			if(!status)
			{
				char buffer[256];
				int size = 0;
				glGetProgramInfoLog(backend->drawProgram, 256, &size, buffer);
				printf("link error gl_canvas_draw: %.*s\n", size, buffer);
				exit(-1);
 			}
		}

		//NOTE: create blit program
		{
			GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
			GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
			backend->blitProgram = glCreateProgram();

			mg_gles_compile_shader(vertexShader, gles_canvas_blit_vertex);
			mg_gles_compile_shader(fragmentShader, gles_canvas_blit_fragment);

			glAttachShader(backend->blitProgram, vertexShader);
			glAttachShader(backend->blitProgram, fragmentShader);
			glLinkProgram(backend->blitProgram);

			int status = 0;
			glGetProgramiv(backend->blitProgram, GL_LINK_STATUS, &status);
			if(!status)
			{
				char buffer[256];
				int size = 0;
				glGetProgramInfoLog(backend->blitProgram, 256, &size, buffer);
				printf("link error gl_canvas_blit: %.*s\n", size, buffer);
				exit(-1);
 			}
		}

		mg_gles_canvas_update_vertex_layout(backend);
	}

	return((mg_canvas_backend*)backend);
}


#undef LOG_SUBSYSTEM
