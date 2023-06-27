/************************************************************//**
*
*	@file: gl_canvas.c
*	@author: Martin Fouilleul
*	@date: 29/01/2023
*	@revision:
*
*****************************************************************/
#include"graphics_surface.h"
#include"macro_helpers.h"
#include"glsl_shaders.h"
#include"gl_api.h"

typedef struct mg_gl_image
{
	mg_image_data interface;
	GLuint texture;
} mg_gl_image;

typedef enum {
	MG_GL_FILL,
	MG_GL_STROKE,
} mg_gl_cmd;

typedef struct mg_gl_path
{
	mg_gl_cmd cmd;
	float uvTransform[9];
	vec4 color;
	vec4 box;
	vec4 clip;
} mg_gl_path;

typedef enum {
	MG_GL_LINE = 1,
	MG_GL_QUADRATIC,
	MG_GL_CUBIC,
} mg_gl_seg_kind;

typedef struct mg_gl_path_elt
{
	int pathIndex;
	int localEltIndex;
	mg_gl_seg_kind kind;
	vec2 p[4];
} mg_gl_path_elt;

////////////////////////////////////////////////////////////
//NOTE: these are just here for the sizes...

typedef struct mg_gl_segment
{
	int kind;
	int pathIndex;
	int config;
	int windingIncrement;
	vec4 box;
	float hullMatrix[9];
	float implicitMatrix[9];
	float sign;
	vec2 hullVertex;
	int debugID;

} mg_gl_segment;

typedef struct mg_gl_path_queue
{
	vec4 area;
	int tileQueues;
} mg_gl_path_queue;

typedef struct mg_gl_tile_op
{
	int kind;
	int index;
	int next;
	bool crossRight;
	int windingOffset;

} mg_gl_tile_op;

typedef struct mg_gl_tile_queue
{
	int windingOffset;
	int first;
	int last;

} mg_gl_tile_queue;

typedef struct mg_gl_encoding_context
{
	int glEltCount;
	mg_gl_path* pathBufferData;
	mg_gl_path_elt* elementBufferData;
	int pathIndex;
	int localEltIndex;
	mg_primitive* primitive;
	vec4 pathScreenExtents;
	vec4 pathUserExtents;

} mg_gl_encoding_context;
////////////////////////////////////////////////////////////


enum {
//	MG_GL_INPUT_BUFFERS_COUNT = 3,
	MG_GL_TILE_SIZE = 16,
	MG_GL_MSAA_COUNT = 8,
};

typedef struct mg_gl_canvas_backend
{
	mg_canvas_backend interface;
	mg_wgl_surface* surface;

	GLuint vao;

	GLuint pathSetup;
	GLuint segmentSetup;
	GLuint backprop;
	GLuint merge;
	GLuint raster;
	GLuint blit;

	GLuint outTexture;

	int pathBufferOffset;
	int elementBufferOffset;
	int bufferIndex;
	//TODO buffer semaphore...

	GLuint pathBuffer;
	GLuint elementBuffer;

	GLuint segmentBuffer;
	GLuint segmentCountBuffer;
	GLuint pathQueueBuffer;
	GLuint tileQueueBuffer;
	GLuint tileQueueCountBuffer;
	GLuint tileOpBuffer;
	GLuint tileOpCountBuffer;
	GLuint screenTilesBuffer;

	GLuint dummyVertexBuffer;

	mg_gl_path* pathBufferData;
	mg_gl_path_elt* elementBufferData;

	int msaaCount;
	vec2 frameSize;

} mg_gl_canvas_backend;

static void mg_update_path_extents(vec4* extents, vec2 p)
{
	extents->x = minimum(extents->x, p.x);
	extents->y = minimum(extents->y, p.y);
	extents->z = maximum(extents->z, p.x);
	extents->w = maximum(extents->w, p.y);
}

void mg_gl_canvas_encode_element(mg_gl_encoding_context* context, mg_path_elt_type kind, vec2* p)
{
	mg_gl_path_elt* glElt = &context->elementBufferData[context->glEltCount];
	context->glEltCount++;

	glElt->pathIndex = context->pathIndex;
	int count = 0;
	switch(kind)
	{
		case MG_PATH_LINE:
			glElt->kind = MG_GL_LINE;
			count = 2;
			break;

		case MG_PATH_QUADRATIC:
			glElt->kind = MG_GL_QUADRATIC;
			count = 3;
			break;

		case MG_PATH_CUBIC:
			glElt->kind = MG_GL_CUBIC;
			count = 4;
			break;

		default:
			break;
	}

	glElt->localEltIndex = context->localEltIndex;

	for(int i=0; i<count; i++)
	{
		mg_update_path_extents(&context->pathUserExtents, p[i]);

		vec2 screenP = mg_mat2x3_mul(context->primitive->attributes.transform, p[i]);
		glElt->p[i] = (vec2){screenP.x, screenP.y};

		mg_update_path_extents(&context->pathScreenExtents, screenP);
	}
}

