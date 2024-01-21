/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#define EGL_EGLEXT_PROTOTYPES
#include "app/app_internal.h"
#include "gl_loader.h"
#include "graphics_surface.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>

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

typedef struct oc_egl_surface
{
    oc_surface_data interface;

    EGLDisplay eglDisplay;
    EGLConfig eglConfig;
    EGLContext eglContext;
    EGLSurface eglSurface;

    oc_gl_api api;

} oc_egl_surface;

void oc_egl_surface_destroy(oc_surface_data* interface)
{
    oc_egl_surface* surface = (oc_egl_surface*)interface;

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

    oc_surface_cleanup((oc_surface_data*)surface);
    free(surface);
}

void oc_egl_surface_prepare(oc_surface_data* interface)
{
    oc_egl_surface* surface = (oc_egl_surface*)interface;
    eglMakeCurrent(surface->eglDisplay, surface->eglSurface, surface->eglSurface, surface->eglContext);
    oc_gl_select_api(&surface->api);
}

void oc_egl_surface_present(oc_surface_data* interface)
{
    oc_egl_surface* surface = (oc_egl_surface*)interface;
    eglSwapBuffers(surface->eglDisplay, surface->eglSurface);
}

void oc_egl_surface_deselect(oc_surface_data* interface)
{
    oc_egl_surface* surface = (oc_egl_surface*)interface;
    eglMakeCurrent(surface->eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    oc_gl_deselect_api();
}

void oc_egl_surface_swap_interval(oc_surface_data* interface, int swap)
{
    oc_egl_surface* surface = (oc_egl_surface*)interface;
    eglSwapInterval(surface->eglDisplay, swap);
}

void oc_egl_surface_init(oc_egl_surface* surface)
{
    void* nativeLayer = surface->interface.nativeLayer((oc_surface_data*)surface);

    surface->interface.api = OC_GLES;

    surface->interface.destroy = oc_egl_surface_destroy;
    surface->interface.prepare = oc_egl_surface_prepare;
    surface->interface.present = oc_egl_surface_present;
    surface->interface.deselect = oc_egl_surface_deselect;
    surface->interface.swapInterval = oc_egl_surface_swap_interval;

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

oc_surface_data* oc_egl_surface_create_for_window(oc_window window)
{
    oc_egl_surface* surface = 0;
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        surface = oc_malloc_type(oc_egl_surface);
        if(surface)
        {
            memset(surface, 0, sizeof(oc_egl_surface));

            oc_surface_init_for_window((oc_surface_data*)surface, windowData);
            oc_egl_surface_init(surface);
        }
    }
    return ((oc_surface_data*)surface);
}
