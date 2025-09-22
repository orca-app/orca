void oc_bridge_log_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	oc_log_level level = (oc_log_level)*(i32*)&_params[0];
	int functionLen = (int)*(i32*)&_params[1];
	char* function = (char*)((char*)_mem + *(u32*)&_params[2]);
	int fileLen = (int)*(i32*)&_params[3];
	char* file = (char*)((char*)_mem + *(u32*)&_params[4]);
	int line = (int)*(i32*)&_params[5];
	int msgLen = (int)*(i32*)&_params[6];
	char* msg = (char*)((char*)_mem + *(u32*)&_params[7]);
	{
		OC_ASSERT_DIALOG(((char*)function >= (char*)_mem) && (((char*)function - (char*)_mem) < _memSize), "parameter 'function' is out of bounds");
		OC_ASSERT_DIALOG((char*)function + functionLen*sizeof(char) <= ((char*)_mem + _memSize), "parameter 'function' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)file >= (char*)_mem) && (((char*)file - (char*)_mem) < _memSize), "parameter 'file' is out of bounds");
		OC_ASSERT_DIALOG((char*)file + fileLen*sizeof(char) <= ((char*)_mem + _memSize), "parameter 'file' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)msg >= (char*)_mem) && (((char*)msg - (char*)_mem) < _memSize), "parameter 'msg' is out of bounds");
		OC_ASSERT_DIALOG((char*)msg + msgLen*sizeof(char) <= ((char*)_mem + _memSize), "parameter 'msg' is out of bounds");
	}
	oc_bridge_log(level, functionLen, function, fileLen, file, line, msgLen, msg);
}

void oc_mem_grow_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	u64 size = (u64)*(i64*)&_params[0];
	*((i32*)&_returns[0]) = (i32)oc_mem_grow(size);
}

void oc_assert_fail_dialog_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	const char* file = (const char*)((char*)_mem + *(u32*)&_params[0]);
	const char* function = (const char*)((char*)_mem + *(u32*)&_params[1]);
	int line = (int)*(i32*)&_params[2];
	const char* src = (const char*)((char*)_mem + *(u32*)&_params[3]);
	const char* note = (const char*)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)file >= (char*)_mem) && (((char*)file - (char*)_mem) < _memSize), "parameter 'file' is out of bounds");
		OC_ASSERT_DIALOG((char*)file + orca_check_cstring(instance, file)*sizeof(const char) <= ((char*)_mem + _memSize), "parameter 'file' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)function >= (char*)_mem) && (((char*)function - (char*)_mem) < _memSize), "parameter 'function' is out of bounds");
		OC_ASSERT_DIALOG((char*)function + orca_check_cstring(instance, function)*sizeof(const char) <= ((char*)_mem + _memSize), "parameter 'function' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)src >= (char*)_mem) && (((char*)src - (char*)_mem) < _memSize), "parameter 'src' is out of bounds");
		OC_ASSERT_DIALOG((char*)src + orca_check_cstring(instance, src)*sizeof(const char) <= ((char*)_mem + _memSize), "parameter 'src' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)note >= (char*)_mem) && (((char*)note - (char*)_mem) < _memSize), "parameter 'note' is out of bounds");
		OC_ASSERT_DIALOG((char*)note + orca_check_cstring(instance, note)*sizeof(const char) <= ((char*)_mem + _memSize), "parameter 'note' is out of bounds");
	}
	oc_assert_fail_dialog(file, function, line, src, note);
}

void oc_bridge_exit_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	int ec = (int)*(i32*)&_params[0];
	oc_bridge_exit(ec);
}

