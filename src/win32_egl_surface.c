/************************************************************//**
*
*	@file: win32_egl_surface.cpp
*	@author: Martin Fouilleul
*	@date: 17/02/2023
*	@revision:
*
*****************************************************************/

#define EGL_EGLEXT_PROTOTYPES
#include<EGL/egl.h>
#include<EGL/eglext.h>

#include"mp_app_internal.h"
#include"graphics_internal.h"
#include"gl_loader.h"

typedef struct mg_egl_surface
{
	mg_surface_data interface;

	HWND hWnd;
	vec2 contentsScaling;

	EGLDisplay eglDisplay;
	EGLConfig eglConfig;
	EGLContext eglContext;
	EGLSurface eglSurface;

	mg_gl_api api;

} mg_egl_surface;

void mg_egl_surface_destroy(mg_surface_data* interface)
{
	mg_egl_surface* surface = (mg_egl_surface*)interface;

	if(&surface->api == mg_gl_get_api())
	{
		mg_gl_select_api(0);
	}
	if(eglGetCurrentContext() == surface->eglContext)
	{
		eglMakeCurrent(surface->eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	}
	eglDestroyContext(surface->eglDisplay, surface->eglContext);
	eglDestroySurface(surface->eglDisplay, surface->eglSurface);

	DestroyWindow(surface->hWnd);
	free(surface);
}

void mg_egl_surface_prepare(mg_surface_data* interface)
{
	mg_egl_surface* surface = (mg_egl_surface*)interface;
	eglMakeCurrent(surface->eglDisplay, surface->eglSurface, surface->eglSurface, surface->eglContext);
	mg_gl_select_api(&surface->api);
}

void mg_egl_surface_present(mg_surface_data* interface)
{
	mg_egl_surface* surface = (mg_egl_surface*)interface;
	eglSwapBuffers(surface->eglDisplay, surface->eglSurface);
}

void mg_egl_surface_swap_interval(mg_surface_data* interface, int swap)
{
	mg_egl_surface* surface = (mg_egl_surface*)interface;
	eglSwapInterval(surface->eglDisplay, swap);
}

vec2 mg_egl_surface_contents_scaling(mg_surface_data* interface)
{
	mg_egl_surface* surface = (mg_egl_surface*)interface;
	return(surface->contentsScaling);
}

mp_rect mg_egl_surface_get_frame(mg_surface_data* interface)
{
	mp_rect res = {0};
	mg_egl_surface* surface = (mg_egl_surface*)interface;
	if(surface)
	{
		RECT rect = {0};
		GetClientRect(surface->hWnd, &rect);

		vec2 scale = surface->contentsScaling;

		res = (mp_rect){rect.left/scale.x,
	               	    rect.bottom/scale.y,
	               	    (rect.right - rect.left)/scale.x,
	               	    (rect.bottom - rect.top)/scale.y};
	}
	return(res);
}

void mg_egl_surface_set_frame(mg_surface_data* interface, mp_rect frame)
{
	mg_egl_surface* surface = (mg_egl_surface*)interface;
	if(surface)
	{
		HWND parent = GetParent(surface->hWnd);
		RECT parentContentRect;
		GetClientRect(parent, &parentContentRect);
		int parentHeight = 	parentContentRect.bottom - parentContentRect.top;

		SetWindowPos(surface->hWnd,
			         HWND_TOP,
			         frame.x * surface->contentsScaling.x,
			         parentHeight - (frame.y + frame.h) * surface->contentsScaling.y,
			         frame.w * surface->contentsScaling.x,
			         frame.h * surface->contentsScaling.y,
			         SWP_NOACTIVATE | SWP_NOZORDER);
	}
}

void mg_egl_surface_set_hidden(mg_surface_data* interface, bool hidden)
{
	mg_egl_surface* surface = (mg_egl_surface*)interface;
	if(surface)
	{
		ShowWindow(surface->hWnd, hidden ? SW_HIDE : SW_NORMAL);
	}
}

bool mg_egl_surface_get_hidden(mg_surface_data* interface)
{
	bool hidden = false;
	mg_egl_surface* surface = (mg_egl_surface*)interface;
	if(surface)
	{
		hidden = !IsWindowVisible(surface->hWnd);
	}
	return(hidden);
}


LRESULT mg_egl_surface_window_proc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	return(DefWindowProc(windowHandle, message, wParam, lParam));
}


