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

#define MG_INCLUDE_GL_API
#include"milepost.h"

#include"orca_runtime.h"

#include"memory_impl.c"

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

void orca_assert(bool x)
{
	ASSERT(x);
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

	orca_runtime runtime;

} orca_app;

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

void orca_runtime_init(orca_runtime* runtime)
{
	memset(runtime, 0, sizeof(orca_runtime));
	mem_base_allocator* allocator = mem_base_allocator_default();
	runtime->wasmMemory.committed = 0;
	runtime->wasmMemory.reserved = 4ULL<<30;
	runtime->wasmMemory.ptr = mem_base_reserve(allocator, runtime->wasmMemory.reserved);
}

orca_app __orcaApp;

orca_runtime* orca_runtime_get()
{
	return(&__orcaApp.runtime);
}

void* orca_runloop(void* user)
{
	orca_app* app = &__orcaApp;

	orca_runtime_init(&app->runtime);

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

	app->runtime.wasmBytecode.len = wasmSize;
	app->runtime.wasmBytecode.ptr = malloc_array(char, wasmSize);
	fread(app->runtime.wasmBytecode.ptr, 1, app->runtime.wasmBytecode.len, file);
	fclose(file);

	u32 stackSize = 65536;
	app->runtime.m3Env = m3_NewEnvironment();

	app->runtime.m3Runtime = m3_NewRuntime(app->runtime.m3Env, stackSize, NULL);
	m3_RuntimeSetMemoryCallbacks(app->runtime.m3Runtime, wasm_memory_resize_callback, wasm_memory_free_callback, &app->runtime.wasmMemory);
	//NOTE: host memory will be freed when runtime is freed.

	//TODO check errors
	m3_ParseModule(app->runtime.m3Env, &app->runtime.m3Module, (u8*)app->runtime.wasmBytecode.ptr, app->runtime.wasmBytecode.len);
	m3_LoadModule(app->runtime.m3Runtime, app->runtime.m3Module);
	m3_SetModuleName(app->runtime.m3Module, bundleNameCString);

	mem_scratch_clear();

	//NOTE: bind orca APIs
	bindgen_link_core_api(app->runtime.m3Module);
	bindgen_link_canvas_api(app->runtime.m3Module);
	bindgen_link_gles_api(app->runtime.m3Module);
	manual_link_gles_api(app->runtime.m3Module);

	//NOTE: compile
	M3Result res = m3_CompileModule(app->runtime.m3Module);
	if(res)
	{
		M3ErrorInfo errInfo = {};
		m3_GetErrorInfo(app->runtime.m3Runtime, &errInfo);

		LOG_ERROR("wasm error: %s\n", errInfo.message);
		return((void*)-1);
	}

	//NOTE: Find heap base
	u32 heapBase = 0;
	{
		IM3Global global = m3_FindGlobal(app->runtime.m3Module, "__heap_base");
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
	LOG_MESSAGE("mem_size = %u,  __heap_base = %u\n", m3_GetMemorySize(app->runtime.m3Runtime), heapBase);

	//NOTE: Find and type check event handlers.

	for(int i=0; i<G_EVENT_COUNT; i++)
	{
		const g_event_handler_desc* desc = &G_EVENT_HANDLER_DESC[i];
		IM3Function handler = 0;
		m3_FindFunction(&handler, app->runtime.m3Runtime, desc->name.ptr);

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
				app->runtime.eventHandlers[i] = handler;
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

	IM3Function* eventHandlers = app->runtime.eventHandlers;

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

	__orcaApp = (orca_app){.window = window,
	                       .surface = surface,
	                       .mtlSurface = mtlSurface,
	                       .canvas = canvas};

	pthread_t runloopThread;
	pthread_create(&runloopThread, 0, orca_runloop, 0);

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
