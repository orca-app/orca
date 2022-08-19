/************************************************************//**
*
*	@file: graphics.cpp
*	@author: Martin Fouilleul
*	@date: 01/08/2022
*	@revision:
*
*****************************************************************/
#include<math.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include"stb_truetype.h"

#include"lists.h"
#include"memory.h"
#include"macro_helpers.h"
#include"graphics_internal.h"

#define LOG_SUBSYSTEM "Graphics"

//---------------------------------------------------------------
// graphics canvas structs
//---------------------------------------------------------------

typedef enum { MG_PATH_MOVE,
               MG_PATH_LINE,
	           MG_PATH_QUADRATIC,
	           MG_PATH_CUBIC } mg_path_elt_type;

typedef struct mg_path_elt
{
	mg_path_elt_type type;
	vec2 p[3];

} mg_path_elt;

typedef struct mg_path_descriptor
{
	u32 startIndex;
	u32 count;
	vec2 startPoint;

} mg_path_descriptor;

typedef struct mg_attributes
{
	f32 width;
	f32 tolerance;
	mg_color color;
	mg_joint_type joint;
	f32 maxJointExcursion;
	mg_cap_type cap;

	mg_font font;
	f32 fontSize;

	mg_image image;

	mp_rect clip;

} mg_attributes;

typedef struct mg_rounded_rect
{
	f32 x;
	f32 y;
	f32 w;
	f32 h;
	f32 r;
} mg_rounded_rect;

typedef enum { MG_CMD_CLEAR = 0,
	       MG_CMD_FILL,
	       MG_CMD_STROKE,
	       MG_CMD_RECT_FILL,
	       MG_CMD_RECT_STROKE,
	       MG_CMD_ROUND_RECT_FILL,
	       MG_CMD_ROUND_RECT_STROKE,
	       MG_CMD_ELLIPSE_FILL,
	       MG_CMD_ELLIPSE_STROKE,
	       MG_CMD_JUMP,
	       MG_CMD_MATRIX_PUSH,
	       MG_CMD_MATRIX_POP,
	       MG_CMD_CLIP_PUSH,
	       MG_CMD_CLIP_POP,
	       MG_CMD_IMAGE_DRAW,
	       MG_CMD_ROUNDED_IMAGE_DRAW,
	     } mg_primitive_cmd;

typedef struct mg_primitive
{
	mg_primitive_cmd cmd;
	mg_attributes attributes;

	union
	{
		mg_path_descriptor path;
		mp_rect rect;
		mg_rounded_rect roundedRect;
		utf32 codePoint;
		u32 jump;
		mg_mat2x3 matrix;
	};

} mg_primitive;

typedef struct mg_glyph_map_entry
{
	unicode_range range;
	u32 firstGlyphIndex;

} mg_glyph_map_entry;

typedef struct mg_glyph_info
{
	bool exists;
	utf32 codePoint;
	mg_path_descriptor pathDescriptor;
	mg_text_extents extents;
	//...

} mg_glyph_info;

const u32 MG_STREAM_MAX_COUNT = 128;
const u32 MG_IMAGE_MAX_COUNT = 128;

typedef struct mg_stream_data
{
	list_elt freeListElt;
	u32 generation;
	u64 frame;

	u32 firstCommand;
	u32 lastCommand;
	u32 count;

	bool pendingJump;

} mg_stream_data;

typedef struct mg_image_data
{
	list_elt listElt;
	u32 generation;

	mp_rect rect;

} mg_image_data;

const u32 MG_MATRIX_STACK_MAX_DEPTH = 64;
const u32 MG_CLIP_STACK_MAX_DEPTH = 64;
const u32 MG_MAX_PATH_ELEMENT_COUNT = 2<<20;
const u32 MG_MAX_PRIMITIVE_COUNT = 8<<10;

typedef struct mg_canvas_data
{
	list_elt freeListElt;
	u32 generation;

	mg_stream_data streams[MG_STREAM_MAX_COUNT];
	list_info streamsFreeList;
	u64 frameCounter;

	mg_stream_data* currentStream;

	mg_mat2x3 transform;
	mp_rect clip;

	mg_attributes attributes;
	bool textFlip;
	mg_path_elt pathElements[MG_MAX_PATH_ELEMENT_COUNT];
	mg_path_descriptor path;
	vec2 subPathStartPoint;
	vec2 subPathLastPoint;

	mg_mat2x3 matrixStack[MG_MATRIX_STACK_MAX_DEPTH];
	u32 matrixStackSize;

	mp_rect clipStack[MG_CLIP_STACK_MAX_DEPTH];
	u32 clipStackSize;

	u32 nextZIndex;
	u32 primitiveCount;
	mg_primitive primitives[MG_MAX_PRIMITIVE_COUNT];

	u32 vertexCount;
	u32 indexCount;

	mg_canvas_painter* painter;

	mg_image_data images[MG_IMAGE_MAX_COUNT];
	u32 imageNextIndex;
	list_info imageFreeList;

	vec2 atlasPos;
	u32 atlasLineHeight;
	mg_image blankImage;

} mg_canvas_data;

typedef struct mg_font_info
{
	list_elt freeListElt;
	u32 generation;

	u32 rangeCount;
	u32 glyphCount;
	u32 outlineCount;
	mg_glyph_map_entry* glyphMap;
	mg_glyph_info*      glyphs;
	mg_path_elt* outlines;

	f32 unitsPerEm;
	mg_font_extents extents;

} mg_font_info;

//---------------------------------------------------------------
// internal handle system
//---------------------------------------------------------------

const u32 MG_MAX_RESOURCE_SLOTS = 256,
          MG_MAX_CONTEXTS = 256,
          MG_FONT_MAX_COUNT = 256;

typedef struct mg_resource_slot
{
	list_elt freeListElt;
	u32 generation;
	union
	{
		mg_surface_info* surface;
		mg_surface_server_info* server;
		mg_surface_client_info* client;
	};

} mg_resource_slot;

typedef struct mg_resource_pool
{
	mg_resource_slot slots[MG_MAX_RESOURCE_SLOTS];
	list_info freeList;
	u32 nextIndex;

} mg_resource_pool;

typedef struct mg_info
{
	bool init;

	mg_resource_pool surfaces;
	mg_resource_pool servers;
	mg_resource_pool clients;

	list_info contextFreeList;
	list_info fontFreeList;
	u32 fontsNextIndex;

	mg_canvas_data contexts[MG_MAX_CONTEXTS];
	mg_font_info fonts[MG_FONT_MAX_COUNT];

} mg_info;

static mg_info __mgInfo = {0};


void mg_init()
{
	if(!__mgInfo.init)
	{
		__mgInfo.init = true;

		//TODO: make context handles suitable for zero init

		ListInit(&__mgInfo.contextFreeList);
		for(int i=0; i<MG_MAX_CONTEXTS; i++)
		{
			__mgInfo.contexts[i].generation = 1;
			ListAppend(&__mgInfo.contextFreeList, &__mgInfo.contexts[i].freeListElt);
		}
	}
}

mg_resource_slot* mg_resource_slot_alloc(mg_resource_pool* pool)
{
	mg_resource_slot* slot = ListPopEntry(&pool->freeList, mg_resource_slot, freeListElt);
	if(!slot && pool->nextIndex < MG_MAX_RESOURCE_SLOTS)
	{
		slot = &pool->slots[pool->nextIndex];
		slot->generation = 1;
		pool->nextIndex++;
	}
	return(slot);
}

void mg_resource_slot_recycle(mg_resource_pool* pool, mg_resource_slot* slot)
{
	DEBUG_ASSERT(slot >= pool->slots && slot < pool->slots + MG_MAX_RESOURCE_SLOTS);
	#ifdef DEBUG
		if(slot->generation == UINT32_MAX)
		{
			LOG_ERROR("surface slot generation wrap around\n");
		}
	#endif
	slot->generation++;
	ListPush(&pool->freeList, &slot->freeListElt);
}

mg_resource_slot* mg_resource_slot_from_handle(mg_resource_pool* pool, u64 h)
{
	u32 index = h>>32;
	u32 generation = h & 0xffffffff;
	if(index >= MG_MAX_RESOURCE_SLOTS)
	{
		return(0);
	}
	mg_resource_slot* slot = &pool->slots[index];
	if(slot->generation != generation)
	{
		return(0);
	}
	else
	{
		return(slot);
	}
}

u64 mg_resource_handle_from_slot(mg_resource_pool* pool, mg_resource_slot* slot)
{
	DEBUG_ASSERT(  (slot - pool->slots) >= 0
	            && (slot - pool->slots) < MG_MAX_RESOURCE_SLOTS);

	u64 h = ((u64)(slot - pool->slots))<<32
	       |((u64)(slot->generation));
	return(h);
}

//------------------------------------------------------------------------
// surface handles
//------------------------------------------------------------------------

mg_surface mg_surface_nil()
{
	return((mg_surface){.h = 0});
}

mg_surface mg_surface_alloc_handle(mg_surface_info* surface)
{
	mg_resource_slot* slot = mg_resource_slot_alloc(&__mgInfo.surfaces);
	if(!slot)
	{
		LOG_ERROR("no more surface slots\n");
		return(mg_surface_nil());
	}
	slot->surface = surface;
	u64 h = mg_resource_handle_from_slot(&__mgInfo.surfaces, slot);
	mg_surface handle = {h};
	return(handle);
}

mg_surface_info* mg_surface_ptr_from_handle(mg_surface surface)
{
	mg_resource_slot* slot = mg_resource_slot_from_handle(&__mgInfo.surfaces, surface.h);
	if(slot)
	{
		return(slot->surface);
	}
	else
	{
		return(0);
	}
}

//------------------------------------------------------------------------
// surface server handles
//------------------------------------------------------------------------

mg_surface_server mg_surface_server_nil()
{
	return((mg_surface_server){.h = 0});
}

mg_surface_server mg_surface_server_alloc_handle(mg_surface_server_info* server)
{
	mg_resource_slot* slot = mg_resource_slot_alloc(&__mgInfo.servers);
	if(!slot)
	{
		LOG_ERROR("no more server slots\n");
		return(mg_surface_server_nil());
	}
	slot->server = server;
	u64 h = mg_resource_handle_from_slot(&__mgInfo.servers, slot);
	mg_surface_server handle = {h};
	return(handle);
}

mg_surface_server_info* mg_surface_server_ptr_from_handle(mg_surface_server server)
{
	mg_resource_slot* slot = mg_resource_slot_from_handle(&__mgInfo.servers, server.h);
	if(slot)
	{
		return(slot->server);
	}
	else
	{
		return(0);
	}
}

//------------------------------------------------------------------------
// surface client handles
//------------------------------------------------------------------------
mg_surface_client mg_surface_client_nil()
{
	return((mg_surface_client){.h = 0});
}

mg_surface_client mg_surface_client_alloc_handle(mg_surface_client_info* client)
{
	mg_resource_slot* slot = mg_resource_slot_alloc(&__mgInfo.clients);
	if(!slot)
	{
		LOG_ERROR("no more client slots\n");
		return(mg_surface_client_nil());
	}
	slot->client = client;
	u64 h = mg_resource_handle_from_slot(&__mgInfo.clients, slot);
	mg_surface_client handle = {h};
	return(handle);
}

mg_surface_client_info* mg_surface_client_ptr_from_handle(mg_surface_client client)
{
	mg_resource_slot* slot = mg_resource_slot_from_handle(&__mgInfo.clients, client.h);
	if(slot)
	{
		return(slot->client);
	}
	else
	{
		return(0);
	}
}

//------------------------------------------------------------------------
// canvas
//------------------------------------------------------------------------
mg_canvas_data* mg_canvas_alloc()
{
	return(ListPopEntry(&__mgInfo.contextFreeList, mg_canvas_data, freeListElt));
}

void mg_canvas_recycle(mg_canvas_data* context)
{
	#ifdef DEBUG
		if(context->generation == UINT32_MAX)
		{
			LOG_ERROR("graphics context generation wrap around\n");
		}
	#endif
	context->generation++;
	ListPush(&__mgInfo.contextFreeList, &context->freeListElt);
}

mg_canvas_data* mg_canvas_ptr_from_handle(mg_canvas handle)
{
	u32 index = handle.h>>32;
	u32 generation = handle.h & 0xffffffff;
	if(index >= MG_MAX_CONTEXTS)
	{
		return(0);
	}
	mg_canvas_data* context = &__mgInfo.contexts[index];
	if(context->generation != generation)
	{
		return(0);
	}
	else
	{
		return(context);
	}
}

mg_canvas mg_canvas_handle_from_ptr(mg_canvas_data* context)
{
	DEBUG_ASSERT(  (context - __mgInfo.contexts) >= 0
	            && (context - __mgInfo.contexts) < MG_MAX_CONTEXTS);

	u64 h = ((u64)(context - __mgInfo.contexts))<<32
	       |((u64)(context->generation));
	return((mg_canvas){.h = h});
}

mg_canvas mg_canvas_nil()
{
	return((mg_canvas){.h = 0});
}

mg_image_data* mg_image_ptr_from_handle(mg_canvas_data* context, mg_image handle)
{
	u32 index = handle.h>>32;
	u32 generation = handle.h & 0xffffffff;

	if(index >= MG_IMAGE_MAX_COUNT)
	{
		return(0);
	}
	mg_image_data* image = &context->images[index];
	if(image->generation != generation)
	{
		return(0);
	}
	else
	{
		return(image);
	}
}

//---------------------------------------------------------------
// graphics surface API
//---------------------------------------------------------------

#ifdef MG_IMPLEMENTS_BACKEND_METAL
	//NOTE: function is defined in metal_backend.mm
	mg_surface mg_metal_surface_create_for_window(mp_window window);
	mg_surface mg_metal_surface_create_for_view(mp_view view);
#endif //MG_IMPLEMENTS_BACKEND_METAL

#ifdef MG_IMPLEMENTS_BACKEND_GLES
	mg_surface mg_gles_surface_create_offscreen(u32 width, u32 height);
	mg_surface_server mg_gles_surface_create_server(mg_surface_info* surface);
#endif //MG_IMPLEMENTS_BACKEND_GLES

void mg_init();

mg_surface mg_surface_create_for_window(mp_window window, mg_backend_id backend)
{
	DEBUG_ASSERT(__mgInfo.init);

	mg_surface surface = mg_surface_nil();

	switch(backend)
	{
		#ifdef MG_IMPLEMENTS_BACKEND_METAL
			case MG_BACKEND_METAL:
				surface = mg_metal_surface_create_for_window(window);
				break;
		#endif

		//...

			default:
				break;
	}

	return(surface);
}

mg_surface mg_surface_create_for_view(mp_view view, mg_backend_id backend)
{
	DEBUG_ASSERT(__mgInfo.init);

	mg_surface surface = mg_surface_nil();

	switch(backend)
	{
		#ifdef MG_IMPLEMENTS_BACKEND_METAL
			case MG_BACKEND_METAL:
				surface = mg_metal_surface_create_for_view(view);
				break;
		#endif

		//...

			default:
				break;
	}

	return(surface);
}

mg_surface mg_surface_create_offscreen(mg_backend_id backend, u32 width, u32 height)
{
	DEBUG_ASSERT(__mgInfo.init);

	mg_surface surface = mg_surface_nil();

	switch(backend)
	{
		#ifdef MG_IMPLEMENTS_BACKEND_GLES
			case MG_BACKEND_GLES:
				surface = mg_gles_surface_create_offscreen(width, height);
				break;
		#endif

		//...

			default:
				break;
	}

	return(surface);
}

void* mg_surface_get_os_resource(mg_surface surface)
{
	DEBUG_ASSERT(__mgInfo.init);

	void* res = 0;
	mg_resource_slot* slot = mg_resource_slot_from_handle(&__mgInfo.surfaces, surface.h);
	if(slot)
	{
		res = slot->surface->getOSResource(slot->surface);
	}
	return(res);
}

void mg_surface_destroy(mg_surface handle)
{
	DEBUG_ASSERT(__mgInfo.init);

	mg_resource_slot* slot = mg_resource_slot_from_handle(&__mgInfo.surfaces, handle.h);
	if(slot)
	{
		slot->surface->destroy(slot->surface);
		mg_resource_slot_recycle(&__mgInfo.surfaces, slot);
	}
}

void mg_surface_resize(mg_surface surface, int width, int height)
{
	DEBUG_ASSERT(__mgInfo.init);

	mg_surface_info* surfaceInfo = mg_surface_ptr_from_handle(surface);
	if(surfaceInfo)
	{
		surfaceInfo->resize(surfaceInfo, width, height);
	}
}

void mg_surface_prepare(mg_surface surface)
{
	DEBUG_ASSERT(__mgInfo.init);

	mg_surface_info* surfaceInfo = mg_surface_ptr_from_handle(surface);
	if(surfaceInfo)
	{
		surfaceInfo->prepare(surfaceInfo);
	}
}

void mg_surface_present(mg_surface surface)
{
	DEBUG_ASSERT(__mgInfo.init);

	mg_surface_info* surfaceInfo = mg_surface_ptr_from_handle(surface);
	if(surfaceInfo)
	{
		surfaceInfo->present(surfaceInfo);
	}
}

void mg_surface_set_hidden(mg_surface surface, bool hidden)
{
	DEBUG_ASSERT(__mgInfo.init);

	mg_surface_info* surfaceInfo = mg_surface_ptr_from_handle(surface);
	if(surfaceInfo)
	{
		surfaceInfo->setHidden(surfaceInfo, hidden);
	}
}


vec2 mg_surface_size(mg_surface surface)
{
	DEBUG_ASSERT(__mgInfo.init);
	vec2 res = {};
	mg_surface_info* surfaceInfo = mg_surface_ptr_from_handle(surface);
	if(surfaceInfo)
	{
		res = surfaceInfo->getSize(surfaceInfo);
	}
	return(res);
}

//---------------------------------------------------------------
// graphics surface server
//---------------------------------------------------------------

mg_surface_server mg_gles_surface_server_create_native(void* p);

mg_surface_server mg_surface_server_create_native(void* p)
{
	return(mg_gles_surface_server_create_native(p));
}

mg_surface_server mg_surface_server_create(mg_surface surface)
{
	mg_surface_server server = mg_surface_server_nil();

	mg_surface_info* surfaceInfo = mg_surface_ptr_from_handle(surface);
	if(surfaceInfo)
	{
		switch(surfaceInfo->backend)
		{
			case MG_BACKEND_GLES:
				server = mg_gles_surface_create_server(surfaceInfo);
				break;

			default:
				break;
		}
	}
	return(server);
}