void mg_gl_render_batch(mg_gl_canvas_backend* backend,
                        mg_wgl_surface* surface,
                        int pathCount,
                        int eltCount,
                        mg_image_data* image,
                        int tileSize,
                        int nTilesX,
                        int nTilesY,
                        vec2 viewportSize,
                        f32 scale)
{
	//NOTE: clear counters
	int zero = 0;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->segmentCountBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int), &zero, GL_DYNAMIC_COPY);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->tileQueueCountBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int), &zero, GL_DYNAMIC_COPY);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->tileOpCountBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int), &zero, GL_DYNAMIC_COPY);

	//NOTE: path setup pass
	glUseProgram(backend->pathSetup);

	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, backend->pathBuffer, backend->pathBufferOffset, pathCount*sizeof(mg_gl_path));
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, backend->pathQueueBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, backend->tileQueueBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, backend->tileQueueCountBuffer);

	glUniform1i(0, tileSize);
	glUniform1f(1, scale);

	glDispatchCompute(pathCount, 1, 1);

	//NOTE: segment setup pass
	glUseProgram(backend->segmentSetup);

	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, backend->elementBuffer, backend->elementBufferOffset, eltCount*sizeof(mg_gl_path_elt));
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, backend->segmentCountBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, backend->segmentBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, backend->pathQueueBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, backend->tileQueueBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, backend->tileOpBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, backend->tileOpCountBuffer);

	glUniform1i(0, tileSize);
	glUniform1f(1, scale);

	glDispatchCompute(eltCount, 1, 1);

	//NOTE: backprop pass
	glUseProgram(backend->backprop);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, backend->pathQueueBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, backend->tileQueueBuffer);

	glDispatchCompute(pathCount*16, 1, 1);

	//NOTE: merge pass
	glUseProgram(backend->merge);

	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, backend->pathBuffer, backend->pathBufferOffset, pathCount*sizeof(mg_gl_path));
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, backend->pathQueueBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, backend->tileQueueBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, backend->tileOpBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, backend->tileOpCountBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, backend->screenTilesBuffer);

	glUniform1i(0, tileSize);
	glUniform1f(1, scale);

	glDispatchCompute(nTilesX, nTilesY, 1);

	//NOTE: raster pass
	glUseProgram(backend->raster);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, backend->screenTilesBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, backend->tileOpBuffer);
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2, backend->pathBuffer, backend->pathBufferOffset, pathCount*sizeof(mg_gl_path));
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, backend->segmentBuffer);

	glUniform1i(0, tileSize);
	glUniform1f(1, scale);
	glUniform1i(2, backend->msaaCount);

	glBindImageTexture(0, backend->outTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

	if(image)
	{
		//TODO: make sure this image belongs to that context
		mg_gl_image* glImage = (mg_gl_image*)image;
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, glImage->texture);
		glUniform1ui(3, 1);
	}
	else
	{
		glUniform1ui(3, 0);
	}

	glDispatchCompute(viewportSize.x, viewportSize.y, 1);

	//NOTE: blit pass
	glUseProgram(backend->blit);
	glBindBuffer(GL_ARRAY_BUFFER, backend->dummyVertexBuffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, backend->outTexture);
	glUniform1i(0, 0);

	glDrawArrays(GL_TRIANGLES, 0, 6);
}

/////////////////////////////////////////////////////////////////////////
//TODO
void mg_gl_canvas_resize(mg_gl_canvas_backend* backend, vec2 size);
/////////////////////////////////////////////////////////////////////////