mg_surface mg_egl_surface_create_for_window(mp_window window)
{
	mg_surface res = mg_surface_nil();

	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		//NOTE(martin): create a child window for the surface
		WNDCLASS childWindowClass = {.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
		                        .lpfnWndProc = mg_egl_surface_window_proc,
		                        .hInstance = GetModuleHandleW(NULL),
		                        .lpszClassName = "egl_surface_window_class",
		                        .hCursor = LoadCursor(0, IDC_ARROW)};

		if(!RegisterClass(&childWindowClass))
		{
			//TODO: error
		}

		mp_rect frame = mp_window_get_content_rect(window);

		u32 dpi = GetDpiForWindow(windowData->win32.hWnd);
		vec2 contentsScaling = (vec2){(float)dpi/96., (float)dpi/96.};

		HWND childWindow = CreateWindow("egl_surface_window_class", "Test",
	                                 	       WS_CHILD|WS_VISIBLE,
	                                 	       0, 0, frame.w*contentsScaling.x, frame.h*contentsScaling.y,
	                                 	       windowData->win32.hWnd,
	                                 	       0,
	                                 	       childWindowClass.hInstance,
	                                 	       0);

		mg_egl_surface* surface = malloc_type(mg_egl_surface);
		memset(surface, 0, sizeof(mg_egl_surface));

		surface->interface.backend = MG_BACKEND_GLES;
		surface->interface.destroy = mg_egl_surface_destroy;
		surface->interface.prepare = mg_egl_surface_prepare;
		surface->interface.present = mg_egl_surface_present;
		surface->interface.swapInterval = mg_egl_surface_swap_interval;
		surface->interface.contentsScaling = mg_egl_surface_contents_scaling;
		surface->interface.getFrame = mg_egl_surface_get_frame;
		surface->interface.setFrame = mg_egl_surface_set_frame;
		surface->interface.getHidden = mg_egl_surface_get_hidden;
		surface->interface.setHidden = mg_egl_surface_set_hidden;

		surface->hWnd = childWindow;
		surface->contentsScaling = contentsScaling;

		EGLAttrib displayAttribs[] = {
			EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE,
	    	EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE,
	    	EGL_NONE};

		surface->eglDisplay = eglGetPlatformDisplay(EGL_PLATFORM_ANGLE_ANGLE, (void*)EGL_DEFAULT_DISPLAY, displayAttribs);
		eglInitialize(surface->eglDisplay, NULL, NULL);

		EGLint const configAttributes[] = {
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_RED_SIZE, 8,
			EGL_GREEN_SIZE, 8,
			EGL_BLUE_SIZE, 8,
			EGL_ALPHA_SIZE, 8,
			EGL_DEPTH_SIZE, 24,
			EGL_STENCIL_SIZE, 8,
			EGL_SAMPLE_BUFFERS, 0,
			EGL_SAMPLES, EGL_DONT_CARE,
			EGL_COLOR_COMPONENT_TYPE_EXT, EGL_COLOR_COMPONENT_TYPE_FIXED_EXT,
			EGL_NONE };

		int numConfigs = 0;
		eglChooseConfig(surface->eglDisplay, configAttributes, &surface->eglConfig, 1, &numConfigs);

		EGLint const surfaceAttributes[] = {EGL_NONE};
		surface->eglSurface = eglCreateWindowSurface(surface->eglDisplay,
		                                             surface->eglConfig,
		                                             surface->hWnd,
		                                             surfaceAttributes);

		eglBindAPI(EGL_OPENGL_ES_API);
		EGLint contextAttributes[] = {
			EGL_CONTEXT_MAJOR_VERSION_KHR, 3,
			EGL_CONTEXT_MINOR_VERSION_KHR, 1,
			EGL_CONTEXT_BIND_GENERATES_RESOURCE_CHROMIUM, EGL_TRUE,
			EGL_CONTEXT_CLIENT_ARRAYS_ENABLED_ANGLE, EGL_TRUE,
			EGL_CONTEXT_OPENGL_BACKWARDS_COMPATIBLE_ANGLE, EGL_FALSE,
			EGL_NONE};

		surface->eglContext = eglCreateContext(surface->eglDisplay, surface->eglConfig, EGL_NO_CONTEXT, contextAttributes);
		eglMakeCurrent(surface->eglDisplay, surface->eglSurface, surface->eglSurface, surface->eglContext);

		mg_gl_load_gles32(&surface->api, (mg_gl_load_proc)eglGetProcAddress);

		eglSwapInterval(surface->eglDisplay, 1);

		res = mg_surface_alloc_handle((mg_surface_data*)surface);
	}
	return(res);
}