void mg_surface_server_destroy(mg_surface_server handle)
{
	mg_resource_slot* slot = mg_resource_slot_from_handle(&__mgInfo.servers, handle.h);
	if(slot)
	{
		slot->server->destroy(slot->server);
		mg_resource_slot_recycle(&__mgInfo.servers, slot);
	}
}

mg_surface_server_id mg_surface_server_get_id(mg_surface_server server)
{
	mg_surface_server_info* serverInfo = mg_surface_server_ptr_from_handle(server);
	if(serverInfo)
	{
		return(serverInfo->getID(serverInfo));
	}
	else
	{
		return(0);
	}
}

//---------------------------------------------------------------
// graphics surface client
//---------------------------------------------------------------

//TODO: move elsewhere, guard with OS ifdef
mg_surface_client mg_osx_surface_client_create(mg_surface_server_id id);

mg_surface_client mg_surface_client_create(mg_surface_server_id id)
{
	mg_surface_client client = mg_surface_client_nil();

	client = mg_osx_surface_client_create(id);
	return(client);
}

void mg_surface_client_destroy(mg_surface_client handle)
{
	mg_resource_slot* slot = mg_resource_slot_from_handle(&__mgInfo.clients, handle.h);
	if(slot)
	{
		slot->client->destroy(slot->client);
		mg_resource_slot_recycle(&__mgInfo.clients, slot);
	}
}

void mg_surface_client_attach_to_view(mg_surface_client client, mp_view view)
{
	mg_surface_client_info* clientInfo = mg_surface_client_ptr_from_handle(client);

	if(clientInfo)
	{
		clientInfo->attachment = view;
		clientInfo->attach(clientInfo);
	}
}

void mg_surface_client_detach(mg_surface_client client)
{
	mg_surface_client_info* clientInfo = mg_surface_client_ptr_from_handle(client);
	if(clientInfo)
	{
		clientInfo->detach(clientInfo);
	}
}

//---------------------------------------------------------------
// graphics stream handles
//---------------------------------------------------------------
/*
	Graphics command stream handles are invalidated when the command stream is appended to the current stream,
	and at the end of the frame.
	Thus command streams handles contain the index of the stream, the frame of the stream, and a generation
	count inside that frame:

	0                                  40                       56           64
	+---------------------------------+------------------------+------------+
	|          frameCounter           |      generation        |    index   |
	+---------------------------------+------------------------+------------+
	               40 bits                     16 bits              8 bits

	(This gives use 2^40 frames, ie ~600 years at 60Hz, 65536 possible reuse per frames and max 256 simultaneous streams.)

	This way we can invalidate use of old handles in the same frame by incrementing the generation counter of the recycled stream,
	and we can invalidate all previous handles at the end of a frame by just incrementing the frame counter of the graphics context.
*/
mg_stream_data* mg_stream_alloc(mg_canvas_data* context)
{
	mg_stream_data* stream = ListPopEntry(&context->streamsFreeList, mg_stream_data, freeListElt);
	if(stream)
	{
		stream->frame = context->frameCounter;
		stream->pendingJump = false;
		stream->count = 0;
		stream->firstCommand = 0;
		stream->lastCommand = 0;
	}
	return(stream);
}

void mg_stream_recycle(mg_canvas_data* context, mg_stream_data* stream)
{
	#ifdef DEBUG
		if(stream->generation == UINT32_MAX)
		{
			LOG_ERROR("graphics command stream generation wrap around\n");
		}
	#endif
	stream->generation++;
	ListPush(&context->streamsFreeList, &stream->freeListElt);
}

mg_stream_data* mg_stream_ptr_from_handle(mg_canvas_data* context, mg_stream handle)
{
	u32 index = handle.h>>56;
	u32 generation = (handle.h>>40) & 0xffff;
	u64 frame = handle.h & 0xffffffffff;

	if(index >= MG_STREAM_MAX_COUNT)
	{
		return(0);
	}
	mg_stream_data* stream = &context->streams[index];
	if( stream->generation != generation
	  ||stream->frame != context->frameCounter)
	{
		return(0);
	}
	else
	{
		return(stream);
	}
}

mg_stream mg_stream_handle_from_ptr(mg_canvas_data* context, mg_stream_data* stream)
{
	DEBUG_ASSERT(  (stream - context->streams) >= 0
	            && (stream - context->streams) < MG_STREAM_MAX_COUNT);

	u64 h = ((u64)(stream - context->streams))<<56
	       |((u64)(stream->generation))<<40
	       |((u64)(context->frameCounter));

	return((mg_stream){.h = h});
}

mg_stream mg_stream_null_handle()
{
	return((mg_stream){.h = 0});
}

//---------------------------------------------------------------
// internal graphics context functions
//---------------------------------------------------------------

mg_mat2x3 mg_mat2x3_mul_m(mg_mat2x3 lhs, mg_mat2x3 rhs)
{
	mg_mat2x3 res;
	res.m[0] = lhs.m[0]*rhs.m[0] + lhs.m[1]*rhs.m[3];
	res.m[1] = lhs.m[0]*rhs.m[1] + lhs.m[1]*rhs.m[4];
	res.m[2] = lhs.m[0]*rhs.m[2] + lhs.m[1]*rhs.m[5] + lhs.m[2];
	res.m[3] = lhs.m[3]*rhs.m[0] + lhs.m[4]*rhs.m[3];
	res.m[4] = lhs.m[3]*rhs.m[1] + lhs.m[4]*rhs.m[4];
	res.m[5] = lhs.m[3]*rhs.m[2] + lhs.m[4]*rhs.m[5] + lhs.m[5];

	return(res);
}

vec2 mg_mat2x3_mul(mg_mat2x3 m, vec2 p)
{
	f32 x = p.x*m.m[0] + p.y*m.m[1] + m.m[2];
	f32 y = p.x*m.m[3] + p.y*m.m[4] + m.m[5];
	return((vec2){x, y});
}

void mg_reset_z_index(mg_canvas_data* context)
{
	context->nextZIndex = 1;
}

u32 mg_get_next_z_index(mg_canvas_data* context)
{
	return(context->nextZIndex++);
}

///////////////////////////////////////  WIP  /////////////////////////////////////////////////////////////////////////
u32 mg_vertices_base_index(mg_canvas_data* context)
{
	return(context->vertexCount);
}

void mg_push_textured_vertex(mg_canvas_data* context, vec2 pos, vec4 cubic, vec2 uv, mg_color color, u64 zIndex)
{
	mgc_vertex_layout* layout = &context->painter->vertexLayout;

	DEBUG_ASSERT(context->vertexCount < layout->maxVertexCount);

	u32 offset = context->vertexCount;

	*(vec2*)(((char*)layout->posBuffer) + offset*layout->posStride) = pos;
	*(vec4*)(((char*)layout->cubicBuffer) + offset*layout->cubicStride) = cubic;
	*(vec2*)(((char*)layout->uvBuffer) + offset*layout->uvStride) = uv;
	*(mg_color*)(((char*)layout->colorBuffer) + offset*layout->colorStride) = color;
	*(u32*)(((char*)layout->zIndexBuffer) + offset*layout->zIndexStride) = zIndex;
	*(mp_rect*)(((char*)layout->clipsBuffer) + offset*layout->clipsStride) = context->clip;

	context->vertexCount++;
}

void mg_push_vertex(mg_canvas_data* context, vec2 pos, vec4 cubic, mg_color color, u64 zIndex)
{
	mg_push_textured_vertex(context, pos, cubic, (vec2){0, 0}, color, zIndex);
}


///////////////////////////////////////  WIP  /////////////////////////////////////////////////////////////////////////

int* mg_reserve_indices(mg_canvas_data* context, u32 indexCount)
{
	mgc_vertex_layout* layout = &context->painter->vertexLayout;

	ASSERT(context->indexCount + indexCount < layout->maxIndexCount);
	int* base = ((int*)layout->indexBuffer) + context->indexCount;
	context->indexCount += indexCount;
	return(base);
}

//-----------------------------------------------------------------------------------------------------------
// Path Filling
//-----------------------------------------------------------------------------------------------------------
//NOTE(martin): forward declarations
void mg_render_fill_cubic(mg_canvas_data* context, vec2 p[4], u32 zIndex, mg_color color);

//NOTE(martin): quadratics filling

void mg_render_fill_quadratic(mg_canvas_data* context, vec2 p[3], u32 zIndex, mg_color color)
{
	u32 baseIndex = mg_vertices_base_index(context);

	i32* indices = mg_reserve_indices(context, 3);

	mg_push_vertex(context,
	               mg_mat2x3_mul(context->transform, (vec2){p[0].x, p[0].y}),
	               (vec4){0, 0, 0, 1},
	               color,
	               zIndex);

	mg_push_vertex(context,
	               mg_mat2x3_mul(context->transform, (vec2){p[1].x, p[1].y}),
	               (vec4){0.5, 0, 0.5, 1},
	               color,
	               zIndex);

	mg_push_vertex(context,
	               mg_mat2x3_mul(context->transform, (vec2){p[2].x, p[2].y}),
	               (vec4){1, 1, 1, 1},
	               color,
	               zIndex);

	indices[0] = baseIndex + 0;
	indices[1] = baseIndex + 1;
	indices[2] = baseIndex + 2;
}

//NOTE(martin): cubic filling

void mg_split_and_fill_cubic(mg_canvas_data* context, vec2 p[4], f32 tSplit, u32 zIndex, mg_color color)
{
	int subVertexCount = 0;
	int subIndexCount = 0;

	f32 OneMinusTSplit = 1-tSplit;

	vec2 q0 = {OneMinusTSplit*p[0].x + tSplit*p[1].x,
		   OneMinusTSplit*p[0].y + tSplit*p[1].y};

	vec2 q1 = {OneMinusTSplit*p[1].x + tSplit*p[2].x,
		   OneMinusTSplit*p[1].y + tSplit*p[2].y};

	vec2 q2 = {OneMinusTSplit*p[2].x + tSplit*p[3].x,
		   OneMinusTSplit*p[2].y + tSplit*p[3].y};

	vec2 r0 = {OneMinusTSplit*q0.x + tSplit*q1.x,
		   OneMinusTSplit*q0.y + tSplit*q1.y};

	vec2 r1 = {OneMinusTSplit*q1.x + tSplit*q2.x,
		   OneMinusTSplit*q1.y + tSplit*q2.y};

	vec2 split = {OneMinusTSplit*r0.x + tSplit*r1.x,
		     OneMinusTSplit*r0.y + tSplit*r1.y};;

	vec2 subPointsLow[4] = {p[0], q0, r0, split};
	vec2 subPointsHigh[4] = {split, r1, q2, p[3]};

	//NOTE(martin): add base triangle
	u32 baseIndex = mg_vertices_base_index(context);
	i32* indices = mg_reserve_indices(context, 3);

	mg_push_vertex(context,
	                mg_mat2x3_mul(context->transform, (vec2){p[0].x, p[0].y}),
	                (vec4){1, 1, 1, 1},
	                color,
	                zIndex);

	mg_push_vertex(context,
	               mg_mat2x3_mul(context->transform, (vec2){split.x, split.y}),
	               (vec4){1, 1, 1, 1},
	               color,
	               zIndex);

	mg_push_vertex(context,
	               mg_mat2x3_mul(context->transform, (vec2){p[3].x, p[3].y}),
	               (vec4){1, 1, 1, 1},
	               color,
	               zIndex);

	indices[0] = baseIndex + 0;
	indices[1] = baseIndex + 1;
	indices[2] = baseIndex + 2;

	mg_render_fill_cubic(context, subPointsLow, zIndex, color);
	mg_render_fill_cubic(context, subPointsHigh, zIndex, color);

	return;
}

