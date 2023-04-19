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

/*
void orca_log(int len, const char* ptr)
{
	log_info("%.*s", len, ptr);
}
*/

void orca_log_entry(log_level level,
                    int fileLen,
                    char* file,
                    int functionLen,
                    char* function,
                    int line,
                    int msgLen,
                    char* msg)
{
	log_entry(level,
	          str8_from_buffer(fileLen, file),
	          str8_from_buffer(functionLen, function),
	          line,
	          "%.*s\n",
	          msgLen,
	          msg);
}

void mg_matrix_push_flat(float a11, float a12, float a13,
                         float a21, float a22, float a23)
{
	mg_mat2x3 m = {a11, a12, a13, a21, a22, a23};
	mg_matrix_push(m);
}

int orca_assert(const char* file, const char* function, int line, const char* src, const char* note)
{
	mem_arena* scratch = mem_scratch();
	str8 msg = str8_pushf(scratch,
	                      "Assertion failed in function %s() in file \"%s\", line %i:\n%s\nNote: %s\n",
	                      function,
	                      file,
	                      line,
	                      src,
	                      note);

	const char* msgCStr = str8_to_cstring(scratch, msg);
	log_error(msgCStr);

	const char* options[] = {"OK"};
	mp_alert_popup("Assertion Failed", msgCStr, 1, options);

	//TODO: should terminate more gracefully...
	exit(-1);
}

mg_font orca_font_create(const char* resourcePath)
{
	//NOTE(martin): create default font
	str8 fontPath = mp_app_get_resource_path(mem_scratch(), resourcePath);
	char* fontPathCString = str8_to_cstring(mem_scratch(), fontPath);

	FILE* fontFile = fopen(fontPathCString, "r");
	if(!fontFile)
	{
		log_error("Could not load font file '%s': %s\n", fontPathCString, strerror(errno));
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

mg_font mg_font_create_default()
{
	return(orca_font_create("../resources/OpenSansLatinSubset.ttf"));
}


#include"bindgen_core_api.c"
#include"canvas_api_bind.c"
#include"bindgen_gles_api.c"
#include"manual_gles_api.c"

typedef struct orca_debug_overlay
{
	bool show;
	mg_surface surface;
	mg_canvas canvas;
	mg_font font;
	ui_context ui;

} orca_debug_overlay;

void debug_overlay_toogle(orca_debug_overlay* overlay)
{
	overlay->show = !overlay->show;
	mg_surface_set_hidden(overlay->surface, !overlay->show);
}

typedef struct orca_app
{
	mp_window window;
	mg_surface surface;
	mg_canvas canvas;

	orca_runtime runtime;

	orca_debug_overlay debugOverlay;

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
		log_error("Couldn't load wasm module at %s\n", modulePathCString);
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

		log_error("wasm error: %s\n", errInfo.message);
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
				log_error("couldn't get value of __heap_base\n");
				return((void*)-1);
			}
		}
		else
		{
			log_error("couldn't locate __heap_base\n");
			return((void*)-1);
		}
	}
	//NOTE: align heap base on 16Bytes
	heapBase = AlignUpOnPow2(heapBase, 16);
	log_info("mem_size = %u,  __heap_base = %u\n", m3_GetMemorySize(app->runtime.m3Runtime), heapBase);

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
				log_error("type mismatch for event handler %.*s\n", (int)desc->name.len, desc->name.ptr);
			}
		}
	}

	//NOTE: setup ui context

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

	ui_set_context(&app->debugOverlay.ui);

	while(!mp_should_quit())
	{
		mp_event* event = 0;
		while((event = mp_next_event(mem_scratch())) != 0)
		{
			if(app->debugOverlay.show)
			{
				ui_process_event(event);
			}

			switch(event->type)
			{
				case MP_EVENT_WINDOW_CLOSE:
				{
					mp_request_quit();
				} break;

				case MP_EVENT_WINDOW_RESIZE:
				{
					mp_rect frame = {0, 0, event->frame.rect.w, event->frame.rect.h};
					mg_surface_set_frame(app->surface, frame);

					if(eventHandlers[G_EVENT_FRAME_RESIZE])
					{
						u32 width = (u32)event->frame.rect.w;
						u32 height = (u32)event->frame.rect.h;
						const void* args[2] = {&width, &height};
						m3_Call(eventHandlers[G_EVENT_FRAME_RESIZE], 2, args);
					}
				} break;

				case MP_EVENT_MOUSE_BUTTON:
				{
					if(event->key.action == MP_KEY_PRESS)
					{
						if(eventHandlers[G_EVENT_MOUSE_DOWN])
						{
							int key = event->key.code;
							const void* args[1] = {&key};
							m3_Call(eventHandlers[G_EVENT_MOUSE_DOWN], 1, args);
						}
					}
					else
					{
						if(eventHandlers[G_EVENT_MOUSE_UP])
						{
							int key = event->key.code;
							const void* args[1] = {&key};
							m3_Call(eventHandlers[G_EVENT_MOUSE_UP], 1, args);
						}
					}
				} break;

				case MP_EVENT_MOUSE_MOVE:
				{
					if(eventHandlers[G_EVENT_MOUSE_MOVE])
					{
						const void* args[4] = {&event->move.x, &event->move.y, &event->move.deltaX, &event->move.deltaY};
						m3_Call(eventHandlers[G_EVENT_MOUSE_MOVE], 4, args);
					}
				} break;

				case MP_EVENT_KEYBOARD_KEY:
				{
					if(event->key.action == MP_KEY_PRESS)
					{
						if(event->key.code == MP_KEY_D && (event->key.mods & (MP_KEYMOD_SHIFT | MP_KEYMOD_CMD)))
						{
							debug_overlay_toogle(&app->debugOverlay);
						}

						if(eventHandlers[G_EVENT_KEY_DOWN])
						{
							const void* args[1] = {&event->key.code};
							m3_Call(eventHandlers[G_EVENT_KEY_DOWN], 1, args);
						}
					}
					else if(event->key.action == MP_KEY_RELEASE)
					{
						if(eventHandlers[G_EVENT_KEY_UP])
						{
							const void* args[1] = {&event->key.code};
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

		if(app->debugOverlay.show)
		{
			ui_style debugUIDefaultStyle = {.bgColor = {0},
			                                .color = {1, 1, 1, 1},
			                                .font = app->debugOverlay.font,
			                                .fontSize = 16,
			                                .borderColor = {1, 0, 0, 1},
			                                .borderSize = 2};

			ui_style_mask debugUIDefaultMask = UI_STYLE_BG_COLOR
			                                 | UI_STYLE_COLOR
			                                 | UI_STYLE_BORDER_COLOR
			                                 | UI_STYLE_BORDER_SIZE
			                                 | UI_STYLE_FONT
			                                 | UI_STYLE_FONT_SIZE;

			ui_frame(&debugUIDefaultStyle, debugUIDefaultMask)
			{
				ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 1},
				                          .size.height = {UI_SIZE_PARENT, 1, 1}},
				               UI_STYLE_SIZE);

				ui_container("overlay area", 0)
				{
					//...
				}

				ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 1},
				                          .size.height = {UI_SIZE_PARENT, 0.4},
				                          .bgColor = {0, 0, 0, 0.5}},
				               UI_STYLE_SIZE
				              |UI_STYLE_BG_COLOR);

				ui_container("log console", UI_FLAG_DRAW_BACKGROUND)
				{
					ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 1},
					                          .size.height = {UI_SIZE_PARENT, 1}},
					              UI_STYLE_SIZE);

					ui_panel("log console", UI_FLAG_CLICKABLE)
					{
						ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 1},
						                          .size.height = {UI_SIZE_PIXELS, 800}},
						              UI_STYLE_SIZE);

						ui_container("contents", 0)
						{
						}
					}
				}
			}

			mg_canvas_prepare(app->debugOverlay.canvas);

				ui_draw();
			/*
				mg_set_font(app->debugOverlay.font);
				mg_set_font_size(32);
				mg_set_color_rgba(0.2, 0.2, 0.2, 1);
				mg_move_to(30, 30);
				mg_text_outlines(STR8("Debug Overlay"));
				mg_fill();
			*/
			mg_present();
		}

		mem_scratch_clear();
	}

	return(0);
}

