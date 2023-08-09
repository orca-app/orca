/************************************************************//**
*
*	@file: graphics_common.c
*	@author: Martin Fouilleul
*	@date: 23/01/2023
*	@revision:
*
*****************************************************************/

#define _USE_MATH_DEFINES //NOTE: necessary for MSVC
#include<math.h>
#include"platform/platform.h"

#define STB_IMAGE_IMPLEMENTATION
#if PLATFORM_ORCA
	#define STBI_NO_STDIO
	#define STBI_NO_HDR
#endif
#include"stb/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include"stb/stb_truetype.h"

#include"platform/platform_debug.h"
#include"platform/platform_debug.h"
#include"graphics_common.h"

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

enum { MG_HANDLES_MAX_COUNT = 512 };

typedef struct mg_data
{
	bool init;

	mg_handle_slot handleArray[MG_HANDLES_MAX_COUNT];
	int handleNextIndex;
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

	vec4 shapeExtents;
	vec4 shapeScreenExtents;

} mg_canvas_data;

static mg_data __mgData = {0};


void mg_init()
{
	if(!__mgData.init)
	{
		__mgData.handleNextIndex = 0;
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

	mg_handle_slot* slot = list_pop_entry(&__mgData.handleFreeList, mg_handle_slot, freeListElt);
	if(!slot && __mgData.handleNextIndex < MG_HANDLES_MAX_COUNT)
	{
		slot = &__mgData.handleArray[__mgData.handleNextIndex];
		__mgData.handleNextIndex++;

		slot->generation = 1;
	}
	u64 h = 0;
	if(slot)
	{
		slot->kind = kind;
		slot->data = data;

		h = ((u64)(slot - __mgData.handleArray))<<32
		  |((u64)(slot->generation));
	}
	return(h);
}

void mg_handle_recycle(u64 h)
{
	DEBUG_ASSERT(__mgData.init);

	u32 index = h>>32;
	u32 generation = h & 0xffffffff;

	if(index*sizeof(mg_handle_slot) < __mgData.handleNextIndex)
	{
		mg_handle_slot* slot = &__mgData.handleArray[index];
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

	if(index < __mgData.handleNextIndex)
	{
		mg_handle_slot* slot = &__mgData.handleArray[index];
		if(  slot->generation == generation
		  && slot->kind == kind)
		{
			data = slot->data;
		}
	}
	return(data);
}

//------------------------------------------------------------------------------------------
//NOTE(martin): graphics canvas internal
//------------------------------------------------------------------------------------------

mp_thread_local mg_canvas_data* __mgCurrentCanvas = 0;
mp_thread_local mg_canvas __mgCurrentCanvasHandle = {0};

//TODO put these elsewhere
bool vec2_equal(vec2 v0, vec2 v1)
{
	return(v0.x == v1.x && v0.y == v1.y);
}

bool vec2_close(vec2 p0, vec2 p1, f32 tolerance)
{
	f32 norm2 = (p1.x - p0.x)*(p1.x - p0.x) + (p1.y - p0.y)*(p1.y - p0.y);
	return(fabs(norm2) < tolerance);
}

vec2 vec2_mul(f32 f, vec2 v)
{
	return((vec2){f*v.x, f*v.y});
}

vec2 vec2_add(vec2 v0, vec2 v1)
{
	return((vec2){v0.x + v1.x, v0.y + v1.y});
}

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
		log_error("matrix stack overflow\n");
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
		log_error("matrix stack underflow\n");
	}
	else
	{
		canvas->matrixStackSize--;
		mg_matrix_stack_top(canvas);
	}
}

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
		log_error("clip stack overflow\n");
	}
	else
	{
		canvas->clipStack[canvas->clipStackSize] = clip;
		canvas->clipStackSize++;
	}
}

void mg_clip_stack_pop(mg_canvas_data* canvas)
{
	if(canvas->clipStackSize == 0)
	{
		log_error("clip stack underflow\n");
	}
	else
	{
		canvas->clipStackSize--;
	}
}

void mg_push_command(mg_canvas_data* canvas, mg_primitive primitive)
{
	//NOTE(martin): push primitive and updates current stream, eventually patching a pending jump.
	ASSERT(canvas->primitiveCount < MG_MAX_PRIMITIVE_COUNT);

	canvas->primitives[canvas->primitiveCount] = primitive;
	canvas->primitives[canvas->primitiveCount].attributes = canvas->attributes;
	canvas->primitives[canvas->primitiveCount].attributes.transform = mg_matrix_stack_top(canvas);
	canvas->primitives[canvas->primitiveCount].attributes.clip = mg_clip_stack_top(canvas);
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
	ASSERT(canvas->path.count + canvas->path.startIndex + count < MG_MAX_PATH_ELEMENT_COUNT);
	memcpy(canvas->pathElements + canvas->path.startIndex + canvas->path.count, elements, count*sizeof(mg_path_elt));
	canvas->path.count += count;

	ASSERT(canvas->path.count < MG_MAX_PATH_ELEMENT_COUNT);
}

