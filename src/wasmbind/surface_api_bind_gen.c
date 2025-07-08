void oc_image_size_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	oc_vec2* __retPtr = (oc_vec2*)((char*)_mem + *(i32*)&_params[0]);
	{
		OC_ASSERT_DIALOG(((char*)__retPtr >= (char*)_mem) && (((char*)__retPtr - (char*)_mem) < oc_wasm_mem_size(wasm)), "return pointer is out of bounds");
		OC_ASSERT_DIALOG((char*)__retPtr + sizeof(oc_vec2) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "return pointer is out of bounds");
	}
	oc_image image = *(oc_image*)((char*)_mem + *(u32*)&_params[1]);
	*__retPtr = oc_image_size(image);
}

void oc_image_create_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	oc_image* __retPtr = (oc_image*)((char*)_mem + *(i32*)&_params[0]);
	{
		OC_ASSERT_DIALOG(((char*)__retPtr >= (char*)_mem) && (((char*)__retPtr - (char*)_mem) < oc_wasm_mem_size(wasm)), "return pointer is out of bounds");
		OC_ASSERT_DIALOG((char*)__retPtr + sizeof(oc_image) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "return pointer is out of bounds");
	}
	oc_canvas_renderer renderer = *(oc_canvas_renderer*)((char*)_mem + *(u32*)&_params[1]);
	u32 width = (u32)*(i32*)&_params[2];
	u32 height = (u32)*(i32*)&_params[3];
	*__retPtr = oc_image_create(renderer, width, height);
}

void oc_image_destroy_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	oc_image image = *(oc_image*)((char*)_mem + *(u32*)&_params[0]);
	oc_image_destroy(image);
}

void oc_image_upload_region_rgba8_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	oc_image image = *(oc_image*)((char*)_mem + *(u32*)&_params[0]);
	oc_rect region = *(oc_rect*)((char*)_mem + *(u32*)&_params[1]);
	u8* pixels = (u8*)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)pixels >= (char*)_mem) && (((char*)pixels - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'pixels' is out of bounds");
		OC_ASSERT_DIALOG((char*)pixels + orca_image_upload_region_rgba8_length(wasm, region)*sizeof(u8) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'pixels' is out of bounds");
	}
	oc_image_upload_region_rgba8(image, region, pixels);
}

void oc_surface_get_size_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	oc_vec2* __retPtr = (oc_vec2*)((char*)_mem + *(i32*)&_params[0]);
	{
		OC_ASSERT_DIALOG(((char*)__retPtr >= (char*)_mem) && (((char*)__retPtr - (char*)_mem) < oc_wasm_mem_size(wasm)), "return pointer is out of bounds");
		OC_ASSERT_DIALOG((char*)__retPtr + sizeof(oc_vec2) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "return pointer is out of bounds");
	}
	oc_surface surface = *(oc_surface*)((char*)_mem + *(u32*)&_params[1]);
	*__retPtr = oc_surface_get_size(surface);
}

void oc_surface_contents_scaling_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	oc_vec2* __retPtr = (oc_vec2*)((char*)_mem + *(i32*)&_params[0]);
	{
		OC_ASSERT_DIALOG(((char*)__retPtr >= (char*)_mem) && (((char*)__retPtr - (char*)_mem) < oc_wasm_mem_size(wasm)), "return pointer is out of bounds");
		OC_ASSERT_DIALOG((char*)__retPtr + sizeof(oc_vec2) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "return pointer is out of bounds");
	}
	oc_surface surface = *(oc_surface*)((char*)_mem + *(u32*)&_params[1]);
	*__retPtr = oc_surface_contents_scaling(surface);
}

void oc_surface_bring_to_front_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	oc_surface surface = *(oc_surface*)((char*)_mem + *(u32*)&_params[0]);
	oc_surface_bring_to_front(surface);
}

void oc_surface_send_to_back_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	oc_surface surface = *(oc_surface*)((char*)_mem + *(u32*)&_params[0]);
	oc_surface_send_to_back(surface);
}

void oc_bridge_canvas_renderer_create_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	oc_canvas_renderer* __retPtr = (oc_canvas_renderer*)((char*)_mem + *(i32*)&_params[0]);
	{
		OC_ASSERT_DIALOG(((char*)__retPtr >= (char*)_mem) && (((char*)__retPtr - (char*)_mem) < oc_wasm_mem_size(wasm)), "return pointer is out of bounds");
		OC_ASSERT_DIALOG((char*)__retPtr + sizeof(oc_canvas_renderer) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "return pointer is out of bounds");
	}
	*__retPtr = oc_bridge_canvas_renderer_create();
}

void oc_bridge_canvas_surface_create_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	oc_surface* __retPtr = (oc_surface*)((char*)_mem + *(i32*)&_params[0]);
	{
		OC_ASSERT_DIALOG(((char*)__retPtr >= (char*)_mem) && (((char*)__retPtr - (char*)_mem) < oc_wasm_mem_size(wasm)), "return pointer is out of bounds");
		OC_ASSERT_DIALOG((char*)__retPtr + sizeof(oc_surface) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "return pointer is out of bounds");
	}
	oc_canvas_renderer renderer = *(oc_canvas_renderer*)((char*)_mem + *(u32*)&_params[1]);
	*__retPtr = oc_bridge_canvas_surface_create(renderer);
}

