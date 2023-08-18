/************************************************************//**
*
*	@file: wgl_surface.c
*	@author: Martin Fouilleul
*	@date: 01/08/2022
*	@revision:
*
*****************************************************************/
#include"app/win32_app.h"
#include"graphics_surface.h"
#include"gl_loader.h"

#include<GL/wglext.h>
#include"util/macros.h"

#define OC_WGL_PROC_LIST \
	OC_WGL_PROC(WGLCHOOSEPIXELFORMATARB, wglChoosePixelFormatARB) \
	OC_WGL_PROC(WGLCREATECONTEXTATTRIBSARB, wglCreateContextAttribsARB) \
	OC_WGL_PROC(WGLMAKECONTEXTCURRENTARB, wglMakeContextCurrentARB) \
	OC_WGL_PROC(WGLSWAPINTERVALEXT, wglSwapIntervalEXT) \

//NOTE: wgl function pointers declarations

#define OC_WGL_PROC(type, name) OC_CAT3(PFN, type, PROC) name = 0;
	OC_WGL_PROC_LIST
#undef OC_WGL_PROC

//NOTE: wgl loader

typedef struct oc_wgl_dummy_context
{
	bool init;
	HWND hWnd;
	HDC hDC;
	HGLRC glContext;

} oc_wgl_dummy_context;

static oc_wgl_dummy_context oc_wglDummyContext = {0};

static void oc_wgl_init()
{
	if(!oc_wglDummyContext.init)
	{
		//NOTE: create a dummy window
		WNDCLASS windowClass = {.style = CS_OWNDC,
		                        .lpfnWndProc = DefWindowProc,
		                        .hInstance = GetModuleHandleW(NULL),
		                        .lpszClassName = "oc_wgl_helper_window_class",
		                        .hCursor = LoadCursor(0, IDC_ARROW)};

		if(!RegisterClass(&windowClass))
		{
			//TODO: error
			goto quit;
		}

		oc_wglDummyContext.hWnd = CreateWindow("oc_wgl_helper_window_class",
		                                        "dummy",
		                                        WS_OVERLAPPEDWINDOW,
	                                            0, 0, 100, 100,
	                                            0, 0, windowClass.hInstance, 0);

		if(!oc_wglDummyContext.hWnd)
		{
			//TODO: error
			goto quit;
		}
		oc_wglDummyContext.hDC = GetDC(oc_wglDummyContext.hWnd);

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

		int pixelFormat = ChoosePixelFormat(oc_wglDummyContext.hDC, &pixelFormatDesc);
		SetPixelFormat(oc_wglDummyContext.hDC, pixelFormat, &pixelFormatDesc);

		oc_wglDummyContext.glContext = wglCreateContext(oc_wglDummyContext.hDC);
		wglMakeCurrent(oc_wglDummyContext.hDC, oc_wglDummyContext.glContext);

		//NOTE(martin): now load WGL extension functions
		#define OC_WGL_PROC(type, name) name = (OC_CAT3(PFN, type, PROC))wglGetProcAddress( #name );
			OC_WGL_PROC_LIST
		#undef OC_WGL_PROC

		oc_wglDummyContext.init = true;
	}
	else
	{
		wglMakeCurrent(oc_wglDummyContext.hDC, oc_wglDummyContext.glContext);
	}
	quit:;
}

#undef OC_WGL_PROC_LIST


typedef struct oc_wgl_surface
{
	oc_surface_data interface;

	HDC hDC;
	HGLRC glContext;

	//NOTE: this may be a bit wasteful to have one api struct per surface, but win32 docs says that loading procs
	//      from different contexts might select different implementations (eg. depending on context version/pixel format)
	oc_gl_api api;
} oc_wgl_surface;

void oc_wgl_surface_destroy(oc_surface_data* interface)
{
	oc_wgl_surface* surface = (oc_wgl_surface*)interface;

	if(surface->glContext == wglGetCurrentContext())
	{
		wglMakeCurrent(NULL, NULL);
	}
	wglDeleteContext(surface->glContext);

	oc_surface_cleanup(interface);

	free(surface);
}

void oc_wgl_surface_prepare(oc_surface_data* interface)
{
	oc_wgl_surface* surface = (oc_wgl_surface*)interface;

	wglMakeCurrent(surface->hDC, surface->glContext);
	oc_gl_select_api(&surface->api);
}

void oc_wgl_surface_present(oc_surface_data* interface)
{
	oc_wgl_surface* surface = (oc_wgl_surface*)interface;

	SwapBuffers(surface->hDC);
}

void oc_wgl_surface_deselect(oc_surface_data* interface)
{
	wglMakeCurrent(NULL, NULL);
	oc_gl_deselect_api();
}

void oc_wgl_surface_swap_interval(oc_surface_data* interface, int swap)
{
	oc_wgl_surface* surface = (oc_wgl_surface*)interface;
	wglSwapIntervalEXT(swap);
}

void* oc_wgl_get_proc(const char* name)
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

oc_surface_data* oc_wgl_surface_create_for_window(oc_window window)
{
	oc_wgl_surface* surface = 0;

	oc_window_data* windowData = oc_window_ptr_from_handle(window);
	if(windowData)
	{
		oc_wgl_init();

		//NOTE: fill surface data and load api
		surface = oc_malloc_type(oc_wgl_surface);
		if(surface)
		{
			memset(surface, 0, sizeof(oc_wgl_surface));
			oc_surface_init_for_window((oc_surface_data*)surface, windowData);

			surface->interface.api = OC_GL;
			surface->interface.destroy = oc_wgl_surface_destroy;
			surface->interface.prepare = oc_wgl_surface_prepare;
			surface->interface.present = oc_wgl_surface_present;
			surface->interface.swapInterval = oc_wgl_surface_swap_interval;
			surface->interface.deselect = oc_wgl_surface_deselect;

			surface->hDC = GetDC(surface->interface.layer.hWnd);

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
				WGL_TRANSPARENT_ARB, TRUE,
				WGL_COLOR_BITS_ARB, 32,
				WGL_RED_BITS_ARB, 8,
				WGL_GREEN_BITS_ARB, 8,
				WGL_BLUE_BITS_ARB, 8,
				WGL_ALPHA_BITS_ARB, 8,
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
				WGL_CONTEXT_MINOR_VERSION_ARB, 4,
				WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
				0};

			surface->glContext = wglCreateContextAttribsARB(surface->hDC, oc_wglDummyContext.glContext, contextAttrs);

			if(!surface->glContext)
			{
				//TODO error
				int error = GetLastError();
				printf("error: %i\n", error);
			}

			//NOTE: make gl context current and load api
			wglMakeCurrent(surface->hDC, surface->glContext);
			wglSwapIntervalEXT(1);
			oc_gl_load_gl44(&surface->api, oc_wgl_get_proc);
		}
	}
	return((oc_surface_data*)surface);
}
