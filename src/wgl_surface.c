/************************************************************//**
*
*	@file: wgl_surface.c
*	@author: Martin Fouilleul
*	@date: 01/08/2022
*	@revision:
*
*****************************************************************/
#include"win32_app.h"
#include"graphics_internal.h"
#include"gl_loader.h"

#include<GL/wglext.h>
#include"macro_helpers.h"

#define WGL_PROC_LIST \
	WGL_PROC(WGLCHOOSEPIXELFORMATARB, wglChoosePixelFormatARB) \
	WGL_PROC(WGLCREATECONTEXTATTRIBSARB, wglCreateContextAttribsARB) \
	WGL_PROC(WGLMAKECONTEXTCURRENTARB, wglMakeContextCurrentARB) \
	WGL_PROC(WGLSWAPINTERVALEXT, wglSwapIntervalEXT) \

//NOTE: wgl function pointers declarations

#define WGL_PROC(type, name) _cat3_(PFN, type, PROC) name = 0;
	WGL_PROC_LIST
#undef WGL_PROC

//NOTE: wgl loader

typedef struct wgl_dummy_context
{
	bool init;
	HWND hWnd;
	HDC hDC;
	HGLRC glContext;

} wgl_dummy_context;

static wgl_dummy_context __mgWGLDummyContext = {0};

static void wgl_init()
{
	if(!__mgWGLDummyContext.init)
	{
		//NOTE: create a dummy window
		WNDCLASS windowClass = {.style = CS_OWNDC,
		                        .lpfnWndProc = DefWindowProc,
		                        .hInstance = GetModuleHandleW(NULL),
		                        .lpszClassName = "wgl_helper_window_class",
		                        .hCursor = LoadCursor(0, IDC_ARROW)};

		if(!RegisterClass(&windowClass))
		{
			//TODO: error
			goto quit;
		}

		__mgWGLDummyContext.hWnd = CreateWindow("wgl_helper_window_class",
		                                        "dummy",
		                                        WS_OVERLAPPEDWINDOW,
	                                            0, 0, 100, 100,
	                                            0, 0, windowClass.hInstance, 0);

		if(!__mgWGLDummyContext.hWnd)
		{
			//TODO: error
			goto quit;
		}
		__mgWGLDummyContext.hDC = GetDC(__mgWGLDummyContext.hWnd);

		PIXELFORMATDESCRIPTOR pixelFormatDesc =
		{
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    // Flags
			PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
			32,                   // Colordepth of the framebuffer.
			0, 0, 0, 0, 0, 0,
			0,
			0,
			0,
			0, 0, 0, 0,
			24,                   // Number of bits for the depthbuffer
			8,                    // Number of bits for the stencilbuffer
			0,                    // Number of Aux buffers in the framebuffer.
			PFD_MAIN_PLANE,
			0,
			0, 0, 0
		};

		int pixelFormat = ChoosePixelFormat(__mgWGLDummyContext.hDC, &pixelFormatDesc);
		SetPixelFormat(__mgWGLDummyContext.hDC, pixelFormat, &pixelFormatDesc);

		__mgWGLDummyContext.glContext = wglCreateContext(__mgWGLDummyContext.hDC);
		wglMakeCurrent(__mgWGLDummyContext.hDC, __mgWGLDummyContext.glContext);

		//NOTE(martin): now load WGL extension functions
		#define WGL_PROC(type, name) name = (_cat3_(PFN, type, PROC))wglGetProcAddress( #name );
			WGL_PROC_LIST
		#undef WGL_PROC

		__mgWGLDummyContext.init = true;
	}
	else
	{
		wglMakeCurrent(__mgWGLDummyContext.hDC, __mgWGLDummyContext.glContext);
	}
	quit:;
}

#undef WGL_PROC_LIST


typedef struct mg_wgl_surface
{
	mg_surface_data interface;

	mp_layer layer;
	HDC hDC;
	HGLRC glContext;

	//NOTE: this may be a bit wasteful to have one api struct per surface, but win32 docs says that loading procs
	//      from different contexts might select different implementations (eg. depending on context version/pixel format)
	mg_gl_api api;
} mg_wgl_surface;

void mg_wgl_surface_destroy(mg_surface_data* interface)
{
	mg_wgl_surface* surface = (mg_wgl_surface*)interface;

	if(surface->glContext == wglGetCurrentContext())
	{
		wglMakeCurrent(NULL, NULL);
	}
	wglDeleteContext(surface->glContext);

	mp_layer_cleanup(&surface->layer);

	free(surface);
}

void mg_wgl_surface_prepare(mg_surface_data* interface)
{
	mg_wgl_surface* surface = (mg_wgl_surface*)interface;

	wglMakeCurrent(surface->hDC, surface->glContext);
	mg_gl_select_api(&surface->api);
}

void mg_wgl_surface_present(mg_surface_data* interface)
{
	mg_wgl_surface* surface = (mg_wgl_surface*)interface;
	SwapBuffers(surface->hDC);
}