int main(int argc, char** argv)
{
	log_set_level(LOG_LEVEL_INFO);

	mp_init();
	mp_clock_init();

	orca_app* orca = &__orcaApp;

	mp_rect windowRect = {.x = 100, .y = 100, .w = 810, .h = 610};
	orca->window = mp_window_create(windowRect, "orca", 0);
	orca->surface = mg_surface_create_for_window(orca->window, MG_BACKEND_DEFAULT);
	orca->canvas = mg_canvas_create(orca->surface);

	mg_surface_swap_interval(orca->surface, 1);

	orca->debugOverlay.show = false;
	orca->debugOverlay.surface = mg_surface_create_for_window(orca->window, MG_BACKEND_DEFAULT);
	orca->debugOverlay.canvas = mg_canvas_create(orca->debugOverlay.surface);
	orca->debugOverlay.font = orca_font_create("../resources/Andale Mono.ttf");

	mg_surface_set_hidden(orca->debugOverlay.surface, true);

	//WARN: this is a workaround to avoid stalling the first few times we acquire drawables from
	//      the surfaces... This should probably be fixed in the implementation of mtl_surface!
	for(int i=0; i<4; i++)
	{
		mg_canvas_prepare(orca->canvas);
		mg_present();
		mg_canvas_prepare(orca->debugOverlay.canvas);
		mg_present();
	}

	ui_init(&orca->debugOverlay.ui);

	//NOTE: show window and start runloop
	mp_window_bring_to_front(orca->window);
	mp_window_focus(orca->window);

	pthread_t runloopThread;
	pthread_create(&runloopThread, 0, orca_runloop, 0);

	while(!mp_should_quit())
	{
		mp_pump_events(0);
		//TODO: what to do with mem scratch here?
	}

	void* res;
	pthread_join(runloopThread, &res);

	mg_canvas_destroy(orca->canvas);
	mg_surface_destroy(orca->surface);
	mp_window_destroy(orca->window);

	mp_terminate();
	return(0);
}

#undef LOG_SUBSYSTEM
