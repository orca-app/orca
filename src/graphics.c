/************************************************************//**
*
*	@file: graphics.c
*	@author: Martin Fouilleul
*	@date: 23/01/2023
*	@revision:
*
*****************************************************************/

#define _USE_MATH_DEFINES //NOTE: necessary for MSVC
#include<math.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include"stb_truetype.h"

#define STB_IMAGE_IMPLEMENTATION
#include"stb_image.h"

#include"debug_log.h"
#include"graphics_internal.h"

#define LOG_SUBSYSTEM "Graphics"

//------------------------------------------------------------------------
// canvas structs
//------------------------------------------------------------------------
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
	mp_rect srcRegion;

	mg_mat2x3 transform;
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

typedef enum { MG_CMD_FILL,
	           MG_CMD_STROKE,
	           MG_CMD_RECT_FILL,
	           MG_CMD_RECT_STROKE,
	           MG_CMD_ROUND_RECT_FILL,
	           MG_CMD_ROUND_RECT_STROKE,
	           MG_CMD_ELLIPSE_FILL,
	           MG_CMD_ELLIPSE_STROKE,
	           MG_CMD_JUMP,
	           MG_CMD_CLIP_PUSH,
	           MG_CMD_CLIP_POP,
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
	};

} mg_primitive;

typedef struct mg_glyph_map_entry
{
	unicode_range range;
	u32 firstGlyphIndex;

} mg_glyph_map_entry;

typedef struct mg_glyph_data
{
	bool exists;
	utf32 codePoint;
	mg_path_descriptor pathDescriptor;
	mg_text_extents extents;
	//...

} mg_glyph_data;

enum
{
	MG_MATRIX_STACK_MAX_DEPTH = 64,
	MG_CLIP_STACK_MAX_DEPTH = 64,
	MG_MAX_PATH_ELEMENT_COUNT = 2<<20,
	MG_MAX_PRIMITIVE_COUNT = 8<<10
};

typedef struct mg_font_data
{
	list_elt freeListElt;

	u32 rangeCount;
	u32 glyphCount;
	u32 outlineCount;
	mg_glyph_map_entry* glyphMap;
	mg_glyph_data*      glyphs;
	mg_path_elt* outlines;

	f32 unitsPerEm;
	mg_font_extents extents;

} mg_font_data;

typedef struct mg_canvas_data mg_canvas_data;
typedef struct mg_image_data mg_image_data;

typedef enum mg_handle_kind
{
	MG_HANDLE_NONE = 0,
	MG_HANDLE_SURFACE,
	MG_HANDLE_CANVAS,
	MG_HANDLE_FONT,
	MG_HANDLE_IMAGE,
	MG_HANDLE_SURFACE_SERVER,
} mg_handle_kind;

typedef struct mg_handle_slot
{
	list_elt freeListElt;
	u32 generation;
	mg_handle_kind kind;

	void* data;

} mg_handle_slot;

typedef struct mg_data
{
	bool init;

	mem_arena handleArena;
	list_info handleFreeList;

	mem_arena resourceArena;
	list_info canvasFreeList;
	list_info fontFreeList;

} mg_data;

typedef struct mg_canvas_data
{
	list_elt freeListElt;

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

	u32 primitiveCount;
	mg_primitive primitives[MG_MAX_PRIMITIVE_COUNT];

	//NOTE: these are used at render time
	mg_color clearColor;
	mp_rect clip;
	mg_mat2x3 transform;
	mg_image image;
	mp_rect srcRegion;

	vec4 shapeExtents;
	u32 nextShapeIndex;
	u32 vertexCount;
	u32 indexCount;

	mg_surface surface;
	mg_canvas_backend* backend;

} mg_canvas_data;

static mg_data __mgData = {0};


void mg_init()
{
	if(!__mgData.init)
	{
		mem_arena_init(&__mgData.handleArena);
		mem_arena_init(&__mgData.resourceArena);
		__mgData.init = true;
	}
}

//------------------------------------------------------------------------
// handle pools procedures
//------------------------------------------------------------------------

u64 mg_handle_alloc(mg_handle_kind kind, void* data)
{
	if(!__mgData.init)
	{
		mg_init();
	}
	mem_arena* arena = &__mgData.handleArena;

	mg_handle_slot* slot = list_pop_entry(&__mgData.handleFreeList, mg_handle_slot, freeListElt);
	if(!slot)
	{
		slot = mem_arena_alloc_type(arena, mg_handle_slot);
		DEBUG_ASSERT(slot);
		slot->generation = 1;
	}
	slot->kind = kind;
	slot->data = data;

	u64 h = ((u64)(slot - (mg_handle_slot*)arena->ptr))<<32
	       |((u64)(slot->generation));

	return(h);
}

void mg_handle_recycle(u64 h)
{
	DEBUG_ASSERT(__mgData.init);

	u32 index = h>>32;
	u32 generation = h & 0xffffffff;
	mem_arena* arena = &__mgData.handleArena;

	if(index*sizeof(mg_handle_slot) < arena->offset)
	{
		mg_handle_slot* slot = (mg_handle_slot*)arena->ptr + index;
		if(slot->generation == generation)
		{
			DEBUG_ASSERT(slot->generation != UINT32_MAX, "surface slot generation wrap around\n");
			slot->generation++;
			list_push(&__mgData.handleFreeList, &slot->freeListElt);
		}
	}
}

void* mg_data_from_handle(mg_handle_kind kind, u64 h)
{
	DEBUG_ASSERT(__mgData.init);

	void* data = 0;

	u32 index = h>>32;
	u32 generation = h & 0xffffffff;
	mem_arena* arena = &__mgData.handleArena;

	if(index*sizeof(mg_handle_slot) < arena->offset)
	{
		mg_handle_slot* slot = (mg_handle_slot*)arena->ptr + index;
		if(  slot->generation == generation
		  && slot->kind == kind)
		{
			data = slot->data;
		}
	}
	return(data);
}

//---------------------------------------------------------------
// typed handles functions
//---------------------------------------------------------------
mg_surface mg_surface_handle_alloc(mg_surface_data* surface)
{
	mg_surface handle = {.h = mg_handle_alloc(MG_HANDLE_SURFACE, (void*)surface) };
	return(handle);
}

mg_surface_data* mg_surface_data_from_handle(mg_surface handle)
{
	mg_surface_data* data = mg_data_from_handle(MG_HANDLE_SURFACE, handle.h);
	return(data);
}

mg_canvas mg_canvas_handle_alloc(mg_canvas_data* canvas)
{
	mg_canvas handle = {.h = mg_handle_alloc(MG_HANDLE_CANVAS, (void*)canvas) };
	return(handle);
}

mg_canvas_data* mg_canvas_data_from_handle(mg_canvas handle)
{
	mg_canvas_data* data = mg_data_from_handle(MG_HANDLE_CANVAS, handle.h);
	return(data);
}

mg_font mg_font_handle_alloc(mg_font_data* font)
{
	mg_font handle = {.h = mg_handle_alloc(MG_HANDLE_FONT, (void*)font) };
	return(handle);
}

mg_font_data* mg_font_data_from_handle(mg_font handle)
{
	mg_font_data* data = mg_data_from_handle(MG_HANDLE_FONT, handle.h);
	return(data);
}

mg_image mg_image_handle_alloc(mg_image_data* image)
{
	mg_image handle = {.h = mg_handle_alloc(MG_HANDLE_IMAGE, (void*)image) };
	return(handle);
}

mg_image_data* mg_image_data_from_handle(mg_image handle)
{
	mg_image_data* data = mg_data_from_handle(MG_HANDLE_IMAGE, handle.h);
	return(data);
}

//---------------------------------------------------------------
// surface API
//---------------------------------------------------------------

#if MG_COMPILE_BACKEND_GL
	#if defined(OS_WIN64)
		#include"wgl_surface.h"
		#define gl_surface_create_for_window mg_wgl_surface_create_for_window
	#elif defined(OS_MACOS)
/*
		#include"nsgl_surface.h"
		#define gl_surface_create_for_window nsgl_surface_create_for_window
*/
	#endif
#endif

#if MG_COMPILE_BACKEND_GLES
	#include"egl_surface.h"
#endif

#if MG_COMPILE_BACKEND_METAL
	#include"mtl_surface.h"
#endif

bool mg_is_surface_backend_available(mg_backend_id backend)
{
	bool result = false;
	switch(backend)
	{
		#if MG_COMPILE_BACKEND_METAL
			case MG_BACKEND_METAL:
		#endif
		#if MG_COMPILE_BACKEND_GL
			case MG_BACKEND_GL:
		#endif
		#if MG_COMPILE_BACKEND_GLES
			case MG_BACKEND_GLES:
		#endif
			result = true;
			break;

		default:
			break;
	}
	return(result);
}

bool mg_is_canvas_backend_available(mg_backend_id backend)
{
	bool result = false;
	switch(backend)
	{
		#if MG_COMPILE_BACKEND_METAL
			case MG_BACKEND_METAL:
		#endif
		#if MG_COMPILE_BACKEND_GL && defined(OS_WIN64)
			case MG_BACKEND_GL:
		#endif
			result = true;
			break;

		default:
			break;
	}
	return(result);
}

mg_surface mg_surface_nil() { return((mg_surface){.h = 0}); }
bool mg_surface_is_nil(mg_surface surface) { return(surface.h == 0); }

mg_surface mg_surface_create_for_window(mp_window window, mg_backend_id backend)
{
	if(__mgData.init)
	{
		mg_init();
	}
	mg_surface surfaceHandle = mg_surface_nil();
	mg_surface_data* surface = 0;

	switch(backend)
	{
	#if MG_COMPILE_BACKEND_GL
		case MG_BACKEND_GL:
			surface = gl_surface_create_for_window(window);
			break;
	#endif

	#if MG_COMPILE_BACKEND_GLES
		case MG_BACKEND_GLES:
			surface = mg_egl_surface_create_for_window(window);
			break;
	#endif

	#if MG_COMPILE_BACKEND_METAL
		case MG_BACKEND_METAL:
			surface = mg_mtl_surface_create_for_window(window);
			break;
	#endif

		default:
			break;
	}
	if(surface)
	{
		surfaceHandle = mg_surface_handle_alloc(surface);
	}
	return(surfaceHandle);
}

mg_surface mg_surface_create_remote(u32 width, u32 height, mg_backend_id backend)
{
	if(__mgData.init)
	{
		mg_init();
	}
	mg_surface surfaceHandle = mg_surface_nil();
	mg_surface_data* surface = 0;

	switch(backend)
	{
	#if MG_COMPILE_BACKEND_GLES
		case MG_BACKEND_GLES:
			surface = mg_egl_surface_create_remote(width, height);
			break;
	#endif

		default:
			break;
	}
	if(surface)
	{
		surfaceHandle = mg_surface_handle_alloc(surface);
	}
	return(surfaceHandle);
}

mg_surface mg_surface_create_host(mp_window window)
{
	if(__mgData.init)
	{
		mg_init();
	}
	mg_surface handle = mg_surface_nil();
	mg_surface_data* surface = 0;
	#if OS_MACOS
		surface = mg_osx_surface_create_host(window);
	#elif OS_WIN64
		surface = mg_win32_surface_create_host(window);
	#endif

	if(surface)
	{
		handle = mg_surface_handle_alloc(surface);
	}
	return(handle);
}

void mg_surface_destroy(mg_surface handle)
{
	DEBUG_ASSERT(__mgData.init);
	mg_surface_data* surface = mg_surface_data_from_handle(handle);
	if(surface)
	{
		surface->destroy(surface);
		mg_handle_recycle(handle.h);
	}
}

