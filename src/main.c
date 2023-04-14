/************************************************************//**
*
*	@file: main.c
*	@author: Martin Fouilleul
*	@date: 11/04/2023
*
*****************************************************************/
#include<stdio.h>
#include<errno.h>
#include<pthread.h>
#include<math.h>

#include"wasm3.h"
#include"m3_env.h"
#include"m3_compile.h"

#define MG_INCLUDE_GL_API
#include"milepost.h"

#define LOG_SUBSYSTEM "Orca"


void log_string_flat(u64 len, char* ptr)
{
	LOG_MESSAGE("%.*s", (int)len, ptr);
}

void log_int(int i)
{
	printf("%i ", i);
}

void mg_matrix_push_flat(float a11, float a12, float a13,
                         float a21, float a22, float a23)
{
	mg_mat2x3 m = {a11, a12, a13, a21, a22, a23};
	mg_matrix_push(m);
}

mg_font mg_font_create_default()
{
	//NOTE(martin): create default font
	str8 fontPath = mp_app_get_resource_path(mem_scratch(), "../resources/OpenSansLatinSubset.ttf");
	char* fontPathCString = str8_to_cstring(mem_scratch(), fontPath);

	FILE* fontFile = fopen(fontPathCString, "r");
	if(!fontFile)
	{
		LOG_ERROR("Could not load font file '%s': %s\n", fontPathCString, strerror(errno));
		return(mg_font_nil());
	}
	unsigned char* fontData = 0;
	fseek(fontFile, 0, SEEK_END);
	u32 fontDataSize = ftell(fontFile);
	rewind(fontFile);
	fontData = (unsigned char*)malloc(fontDataSize);
	fread(fontData, 1, fontDataSize, fontFile);
	fclose(fontFile);

	unicode_range ranges[5] = {UNICODE_RANGE_BASIC_LATIN,
	                           UNICODE_RANGE_C1_CONTROLS_AND_LATIN_1_SUPPLEMENT,
	                           UNICODE_RANGE_LATIN_EXTENDED_A,
	                           UNICODE_RANGE_LATIN_EXTENDED_B,
	                           UNICODE_RANGE_SPECIALS};

	mg_font font = mg_font_create_from_memory(fontDataSize, fontData, 5, ranges);

	free(fontData);

	return(font);
}

#include"bindgen_core_api.c"
#include"canvas_api_bind.c"
#include"bindgen_gles_api.c"
#include"manual_gles_api.c"

typedef struct orca_app
{
	mp_window window;
	mg_surface surface;
	mg_surface mtlSurface;
	mg_canvas canvas;

} orca_app;

#define G_EVENTS(X) \
	X(G_EVENT_START, "OnInit", "", "") \
	X(G_EVENT_MOUSE_DOWN, "OnMouseDown", "", "i") \
	X(G_EVENT_MOUSE_UP, "OnMouseUp", "", "i") \
	X(G_EVENT_MOUSE_ENTER, "OnMouseEnter", "", "") \
	X(G_EVENT_MOUSE_LEAVE, "OnMouseLeave", "", "") \
	X(G_EVENT_MOUSE_MOVE, "OnMouseMove", "", "ffff") \
	X(G_EVENT_MOUSE_WHEEL, "OnMouseWheel", "", "ff") \
	X(G_EVENT_KEY_DOWN, "OnKeyDown", "", "i") \
	X(G_EVENT_KEY_UP, "OnKeyUp", "", "i") \
	X(G_EVENT_FRAME_REFRESH, "OnFrameRefresh", "", "") \
	X(G_EVENT_FRAME_RESIZE, "OnFrameResize", "", "ii")

typedef enum {
	#define G_EVENT_KIND(kind, ...) kind,
	G_EVENTS(G_EVENT_KIND)
	G_EVENT_COUNT
} guest_event_kind;


typedef struct g_event_handler_desc
{
	str8 name;
	str8 retTags;
	str8 argTags;
} g_event_handler_desc;

