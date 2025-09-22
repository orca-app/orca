void oc_image_size_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	oc_vec2* __retPtr = (oc_vec2*)((char*)_mem + *(i32*)&_params[0]);
	{
		OC_ASSERT_DIALOG(((char*)__retPtr >= (char*)_mem) && (((char*)__retPtr - (char*)_mem) < _memSize), "return pointer is out of bounds");
		OC_ASSERT_DIALOG((char*)__retPtr + sizeof(oc_vec2) <= ((char*)_mem + _memSize), "return pointer is out of bounds");
	}
	oc_image image = *(oc_image*)((char*)_mem + *(u32*)&_params[1]);
	*__retPtr = oc_image_size(image);
}

void oc_image_create_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	oc_image* __retPtr = (oc_image*)((char*)_mem + *(i32*)&_params[0]);
	{
		OC_ASSERT_DIALOG(((char*)__retPtr >= (char*)_mem) && (((char*)__retPtr - (char*)_mem) < _memSize), "return pointer is out of bounds");
		OC_ASSERT_DIALOG((char*)__retPtr + sizeof(oc_image) <= ((char*)_mem + _memSize), "return pointer is out of bounds");
	}
	oc_canvas_renderer renderer = *(oc_canvas_renderer*)((char*)_mem + *(u32*)&_params[1]);
	u32 width = (u32)*(i32*)&_params[2];
	u32 height = (u32)*(i32*)&_params[3];
	*__retPtr = oc_image_create(renderer, width, height);
}

void oc_image_destroy_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	oc_image image = *(oc_image*)((char*)_mem + *(u32*)&_params[0]);
	oc_image_destroy(image);
}

void oc_image_upload_region_rgba8_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	oc_image image = *(oc_image*)((char*)_mem + *(u32*)&_params[0]);
	oc_rect region = *(oc_rect*)((char*)_mem + *(u32*)&_params[1]);
	u8* pixels = (u8*)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)pixels >= (char*)_mem) && (((char*)pixels - (char*)_mem) < _memSize), "parameter 'pixels' is out of bounds");
		OC_ASSERT_DIALOG((char*)pixels + orca_image_upload_region_rgba8_length(instance, region)*sizeof(u8) <= ((char*)_mem + _memSize), "parameter 'pixels' is out of bounds");
	}
	oc_image_upload_region_rgba8(image, region, pixels);
}

void oc_surface_get_size_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	oc_vec2* __retPtr = (oc_vec2*)((char*)_mem + *(i32*)&_params[0]);
	{
		OC_ASSERT_DIALOG(((char*)__retPtr >= (char*)_mem) && (((char*)__retPtr - (char*)_mem) < _memSize), "return pointer is out of bounds");
		OC_ASSERT_DIALOG((char*)__retPtr + sizeof(oc_vec2) <= ((char*)_mem + _memSize), "return pointer is out of bounds");
	}
	oc_surface surface = *(oc_surface*)((char*)_mem + *(u32*)&_params[1]);
	*__retPtr = oc_surface_get_size(surface);
}

void oc_surface_contents_scaling_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	oc_vec2* __retPtr = (oc_vec2*)((char*)_mem + *(i32*)&_params[0]);
	{
		OC_ASSERT_DIALOG(((char*)__retPtr >= (char*)_mem) && (((char*)__retPtr - (char*)_mem) < _memSize), "return pointer is out of bounds");
		OC_ASSERT_DIALOG((char*)__retPtr + sizeof(oc_vec2) <= ((char*)_mem + _memSize), "return pointer is out of bounds");
	}
	oc_surface surface = *(oc_surface*)((char*)_mem + *(u32*)&_params[1]);
	*__retPtr = oc_surface_contents_scaling(surface);
}

void oc_surface_bring_to_front_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	oc_surface surface = *(oc_surface*)((char*)_mem + *(u32*)&_params[0]);
	oc_surface_bring_to_front(surface);
}

void oc_surface_send_to_back_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	oc_surface surface = *(oc_surface*)((char*)_mem + *(u32*)&_params[0]);
	oc_surface_send_to_back(surface);
}

void oc_bridge_canvas_renderer_create_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	oc_canvas_renderer* __retPtr = (oc_canvas_renderer*)((char*)_mem + *(i32*)&_params[0]);
	{
		OC_ASSERT_DIALOG(((char*)__retPtr >= (char*)_mem) && (((char*)__retPtr - (char*)_mem) < _memSize), "return pointer is out of bounds");
		OC_ASSERT_DIALOG((char*)__retPtr + sizeof(oc_canvas_renderer) <= ((char*)_mem + _memSize), "return pointer is out of bounds");
	}
	*__retPtr = oc_bridge_canvas_renderer_create();
}