void mg_surface_prepare(mg_surface surface)
{
	DEBUG_ASSERT(__mgData.init);
	mg_surface_data* surfaceData = mg_surface_data_from_handle(surface);
	if(surfaceData && surfaceData->prepare)
	{
		surfaceData->prepare(surfaceData);
	}
}

void mg_surface_present(mg_surface surface)
{
	DEBUG_ASSERT(__mgData.init);
	mg_surface_data* surfaceData = mg_surface_data_from_handle(surface);
	if(surfaceData && surfaceData->present)
	{
		surfaceData->present(surfaceData);
	}
}

void mg_surface_swap_interval(mg_surface surface, int swap)
{
	DEBUG_ASSERT(__mgData.init);
	mg_surface_data* surfaceData = mg_surface_data_from_handle(surface);
	if(surfaceData && surfaceData->swapInterval)
	{
		surfaceData->swapInterval(surfaceData, swap);
	}
}

vec2 mg_surface_contents_scaling(mg_surface surface)
{
	DEBUG_ASSERT(__mgData.init);
	vec2 scaling = {1, 1};
	mg_surface_data* surfaceData = mg_surface_data_from_handle(surface);
	if(surfaceData && surfaceData->contentsScaling)
	{
		scaling = surfaceData->contentsScaling(surfaceData);
	}
	return(scaling);
}


void mg_surface_set_frame(mg_surface surface, mp_rect frame)
{
	DEBUG_ASSERT(__mgData.init);
	mg_surface_data* surfaceData = mg_surface_data_from_handle(surface);
	if(surfaceData && surfaceData->setFrame)
	{
		surfaceData->setFrame(surfaceData, frame);
	}
}

mp_rect mg_surface_get_frame(mg_surface surface)
{
	DEBUG_ASSERT(__mgData.init);
	mp_rect res = {0};
	mg_surface_data* surfaceData = mg_surface_data_from_handle(surface);
	if(surfaceData && surfaceData->getFrame)
	{
		res = surfaceData->getFrame(surfaceData);
	}
	return(res);
}

void mg_surface_set_hidden(mg_surface surface, bool hidden)
{
	DEBUG_ASSERT(__mgData.init);
	mg_surface_data* surfaceData = mg_surface_data_from_handle(surface);
	if(surfaceData && surfaceData->setHidden)
	{
		surfaceData->setHidden(surfaceData, hidden);
	}
}

bool mg_surface_get_hidden(mg_surface surface)
{
	DEBUG_ASSERT(__mgData.init);
	bool res = false;
	mg_surface_data* surfaceData = mg_surface_data_from_handle(surface);
	if(surfaceData && surfaceData->getHidden)
	{
		res = surfaceData->getHidden(surfaceData);
	}
	return(res);
}

void* mg_surface_native_layer(mg_surface surface)
{
	void* res = 0;
	mg_surface_data* surfaceData = mg_surface_data_from_handle(surface);
	if(surfaceData && surfaceData->nativeLayer)
	{
		res = surfaceData->nativeLayer(surfaceData);
	}
	return(res);
}

mg_surface_id mg_surface_remote_id(mg_surface handle)
{
	mg_surface_id remoteId = 0;
	mg_surface_data* surface = mg_surface_data_from_handle(handle);
	if(surface && surface->remoteID)
	{
		remoteId = surface->remoteID(surface);
	}
	return(remoteId);
}

void mg_surface_host_connect(mg_surface handle, mg_surface_id remoteID)
{
	mg_surface_data* surface = mg_surface_data_from_handle(handle);
	if(surface && surface->hostConnect)
	{
		surface->hostConnect(surface, remoteID);
	}
}

//------------------------------------------------------------------------------------------
//NOTE(martin): graphics canvas internal
//------------------------------------------------------------------------------------------

mp_thread_local mg_canvas_data* __mgCurrentCanvas = 0;
mp_thread_local mg_canvas __mgCurrentCanvasHandle = {0};

//TODO: move elsewhere?
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

mg_mat2x3 mg_mat2x3_inv(mg_mat2x3 x)
{
	mg_mat2x3 res;
	res.m[0] = x.m[4]/(x.m[0]*x.m[4] - x.m[1]*x.m[3]);
	res.m[1] = x.m[1]/(x.m[1]*x.m[3] - x.m[0]*x.m[4]);
	res.m[3] = x.m[3]/(x.m[1]*x.m[3] - x.m[0]*x.m[4]);
	res.m[4] = x.m[0]/(x.m[0]*x.m[4] - x.m[1]*x.m[3]);
	res.m[2] = -(x.m[2]*res.m[0] + x.m[5]*res.m[1]);
	res.m[5] = -(x.m[2]*res.m[3] + x.m[5]*res.m[4]);
	return(res);
}

vec2 mg_mat2x3_mul(mg_mat2x3 m, vec2 p)
{
	f32 x = p.x*m.m[0] + p.y*m.m[1] + m.m[2];
	f32 y = p.x*m.m[3] + p.y*m.m[4] + m.m[5];
	return((vec2){x, y});
}

mg_mat2x3 mg_matrix_stack_top(mg_canvas_data* canvas)
{
	if(canvas->matrixStackSize == 0)
	{
		return((mg_mat2x3){1, 0, 0,
				   0, 1, 0});
	}
	else
	{
		return(canvas->matrixStack[canvas->matrixStackSize-1]);
	}
}

void mg_matrix_stack_push(mg_canvas_data* canvas, mg_mat2x3 transform)
{
	if(canvas->matrixStackSize >= MG_MATRIX_STACK_MAX_DEPTH)
	{
		LOG_ERROR("matrix stack overflow\n");
	}
	else
	{
		canvas->matrixStack[canvas->matrixStackSize] = transform;
		canvas->matrixStackSize++;
	}
}

void mg_matrix_stack_pop(mg_canvas_data* canvas)
{
	if(canvas->matrixStackSize == 0)
	{
		LOG_ERROR("matrix stack underflow\n");
	}
	else
	{
		canvas->matrixStackSize--;
		mg_matrix_stack_top(canvas);
	}
}

void mg_push_command(mg_canvas_data* canvas, mg_primitive primitive)
{
	//NOTE(martin): push primitive and updates current stream, eventually patching a pending jump.
	ASSERT(canvas->primitiveCount < MG_MAX_PRIMITIVE_COUNT);
	canvas->primitives[canvas->primitiveCount] = primitive;
	canvas->primitives[canvas->primitiveCount].attributes = canvas->attributes;
	canvas->primitives[canvas->primitiveCount].attributes.transform = mg_matrix_stack_top(canvas);
	canvas->primitiveCount++;
}

void mg_new_path(mg_canvas_data* canvas)
{
	canvas->path.startIndex += canvas->path.count;
	canvas->path.count = 0;
	canvas->subPathStartPoint = canvas->subPathLastPoint;
	canvas->path.startPoint = canvas->subPathStartPoint;
}

void mg_path_push_elements(mg_canvas_data* canvas, u32 count, mg_path_elt* elements)
{
	ASSERT(canvas->path.count + canvas->path.startIndex + count <= MG_MAX_PATH_ELEMENT_COUNT);
	memcpy(canvas->pathElements + canvas->path.startIndex + canvas->path.count, elements, count*sizeof(mg_path_elt));
	canvas->path.count += count;
}

void mg_path_push_element(mg_canvas_data* canvas, mg_path_elt elt)
{
	mg_path_push_elements(canvas, 1, &elt);
}

///////////////////////////////////////  WIP  /////////////////////////////////////////////////////////////////////////

void mg_reset_shape_index(mg_canvas_data* canvas)
{
	canvas->nextShapeIndex = 0;
	canvas->shapeExtents = (vec4){FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX};
}

void mg_finalize_shape(mg_canvas_data* canvas)
{
	if(canvas->nextShapeIndex)
	{
		//NOTE: set shape's uv transform for the _current_ shape
		vec2 texSize = mg_image_size(canvas->image);

		mp_rect srcRegion = canvas->srcRegion;

		mp_rect destRegion = {canvas->shapeExtents.x,
		                      canvas->shapeExtents.y,
		                      canvas->shapeExtents.z - canvas->shapeExtents.x,
		                      canvas->shapeExtents.w - canvas->shapeExtents.y};

		mg_mat2x3 srcRegionToImage = {1/texSize.x, 0,           srcRegion.x/texSize.x,
		                                0,           1/texSize.y, srcRegion.y/texSize.y};
		mg_mat2x3 destRegionToSrcRegion = {srcRegion.w/destRegion.w, 0,                        0,
		                                   0,                        srcRegion.h/destRegion.h, 0};
		mg_mat2x3 userToDestRegion = {1, 0, -destRegion.x,
		                              0, 1, -destRegion.y};

		mg_mat2x3 screenToUser = mg_mat2x3_inv(canvas->transform);

		mg_mat2x3 uvTransform = srcRegionToImage;
		uvTransform = mg_mat2x3_mul_m(uvTransform, destRegionToSrcRegion);
		uvTransform = mg_mat2x3_mul_m(uvTransform, userToDestRegion);
		uvTransform = mg_mat2x3_mul_m(uvTransform, screenToUser);

		int index = canvas->nextShapeIndex-1;
		mg_vertex_layout* layout = &canvas->backend->vertexLayout;
		*(mg_mat2x3*)(layout->uvTransformBuffer + index*layout->uvTransformStride) = uvTransform;

	}
}

u32 mg_next_shape(mg_canvas_data* canvas, mg_attributes* attributes)
{
	mg_finalize_shape(canvas);

	canvas->transform = attributes->transform;
	canvas->srcRegion = attributes->srcRegion;
	canvas->shapeExtents = (vec4){FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX};

	mg_vertex_layout* layout = &canvas->backend->vertexLayout;
	int index = canvas->nextShapeIndex;
	canvas->nextShapeIndex++;

	mp_rect clip = {canvas->clip.x,
	                canvas->clip.y,
	                canvas->clip.x + canvas->clip.w,
	                canvas->clip.y + canvas->clip.h};

	*(mp_rect*)(((char*)layout->clipBuffer) + index*layout->clipStride) = clip;
	*(mg_color*)(((char*)layout->colorBuffer) + index*layout->colorStride) = attributes->color;

	return(index);
}

//TODO(martin): rename with something more explicit
u32 mg_vertices_base_index(mg_canvas_data* canvas)
{
	return(canvas->vertexCount);
}

int* mg_reserve_indices(mg_canvas_data* canvas, u32 indexCount)
{
	mg_vertex_layout* layout = &canvas->backend->vertexLayout;

	//TODO: do something here...
	ASSERT(canvas->indexCount + indexCount < layout->maxIndexCount);

	int* base = ((int*)layout->indexBuffer) + canvas->indexCount;
	canvas->indexCount += indexCount;
	return(base);
}