const g_event_handler_desc G_EVENT_HANDLER_DESC[] = {
	#define G_EVENT_HANDLER_DESC_ENTRY(kind, name, rets, args) {STR8(name), STR8(rets), STR8(args)},
	G_EVENTS(G_EVENT_HANDLER_DESC_ENTRY)
};

char m3_type_to_tag(M3ValueType type)
{
	switch(type)
	{
		case c_m3Type_none:
			return('v');
		case c_m3Type_i32:
			return('i');
		case c_m3Type_i64:
			return('I');
		case c_m3Type_f32:
			return('f');
		case c_m3Type_f64:
			return('d');

		case c_m3Type_unknown:
			return('!');
	}
}

typedef struct host_memory
{
	char* ptr;
	u32 reserved;
	u32 committed;

} host_memory;

void host_memory_init(host_memory* memory)
{
	mem_base_allocator* allocator = mem_base_allocator_default();
	memory->committed = 0;
	memory->reserved = 4<<20;
	memory->ptr = mem_base_reserve(allocator, memory->reserved);
}

void* host_memory_resize_callback(void* p, unsigned long size, void* userData)
{
	host_memory* memory = (host_memory*)userData;

	if(memory->committed >= size)
	{
		return(memory->ptr);
	}
	else if(memory->committed < memory->reserved)
	{
		u32 commitSize = size - memory->committed;

		mem_base_allocator* allocator = mem_base_allocator_default();
		mem_base_commit(allocator, memory->ptr + memory->committed, commitSize);
		memory->committed += commitSize;
		return(memory->ptr);
	}
	else
	{
		DEBUG_ASSERT(0, "Out of memory");
		return(0);
	}
}

void host_memory_free_callback(void* p, void* userData)
{
	host_memory* memory = (host_memory*)userData;

	mem_base_allocator* allocator = mem_base_allocator_default();
	mem_base_release(allocator, memory->ptr, memory->reserved);
	memset(memory, 0, sizeof(host_memory));
}

