#include "graphics/graphics.h"
#include "wasmbind/hostcalls.h"

oc_vec2 oc_image_size(oc_image image)
{
    oc_vec2 ret;
    oc_hostcall_image_size(&image, &ret);
    return (ret);
}

oc_image oc_image_create(oc_canvas_renderer renderer, u32 width, u32 height)
{
    oc_image ret;
    oc_hostcall_image_create(&renderer, width, height, &ret);
    return (ret);
}

void oc_image_destroy(oc_image image)
{
    oc_hostcall_image_destroy(&image);
}

void oc_image_upload_region_rgba8(oc_image image, oc_rect region, u8* pixels)
{
    oc_hostcall_image_upload_region_rgba8(&image, &region, pixels);
}

oc_vec2 oc_surface_get_size(oc_surface surface)
{
    oc_vec2 ret;
    oc_hostcall_surface_get_size(&surface, &ret);
    return (ret);
}

oc_vec2 oc_surface_contents_scaling(oc_surface surface)
{
    oc_vec2 ret;
    oc_hostcall_surface_contents_scaling(&surface, &ret);
    return (ret);
}

void oc_surface_bring_to_front(oc_surface surface)
{
    oc_hostcall_surface_bring_to_front(&surface);
}

void oc_surface_send_to_back(oc_surface surface)
{
    oc_hostcall_surface_send_to_back(&surface);
}

oc_canvas_renderer oc_canvas_renderer_create(void)
{
    oc_canvas_renderer ret;
    oc_hostcall_canvas_renderer_create(&ret);
    return (ret);
}

oc_surface oc_canvas_surface_create(oc_canvas_renderer renderer)
{
    oc_surface ret;
    oc_hostcall_canvas_surface_create(&renderer, &ret);
    return (ret);
}

void oc_canvas_renderer_submit(oc_canvas_renderer renderer,
                               oc_surface surface,
                               u32 msaaSampleCount,
                               bool clear,
                               oc_color clearColor,
                               u32 primitiveCount,
                               oc_primitive* primitives,
                               u32 eltCount,
                               oc_path_elt* elements)
{
    oc_hostcall_canvas_renderer_submit(&renderer,
                                       &surface,
                                       msaaSampleCount,
                                       clear,
                                       &clearColor,
                                       primitiveCount,
                                       primitives,
                                       eltCount,
                                       elements);
}

void oc_canvas_present(oc_canvas_renderer renderer, oc_surface surface)
{
    oc_hostcall_canvas_present(&renderer, &surface);
}

oc_surface oc_gles_surface_create(void)
{
    oc_surface ret;
    oc_hostcall_gles_surface_create(&ret);
    return (ret);
}

void oc_gles_surface_make_current(oc_surface surface)
{
    oc_hostcall_gles_surface_make_current(&surface);
}

void oc_gles_surface_swap_buffers(oc_surface surface)
{
    oc_hostcall_gles_surface_swap_buffers(&surface);
}