void mg_push_vertex_cubic(mg_canvas_data* canvas, vec2 pos, vec4 cubic)
{
	canvas->shapeExtents.x = minimum(canvas->shapeExtents.x, pos.x);
	canvas->shapeExtents.y = minimum(canvas->shapeExtents.y, pos.y);
	canvas->shapeExtents.z = maximum(canvas->shapeExtents.z, pos.x);
	canvas->shapeExtents.w = maximum(canvas->shapeExtents.w, pos.y);

	vec2 screenPos = mg_mat2x3_mul(canvas->transform, pos);

	mg_vertex_layout* layout = &canvas->backend->vertexLayout;
	DEBUG_ASSERT(canvas->vertexCount < layout->maxVertexCount);
	DEBUG_ASSERT(canvas->nextShapeIndex > 0);

	int shapeIndex = maximum(0, canvas->nextShapeIndex-1);
	u32 index = canvas->vertexCount;
	canvas->vertexCount++;

	*(vec2*)(((char*)layout->posBuffer) + index*layout->posStride) = screenPos;
	*(vec4*)(((char*)layout->cubicBuffer) + index*layout->cubicStride) = cubic;
	*(u32*)(((char*)layout->shapeIndexBuffer) + index*layout->shapeIndexStride) = shapeIndex;
}

void mg_push_vertex(mg_canvas_data* canvas, vec2 pos)
{
	mg_push_vertex_cubic(canvas, pos, (vec4){1, 1, 1, 1});
}
//-----------------------------------------------------------------------------------------------------------
// Path Filling
//-----------------------------------------------------------------------------------------------------------
//NOTE(martin): forward declarations
void mg_render_fill_cubic(mg_canvas_data* canvas, vec2 p[4]);

//NOTE(martin): quadratics filling

void mg_render_fill_quadratic(mg_canvas_data* canvas, vec2 p[3])
{
	u32 baseIndex = mg_vertices_base_index(canvas);

	i32* indices = mg_reserve_indices(canvas, 3);

	mg_push_vertex_cubic(canvas, (vec2){p[0].x, p[0].y}, (vec4){0, 0, 0, 1});
	mg_push_vertex_cubic(canvas, (vec2){p[1].x, p[1].y}, (vec4){0.5, 0, 0.5, 1});
	mg_push_vertex_cubic(canvas, (vec2){p[2].x, p[2].y}, (vec4){1, 1, 1, 1});

	indices[0] = baseIndex + 0;
	indices[1] = baseIndex + 1;
	indices[2] = baseIndex + 2;
}

//NOTE(martin): cubic filling

void mg_split_and_fill_cubic(mg_canvas_data* canvas, vec2 p[4], f32 tSplit)
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
	u32 baseIndex = mg_vertices_base_index(canvas);
	i32* indices = mg_reserve_indices(canvas, 3);

	mg_push_vertex(canvas, (vec2){p[0].x, p[0].y});
	mg_push_vertex(canvas, (vec2){split.x, split.y});
	mg_push_vertex(canvas, (vec2){p[3].x, p[3].y});

	indices[0] = baseIndex + 0;
	indices[1] = baseIndex + 1;
	indices[2] = baseIndex + 2;

	mg_render_fill_cubic(canvas, subPointsLow);
	mg_render_fill_cubic(canvas, subPointsHigh);
}