void oc_bridge_canvas_surface_create_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	oc_surface* __retPtr = (oc_surface*)((char*)_mem + *(i32*)&_params[0]);
	{
		OC_ASSERT_DIALOG(((char*)__retPtr >= (char*)_mem) && (((char*)__retPtr - (char*)_mem) < _memSize), "return pointer is out of bounds");
		OC_ASSERT_DIALOG((char*)__retPtr + sizeof(oc_surface) <= ((char*)_mem + _memSize), "return pointer is out of bounds");
	}
	oc_canvas_renderer renderer = *(oc_canvas_renderer*)((char*)_mem + *(u32*)&_params[1]);
	*__retPtr = oc_bridge_canvas_surface_create(renderer);
}

void oc_bridge_canvas_renderer_submit_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
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
		OC_ASSERT_DIALOG(((char*)primitives >= (char*)_mem) && (((char*)primitives - (char*)_mem) < _memSize), "parameter 'primitives' is out of bounds");
		OC_ASSERT_DIALOG((char*)primitives + primitiveCount*sizeof(oc_primitive) <= ((char*)_mem + _memSize), "parameter 'primitives' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)elements >= (char*)_mem) && (((char*)elements - (char*)_mem) < _memSize), "parameter 'elements' is out of bounds");
		OC_ASSERT_DIALOG((char*)elements + eltCount*sizeof(oc_path_elt) <= ((char*)_mem + _memSize), "parameter 'elements' is out of bounds");
	}
	oc_bridge_canvas_renderer_submit(renderer, surface, msaaSampleCount, clear, clearColor, primitiveCount, primitives, eltCount, elements);
}

void oc_canvas_present_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	oc_canvas_renderer renderer = *(oc_canvas_renderer*)((char*)_mem + *(u32*)&_params[0]);
	oc_surface surface = *(oc_surface*)((char*)_mem + *(u32*)&_params[1]);
	oc_canvas_present(renderer, surface);
}

void oc_bridge_gles_surface_create_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	oc_surface* __retPtr = (oc_surface*)((char*)_mem + *(i32*)&_params[0]);
	{
		OC_ASSERT_DIALOG(((char*)__retPtr >= (char*)_mem) && (((char*)__retPtr - (char*)_mem) < _memSize), "return pointer is out of bounds");
		OC_ASSERT_DIALOG((char*)__retPtr + sizeof(oc_surface) <= ((char*)_mem + _memSize), "return pointer is out of bounds");
	}
	*__retPtr = oc_bridge_gles_surface_create();
}

void oc_gles_surface_make_current_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	oc_surface surface = *(oc_surface*)((char*)_mem + *(u32*)&_params[0]);
	oc_gles_surface_make_current(surface);
}

void oc_gles_surface_swap_buffers_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	oc_surface surface = *(oc_surface*)((char*)_mem + *(u32*)&_params[0]);
	oc_gles_surface_swap_buffers(surface);
}

int bindgen_link_surface_api(oc_arena* arena, wa_import_package* package)
{
	wa_status status;
	int ret = 0;

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("oc_image_size_argptr_stub");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_image_size_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("oc_image_create_argptr_stub");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_image_create_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("oc_image_destroy_argptr_stub");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_image_destroy_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("oc_image_upload_region_rgba8_argptr_stub");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_image_upload_region_rgba8_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("oc_surface_get_size_argptr_stub");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_surface_get_size_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("oc_surface_contents_scaling_argptr_stub");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_surface_contents_scaling_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("oc_surface_bring_to_front_argptr_stub");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_surface_bring_to_front_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("oc_surface_send_to_back_argptr_stub");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_surface_send_to_back_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("oc_canvas_renderer_create_argptr_stub");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_bridge_canvas_renderer_create_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("oc_canvas_surface_create_argptr_stub");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_bridge_canvas_surface_create_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("oc_canvas_renderer_submit_argptr_stub");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_bridge_canvas_renderer_submit_stub;
		binding.hostFunction.type.paramCount = 9;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("oc_canvas_present_argptr_stub");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_canvas_present_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("oc_gles_surface_create_argptr_stub");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_bridge_gles_surface_create_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("oc_gles_surface_make_current_argptr_stub");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_gles_surface_make_current_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("oc_gles_surface_swap_buffers_argptr_stub");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_gles_surface_swap_buffers_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	return(ret);
}