void oc_abort_ext_dialog_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	const char* file = (const char*)((char*)_mem + *(u32*)&_params[0]);
	const char* function = (const char*)((char*)_mem + *(u32*)&_params[1]);
	int line = (int)*(i32*)&_params[2];
	const char* note = (const char*)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)file >= (char*)_mem) && (((char*)file - (char*)_mem) < _memSize), "parameter 'file' is out of bounds");
		OC_ASSERT_DIALOG((char*)file + orca_check_cstring(instance, file)*sizeof(const char) <= ((char*)_mem + _memSize), "parameter 'file' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)function >= (char*)_mem) && (((char*)function - (char*)_mem) < _memSize), "parameter 'function' is out of bounds");
		OC_ASSERT_DIALOG((char*)function + orca_check_cstring(instance, function)*sizeof(const char) <= ((char*)_mem + _memSize), "parameter 'function' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)note >= (char*)_mem) && (((char*)note - (char*)_mem) < _memSize), "parameter 'note' is out of bounds");
		OC_ASSERT_DIALOG((char*)note + orca_check_cstring(instance, note)*sizeof(const char) <= ((char*)_mem + _memSize), "parameter 'note' is out of bounds");
	}
	oc_abort_ext_dialog(file, function, line, note);
}

void oc_get_host_platform_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	*((i32*)&_returns[0]) = (i32)oc_get_host_platform();
}

void oc_bridge_request_quit_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	oc_bridge_request_quit();
}

void oc_bridge_window_set_title_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	oc_wasm_str8 title = *(oc_wasm_str8*)((char*)_mem + *(u32*)&_params[0]);
	oc_bridge_window_set_title(title);
}

void oc_bridge_window_set_size_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	oc_vec2 size = *(oc_vec2*)((char*)_mem + *(u32*)&_params[0]);
	oc_bridge_window_set_size(size);
}

void oc_scancode_to_keycode_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	oc_scan_code scanCode = (oc_scan_code)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)oc_scancode_to_keycode(scanCode);
}

void oc_bridge_clipboard_get_string_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	oc_wasm_str8* __retPtr = (oc_wasm_str8*)((char*)_mem + *(i32*)&_params[0]);
	{
		OC_ASSERT_DIALOG(((char*)__retPtr >= (char*)_mem) && (((char*)__retPtr - (char*)_mem) < _memSize), "return pointer is out of bounds");
		OC_ASSERT_DIALOG((char*)__retPtr + sizeof(oc_wasm_str8) <= ((char*)_mem + _memSize), "return pointer is out of bounds");
	}
	i32 arena = (i32)*(i32*)&_params[1];
	*__retPtr = oc_bridge_clipboard_get_string(arena);
}

void oc_bridge_clipboard_set_string_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	oc_wasm_str8 value = *(oc_wasm_str8*)((char*)_mem + *(u32*)&_params[0]);
	oc_bridge_clipboard_set_string(value);
}

int bindgen_link_core_api(oc_arena* arena, wa_import_package* package)
{
	wa_status status;
	int ret = 0;

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("oc_bridge_log");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_bridge_log_stub;
		binding.hostFunction.type.paramCount = 8;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I64, };
		wa_value_type returnTypes[] = { WA_TYPE_I32 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("oc_mem_grow");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_mem_grow_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("oc_bridge_assert_fail");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_assert_fail_dialog_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("oc_bridge_exit");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_bridge_exit_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("oc_bridge_abort_ext");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_abort_ext_dialog_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[1];
		wa_value_type returnTypes[] = { WA_TYPE_I32 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("oc_get_host_platform");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_get_host_platform_stub;
		binding.hostFunction.type.paramCount = 0;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[1];
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("oc_request_quit");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_bridge_request_quit_stub;
		binding.hostFunction.type.paramCount = 0;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("oc_window_set_title_argptr_stub");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_bridge_window_set_title_stub;
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
		binding.name = OC_STR8("oc_window_set_size_argptr_stub");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_bridge_window_set_size_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[] = { WA_TYPE_I32 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("oc_scancode_to_keycode");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_scancode_to_keycode_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("oc_clipboard_get_string_argptr_stub");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_bridge_clipboard_get_string_stub;
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
		binding.name = OC_STR8("oc_clipboard_set_string_argptr_stub");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_bridge_clipboard_set_string_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	return(ret);
}
