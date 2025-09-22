/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "app/app_internal.h"
#include "gl_loader.h"
#include "surface.h"
#include "gles_surface.h"

#if OC_PLATFORM_MACOS
    //NOTE: EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE on osx defaults to CGL backend, which doesn't handle SwapInterval correctly
    #define OC_EGL_PLATFORM_ANGLE_TYPE EGL_PLATFORM_ANGLE_TYPE_METAL_ANGLE

    //NOTE: hardcode GLES versions for now
    //TODO: use version hints, once we have all api versions correctly categorized by glapi.py
    #define OC_GLES_VERSION_MAJOR 3
    #define OC_GLES_VERSION_MINOR 0
    #define oc_gl_load_gles oc_gl_load_gles31
#else
    #define OC_EGL_PLATFORM_ANGLE_TYPE EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE
    #define OC_GLES_VERSION_MAJOR 3
    #define OC_GLES_VERSION_MINOR 1
    #define oc_gl_load_gles oc_gl_load_gles32
#endif

#if OC_PLATFORM_LINUX
    #include <app/linux_app.h>
#endif

typedef struct oc_gles_surface
{
    oc_surface_base base;

    EGLDisplay eglDisplay;
    EGLConfig eglConfig;
    EGLContext eglContext;
    EGLSurface eglSurface;

    oc_gl_api api;

} oc_gles_surface;