void mg_render_fill_cubic(mg_canvas_data* context, vec2 p[4], u32 zIndex, mg_color color)
{
	LOG_DEBUG("graphics render fill cubic\n");

	vec4 testCoords[4];

	/*NOTE(martin): first convert the control points to power basis, multiplying by M3

		     | 1  0  0  0|
		M3 = |-3  3  0  0|
		     | 3 -6  3  0|
		     |-1  3 -3  1|
		ie:
		    c0 = p0
		    c1 = -3*p0 + 3*p1
		    c2 = 3*p0 - 6*p1 + 3*p2
		    c3 = -p0 + 3*p1 - 3*p2 + p3
	*/
	f32 c1x = 3.0*p[1].x - 3.0*p[0].x;
	f32 c1y = 3.0*p[1].y - 3.0*p[0].y;

	f32 c2x = 3.0*p[0].x + 3.0*p[2].x - 6.0*p[1].x;
	f32 c2y = 3.0*p[0].y + 3.0*p[2].y - 6.0*p[1].y;

	f32 c3x = 3.0*p[1].x - 3.0*p[2].x + p[3].x - p[0].x;
	f32 c3y = 3.0*p[1].y - 3.0*p[2].y + p[3].y - p[0].y;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//TODO(martin): we should do the tex coords computations in f64 and scale them to avoid f32 precision/range glitches in shader
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	c1x /= 10;
	c1y /= 10;
	c2x /= 10;
	c2y /= 10;
	c3x /= 10;
	c3y /= 10;

	/*NOTE(martin):
		now, compute determinants d0, d1, d2, d3, which gives the coefficients of the
	        inflection points polynomial:

		I(t, s) = d0*t^3 - 3*d1*t^2*s + 3*d2*t*s^2 - d3*s^3

		The roots of this polynomial are the inflection points of the parametric curve, in homogeneous
		coordinates (ie we can have an inflection point at inifinity with s=0).

		         |x3 y3 w3|              |x3 y3 w3|             |x3 y3 w3|              |x2 y2 w2|
		d0 = det |x2 y2 w2|    d1 = -det |x2 y2 w2|    d2 = det |x1 y1 w1|    d3 = -det |x1 y1 w1|
		         |x1 y1 w1|              |x0 y0 w0|             |x0 y0 w0|              |x0 y0 w0|

		In our case, the pi.w equal 1 (no point at infinity), so _in_the_power_basis_, w1 = w2 = w3 = 0 and w0 = 1
		(which also means d0 = 0)
	*/

	f32 d1 = c3y*c2x - c3x*c2y;
	f32 d2 = c3x*c1y - c3y*c1x;
	f32 d3 = c2y*c1x - c2x*c1y;

	//NOTE(martin): compute the second factor of the discriminant discr(I) = d1^2*(3*d2^2 - 4*d3*d1)
	f32 discrFactor2 = 3.0*Square(d2) - 4.0*d3*d1;

	//NOTE(martin): each following case gives the number of roots, hence the category of the parametric curve
	if(fabs(d1) < 0.1 && fabs(d2) < 0.1 && d3 != 0)
	{
		//NOTE(martin): quadratic degenerate case
		LOG_DEBUG("quadratic curve\n");

		//NOTE(martin): compute quadratic curve control point, which is at p0 + 1.5*(p1-p0) = 1.5*p1 - 0.5*p0
		vec2 quadControlPoints[3] = { p[0],
		                             {1.5*p[1].x - 0.5*p[0].x, 1.5*p[1].y - 0.5*p[0].y},
				  	     p[3]};

		mg_render_fill_quadratic(context, quadControlPoints, zIndex, color);
		return;
	}
	else if( (discrFactor2 > 0 && d1 != 0)
	  ||(discrFactor2 == 0 && d1 != 0))
	{
		//NOTE(martin): serpentine curve or cusp with inflection at infinity
		//              (these two cases are handled the same way).
		LOG_DEBUG("%s\n", (discrFactor2 > 0 && d1 != 0) ? "serpentine curve" : "cusp with inflection at infinity");

		//NOTE(martin): compute the solutions (tl, sl), (tm, sm), and (tn, sn) of the inflection point equation
		f32 tl = d2 + sqrt(discrFactor2/3);
		f32 sl = 2*d1;
		f32 tm = d2 - sqrt(discrFactor2/3);
		f32 sm = sl;

		/*NOTE(martin):
			the power basis coefficients of points k,l,m,n are collected into the rows of the 4x4 matrix F:

				| tl*tm            tl^3        tm^3        1 |
				| -sm*tl - sl*tm   -3sl*tl^2   -3*sm*tm^2  0 |
				| sl*sm            3*sl^2*tl   3*sm^2*tm   0 |
				| 0                -sl^3       -sm^3       0 |

			This matrix is then multiplied by M3^(-1) on the left which yelds the bezier coefficients of k, l, m, n
			which are assigned as a 4D texture coordinates to control points.


			                   | 1  0   0   0 |
				M3^(-1) =  | 1  1/3 0   0 |
				           | 1  2/3 1/3 0 |
					   | 1  1   1   1 |
		*/
		testCoords[0].x = tl*tm;
		testCoords[0].y = Cube(tl);
		testCoords[0].z = Cube(tm);

		testCoords[1].x = tl*tm - (sm*tl + sl*tm)/3;
		testCoords[1].y = Cube(tl) - sl*Square(tl);
		testCoords[1].z = Cube(tm) - sm*Square(tm);

		testCoords[2].x = tl*tm - (sm*tl + sl*tm)*2/3 + sl*sm/3;
		testCoords[2].y = Cube(tl) - 2*sl*Square(tl) + Square(sl)*tl;
		testCoords[2].z = Cube(tm) - 2*sm*Square(tm) + Square(sm)*tm;

		testCoords[3].x = tl*tm - (sm*tl + sl*tm) + sl*sm;
		testCoords[3].y = Cube(tl) - 3*sl*Square(tl) + 3*Square(sl)*tl - Cube(sl);
		testCoords[3].z = Cube(tm) - 3*sm*Square(tm) + 3*Square(sm)*tm - Cube(sm);
	}
	else if(discrFactor2 < 0 && d1 != 0)
	{
		//NOTE(martin): loop curve
		LOG_DEBUG("loop curve\n");

		f32 td = d2 + sqrt(-discrFactor2);
		f32 sd = 2*d1;
		f32 te = d2 - sqrt(-discrFactor2);
		f32 se = sd;

		//NOTE(martin): if one of the parameters (td/sd) or (te/se) is in the interval [0,1], the double point
		//              is inside the control points convex hull and would cause a shading anomaly. If this is
		//              the case, subdivide the curve at that point

		//TODO: study edge case where td/sd ~ 1 or 0 (which causes an infinite recursion in split and fill).
		//      quick fix for now is adding a little slop in the check...

		if(sd != 0 && td/sd < 0.99 && td/sd > 0.01)
		{
			LOG_DEBUG("split curve at first double point\n");
			mg_split_and_fill_cubic(context, p, td/sd, zIndex, color);
			return;
		}
		if(se != 0 && te/se < 0.99 && te/se > 0.01)
		{
			LOG_DEBUG("split curve at second double point\n");
			mg_split_and_fill_cubic(context, p, te/se, zIndex, color);
			return;
		}

		/*NOTE(martin):
			the power basis coefficients of points k,l,m,n are collected into the rows of the 4x4 matrix F:

				| td*te            td^2*te                 td*te^2                1 |
				| -se*td - sd*te   -se*td^2 - 2sd*te*td    -sd*te^2 - 2*se*td*te  0 |
				| sd*se            te*sd^2 + 2*se*td*sd    td*se^2 + 2*sd*te*se   0 |
				| 0                -sd^2*se                -sd*se^2               0 |

			This matrix is then multiplied by M3^(-1) on the left which yelds the bezier coefficients of k, l, m, n
			which are assigned as a 4D texture coordinates to control points.


			                   | 1  0   0   0 |
				M3^(-1) =  | 1  1/3 0   0 |
				           | 1  2/3 1/3 0 |
					   | 1  1   1   1 |
		*/
		testCoords[0].x = td*te;
		testCoords[0].y = Square(td)*te;
		testCoords[0].z = td*Square(te);

		testCoords[1].x = td*te - (se*td + sd*te)/3.0;
		testCoords[1].y = Square(td)*te - (se*Square(td) + 2.*sd*te*td)/3.0;
		testCoords[1].z = td*Square(te) - (sd*Square(te) + 2*se*td*te)/3.0;

		testCoords[2].x = td*te - 2.0*(se*td + sd*te)/3.0 + sd*se/3.0;
		testCoords[2].y = Square(td)*te - 2.0*(se*Square(td) + 2.0*sd*te*td)/3.0 + (te*Square(sd) + 2.0*se*td*sd)/3.0;
		testCoords[2].z = td*Square(te) - 2.0*(sd*Square(te) + 2.0*se*td*te)/3.0 + (td*Square(se) + 2.0*sd*te*se)/3.0;

		testCoords[3].x = td*te - (se*td + sd*te) + sd*se;
		testCoords[3].y = Square(td)*te - (se*Square(td) + 2.0*sd*te*td) + (te*Square(sd) + 2.0*se*td*sd) - Square(sd)*se;
		testCoords[3].z = td*Square(te) - (sd*Square(te) + 2.0*se*td*te) + (td*Square(se) + 2.0*sd*te*se) - sd*Square(se);
	}
	else if(d1 == 0 && d2 != 0)
	{
		//NOTE(martin): cusp with cusp at infinity
		LOG_DEBUG("cusp at infinity curve\n");

		f32 tl = d3;
		f32 sl = 3*d2;

		/*NOTE(martin):
			the power basis coefficients of points k,l,m,n are collected into the rows of the 4x4 matrix F:

				| tl    tl^3        1  1 |
				| -sl   -3sl*tl^2   0  0 |
				| 0     3*sl^2*tl   0  0 |
				| 0     -sl^3       0  0 |

			This matrix is then multiplied by M3^(-1) on the left which yelds the bezier coefficients of k, l, m, n
			which are assigned as a 4D texture coordinates to control points.


			                   | 1  0   0   0 |
				M3^(-1) =  | 1  1/3 0   0 |
				           | 1  2/3 1/3 0 |
					   | 1  1   1   1 |
		*/

		testCoords[0].x = tl;
		testCoords[0].y = Cube(tl);
		testCoords[0].z = 1;

		testCoords[1].x = tl - sl/3;
		testCoords[1].y = Cube(tl) - sl*Square(tl);
		testCoords[1].z = 1;

		testCoords[2].x = tl - sl*2/3;
		testCoords[2].y = Cube(tl) - 2*sl*Square(tl) + Square(sl)*tl;
		testCoords[2].z = 1;

		testCoords[3].x = tl - sl;
		testCoords[3].y = Cube(tl) - 3*sl*Square(tl) + 3*Square(sl)*tl - Cube(sl);
		testCoords[3].z = 1;
	}
	else if(d1 == 0 && d2 == 0 && d3 == 0)
	{
		//NOTE(martin): line or point degenerate case, ignored
		LOG_DEBUG("line or point curve (ignored)\n");
		return;
	}
	else
	{
		//TODO(martin): handle error ? put some epsilon slack on the conditions ?
		LOG_DEBUG("none of the above...\n");
		ASSERT(0, "not implemented yet !");
		return;
	}

	//NOTE(martin): compute convex hull indices using Gift wrapping / Jarvis' march algorithm
	int convexHullIndices[4];
	int leftMostPointIndex = 0;

	for(int i=0; i<4; i++)
	{
		if(p[i].x < p[leftMostPointIndex].x)
		{
			leftMostPointIndex = i;
		}
	}
	int currentPointIndex = leftMostPointIndex;
	int i=0;
	int convexHullCount = 0;

	do
	{
		convexHullIndices[i] = currentPointIndex;
		convexHullCount++;
		int bestGuessIndex = 0;

		for(int j=0; j<4; j++)
		{
			vec2 bestGuessEdge = {.x = p[bestGuessIndex].x - p[currentPointIndex].x,
		                              .y = p[bestGuessIndex].y - p[currentPointIndex].y};

			vec2 nextGuessEdge = {.x = p[j].x - p[currentPointIndex].x,
		                              .y = p[j].y - p[currentPointIndex].y};

			//NOTE(martin): if control point j is on the right of current edge, it is a best guess
			//              (in case of colinearity we choose the point which is farthest from the current point)

			f32 crossProduct = bestGuessEdge.x*nextGuessEdge.y - bestGuessEdge.y*nextGuessEdge.x;

			if(  bestGuessIndex == currentPointIndex
			  || crossProduct < 0)
			{
				bestGuessIndex = j;
			}
			else if(crossProduct == 0)
			{

				//NOTE(martin): if vectors v1, v2 are colinear and distinct, and ||v1|| > ||v2||,
				//               either abs(v1.x) > abs(v2.x) or abs(v1.y) > abs(v2.y)
				//               so we don't actually need to compute their norm to select the greatest
				//               (and if v1 and v2 are equal we don't have to update our best guess.)

				//TODO(martin): in case of colinearity we should rather select the edge that has the greatest dot product with last edge ??

				if(fabs(nextGuessEdge.x) > fabs(bestGuessEdge.x)
				  || fabs(nextGuessEdge.y) > fabs(bestGuessEdge.y))
				{
					bestGuessIndex = j;
				}
			}
		}
		i++;
		currentPointIndex = bestGuessIndex;

	} while(currentPointIndex != leftMostPointIndex && i<4);

	//TODO: quick fix, maybe later cull degenerate hulls beforehand
	if(convexHullCount <= 2)
	{
		//NOTE(martin): if convex hull has only two point, we have a degenerate cubic that displays nothing.
		return;
	}

	//NOTE(martin): rearrange convex hull to put p0 first
	int startIndex = -1;
	int orderedHullIndices[4];
	for(int i=0; i<convexHullCount; i++)
	{
		if(convexHullIndices[i] == 0)
		{
			startIndex = i;
		}
		if(startIndex >= 0)
		{
			orderedHullIndices[i-startIndex] = convexHullIndices[i];
		}
	}
	for(int i=0; i<startIndex; i++)
	{
		orderedHullIndices[convexHullCount-startIndex+i] = convexHullIndices[i];
	}

	//NOTE(martin): inside/outside tests for the two triangles. In the shader, the outside is defined by s*(k^3 - lm) > 0
	//              ie the 4th coordinate s flips the inside/outside test.
	//              We affect s such that control points p1 and p2 are always outside the covered area.

	if(convexHullCount <= 3)
	{
		//NOTE(martin): the convex hull is a triangle

		//NOTE(martin): when we degenerate from 4 control points to a 3 points convex hull, this means one of the
		//              control points p1 or p2 could be inside the covered area. We want to compute the test for the
		//              control point which belongs to the convex hull, as we know it should be outside the covered area.
		//
		//              Since there are 3 points in the hull and p0 is the first, and p3 belongs to the hull, this means we
		//              must select the point of the convex hull which is neither the first, nor p3.

		int testPointIndex = orderedHullIndices[1] == 3 ? orderedHullIndices[2] : orderedHullIndices[1];
		int outsideTest = 1;
		if(Cube(testCoords[testPointIndex].x)-testCoords[testPointIndex].y*testCoords[testPointIndex].z < 0)
		{
			outsideTest = -1;
		}

		u32 baseIndex = mg_vertices_base_index(context);
		i32* indices = mg_reserve_indices(context, 3);

		for(int i=0; i<3; i++)
		{
			vec2 pos = mg_mat2x3_mul(context->transform, p[orderedHullIndices[i]]);
			vec4 cubic = testCoords[orderedHullIndices[i]];
			cubic.w = outsideTest;
			mg_push_vertex(context, pos, cubic, color, zIndex);

			indices[i] = baseIndex + i;
		}
	}
	else if(orderedHullIndices[2] == 3)
	{
		//NOTE(martin): p1 and p2 are not on the same side of (p0,p3). The outside test can be different for
		//              the two triangles
		int outsideTest1 = 1;
		int outsideTest2 = 1;

		int testIndex = orderedHullIndices[1];
		if(Cube(testCoords[testIndex].x)-testCoords[testIndex].y*testCoords[testIndex].z < 0)
		{
			outsideTest1 = -1;
		}

		testIndex = orderedHullIndices[3];
		if(Cube(testCoords[testIndex].x)-testCoords[testIndex].y*testCoords[testIndex].z < 0)
		{
			outsideTest2 = -1;
		}

		u32 baseIndex = mg_vertices_base_index(context);
		i32* indices = mg_reserve_indices(context, 6);

		mg_push_vertex(context,
		               mg_mat2x3_mul(context->transform, p[orderedHullIndices[0]]),
		               (vec4){vec4_expand_xyz(testCoords[orderedHullIndices[0]]), outsideTest1},
		               color,
		               zIndex);

		mg_push_vertex(context,
		               mg_mat2x3_mul(context->transform, p[orderedHullIndices[1]]),
		               (vec4){vec4_expand_xyz(testCoords[orderedHullIndices[1]]), outsideTest1},
		               color,
		               zIndex);

		mg_push_vertex(context,
		               mg_mat2x3_mul(context->transform, p[orderedHullIndices[2]]),
		               (vec4){vec4_expand_xyz(testCoords[orderedHullIndices[2]]), outsideTest1},
		               color,
		               zIndex);

		mg_push_vertex(context,
		               mg_mat2x3_mul(context->transform, p[orderedHullIndices[0]]),
		               (vec4){vec4_expand_xyz(testCoords[orderedHullIndices[0]]), outsideTest2},
		               color,
		               zIndex);

		mg_push_vertex(context,
		               mg_mat2x3_mul(context->transform, p[orderedHullIndices[2]]),
		               (vec4){vec4_expand_xyz(testCoords[orderedHullIndices[2]]), outsideTest2},
		               color,
		               zIndex);

		mg_push_vertex(context,
		               mg_mat2x3_mul(context->transform, p[orderedHullIndices[3]]),
		               (vec4){vec4_expand_xyz(testCoords[orderedHullIndices[3]]), outsideTest2},
		               color,
		               zIndex);

		indices[0] = baseIndex + 0;
		indices[1] = baseIndex + 1;
		indices[2] = baseIndex + 2;
		indices[3] = baseIndex + 3;
		indices[4] = baseIndex + 4;
		indices[5] = baseIndex + 5;
	}
	else
	{
		//NOTE(martin): if p1 and p2 are on the same side of (p0,p3), the outside test is the same for both triangles
		int outsideTest = 1;
		if(Cube(testCoords[1].x)-testCoords[1].y*testCoords[1].z < 0)
		{
			outsideTest = -1;
		}

		u32 baseIndex = mg_vertices_base_index(context);
		i32* indices = mg_reserve_indices(context, 6);

		for(int i=0; i<4; i++)
		{
			mg_push_vertex(context,
		                   mg_mat2x3_mul(context->transform, p[orderedHullIndices[i]]),
		                   (vec4){vec4_expand_xyz(testCoords[orderedHullIndices[i]]), outsideTest},
		                   color,
		                   zIndex);
		}

		indices[0] = baseIndex + 0;
		indices[1] = baseIndex + 1;
		indices[2] = baseIndex + 2;
		indices[3] = baseIndex + 0;
		indices[4] = baseIndex + 2;
		indices[5] = baseIndex + 3;
	}
}

//NOTE(martin): global path fill

void mg_render_fill(mg_canvas_data* context, mg_path_elt* elements, mg_path_descriptor* path, u32 zIndex, mg_color color)
{
	u32 eltCount = path->count;
	vec2 startPoint = path->startPoint;
	vec2 endPoint = path->startPoint;
	vec2 currentPoint = path->startPoint;

	for(int eltIndex=0; eltIndex<eltCount; eltIndex++)
	{
		mg_path_elt* elt = &(elements[eltIndex]);

		vec2 controlPoints[4] = {currentPoint, elt->p[0], elt->p[1], elt->p[2]};

		switch(elt->type)
		{
			case MG_PATH_MOVE:
			{
				startPoint = elt->p[0];
				endPoint = elt->p[0];
				currentPoint = endPoint;
				continue;
			} break;

			case MG_PATH_LINE:
			{
				endPoint = controlPoints[1];
			} break;

			case MG_PATH_QUADRATIC:
			{
				mg_render_fill_quadratic(context, controlPoints, zIndex, color);
				endPoint = controlPoints[2];

			} break;

			case MG_PATH_CUBIC:
			{
				mg_render_fill_cubic(context, controlPoints, zIndex, color);
				endPoint = controlPoints[3];
			} break;
		}

		//NOTE(martin): now fill interior triangle
		u32 baseIndex = mg_vertices_base_index(context);
		int* indices = mg_reserve_indices(context, 3);

		vec2 pos[3];
		pos[0] = mg_mat2x3_mul(context->transform, startPoint);
		pos[1] = mg_mat2x3_mul(context->transform, currentPoint);
		pos[2] = mg_mat2x3_mul(context->transform, endPoint);

		vec4 cubic = {1, 1, 1, 1};

		for(int i=0; i<3; i++)
		{
			mg_push_vertex(context, pos[i], cubic, color, zIndex);
			indices[i] = baseIndex + i;
		}

		currentPoint = endPoint;
	}
}

//-----------------------------------------------------------------------------------------------------------
// Path Stroking
//-----------------------------------------------------------------------------------------------------------

void mg_render_stroke_line(mg_canvas_data* context, vec2 p[2], u32 zIndex, mg_attributes* attributes)
{
	//NOTE(martin): get normals multiplied by halfWidth
	f32 halfW = attributes->width/2;

	vec2 n0 = {p[0].y - p[1].y,
		   p[1].x - p[0].x};
	f32 norm0 = sqrt(n0.x*n0.x + n0.y*n0.y);
	n0.x *= halfW/norm0;
	n0.y *= halfW/norm0;


	mg_color color = attributes->color;

	u32 baseIndex = mg_vertices_base_index(context);
	i32* indices = mg_reserve_indices(context, 6);

	mg_push_vertex(context,
	               mg_mat2x3_mul(context->transform, (vec2){p[0].x + n0.x, p[0].y + n0.y}),
	               (vec4){1, 1, 1, 1},
	               color,
	               zIndex);

	mg_push_vertex(context,
	               mg_mat2x3_mul(context->transform, (vec2){p[1].x + n0.x, p[1].y + n0.y}),
	               (vec4){1, 1, 1, 1},
	               color,
	               zIndex);

	mg_push_vertex(context,
	               mg_mat2x3_mul(context->transform, (vec2){p[1].x - n0.x, p[1].y - n0.y}),
	               (vec4){1, 1, 1, 1},
	               color,
	               zIndex);

	mg_push_vertex(context,
	               mg_mat2x3_mul(context->transform, (vec2){p[0].x - n0.x, p[0].y - n0.y}),
	               (vec4){1, 1, 1, 1},
	               color,
	               zIndex);

	indices[0] = baseIndex;
	indices[1] = baseIndex + 1;
	indices[2] = baseIndex + 2;
	indices[3] = baseIndex;
	indices[4] = baseIndex + 2;
	indices[5] = baseIndex + 3;
}

void mg_offset_hull(int count, vec2* p, vec2* result, f32 offset)
{
//////////////////////////////////////////////////////////////////////////////////////
//WARN: quick fix for coincident middle control points
	if(count == 4 && (p[1].x - p[2].x < 0.01) && (p[1].y - p[2].y < 0.01))
	{
		vec2 hull3[3] = {p[0], p[1], p[3]};
		vec2 result3[3];
		mg_offset_hull(3, hull3, result3, offset);
		result[0] = result3[0];
		result[1] = result3[1];
		result[2] = result3[1];
		result[3] = result3[2];
		return;
	}
/////////////////////////////////////////////////////////////////////////////////////:

	//TODO(martin): review edge cases (coincident points ? colinear points ? control points pointing outward end point?)
	//NOTE(martin): first offset control point is just the offset of first control point
	vec2 n = {p[0].y - p[1].y,
	          p[1].x - p[0].x};
	f32 norm = sqrt(n.x*n.x + n.y*n.y);
	n.x *= offset/norm;
	n.y *= offset/norm;

	result[0].x = p[0].x + n.x;
	result[0].y = p[0].y + n.y;

	//NOTE(martin): subsequent offset control points are the intersection of offset control lines
	for(int i=1; i<count-1; i++)
	{
		vec2 p0 = p[i-1];
		vec2 p1 = p[i];
		vec2 p2 = p[i+1];

		//NOTE(martin): get normals
		vec2 n0 = {p0.y - p1.y,
			   p1.x - p0.x};
		f32 norm0 = sqrt(n0.x*n0.x + n0.y*n0.y);
		n0.x /= norm0;
		n0.y /= norm0;

		vec2 n1 = {p1.y - p2.y,
			   p2.x - p1.x};
		f32 norm1 = sqrt(n1.x*n1.x + n1.y*n1.y);
		n1.x /= norm1;
		n1.y /= norm1;

		/*NOTE(martin): let vector u = (n0+n1) and vector v = pIntersect - p1
		                then v = u * (2*offset / norm(u)^2)
			        (this can be derived from writing the pythagoras theorems in the triangles of the joint)
		*/
		vec2 u = {n0.x + n1.x, n0.y + n1.y};
		f32 uNormSquare = u.x*u.x + u.y*u.y;
		f32 alpha = 2*offset / uNormSquare;
		vec2 v = {u.x * alpha, u.y * alpha};

		result[i].x = p1.x + v.x;
		result[i].y = p1.y + v.y;

		ASSERT(!isnan(result[i].x));
		ASSERT(!isnan(result[i].y));
	}

	//NOTE(martin): last offset control point is just the offset of last control point
	n = (vec2){p[count-2].y - p[count-1].y,
	           p[count-1].x - p[count-2].x};
	norm = sqrt(n.x*n.x + n.y*n.y);
	n.x *= offset/norm;
	n.y *= offset/norm;

	result[count-1].x = p[count-1].x + n.x;
	result[count-1].y = p[count-1].y + n.y;
}