void mg_wgl_surface_swap_interval(mg_surface_data* interface, int swap)
{
	mg_wgl_surface* surface = (mg_wgl_surface*)interface;
	wglSwapIntervalEXT(swap);
}

vec2 mg_wgl_surface_contents_scaling(mg_surface_data* interface)
{
	mg_wgl_surface* surface = (mg_wgl_surface*)interface;
	return(mp_layer_contents_scaling(&surface->layer));
}

mp_rect mg_wgl_surface_get_frame(mg_surface_data* interface)
{
	mg_wgl_surface* surface = (mg_wgl_surface*)interface;
	return(mp_layer_get_frame(&surface->layer));
}

void mg_wgl_surface_set_frame(mg_surface_data* interface, mp_rect frame)
{
	mg_wgl_surface* surface = (mg_wgl_surface*)interface;
	mp_layer_set_frame(&surface->layer, frame);
}

void mg_wgl_surface_set_hidden(mg_surface_data* interface, bool hidden)
{
	mg_wgl_surface* surface = (mg_wgl_surface*)interface;
	mp_layer_set_hidden(&surface->layer, hidden);
}

bool mg_wgl_surface_get_hidden(mg_surface_data* interface)
{
	mg_wgl_surface* surface = (mg_wgl_surface*)interface;
	return(mp_layer_get_hidden(&surface->layer));
}

void* mg_wgl_surface_native_layer(mg_surface_data* interface)
{
	mg_wgl_surface* surface = (mg_wgl_surface*)interface;
	return(mp_layer_native_surface(&surface->layer));
}

void* mg_wgl_get_proc(const char* name)
{
	void* p = wglGetProcAddress(name);
	if(  p == 0
	  || p == (void*)0x01
	  || p == (void*)0x02
	  || p == (void*)0x03
	  || p == (void*)(-1))
	{
		//TODO: should we avoid re-loading every time?
		HMODULE module = LoadLibrary("opengl32.dll");
		p = (void*)GetProcAddress(module, name);
	}
	return(p);
}

mg_surface_data* mg_wgl_surface_create_for_window(mp_window window)
{
	mg_surface* surface = 0;

	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		wgl_init();

		//NOTE: fill surface data and load api
		mg_wgl_surface* surface = malloc_type(mg_wgl_surface);
		if(surface)
		{
			surface->interface.backend = MG_BACKEND_GL;
			surface->interface.destroy = mg_wgl_surface_destroy;
			surface->interface.prepare = mg_wgl_surface_prepare;
			surface->interface.present = mg_wgl_surface_present;
			surface->interface.swapInterval = mg_wgl_surface_swap_interval;
			surface->interface.contentsScaling = mg_wgl_surface_contents_scaling;
			surface->interface.getFrame = mg_wgl_surface_get_frame;
			surface->interface.setFrame = mg_wgl_surface_set_frame;
			surface->interface.getHidden = mg_wgl_surface_get_hidden;
			surface->interface.setHidden = mg_wgl_surface_set_hidden;

			mp_layer_init_for_window(&surface->layer, windowData);
			surface->hDC = GetDC(surface->layer.hWnd);

			//NOTE(martin): create the pixel format and gl context
			PIXELFORMATDESCRIPTOR pixelFormatDesc =
			{
				sizeof(PIXELFORMATDESCRIPTOR),
				1,
				PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    // Flags
				PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
				32,                   // Colordepth of the framebuffer.
				0, 0, 0, 0, 0, 0,
				0,
				0,
				0,
				0, 0, 0, 0,
				24,                   // Number of bits for the depthbuffer
				8,                    // Number of bits for the stencilbuffer
				0,                    // Number of Aux buffers in the framebuffer.
				PFD_MAIN_PLANE,
				0,
				0, 0, 0
			};

			int pixelFormatAttrs[] = {
				WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
				WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
				WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
				WGL_COLOR_BITS_ARB, 32,
				WGL_DEPTH_BITS_ARB, 24,
				WGL_STENCIL_BITS_ARB, 8,
				0};

			u32 numFormats = 0;
			int pixelFormat = 0;

			wglChoosePixelFormatARB(surface->hDC, pixelFormatAttrs, 0, 1, &pixelFormat, &numFormats);

			if(!pixelFormat)
			{
				//TODO: error
			}
			SetPixelFormat(surface->hDC, pixelFormat, &pixelFormatDesc);

			int contextAttrs[] = {
				WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
				WGL_CONTEXT_MINOR_VERSION_ARB, 3,
				WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
				0};

			surface->glContext = wglCreateContextAttribsARB(surface->hDC, __mgWGLDummyContext.glContext, contextAttrs);

			if(!surface->glContext)
			{
				//TODO error
				int error = GetLastError();
				printf("error: %i\n", error);
			}

			//NOTE: make gl context current and load api
			wglMakeCurrent(surface->hDC, surface->glContext);
			wglSwapIntervalEXT(1);
			mg_gl_load_gl43(&surface->api, mg_wgl_get_proc);
		}
	}
	return((mg_surface_data*)surface);
}