void mg_render_fill_cubic(mg_canvas_data* canvas, vec2 p[4])
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

		mg_render_fill_quadratic(canvas, quadControlPoints);
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
			which are assigned as a 4D image coordinates to control points.


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
			mg_split_and_fill_cubic(canvas, p, td/sd);
			return;
		}
		if(se != 0 && te/se < 0.99 && te/se > 0.01)
		{
			LOG_DEBUG("split curve at second double point\n");
			mg_split_and_fill_cubic(canvas, p, te/se);
			return;
		}

		/*NOTE(martin):
			the power basis coefficients of points k,l,m,n are collected into the rows of the 4x4 matrix F:

				| td*te            td^2*te                 td*te^2                1 |
				| -se*td - sd*te   -se*td^2 - 2sd*te*td    -sd*te^2 - 2*se*td*te  0 |
				| sd*se            te*sd^2 + 2*se*td*sd    td*se^2 + 2*sd*te*se   0 |
				| 0                -sd^2*se                -sd*se^2               0 |

			This matrix is then multiplied by M3^(-1) on the left which yelds the bezier coefficients of k, l, m, n
			which are assigned as a 4D image coordinates to control points.


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
			which are assigned as a 4D image coordinates to control points.


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

		u32 baseIndex = mg_vertices_base_index(canvas);
		i32* indices = mg_reserve_indices(canvas, 3);

		for(int i=0; i<3; i++)
		{
			vec4 cubic = testCoords[orderedHullIndices[i]];
			cubic.w = outsideTest;
			mg_push_vertex_cubic(canvas, p[orderedHullIndices[i]], cubic);
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

		u32 baseIndex = mg_vertices_base_index(canvas);
		i32* indices = mg_reserve_indices(canvas, 6);

		mg_push_vertex_cubic(canvas,
		               p[orderedHullIndices[0]],
		               (vec4){vec4_expand_xyz(testCoords[orderedHullIndices[0]]), outsideTest1});

		mg_push_vertex_cubic(canvas,
		               p[orderedHullIndices[1]],
		               (vec4){vec4_expand_xyz(testCoords[orderedHullIndices[1]]), outsideTest1});

		mg_push_vertex_cubic(canvas,
		               p[orderedHullIndices[2]],
		               (vec4){vec4_expand_xyz(testCoords[orderedHullIndices[2]]), outsideTest1});

		mg_push_vertex_cubic(canvas,
		               p[orderedHullIndices[0]],
		               (vec4){vec4_expand_xyz(testCoords[orderedHullIndices[0]]), outsideTest2});

		mg_push_vertex_cubic(canvas,
		               p[orderedHullIndices[2]],
		               (vec4){vec4_expand_xyz(testCoords[orderedHullIndices[2]]), outsideTest2});

		mg_push_vertex_cubic(canvas,
		               p[orderedHullIndices[3]],
		               (vec4){vec4_expand_xyz(testCoords[orderedHullIndices[3]]), outsideTest2});

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

		u32 baseIndex = mg_vertices_base_index(canvas);
		i32* indices = mg_reserve_indices(canvas, 6);

		for(int i=0; i<4; i++)
		{
			mg_push_vertex_cubic(canvas,
		                   p[orderedHullIndices[i]],
		                   (vec4){vec4_expand_xyz(testCoords[orderedHullIndices[i]]), outsideTest});
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

void mg_render_fill(mg_canvas_data* canvas, mg_path_elt* elements, mg_path_descriptor* path)
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
				mg_render_fill_quadratic(canvas, controlPoints);
				endPoint = controlPoints[2];

			} break;

			case MG_PATH_CUBIC:
			{
				mg_render_fill_cubic(canvas, controlPoints);
				endPoint = controlPoints[3];
			} break;
		}

		//NOTE(martin): now fill interior triangle
		u32 baseIndex = mg_vertices_base_index(canvas);
		int* indices = mg_reserve_indices(canvas, 3);

		mg_push_vertex(canvas, startPoint);
		mg_push_vertex(canvas, currentPoint);
		mg_push_vertex(canvas, endPoint);

		indices[0] = baseIndex;
		indices[1] = baseIndex + 1;
		indices[2] = baseIndex + 2;

		currentPoint = endPoint;
	}
}

//-----------------------------------------------------------------------------------------------------------
// Path Stroking
//-----------------------------------------------------------------------------------------------------------

void mg_render_stroke_line(mg_canvas_data* canvas, vec2 p[2], mg_attributes* attributes)
{
	//NOTE(martin): get normals multiplied by halfWidth
	f32 halfW = attributes->width/2;

	vec2 n0 = {p[0].y - p[1].y,
		   p[1].x - p[0].x};
	f32 norm0 = sqrt(n0.x*n0.x + n0.y*n0.y);
	n0.x *= halfW/norm0;
	n0.y *= halfW/norm0;

	u32 baseIndex = mg_vertices_base_index(canvas);
	i32* indices = mg_reserve_indices(canvas, 6);

	mg_push_vertex(canvas, (vec2){p[0].x + n0.x, p[0].y + n0.y});
	mg_push_vertex(canvas, (vec2){p[1].x + n0.x, p[1].y + n0.y});
	mg_push_vertex(canvas, (vec2){p[1].x - n0.x, p[1].y - n0.y});
	mg_push_vertex(canvas, (vec2){p[0].x - n0.x, p[0].y - n0.y});

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
	if(count == 4 && (fabs(p[1].x - p[2].x) < 0.01) && (fabs(p[1].y - p[2].y) < 0.01))
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


void mg_render_stroke_quadratic(mg_canvas_data* canvas, vec2 p[4], mg_attributes* attributes)
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

	//TODO: maybe refactor by using tolerance in the _check_, not in the computation of the overshoot
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
		vec2 splitLeft[3];
		vec2 splitRight[3];
		mg_quadratic_split(p, maxOvershootParameter, splitLeft, splitRight);
		mg_render_stroke_quadratic(canvas, splitLeft, attributes);
		mg_render_stroke_quadratic(canvas, splitRight, attributes);
	}
	else
	{
		//NOTE(martin): push the actual fill commands for the offset contour

		mg_next_shape(canvas, attributes);

		mg_render_fill_quadratic(canvas, positiveOffsetHull);
		mg_render_fill_quadratic(canvas, negativeOffsetHull);

		//NOTE(martin):	add base triangles
		u32 baseIndex = mg_vertices_base_index(canvas);
		i32* indices = mg_reserve_indices(canvas, 6);

		mg_push_vertex(canvas, positiveOffsetHull[0]);
		mg_push_vertex(canvas, positiveOffsetHull[2]);
		mg_push_vertex(canvas, negativeOffsetHull[2]);
		mg_push_vertex(canvas, negativeOffsetHull[0]);

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

void mg_render_stroke_cubic(mg_canvas_data* canvas, vec2 p[4], mg_attributes* attributes)
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

	//TODO: maybe refactor by using tolerance in the _check_, not in the computation of the overshoot
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
		vec2 splitLeft[4];
		vec2 splitRight[4];
		mg_cubic_split(p, maxOvershootParameter, splitLeft, splitRight);
		mg_render_stroke_cubic(canvas, splitLeft, attributes);
		mg_render_stroke_cubic(canvas, splitRight, attributes);

		//TODO: render joint between the split curves
	}
	else
	{
		//NOTE(martin): push the actual fill commands for the offset contour
		mg_next_shape(canvas, attributes);

		mg_render_fill_cubic(canvas, positiveOffsetHull);
		mg_render_fill_cubic(canvas, negativeOffsetHull);

		//NOTE(martin):	add base triangles
		u32 baseIndex = mg_vertices_base_index(canvas);
		i32* indices = mg_reserve_indices(canvas, 6);

		mg_push_vertex(canvas, positiveOffsetHull[0]);
		mg_push_vertex(canvas, positiveOffsetHull[3]);
		mg_push_vertex(canvas, negativeOffsetHull[3]);
		mg_push_vertex(canvas, negativeOffsetHull[0]);

		indices[0] = baseIndex + 0;
		indices[1] = baseIndex + 1;
		indices[2] = baseIndex + 2;
		indices[3] = baseIndex + 0;
		indices[4] = baseIndex + 2;
		indices[5] = baseIndex + 3;
	}
	#undef CHECK_SAMPLE_COUNT
}

void mg_stroke_cap(mg_canvas_data* canvas, vec2 p0, vec2 direction, mg_attributes* attributes)
{
	//NOTE(martin): compute the tangent and normal vectors (multiplied by half width) at the cap point

	f32 dn = sqrt(Square(direction.x) + Square(direction.y));
	f32 alpha = 0.5 * attributes->width/dn;

	vec2 n0 = {-alpha*direction.y,
		    alpha*direction.x};

	vec2 m0 = {alpha*direction.x,
	           alpha*direction.y};

	mg_next_shape(canvas, attributes);

	u32 baseIndex = mg_vertices_base_index(canvas);
	i32* indices = mg_reserve_indices(canvas, 6);

	mg_push_vertex(canvas, (vec2){p0.x + n0.x, p0.y + n0.y});
	mg_push_vertex(canvas, (vec2){p0.x + n0.x + m0.x, p0.y + n0.y + m0.y});
	mg_push_vertex(canvas, (vec2){p0.x - n0.x + m0.x, p0.y - n0.y + m0.y});
	mg_push_vertex(canvas, (vec2){p0.x - n0.x, p0.y - n0.y});

	indices[0] = baseIndex;
	indices[1] = baseIndex + 1;
	indices[2] = baseIndex + 2;
	indices[3] = baseIndex;
	indices[4] = baseIndex + 2;
	indices[5] = baseIndex + 3;
}

void mg_stroke_joint(mg_canvas_data* canvas,
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

	mg_next_shape(canvas, attributes);

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

		u32 baseIndex = mg_vertices_base_index(canvas);
		i32* indices = mg_reserve_indices(canvas, 6);

		mg_push_vertex(canvas, p0);
		mg_push_vertex(canvas, (vec2){p0.x + n0.x*halfW, p0.y + n0.y*halfW});
		mg_push_vertex(canvas, mitterPoint);
		mg_push_vertex(canvas, (vec2){p0.x + n1.x*halfW, p0.y + n1.y*halfW});

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
		u32 baseIndex = mg_vertices_base_index(canvas);
		i32* indices = mg_reserve_indices(canvas, 3);

		mg_push_vertex(canvas, p0);
		mg_push_vertex(canvas, (vec2){p0.x + n0.x*halfW, p0.y + n0.y*halfW});
		mg_push_vertex(canvas, (vec2){p0.x + n1.x*halfW, p0.y + n1.y*halfW});

		DEBUG_ASSERT(!isnan(n0.x) && !isnan(n0.y) && !isnan(n1.x) && !isnan(n1.y));

		indices[0] = baseIndex;
		indices[1] = baseIndex + 1;
		indices[2] = baseIndex + 2;
	}
}

void mg_render_stroke_element(mg_canvas_data* canvas,
                                      mg_path_elt* element,
				      mg_attributes* attributes,
				      vec2 currentPoint,
				      vec2* startTangent,
				      vec2* endTangent,
				      vec2* endPoint)
{
	vec2 controlPoints[4] = {currentPoint, element->p[0], element->p[1], element->p[2]};
	int endPointIndex = 0;
	mg_next_shape(canvas, attributes);

	switch(element->type)
	{
		case MG_PATH_LINE:
			mg_render_stroke_line(canvas, controlPoints, attributes);
			endPointIndex = 1;
			break;

		case MG_PATH_QUADRATIC:
			mg_render_stroke_quadratic(canvas, controlPoints, attributes);
			endPointIndex = 2;
			break;

		case MG_PATH_CUBIC:
			mg_render_stroke_cubic(canvas, controlPoints, attributes);
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

u32 mg_render_stroke_subpath(mg_canvas_data* canvas,
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
	mg_render_stroke_element(canvas, elements + startIndex, attributes, currentPoint, &startTangent, &endTangent, &endPoint);

	firstTangent = startTangent;
	previousEndTangent = endTangent;
	currentPoint = endPoint;

	//NOTE(martin): render subsequent elements along with their joints
	u32 eltIndex = startIndex + 1;
	for(;
	    eltIndex<eltCount && elements[eltIndex].type != MG_PATH_MOVE;
	    eltIndex++)
	{
		mg_render_stroke_element(canvas, elements + eltIndex, attributes, currentPoint, &startTangent, &endTangent, &endPoint);

		if(attributes->joint != MG_JOINT_NONE)
		{
			mg_stroke_joint(canvas, currentPoint, previousEndTangent, startTangent, attributes);
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
			mg_stroke_joint(canvas, endPoint, endTangent, firstTangent, attributes);
		}
	}
	else if(attributes->cap == MG_CAP_SQUARE)
	{
		//NOTE(martin): add start and end cap
		mg_stroke_cap(canvas, startPoint, (vec2){-startTangent.x, -startTangent.y}, attributes);
		mg_stroke_cap(canvas, endPoint, startTangent, attributes);
	}

	return(eltIndex);
}


void mg_render_stroke(mg_canvas_data* canvas,
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
			startIndex = mg_render_stroke_subpath(canvas, elements, path, attributes, startIndex, startPoint);
		}
	}
}

//-----------------------------------------------------------------------------------------------------------
// Fast shapes primitives
//-----------------------------------------------------------------------------------------------------------

void mg_render_rectangle_fill(mg_canvas_data* canvas, mp_rect rect, mg_attributes* attributes)
{
	u32 baseIndex = mg_vertices_base_index(canvas);
	i32* indices = mg_reserve_indices(canvas, 6);

	mg_next_shape(canvas, attributes);

	mg_push_vertex(canvas, (vec2){rect.x, rect.y});
	mg_push_vertex(canvas, (vec2){rect.x + rect.w, rect.y});
	mg_push_vertex(canvas, (vec2){rect.x + rect.w, rect.y + rect.h});
	mg_push_vertex(canvas, (vec2){rect.x, rect.y + rect.h});

	indices[0] = baseIndex + 0;
	indices[1] = baseIndex + 1;
	indices[2] = baseIndex + 2;
	indices[3] = baseIndex + 0;
	indices[4] = baseIndex + 2;
	indices[5] = baseIndex + 3;
}

void mg_render_rectangle_stroke(mg_canvas_data* canvas, mp_rect rect, mg_attributes* attributes)
{
	//NOTE(martin): stroke a rectangle by fill two scaled rectangles with the same shapeIndex.
	u32 baseIndex = mg_vertices_base_index(canvas);
	i32* indices = mg_reserve_indices(canvas, 12);

	mg_next_shape(canvas, attributes);

	//NOTE(martin): limit stroke width to the minimum dimension of the rectangle
	f32 width = minimum(attributes->width, minimum(rect.w, rect.h));
	f32 halfW = width/2;

	// outer points
	mg_push_vertex(canvas, (vec2){rect.x - halfW, rect.y - halfW});
	mg_push_vertex(canvas, (vec2){rect.x + rect.w + halfW, rect.y - halfW});
	mg_push_vertex(canvas, (vec2){rect.x + rect.w + halfW, rect.y + rect.h + halfW});
	mg_push_vertex(canvas, (vec2){rect.x - halfW, rect.y + rect.h + halfW});

	// innter points
	mg_push_vertex(canvas, (vec2){rect.x + halfW, rect.y + halfW});
	mg_push_vertex(canvas, (vec2){rect.x + rect.w - halfW, rect.y + halfW});
	mg_push_vertex(canvas, (vec2){rect.x + rect.w - halfW, rect.y + rect.h - halfW});
	mg_push_vertex(canvas, (vec2){rect.x + halfW, rect.y + rect.h - halfW});

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

void mg_render_fill_arc_corner(mg_canvas_data* canvas, f32 x, f32 y, f32 rx, f32 ry)
{
	//NOTE(martin): draw a precomputed arc corner, using a bezier approximation
	u32 baseIndex = mg_vertices_base_index(canvas);
	i32* indices = mg_reserve_indices(canvas, 6);

	f32 cx = rx*4*(sqrt(2)-1)/3;
	f32 cy = ry*4*(sqrt(2)-1)/3;

	mg_push_vertex_cubic(canvas, (vec2){x, y + ry}, (vec4){-3.76797, -9.76362, 5.47912, -1});
	mg_push_vertex_cubic(canvas, (vec2){x, y + ry - cy}, (vec4){-4.19896, -9.45223, 7.534, -1});
	mg_push_vertex_cubic(canvas, (vec2){x + rx - cx, y}, (vec4){-4.19896, -7.534, 9.45223, -1});
	mg_push_vertex_cubic(canvas, (vec2){x + rx, y}, (vec4){-3.76797, -5.47912, 9.76362, -1});

	indices[0] = baseIndex + 0;
	indices[1] = baseIndex + 1;
	indices[2] = baseIndex + 2;
	indices[3] = baseIndex + 0;
	indices[4] = baseIndex + 2;
	indices[5] = baseIndex + 3;
}

void mg_render_rounded_rectangle_fill_path(mg_canvas_data* canvas,
							               mg_rounded_rect rect)
{
	//NOTE(martin): draw a rounded rectangle by drawing a normal rectangle and 4 corners,
	//              approximating an arc by a precomputed bezier curve

	u32 baseIndex = mg_vertices_base_index(canvas);
	i32* indices = mg_reserve_indices(canvas, 18);

	//NOTE(martin): inner cutted corner rectangle
	mg_push_vertex(canvas, (vec2){rect.x + rect.r, rect.y});
	mg_push_vertex(canvas, (vec2){rect.x + rect.w - rect.r, rect.y});
	mg_push_vertex(canvas, (vec2){rect.x + rect.w, rect.y + rect.r});
	mg_push_vertex(canvas, (vec2){rect.x + rect.w, rect.y + rect.h - rect.r});
	mg_push_vertex(canvas, (vec2){rect.x + rect.w - rect.r, rect.y + rect.h});
	mg_push_vertex(canvas, (vec2){rect.x + rect.r, rect.y + rect.h});
	mg_push_vertex(canvas, (vec2){rect.x, rect.y + rect.h - rect.r});
	mg_push_vertex(canvas, (vec2){rect.x, rect.y + rect.r});

	static const i32 fanIndices[18] = { 0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 6, 0, 6, 7 }; // inner fan
	for(int i=0; i<18; i++)
	{
		indices[i] = fanIndices[i] + baseIndex;
	}

	mg_render_fill_arc_corner(canvas, rect.x, rect.y, rect.r, rect.r);
	mg_render_fill_arc_corner(canvas, rect.x + rect.w, rect.y, -rect.r, rect.r);
	mg_render_fill_arc_corner(canvas, rect.x + rect.w, rect.y + rect.h, -rect.r, -rect.r);
	mg_render_fill_arc_corner(canvas, rect.x, rect.y + rect.h, rect.r, -rect.r);
}


void mg_render_rounded_rectangle_fill(mg_canvas_data* canvas,
                                      mg_rounded_rect rect,
					                  mg_attributes* attributes)
{
	mg_next_shape(canvas, attributes);
	mg_render_rounded_rectangle_fill_path(canvas, rect);
}

void mg_render_rounded_rectangle_stroke(mg_canvas_data* canvas,
                                        mg_rounded_rect rect,
                                        mg_attributes* attributes)
{
	//NOTE(martin): stroke rounded rectangle by filling two scaled rounded rectangles with the same shapeIndex
	f32 width = minimum(attributes->width, minimum(rect.w, rect.h));
	f32 halfW = width/2;

	mg_rounded_rect inner = {rect.x + halfW, rect.y + halfW, rect.w - width, rect.h - width, rect.r - halfW};
	mg_rounded_rect outer = {rect.x - halfW, rect.y - halfW, rect.w + width, rect.h + width, rect.r + halfW};

	mg_next_shape(canvas, attributes);
	mg_render_rounded_rectangle_fill_path(canvas, outer);
	mg_render_rounded_rectangle_fill_path(canvas, inner);
}

void mg_render_ellipse_fill_path(mg_canvas_data* canvas, mp_rect rect)
{
	//NOTE(martin): draw a filled ellipse by drawing a diamond and 4 corners,
	//              approximating an arc by a precomputed bezier curve
	f32 rx = rect.w/2;
	f32 ry = rect.h/2;

	u32 baseIndex = mg_vertices_base_index(canvas);
	i32* indices = mg_reserve_indices(canvas, 6);

	//NOTE(martin): inner diamond
	mg_push_vertex(canvas, (vec2){rect.x, rect.y + ry});
	mg_push_vertex(canvas, (vec2){rect.x + rx, rect.y});
	mg_push_vertex(canvas, (vec2){rect.x + rect.w, rect.y + ry});
	mg_push_vertex(canvas, (vec2){rect.x + rx, rect.y + rect.h});

	indices[0] = baseIndex + 0;
	indices[1] = baseIndex + 1;
	indices[2] = baseIndex + 2;
	indices[3] = baseIndex + 0;
	indices[4] = baseIndex + 2;
	indices[5] = baseIndex + 3;

	mg_render_fill_arc_corner(canvas, rect.x, rect.y, rx, ry);
	mg_render_fill_arc_corner(canvas, rect.x + rect.w, rect.y, -rx, ry);
	mg_render_fill_arc_corner(canvas, rect.x + rect.w, rect.y + rect.h, -rx, -ry);
	mg_render_fill_arc_corner(canvas, rect.x, rect.y + rect.h, rx, -ry);
}

void mg_render_ellipse_fill(mg_canvas_data* canvas, mp_rect rect, mg_attributes* attributes)
{
	mg_next_shape(canvas, attributes);
	mg_render_ellipse_fill_path(canvas, rect);
}

void mg_render_ellipse_stroke(mg_canvas_data* canvas, mp_rect rect, mg_attributes* attributes)
{
	//NOTE(martin): stroke by filling two scaled ellipsis with the same shapeIndex
	f32 width = minimum(attributes->width, minimum(rect.w, rect.h));
	f32 halfW = width/2;

	mp_rect inner = {rect.x + halfW, rect.y + halfW, rect.w - width, rect.h - width};
	mp_rect outer = {rect.x - halfW, rect.y - halfW, rect.w + width, rect.h + width};

	mg_next_shape(canvas, attributes);
	mg_render_ellipse_fill_path(canvas, outer);
	mg_render_ellipse_fill_path(canvas, inner);
}

//------------------------------------------------------------------------------------------
//NOTE(martin): fonts
//------------------------------------------------------------------------------------------

mg_font mg_font_nil() { return((mg_font){.h = 0}); }
bool mg_font_is_nil(mg_font font) { return(font.h == 0); }

mg_font mg_font_create_from_memory(u32 size, byte* buffer, u32 rangeCount, unicode_range* ranges)
{
	if(!__mgData.init)
	{
		mg_init();
	}
	mg_font fontHandle = mg_font_nil();

	mg_font_data* font = list_pop_entry(&__mgData.fontFreeList, mg_font_data, freeListElt);
	if(!font)
	{
		font = mem_arena_alloc_type(&__mgData.resourceArena, mg_font_data);
	}
	if(font)
	{
		memset(font, 0, sizeof(mg_font_data));
		fontHandle = mg_font_handle_alloc(font);

		stbtt_fontinfo stbttFontInfo;
		stbtt_InitFont(&stbttFontInfo, buffer, 0);

		//NOTE(martin): load font metrics data
		font->unitsPerEm = 1./stbtt_ScaleForMappingEmToPixels(&stbttFontInfo, 1);

		int ascent, descent, lineGap, x0, x1, y0, y1;
		stbtt_GetFontVMetrics(&stbttFontInfo, &ascent, &descent, &lineGap);
		stbtt_GetFontBoundingBox(&stbttFontInfo, &x0, &y0, &x1, &y1);

		font->extents.ascent = ascent;
		font->extents.descent = -descent;
		font->extents.leading = lineGap;
		font->extents.width = x1 - x0;

		stbtt_GetCodepointBox(&stbttFontInfo, 'x', &x0, &y0, &x1, &y1);
		font->extents.xHeight = y1 - y0;

		stbtt_GetCodepointBox(&stbttFontInfo, 'M', &x0, &y0, &x1, &y1);
		font->extents.capHeight = y1 - y0;

		//NOTE(martin): load codepoint ranges
		font->rangeCount = rangeCount;
		font->glyphMap = malloc_array(mg_glyph_map_entry, rangeCount);
		font->glyphCount = 0;

		for(int i=0; i<rangeCount; i++)
		{
			//NOTE(martin): initialize the map entry.
			//              The glyph indices are offseted by 1, to reserve 0 as an invalid glyph index.
			font->glyphMap[i].range = ranges[i];
			font->glyphMap[i].firstGlyphIndex = font->glyphCount + 1;
			font->glyphCount += ranges[i].count;
		}

		font->glyphs = malloc_array(mg_glyph_data, font->glyphCount);

		//NOTE(martin): first do a count of outlines
		int outlineCount = 0;
		for(int rangeIndex=0; rangeIndex<rangeCount; rangeIndex++)
		{
			utf32 codePoint = font->glyphMap[rangeIndex].range.firstCodePoint;
			u32 firstGlyphIndex = font->glyphMap[rangeIndex].firstGlyphIndex;
			u32 endGlyphIndex = firstGlyphIndex + font->glyphMap[rangeIndex].range.count;

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
		font->outlines = malloc_array(mg_path_elt, outlineCount);
		font->outlineCount = 0;

		//NOTE(martin): load metrics and outlines
		for(int rangeIndex=0; rangeIndex<rangeCount; rangeIndex++)
		{
			utf32 codePoint = font->glyphMap[rangeIndex].range.firstCodePoint;
			u32 firstGlyphIndex = font->glyphMap[rangeIndex].firstGlyphIndex;
			u32 endGlyphIndex = firstGlyphIndex + font->glyphMap[rangeIndex].range.count;

			for(int glyphIndex = firstGlyphIndex;
		    	glyphIndex < endGlyphIndex; glyphIndex++)
			{
				mg_glyph_data* glyph = &(font->glyphs[glyphIndex-1]);

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

				glyph->pathDescriptor = (mg_path_descriptor){.startIndex = font->outlineCount,
			                                                      	.count = vertexCount,
									      	.startPoint = {0, 0}};

				mg_path_elt* elements = font->outlines + font->outlineCount;
				font->outlineCount += vertexCount;
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
	}
	return(fontHandle);
}

void mg_font_destroy(mg_font fontHandle)
{
	mg_font_data* fontData = mg_font_data_from_handle(fontHandle);
	if(fontData)
	{
		free(fontData->glyphMap);
		free(fontData->glyphs);
		free(fontData->outlines);

		list_push(&__mgData.fontFreeList, &fontData->freeListElt);
		mg_handle_recycle(fontHandle.h);
	}
}

str32 mg_font_get_glyph_indices_from_font_data(mg_font_data* fontData, str32 codePoints, str32 backing)
{
	u64 count = minimum(codePoints.len, backing.len);

	for(int i = 0; i<count; i++)
	{
		u32 glyphIndex = 0;
		for(int rangeIndex=0; rangeIndex < fontData->rangeCount; rangeIndex++)
		{
			if(codePoints.ptr[i] >= fontData->glyphMap[rangeIndex].range.firstCodePoint
			  && codePoints.ptr[i] < (fontData->glyphMap[rangeIndex].range.firstCodePoint + fontData->glyphMap[rangeIndex].range.count))
			{
				u32 rangeOffset = codePoints.ptr[i] - fontData->glyphMap[rangeIndex].range.firstCodePoint;
				glyphIndex = fontData->glyphMap[rangeIndex].firstGlyphIndex + rangeOffset;
				break;
			}
		}
		if(glyphIndex && !fontData->glyphs[glyphIndex].exists)
		{
			backing.ptr[i] = 0;
		}
		backing.ptr[i] = glyphIndex;
	}
	str32 res = {.len = count, .ptr = backing.ptr};
	return(res);
}

u32 mg_font_get_glyph_index_from_font_data(mg_font_data* fontData, utf32 codePoint)
{
	u32 glyphIndex = 0;
	str32 codePoints = {1, &codePoint};
	str32 backing = {1, &glyphIndex};
	mg_font_get_glyph_indices_from_font_data(fontData, codePoints, backing);
	return(glyphIndex);
}

str32 mg_font_get_glyph_indices(mg_font font, str32 codePoints, str32 backing)
{
	mg_font_data* fontData = mg_font_data_from_handle(font);
	if(!fontData)
	{
		return((str32){0});
	}
	return(mg_font_get_glyph_indices_from_font_data(fontData, codePoints, backing));
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

mg_glyph_data* mg_font_get_glyph_data(mg_font_data* fontData, u32 glyphIndex)
{
	DEBUG_ASSERT(glyphIndex);
	DEBUG_ASSERT(glyphIndex < fontData->glyphCount);
	return(&(fontData->glyphs[glyphIndex-1]));
}

mg_font_extents mg_font_get_extents(mg_font font)
{
	mg_font_data* fontData = mg_font_data_from_handle(font);
	if(!fontData)
	{
		return((mg_font_extents){0});
	}
	return(fontData->extents);
}

mg_font_extents mg_font_get_scaled_extents(mg_font font, f32 emSize)
{
	mg_font_data* fontData = mg_font_data_from_handle(font);
	if(!fontData)
	{
		return((mg_font_extents){0});
	}
	f32 scale = emSize/fontData->unitsPerEm;
	mg_font_extents extents = fontData->extents;

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
	mg_font_data* fontData = mg_font_data_from_handle(font);
	if(!fontData)
	{
		return(0);
	}
	return(emSize/fontData->unitsPerEm);
}

void mg_font_get_glyph_extents_from_font_data(mg_font_data* fontData,
                                              str32 glyphIndices,
                                              mg_text_extents* outExtents)
{
	for(int i=0; i<glyphIndices.len; i++)
	{
		if(!glyphIndices.ptr[i] || glyphIndices.ptr[i] >= fontData->glyphCount)
		{
			continue;
		}
		mg_glyph_data* glyph = mg_font_get_glyph_data(fontData, glyphIndices.ptr[i]);
		outExtents[i] = glyph->extents;
	}
}

int mg_font_get_glyph_extents(mg_font font, str32 glyphIndices, mg_text_extents* outExtents)
{
	mg_font_data* fontData = mg_font_data_from_handle(font);
	if(!fontData)
	{
		return(-1);
	}
	mg_font_get_glyph_extents_from_font_data(fontData, glyphIndices, outExtents);
	return(0);
}

int mg_font_get_codepoint_extents(mg_font font, utf32 codePoint, mg_text_extents* outExtents)
{
	mg_font_data* fontData = mg_font_data_from_handle(font);
	if(!fontData)
	{
		return(-1);
	}
	u32 glyphIndex = 0;
	str32 codePoints = {1, &codePoint};
	str32 backing = {1, &glyphIndex};
	str32 glyphs = mg_font_get_glyph_indices_from_font_data(fontData, codePoints, backing);
	mg_font_get_glyph_extents_from_font_data(fontData, glyphs, outExtents);
	return(0);
}

mp_rect mg_text_bounding_box_utf32(mg_font font, f32 fontSize, str32 codePoints)
{
	if(!codePoints.len || !codePoints.ptr)
	{
		return((mp_rect){0});
	}

	mg_font_data* fontData = mg_font_data_from_handle(font);
	if(!fontData)
	{
		return((mp_rect){0});
	}

	mem_arena* scratch = mem_scratch();
	str32 glyphIndices = mg_font_push_glyph_indices(font, scratch, codePoints);

	//NOTE(martin): find width of missing character
	//TODO(martin): should cache that at font creation...
	mg_text_extents missingGlyphExtents;
	u32 missingGlyphIndex = mg_font_get_glyph_index_from_font_data(fontData, 0xfffd);

	if(missingGlyphIndex)
	{
		mg_font_get_glyph_extents_from_font_data(fontData, (str32){1, &missingGlyphIndex}, &missingGlyphExtents);
	}
	else
	{
		//NOTE(martin): could not find replacement glyph, try to get an 'x' to get a somewhat correct width
		//              to render an empty rectangle. Otherwise just render with the max font width
		f32 boxWidth = fontData->extents.width * 0.8;
		f32 xBearing = fontData->extents.width * 0.1;
		f32 xAdvance = fontData->extents.width;

		missingGlyphIndex = mg_font_get_glyph_index_from_font_data(fontData, 'x');
		if(missingGlyphIndex)
		{
			mg_font_get_glyph_extents_from_font_data(fontData, (str32){1, &missingGlyphIndex}, &missingGlyphExtents);
		}
		else
		{
			missingGlyphExtents.xBearing = fontData->extents.width * 0.1;
			missingGlyphExtents.yBearing = 0;
			missingGlyphExtents.width = fontData->extents.width * 0.8;
			missingGlyphExtents.xAdvance = fontData->extents.width;
			missingGlyphExtents.yAdvance = 0;
		}
	}

	//NOTE(martin): accumulate text extents
	f32 width = 0;
	f32 x = 0;
	f32 y = 0;
	f32 lineHeight = fontData->extents.descent + fontData->extents.ascent;

	for(int i=0; i<glyphIndices.len; i++)
	{
		//TODO(martin): make it failsafe for fonts that don't have a glyph for the line-feed codepoint ?

		mg_glyph_data* glyph = 0;
		mg_text_extents extents;
		if(!glyphIndices.ptr[i] || glyphIndices.ptr[i] >= fontData->glyphCount)
		{
			extents = missingGlyphExtents;
		}
		else
		{
			glyph = mg_font_get_glyph_data(fontData, glyphIndices.ptr[i]);
			extents = glyph->extents;
		}
		x += extents.xAdvance;
		y += extents.yAdvance;

		if(glyph && glyph->codePoint == '\n')
		{
			width = maximum(width, x);
			x = 0;
			y += lineHeight + fontData->extents.leading;
		}
	}
	width = maximum(width, x);

	f32 fontScale = mg_font_get_scale_for_em_pixels(font, fontSize);
	mp_rect rect = {0, -fontData->extents.ascent * fontScale, width * fontScale, (y + lineHeight) * fontScale };
	return(rect);
}

mp_rect mg_text_bounding_box(mg_font font, f32 fontSize, str8 text)
{
	if(!text.len || !text.ptr)
	{
		return((mp_rect){0});
	}

	mem_arena* scratch = mem_scratch();
	str32 codePoints = utf8_push_to_codepoints(scratch, text);
	return(mg_text_bounding_box_utf32(font, fontSize, codePoints));
}

//------------------------------------------------------------------------------------------
//NOTE(martin): graphics canvas API
//------------------------------------------------------------------------------------------

mg_canvas mg_canvas_nil() { return((mg_canvas){.h = 0}); }
bool mg_canvas_is_nil(mg_canvas canvas) { return(canvas.h == 0); }

#if MG_COMPILE_BACKEND_METAL
	mg_canvas_backend* mg_mtl_canvas_create(mg_surface surface);
#endif

#if MG_COMPILE_BACKEND_GL
	mg_canvas_backend* mg_gl_canvas_create(mg_surface surface);
#endif

mg_canvas mg_canvas_create(mg_surface surface)
{
	mg_canvas canvasHandle = mg_canvas_nil();
	mg_surface_data* surfaceData = mg_surface_data_from_handle(surface);
	if(surfaceData)
	{
		mg_canvas_backend* backend = 0;
		switch(surfaceData->backend)
		{
		#if MG_COMPILE_BACKEND_METAL
			case MG_BACKEND_METAL:
				backend = mg_mtl_canvas_create(surface);
				break;
		#endif

		#if MG_COMPILE_BACKEND_GL
			case MG_BACKEND_GL:
				backend = mg_gl_canvas_create(surface);
				break;
		#endif

			default:
				break;
		}

		if(backend)
		{
			mg_canvas_data* canvas = list_pop_entry(&__mgData.canvasFreeList, mg_canvas_data, freeListElt);
			if(!canvas)
			{
				canvas = mem_arena_alloc_type(&__mgData.resourceArena, mg_canvas_data);
			}
			if(canvas)
			{
				memset(canvas, 0, sizeof(mg_canvas_data));
			}

			canvas->surface = surface;
			canvas->backend = backend;

			canvas->attributes.color = (mg_color){0, 0, 0, 1};
			canvas->attributes.tolerance = 1;
			canvas->attributes.width = 10;
			canvas->attributes.clip = (mp_rect){-FLT_MAX/2, -FLT_MAX/2, FLT_MAX, FLT_MAX};

	        canvasHandle = mg_canvas_handle_alloc(canvas);
	        mg_canvas_set_current(canvasHandle);

	        //TODO: move that in mg_canvas_set_current() if needed?
	        mg_surface_prepare(surface);
		}
	}
	return(canvasHandle);
}

void mg_canvas_destroy(mg_canvas handle)
{
	mg_canvas_data* canvas = mg_canvas_data_from_handle(handle);
	if(canvas)
	{
		if(__mgCurrentCanvas == canvas)
		{
			__mgCurrentCanvas = 0;
			__mgCurrentCanvasHandle = mg_canvas_nil();
		}

		if(canvas->backend && canvas->backend->destroy)
		{
			canvas->backend->destroy(canvas->backend);
		}
		list_push(&__mgData.canvasFreeList, &canvas->freeListElt);
		mg_handle_recycle(handle.h);
	}
}

mg_canvas mg_canvas_set_current(mg_canvas canvas)
{
	mg_canvas old = __mgCurrentCanvasHandle;

	__mgCurrentCanvasHandle = canvas;
	__mgCurrentCanvas = mg_canvas_data_from_handle(canvas);

	return(old);
}

vec2 mg_canvas_size(void)
{
	vec2 res = {0};
	if(__mgCurrentCanvas)
	{
		mp_rect frame = mg_surface_get_frame(__mgCurrentCanvas->surface);
		res = (vec2){frame.w, frame.h};
	}
	return(res);
}
////////////////////////////////////////////////////////////


mp_rect mg_clip_stack_top(mg_canvas_data* canvas)
{
	if(canvas->clipStackSize == 0)
	{
		return((mp_rect){-FLT_MAX/2, -FLT_MAX/2, FLT_MAX, FLT_MAX});
	}
	else
	{
		return(canvas->clipStack[canvas->clipStackSize-1]);
	}
}

void mg_clip_stack_push(mg_canvas_data* canvas, mp_rect clip)
{
	if(canvas->clipStackSize >= MG_CLIP_STACK_MAX_DEPTH)
	{
		LOG_ERROR("clip stack overflow\n");
	}
	else
	{
		canvas->clipStack[canvas->clipStackSize] = clip;
		canvas->clipStackSize++;
		canvas->clip = clip;
	}
}

void mg_clip_stack_pop(mg_canvas_data* canvas)
{
	if(canvas->clipStackSize == 0)
	{
		LOG_ERROR("clip stack underflow\n");
	}
	else
	{
		canvas->clipStackSize--;
		canvas->clip = mg_clip_stack_top(canvas);
	}
}

void mg_do_clip_push(mg_canvas_data* canvas, mp_rect clip)
{
	//NOTE(martin): transform clip
	vec2 p0 = mg_mat2x3_mul(canvas->transform, (vec2){clip.x, clip.y});
	vec2 p1 = mg_mat2x3_mul(canvas->transform, (vec2){clip.x + clip.w, clip.y});
	vec2 p2 = mg_mat2x3_mul(canvas->transform, (vec2){clip.x + clip.w, clip.y + clip.h});
	vec2 p3 = mg_mat2x3_mul(canvas->transform, (vec2){clip.x, clip.y + clip.h});

	f32 x0 = minimum(p0.x, minimum(p1.x, minimum(p2.x, p3.x)));
	f32 y0 = minimum(p0.y, minimum(p1.y, minimum(p2.y, p3.y)));
	f32 x1 = maximum(p0.x, maximum(p1.x, maximum(p2.x, p3.x)));
	f32 y1 = maximum(p0.y, maximum(p1.y, maximum(p2.y, p3.y)));

	mp_rect current = mg_clip_stack_top(canvas);

	//NOTE(martin): intersect with current clip
	x0 = maximum(current.x, x0);
	y0 = maximum(current.y, y0);
	x1 = minimum(current.x + current.w, x1);
	y1 = minimum(current.y + current.h, y1);

	mp_rect r = {x0, y0, maximum(0, x1-x0), maximum(0, y1-y0)};
	mg_clip_stack_push(canvas, r);
}

void mg_draw_batch(mg_canvas_data* canvas, mg_image_data* image)
{
	mg_finalize_shape(canvas);

	if(canvas->backend && canvas->backend->drawBatch && canvas->indexCount)
	{
		canvas->backend->drawBatch(canvas->backend, image, canvas->nextShapeIndex, canvas->vertexCount, canvas->indexCount);
	}
	mg_reset_shape_index(canvas);

	canvas->vertexCount = 0;
	canvas->indexCount = 0;
}

void mg_flush_commands(int primitiveCount, mg_primitive* primitives, mg_path_elt* pathElements)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(!canvas)
	{
		return;
	}

	u32 nextIndex = 0;

	mg_reset_shape_index(canvas);

	canvas->clip = (mp_rect){-FLT_MAX/2, -FLT_MAX/2, FLT_MAX, FLT_MAX};
	canvas->image = mg_image_nil();

	canvas->backend->begin(canvas->backend, canvas->clearColor);

	for(int i=0; i<primitiveCount; i++)
	{
		if(nextIndex >= primitiveCount)
		{
			LOG_ERROR("invalid location '%i' in graphics command buffer would cause an overrun\n", nextIndex);
			break;
		}
		mg_primitive* primitive = &(primitives[nextIndex]);
		nextIndex++;

		if(i && primitive->attributes.image.h != canvas->image.h)
		{
			mg_image_data* imageData = mg_image_data_from_handle(canvas->image);
			mg_draw_batch(canvas, imageData);
			canvas->image = primitive->attributes.image;
		}

		switch(primitive->cmd)
		{
			case MG_CMD_FILL:
				{
				mg_next_shape(canvas, &primitive->attributes);
				mg_render_fill(canvas,
						       pathElements + primitive->path.startIndex,
						       &primitive->path);
			} break;

			case MG_CMD_STROKE:
			{
				mg_render_stroke(canvas,
							 pathElements + primitive->path.startIndex,
							 &primitive->path,
							 &primitive->attributes);
			} break;


			case MG_CMD_RECT_FILL:
				mg_render_rectangle_fill(canvas, primitive->rect, &primitive->attributes);
				break;

			case MG_CMD_RECT_STROKE:
				mg_render_rectangle_stroke(canvas, primitive->rect, &primitive->attributes);
				break;

			case MG_CMD_ROUND_RECT_FILL:
				mg_render_rounded_rectangle_fill(canvas, primitive->roundedRect, &primitive->attributes);
				break;

			case MG_CMD_ROUND_RECT_STROKE:
				mg_render_rounded_rectangle_stroke(canvas, primitive->roundedRect, &primitive->attributes);
				break;

			case MG_CMD_ELLIPSE_FILL:
				mg_render_ellipse_fill(canvas, primitive->rect, &primitive->attributes);
				break;

			case MG_CMD_ELLIPSE_STROKE:
				mg_render_ellipse_stroke(canvas, primitive->rect, &primitive->attributes);
				break;

			case MG_CMD_JUMP:
			{
				if(primitive->jump == ~0)
				{
					//NOTE(martin): normal end of stream marker
					goto exit_command_loop;
				}
				else if(primitive->jump >= primitiveCount)
				{
					LOG_ERROR("invalid jump location '%i' in graphics command buffer\n", primitive->jump);
					goto exit_command_loop;
				}
				else
				{
					nextIndex = primitive->jump;
				}
			} break;

			case MG_CMD_CLIP_PUSH:
			{
				//TODO(martin): use only aligned rect and avoid this
				mp_rect r = {primitive->rect.x, primitive->rect.y, primitive->rect.w, primitive->rect.h};
				mg_do_clip_push(canvas, r);
			} break;

			case MG_CMD_CLIP_POP:
			{
				mg_clip_stack_pop(canvas);
			} break;

		}
	}
	exit_command_loop: ;

	mg_image_data* imageData = mg_image_data_from_handle(canvas->image);
	mg_draw_batch(canvas, imageData);

	canvas->backend->end(canvas->backend);

	//NOTE(martin): clear buffers
	canvas->vertexCount = 0;
	canvas->indexCount = 0;
}

void mg_flush()
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(!canvas)
	{
		return;
	}

	mg_flush_commands(canvas->primitiveCount, canvas->primitives, canvas->pathElements);

	canvas->primitiveCount = 0;
	canvas->path.startIndex = 0;
	canvas->path.count = 0;
}

//------------------------------------------------------------------------------------------
//NOTE(martin): transform, viewport and clipping
//------------------------------------------------------------------------------------------

void mg_matrix_push(mg_mat2x3 matrix)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		mg_mat2x3 transform = mg_matrix_stack_top(canvas);
		mg_matrix_stack_push(canvas, mg_mat2x3_mul_m(transform, matrix));
	}
}

void mg_matrix_pop()
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		mg_matrix_stack_pop(canvas);
	}
}

void mg_clip_push(f32 x, f32 y, f32 w, f32 h)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		mg_push_command(canvas, (mg_primitive){.cmd = MG_CMD_CLIP_PUSH,
	                                        .rect = (mp_rect){x, y, w, h}});
	}
}

void mg_clip_pop()
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		mg_push_command(canvas, (mg_primitive){.cmd = MG_CMD_CLIP_POP});
	}
}

//------------------------------------------------------------------------------------------
//NOTE(martin): graphics attributes setting/getting
//------------------------------------------------------------------------------------------
void mg_set_color(mg_color color)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		canvas->attributes.color = color;
	}
}

void mg_set_color_rgba(f32 r, f32 g, f32 b, f32 a)
{
	mg_set_color((mg_color){r, g, b, a});
}

void mg_set_width(f32 width)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		canvas->attributes.width = width;
	}
}

void mg_set_tolerance(f32 tolerance)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		canvas->attributes.tolerance = tolerance;
	}
}

void mg_set_joint(mg_joint_type joint)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		canvas->attributes.joint = joint;
	}
}

void mg_set_max_joint_excursion(f32 maxJointExcursion)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		canvas->attributes.maxJointExcursion = maxJointExcursion;
	}
}

void mg_set_cap(mg_cap_type cap)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		canvas->attributes.cap = cap;
	}
}

void mg_set_font(mg_font font)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		canvas->attributes.font = font;
	}
}

void mg_set_font_size(f32 fontSize)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		canvas->attributes.fontSize = fontSize;
	}
}

void mg_set_text_flip(bool flip)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		canvas->textFlip = flip;
	}
}

void mg_set_image(mg_image image)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		canvas->attributes.image = image;
		vec2 size = mg_image_size(image);
		canvas->attributes.srcRegion = (mp_rect){0, 0, size.x, size.y};
	}
}

void mg_set_image_source_region(mp_rect region)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		canvas->attributes.srcRegion = region;
	}
}

mg_color mg_get_color()
{
	mg_color color = {0};
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		color = canvas->attributes.color;
	}
	return(color);
}

f32 mg_get_width()
{
	f32 width = 0;
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		width = canvas->attributes.width;
	}
	return(width);
}

f32 mg_get_tolerance()
{
	f32 tolerance = 0;
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		tolerance = canvas->attributes.tolerance;
	}
	return(tolerance);
}

mg_joint_type mg_get_joint()
{
	mg_joint_type joint = 0;
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		joint = canvas->attributes.joint;
	}
	return(joint);
}

f32 mg_get_max_joint_excursion()
{
	f32 maxJointExcursion = 0;
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		maxJointExcursion = canvas->attributes.maxJointExcursion;
	}
	return(maxJointExcursion);
}

mg_cap_type mg_get_cap()
{
	mg_cap_type cap = 0;
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		cap = canvas->attributes.cap;
	}
	return(cap);
}

mg_font mg_get_font()
{
	mg_font font = mg_font_nil();
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		font = canvas->attributes.font;
	}
	return(font);
}

f32 mg_get_font_size()
{
	f32 fontSize = 0;
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		fontSize = canvas->attributes.fontSize;
	}
	return(fontSize);
}

bool mg_get_text_flip()
{
	bool flip = false;
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		flip = canvas->textFlip;
	}
	return(flip);
}

//------------------------------------------------------------------------------------------
//NOTE(martin): path construction
//------------------------------------------------------------------------------------------
vec2 mg_get_position()
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(!canvas)
	{
		return((vec2){0, 0});
	}
	return(canvas->subPathLastPoint);
}

void mg_move_to(f32 x, f32 y)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(!canvas)
	{
		return;
	}
	mg_path_push_element(canvas, ((mg_path_elt){.type = MG_PATH_MOVE, .p[0] = {x, y}}));
	canvas->subPathStartPoint = (vec2){x, y};
	canvas->subPathLastPoint = (vec2){x, y};
}

void mg_line_to(f32 x, f32 y)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(!canvas)
	{
		return;
	}
	mg_path_push_element(canvas, ((mg_path_elt){.type = MG_PATH_LINE, .p[0] = {x, y}}));
	canvas->subPathLastPoint = (vec2){x, y};
}

void mg_quadratic_to(f32 x1, f32 y1, f32 x2, f32 y2)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(!canvas)
	{
		return;
	}
	mg_path_push_element(canvas, ((mg_path_elt){.type = MG_PATH_QUADRATIC, .p = {{x1, y1}, {x2, y2}}}));
	canvas->subPathLastPoint = (vec2){x2, y2};
}

void mg_cubic_to(f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(!canvas)
	{
		return;
	}
	mg_path_push_element(canvas, ((mg_path_elt){.type = MG_PATH_CUBIC, .p = {{x1, y1}, {x2, y2}, {x3, y3}}}));
	canvas->subPathLastPoint = (vec2){x3, y3};
}

void mg_close_path()
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(!canvas)
	{
		return;
	}
	if(  canvas->subPathStartPoint.x != canvas->subPathLastPoint.x
	  || canvas->subPathStartPoint.y != canvas->subPathLastPoint.y)
	{
		mg_line_to(canvas->subPathStartPoint.x, canvas->subPathStartPoint.y);
	}
	canvas->subPathStartPoint = canvas->subPathLastPoint;
}

mp_rect mg_glyph_outlines_from_font_data(mg_font_data* fontData, str32 glyphIndices)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;

	f32 startX = canvas->subPathLastPoint.x;
	f32 startY = canvas->subPathLastPoint.y;
	f32 maxWidth = 0;

	f32 scale = canvas->attributes.fontSize/fontData->unitsPerEm;

	for(int i=0; i<glyphIndices.len; i++)
	{
		u32 glyphIndex = glyphIndices.ptr[i];

		f32 xOffset = canvas->subPathLastPoint.x;
		f32 yOffset = canvas->subPathLastPoint.y;
		f32 flip = canvas->textFlip ? 1 : -1;

		if(!glyphIndex || glyphIndex >= fontData->glyphCount)
		{
			LOG_WARNING("code point is not present in font ranges\n");
			//NOTE(martin): try to find the replacement character
			glyphIndex = mg_font_get_glyph_index_from_font_data(fontData, 0xfffd);
			if(!glyphIndex)
			{
				//NOTE(martin): could not find replacement glyph, try to get an 'x' to get a somewhat correct width
				//              to render an empty rectangle. Otherwise just render with the max font width
				f32 boxWidth = fontData->extents.width * 0.8;
				f32 xBearing = fontData->extents.width * 0.1;
				f32 xAdvance = fontData->extents.width;

				glyphIndex = mg_font_get_glyph_index_from_font_data(fontData, 'x');
				if(glyphIndex)
				{
					mg_glyph_data* glyph = &(fontData->glyphs[glyphIndex]);
					boxWidth = glyph->extents.width;
					xBearing = glyph->extents.xBearing;
					xAdvance = glyph->extents.xAdvance;
				}
				f32 oldStrokeWidth = canvas->attributes.width;

				mg_set_width(boxWidth*0.005);
				mg_rectangle_stroke(xOffset + xBearing * scale,
				                    yOffset,
				                    boxWidth * scale * flip,
				                    fontData->extents.capHeight*scale);

				mg_set_width(oldStrokeWidth);
				mg_move_to(xOffset + xAdvance * scale, yOffset);
				maxWidth = maximum(maxWidth, xOffset + xAdvance*scale - startX);
				continue;
			}
		}

		mg_glyph_data* glyph = mg_font_get_glyph_data(fontData, glyphIndex);

		mg_path_push_elements(canvas, glyph->pathDescriptor.count, fontData->outlines + glyph->pathDescriptor.startIndex);

		mg_path_elt* elements = canvas->pathElements + canvas->path.count + canvas->path.startIndex - glyph->pathDescriptor.count;
		for(int eltIndex=0; eltIndex<glyph->pathDescriptor.count; eltIndex++)
		{
			for(int pIndex = 0; pIndex < 3; pIndex++)
			{
				elements[eltIndex].p[pIndex].x = elements[eltIndex].p[pIndex].x * scale + xOffset;
				elements[eltIndex].p[pIndex].y = elements[eltIndex].p[pIndex].y * scale * flip + yOffset;
			}
		}
		mg_move_to(xOffset + scale*glyph->extents.xAdvance, yOffset);

		maxWidth = maximum(maxWidth, xOffset + scale*glyph->extents.xAdvance - startX);
	}
	f32 lineHeight = (fontData->extents.ascent + fontData->extents.descent)*scale;
	mp_rect box = {startX, startY, maxWidth, canvas->subPathLastPoint.y - startY + lineHeight };
	return(box);
}

mp_rect mg_glyph_outlines(str32 glyphIndices)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(!canvas)
	{
		return((mp_rect){0});
	}
	mg_font_data* fontData = mg_font_data_from_handle(canvas->attributes.font);
	if(!fontData)
	{
		return((mp_rect){0});
	}
	return(mg_glyph_outlines_from_font_data(fontData, glyphIndices));
}

void mg_codepoints_outlines(str32 codePoints)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(!canvas)
	{
		return;
	}
	mg_font_data* fontData = mg_font_data_from_handle(canvas->attributes.font);
	if(!fontData)
	{
		return;
	}

	str32 glyphIndices = mg_font_push_glyph_indices(canvas->attributes.font, mem_scratch(), codePoints);
	mg_glyph_outlines_from_font_data(fontData, glyphIndices);
}

void mg_text_outlines(str8 text)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(!canvas)
	{
		return;
	}
	mg_font_data* fontData = mg_font_data_from_handle(canvas->attributes.font);
	if(!fontData)
	{
		return;
	}

	mem_arena* scratch = mem_scratch();
	str32 codePoints = utf8_push_to_codepoints(scratch, text);
	str32 glyphIndices = mg_font_push_glyph_indices(canvas->attributes.font, scratch, codePoints);

	mg_glyph_outlines_from_font_data(fontData, glyphIndices);
}

//------------------------------------------------------------------------------------------
//NOTE(martin): clear/fill/stroke
//------------------------------------------------------------------------------------------

void mg_clear()
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		canvas->primitiveCount = 0;
		canvas->clearColor = canvas->attributes.color;
	}
}

void mg_fill()
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas && canvas->path.count)
	{
		mg_push_command(canvas, (mg_primitive){.cmd = MG_CMD_FILL, .path = canvas->path});
		mg_new_path(canvas);
	}
}

void mg_stroke()
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas && canvas->path.count)
	{
		mg_push_command(canvas, (mg_primitive){.cmd = MG_CMD_STROKE, .path = canvas->path});
		mg_new_path(canvas);
	}
}

//------------------------------------------------------------------------------------------
//NOTE(martin): 'fast' shapes primitives
//------------------------------------------------------------------------------------------
void mg_rectangle_fill(f32 x, f32 y, f32 w, f32 h)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		mg_primitive primitive = {.cmd = MG_CMD_RECT_FILL, .rect = (mp_rect){x, y, w, h}};
		mg_push_command(canvas, primitive);
	}
}

void mg_rectangle_stroke(f32 x, f32 y, f32 w, f32 h)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		mg_primitive primitive = {.cmd = MG_CMD_RECT_STROKE, .rect = (mp_rect){x, y, w, h}};
		mg_push_command(canvas, primitive);
	}
}

void mg_rounded_rectangle_fill(f32 x, f32 y, f32 w, f32 h, f32 r)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		mg_primitive primitive = {.cmd = MG_CMD_ROUND_RECT_FILL,
		                          .roundedRect = (mg_rounded_rect){x, y, w, h, r}};
		mg_push_command(canvas, primitive);
	}
}

void mg_rounded_rectangle_stroke(f32 x, f32 y, f32 w, f32 h, f32 r)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		mg_primitive primitive = {.cmd = MG_CMD_ROUND_RECT_STROKE,
		                          .roundedRect = (mg_rounded_rect){x, y, w, h, r}};
		mg_push_command(canvas, primitive);
	}
}

void mg_circle_fill(f32 x, f32 y, f32 r)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		mg_primitive primitive = {.cmd = MG_CMD_ELLIPSE_FILL,
		                          .rect = (mp_rect){x-r, y-r, 2*r, 2*r}};
		mg_push_command(canvas, primitive);
	}
}

void mg_circle_stroke(f32 x, f32 y, f32 r)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		mg_primitive primitive = {.cmd = MG_CMD_ELLIPSE_STROKE,
		                          .rect = (mp_rect){x-r, y-r, 2*r, 2*r}};
		mg_push_command(canvas, primitive);
	}
}

void mg_ellipse_fill(f32 x, f32 y, f32 rx, f32 ry)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		mg_primitive primitive = {.cmd = MG_CMD_ELLIPSE_FILL,
		                          .rect = (mp_rect){x-rx, y-ry, 2*rx, 2*ry}};
		mg_push_command(canvas, primitive);
	}
}

void mg_ellipse_stroke(f32 x, f32 y, f32 rx, f32 ry)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		mg_primitive primitive = {.cmd = MG_CMD_ELLIPSE_STROKE,
		                          .rect = (mp_rect){x-rx, y-ry, 2*rx, 2*ry}};
		mg_push_command(canvas, primitive);
	}
}

//TODO: change to arc_to?
void mg_arc(f32 x, f32 y, f32 r, f32 arcAngle, f32 startAngle)
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

		mg_move_to(v0.x, v0.y);
		mg_cubic_to(v1.x, v1.y, v2.x, v2.y, v3.x, v3.y);

		startAngle += smallAngle;
	}
}

//------------------------------------------------------------------------------------------
//NOTE(martin): images
//------------------------------------------------------------------------------------------


mg_image mg_image_nil() { return((mg_image){.h = 0}); }
bool mg_image_is_nil(mg_image image) { return(image.h == 0); }

mg_image mg_image_create(u32 width, u32 height)
{
	mg_image image = mg_image_nil();
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		mg_image_data* imageData = canvas->backend->imageCreate(canvas->backend, (vec2){width, height});
		if(imageData)
		{
			image = mg_image_handle_alloc(imageData);
		}
	}
	return(image);
}

mg_image mg_image_create_from_rgba8(u32 width, u32 height, u8* pixels)
{
	mg_image image = mg_image_create(width, height);
	if(!mg_image_is_nil(image))
	{
		mg_image_upload_region_rgba8(image, (mp_rect){0, 0, width, height}, pixels);
	}
	return(image);
}

mg_image mg_image_create_from_data(str8 data, bool flip)
{
	mg_image image = mg_image_nil();
	int width, height, channels;

	stbi_set_flip_vertically_on_load(flip ? 1 : 0);
	u8* pixels = stbi_load_from_memory((u8*)data.ptr, data.len, &width, &height, &channels, 4);

	if(pixels)
	{
		image = mg_image_create_from_rgba8(width, height, pixels);
		free(pixels);
	}
	return(image);
}

mg_image mg_image_create_from_file(str8 path, bool flip)
{
	mg_image image = mg_image_nil();
	int width, height, channels;

	const char* cpath = str8_to_cstring(mem_scratch(), path);

	stbi_set_flip_vertically_on_load(flip ? 1 : 0);
	u8* pixels = stbi_load(cpath, &width, &height, &channels, 4);
	if(pixels)
	{
		image = mg_image_create_from_rgba8(width, height, pixels);
		free(pixels);
	}
	return(image);
}

void mg_image_destroy(mg_image image)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		mg_image_data* imageData = mg_image_data_from_handle(image);
		if(imageData)
		{
			canvas->backend->imageDestroy(canvas->backend, imageData);
			mg_handle_recycle(image.h);
		}
	}
}

void mg_image_upload_region_rgba8(mg_image image, mp_rect region, u8* pixels)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		mg_image_data* imageData = mg_image_data_from_handle(image);
		if(imageData)
		{
			canvas->backend->imageUploadRegion(canvas->backend, imageData, region, pixels);
		}
	}
}

vec2 mg_image_size(mg_image image)
{
	vec2 res = {0};
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		mg_image_data* imageData = mg_image_data_from_handle(image);
		if(imageData)
		{
			res = imageData->size;
		}
	}
	return(res);
}

void mg_image_draw_region(mg_image image, mp_rect srcRegion, mp_rect dstRegion)
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		mg_image oldImage = canvas->attributes.image;
		mp_rect oldSrcRegion = canvas->attributes.srcRegion;
		mg_color oldColor = canvas->attributes.color;

		canvas->attributes.image = image;
		canvas->attributes.srcRegion = srcRegion;
		canvas->attributes.color = (mg_color){1, 1, 1, 1};

		mg_push_command(canvas, (mg_primitive){.cmd = MG_CMD_RECT_FILL, .rect = dstRegion});

		canvas->attributes.image = oldImage;
		canvas->attributes.srcRegion = oldSrcRegion;
		canvas->attributes.color = oldColor;
	}
}

void mg_image_draw(mg_image image, mp_rect rect)
{
	vec2 size = mg_image_size(image);
	mg_image_draw_region(image, (mp_rect){0, 0, size.x, size.y}, rect);
}

//------------------------------------------------------------------------------------------
//NOTE(martin): atlasing
//------------------------------------------------------------------------------------------

//NOTE: rectangle allocator
typedef struct mg_rect_atlas
{
	mem_arena* arena;
	ivec2 size;
	ivec2 pos;
	u32  lineHeight;

} mg_rect_atlas;

mg_rect_atlas* mg_rect_atlas_create(mem_arena* arena, i32 width, i32 height)
{
	mg_rect_atlas* atlas = mem_arena_alloc_type(arena, mg_rect_atlas);
	memset(atlas, 0, sizeof(mg_rect_atlas));
	atlas->arena = arena;
	atlas->size = (ivec2){width, height};
	return(atlas);
}

mp_rect mg_rect_atlas_alloc(mg_rect_atlas* atlas, i32 width, i32 height)
{
	mp_rect rect = {0, 0, 0, 0};
	if(width > 0 && height > 0)
	{
		if(atlas->pos.x + width >= atlas->size.x)
		{
			atlas->pos.x = 0;
			atlas->pos.y += (atlas->lineHeight + 1);
			atlas->lineHeight = 0;
		}
		if(  atlas->pos.x + width < atlas->size.x
	  	&& atlas->pos.y + height < atlas->size.y)
		{
			rect = (mp_rect){atlas->pos.x, atlas->pos.y, width, height};

			atlas->pos.x += (width + 1);
			atlas->lineHeight = maximum(atlas->lineHeight, height);
		}
	}
	return(rect);
}

void mg_rect_atlas_recycle(mg_rect_atlas* atlas, mp_rect rect)
{
	//TODO
}

mg_image_region mg_image_atlas_alloc_from_rgba8(mg_rect_atlas* atlas, mg_image backingImage, u32 width, u32 height, u8* pixels)
{
	mg_image_region imageRgn = {0};

	mp_rect rect = mg_rect_atlas_alloc(atlas, width, height);
	if(rect.w == width && rect.h == height)
	{
		mg_image_upload_region_rgba8(backingImage, rect, pixels);
		imageRgn.rect = rect;
		imageRgn.image = backingImage;
	}
	return(imageRgn);
}

mg_image_region mg_image_atlas_alloc_from_data(mg_rect_atlas* atlas, mg_image backingImage, str8 data, bool flip)
{
	mg_image_region imageRgn = {0};

	stbi_set_flip_vertically_on_load(flip ? 1 : 0);

	int width, height, channels;
	u8* pixels = stbi_load_from_memory((u8*)data.ptr, data.len, &width, &height, &channels, 4);
	if(pixels)
	{
		imageRgn = mg_image_atlas_alloc_from_rgba8(atlas, backingImage, width, height, pixels);
		free(pixels);
	}
	return(imageRgn);
}

mg_image_region mg_image_atlas_alloc_from_file(mg_rect_atlas* atlas, mg_image backingImage, str8 path, bool flip)
{
	mg_image_region imageRgn = {0};

	stbi_set_flip_vertically_on_load(flip ? 1 : 0);

	const char* cpath = str8_to_cstring(mem_scratch(), path);
	int width, height, channels;
	u8* pixels = stbi_load(cpath, &width, &height, &channels, 4);
	if(pixels)
	{
		imageRgn = mg_image_atlas_alloc_from_rgba8(atlas, backingImage, width, height, pixels);
		free(pixels);
	}
	return(imageRgn);
}

void mg_image_atlas_recycle(mg_rect_atlas* atlas, mg_image_region imageRgn)
{
	mg_rect_atlas_recycle(atlas, imageRgn.rect);
}

#undef LOG_SUBSYSTEM