vec2 mg_quadratic_get_point(vec2 p[3], f32 t)
{
	vec2 r;

	f32 oneMt = 1-t;
	f32 oneMt2 = Square(oneMt);
	f32 t2 = Square(t);

	r.x = oneMt2*p[0].x + 2*oneMt*t*p[1].x + t2*p[2].x;
	r.y = oneMt2*p[0].y + 2*oneMt*t*p[1].y + t2*p[2].y;

	return(r);
}

void mg_quadratic_split(vec2 p[3], f32 t, vec2 outLeft[3], vec2 outRight[3])
{
	//NOTE(martin): split bezier curve p at parameter t, using De Casteljau's algorithm
	//              the q_n are the points along the hull's segments at parameter t
	//              s is the split point.

	f32 oneMt = 1-t;

	vec2 q0 = {oneMt*p[0].x + t*p[1].x,
		   oneMt*p[0].y + t*p[1].y};

	vec2 q1 = {oneMt*p[1].x + t*p[2].x,
		   oneMt*p[1].y + t*p[2].y};

	vec2 s = {oneMt*q0.x + t*q1.x,
		   oneMt*q0.y + t*q1.y};

	outLeft[0] = p[0];
	outLeft[1] = q0;
	outLeft[2] = s;

	outRight[0] = s;
	outRight[1] = q1;
	outRight[2] = p[2];
}


void mg_render_stroke_quadratic(mg_canvas_data* context, vec2 p[4], u32 zIndex, mg_attributes* attributes)
{
	#define CHECK_SAMPLE_COUNT 5
	f32 checkSamples[CHECK_SAMPLE_COUNT] = {1./6, 2./6, 3./6, 4./6, 5./6};

	vec2 positiveOffsetHull[3];
	vec2 negativeOffsetHull[3];

	mg_offset_hull(3, p, positiveOffsetHull, 0.5 * attributes->width);
	mg_offset_hull(3, p, negativeOffsetHull, -0.5 * attributes->width);

	//NOTE(martin): the distance d between the offset curve and the path must be between w/2-tolerance and w/2+tolerance
	//              thus, by constraining tolerance to be at most, 0.5*width, we can rewrite this condition like this:
	//
	//              (w/2-tolerance)^2 < d^2 < (w/2+tolerance)^2
	//
	//		we compute the maximum overshoot outside these bounds and split the curve at the corresponding parameter

	f32 tolerance = minimum(attributes->tolerance, 0.5 * attributes->width);
	f32 d2LowBound = Square(0.5 * attributes->width - attributes->tolerance);
	f32 d2HighBound = Square(0.5 * attributes->width + attributes->tolerance);

	f32 maxOvershoot = 0;
	f32 maxOvershootParameter = 0;

	for(int i=0; i<CHECK_SAMPLE_COUNT; i++)
	{
		f32 t = checkSamples[i];

		vec2 c = mg_quadratic_get_point(p, t);
		vec2 cp =  mg_quadratic_get_point(positiveOffsetHull, t);
		vec2 cn =  mg_quadratic_get_point(negativeOffsetHull, t);

		f32 positiveDistSquare = Square(c.x - cp.x) + Square(c.y - cp.y);
		f32 negativeDistSquare = Square(c.x - cn.x) + Square(c.y - cn.y);

		f32 positiveOvershoot = maximum(positiveDistSquare - d2HighBound, d2LowBound - positiveDistSquare);
		f32 negativeOvershoot = maximum(negativeDistSquare - d2HighBound, d2LowBound - negativeDistSquare);

		f32 overshoot = maximum(positiveOvershoot, negativeOvershoot);

		if(overshoot > maxOvershoot)
		{
			maxOvershoot = overshoot;
			maxOvershootParameter = t;
		}
	}

	if(maxOvershoot > 0)
	{
		//TODO(martin): split at maxErrorParameter and recurse
		vec2 splitLeft[3];
		vec2 splitRight[3];
		mg_quadratic_split(p, maxOvershootParameter, splitLeft, splitRight);
		mg_render_stroke_quadratic(context, splitLeft, zIndex, attributes);
		mg_render_stroke_quadratic(context, splitRight, zIndex, attributes);
	}
	else
	{
		//NOTE(martin): push the actual fill commands for the offset contour

		u32 zIndex = mg_get_next_z_index(context);

		mg_render_fill_quadratic(context, positiveOffsetHull, zIndex, attributes->color);
		mg_render_fill_quadratic(context, negativeOffsetHull, zIndex, attributes->color);

		//NOTE(martin):	add base triangles
		u32 baseIndex = mg_vertices_base_index(context);
		i32* indices = mg_reserve_indices(context, 6);

		mg_push_vertex(context,
		               mg_mat2x3_mul(context->transform, positiveOffsetHull[0]),
		               (vec4){1, 1, 1, 1},
		               attributes->color,
		               zIndex);

		mg_push_vertex(context,
		               mg_mat2x3_mul(context->transform, positiveOffsetHull[2]),
		               (vec4){1, 1, 1, 1},
		               attributes->color,
		               zIndex);

		mg_push_vertex(context,
		               mg_mat2x3_mul(context->transform, negativeOffsetHull[2]),
		               (vec4){1, 1, 1, 1},
		               attributes->color,
		               zIndex);

		mg_push_vertex(context,
		               mg_mat2x3_mul(context->transform, negativeOffsetHull[0]),
		               (vec4){1, 1, 1, 1},
		               attributes->color,
		               zIndex);

		indices[0] = baseIndex + 0;
		indices[1] = baseIndex + 1;
		indices[2] = baseIndex + 2;
		indices[3] = baseIndex + 0;
		indices[4] = baseIndex + 2;
		indices[5] = baseIndex + 3;
	}
	#undef CHECK_SAMPLE_COUNT
}

vec2 mg_cubic_get_point(vec2 p[4], f32 t)
{
	vec2 r;

	f32 oneMt = 1-t;
	f32 oneMt2 = Square(oneMt);
	f32 oneMt3 = oneMt2*oneMt;
	f32 t2 = Square(t);
	f32 t3 = t2*t;

	r.x = oneMt3*p[0].x + 3*oneMt2*t*p[1].x + 3*oneMt*t2*p[2].x + t3*p[3].x;
	r.y = oneMt3*p[0].y + 3*oneMt2*t*p[1].y + 3*oneMt*t2*p[2].y + t3*p[3].y;

	return(r);
}

void mg_cubic_split(vec2 p[4], f32 t, vec2 outLeft[4], vec2 outRight[4])
{
	//NOTE(martin): split bezier curve p at parameter t, using De Casteljau's algorithm
	//              the q_n are the points along the hull's segments at parameter t
	//              the r_n are the points along the (q_n, q_n+1) segments at parameter t
	//              s is the split point.

	f32 oneMt = 1-t;

	vec2 q0 = {oneMt*p[0].x + t*p[1].x,
	           oneMt*p[0].y + t*p[1].y};

	vec2 q1 = {oneMt*p[1].x + t*p[2].x,
	           oneMt*p[1].y + t*p[2].y};

	vec2 q2 = {oneMt*p[2].x + t*p[3].x,
	           oneMt*p[2].y + t*p[3].y};

	vec2 r0 = {oneMt*q0.x + t*q1.x,
	           oneMt*q0.y + t*q1.y};

	vec2 r1 = {oneMt*q1.x + t*q2.x,
	           oneMt*q1.y + t*q2.y};

	vec2 s = {oneMt*r0.x + t*r1.x,
	          oneMt*r0.y + t*r1.y};;

	outLeft[0] = p[0];
	outLeft[1] = q0;
	outLeft[2] = r0;
	outLeft[3] = s;

	outRight[0] = s;
	outRight[1] = r1;
	outRight[2] = q2;
	outRight[3] = p[3];
}

void mg_render_stroke_cubic(mg_canvas_data* context, vec2 p[4], u32 zIndex, mg_attributes* attributes)
{
	#define CHECK_SAMPLE_COUNT 5
	f32 checkSamples[CHECK_SAMPLE_COUNT] = {1./6, 2./6, 3./6, 4./6, 5./6};

	vec2 positiveOffsetHull[4];
	vec2 negativeOffsetHull[4];

	mg_offset_hull(4, p, positiveOffsetHull, 0.5 * attributes->width);
	mg_offset_hull(4, p, negativeOffsetHull, -0.5 * attributes->width);

	//NOTE(martin): the distance d between the offset curve and the path must be between w/2-tolerance and w/2+tolerance
	//              thus, by constraining tolerance to be at most, 0.5*width, we can rewrite this condition like this:
	//
	//              (w/2-tolerance)^2 < d^2 < (w/2+tolerance)^2
	//
	//		we compute the maximum overshoot outside these bounds and split the curve at the corresponding parameter

	f32 tolerance = minimum(attributes->tolerance, 0.5 * attributes->width);
	f32 d2LowBound = Square(0.5 * attributes->width - attributes->tolerance);
	f32 d2HighBound = Square(0.5 * attributes->width + attributes->tolerance);

	f32 maxOvershoot = 0;
	f32 maxOvershootParameter = 0;

	for(int i=0; i<CHECK_SAMPLE_COUNT; i++)
	{
		f32 t = checkSamples[i];

		vec2 c = mg_cubic_get_point(p, t);
		vec2 cp =  mg_cubic_get_point(positiveOffsetHull, t);
		vec2 cn =  mg_cubic_get_point(negativeOffsetHull, t);

		f32 positiveDistSquare = Square(c.x - cp.x) + Square(c.y - cp.y);
		f32 negativeDistSquare = Square(c.x - cn.x) + Square(c.y - cn.y);

		f32 positiveOvershoot = maximum(positiveDistSquare - d2HighBound, d2LowBound - positiveDistSquare);
		f32 negativeOvershoot = maximum(negativeDistSquare - d2HighBound, d2LowBound - negativeDistSquare);

		f32 overshoot = maximum(positiveOvershoot, negativeOvershoot);

		if(overshoot > maxOvershoot)
		{
			maxOvershoot = overshoot;
			maxOvershootParameter = t;
		}
	}

	if(maxOvershoot > 0)
	{
		//TODO(martin): split at maxErrorParameter and recurse
		vec2 splitLeft[4];
		vec2 splitRight[4];
		mg_cubic_split(p, maxOvershootParameter, splitLeft, splitRight);
		mg_render_stroke_cubic(context, splitLeft, zIndex, attributes);
		mg_render_stroke_cubic(context, splitRight, zIndex, attributes);
	}
	else
	{
		//NOTE(martin): push the actual fill commands for the offset contour

		u32 zIndex = mg_get_next_z_index(context);

		mg_render_fill_cubic(context, positiveOffsetHull, zIndex, attributes->color);
		mg_render_fill_cubic(context, negativeOffsetHull, zIndex, attributes->color);

		//NOTE(martin):	add base triangles
		u32 baseIndex = mg_vertices_base_index(context);
		i32* indices = mg_reserve_indices(context, 6);


		mg_push_vertex(context,
		                mg_mat2x3_mul(context->transform, positiveOffsetHull[0]),
		                (vec4){1, 1, 1, 1},
		                attributes->color,
		                zIndex);

		mg_push_vertex(context,
		               mg_mat2x3_mul(context->transform, positiveOffsetHull[3]),
		               (vec4){1, 1, 1, 1},
		               attributes->color,
		               zIndex);

		mg_push_vertex(context,
		               mg_mat2x3_mul(context->transform, negativeOffsetHull[3]),
		               (vec4){1, 1, 1, 1},
		               attributes->color,
		               zIndex);

		mg_push_vertex(context,
		               mg_mat2x3_mul(context->transform, negativeOffsetHull[0]),
		               (vec4){1, 1, 1, 1},
		               attributes->color,
		               zIndex);

		indices[0] = baseIndex + 0;
		indices[1] = baseIndex + 1;
		indices[2] = baseIndex + 2;
		indices[3] = baseIndex + 0;
		indices[4] = baseIndex + 2;
		indices[5] = baseIndex + 3;
	}
	#undef CHECK_SAMPLE_COUNT
}

void mg_stroke_cap(mg_canvas_data* context, vec2 p0, vec2 direction, mg_attributes* attributes)
{
	//NOTE(martin): compute the tangent and normal vectors (multiplied by half width) at the cap point

	f32 dn = sqrt(Square(direction.x) + Square(direction.y));
	f32 alpha = 0.5 * attributes->width/dn;

	vec2 n0 = {-alpha*direction.y,
		    alpha*direction.x};

	vec2 m0 = {alpha*direction.x,
	           alpha*direction.y};

	u32 zIndex = mg_get_next_z_index(context);

	u32 baseIndex = mg_vertices_base_index(context);
	i32* indices = mg_reserve_indices(context, 6);

	mg_push_vertex(context,
	               mg_mat2x3_mul(context->transform, (vec2){p0.x + n0.x, p0.y + n0.y}),
	               (vec4){1, 1, 1, 1},
	               attributes->color,
	               zIndex);

	mg_push_vertex(context,
	               mg_mat2x3_mul(context->transform, (vec2){p0.x + n0.x + m0.x, p0.y + n0.y + m0.y}),
	               (vec4){1, 1, 1, 1},
	               attributes->color,
	               zIndex);

	mg_push_vertex(context,
	               mg_mat2x3_mul(context->transform, (vec2){p0.x - n0.x + m0.x, p0.y - n0.y + m0.y}),
	               (vec4){1, 1, 1, 1},
	               attributes->color,
	               zIndex);

	mg_push_vertex(context,
	               mg_mat2x3_mul(context->transform, (vec2){p0.x - n0.x, p0.y - n0.y}),
	               (vec4){1, 1, 1, 1},
	               attributes->color,
	               zIndex);

	indices[0] = baseIndex;
	indices[1] = baseIndex + 1;
	indices[2] = baseIndex + 2;
	indices[3] = baseIndex;
	indices[4] = baseIndex + 2;
	indices[5] = baseIndex + 3;
}

void mg_stroke_joint(mg_canvas_data* context,
                              vec2 p0,
			      vec2 t0,
			      vec2 t1,
			      mg_attributes* attributes)
{
	//NOTE(martin): compute the normals at the joint point
	f32 norm_t0 = sqrt(Square(t0.x) + Square(t0.y));
	f32 norm_t1 = sqrt(Square(t1.x) + Square(t1.y));

	vec2 n0 = {-t0.y, t0.x};
	n0.x /= norm_t0;
	n0.y /= norm_t0;

	vec2 n1 = {-t1.y, t1.x};
	n1.x /= norm_t1;
	n1.y /= norm_t1;

	//NOTE(martin): the sign of the cross product determines if the normals are facing outwards or inwards the angle.
	//              we flip them to face outwards if needed
	f32 crossZ = n0.x*n1.y - n0.y*n1.x;
	if(crossZ > 0)
	{
		n0.x *= -1;
		n0.y *= -1;
		n1.x *= -1;
		n1.y *= -1;
	}

	u32 zIndex = mg_get_next_z_index(context);

	//NOTE(martin): use the same code as hull offset to find mitter point...
	/*NOTE(martin): let vector u = (n0+n1) and vector v = pIntersect - p1
		then v = u * (2*offset / norm(u)^2)
		(this can be derived from writing the pythagoras theorems in the triangles of the joint)
	*/
	f32 halfW = 0.5 * attributes->width;
	vec2 u = {n0.x + n1.x, n0.y + n1.y};
	f32 uNormSquare = u.x*u.x + u.y*u.y;
	f32 alpha = attributes->width / uNormSquare;
	vec2 v = {u.x * alpha, u.y * alpha};

	f32 excursionSquare = uNormSquare * Square(alpha - attributes->width/4);

	if(  attributes->joint == MG_JOINT_MITER
	  && excursionSquare <= Square(attributes->maxJointExcursion))
	{
		vec2 mitterPoint = {p0.x + v.x, p0.y + v.y};

		u32 baseIndex = mg_vertices_base_index(context);
		i32* indices = mg_reserve_indices(context, 6);

		mg_push_vertex(context,
		               mg_mat2x3_mul(context->transform, p0),
		               (vec4){1, 1, 1, 1},
		               attributes->color,
		               zIndex);

		mg_push_vertex(context,
		               mg_mat2x3_mul(context->transform, (vec2){p0.x + n0.x*halfW, p0.y + n0.y*halfW}),
		               (vec4){1, 1, 1, 1},
		               attributes->color,
		               zIndex);

		mg_push_vertex(context,
		               mg_mat2x3_mul(context->transform, mitterPoint),
		               (vec4){1, 1, 1, 1},
		               attributes->color,
		               zIndex);

		mg_push_vertex(context,
	                   mg_mat2x3_mul(context->transform, (vec2){p0.x + n1.x*halfW, p0.y + n1.y*halfW}),
	                   (vec4){1, 1, 1, 1},
	                   attributes->color,
	                   zIndex);

		indices[0] = baseIndex;
		indices[1] = baseIndex + 1;
		indices[2] = baseIndex + 2;
		indices[3] = baseIndex;
		indices[4] = baseIndex + 2;
		indices[5] = baseIndex + 3;
	}
	else
	{
		//NOTE(martin): add a bevel joint
		u32 baseIndex = mg_vertices_base_index(context);
		i32* indices = mg_reserve_indices(context, 3);

		mg_push_vertex(context,
		                        mg_mat2x3_mul(context->transform, p0),
		                        (vec4){1, 1, 1, 1},
		                        attributes->color,
		                        zIndex);

		mg_push_vertex(context,
		                        mg_mat2x3_mul(context->transform, (vec2){p0.x + n0.x*halfW, p0.y + n0.y*halfW}),
		                        (vec4){1, 1, 1, 1},
		                        attributes->color,
		                        zIndex);

		mg_push_vertex(context,
		                        mg_mat2x3_mul(context->transform, (vec2){p0.x + n1.x*halfW, p0.y + n1.y*halfW}),
		                        (vec4){1, 1, 1, 1},
		                        attributes->color,
		                        zIndex);

		DEBUG_ASSERT(!isnan(n0.x) && !isnan(n0.y) && !isnan(n1.x) && !isnan(n1.y));

		indices[0] = baseIndex;
		indices[1] = baseIndex + 1;
		indices[2] = baseIndex + 2;
	}
}