void mg_path_push_element(mg_canvas_data* canvas, mg_path_elt elt)
{
	mg_path_push_elements(canvas, 1, &elt);
}

//------------------------------------------------------------------------------------------
//NOTE(martin): fonts
//------------------------------------------------------------------------------------------

mg_font mg_font_nil() { return((mg_font){.h = 0}); }
bool mg_font_is_nil(mg_font font) { return(font.h == 0); }

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

mg_canvas mg_canvas_create()
{
	if(!__mgData.init)
	{
		mg_init();
	}

	mg_canvas canvasHandle = mg_canvas_nil();
	mg_canvas_data* canvas = list_pop_entry(&__mgData.canvasFreeList, mg_canvas_data, freeListElt);
	if(!canvas)
	{
		canvas = mem_arena_alloc_type(&__mgData.resourceArena, mg_canvas_data);
	}
	if(canvas)
	{
		canvas->textFlip = false;
		canvas->path = (mg_path_descriptor){0};
		canvas->matrixStackSize = 0;
		canvas->clipStackSize = 0;
		canvas->primitiveCount = 0;
		canvas->clearColor = (mg_color){0, 0, 0, 0};

		canvas->attributes = (mg_attributes){0};
		canvas->attributes.color = (mg_color){0, 0, 0, 1};
		canvas->attributes.tolerance = 1;
		canvas->attributes.width = 10;
		canvas->attributes.clip = (mp_rect){-FLT_MAX/2, -FLT_MAX/2, FLT_MAX, FLT_MAX};

		canvasHandle = mg_canvas_handle_alloc(canvas);

		mg_canvas_set_current(canvasHandle);
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

void mg_render(mg_surface surface, mg_canvas canvas)
{
	mg_canvas_data* canvasData = mg_canvas_data_from_handle(canvas);
	if(canvasData)
	{
		int eltCount = canvasData->path.startIndex + canvasData->path.count;
		mg_surface_render_commands(surface,
		                           canvasData->clearColor,
		                           canvasData->primitiveCount,
		                           canvasData->primitives,
		                           eltCount,
		                           canvasData->pathElements);

		canvasData->primitiveCount = 0;
		canvasData->path.startIndex = 0;
		canvasData->path.count = 0;
	}
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
		mp_rect clip = {x, y, w, h};

		//NOTE(martin): transform clip
		mg_mat2x3 transform = mg_matrix_stack_top(canvas);
		vec2 p0 = mg_mat2x3_mul(transform, (vec2){clip.x, clip.y});
		vec2 p1 = mg_mat2x3_mul(transform, (vec2){clip.x + clip.w, clip.y});
		vec2 p2 = mg_mat2x3_mul(transform, (vec2){clip.x + clip.w, clip.y + clip.h});
		vec2 p3 = mg_mat2x3_mul(transform, (vec2){clip.x, clip.y + clip.h});

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

		canvas->attributes.clip = r;
	}
}

void mg_clip_pop()
{
	mg_canvas_data* canvas = __mgCurrentCanvas;
	if(canvas)
	{
		mg_clip_stack_pop(canvas);
		canvas->attributes.clip = mg_clip_stack_top(canvas);
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
			log_warning("code point is not present in font ranges\n");
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
//NOTE(martin): simple shape helpers
//------------------------------------------------------------------------------------------

void mg_rectangle_path(f32 x, f32 y, f32 w, f32 h)
{
	mg_move_to(x, y);
	mg_line_to(x+w, y);
	mg_line_to(x+w, y+h);
	mg_line_to(x, y+h);
	mg_close_path();
}

void mg_rectangle_fill(f32 x, f32 y, f32 w, f32 h)
{
	mg_rectangle_path(x, y, w, h);
	mg_fill();
}

void mg_rectangle_stroke(f32 x, f32 y, f32 w, f32 h)
{
	mg_rectangle_path(x, y, w, h);
	mg_stroke();
}

void mg_rounded_rectangle_path(f32 x, f32 y, f32 w, f32 h, f32 r)
{
	f32 c = r*4*(sqrt(2)-1)/3;

	mg_move_to(x+r, y);
	mg_line_to(x+w-r, y);
	mg_cubic_to(x+w-r+c, y, x+w, y+r-c, x+w, y+r);
	mg_line_to(x+w, y+h-r);
	mg_cubic_to(x+w, y+h-r+c, x+w-r+c, y+h, x+w-r, y+h);
	mg_line_to(x+r, y+h);
	mg_cubic_to(x+r-c, y+h, x, y+h-r+c, x, y+h-r);
	mg_line_to(x, y+r);
	mg_cubic_to(x, y+r-c, x+r-c, y, x+r, y);
}

void mg_rounded_rectangle_fill(f32 x, f32 y, f32 w, f32 h, f32 r)
{
	mg_rounded_rectangle_path(x, y, w, h, r);
	mg_fill();
}

void mg_rounded_rectangle_stroke(f32 x, f32 y, f32 w, f32 h, f32 r)
{
	mg_rounded_rectangle_path(x, y, w, h, r);
	mg_stroke();
}

void mg_ellipse_path(f32 x, f32 y, f32 rx, f32 ry)
{
	f32 cx = rx*4*(sqrt(2)-1)/3;
	f32 cy = ry*4*(sqrt(2)-1)/3;

	mg_move_to(x-rx, y);
	mg_cubic_to(x-rx, y+cy, x-cx, y+ry, x, y+ry);
	mg_cubic_to(x+cx, y+ry, x+rx, y+cy, x+rx, y);
	mg_cubic_to(x+rx, y-cy, x+cx, y-ry, x, y-ry);
	mg_cubic_to(x-cx, y-ry, x-rx, y-cy, x-rx, y);
}

void mg_ellipse_fill(f32 x, f32 y, f32 rx, f32 ry)
{
	mg_ellipse_path(x, y, rx, ry);
	mg_fill();
}

void mg_ellipse_stroke(f32 x, f32 y, f32 rx, f32 ry)
{
	mg_ellipse_path(x, y, rx, ry);
	mg_stroke();
}

void mg_circle_fill(f32 x, f32 y, f32 r)
{
	mg_ellipse_fill(x, y, r, r);
}

void mg_circle_stroke(f32 x, f32 y, f32 r)
{
	mg_ellipse_stroke(x, y, r, r);
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

mg_image mg_image_create_from_rgba8(mg_surface surface, u32 width, u32 height, u8* pixels)
{
	mg_image image = mg_image_create(surface, width, height);
	if(!mg_image_is_nil(image))
	{
		mg_image_upload_region_rgba8(image, (mp_rect){0, 0, width, height}, pixels);
	}
	return(image);
}

mg_image mg_image_create_from_data(mg_surface surface, str8 data, bool flip)
{
	mg_image image = mg_image_nil();
	int width, height, channels;

	stbi_set_flip_vertically_on_load(flip ? 1 : 0);
	u8* pixels = stbi_load_from_memory((u8*)data.ptr, data.len, &width, &height, &channels, 4);

	if(pixels)
	{
		image = mg_image_create_from_rgba8(surface, width, height, pixels);
		free(pixels);
	}
	else
	{
		log_error("stbi_load_from_memory() failed: %s\n", stbi_failure_reason());
	}
	return(image);
}

#if !PLATFORM_ORCA

mg_image mg_image_create_from_file(mg_surface surface, str8 path, bool flip)
{
	mg_image image = mg_image_nil();
	int width, height, channels;

	const char* cpath = str8_to_cstring(mem_scratch(), path);

	stbi_set_flip_vertically_on_load(flip ? 1 : 0);
	u8* pixels = stbi_load(cpath, &width, &height, &channels, 4);
	if(pixels)
	{
		image = mg_image_create_from_rgba8(surface, width, height, pixels);
		free(pixels);
	}
	else
	{
		 log_error("stbi_load() failed: %s\n", stbi_failure_reason());
	}
	return(image);
}

#endif // !PLATFORM_ORCA


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

		mg_move_to(dstRegion.x, dstRegion.y);
		mg_line_to(dstRegion.x+dstRegion.w, dstRegion.y);
		mg_line_to(dstRegion.x+dstRegion.w, dstRegion.y+dstRegion.h);
		mg_line_to(dstRegion.x, dstRegion.y+dstRegion.h);
		mg_close_path();

		mg_fill();

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

#if !PLATFORM_ORCA

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
#endif // !PLATFORM_ORCA

void mg_image_atlas_recycle(mg_rect_atlas* atlas, mg_image_region imageRgn)
{
	mg_rect_atlas_recycle(atlas, imageRgn.rect);
}
