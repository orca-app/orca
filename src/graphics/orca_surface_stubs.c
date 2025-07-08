#include "graphics/graphics.h"

void ORCA_IMPORT(oc_image_size_argptr_stub) (oc_vec2* __retArg, oc_image* image);

oc_vec2 oc_image_size(oc_image image)
{
	oc_vec2 __ret;
	oc_image_size_argptr_stub(&__ret, &image);
	return(__ret);
}

void ORCA_IMPORT(oc_image_create_argptr_stub) (oc_image* __retArg, oc_canvas_renderer* renderer, u32 width, u32 height);

oc_image oc_image_create(oc_canvas_renderer renderer, u32 width, u32 height)
{
	oc_image __ret;
	oc_image_create_argptr_stub(&__ret, &renderer, width, height);
	return(__ret);
}

void ORCA_IMPORT(oc_image_destroy_argptr_stub) (oc_image* image);

void oc_image_destroy(oc_image image)
{
	oc_image_destroy_argptr_stub(&image);
}

void ORCA_IMPORT(oc_image_upload_region_rgba8_argptr_stub) (oc_image* image, oc_rect* region, u8* pixels);

void oc_image_upload_region_rgba8(oc_image image, oc_rect region, u8* pixels)
{
	oc_image_upload_region_rgba8_argptr_stub(&image, &region, pixels);
}

void ORCA_IMPORT(oc_surface_get_size_argptr_stub) (oc_vec2* __retArg, oc_surface* surface);

oc_vec2 oc_surface_get_size(oc_surface surface)
{
	oc_vec2 __ret;
	oc_surface_get_size_argptr_stub(&__ret, &surface);
	return(__ret);
}

void ORCA_IMPORT(oc_surface_contents_scaling_argptr_stub) (oc_vec2* __retArg, oc_surface* surface);

oc_vec2 oc_surface_contents_scaling(oc_surface surface)
{
	oc_vec2 __ret;
	oc_surface_contents_scaling_argptr_stub(&__ret, &surface);
	return(__ret);
}

void ORCA_IMPORT(oc_surface_bring_to_front_argptr_stub) (oc_surface* surface);

void oc_surface_bring_to_front(oc_surface surface)
{
	oc_surface_bring_to_front_argptr_stub(&surface);
}

void ORCA_IMPORT(oc_surface_send_to_back_argptr_stub) (oc_surface* surface);

void oc_surface_send_to_back(oc_surface surface)
{
	oc_surface_send_to_back_argptr_stub(&surface);
}

void ORCA_IMPORT(oc_canvas_renderer_create_argptr_stub) (oc_canvas_renderer* __retArg);

oc_canvas_renderer oc_canvas_renderer_create(void)
{
	oc_canvas_renderer __ret;
	oc_canvas_renderer_create_argptr_stub(&__ret);
	return(__ret);
}

void ORCA_IMPORT(oc_canvas_surface_create_argptr_stub) (oc_surface* __retArg, oc_canvas_renderer* renderer);

oc_surface oc_canvas_surface_create(oc_canvas_renderer renderer)
{
	oc_surface __ret;
	oc_canvas_surface_create_argptr_stub(&__ret, &renderer);
	return(__ret);
}

void ORCA_IMPORT(oc_canvas_renderer_submit_argptr_stub) (oc_canvas_renderer* renderer, oc_surface* surface, u32 msaaSampleCount, bool clear, oc_color* clearColor, u32 primitiveCount, oc_primitive* primitives, u32 eltCount, oc_path_elt* elements);

void oc_canvas_renderer_submit(oc_canvas_renderer renderer, oc_surface surface, u32 msaaSampleCount, bool clear, oc_color clearColor, u32 primitiveCount, oc_primitive* primitives, u32 eltCount, oc_path_elt* elements)
{
	oc_canvas_renderer_submit_argptr_stub(&renderer, &surface, msaaSampleCount, clear, &clearColor, primitiveCount, primitives, eltCount, elements);
}

void ORCA_IMPORT(oc_canvas_present_argptr_stub) (oc_canvas_renderer* renderer, oc_surface* surface);

void oc_canvas_present(oc_canvas_renderer renderer, oc_surface surface)
{
	oc_canvas_present_argptr_stub(&renderer, &surface);
}

void ORCA_IMPORT(oc_gles_surface_create_argptr_stub) (oc_surface* __retArg);

oc_surface oc_gles_surface_create(void)
{
	oc_surface __ret;
	oc_gles_surface_create_argptr_stub(&__ret);
	return(__ret);
}

void ORCA_IMPORT(oc_gles_surface_make_current_argptr_stub) (oc_surface* surface);

void oc_gles_surface_make_current(oc_surface surface)
{
	oc_gles_surface_make_current_argptr_stub(&surface);
}

void ORCA_IMPORT(oc_gles_surface_swap_buffers_argptr_stub) (oc_surface* surface);

void oc_gles_surface_swap_buffers(oc_surface surface)
{
	oc_gles_surface_swap_buffers_argptr_stub(&surface);
}