void mg_render_stroke_element(mg_canvas_data* context,
                                      mg_path_elt* element,
				      mg_attributes* attributes,
				      vec2 currentPoint,
				      vec2* startTangent,
				      vec2* endTangent,
				      vec2* endPoint)
{
	vec2 controlPoints[4] = {currentPoint, element->p[0], element->p[1], element->p[2]};
	int endPointIndex = 0;
	u32 zIndex = mg_get_next_z_index(context);

	switch(element->type)
	{
		case MG_PATH_LINE:
			mg_render_stroke_line(context, controlPoints, zIndex, attributes);
			endPointIndex = 1;
			break;

		case MG_PATH_QUADRATIC:
			mg_render_stroke_quadratic(context, controlPoints, zIndex, attributes);
			endPointIndex = 2;
			break;

		case MG_PATH_CUBIC:
			mg_render_stroke_cubic(context, controlPoints, zIndex, attributes);
			endPointIndex = 3;
			break;

		case MG_PATH_MOVE:
			ASSERT(0, "should be unreachable");
			break;
	}

	*startTangent = (vec2){.x = controlPoints[1].x - controlPoints[0].x,
			      .y = controlPoints[1].y - controlPoints[0].y};

	*endTangent = (vec2){controlPoints[endPointIndex].x - controlPoints[endPointIndex-1].x,
			     controlPoints[endPointIndex].y - controlPoints[endPointIndex-1].y};

	*endPoint = controlPoints[endPointIndex];

}

u32 mg_render_stroke_subpath(mg_canvas_data* context,
                                     mg_path_elt* elements,
				      mg_path_descriptor* path,
				      mg_attributes* attributes,
				      u32 startIndex,
				      vec2 startPoint)
{
	u32 eltCount = path->count;
	DEBUG_ASSERT(startIndex < eltCount);

	vec2 currentPoint = startPoint;
	vec2 endPoint = {0, 0};
	vec2 previousEndTangent = {0, 0};
	vec2 firstTangent = {0, 0};
	vec2 startTangent = {0, 0};
	vec2 endTangent = {0, 0};

	//NOTE(martin): render first element and compute first tangent
	mg_render_stroke_element(context, elements + startIndex, attributes, currentPoint, &startTangent, &endTangent, &endPoint);

	firstTangent = startTangent;
	previousEndTangent = endTangent;
	currentPoint = endPoint;

	//NOTE(martin): render subsequent elements along with their joints
	u32 eltIndex = startIndex + 1;
	for(;
	    eltIndex<eltCount && elements[eltIndex].type != MG_PATH_MOVE;
	    eltIndex++)
	{
		mg_render_stroke_element(context, elements + eltIndex, attributes, currentPoint, &startTangent, &endTangent, &endPoint);

		if(attributes->joint != MG_JOINT_NONE)
		{
			mg_stroke_joint(context, currentPoint, previousEndTangent, startTangent, attributes);
		}
		previousEndTangent = endTangent;
		currentPoint = endPoint;
	}
	u32 subPathEltCount = eltIndex - (startIndex+1);

	//NOTE(martin): draw end cap / joint. We ensure there's at least two segments to draw a closing joint
	if(  subPathEltCount > 1
	  && startPoint.x == endPoint.x
	  && startPoint.y == endPoint.y)
	{
		if(attributes->joint != MG_JOINT_NONE)
		{
			//NOTE(martin): add a closing joint if the path is closed
			mg_stroke_joint(context, endPoint, endTangent, firstTangent, attributes);
		}
	}
	else if(attributes->cap == MG_CAP_SQUARE)
	{
		//NOTE(martin): add start and end cap
		mg_stroke_cap(context, startPoint, (vec2){-startTangent.x, -startTangent.y}, attributes);
		mg_stroke_cap(context, endPoint, startTangent, attributes);
	}

	return(eltIndex);
}


void mg_render_stroke(mg_canvas_data* context,
                              mg_path_elt* elements,
			      mg_path_descriptor* path,
			      mg_attributes* attributes)
{
	u32 eltCount = path->count;
	DEBUG_ASSERT(eltCount);

	vec2 startPoint = path->startPoint;
	u32 startIndex = 0;

	while(startIndex < eltCount)
	{
		//NOTE(martin): eliminate leading moves
		while(startIndex < eltCount && elements[startIndex].type == MG_PATH_MOVE)
		{
			startPoint = elements[startIndex].p[0];
			startIndex++;
		}

		if(startIndex < eltCount)
		{
			startIndex = mg_render_stroke_subpath(context, elements, path, attributes, startIndex, startPoint);
		}
	}
}

//-----------------------------------------------------------------------------------------------------------
// Fast shapes primitives
//-----------------------------------------------------------------------------------------------------------

void mg_render_rectangle_fill(mg_canvas_data* context, mp_rect rect, mg_attributes* attributes)
{
	u32 baseIndex = mg_vertices_base_index(context);
	i32* indices = mg_reserve_indices(context, 6);

	u32 zIndex = mg_get_next_z_index(context);

	vec2 points[4] = {{rect.x, rect.y},
	                  {rect.x + rect.w, rect.y},
	                  {rect.x + rect.w, rect.y + rect.h},
	                  {rect.x, rect.y + rect.h}};

	vec4 cubic = {1, 1, 1, 1};
	for(int i=0; i<4; i++)
	{
		vec2 pos = mg_mat2x3_mul(context->transform, points[i]);
		mg_push_vertex(context, pos, cubic, attributes->color, zIndex);
	}
	indices[0] = baseIndex + 0;
	indices[1] = baseIndex + 1;
	indices[2] = baseIndex + 2;
	indices[3] = baseIndex + 0;
	indices[4] = baseIndex + 2;
	indices[5] = baseIndex + 3;
}

void mg_render_rectangle_stroke(mg_canvas_data* context, mp_rect rect, mg_attributes* attributes)
{
	//NOTE(martin): stroke a rectangle by fill two scaled rectangles with the same zIndex.
	u32 baseIndex = mg_vertices_base_index(context);
	i32* indices = mg_reserve_indices(context, 12);

	u32 zIndex = mg_get_next_z_index(context);

	//NOTE(martin): limit stroke width to the minimum dimension of the rectangle
	f32 width = minimum(attributes->width, minimum(rect.w, rect.h));
	f32 halfW = width/2;

	vec2 outerPoints[4] = {{rect.x - halfW, rect.y - halfW},
	                       {rect.x + rect.w + halfW, rect.y - halfW},
	                       {rect.x + rect.w + halfW, rect.y + rect.h + halfW},
	                       {rect.x - halfW, rect.y + rect.h + halfW}};

	vec2 innerPoints[4] = {{rect.x + halfW, rect.y + halfW},
	                       {rect.x + rect.w - halfW, rect.y + halfW},
	                       {rect.x + rect.w - halfW, rect.y + rect.h - halfW},
	                       {rect.x + halfW, rect.y + rect.h - halfW}};

	vec4 cubic = {1, 1, 1, 1};
	for(int i=0; i<4; i++)
	{
		vec2 pos = mg_mat2x3_mul(context->transform, outerPoints[i]);
		mg_push_vertex(context, pos, cubic, attributes->color, zIndex);
	}

	for(int i=0; i<4; i++)
	{
		vec2 pos = mg_mat2x3_mul(context->transform, innerPoints[i]);
		mg_push_vertex(context, pos, cubic, attributes->color, zIndex);
	}

	indices[0] = baseIndex + 0;
	indices[1] = baseIndex + 1;
	indices[2] = baseIndex + 2;
	indices[3] = baseIndex + 0;
	indices[4] = baseIndex + 2;
	indices[5] = baseIndex + 3;
	indices[6] = baseIndex + 4;
	indices[7] = baseIndex + 5;
	indices[8] = baseIndex + 6;
	indices[9] = baseIndex + 4;
	indices[10] = baseIndex + 6;
	indices[11] = baseIndex + 7;
}

void mg_render_fill_arc_corner(mg_canvas_data* context, f32 x, f32 y, f32 rx, f32 ry, u32 zIndex, mg_color color)
{
	//NOTE(martin): draw a precomputed arc corner, using a bezier approximation
	u32 baseIndex = mg_vertices_base_index(context);
	i32* indices = mg_reserve_indices(context, 6);

	static const vec4 cubics[4] = {{-3.76797, -9.76362, 5.47912, -1},
	                            {-4.19896, -9.45223, 7.534, -1},
	                            {-4.19896, -7.534, 9.45223, -1},
	                            {-3.76797, -5.47912, 9.76362, -1}};

	f32 cx = rx*4*(sqrt(2)-1)/3;
	f32 cy = ry*4*(sqrt(2)-1)/3;

	vec2 points[4] = {{x, y + ry},
	                  {x, y + ry - cy},
	                  {x + rx - cx, y},
	                  {x + rx, y}};

	for(int i=0; i<4; i++)
	{
		vec2 pos = mg_mat2x3_mul(context->transform, points[i]);
		mg_push_vertex(context, pos, cubics[i], color, zIndex);
	}
	indices[0] = baseIndex + 0;
	indices[1] = baseIndex + 1;
	indices[2] = baseIndex + 2;
	indices[3] = baseIndex + 0;
	indices[4] = baseIndex + 2;
	indices[5] = baseIndex + 3;
}

void mg_render_rounded_rectangle_fill_with_z_index(mg_canvas_data* context,
							                       mg_rounded_rect rect,
							                       mg_attributes* attributes,
							                       u32 zIndex)
{
	//NOTE(martin): draw a rounded rectangle by drawing a normal rectangle and 4 corners,
	//              approximating an arc by a precomputed bezier curve

	u32 baseIndex = mg_vertices_base_index(context);
	i32* indices = mg_reserve_indices(context, 18);

	//NOTE(martin): inner cutted corner rectangle
	vec2 points[8] = {{rect.x + rect.r, rect.y},
	                  {rect.x + rect.w - rect.r, rect.y},
	                  {rect.x + rect.w, rect.y + rect.r},
	                  {rect.x + rect.w, rect.y + rect.h - rect.r},
	                  {rect.x + rect.w - rect.r, rect.y + rect.h},
	                  {rect.x + rect.r, rect.y + rect.h},
	                  {rect.x, rect.y + rect.h - rect.r},
	                  {rect.x, rect.y + rect.r}};

	vec4 cubic = {1, 1, 1, 1};

	for(int i=0; i<8; i++)
	{
		vec2 pos = mg_mat2x3_mul(context->transform, points[i]);
		mg_push_vertex(context, pos, cubic, attributes->color, zIndex);
	}

	static const i32 fanIndices[18] = { 0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 6, 0, 6, 7 }; // inner fan
	for(int i=0; i<18; i++)
	{
		indices[i] = fanIndices[i] + baseIndex;
	}

	mg_render_fill_arc_corner(context, rect.x, rect.y, rect.r, rect.r, zIndex, attributes->color);
	mg_render_fill_arc_corner(context, rect.x + rect.w, rect.y, -rect.r, rect.r, zIndex, attributes->color);
	mg_render_fill_arc_corner(context, rect.x + rect.w, rect.y + rect.h, -rect.r, -rect.r, zIndex, attributes->color);
	mg_render_fill_arc_corner(context, rect.x, rect.y + rect.h, rect.r, -rect.r, zIndex, attributes->color);
}


void mg_render_rounded_rectangle_fill(mg_canvas_data* context,
                                              mg_rounded_rect rect,
					      mg_attributes* attributes)
{
	u32 zIndex = mg_get_next_z_index(context);
	mg_render_rounded_rectangle_fill_with_z_index(context, rect, attributes, zIndex);
}

void mg_render_rounded_rectangle_stroke(mg_canvas_data* context,
						mg_rounded_rect rect,
						mg_attributes* attributes)
{
	//NOTE(martin): stroke rounded rectangle by filling two scaled rounded rectangles with the same zIndex
	f32 width = minimum(attributes->width, minimum(rect.w, rect.h));
	f32 halfW = width/2;

	mg_rounded_rect inner = {rect.x + halfW, rect.y + halfW, rect.w - width, rect.h - width, rect.r - halfW};
	mg_rounded_rect outer = {rect.x - halfW, rect.y - halfW, rect.w + width, rect.h + width, rect.r + halfW};

	u32 zIndex = mg_get_next_z_index(context);
	mg_render_rounded_rectangle_fill_with_z_index(context, outer, attributes, zIndex);
	mg_render_rounded_rectangle_fill_with_z_index(context, inner, attributes, zIndex);
}

void mg_render_ellipse_fill_with_z_index(mg_canvas_data* context,
                                                 mp_rect rect,
						 mg_attributes* attributes,
						 u32 zIndex)
{
	//NOTE(martin): draw a filled ellipse by drawing a diamond and 4 corners,
	//              approximating an arc by a precomputed bezier curve

	u32 baseIndex = mg_vertices_base_index(context);
	i32* indices = mg_reserve_indices(context, 6);

	f32 rx = rect.w/2;
	f32 ry = rect.h/2;

	//NOTE(martin): inner diamond
	vec2 points[4] = {{rect.x, rect.y + ry},
	                  {rect.x + rx, rect.y},
	                  {rect.x + rect.w, rect.y + ry},
	                  {rect.x + rx, rect.y + rect.h}};

	vec4 cubic = {1, 1, 1, 1};

	for(int i=0; i<4; i++)
	{
		vec2 pos = mg_mat2x3_mul(context->transform, points[i]);
		mg_push_vertex(context, pos, cubic, attributes->color, zIndex);
	}

	indices[0] = baseIndex + 0;
	indices[1] = baseIndex + 1;
	indices[2] = baseIndex + 2;
	indices[3] = baseIndex + 0;
	indices[4] = baseIndex + 2;
	indices[5] = baseIndex + 3;

	mg_render_fill_arc_corner(context, rect.x, rect.y, rx, ry, zIndex, attributes->color);
	mg_render_fill_arc_corner(context, rect.x + rect.w, rect.y, -rx, ry, zIndex, attributes->color);
	mg_render_fill_arc_corner(context, rect.x + rect.w, rect.y + rect.h, -rx, -ry, zIndex, attributes->color);
	mg_render_fill_arc_corner(context, rect.x, rect.y + rect.h, rx, -ry, zIndex, attributes->color);
}

void mg_render_ellipse_fill(mg_canvas_data* context, mp_rect rect, mg_attributes* attributes)
{
	u32 zIndex = mg_get_next_z_index(context);
	mg_render_ellipse_fill_with_z_index(context, rect, attributes, zIndex);
}

void mg_render_ellipse_stroke(mg_canvas_data* context, mp_rect rect, mg_attributes* attributes)
{
	//NOTE(martin): stroke by filling two scaled ellipsis with the same zIndex
	f32 width = minimum(attributes->width, minimum(rect.w, rect.h));
	f32 halfW = width/2;

	mp_rect inner = {rect.x + halfW, rect.y + halfW, rect.w - width, rect.h - width};
	mp_rect outer = {rect.x - halfW, rect.y - halfW, rect.w + width, rect.h + width};

	u32 zIndex = mg_get_next_z_index(context);
	mg_render_ellipse_fill_with_z_index(context, outer, attributes, zIndex);
	mg_render_ellipse_fill_with_z_index(context, inner, attributes, zIndex);
}

void mg_render_image(mg_canvas_data* context, mg_image image, mp_rect rect)
{
	mg_image_data* imageData = mg_image_ptr_from_handle(context, image);
	if(!imageData)
	{
		return;
	}

	u32 baseIndex = mg_vertices_base_index(context);
	i32* indices = mg_reserve_indices(context, 6);

	u32 zIndex = mg_get_next_z_index(context);

	vec2 points[4] = {{rect.x, rect.y},
	                  {rect.x + rect.w, rect.y},
	                  {rect.x + rect.w, rect.y + rect.h},
	                  {rect.x, rect.y + rect.h}};

	vec2 uv[4] = {{imageData->rect.x + 0.5, imageData->rect.y + 0.5},
	              {imageData->rect.x + imageData->rect.w - 0.5, imageData->rect.y + 0.5},
	              {imageData->rect.x + imageData->rect.w - 0.5, imageData->rect.y + imageData->rect.h - 0.5},
	              {imageData->rect.x + 0.5, imageData->rect.y + imageData->rect.h - 0.5}};

	vec4 cubic = {1, 1, 1, 1};
	mg_color color = {1, 1, 1, 1};

	for(int i=0; i<4; i++)
	{
		vec2 transformedUV = {uv[i].x / MG_ATLAS_SIZE, uv[i].y / MG_ATLAS_SIZE};

		vec2 pos = mg_mat2x3_mul(context->transform, points[i]);
		mg_push_textured_vertex(context, pos, cubic, transformedUV, color, zIndex);
	}
	indices[0] = baseIndex + 0;
	indices[1] = baseIndex + 1;
	indices[2] = baseIndex + 2;
	indices[3] = baseIndex + 0;
	indices[4] = baseIndex + 2;
	indices[5] = baseIndex + 3;
}

