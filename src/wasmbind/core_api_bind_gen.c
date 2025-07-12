void oc_bridge_log_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	oc_log_level level = (oc_log_level)*(i32*)&_params[0];
	int functionLen = (int)*(i32*)&_params[1];
	char* function = (char*)((char*)_mem + *(u32*)&_params[2]);
	int fileLen = (int)*(i32*)&_params[3];
	char* file = (char*)((char*)_mem + *(u32*)&_params[4]);
	int line = (int)*(i32*)&_params[5];
	int msgLen = (int)*(i32*)&_params[6];
	char* msg = (char*)((char*)_mem + *(u32*)&_params[7]);
	{
		OC_ASSERT_DIALOG(((char*)function >= (char*)_mem) && (((char*)function - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'function' is out of bounds");
		OC_ASSERT_DIALOG((char*)function + functionLen*sizeof(char) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'function' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)file >= (char*)_mem) && (((char*)file - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'file' is out of bounds");
		OC_ASSERT_DIALOG((char*)file + fileLen*sizeof(char) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'file' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)msg >= (char*)_mem) && (((char*)msg - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'msg' is out of bounds");
		OC_ASSERT_DIALOG((char*)msg + msgLen*sizeof(char) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'msg' is out of bounds");
	}
	oc_bridge_log(level, functionLen, function, fileLen, file, line, msgLen, msg);
}

void oc_mem_grow_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	u64 size = (u64)*(i64*)&_params[0];
	*((i32*)&_returns[0]) = (i32)oc_mem_grow(size);
}

void oc_assert_fail_dialog_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	const char* file = (const char*)((char*)_mem + *(u32*)&_params[0]);
	const char* function = (const char*)((char*)_mem + *(u32*)&_params[1]);
	int line = (int)*(i32*)&_params[2];
	const char* src = (const char*)((char*)_mem + *(u32*)&_params[3]);
	const char* note = (const char*)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)file >= (char*)_mem) && (((char*)file - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'file' is out of bounds");
		OC_ASSERT_DIALOG((char*)file + orca_check_cstring(wasm, file)*sizeof(const char) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'file' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)function >= (char*)_mem) && (((char*)function - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'function' is out of bounds");
		OC_ASSERT_DIALOG((char*)function + orca_check_cstring(wasm, function)*sizeof(const char) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'function' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)src >= (char*)_mem) && (((char*)src - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'src' is out of bounds");
		OC_ASSERT_DIALOG((char*)src + orca_check_cstring(wasm, src)*sizeof(const char) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'src' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)note >= (char*)_mem) && (((char*)note - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'note' is out of bounds");
		OC_ASSERT_DIALOG((char*)note + orca_check_cstring(wasm, note)*sizeof(const char) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'note' is out of bounds");
	}
	oc_assert_fail_dialog(file, function, line, src, note);
}

void oc_bridge_exit_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	int ec = (int)*(i32*)&_params[0];
	oc_bridge_exit(ec);
}

void oc_abort_ext_dialog_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	const char* file = (const char*)((char*)_mem + *(u32*)&_params[0]);
	const char* function = (const char*)((char*)_mem + *(u32*)&_params[1]);
	int line = (int)*(i32*)&_params[2];
	const char* note = (const char*)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)file >= (char*)_mem) && (((char*)file - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'file' is out of bounds");
		OC_ASSERT_DIALOG((char*)file + orca_check_cstring(wasm, file)*sizeof(const char) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'file' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)function >= (char*)_mem) && (((char*)function - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'function' is out of bounds");
		OC_ASSERT_DIALOG((char*)function + orca_check_cstring(wasm, function)*sizeof(const char) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'function' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)note >= (char*)_mem) && (((char*)note - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'note' is out of bounds");
		OC_ASSERT_DIALOG((char*)note + orca_check_cstring(wasm, note)*sizeof(const char) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'note' is out of bounds");
	}
	oc_abort_ext_dialog(file, function, line, note);
}

void oc_get_host_platform_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	*((i32*)&_returns[0]) = (i32)oc_get_host_platform();
}

void oc_bridge_request_quit_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	oc_bridge_request_quit();
}

void oc_bridge_window_set_title_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	oc_wasm_str8 title = *(oc_wasm_str8*)((char*)_mem + *(u32*)&_params[0]);
	oc_bridge_window_set_title(title);
}

void oc_bridge_window_set_size_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	oc_vec2 size = *(oc_vec2*)((char*)_mem + *(u32*)&_params[0]);
	oc_bridge_window_set_size(size);
}

void oc_scancode_to_keycode_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	oc_scan_code scanCode = (oc_scan_code)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)oc_scancode_to_keycode(scanCode);
}

void oc_bridge_clipboard_get_string_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	oc_wasm_str8* __retPtr = (oc_wasm_str8*)((char*)_mem + *(i32*)&_params[0]);
	{
		OC_ASSERT_DIALOG(((char*)__retPtr >= (char*)_mem) && (((char*)__retPtr - (char*)_mem) < oc_wasm_mem_size(wasm)), "return pointer is out of bounds");
		OC_ASSERT_DIALOG((char*)__retPtr + sizeof(oc_wasm_str8) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "return pointer is out of bounds");
	}
	i32 arena = (i32)*(i32*)&_params[1];
	*__retPtr = oc_bridge_clipboard_get_string(arena);
}

void oc_bridge_clipboard_set_string_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	oc_wasm_str8 value = *(oc_wasm_str8*)((char*)_mem + *(u32*)&_params[0]);
	oc_bridge_clipboard_set_string(value);
}

int bindgen_link_core_api(oc_wasm* wasm)
{
	oc_wasm_status status;
	int ret = 0;

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_bridge_log");
		binding.proc = oc_bridge_log_stub;
		binding.countParams = 8;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_bridge_log (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I64, };
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_I32 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_mem_grow");
		binding.proc = oc_mem_grow_stub;
		binding.countParams = 1;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_mem_grow (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_bridge_assert_fail");
		binding.proc = oc_assert_fail_dialog_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_bridge_assert_fail (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_bridge_exit");
		binding.proc = oc_bridge_exit_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_bridge_exit (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_bridge_abort_ext");
		binding.proc = oc_abort_ext_dialog_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_bridge_abort_ext (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[1];
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_I32 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_get_host_platform");
		binding.proc = oc_get_host_platform_stub;
		binding.countParams = 0;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_get_host_platform (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[1];
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_request_quit");
		binding.proc = oc_bridge_request_quit_stub;
		binding.countParams = 0;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_request_quit (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_window_set_title_argptr_stub");
		binding.proc = oc_bridge_window_set_title_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_window_set_title_argptr_stub (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_window_set_size_argptr_stub");
		binding.proc = oc_bridge_window_set_size_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_window_set_size_argptr_stub (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_I32 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_scancode_to_keycode");
		binding.proc = oc_scancode_to_keycode_stub;
		binding.countParams = 1;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_scancode_to_keycode (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_clipboard_get_string_argptr_stub");
		binding.proc = oc_bridge_clipboard_get_string_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_clipboard_get_string_argptr_stub (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_clipboard_set_string_argptr_stub");
		binding.proc = oc_bridge_clipboard_set_string_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_clipboard_set_string_argptr_stub (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	return(ret);
}
