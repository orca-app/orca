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
static void wgl_load_procs()
{
	#define WGL_PROC(type, name) name = (_cat3_(PFN, type, PROC))wglGetProcAddress( #name );
		WGL_PROC_LIST
	#undef WGL_PROC
}
#undef WGL_PROC_LIST


typedef struct mg_wgl_surface
{
	mg_surface_data interface;

	HWND hWnd;
	HDC hDC;
	HGLRC glContext;
	vec2 contentsScaling;

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

vec2 mg_wgl_contents_scaling(mg_surface_data* interface)
{
	mg_wgl_surface* surface = (mg_wgl_surface*)interface;
	return(surface->contentsScaling);
}

mp_rect mg_wgl_surface_get_frame(mg_surface_data* interface)
{
	mg_wgl_surface* surface = (mg_wgl_surface*)interface;
	RECT rect = {0};
	GetClientRect(surface->hWnd, &rect);

	vec2 scale = surface->contentsScaling;

	mp_rect res = {rect.left/scale.x,
	               rect.bottom/scale.y,
	               (rect.right - rect.left)/scale.x,
	               (rect.bottom - rect.top)/scale.y};
	return(res);
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

mg_surface mg_wgl_surface_create_for_window(mp_window window)
{
	mg_surface surfaceHandle = mg_surface_nil();

	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		//NOTE: create a dummy window
		WNDCLASS windowClass = {.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
		                        .lpfnWndProc = WinProc,
		                        .hInstance = GetModuleHandleW(NULL),
		                        .lpszClassName = "HelperWindowClass",
		                        .hCursor = LoadCursor(0, IDC_ARROW)};

		if(!RegisterClass(&windowClass))
		{
			//TODO: error
			goto quit;
		}

		HWND helperWindowHandle = CreateWindow("HelperWindowClass", "Test Window",
	                                 	WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
	                                 	800, 600,
	                                 	0, 0, windowClass.hInstance, 0);

		if(!helperWindowHandle)
		{
			//TODO: error
			goto quit;
		}

		//NOTE(martin): create a dummy OpenGL context, to be able to load extensions
		HDC dummyDC = GetDC(helperWindowHandle);

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

		int pixelFormat = ChoosePixelFormat(dummyDC, &pixelFormatDesc);
		SetPixelFormat(dummyDC, pixelFormat, &pixelFormatDesc);

		HGLRC dummyGLContext = wglCreateContext(dummyDC);
		wglMakeCurrent(dummyDC, dummyGLContext);

		//NOTE(martin): now load WGL extension functions
		wgl_load_procs();

		//NOTE(martin): now create the true pixel format and gl context
		int pixelFormatAttrs[] = {
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_COLOR_BITS_ARB, 32,
			WGL_DEPTH_BITS_ARB, 24,
			WGL_STENCIL_BITS_ARB, 8,
			0};

		HDC hDC = GetDC(windowData->win32.hWnd);
		u32 numFormats = 0;

		wglChoosePixelFormatARB(hDC, pixelFormatAttrs, 0, 1, &pixelFormat, &numFormats);

		if(!pixelFormat)
		{
			//TODO: error
		}
		SetPixelFormat(hDC, pixelFormat, &pixelFormatDesc);

		int contextAttrs[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
			WGL_CONTEXT_MINOR_VERSION_ARB, 3,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0};

		HGLRC glContext = wglCreateContextAttribsARB(hDC, dummyGLContext, contextAttrs);

		if(!glContext)
		{
			//TODO error
			int error = GetLastError();
			printf("error: %i\n", error);
		}

		//NOTE: destroy dummy context and dummy window
		wglMakeCurrent(hDC, 0);
		wglDeleteContext(dummyGLContext);
		DestroyWindow(helperWindowHandle);

		//NOTE: make gl context current
		wglMakeCurrent(hDC, glContext);
		wglSwapIntervalEXT(1);

		//NOTE: fill surface data and load api
		mg_wgl_surface* surface = malloc_type(mg_wgl_surface);

		surface->interface.backend = MG_BACKEND_GL;
		surface->interface.destroy = mg_wgl_surface_destroy;
		surface->interface.prepare = mg_wgl_surface_prepare;
		surface->interface.present = mg_wgl_surface_present;
		surface->interface.swapInterval = mg_wgl_surface_swap_interval;
		surface->interface.contentsScaling = mg_wgl_contents_scaling;
		surface->interface.getFrame = mg_wgl_surface_get_frame;
		//TODO: get/set frame/hidden

		surface->hWnd = windowData->win32.hWnd;
		surface->hDC = hDC;
		surface->glContext = glContext;

		u32 dpi = GetDpiForWindow(windowData->win32.hWnd);
		surface->contentsScaling = (vec2){(float)dpi/96., (float)dpi/96.};

		 mg_gl_load_gl43(&surface->api, mg_wgl_get_proc);

		surfaceHandle = mg_surface_alloc_handle((mg_surface_data*)surface);
	}

	quit:;
	return(surfaceHandle);
}