void mg_render_rounded_image(mg_canvas_data* context, mg_image image, mg_rounded_rect rect, mg_attributes* attributes)
{
	mg_image_data* imageData = mg_image_ptr_from_handle(context, image);
	if(!imageData)
	{
		return;
	}

	////////////////////////////////////////////////////////////////////////////////
	//TODO: this does not work for rotated rectangles
	////////////////////////////////////////////////////////////////////////////////
	vec2 uvMin = {(imageData->rect.x + 0.5) / MG_ATLAS_SIZE, (imageData->rect.y + 0.5) / MG_ATLAS_SIZE};
	vec2 uvMax = {(imageData->rect.x + imageData->rect.w - 0.5) / MG_ATLAS_SIZE, (imageData->rect.y + imageData->rect.h - 0.5) / MG_ATLAS_SIZE};
	mp_rect uvRect = {uvMin.x, uvMin.y, uvMax.x - uvMin.x, uvMax.y - uvMin.y};

	vec2 pMin = mg_mat2x3_mul(context->transform, (vec2){rect.x, rect.y});
	vec2 pMax = mg_mat2x3_mul(context->transform, (vec2){rect.x + rect.w, rect.y + rect.h});
	mp_rect pRect = {pMin.x, pMin.y, pMax.x - pMin.x, pMax.y - pMin.y};

	u32 startIndex = mg_vertices_base_index(context);

	mgc_vertex_layout* layout = &context->painter->vertexLayout;

	attributes->color = (mg_color){1, 1, 1, 1};
	mg_render_rounded_rectangle_fill(context, rect, attributes);

	u32 indexCount = mg_vertices_base_index(context) - startIndex;

	for(int i=0; i<indexCount; i++)
	{
		u32 index = startIndex + i;
		vec2* pos = (vec2*)(((char*)layout->posBuffer) + index*layout->posStride);
		vec2* uv = (vec2*)(((char*)layout->uvBuffer) + index*layout->uvStride);

		vec2 coordInBoundingSpace = {(pos->x - pRect.x)/pRect.w,
		                             (pos->y - pRect.y)/pRect.h};

		vec2 mappedUV = {uvRect.x + coordInBoundingSpace.x * uvRect.w,
		                 uvRect.y + coordInBoundingSpace.y * uvRect.h};

		*uv = mappedUV;
	}

}

//-----------------------------------------------------------------------------------------------------------
// Graphics canvas
//-----------------------------------------------------------------------------------------------------------

#ifdef MG_IMPLEMENTS_BACKEND_METAL
	typedef struct mg_metal_surface mg_metal_surface;
	mg_canvas_painter* mg_metal_painter_create_for_surface(mg_metal_surface* surface, mp_rect viewPort);
#endif

mg_canvas_painter* mg_canvas_painter_create_for_surface(mg_surface surfaceHandle, mp_rect viewPort)
{
	mg_surface_info* surface = mg_surface_ptr_from_handle(surfaceHandle);
	if(!surface)
	{
		return(0);
	}

	switch(surface->backend)
	{
		case MG_BACKEND_METAL:
			return(mg_metal_painter_create_for_surface((mg_metal_surface*)surface, viewPort));

		default:
			return(0);
	}
}

void mg_canvas_reset_streams(mg_canvas_data* context)
{
	ListInit(&context->streamsFreeList);
	for(int i=0; i<MG_STREAM_MAX_COUNT; i++)
	{
		ListAppend(&context->streamsFreeList, &context->streams[i].freeListElt);
		context->streams[i].generation = 1;
	}
	context->currentStream = mg_stream_alloc(context);
}

mg_canvas mg_canvas_create(mg_surface surface, mp_rect viewPort)
{
	mg_canvas_data* context = mg_canvas_alloc();
	if(!context)
	{
		return(mg_canvas_nil());
	}

	context->path.startIndex = 0;
	context->path.count = 0;
	context->nextZIndex = 1;
	context->attributes.color = (mg_color){0, 0, 0, 1};
	context->attributes.tolerance = 1;
	context->attributes.width = 10;
	context->attributes.clip = (mp_rect){-FLT_MAX/2, -FLT_MAX/2, FLT_MAX, FLT_MAX};

	context->transform = (mg_mat2x3){{1, 0, 0,
	                                  0, 1, 0}};

	mg_canvas_reset_streams(context);

	context->painter = mg_canvas_painter_create_for_surface(surface, viewPort);
	if(!context->painter)
	{
		LOG_ERROR("Couldn't create painter for surface\n");
		mg_canvas_recycle(context);
		return(mg_canvas_nil());
	}

	mg_canvas handle = mg_canvas_handle_from_ptr(context);

	//NOTE: create a blank image
	u8 bytes[4] = {255, 255, 255, 255};
	context->blankImage = mg_image_create_from_rgba8(handle, 1, 1, bytes);

	return(handle);
}

void mg_canvas_destroy(mg_canvas handle)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	mg_canvas_recycle(context);
}

void mg_canvas_viewport(mg_canvas handle, mp_rect viewport)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	context->painter->setViewPort(context->painter, viewport);
}

void mg_canvas_begin_draw(mg_canvas handle)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	context->vertexCount = 0;
	context->indexCount = 0;
	context->path.startIndex = 0;
	context->path.count = 0;
}

mg_mat2x3 mg_matrix_stack_top(mg_canvas_data* context)
{
	if(context->matrixStackSize == 0)
	{
		return((mg_mat2x3){1, 0, 0,
				   0, 1, 0});
	}
	else
	{
		return(context->matrixStack[context->matrixStackSize-1]);
	}
}

void mg_matrix_stack_push(mg_canvas_data* context, mg_mat2x3 transform)
{
	if(context->matrixStackSize >= MG_MATRIX_STACK_MAX_DEPTH)
	{
		LOG_ERROR("matrix stack overflow\n");
	}
	else
	{
		context->matrixStack[context->matrixStackSize] = transform;
		context->matrixStackSize++;
		context->transform = transform;
	}
}

void mg_matrix_stack_pop(mg_canvas_data* context)
{
	if(context->matrixStackSize == 0)
	{
		LOG_ERROR("matrix stack underflow\n");
	}
	else
	{
		context->matrixStackSize--;
		context->transform = mg_matrix_stack_top(context);
	}
}


mp_rect mg_clip_stack_top(mg_canvas_data* context)
{
	if(context->clipStackSize == 0)
	{
		return((mp_rect){-FLT_MAX/2, -FLT_MAX/2, FLT_MAX, FLT_MAX});
	}
	else
	{
		return(context->clipStack[context->clipStackSize-1]);
	}
}

void mg_clip_stack_push(mg_canvas_data* context, mp_rect clip)
{
	if(context->clipStackSize >= MG_CLIP_STACK_MAX_DEPTH)
	{
		LOG_ERROR("clip stack overflow\n");
	}
	else
	{
		context->clipStack[context->clipStackSize] = clip;
		context->clipStackSize++;
		context->clip = clip;
	}
}

void mg_clip_stack_pop(mg_canvas_data* context)
{
	if(context->clipStackSize == 0)
	{
		LOG_ERROR("clip stack underflow\n");
	}
	else
	{
		context->clipStackSize--;
		context->clip = mg_clip_stack_top(context);
	}
}

void mg_do_clip_push(mg_canvas_data* context, mp_rect clip)
{
	//NOTE(martin): transform clip
	vec2 p0 = mg_mat2x3_mul(context->transform, (vec2){clip.x, clip.y});
	vec2 p1 = mg_mat2x3_mul(context->transform, (vec2){clip.x + clip.w, clip.y});
	vec2 p2 = mg_mat2x3_mul(context->transform, (vec2){clip.x + clip.w, clip.y + clip.h});
	vec2 p3 = mg_mat2x3_mul(context->transform, (vec2){clip.x, clip.y + clip.h});

	f32 x0 = minimum(p0.x, minimum(p1.x, minimum(p2.x, p3.x)));
	f32 y0 = minimum(p0.y, minimum(p1.y, minimum(p2.y, p3.y)));
	f32 x1 = maximum(p0.x, maximum(p1.x, maximum(p2.x, p3.x)));
	f32 y1 = maximum(p0.y, maximum(p1.y, maximum(p2.y, p3.y)));

	mp_rect current = mg_clip_stack_top(context);

	//NOTE(martin): intersect with current clip
	x0 = maximum(current.x, x0);
	y0 = maximum(current.y, y0);
	x1 = minimum(current.x + current.w, x1);
	y1 = minimum(current.y + current.h, y1);

	mp_rect r = {x0, y0, maximum(0, x1-x0), maximum(0, y1-y0)};
	mg_clip_stack_push(context, r);
}

void mg_canvas_flush(mg_canvas contextHandle)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(contextHandle);
	if(!context)
	{
		return;
	}

	mg_color clearColor = {0, 0, 0, 1};

	u32 count = context->primitiveCount;

	mg_stream_data* stream = context->currentStream;
	DEBUG_ASSERT(stream);
	DEBUG_ASSERT(stream->count <= count);

	u32 nextIndex = stream->firstCommand;

	mg_reset_z_index(context);
	context->transform = (mg_mat2x3){1, 0, 0,
	                                 0, 1, 0};
	context->clip = (mp_rect){-FLT_MAX/2, -FLT_MAX/2, FLT_MAX, FLT_MAX};

	for(int i=0; i<stream->count; i++)
	{
		if(nextIndex >= count)
		{
			LOG_ERROR("invalid location '%i' in graphics command buffer would cause an overrun\n", nextIndex);
			break;
		}
		mg_primitive* primitive = &(context->primitives[nextIndex]);
		nextIndex++;

		switch(primitive->cmd)
		{
			case MG_CMD_CLEAR:
			{
				//NOTE(martin): clear buffers
				context->vertexCount = 0;
				context->indexCount = 0;

				clearColor = primitive->attributes.color;
			} break;

			case MG_CMD_FILL:
			{
				u32 zIndex = mg_get_next_z_index(context);
				mg_render_fill(context,
						       context->pathElements + primitive->path.startIndex,
						       &primitive->path,
						       zIndex,
						       primitive->attributes.color);
			} break;

			case MG_CMD_STROKE:
			{
				mg_render_stroke(context,
							 context->pathElements + primitive->path.startIndex,
							 &primitive->path,
							 &primitive->attributes);
			} break;


			case MG_CMD_RECT_FILL:
				mg_render_rectangle_fill(context, primitive->rect, &primitive->attributes);
				break;

			case MG_CMD_RECT_STROKE:
				mg_render_rectangle_stroke(context, primitive->rect, &primitive->attributes);
				break;

			case MG_CMD_ROUND_RECT_FILL:
				mg_render_rounded_rectangle_fill(context, primitive->roundedRect, &primitive->attributes);
				break;

			case MG_CMD_ROUND_RECT_STROKE:
				mg_render_rounded_rectangle_stroke(context, primitive->roundedRect, &primitive->attributes);
				break;

			case MG_CMD_ELLIPSE_FILL:
				mg_render_ellipse_fill(context, primitive->rect, &primitive->attributes);
				break;

			case MG_CMD_ELLIPSE_STROKE:
				mg_render_ellipse_stroke(context, primitive->rect, &primitive->attributes);
				break;

			case MG_CMD_JUMP:
			{
				if(primitive->jump == ~0)
				{
					//NOTE(martin): normal end of stream marker
					goto exit_command_loop;
				}
				else if(primitive->jump >= count)
				{
					LOG_ERROR("invalid jump location '%i' in graphics command buffer\n", primitive->jump);
					goto exit_command_loop;
				}
				else
				{
					nextIndex = primitive->jump;
				}
			} break;

			case MG_CMD_MATRIX_PUSH:
			{
				mg_mat2x3 transform = mg_matrix_stack_top(context);
				mg_matrix_stack_push(context, mg_mat2x3_mul_m(transform, primitive->matrix));
			} break;

			case MG_CMD_MATRIX_POP:
			{
				mg_matrix_stack_pop(context);
			} break;

			case MG_CMD_CLIP_PUSH:
			{
				//TODO(martin): use only aligned rect and avoid this
				mp_rect r = {primitive->rect.x, primitive->rect.y, primitive->rect.w, primitive->rect.h};
				mg_do_clip_push(context, r);
			} break;

			case MG_CMD_CLIP_POP:
			{
				mg_clip_stack_pop(context);
			} break;

			case MG_CMD_IMAGE_DRAW:
			{
				mg_render_image(context, primitive->attributes.image, primitive->rect);
			} break;

			case MG_CMD_ROUNDED_IMAGE_DRAW:
			{
				mg_render_rounded_image(context, primitive->attributes.image, primitive->roundedRect, &primitive->attributes);
			} break;

		}
	}
	exit_command_loop: ;

	if(context->painter && context->painter->drawBuffers)
	{
		context->painter->drawBuffers(context->painter, context->vertexCount, context->indexCount, clearColor);
	}

	//NOTE(martin): clear buffers
	context->primitiveCount = 0;
	context->vertexCount = 0;
	context->indexCount = 0;

	context->path.startIndex = 0;
	context->path.count = 0;

	//NOTE(martin): invalidate all streams
	context->frameCounter++;
	mg_canvas_reset_streams(context);
}

void mg_push_command(mg_canvas_data* context, mg_primitive primitive)
{
	//NOTE(martin): push primitive and updates current stream, eventually patching a pending jump.
	ASSERT(context->primitiveCount < MG_MAX_PRIMITIVE_COUNT);
	context->primitives[context->primitiveCount] = primitive;
	context->primitiveCount++;

	mg_stream_data* stream = context->currentStream;
	DEBUG_ASSERT(stream);

	u32 lastCommand = context->primitiveCount-1;
	if(stream->pendingJump)
	{
		DEBUG_ASSERT(context->primitives[stream->lastCommand].cmd == MG_CMD_JUMP);
		context->primitives[stream->lastCommand].jump = lastCommand;
		stream->pendingJump = false;
	}
	stream->lastCommand = lastCommand;
	stream->count++;
}

//-----------------------------------------------------------------------------------------------------------
// Graphics command streams
//-----------------------------------------------------------------------------------------------------------

mg_stream mg_stream_create(mg_canvas contextHandle)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(contextHandle);
	if(!context)
	{
		return(mg_stream_null_handle());
	}

	mg_stream_data* stream = mg_stream_alloc(context);
	if(!stream)
	{
		return(mg_stream_null_handle());
	}

	return(mg_stream_handle_from_ptr(context, stream));
}

mg_stream mg_stream_swap(mg_canvas contextHandle, mg_stream streamHandle)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(contextHandle);
	if(!context)
	{
		return(mg_stream_null_handle());
	}
	mg_stream_data* stream = mg_stream_ptr_from_handle(context, streamHandle);
	if(!stream)
	{
		return(mg_stream_null_handle());
	}

	if(stream == context->currentStream)
	{
		//NOTE(martin): prevent swapping for itself
		return(mg_stream_null_handle());
	}

	//NOTE(martin): push an unitialized jump to the current stream
	mg_push_command(context, (mg_primitive){.cmd = MG_CMD_JUMP, .jump = ~0});
	context->currentStream->pendingJump = true;

	//NOTE(martin): set the new current stream
	mg_stream_data* oldStream = context->currentStream;
	context->currentStream = stream;

	if(!stream->count)
	{
		//NOTE(martin): if stream is new, set the first command to the current offset in command buffer
		stream->firstCommand = context->primitiveCount;
		stream->lastCommand = stream->firstCommand;
	}
	else
	{
		//NOTE(martin): else, we just make sure there is a pending jump to be patched when we push the next command
		DEBUG_ASSERT(stream->pendingJump);
	}

	return(mg_stream_handle_from_ptr(context, oldStream));
}

void mg_stream_append(mg_canvas contextHandle, mg_stream streamHandle)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(contextHandle);
	if(!context)
	{
		return;
	}
	mg_stream_data* stream = mg_stream_ptr_from_handle(context, streamHandle);
	if(!stream)
	{
		return;
	}
	if(stream == context->currentStream)
	{
		//NOTE(martin): prevent appending to itself
		return;
	}

	if(stream->count)
	{
		//NOTE(martin): push a jump set to the appended stream's first command
		mg_push_command(context, (mg_primitive){.cmd = MG_CMD_JUMP, .jump = stream->firstCommand});

		//NOTE(martin): update current stream last command and make sure it's a jump
		context->currentStream->lastCommand = stream->lastCommand;
		context->currentStream->count += stream->count;
		context->currentStream->pendingJump = true;

		DEBUG_ASSERT(context->primitives[context->currentStream->lastCommand].cmd == MG_CMD_JUMP);
	}

	//NOTE(martin): invalidate appended stream
	mg_stream_recycle(context, stream);
}

//-----------------------------------------------------------------------------------------------------------
// Fonts management
//-----------------------------------------------------------------------------------------------------------

mg_font mg_font_nil()
{
	return((mg_font){0});
}

