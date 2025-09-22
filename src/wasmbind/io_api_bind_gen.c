void oc_bridge_io_wait_single_req_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	oc_io_cmp* __retPtr = (oc_io_cmp*)((char*)_mem + *(i32*)&_params[0]);
	{
		OC_ASSERT_DIALOG(((char*)__retPtr >= (char*)_mem) && (((char*)__retPtr - (char*)_mem) < _memSize), "return pointer is out of bounds");
		OC_ASSERT_DIALOG((char*)__retPtr + sizeof(oc_io_cmp) <= ((char*)_mem + _memSize), "return pointer is out of bounds");
	}
	oc_io_req* req = (oc_io_req*)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)req >= (char*)_mem) && (((char*)req - (char*)_mem) < _memSize), "parameter 'req' is out of bounds");
		OC_ASSERT_DIALOG((char*)req + 1*sizeof(oc_io_req) <= ((char*)_mem + _memSize), "parameter 'req' is out of bounds");
	}
	*__retPtr = oc_bridge_io_wait_single_req(req);
}

void oc_file_open_with_request_bridge_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	oc_file* __retPtr = (oc_file*)((char*)_mem + *(i32*)&_params[0]);
	{
		OC_ASSERT_DIALOG(((char*)__retPtr >= (char*)_mem) && (((char*)__retPtr - (char*)_mem) < _memSize), "return pointer is out of bounds");
		OC_ASSERT_DIALOG((char*)__retPtr + sizeof(oc_file) <= ((char*)_mem + _memSize), "return pointer is out of bounds");
	}
	oc_wasm_str8 path = *(oc_wasm_str8*)((char*)_mem + *(u32*)&_params[1]);
	oc_file_access rights = (oc_file_access)*(i32*)&_params[2];
	oc_file_open_flags flags = (oc_file_open_flags)*(i32*)&_params[3];
	*__retPtr = oc_file_open_with_request_bridge(path, rights, flags);
}

void oc_file_open_with_dialog_bridge_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	oc_wasm_file_open_with_dialog_result* __retPtr = (oc_wasm_file_open_with_dialog_result*)((char*)_mem + *(i32*)&_params[0]);
	{
		OC_ASSERT_DIALOG(((char*)__retPtr >= (char*)_mem) && (((char*)__retPtr - (char*)_mem) < _memSize), "return pointer is out of bounds");
		OC_ASSERT_DIALOG((char*)__retPtr + sizeof(oc_wasm_file_open_with_dialog_result) <= ((char*)_mem + _memSize), "return pointer is out of bounds");
	}
	i32 arena = (i32)*(i32*)&_params[1];
	oc_file_access rights = (oc_file_access)*(i32*)&_params[2];
	oc_file_open_flags flags = (oc_file_open_flags)*(i32*)&_params[3];
	oc_wasm_file_dialog_desc* desc = (oc_wasm_file_dialog_desc*)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)desc >= (char*)_mem) && (((char*)desc - (char*)_mem) < _memSize), "parameter 'desc' is out of bounds");
		OC_ASSERT_DIALOG((char*)desc + 1*sizeof(oc_wasm_file_dialog_desc) <= ((char*)_mem + _memSize), "parameter 'desc' is out of bounds");
	}
	*__retPtr = oc_file_open_with_dialog_bridge(arena, rights, flags, desc);
}

void oc_file_listdir_bridge_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	oc_wasm_file_list* __retPtr = (oc_wasm_file_list*)((char*)_mem + *(i32*)&_params[0]);
	{
		OC_ASSERT_DIALOG(((char*)__retPtr >= (char*)_mem) && (((char*)__retPtr - (char*)_mem) < _memSize), "return pointer is out of bounds");
		OC_ASSERT_DIALOG((char*)__retPtr + sizeof(oc_wasm_file_list) <= ((char*)_mem + _memSize), "return pointer is out of bounds");
	}
	i32 arena = (i32)*(i32*)&_params[1];
	oc_file directory = *(oc_file*)((char*)_mem + *(u32*)&_params[2]);
	*__retPtr = oc_file_listdir_bridge(arena, directory);
}

int bindgen_link_io_api(oc_arena* arena, wa_import_package* package)
{
	wa_status status;
	int ret = 0;

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("oc_io_wait_single_req_argptr_stub");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_bridge_io_wait_single_req_stub;
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
		binding.name = OC_STR8("oc_file_open_with_request_argptr_stub");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_file_open_with_request_bridge_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("oc_file_open_with_dialog_argptr_stub");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_file_open_with_dialog_bridge_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("oc_file_listdir_argptr_stub");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_file_listdir_bridge_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	return(ret);
}