void* orca_runloop(void* user)
{
	orca_app* app = (orca_app*)user;
	mem_arena_init(mem_scratch());

	//NOTE: loads wasm module
	const char* bundleNameCString = "module";
	str8 modulePath = mp_app_get_resource_path(mem_scratch(), "../wasm/module.wasm");
	const char* modulePathCString = str8_to_cstring(mem_scratch(), modulePath);

	FILE* file = fopen(modulePathCString, "rb");
	if(!file)
	{
		LOG_ERROR("Couldn't load wasm module at %s\n", modulePathCString);
		return((void*)-1);
	}

	fseek(file, 0, SEEK_END);
	u64 wasmSize = ftell(file);
	rewind(file);

	u8* wasmBytes = malloc_array(u8, wasmSize);
	fread(wasmBytes, 1, wasmSize, file);
	fclose(file);

	u32 stackSize = 65536;
	IM3Environment env = m3_NewEnvironment();

	host_memory hostMemory = {};
	host_memory_init(&hostMemory);

	IM3Runtime runtime = m3_NewRuntime(env, stackSize, NULL);
	m3_RuntimeSetMemoryCallbacks(runtime, host_memory_resize_callback, host_memory_free_callback, &hostMemory);
	//NOTE: host memory will be freed when runtime is freed.

	IM3Module module = 0;

	//TODO check errors
	m3_ParseModule(env, &module, wasmBytes, wasmSize);
	m3_LoadModule(runtime, module);
	m3_SetModuleName(module, bundleNameCString);

	mem_scratch_clear();

	//NOTE: bind orca APIs
	bindgen_link_core_api(module);
	bindgen_link_canvas_api(module);
	bindgen_link_gles_api(module);
	manual_link_gles_api(module);

	//NOTE: compile
	M3Result res = m3_CompileModule(module);
	if(res)
	{
		M3ErrorInfo errInfo = {};
		m3_GetErrorInfo(runtime, &errInfo);

		LOG_ERROR("wasm error: %s\n", errInfo.message);
	}

	//NOTE: Find heap base
	u32 heapBase = 0;
	{
		IM3Global global = m3_FindGlobal(module, "__heap_base");
		if(global)
		{
			M3TaggedValue val;
			M3Result res = m3_GetGlobal(global, &val);
			if(!res && val.type == c_m3Type_i32)
			{
				heapBase = val.value.i32;
			}
			else
			{
				LOG_ERROR("couldn't get value of __heap_base\n");
				return((void*)-1);
			}
		}
		else
		{
			LOG_ERROR("couldn't locate __heap_base\n");
			return((void*)-1);
		}
	}
	//NOTE: align heap base on 16Bytes
	heapBase = AlignUpOnPow2(heapBase, 16);
	LOG_MESSAGE("mem_size = %u,  __heap_base = %u\n", m3_GetMemorySize(runtime), heapBase);

	//NOTE: Find and type check event handlers.
	IM3Function eventHandlers[G_EVENT_COUNT] = {0};
	for(int i=0; i<G_EVENT_COUNT; i++)
	{
		const g_event_handler_desc* desc = &G_EVENT_HANDLER_DESC[i];
		IM3Function handler = 0;
		m3_FindFunction(&handler, runtime, desc->name.ptr);

		if(handler)
		{
			bool checked = false;

			//NOTE: check function signature
			int retCount = m3_GetRetCount(handler);
			int argCount = m3_GetArgCount(handler);
			if(retCount == desc->retTags.len && argCount == desc->argTags.len)
			{
				checked = true;
				for(int retIndex = 0; retIndex < retCount; retIndex++)
				{
					M3ValueType m3Type = m3_GetRetType(handler, retIndex);
					char tag = m3_type_to_tag(m3Type);

					if(tag != desc->retTags.ptr[retIndex])
					{
						checked = false;
						break;
					}
				}
				if(checked)
				{
					for(int argIndex = 0; argIndex < argCount; argIndex++)
					{
						M3ValueType m3Type = m3_GetArgType(handler, argIndex);
						char tag = m3_type_to_tag(m3Type);

						if(tag != desc->argTags.ptr[argIndex])
						{
							checked = false;
							break;
						}
					}
				}
			}

			if(checked)
			{
				eventHandlers[i] = handler;
			}
			else
			{
				LOG_ERROR("type mismatch for event handler %.*s\n", (int)desc->name.len, desc->name.ptr);
			}
		}
	}

	//mg_canvas_prepare(app->canvas);

	//NOTE: prepare GL surface
	mg_surface_prepare(app->surface);

	//NOTE: call init handler
	if(eventHandlers[G_EVENT_START])
	{
		m3_Call(eventHandlers[G_EVENT_START], 0, 0);
	}

	if(eventHandlers[G_EVENT_FRAME_RESIZE])
	{
		mp_rect frame = mg_surface_get_frame(app->surface);
		u32 width = (u32)frame.w;
		u32 height = (u32)frame.h;
		const void* args[2] = {&width, &height};
		m3_Call(eventHandlers[G_EVENT_FRAME_RESIZE], 2, args);
	}

	while(!mp_should_quit())
	{
		mp_event event = {0};
		while(mp_next_event(&event))
		{
			switch(event.type)
			{
				case MP_EVENT_WINDOW_CLOSE:
				{
					mp_request_quit();
				} break;

				case MP_EVENT_WINDOW_RESIZE:
				{
					mp_rect frame = {0, 0, event.frame.rect.w, event.frame.rect.h};
					mg_surface_set_frame(app->surface, frame);

					if(eventHandlers[G_EVENT_FRAME_RESIZE])
					{
						u32 width = (u32)event.frame.rect.w;
						u32 height = (u32)event.frame.rect.h;
						const void* args[2] = {&width, &height};
						m3_Call(eventHandlers[G_EVENT_FRAME_RESIZE], 2, args);
					}
				} break;

				case MP_EVENT_MOUSE_BUTTON:
				{
					if(event.key.action == MP_KEY_PRESS)
					{
						if(eventHandlers[G_EVENT_MOUSE_DOWN])
						{
							int key = event.key.code;
							const void* args[1] = {&key};
							m3_Call(eventHandlers[G_EVENT_MOUSE_DOWN], 1, args);
						}
					}
					else
					{
						if(eventHandlers[G_EVENT_MOUSE_UP])
						{
							int key = event.key.code;
							const void* args[1] = {&key};
							m3_Call(eventHandlers[G_EVENT_MOUSE_UP], 1, args);
						}
					}
				} break;

				case MP_EVENT_MOUSE_MOVE:
				{
					if(eventHandlers[G_EVENT_MOUSE_MOVE])
					{
						const void* args[4] = {&event.move.x, &event.move.y, &event.move.deltaX, &event.move.deltaY};
						m3_Call(eventHandlers[G_EVENT_MOUSE_MOVE], 4, args);
					}
				} break;

				case MP_EVENT_KEYBOARD_KEY:
				{
					if(event.key.action == MP_KEY_PRESS)
					{
						if(eventHandlers[G_EVENT_KEY_DOWN])
						{
							const void* args[1] = {&event.key.code};
							m3_Call(eventHandlers[G_EVENT_KEY_DOWN], 1, args);
						}
					}
					else if(event.key.action == MP_KEY_RELEASE)
					{
						if(eventHandlers[G_EVENT_KEY_UP])
						{
							const void* args[1] = {&event.key.code};
							m3_Call(eventHandlers[G_EVENT_KEY_UP], 1, args);
						}
					}
				} break;

				default:
					break;
			}
		}

/*		mg_surface_prepare(app->surface);
			glClearColor(1, 0, 1, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			if(eventHandlers[G_EVENT_FRAME_REFRESH])
			{
				m3_Call(eventHandlers[G_EVENT_FRAME_REFRESH], 0, 0);
			}

		mg_surface_present(app->surface);
*/

		mg_canvas_prepare(app->canvas);

			if(eventHandlers[G_EVENT_FRAME_REFRESH])
			{
				m3_Call(eventHandlers[G_EVENT_FRAME_REFRESH], 0, 0);
			}

		mg_present();

		//TODO: update and render
		mem_scratch_clear();
	}

	return(0);
}