mg_font mg_font_create_from_memory(u32 size, byte* buffer, u32 rangeCount, unicode_range* ranges)
{
	mg_font_info* fontInfo = ListPopEntry(&__mgInfo.fontFreeList, mg_font_info, freeListElt);
	if(!fontInfo)
	{
		if(__mgInfo.fontsNextIndex < MG_FONT_MAX_COUNT)
		{
			fontInfo = &(__mgInfo.fonts[__mgInfo.fontsNextIndex]);
			__mgInfo.fontsNextIndex++;
			memset(fontInfo, 0, sizeof(*fontInfo));
			fontInfo->generation = 1;
		}
		else
		{
			LOG_ERROR("can't allocate new font\n");
			return((mg_font){0});
		}
	}

	stbtt_fontinfo stbttFontInfo;
	stbtt_InitFont(&stbttFontInfo, buffer, 0);

	//NOTE(martin): load font metrics data
	fontInfo->unitsPerEm = 1./stbtt_ScaleForMappingEmToPixels(&stbttFontInfo, 1);

	int ascent, descent, lineGap, x0, x1, y0, y1;
	stbtt_GetFontVMetrics(&stbttFontInfo, &ascent, &descent, &lineGap);
	stbtt_GetFontBoundingBox(&stbttFontInfo, &x0, &y0, &x1, &y1);

	fontInfo->extents.ascent = ascent;
	fontInfo->extents.descent = -descent;
	fontInfo->extents.leading = lineGap;
	fontInfo->extents.width = x1 - x0;

	stbtt_GetCodepointBox(&stbttFontInfo, 'x', &x0, &y0, &x1, &y1);
	fontInfo->extents.xHeight = y1 - y0;

	stbtt_GetCodepointBox(&stbttFontInfo, 'M', &x0, &y0, &x1, &y1);
	fontInfo->extents.capHeight = y1 - y0;

	//NOTE(martin): load codepoint ranges
	fontInfo->rangeCount = rangeCount;
	fontInfo->glyphMap = malloc_array(mg_glyph_map_entry, rangeCount);
	fontInfo->glyphCount = 0;

	for(int i=0; i<rangeCount; i++)
	{
		//NOTE(martin): initialize the map entry.
		//              The glyph indices are offseted by 1, to reserve 0 as an invalid glyph index.
		fontInfo->glyphMap[i].range = ranges[i];
		fontInfo->glyphMap[i].firstGlyphIndex = fontInfo->glyphCount + 1;
		fontInfo->glyphCount += ranges[i].count;
	}

	fontInfo->glyphs = malloc_array(mg_glyph_info, fontInfo->glyphCount);

	//NOTE(martin): first do a count of outlines
	int outlineCount = 0;
	for(int rangeIndex=0; rangeIndex<rangeCount; rangeIndex++)
	{
		utf32 codePoint = fontInfo->glyphMap[rangeIndex].range.firstCodePoint;
		u32 firstGlyphIndex = fontInfo->glyphMap[rangeIndex].firstGlyphIndex;
		u32 endGlyphIndex = firstGlyphIndex + fontInfo->glyphMap[rangeIndex].range.count;

		for(int glyphIndex = firstGlyphIndex;
		    glyphIndex < endGlyphIndex; glyphIndex++)
		{
			int stbttGlyphIndex = stbtt_FindGlyphIndex(&stbttFontInfo, codePoint);
			if(stbttGlyphIndex == 0)
			{
				//NOTE(martin): the codepoint is not found in the font
				codePoint++;
				continue;
			}
			//NOTE(martin): load glyph outlines
			stbtt_vertex* vertices = 0;
			outlineCount += stbtt_GetGlyphShape(&stbttFontInfo, stbttGlyphIndex, &vertices);
			stbtt_FreeShape(&stbttFontInfo, vertices);
			codePoint++;
		}
	}
	//NOTE(martin): allocate outlines
	fontInfo->outlines = malloc_array(mg_path_elt, outlineCount);
	fontInfo->outlineCount = 0;

	//NOTE(martin): load metrics and outlines
	for(int rangeIndex=0; rangeIndex<rangeCount; rangeIndex++)
	{
		utf32 codePoint = fontInfo->glyphMap[rangeIndex].range.firstCodePoint;
		u32 firstGlyphIndex = fontInfo->glyphMap[rangeIndex].firstGlyphIndex;
		u32 endGlyphIndex = firstGlyphIndex + fontInfo->glyphMap[rangeIndex].range.count;

		for(int glyphIndex = firstGlyphIndex;
		    glyphIndex < endGlyphIndex; glyphIndex++)
		{
			mg_glyph_info* glyph = &(fontInfo->glyphs[glyphIndex-1]);

			int stbttGlyphIndex = stbtt_FindGlyphIndex(&stbttFontInfo, codePoint);
			if(stbttGlyphIndex == 0)
			{
				//NOTE(martin): the codepoint is not found in the font, we zero the glyph info
				memset(glyph, 0, sizeof(*glyph));
				codePoint++;
				continue;
			}

			glyph->exists = true;
			glyph->codePoint = codePoint;

			//NOTE(martin): load glyph metric
			int xAdvance, xBearing, x0, y0, x1, y1;
			stbtt_GetGlyphHMetrics(&stbttFontInfo, stbttGlyphIndex, &xAdvance, &xBearing);
			stbtt_GetGlyphBox(&stbttFontInfo, stbttGlyphIndex, &x0, &y0, &x1, &y1);

			glyph->extents.xAdvance	= (f32)xAdvance;
			glyph->extents.yAdvance = 0;
			glyph->extents.xBearing = (f32)xBearing;
			glyph->extents.yBearing = y0;

			glyph->extents.width = x1 - x0;
			glyph->extents.height = y1 - y0;

			//NOTE(martin): load glyph outlines

			stbtt_vertex* vertices = 0;
			int vertexCount = stbtt_GetGlyphShape(&stbttFontInfo, stbttGlyphIndex, &vertices);

			glyph->pathDescriptor = (mg_path_descriptor){.startIndex = fontInfo->outlineCount,
			                                                      .count = vertexCount,
									      .startPoint = {0, 0}};

			mg_path_elt* elements = fontInfo->outlines + fontInfo->outlineCount;
			fontInfo->outlineCount += vertexCount;
			vec2 currentPos = {0, 0};

			for(int vertIndex = 0; vertIndex < vertexCount; vertIndex++)
			{
				f32 x = vertices[vertIndex].x;
				f32 y = vertices[vertIndex].y;
				f32 cx = vertices[vertIndex].cx;
				f32 cy = vertices[vertIndex].cy;
				f32 cx1 = vertices[vertIndex].cx1;
				f32 cy1 = vertices[vertIndex].cy1;

				switch(vertices[vertIndex].type)
				{
					case STBTT_vmove:
						elements[vertIndex].type = MG_PATH_MOVE;
						elements[vertIndex].p[0] = (vec2){x, y};
						break;

					case STBTT_vline:
						elements[vertIndex].type = MG_PATH_LINE;
						elements[vertIndex].p[0] = (vec2){x, y};
						break;

					case STBTT_vcurve:
					{
						elements[vertIndex].type = MG_PATH_QUADRATIC;
						elements[vertIndex].p[0] = (vec2){cx, cy};
						elements[vertIndex].p[1] = (vec2){x, y};
					} break;

					case STBTT_vcubic:
						elements[vertIndex].type = MG_PATH_CUBIC;
						elements[vertIndex].p[0] = (vec2){cx, cy};
						elements[vertIndex].p[1] = (vec2){cx1, cy1};
						elements[vertIndex].p[2] = (vec2){x, y};
						break;
				}
				currentPos = (vec2){x, y};
			}
			stbtt_FreeShape(&stbttFontInfo, vertices);
			codePoint++;
		}
	}
	u64 h = ((u64)(fontInfo - __mgInfo.fonts))<<32 | ((u64)fontInfo->generation);
	return((mg_font){h});
}

mg_font_info* mg_get_font_info(mg_font fontHandle)
{
	u32 fontIndex = (u32)(fontHandle.h >> 32);
	u32 fontGeneration = (u32)(fontHandle.h & 0xffffffff);

	if(  fontIndex >= MG_FONT_MAX_COUNT
	  || fontIndex >= __mgInfo.fontsNextIndex
	  || fontGeneration != __mgInfo.fonts[fontIndex].generation)
	{
		LOG_WARNING("invalid font handle\n");
		return(0);
	}
	return(&(__mgInfo.fonts[fontIndex]));
}

void mg_font_destroy(mg_font fontHandle)
{
	mg_font_info* fontInfo = mg_get_font_info(fontHandle);
	if(!fontInfo)
	{
		return;
	}
	#ifdef DEBUG
		if(fontInfo->generation == UINT32_MAX)
		{
			LOG_ERROR("font info generation wrap around\n");
		}
	#endif
	fontInfo->generation++;

	free(fontInfo->glyphMap);
	free(fontInfo->glyphs);
	free(fontInfo->outlines);

	ListPush(&__mgInfo.fontFreeList, &fontInfo->freeListElt);
}

str32 mg_font_get_glyph_indices_from_font_info(mg_font_info* fontInfo, str32 codePoints, str32 backing)
{
	u64 count = minimum(codePoints.len, backing.len);

	for(int i = 0; i<count; i++)
	{
		u32 glyphIndex = 0;
		for(int rangeIndex=0; rangeIndex < fontInfo->rangeCount; rangeIndex++)
		{
			if(codePoints.ptr[i] >= fontInfo->glyphMap[rangeIndex].range.firstCodePoint
			  && codePoints.ptr[i] < (fontInfo->glyphMap[rangeIndex].range.firstCodePoint + fontInfo->glyphMap[rangeIndex].range.count))
			{
				u32 rangeOffset = codePoints.ptr[i] - fontInfo->glyphMap[rangeIndex].range.firstCodePoint;
				glyphIndex = fontInfo->glyphMap[rangeIndex].firstGlyphIndex + rangeOffset;
				break;
			}
		}
		if(glyphIndex && !fontInfo->glyphs[glyphIndex].exists)
		{
			backing.ptr[i] = 0;
		}
		backing.ptr[i] = glyphIndex;
	}
	str32 res = {.len = count, .ptr = backing.ptr};
	return(res);
}

u32 mg_font_get_glyph_index_from_font_info(mg_font_info* fontInfo, utf32 codePoint)
{
	u32 glyphIndex = 0;
	str32 codePoints = {1, &codePoint};
	str32 backing = {1, &glyphIndex};
	mg_font_get_glyph_indices_from_font_info(fontInfo, codePoints, backing);
	return(glyphIndex);
}

str32 mg_font_get_glyph_indices(mg_font font, str32 codePoints, str32 backing)
{
	mg_font_info* fontInfo = mg_get_font_info(font);
	if(!fontInfo)
	{
		return((str32){});
	}
	return(mg_font_get_glyph_indices_from_font_info(fontInfo, codePoints, backing));
}

str32 mg_font_push_glyph_indices(mg_font font, mem_arena* arena, str32 codePoints)
{
	u32* buffer = mem_arena_alloc_array(arena, u32, codePoints.len);
	str32 backing = {codePoints.len, buffer};
	return(mg_font_get_glyph_indices(font, codePoints, backing));
}

u32 mg_font_get_glyph_index(mg_font font, utf32 codePoint)
{
	u32 glyphIndex = 0;
	str32 codePoints = {1, &codePoint};
	str32 backing = {1, &glyphIndex};
	mg_font_get_glyph_indices(font, codePoints, backing);
	return(glyphIndex);
}

mg_glyph_info* mg_font_get_glyph_info(mg_font_info* fontInfo, u32 glyphIndex)
{
	DEBUG_ASSERT(glyphIndex);
	DEBUG_ASSERT(glyphIndex < fontInfo->glyphCount);
	return(&(fontInfo->glyphs[glyphIndex-1]));
}

mg_font_extents mg_font_get_extents(mg_font font)
{
	mg_font_info* fontInfo = mg_get_font_info(font);
	if(!fontInfo)
	{
		return((mg_font_extents){});
	}
	return(fontInfo->extents);
}

mg_font_extents mg_font_get_scaled_extents(mg_font font, f32 emSize)
{
	mg_font_info* fontInfo = mg_get_font_info(font);
	if(!fontInfo)
	{
		return((mg_font_extents){});
	}
	f32 scale = emSize/fontInfo->unitsPerEm;
	mg_font_extents extents = fontInfo->extents;

	extents.ascent *= scale;
	extents.descent *= scale;
	extents.leading *= scale;
	extents.xHeight *= scale;
	extents.capHeight *= scale;
	extents.width *= scale;

	return(extents);
}


f32 mg_font_get_scale_for_em_pixels(mg_font font, f32 emSize)
{
	mg_font_info* fontInfo = mg_get_font_info(font);
	if(!fontInfo)
	{
		return(0);
	}
	return(emSize/fontInfo->unitsPerEm);
}

void mg_font_get_glyph_extents_from_font_info(mg_font_info* fontInfo,
                                              str32 glyphIndices,
                                              mg_text_extents* outExtents)
{
	for(int i=0; i<glyphIndices.len; i++)
	{
		if(!glyphIndices.ptr[i] || glyphIndices.ptr[i] >= fontInfo->glyphCount)
		{
			continue;
		}
		mg_glyph_info* glyph = mg_font_get_glyph_info(fontInfo, glyphIndices.ptr[i]);
		outExtents[i] = glyph->extents;
	}
}

int mg_font_get_glyph_extents(mg_font font, str32 glyphIndices, mg_text_extents* outExtents)
{
	mg_font_info* fontInfo = mg_get_font_info(font);
	if(!fontInfo)
	{
		return(-1);
	}
	mg_font_get_glyph_extents_from_font_info(fontInfo, glyphIndices, outExtents);
	return(0);
}

int mg_font_get_codepoint_extents(mg_font font, utf32 codePoint, mg_text_extents* outExtents)
{
	mg_font_info* fontInfo = mg_get_font_info(font);
	if(!fontInfo)
	{
		return(-1);
	}
	u32 glyphIndex = 0;
	str32 codePoints = {1, &codePoint};
	str32 backing = {1, &glyphIndex};
	str32 glyphs = mg_font_get_glyph_indices_from_font_info(fontInfo, codePoints, backing);
	mg_font_get_glyph_extents_from_font_info(fontInfo, glyphs, outExtents);
	return(0);
}

mp_rect mg_text_bounding_box_utf32(mg_font font, f32 fontSize, str32 codePoints)
{
	if(!codePoints.len || !codePoints.ptr)
	{
		return((mp_rect){});
	}

	mg_font_info* fontInfo = mg_get_font_info(font);
	if(!fontInfo)
	{
		return((mp_rect){});
	}

	mem_arena* scratch = mem_scratch();
	str32 glyphIndices = mg_font_push_glyph_indices(font, scratch, codePoints);

	//NOTE(martin): find width of missing character
	//TODO(martin): should cache that at font creation...
	mg_text_extents missingGlyphExtents;
	u32 missingGlyphIndex = mg_font_get_glyph_index_from_font_info(fontInfo, 0xfffd);

	if(missingGlyphIndex)
	{
		mg_font_get_glyph_extents_from_font_info(fontInfo, (str32){1, &missingGlyphIndex}, &missingGlyphExtents);
	}
	else
	{
		//NOTE(martin): could not find replacement glyph, try to get an 'x' to get a somewhat correct width
		//              to render an empty rectangle. Otherwise just render with the max font width
		f32 boxWidth = fontInfo->extents.width * 0.8;
		f32 xBearing = fontInfo->extents.width * 0.1;
		f32 xAdvance = fontInfo->extents.width;

		missingGlyphIndex = mg_font_get_glyph_index_from_font_info(fontInfo, 'x');
		if(missingGlyphIndex)
		{
			mg_font_get_glyph_extents_from_font_info(fontInfo, (str32){1, &missingGlyphIndex}, &missingGlyphExtents);
		}
		else
		{
			missingGlyphExtents.xBearing = fontInfo->extents.width * 0.1;
			missingGlyphExtents.yBearing = 0;
			missingGlyphExtents.width = fontInfo->extents.width * 0.8;
			missingGlyphExtents.xAdvance = fontInfo->extents.width;
			missingGlyphExtents.yAdvance = 0;
		}
	}

	//NOTE(martin): accumulate text extents
	f32 width = 0;
	f32 x = 0;
	f32 y = 0;
	f32 lineHeight = fontInfo->extents.descent + fontInfo->extents.ascent;

	for(int i=0; i<glyphIndices.len; i++)
	{
		//TODO(martin): make it failsafe for fonts that don't have a glyph for the line-feed codepoint ?

		mg_glyph_info* glyph = 0;
		mg_text_extents extents;
		if(!glyphIndices.ptr[i] || glyphIndices.ptr[i] >= fontInfo->glyphCount)
		{
			extents = missingGlyphExtents;
		}
		else
		{
			glyph = mg_font_get_glyph_info(fontInfo, glyphIndices.ptr[i]);
			extents = glyph->extents;
		}
		x += extents.xAdvance;
		y += extents.yAdvance;

		if(glyph && glyph->codePoint == '\n')
		{
			width = maximum(width, x);
			x = 0;
			y += lineHeight + fontInfo->extents.leading;
		}
	}
	width = maximum(width, x);

	f32 fontScale = mg_font_get_scale_for_em_pixels(font, fontSize);
	mp_rect rect = {0, -fontInfo->extents.ascent * fontScale, width * fontScale, (y + lineHeight) * fontScale };
	return(rect);
}

mp_rect mg_text_bounding_box(mg_font font, f32 fontSize, str8 text)
{
	if(!text.len || !text.ptr)
	{
		return((mp_rect){});
	}

	mem_arena* scratch = mem_scratch();
	str32 codePoints = utf8_push_to_codepoints(scratch, text);
	return(mg_text_bounding_box_utf32(font, fontSize, codePoints));
}

//-----------------------------------------------------------------------------------------------------------
// Transform matrix settings
//-----------------------------------------------------------------------------------------------------------

void mg_matrix_push(mg_canvas handle, mg_mat2x3 matrix)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}

	mg_push_command(context, (mg_primitive){.cmd = MG_CMD_MATRIX_PUSH, .matrix = matrix});
}

void mg_matrix_pop(mg_canvas handle)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	mg_push_command(context, (mg_primitive){.cmd = MG_CMD_MATRIX_POP});
}
//-----------------------------------------------------------------------------------------------------------
// Graphics attributes settings
//-----------------------------------------------------------------------------------------------------------

void mg_set_color(mg_canvas handle, mg_color color)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	context->attributes.color = color;
}

void mg_set_color_rgba(mg_canvas handle, f32 r, f32 g, f32 b, f32 a)
{
	mg_set_color(handle, (mg_color){r, g, b, a});
}

void mg_set_width(mg_canvas handle, f32 width)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	context->attributes.width = width;
}

void mg_set_tolerance(mg_canvas handle, f32 tolerance)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	context->attributes.tolerance = tolerance;
}

void mg_set_joint(mg_canvas handle, mg_joint_type joint)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	context->attributes.joint = joint;
}

void mg_set_max_joint_excursion(mg_canvas handle, f32 maxJointExcursion)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	context->attributes.maxJointExcursion = maxJointExcursion;
}

void mg_set_cap(mg_canvas handle, mg_cap_type cap)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	context->attributes.cap = cap;
}

void mg_set_font(mg_canvas handle, mg_font font)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	context->attributes.font = font;
}

void mg_set_font_size(mg_canvas handle, f32 fontSize)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	context->attributes.fontSize = fontSize;
}

void mg_set_text_flip(mg_canvas handle, bool flip)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	context->textFlip = flip;
}



mg_color mg_get_color(mg_canvas handle)
{
	mg_color color = {0};
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(context)
	{
		color = context->attributes.color;
	}
	return(color);
}

f32 mg_get_width(mg_canvas handle)
{
	f32 width = 0;
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(context)
	{
		width = context->attributes.width;
	}
	return(width);
}

f32 mg_get_tolerance(mg_canvas handle)
{
	f32 tolerance = 0;
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(context)
	{
		tolerance = context->attributes.tolerance;
	}
	return(tolerance);
}

mg_joint_type mg_get_joint(mg_canvas handle)
{
	mg_joint_type joint = 0;
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(context)
	{
		joint = context->attributes.joint;
	}
	return(joint);
}

f32 mg_get_max_joint_excursion(mg_canvas handle)
{
	f32 maxJointExcursion = 0;
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(context)
	{
		maxJointExcursion = context->attributes.maxJointExcursion;
	}
	return(maxJointExcursion);
}