i32 oc_gles_surface_destroy_callback(void* user)
{
    oc_surface_base* base = (oc_surface_base*)user;

    oc_gles_surface* surface = (oc_gles_surface*)base;

    if(&surface->api == oc_gl_get_api())
    {
        oc_gl_deselect_api();
    }
    if(eglGetCurrentContext() == surface->eglContext)
    {
        eglMakeCurrent(surface->eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }
    eglDestroyContext(surface->eglDisplay, surface->eglContext);
    eglDestroySurface(surface->eglDisplay, surface->eglSurface);

    oc_surface_base_cleanup((oc_surface_base*)surface);
    free(surface);

    return 0;
}

void oc_gles_surface_destroy(oc_surface_base* base)
{
    oc_dispatch_on_main_thread_sync(oc_gles_surface_destroy_callback, base);
}

void oc_gles_surface_make_current(oc_surface handle)
{
    oc_surface_base* base = oc_surface_from_handle(handle);
    if(base && base->api == OC_SURFACE_GLES)
    {
        oc_gles_surface* surface = (oc_gles_surface*)base;
        eglMakeCurrent(surface->eglDisplay, surface->eglSurface, surface->eglSurface, surface->eglContext);
        oc_gl_select_api(&surface->api);
    }
}

void oc_gles_surface_swap_buffers(oc_surface handle)
{
    oc_surface_base* base = oc_surface_from_handle(handle);
    if(base && base->api == OC_SURFACE_GLES)
    {
        oc_gles_surface* surface = (oc_gles_surface*)base;
        eglSwapBuffers(surface->eglDisplay, surface->eglSurface);
    }
}

void oc_gles_surface_swap_interval(oc_surface handle, int swap)
{
    oc_surface_base* base = oc_surface_from_handle(handle);
    if(base && base->api == OC_SURFACE_GLES)
    {
        oc_gles_surface* surface = (oc_gles_surface*)base;
        eglSwapInterval(surface->eglDisplay, swap);
    }
}

void* oc_gles_surface_native_pointer(oc_gles_surface* surface);

#if OC_PLATFORM_MACOS

void* oc_gles_surface_native_pointer(oc_gles_surface* surface)
{
    return ((void*)surface->base.view.layer);
}

#elif OC_PLATFORM_WINDOWS

void* oc_gles_surface_native_pointer(oc_gles_surface* surface)
{
    return ((void*)surface->base.view.hWnd);
}

#elif OC_PLATFORM_LINUX

void* oc_gles_surface_native_pointer(oc_gles_surface* surface)
{
    oc_unimplemented();
    return NULL;
}

#else
    #error "unsupported platform"
#endif

void oc_gles_surface_init(oc_gles_surface* surface)
{
    void* nativeLayer = oc_gles_surface_native_pointer(surface);

    surface->base.api = OC_SURFACE_GLES;
    surface->base.destroy = oc_gles_surface_destroy;

    EGLDisplay eglDisplay = NULL;
    {
        #if OC_PLATFORM_LINUX
        Display* dpy = oc_appData.linux.x11.display;
        eglDisplay = eglGetPlatformDisplay(EGL_PLATFORM_X11_EXT, dpy, NULL);
        #else
        EGLAttrib displayAttribs[] = {
            EGL_PLATFORM_ANGLE_TYPE_ANGLE, OC_EGL_PLATFORM_ANGLE_TYPE,
            EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE,
            EGL_NONE
        };
        eglDisplay = eglGetPlatformDisplay(EGL_PLATFORM_ANGLE_ANGLE, (void*)EGL_DEFAULT_DISPLAY, displayAttribs);
        #endif
    }

    surface->eglDisplay = eglDisplay;
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
        EGL_NONE
    };

    int numConfigs = 0;
    eglChooseConfig(surface->eglDisplay, configAttributes, &surface->eglConfig, 1, &numConfigs);

    EGLint const surfaceAttributes[] = { EGL_NONE };
    surface->eglSurface = eglCreateWindowSurface(surface->eglDisplay,
                                                 surface->eglConfig,
                                                 (u32)(uintptr_t)nativeLayer,
                                                 surfaceAttributes);

    eglBindAPI(EGL_OPENGL_ES_API);
    EGLint contextAttributes[] = {
        EGL_CONTEXT_MAJOR_VERSION_KHR, OC_GLES_VERSION_MAJOR,
        EGL_CONTEXT_MINOR_VERSION_KHR, OC_GLES_VERSION_MINOR,
        #if !OC_PLATFORM_LINUX
        EGL_CONTEXT_BIND_GENERATES_RESOURCE_CHROMIUM, EGL_TRUE,
        EGL_CONTEXT_CLIENT_ARRAYS_ENABLED_ANGLE, EGL_TRUE,
        EGL_CONTEXT_OPENGL_BACKWARDS_COMPATIBLE_ANGLE, EGL_FALSE,
        #endif
        EGL_NONE
    };

    surface->eglContext = eglCreateContext(surface->eglDisplay, surface->eglConfig, EGL_NO_CONTEXT, contextAttributes);
    eglMakeCurrent(surface->eglDisplay, surface->eglSurface, surface->eglSurface, surface->eglContext);

    oc_gl_load_gles(&surface->api, (oc_gl_load_proc)eglGetProcAddress);

    eglSwapInterval(surface->eglDisplay, 1);
}

typedef struct oc_gles_surface_create_data
{
    oc_surface surface;
    oc_window window;
} oc_gles_surface_create_data;

i32 oc_gles_surface_create_callback(void* user)
{
    oc_gles_surface_create_data* data = (oc_gles_surface_create_data*)user;

    oc_gles_surface* surface = 0;
    oc_window_data* windowData = oc_window_ptr_from_handle(data->window);
    if(windowData)
    {
        surface = oc_malloc_type(oc_gles_surface);
        if(surface)
        {
            memset(surface, 0, sizeof(oc_gles_surface));

            oc_surface_base_init_for_window((oc_surface_base*)surface, windowData);
            oc_gles_surface_init(surface);
        }
    }

    oc_surface handle = oc_surface_nil();
    if(surface)
    {
        handle = oc_surface_handle_alloc((oc_surface_base*)surface);
    }
    data->surface = handle;
    return 0;
}

oc_surface oc_gles_surface_create_for_window(oc_window window)
{
    oc_gles_surface_create_data data = {
        .window = window,
    };
    oc_dispatch_on_main_thread_sync(oc_gles_surface_create_callback, &data);

    return (data.surface);
}