int main(int argc, char** argv)
{
	LogLevel(LOG_LEVEL_DEBUG);

	mp_init();
	mp_clock_init();

	mp_rect windowRect = {.x = 100, .y = 100, .w = 810, .h = 610};
	mp_window window = mp_window_create(windowRect, "orca", 0);

	//NOTE: create surface and canvas
	mg_surface surface = mg_surface_create_for_window(window, MG_BACKEND_GLES);
	mg_surface_swap_interval(surface, 1);
/*	mg_canvas canvas = mg_canvas_create(surface);

	if(mg_canvas_is_nil(canvas))
	{
		printf("Error: couldn't create canvas\n");
		return(-1);
	}
*/
	//NOTE: show window and start runloop
	mp_window_bring_to_front(window);
	mp_window_focus(window);

	mg_surface mtlSurface = mg_surface_create_for_window(window, MG_BACKEND_DEFAULT);
	mg_surface_swap_interval(mtlSurface, 1);
	mg_canvas canvas = mg_canvas_create(mtlSurface);

	orca_app app = {.window = window,
	                .surface = surface,
	                .mtlSurface = mtlSurface,
	                .canvas = canvas};

	pthread_t runloopThread;
	pthread_create(&runloopThread, 0, orca_runloop, &app);

	while(!mp_should_quit())
	{
		mp_pump_events(0);
		//TODO: what to do with mem scratch here?
	}

	void* res;
	pthread_join(runloopThread, &res);

//	mg_canvas_destroy(canvas);
//	mg_surface_destroy(surface);
	mp_window_destroy(window);

	mp_terminate();
	return(0);
}

#undef LOG_SUBSYSTEM