void mg_gl_canvas_render(mg_canvas_backend* interface,
                         mg_color clearColor,
                         u32 primitiveCount,
                         mg_primitive* primitives,
                         u32 eltCount,
                         mg_path_elt* pathElements)
{
	mg_gl_canvas_backend* backend = (mg_gl_canvas_backend*)interface;

	//TODO rolling buffer

	//TODO update screen tiles buffer size
	mg_wgl_surface* surface = backend->surface;
	mp_rect frame = surface->interface.getFrame((mg_surface_data*)surface);
	vec2 contentsScaling = surface->interface.contentsScaling((mg_surface_data*)surface);
	//TODO support scaling in both axes
	f32 scale = contentsScaling.x;

	vec2 viewportSize = {frame.w * scale, frame.h * scale};
	int tileSize = MG_GL_TILE_SIZE;
	int nTilesX = (int)(frame.w * scale + tileSize - 1)/tileSize;
	int nTilesY = (int)(frame.h * scale + tileSize - 1)/tileSize;

	if(viewportSize.x != backend->frameSize.x || viewportSize.y != backend->frameSize.y)
	{
		//TODO:	mg_gl_canvas_resize(backend, viewportSize);
	}

	//NOTE: clear screen and reset input buffer offsets
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT);

	backend->pathBufferOffset = 0;
	backend->elementBufferOffset = 0;

	//NOTE: encode and render batches
	int pathCount = 0;
	vec2 currentPos = {0};
	mg_image currentImage = mg_image_nil();

	/////////////////////////////////////////////////////////////////////////////////
	//TODO: we must map or allocate elementBufferData and pathBufferData...
	/////////////////////////////////////////////////////////////////////////////////
	mg_gl_encoding_context context = {.glEltCount = 0,
	                                  .elementBufferData = backend->elementBufferData,
	                                  .pathBufferData = backend->pathBufferData };

	for(int primitiveIndex = 0; primitiveIndex < primitiveCount; primitiveIndex++)
	{
		mg_primitive* primitive = &primitives[primitiveIndex];

		if(primitiveIndex && (primitive->attributes.image.h != currentImage.h))
		{
			mg_image_data* imageData = mg_image_data_from_handle(currentImage);

			mg_gl_render_batch(backend,
			                   surface,
			                   pathCount,
			                   context.glEltCount,
			                   imageData,
			                   tileSize,
			                   nTilesX,
			                   nTilesY,
			                   viewportSize,
			                   scale);

			backend->pathBufferOffset += pathCount * sizeof(mg_gl_path);
			backend->elementBufferOffset += context.glEltCount * sizeof(mg_gl_path_elt);
			pathCount = 0;
			context.glEltCount = 0;
			context.elementBufferData = (mg_gl_path_elt*)((char*)backend->elementBufferData + backend->elementBufferOffset);
			context.pathBufferData = (mg_gl_path*)((char*)backend->pathBufferData + backend->pathBufferOffset);
		}
		currentImage = primitive->attributes.image;

		if(primitive->path.count)
		{
			context.primitive = primitive;
			context.pathIndex = pathCount;
			context.pathScreenExtents = (vec4){FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX};
			context.pathUserExtents = (vec4){FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX};

			if(primitive->cmd == MG_CMD_STROKE)
			{
//TODO				mg_gl_render_stroke(&context, pathElements + primitive->path.startIndex, &primitive->path);
			}
			else
			{
				int segCount = 0;
				for(int eltIndex = 0;
			    	(eltIndex < primitive->path.count) && (primitive->path.startIndex + eltIndex < eltCount);
			    	eltIndex++)
				{
					context.localEltIndex = segCount;

					mg_path_elt* elt = &pathElements[primitive->path.startIndex + eltIndex];

					if(elt->type != MG_PATH_MOVE)
					{
						vec2 p[4] = {currentPos, elt->p[0], elt->p[1], elt->p[2]};
						mg_gl_canvas_encode_element(&context, elt->type, p);
						segCount++;
					}
					switch(elt->type)
					{
						case MG_PATH_MOVE:
							currentPos = elt->p[0];
							break;

						case MG_PATH_LINE:
							currentPos = elt->p[0];
							break;

						case MG_PATH_QUADRATIC:
							currentPos = elt->p[1];
							break;

						case MG_PATH_CUBIC:
							currentPos = elt->p[2];
							break;
					}
				}
			}
			//NOTE: push path
			mg_gl_path* path = &context.pathBufferData[pathCount];
			pathCount++;

			path->cmd =	(mg_gl_cmd)primitive->cmd;

			path->box = (vec4){context.pathScreenExtents.x,
			                   context.pathScreenExtents.y,
			                   context.pathScreenExtents.z,
			                   context.pathScreenExtents.w};

			path->clip = (vec4){primitive->attributes.clip.x,
			                    primitive->attributes.clip.y,
			                    primitive->attributes.clip.x + primitive->attributes.clip.w,
			                    primitive->attributes.clip.y + primitive->attributes.clip.h};

			path->color = (vec4){primitive->attributes.color.r,
			                     primitive->attributes.color.g,
			                     primitive->attributes.color.b,
			                     primitive->attributes.color.a};

			mp_rect srcRegion = primitive->attributes.srcRegion;

			mp_rect destRegion = {context.pathUserExtents.x,
			                      context.pathUserExtents.y,
			                      context.pathUserExtents.z - context.pathUserExtents.x,
			                      context.pathUserExtents.w - context.pathUserExtents.y};

			if(!mg_image_is_nil(primitive->attributes.image))
			{
				vec2 texSize = mg_image_size(primitive->attributes.image);

				mg_mat2x3 srcRegionToImage = {1/texSize.x, 0, srcRegion.x/texSize.x,
				                              0, 1/texSize.y, srcRegion.y/texSize.y};

				mg_mat2x3 destRegionToSrcRegion = {srcRegion.w/destRegion.w, 0, 0,
				                                   0, srcRegion.h/destRegion.h, 0};

				mg_mat2x3 userToDestRegion = {1, 0, -destRegion.x,
				                              0, 1, -destRegion.y};

				mg_mat2x3 screenToUser = mg_mat2x3_inv(primitive->attributes.transform);

				mg_mat2x3 uvTransform = srcRegionToImage;
				uvTransform = mg_mat2x3_mul_m(uvTransform, destRegionToSrcRegion);
				uvTransform = mg_mat2x3_mul_m(uvTransform, userToDestRegion);
				uvTransform = mg_mat2x3_mul_m(uvTransform, screenToUser);

				path->uvTransform[0] = uvTransform.m[0]/scale;
				path->uvTransform[1] = uvTransform.m[3]/scale;
				path->uvTransform[2] = 0;
				path->uvTransform[3] = uvTransform.m[1]/scale;
				path->uvTransform[4] = uvTransform.m[4]/scale;
				path->uvTransform[5] = 0;
				path->uvTransform[6] = uvTransform.m[2];
				path->uvTransform[7] = uvTransform.m[5];
				path->uvTransform[8] = 1;
			}
		}
	}

	mg_image_data* imageData = mg_image_data_from_handle(currentImage);
	mg_gl_render_batch(backend,
	                    surface,
	                    pathCount,
	                    context.glEltCount,
	                    imageData,
	                    tileSize,
	                    nTilesX,
	                    nTilesY,
	                    viewportSize,
	                    scale);

	//TODO add completion handler for rolling input buffers
}