void oc_bridge_canvas_renderer_submit_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	oc_canvas_renderer renderer = *(oc_canvas_renderer*)((char*)_mem + *(u32*)&_params[0]);
	oc_surface surface = *(oc_surface*)((char*)_mem + *(u32*)&_params[1]);
	u32 msaaSampleCount = (u32)*(i32*)&_params[2];
	bool clear = (bool)*(i32*)&_params[3];
	oc_color clearColor = *(oc_color*)((char*)_mem + *(u32*)&_params[4]);
	u32 primitiveCount = (u32)*(i32*)&_params[5];
	oc_primitive* primitives = (oc_primitive*)((char*)_mem + *(u32*)&_params[6]);
	u32 eltCount = (u32)*(i32*)&_params[7];
	oc_path_elt* elements = (oc_path_elt*)((char*)_mem + *(u32*)&_params[8]);
	{
		OC_ASSERT_DIALOG(((char*)primitives >= (char*)_mem) && (((char*)primitives - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'primitives' is out of bounds");
		OC_ASSERT_DIALOG((char*)primitives + primitiveCount*sizeof(oc_primitive) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'primitives' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)elements >= (char*)_mem) && (((char*)elements - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'elements' is out of bounds");
		OC_ASSERT_DIALOG((char*)elements + eltCount*sizeof(oc_path_elt) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'elements' is out of bounds");
	}
	oc_bridge_canvas_renderer_submit(renderer, surface, msaaSampleCount, clear, clearColor, primitiveCount, primitives, eltCount, elements);
}

void oc_canvas_present_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	oc_canvas_renderer renderer = *(oc_canvas_renderer*)((char*)_mem + *(u32*)&_params[0]);
	oc_surface surface = *(oc_surface*)((char*)_mem + *(u32*)&_params[1]);
	oc_canvas_present(renderer, surface);
}

void oc_bridge_gles_surface_create_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	oc_surface* __retPtr = (oc_surface*)((char*)_mem + *(i32*)&_params[0]);
	{
		OC_ASSERT_DIALOG(((char*)__retPtr >= (char*)_mem) && (((char*)__retPtr - (char*)_mem) < oc_wasm_mem_size(wasm)), "return pointer is out of bounds");
		OC_ASSERT_DIALOG((char*)__retPtr + sizeof(oc_surface) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "return pointer is out of bounds");
	}
	*__retPtr = oc_bridge_gles_surface_create();
}

void oc_gles_surface_make_current_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	oc_surface surface = *(oc_surface*)((char*)_mem + *(u32*)&_params[0]);
	oc_gles_surface_make_current(surface);
}

void oc_gles_surface_swap_buffers_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	oc_surface surface = *(oc_surface*)((char*)_mem + *(u32*)&_params[0]);
	oc_gles_surface_swap_buffers(surface);
}

int bindgen_link_surface_api(oc_wasm* wasm)
{
	oc_wasm_status status;
	int ret = 0;

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_image_size_argptr_stub");
		binding.proc = oc_image_size_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_image_size_argptr_stub (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_image_create_argptr_stub");
		binding.proc = oc_image_create_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_image_create_argptr_stub (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_image_destroy_argptr_stub");
		binding.proc = oc_image_destroy_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_image_destroy_argptr_stub (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_image_upload_region_rgba8_argptr_stub");
		binding.proc = oc_image_upload_region_rgba8_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_image_upload_region_rgba8_argptr_stub (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_surface_get_size_argptr_stub");
		binding.proc = oc_surface_get_size_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_surface_get_size_argptr_stub (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_surface_contents_scaling_argptr_stub");
		binding.proc = oc_surface_contents_scaling_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_surface_contents_scaling_argptr_stub (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_surface_bring_to_front_argptr_stub");
		binding.proc = oc_surface_bring_to_front_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_surface_bring_to_front_argptr_stub (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_surface_send_to_back_argptr_stub");
		binding.proc = oc_surface_send_to_back_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_surface_send_to_back_argptr_stub (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_canvas_renderer_create_argptr_stub");
		binding.proc = oc_bridge_canvas_renderer_create_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_canvas_renderer_create_argptr_stub (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_canvas_surface_create_argptr_stub");
		binding.proc = oc_bridge_canvas_surface_create_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_canvas_surface_create_argptr_stub (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_canvas_renderer_submit_argptr_stub");
		binding.proc = oc_bridge_canvas_renderer_submit_stub;
		binding.countParams = 9;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_canvas_renderer_submit_argptr_stub (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_canvas_present_argptr_stub");
		binding.proc = oc_canvas_present_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_canvas_present_argptr_stub (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_gles_surface_create_argptr_stub");
		binding.proc = oc_bridge_gles_surface_create_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_gles_surface_create_argptr_stub (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_gles_surface_make_current_argptr_stub");
		binding.proc = oc_gles_surface_make_current_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_gles_surface_make_current_argptr_stub (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_gles_surface_swap_buffers_argptr_stub");
		binding.proc = oc_gles_surface_swap_buffers_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_gles_surface_swap_buffers_argptr_stub (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	return(ret);
}
