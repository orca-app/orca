/************************************************************//**
*
*	@file: osx_gles_surface.cpp
*	@author: Martin Fouilleul
*	@date: 18/08/2022
*	@revision:
*
*****************************************************************/
#include<GLES3/gl31.h>
#define EGL_EGLEXT_PROTOTYPES
#include<EGL/egl.h>
#include<EGL/eglext.h>

#include"graphics_internal.h"

typedef struct mg_gles_surface
{
	mg_surface_data interface;

	HWND hWnd;

	EGLDisplay eglDisplay;
	EGLConfig eglConfig;
	EGLContext eglContext;
	EGLSurface eglSurface;

} mg_gles_surface;

void mg_gles_surface_destroy(mg_surface_data* interface)
{
	//////////////////////////////////////////////////
	//TODO
	//////////////////////////////////////////////////
}

void mg_gles_surface_prepare(mg_surface_data* interface)
{
	mg_gles_surface* surface = (mg_gles_surface*)interface;
	eglMakeCurrent(surface->eglDisplay, surface->eglSurface, surface->eglSurface, surface->eglContext);
}

void mg_gles_surface_present(mg_surface_data* interface)
{
	mg_gles_surface* surface = (mg_gles_surface*)interface;
	eglSwapBuffers(surface->eglDisplay, surface->eglSurface);
}

/*
void mg_gles_surface_set_frame(mg_surface_data* interface, mp_rect frame);

void mg_gles_surface_set_hidden(mg_surface_data* interface, bool hidden);
bool mg_gles_surface_get_hidden(mg_surface_data* interface);
*/

mp_rect mg_gles_surface_get_frame(mg_surface_data* interface)
{
	mg_gles_surface* surface = (mg_gles_surface*)interface;
	RECT rect = {0};
	GetClientRect(surface->hWnd, &rect);

	mp_rect res = {rect.left, rect.bottom, rect.right - rect.left, rect.bottom - rect.top};
	return(res);
}

mg_surface mg_gles_surface_create_for_window(mp_window window)
{
	mg_surface res = mg_surface_nil();
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		mg_gles_surface* surface = malloc_type(mg_gles_surface);
		memset(surface, 0, sizeof(mg_gles_surface));

		surface->interface.backend = MG_BACKEND_GLES;
		surface->interface.destroy = mg_gles_surface_destroy;
		surface->interface.prepare = mg_gles_surface_prepare;
		surface->interface.present = mg_gles_surface_present;
		surface->interface.getFrame = mg_gles_surface_get_frame;
		/*TODO
		surface->interface.setFrame = mg_gles_surface_set_frame;
		surface->interface.getHidden = mg_gles_surface_get_hidden;
		surface->interface.setHidden = mg_gles_surface_set_hidden;
		*/

		surface->hWnd = windowData->win32.hWnd;

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
		surface->eglSurface = eglCreateWindowSurface(surface->eglDisplay, surface->eglConfig, surface->hWnd, surfaceAttributes);

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

//TODO: reactivate this when finished testing!
//		eglSwapInterval(surface->eglDisplay, 1);
		eglSwapInterval(surface->eglDisplay, 0);

		res = mg_surface_alloc_handle((mg_surface_data*)surface);
	}
	return(res);
}