//--------------------------------------------------------------------
// Image API
//--------------------------------------------------------------------
mg_image_data* mg_gl_canvas_image_create(mg_canvas_backend* interface, vec2 size)
{
	mg_gl_image* image = 0;

	image = malloc_type(mg_gl_image);
	if(image)
	{
		glGenTextures(1, &image->texture);
		glBindTexture(GL_TEXTURE_2D, image->texture);
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
	glDeleteTextures(1, &image->texture);
	free(image);
}

void mg_gl_canvas_image_upload_region(mg_canvas_backend* interface,
                                      mg_image_data* imageInterface,
                                      mp_rect region,
                                      u8* pixels)
{
	//TODO: check that this image belongs to this context
	mg_gl_image* image = (mg_gl_image*)imageInterface;
	glBindTexture(GL_TEXTURE_2D, image->texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, region.w, region.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

//--------------------------------------------------------------------
// Canvas setup / destroy
//--------------------------------------------------------------------

void mg_gl_canvas_destroy(mg_canvas_backend* interface)
{
	mg_gl_canvas_backend* backend = (mg_gl_canvas_backend*)interface;

	////////////////////////////////////////////////////////////////////
	//TODO
	////////////////////////////////////////////////////////////////////

	free(backend);
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
			log_error("Shader link error (%s): %.*s\n", name, size, buffer);

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
			log_error("Shader link error (%s): %.*s\n", progName, size, buffer);
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

const u32 MG_GL_PATH_BUFFER_SIZE       = (4<<20)*sizeof(mg_gl_path),
          MG_GL_ELEMENT_BUFFER_SIZE    = (4<<20)*sizeof(mg_gl_path_elt),
          MG_GL_SEGMENT_BUFFER_SIZE    = (4<<20)*sizeof(mg_gl_segment),
          MG_GL_PATH_QUEUE_BUFFER_SIZE = (4<<20)*sizeof(mg_gl_path_queue),
          MG_GL_TILE_QUEUE_BUFFER_SIZE = (4<<20)*sizeof(mg_gl_tile_queue),
          MG_GL_TILE_OP_BUFFER_SIZE    = (4<<20)*sizeof(mg_gl_tile_op);

mg_canvas_backend* gl_canvas_backend_create(mg_wgl_surface* surface)
{
	mg_gl_canvas_backend* backend = malloc_type(mg_gl_canvas_backend);
	if(backend)
	{
		memset(backend, 0, sizeof(mg_gl_canvas_backend));
		backend->surface = surface;

		backend->msaaCount = MG_GL_MSAA_COUNT;

		//NOTE(martin): setup interface functions
		backend->interface.destroy = mg_gl_canvas_destroy;
		backend->interface.render = mg_gl_canvas_render;
		backend->interface.imageCreate = mg_gl_canvas_image_create;
		backend->interface.imageDestroy = mg_gl_canvas_image_destroy;
		backend->interface.imageUploadRegion = mg_gl_canvas_image_upload_region;

		surface->interface.prepare((mg_surface_data*)surface);

		glGenVertexArrays(1, &backend->vao);
		glBindVertexArray(backend->vao);

		//NOTE: create programs
		int err = 0;
		err |= mg_gl_canvas_compile_compute_program(glsl_path_setup, &backend->pathSetup);
		err |= mg_gl_canvas_compile_compute_program(glsl_segment_setup, &backend->segmentSetup);
		err |= mg_gl_canvas_compile_compute_program(glsl_backprop, &backend->backprop);
		err |= mg_gl_canvas_compile_compute_program(glsl_merge, &backend->merge);
		err |= mg_gl_canvas_compile_compute_program(glsl_raster, &backend->raster);
		err |= mg_gl_canvas_compile_render_program("blit", glsl_blit_vertex, glsl_blit_fragment, &backend->blit);

		if(glGetError() != GL_NO_ERROR)
		{
			err |= -1;
		}

		//NOTE: create out texture
		mp_rect frame = surface->interface.getFrame((mg_surface_data*)surface);
		vec2 scale = surface->interface.contentsScaling((mg_surface_data*)surface);

		glGenTextures(1, &backend->outTexture);
		glBindTexture(GL_TEXTURE_2D, backend->outTexture);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, frame.w*scale.x, frame.h*scale.y);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//NOTE: generate buffers
		glGenBuffers(1, &backend->dummyVertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, backend->dummyVertexBuffer);

		glGenBuffers(1, &backend->pathBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->pathBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MG_GL_PATH_BUFFER_SIZE, 0, GL_DYNAMIC_COPY);

		//TODO change flags
		glGenBuffers(1, &backend->elementBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->elementBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MG_GL_ELEMENT_BUFFER_SIZE, 0, GL_DYNAMIC_COPY);

		glGenBuffers(1, &backend->segmentBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->segmentBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MG_GL_SEGMENT_BUFFER_SIZE, 0, GL_DYNAMIC_COPY);

		glGenBuffers(1, &backend->segmentCountBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->segmentCountBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int), 0, GL_DYNAMIC_COPY);

		glGenBuffers(1, &backend->pathQueueBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->pathQueueBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MG_GL_PATH_QUEUE_BUFFER_SIZE, 0, GL_DYNAMIC_COPY);

		glGenBuffers(1, &backend->tileQueueBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->tileQueueBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MG_GL_TILE_QUEUE_BUFFER_SIZE, 0, GL_DYNAMIC_COPY);

		glGenBuffers(1, &backend->tileQueueCountBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->tileQueueCountBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int), 0, GL_DYNAMIC_COPY);

		glGenBuffers(1, &backend->tileOpBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->tileOpBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MG_GL_TILE_OP_BUFFER_SIZE, 0, GL_DYNAMIC_COPY);

		glGenBuffers(1, &backend->tileOpCountBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->tileOpCountBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int), 0, GL_DYNAMIC_COPY);

		int tileSize = MG_GL_TILE_SIZE;
		int nTilesX = (int)(frame.w * scale.x + tileSize - 1)/tileSize;
		int nTilesY = (int)(frame.h * scale.y + tileSize - 1)/tileSize;

		glGenBuffers(1, &backend->screenTilesBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, backend->screenTilesBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, nTilesX*nTilesY*sizeof(int), 0, GL_DYNAMIC_COPY);

		backend->pathBufferData = malloc(MG_GL_PATH_BUFFER_SIZE);
		backend->elementBufferData = malloc(MG_GL_ELEMENT_BUFFER_SIZE);

		if(err)
		{
			mg_gl_canvas_destroy((mg_canvas_backend*)backend);
			backend = 0;
		}
	}
	return((mg_canvas_backend*)backend);
}

mg_surface_data* gl_canvas_surface_create_for_window(mp_window window)
{
	mg_wgl_surface* surface = (mg_wgl_surface*)mg_wgl_surface_create_for_window(window);

	if(surface)
	{
		surface->interface.backend = gl_canvas_backend_create(surface);
		if(surface->interface.backend)
		{
			surface->interface.api = MG_CANVAS;
		}
		else
		{
			surface->interface.destroy((mg_surface_data*)surface);
			surface = 0;
		}
	}
	return((mg_surface_data*)surface);
}
