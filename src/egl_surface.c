/************************************************************//**
*
*	@file: egl_surface.cpp
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

#if OS_MACOS
	//NOTE: EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE on osx defaults to CGL backend, which doesn't handle SwapInterval correctly
	#define MG_EGL_PLATFORM_ANGLE_TYPE EGL_PLATFORM_ANGLE_TYPE_METAL_ANGLE

	//NOTE: hardcode GLES versions for now
	//TODO: use version hints, once we have all api versions correctly categorized by glapi.py
	#define MG_GLES_VERSION_MAJOR 3
	#define MG_GLES_VERSION_MINOR 0
	#define mg_gl_load_gles mg_gl_load_gles30
#else
	#define MG_EGL_PLATFORM_ANGLE_TYPE EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE
	#define MG_GLES_VERSION_MAJOR 3
	#define MG_GLES_VERSION_MINOR 1
	#define mg_gl_load_gles mg_gl_load_gles31
#endif


typedef struct mg_egl_surface
{
	mg_surface_data interface;

	mp_layer layer;

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

	mp_layer_cleanup(&surface->layer);
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
	return(mp_layer_contents_scaling(&surface->layer));
}

mp_rect mg_egl_surface_get_frame(mg_surface_data* interface)
{
	mg_egl_surface* surface = (mg_egl_surface*)interface;
	return(mp_layer_get_frame(&surface->layer));
}

void mg_egl_surface_set_frame(mg_surface_data* interface, mp_rect frame)
{
	mg_egl_surface* surface = (mg_egl_surface*)interface;
	mp_layer_set_frame(&surface->layer, frame);
}

void mg_egl_surface_set_hidden(mg_surface_data* interface, bool hidden)
{
	mg_egl_surface* surface = (mg_egl_surface*)interface;
	mp_layer_set_hidden(&surface->layer, hidden);
}

bool mg_egl_surface_get_hidden(mg_surface_data* interface)
{
	mg_egl_surface* surface = (mg_egl_surface*)interface;
	return(mp_layer_get_hidden(&surface->layer));
}

mg_surface mg_egl_surface_create_for_window(mp_window window)
{
	mg_surface res = mg_surface_nil();

	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
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

		mp_layer_init_for_window(&surface->layer, windowData);

		EGLAttrib displayAttribs[] = {
			EGL_PLATFORM_ANGLE_TYPE_ANGLE, MG_EGL_PLATFORM_ANGLE_TYPE,
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
		                                             mp_layer_native_surface(&surface->layer),
		                                             surfaceAttributes);

		eglBindAPI(EGL_OPENGL_ES_API);
		EGLint contextAttributes[] = {
			EGL_CONTEXT_MAJOR_VERSION_KHR, MG_GLES_VERSION_MAJOR,
			EGL_CONTEXT_MINOR_VERSION_KHR, MG_GLES_VERSION_MINOR,
			EGL_CONTEXT_BIND_GENERATES_RESOURCE_CHROMIUM, EGL_TRUE,
			EGL_CONTEXT_CLIENT_ARRAYS_ENABLED_ANGLE, EGL_TRUE,
			EGL_CONTEXT_OPENGL_BACKWARDS_COMPATIBLE_ANGLE, EGL_FALSE,
			EGL_NONE};

		surface->eglContext = eglCreateContext(surface->eglDisplay, surface->eglConfig, EGL_NO_CONTEXT, contextAttributes);
		eglMakeCurrent(surface->eglDisplay, surface->eglSurface, surface->eglSurface, surface->eglContext);

		mg_gl_load_gles(&surface->api, (mg_gl_load_proc)eglGetProcAddress);

		eglSwapInterval(surface->eglDisplay, 1);

		res = mg_surface_alloc_handle((mg_surface_data*)surface);
	}
	return(res);
}
