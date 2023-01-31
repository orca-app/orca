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
	GLint program;

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

void mg_gles_canvas_draw_buffers(mg_canvas_backend* interface, u32 vertexCount, u32 indexCount, mg_color clearColor)
{
	mg_gles_canvas_backend* backend = (mg_gles_canvas_backend*)interface;
	mg_gles_surface* surface = mg_gles_canvas_get_surface(backend);
	if(!surface)
	{
		return;
	}

	//WARN: dummy test code

	indexCount = 3;
	*(vec2*)(interface->vertexLayout.posBuffer) = (vec2){400, 300};
	*(vec2*)(interface->vertexLayout.posBuffer + interface->vertexLayout.posStride) = (vec2){450, 300};
	*(vec2*)(interface->vertexLayout.posBuffer + 2*interface->vertexLayout.posStride) = (vec2){400, 350};

	for(int i=0; i<3; i++)
	{
		*(vec4*)(interface->vertexLayout.cubicBuffer + i*interface->vertexLayout.cubicStride) = (vec4){1, 1, 1, 1};
		*(vec2*)(interface->vertexLayout.uvBuffer + i*interface->vertexLayout.uvStride) = (vec2){0, 0};
		*(vec4*)(interface->vertexLayout.colorBuffer + i*interface->vertexLayout.colorStride) = (vec4){1, 0, 0, 1};
		*(vec4*)(interface->vertexLayout.clipBuffer + i*interface->vertexLayout.clipStride) = (vec4){-FLT_MAX/2, -FLT_MAX/2, FLT_MAX, FLT_MAX};
		*(u32*)(interface->vertexLayout.zIndexBuffer + i*interface->vertexLayout.zIndexStride) = 1;
		*(u32*)(interface->vertexLayout.indexBuffer + i*interface->vertexLayout.indexStride) = i;
	}

	// end dummy test code

	glUseProgram(backend->program);
	glBindBuffer(GL_ARRAY_BUFFER, backend->dummyVertexBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, backend->vertexBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, backend->indexBuffer);
	glUniform1i(0, indexCount);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

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
	MG_GLES_CANVAS_DEFAULT_BUFFER_LENGTH = 1<<20,
	MG_GLES_CANVAS_VERTEX_BUFFER_SIZE = MG_GLES_CANVAS_DEFAULT_BUFFER_LENGTH * LAYOUT_VERTEX_SIZE,
	MG_GLES_CANVAS_INDEX_BUFFER_SIZE = MG_GLES_CANVAS_DEFAULT_BUFFER_LENGTH * LAYOUT_INT_SIZE,
};

void mg_gles_canvas_update_vertex_layout(mg_gles_canvas_backend* backend)
{
	if(backend->vertexMapping)
	{
		glUnmapBuffer(backend->vertexBuffer);
	}
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->vertexBuffer);
	backend->vertexMapping = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, MG_GLES_CANVAS_VERTEX_BUFFER_SIZE, GL_MAP_WRITE_BIT);

	if(backend->indexMapping)
	{
		free(backend->indexMapping);
	}
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

void mg_gles_canvas_destroy(mg_canvas_backend* interface)
{
	mg_gles_canvas_backend* backend = (mg_gles_canvas_backend*)interface;

	//TODO
}

void mg_gles_canvas_atlas_upload(mg_canvas_backend* interface, mp_rect rect, u8* bytes)
{
	//TODO
}

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
		backend->interface.drawBuffers = mg_gles_canvas_draw_buffers;
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

		glGenBuffers(1, &backend->dummyVertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, backend->dummyVertexBuffer);

		unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
		unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		backend->program = glCreateProgram();

		compile_shader(vertexShader, gles_canvas_vertex);
		compile_shader(fragmentShader, gles_canvas_fragment);

		glAttachShader(backend->program, vertexShader);
		glAttachShader(backend->program, fragmentShader);
		glLinkProgram(backend->program);

		int status = 0;
		glGetProgramiv(backend->program, GL_LINK_STATUS, &status);
		if(!status)
		{
			char buffer[256];
			int size = 0;
			glGetProgramInfoLog(backend->program, 256, &size, buffer);
			printf("link error: %.*s\n", size, buffer);
 		}

		glUseProgram(backend->program);

		mg_gles_canvas_update_vertex_layout(backend);
	}

	return((mg_canvas_backend*)backend);
}


#undef LOG_SUBSYSTEM