mg_cap_type mg_get_cap(mg_canvas handle)
{
	mg_cap_type cap = 0;
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(context)
	{
		cap = context->attributes.cap;
	}
	return(cap);
}

mg_font mg_get_font(mg_canvas handle)
{
	mg_font font = mg_font_nil();
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(context)
	{
		font = context->attributes.font;
	}
	return(font);
}

f32 mg_get_font_size(mg_canvas handle)
{
	f32 fontSize = 0;
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(context)
	{
		fontSize = context->attributes.fontSize;
	}
	return(fontSize);
}

bool mg_get_text_flip(mg_canvas handle)
{
	bool flip = false;
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(context)
	{
		flip = context->textFlip;
	}
	return(flip);
}



//-----------------------------------------------------------------------------------------------------------
// Clip
//-----------------------------------------------------------------------------------------------------------

void mg_clip_push(mg_canvas handle, f32 x, f32 y, f32 w, f32 h)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	mg_push_command(context, (mg_primitive){.cmd = MG_CMD_CLIP_PUSH,
	                                        .rect = (mp_rect){x, y, w, h}});
}

void mg_clip_pop(mg_canvas handle)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	mg_push_command(context, (mg_primitive){.cmd = MG_CMD_CLIP_POP});
}

//-----------------------------------------------------------------------------------------------------------
// Path construction
//-----------------------------------------------------------------------------------------------------------

void mg_new_path(mg_canvas_data* context)
{
	context->path.startIndex += context->path.count;
	context->path.count = 0;
	context->subPathStartPoint = context->subPathLastPoint;
	context->path.startPoint = context->subPathStartPoint;
}

void mg_clear(mg_canvas handle)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	mg_push_command(context, (mg_primitive){.cmd = MG_CMD_CLEAR, .attributes = context->attributes});
}

vec2 mg_get_position(mg_canvas handle)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return((vec2){0, 0});
	}
	return(context->subPathLastPoint);
}

void mg_path_push_elements(mg_canvas_data* context, u32 count, mg_path_elt* elements)
{
	ASSERT(context->path.count + context->path.startIndex + count <= MG_MAX_PATH_ELEMENT_COUNT);
	memcpy(context->pathElements + context->path.startIndex + context->path.count, elements, count*sizeof(mg_path_elt));
	context->path.count += count;
}

void mg_path_push_element(mg_canvas_data* context, mg_path_elt elt)
{
	mg_path_push_elements(context, 1, &elt);
}



void mg_move_to(mg_canvas handle, f32 x, f32 y)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	mg_path_push_element(context, ((mg_path_elt){.type = MG_PATH_MOVE, .p[0] = {x, y}}));
	context->subPathStartPoint = (vec2){x, y};
	context->subPathLastPoint = (vec2){x, y};
}

void mg_line_to(mg_canvas handle, f32 x, f32 y)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	mg_path_push_element(context, ((mg_path_elt){.type = MG_PATH_LINE, .p[0] = {x, y}}));
	context->subPathLastPoint = (vec2){x, y};
}

void mg_quadratic_to(mg_canvas handle, f32 x1, f32 y1, f32 x2, f32 y2)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	mg_path_push_element(context, ((mg_path_elt){.type = MG_PATH_QUADRATIC, .p = {{x1, y1}, {x2, y2}}}));
	context->subPathLastPoint = (vec2){x2, y2};
}

void mg_cubic_to(mg_canvas handle, f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	mg_path_push_element(context, ((mg_path_elt){.type = MG_PATH_CUBIC, .p = {{x1, y1}, {x2, y2}, {x3, y3}}}));
	context->subPathLastPoint = (vec2){x3, y3};
}

void mg_close_path(mg_canvas handle)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	if(  context->subPathStartPoint.x != context->subPathLastPoint.x
	  || context->subPathStartPoint.y != context->subPathLastPoint.y)
	{
		mg_line_to(handle, context->subPathStartPoint.x, context->subPathStartPoint.y);
	}
	context->subPathStartPoint = context->subPathLastPoint;
}

mp_rect mg_glyph_outlines_from_font_info(mg_canvas_data* context, mg_font_info* fontInfo, str32 glyphIndices)
{
	mg_canvas contextHandle = mg_canvas_handle_from_ptr(context);

	f32 startX = context->subPathLastPoint.x;
	f32 startY = context->subPathLastPoint.y;
	f32 maxWidth = 0;

	f32 scale = context->attributes.fontSize/fontInfo->unitsPerEm;

	for(int i=0; i<glyphIndices.len; i++)
	{
		u32 glyphIndex = glyphIndices.ptr[i];

		f32 xOffset = context->subPathLastPoint.x;
		f32 yOffset = context->subPathLastPoint.y;
		f32 flip = context->textFlip ? -1 : 1;

		if(!glyphIndex || glyphIndex >= fontInfo->glyphCount)
		{
			LOG_WARNING("code point is not present in font ranges\n");
			//NOTE(martin): try to find the replacement character
			glyphIndex = mg_font_get_glyph_index_from_font_info(fontInfo, 0xfffd);
			if(!glyphIndex)
			{
				//NOTE(martin): could not find replacement glyph, try to get an 'x' to get a somewhat correct width
				//              to render an empty rectangle. Otherwise just render with the max font width
				f32 boxWidth = fontInfo->extents.width * 0.8;
				f32 xBearing = fontInfo->extents.width * 0.1;
				f32 xAdvance = fontInfo->extents.width;

				glyphIndex = mg_font_get_glyph_index_from_font_info(fontInfo, 'x');
				if(glyphIndex)
				{
					mg_glyph_info* glyph = &(fontInfo->glyphs[glyphIndex]);
					boxWidth = glyph->extents.width;
					xBearing = glyph->extents.xBearing;
					xAdvance = glyph->extents.xAdvance;
				}
				f32 oldStrokeWidth = context->attributes.width;

				mg_set_width(contextHandle, boxWidth*0.005);
				mg_rectangle_stroke(contextHandle,
				                    xOffset + xBearing * scale,
				                    yOffset,
				                    boxWidth * scale * flip,
				                    fontInfo->extents.capHeight*scale);

				mg_set_width(contextHandle, oldStrokeWidth);
				mg_move_to(contextHandle, xOffset + xAdvance * scale, yOffset);
				maxWidth = maximum(maxWidth, xOffset + xAdvance*scale - startX);
				continue;
			}
		}

		mg_glyph_info* glyph = mg_font_get_glyph_info(fontInfo, glyphIndex);

		mg_path_push_elements(context, glyph->pathDescriptor.count, fontInfo->outlines + glyph->pathDescriptor.startIndex);

		mg_path_elt* elements = context->pathElements + context->path.count + context->path.startIndex - glyph->pathDescriptor.count;
		for(int eltIndex=0; eltIndex<glyph->pathDescriptor.count; eltIndex++)
		{
			for(int pIndex = 0; pIndex < 3; pIndex++)
			{
				elements[eltIndex].p[pIndex].x = elements[eltIndex].p[pIndex].x * scale + xOffset;
				elements[eltIndex].p[pIndex].y = elements[eltIndex].p[pIndex].y * scale * flip + yOffset;
			}
		}
		mg_move_to(contextHandle, xOffset + scale*glyph->extents.xAdvance, yOffset);

		maxWidth = maximum(maxWidth, xOffset + scale*glyph->extents.xAdvance - startX);
	}
	f32 lineHeight = (fontInfo->extents.ascent + fontInfo->extents.descent)*scale;
	mp_rect box = {startX, startY, maxWidth, context->subPathLastPoint.y - startY + lineHeight };
	return(box);
}

mp_rect mg_glyph_outlines(mg_canvas handle, str32 glyphIndices)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return((mp_rect){});
	}
	mg_font_info* fontInfo = mg_get_font_info(context->attributes.font);
	if(!fontInfo)
	{
		return((mp_rect){});
	}
	return(mg_glyph_outlines_from_font_info(context, fontInfo, glyphIndices));
}

void mg_codepoints_outlines(mg_canvas handle, str32 codePoints)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	mg_font_info* fontInfo = mg_get_font_info(context->attributes.font);
	if(!fontInfo)
	{
		return;
	}

	str32 glyphIndices = mg_font_push_glyph_indices(context->attributes.font, mem_scratch(), codePoints);
	mg_glyph_outlines_from_font_info(context, fontInfo, glyphIndices);
}

void mg_text_outlines(mg_canvas handle, str8 text)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	mg_font_info* fontInfo = mg_get_font_info(context->attributes.font);
	if(!fontInfo)
	{
		return;
	}

	mem_arena* scratch = mem_scratch();
	str32 codePoints = utf8_push_to_codepoints(scratch, text);
	str32 glyphIndices = mg_font_push_glyph_indices(context->attributes.font, scratch, codePoints);

	mg_glyph_outlines_from_font_info(context, fontInfo, glyphIndices);
}

//-----------------------------------------------------------------------------------------------------------
// Path primitives commands
//-----------------------------------------------------------------------------------------------------------

void mg_fill(mg_canvas handle)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	if(context->path.count)
	{
		mg_push_command(context, ((mg_primitive){.cmd = MG_CMD_FILL, .path = context->path, .attributes = context->attributes}));
		mg_new_path(context);
	}
}

void mg_stroke(mg_canvas handle)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	if(context->path.count)
	{
		mg_push_command(context, ((mg_primitive){.cmd = MG_CMD_STROKE, .path = context->path, .attributes = context->attributes}));
		mg_new_path(context);
	}
}

//-----------------------------------------------------------------------------------------------------------
// Fast shapes primitives commands
//-----------------------------------------------------------------------------------------------------------

void mg_rectangle_fill(mg_canvas handle, f32 x, f32 y, f32 w, f32 h)
{
//	DEBUG_ASSERT(w>=0 && h>=0);

	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	mg_push_command(context,
	             ((mg_primitive){.cmd = MG_CMD_RECT_FILL, .rect = (mp_rect){x, y, w, h}, .attributes = context->attributes}));
}
void mg_rectangle_stroke(mg_canvas handle, f32 x, f32 y, f32 w, f32 h)
{
//	DEBUG_ASSERT(w>=0 && h>=0);

	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	mg_push_command(context,
	             ((mg_primitive){.cmd = MG_CMD_RECT_STROKE, .rect = (mp_rect){x, y, w, h}, .attributes = context->attributes}));
}

void mg_rounded_rectangle_fill(mg_canvas handle, f32 x, f32 y, f32 w, f32 h, f32 r)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	mg_push_command(context,
	             ((mg_primitive){.cmd = MG_CMD_ROUND_RECT_FILL,
		                              .roundedRect = (mg_rounded_rect){x, y, w, h, r},
					      .attributes = context->attributes}));
}

void mg_rounded_rectangle_stroke(mg_canvas handle, f32 x, f32 y, f32 w, f32 h, f32 r)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	mg_push_command(context,
	             ((mg_primitive){.cmd = MG_CMD_ROUND_RECT_STROKE,
		                              .roundedRect = (mg_rounded_rect){x, y, w, h, r},
					      .attributes = context->attributes}));
}

void mg_circle_fill(mg_canvas handle, f32 x, f32 y, f32 r)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	mg_push_command(context,
	             ((mg_primitive){.cmd = MG_CMD_ELLIPSE_FILL,
		                              .rect = (mp_rect){x-r, y-r, 2*r, 2*r},
					      .attributes = context->attributes}));
}

void mg_circle_stroke(mg_canvas handle, f32 x, f32 y, f32 r)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	mg_push_command(context,
	             ((mg_primitive){.cmd = MG_CMD_ELLIPSE_STROKE,
		                              .rect = (mp_rect){x-r, y-r, 2*r, 2*r},
					      .attributes = context->attributes}));
}

void mg_ellipse_fill(mg_canvas handle, f32 x, f32 y, f32 rx, f32 ry)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	mg_push_command(context,
	             ((mg_primitive){.cmd = MG_CMD_ELLIPSE_FILL,
		                              .rect = (mp_rect){x-rx, y-ry, 2*rx, 2*ry},
					      .attributes = context->attributes}));
}

void mg_ellipse_stroke(mg_canvas handle, f32 x, f32 y, f32 rx, f32 ry)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	mg_push_command(context,
	             ((mg_primitive){.cmd = MG_CMD_ELLIPSE_STROKE,
		                              .rect = (mp_rect){x-rx, y-ry, 2*rx, 2*ry},
					      .attributes = context->attributes}));
}

void mg_arc(mg_canvas handle, f32 x, f32 y, f32 r, f32 arcAngle, f32 startAngle)
{
	f32 endAngle = startAngle + arcAngle;

	while(startAngle < endAngle)
	{
		f32 smallAngle = minimum(endAngle - startAngle, M_PI/4.);
		if(smallAngle < 0.001)
		{
			break;
		}

		vec2 v0 = {cos(smallAngle/2), sin(smallAngle/2)};
		vec2 v1 = {(4-v0.x)/3, (1-v0.x)*(3-v0.x)/(3*v0.y)};
		vec2 v2 = {v1.x, -v1.y};
		vec2 v3 = {v0.x, -v0.y};

		f32 rotAngle = smallAngle/2 + startAngle;
		f32 rotCos = cos(rotAngle);
		f32 rotSin = sin(rotAngle);

		mg_mat2x3 t = {r*rotCos, -r*rotSin, x,
			       r*rotSin, r*rotCos, y};

		v0 = mg_mat2x3_mul(t, v0);
		v1 = mg_mat2x3_mul(t, v1);
		v2 = mg_mat2x3_mul(t, v2);
		v3 = mg_mat2x3_mul(t, v3);

		mg_move_to(handle, v0.x, v0.y);
		mg_cubic_to(handle, v1.x, v1.y, v2.x, v2.y, v3.x, v3.y);

		startAngle += smallAngle;
	}
}

//------------------------------------------------------------------------------------------
//NOTE(martin): images
//------------------------------------------------------------------------------------------

#define STB_IMAGE_IMPLEMENTATION
#include"stb_image.h"

mg_image mg_image_nil() { return((mg_image){0}); }

bool mg_image_equal(mg_image a, mg_image b)
{
	return(a.h == b.h);
}

mg_image mg_image_handle_from_ptr(mg_canvas_data* canvas, mg_image_data* imageData)
{
	DEBUG_ASSERT(  (imageData - canvas->images) >= 0
	            && (imageData - canvas->images) < MG_MAX_CONTEXTS);

	u64 h = ((u64)(imageData - canvas->images))<<32
	       |((u64)(imageData->generation));
	return((mg_image){.h = h});
}

void mg_image_data_recycle(mg_canvas_data* canvas, mg_image_data* image)
{
	image->generation++;
	ListPush(&canvas->imageFreeList, &image->listElt);
}

mg_image mg_image_create_from_rgba8(mg_canvas handle, u32 width, u32 height, u8* bytes)
{
	mg_image image = mg_image_nil();

	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(context)
	{
		mg_image_data* imageData = ListPopEntry(&context->imageFreeList, mg_image_data, listElt);
		if(!imageData)
		{
			if(context->imageNextIndex < MG_IMAGE_MAX_COUNT)
			{
				imageData = &context->images[context->imageNextIndex];
				imageData->generation = 1;
				context->imageNextIndex++;
			}
			else
			{
				LOG_ERROR("image pool full\n");
			}
		}

		if(imageData)
		{
			if(context->atlasPos.x + width >= MG_ATLAS_SIZE)
			{
				context->atlasPos.x = 0;
				context->atlasPos.y += context->atlasLineHeight;
			}
			if(context->atlasPos.x + width < MG_ATLAS_SIZE
			  && context->atlasPos.y + height < MG_ATLAS_SIZE)
			{
				imageData->rect = (mp_rect){context->atlasPos.x,
				                            context->atlasPos.y,
				                            width,
				                            height};

				context->atlasPos.x += width;
				context->atlasLineHeight = maximum(context->atlasLineHeight, height);

				context->painter->atlasUpload(context->painter, imageData->rect, bytes);
				image = mg_image_handle_from_ptr(context, imageData);
			}
			else
			{
				mg_image_data_recycle(context, imageData);
			}
		}
	}
	return(image);
}

mg_image mg_image_create_from_data(mg_canvas canvas, str8 data, bool flip)
{
	mg_image image = mg_image_nil();
	int width, height, channels;

	stbi_set_flip_vertically_on_load(flip ? 1 : 0);
	u8* pixels = stbi_load_from_memory((u8*)data.ptr, data.len, &width, &height, &channels, 4);
	if(pixels)
	{
		image = mg_image_create_from_rgba8(canvas, width, height, pixels);
		free(pixels);
	}
	return(image);
}

mg_image mg_image_create_from_file(mg_canvas canvas, str8 path, bool flip)
{
	mg_image image = mg_image_nil();
	int width, height, channels;

	const char* cpath = str8_to_cstring(mem_scratch(), path);

	stbi_set_flip_vertically_on_load(flip ? 1 : 0);
	u8* pixels = stbi_load(cpath, &width, &height, &channels, 4);
	if(pixels)
	{
		image = mg_image_create_from_rgba8(canvas, width, height, pixels);
		free(pixels);
	}
	return(image);
}

void mg_image_destroy(mg_canvas handle, mg_image image)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	//TODO invalidate image handle, maybe free atlas area
}

vec2 mg_image_size(mg_canvas handle, mg_image image)
{
	vec2 size = {0};
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(context)
	{
		mg_image_data* imageData = mg_image_ptr_from_handle(context, image);
		if(imageData)
		{
			size = (vec2){imageData->rect.w, imageData->rect.h};
		}
	}
	return(size);
}

void mg_image_draw(mg_canvas handle, mg_image image, mp_rect rect)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	mg_primitive primitive = {.cmd = MG_CMD_IMAGE_DRAW,
	                          .rect = (mp_rect){rect.x, rect.y, rect.w, rect.h},
	                          .attributes = context->attributes};
	primitive.attributes.image = image;

	mg_push_command(context, primitive);
}

void mg_rounded_image_draw(mg_canvas handle, mg_image image, mp_rect rect, f32 roundness)
{
	mg_canvas_data* context = mg_canvas_ptr_from_handle(handle);
	if(!context)
	{
		return;
	}
	mg_primitive primitive = {.cmd = MG_CMD_ROUNDED_IMAGE_DRAW,
	                          .roundedRect = {rect.x, rect.y, rect.w, rect.h, roundness},
	                          .attributes = context->attributes};
	primitive.attributes.image = image;

	mg_push_command(context, primitive);
}


#undef LOG_SUBSYSTEM
