void oc_bridge_io_wait_single_req_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	oc_io_cmp* __retPtr = (oc_io_cmp*)((char*)_mem + *(i32*)&_params[0]);
	{
		OC_ASSERT_DIALOG(((char*)__retPtr >= (char*)_mem) && (((char*)__retPtr - (char*)_mem) < oc_wasm_mem_size(wasm)), "return pointer is out of bounds");
		OC_ASSERT_DIALOG((char*)__retPtr + sizeof(oc_io_cmp) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "return pointer is out of bounds");
	}
	oc_io_req* req = (oc_io_req*)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)req >= (char*)_mem) && (((char*)req - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'req' is out of bounds");
		OC_ASSERT_DIALOG((char*)req + 1*sizeof(oc_io_req) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'req' is out of bounds");
	}
	*__retPtr = oc_bridge_io_wait_single_req(req);
}

void oc_file_open_with_request_bridge_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	oc_file* __retPtr = (oc_file*)((char*)_mem + *(i32*)&_params[0]);
	{
		OC_ASSERT_DIALOG(((char*)__retPtr >= (char*)_mem) && (((char*)__retPtr - (char*)_mem) < oc_wasm_mem_size(wasm)), "return pointer is out of bounds");
		OC_ASSERT_DIALOG((char*)__retPtr + sizeof(oc_file) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "return pointer is out of bounds");
	}
	oc_wasm_str8 path = *(oc_wasm_str8*)((char*)_mem + *(u32*)&_params[1]);
	oc_file_access rights = (oc_file_access)*(i32*)&_params[2];
	oc_file_open_flags flags = (oc_file_open_flags)*(i32*)&_params[3];
	*__retPtr = oc_file_open_with_request_bridge(path, rights, flags);
}

void oc_file_open_with_dialog_bridge_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	oc_wasm_file_open_with_dialog_result* __retPtr = (oc_wasm_file_open_with_dialog_result*)((char*)_mem + *(i32*)&_params[0]);
	{
		OC_ASSERT_DIALOG(((char*)__retPtr >= (char*)_mem) && (((char*)__retPtr - (char*)_mem) < oc_wasm_mem_size(wasm)), "return pointer is out of bounds");
		OC_ASSERT_DIALOG((char*)__retPtr + sizeof(oc_wasm_file_open_with_dialog_result) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "return pointer is out of bounds");
	}
	i32 arena = (i32)*(i32*)&_params[1];
	oc_file_access rights = (oc_file_access)*(i32*)&_params[2];
	oc_file_open_flags flags = (oc_file_open_flags)*(i32*)&_params[3];
	oc_wasm_file_dialog_desc* desc = (oc_wasm_file_dialog_desc*)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)desc >= (char*)_mem) && (((char*)desc - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'desc' is out of bounds");
		OC_ASSERT_DIALOG((char*)desc + 1*sizeof(oc_wasm_file_dialog_desc) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'desc' is out of bounds");
	}
	*__retPtr = oc_file_open_with_dialog_bridge(arena, rights, flags, desc);
}

void oc_file_listdir_bridge_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	oc_wasm_file_list* __retPtr = (oc_wasm_file_list*)((char*)_mem + *(i32*)&_params[0]);
	{
		OC_ASSERT_DIALOG(((char*)__retPtr >= (char*)_mem) && (((char*)__retPtr - (char*)_mem) < oc_wasm_mem_size(wasm)), "return pointer is out of bounds");
		OC_ASSERT_DIALOG((char*)__retPtr + sizeof(oc_wasm_file_list) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "return pointer is out of bounds");
	}
	i32 arena = (i32)*(i32*)&_params[1];
	oc_file directory = *(oc_file*)((char*)_mem + *(u32*)&_params[2]);
	*__retPtr = oc_file_listdir_bridge(arena, directory);
}

int bindgen_link_io_api(oc_wasm* wasm)
{
	oc_wasm_status status;
	int ret = 0;

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_io_wait_single_req_argptr_stub");
		binding.proc = oc_bridge_io_wait_single_req_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_io_wait_single_req_argptr_stub (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_file_open_with_request_argptr_stub");
		binding.proc = oc_file_open_with_request_bridge_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_file_open_with_request_argptr_stub (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_file_open_with_dialog_argptr_stub");
		binding.proc = oc_file_open_with_dialog_bridge_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_file_open_with_dialog_argptr_stub (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_file_listdir_argptr_stub");
		binding.proc = oc_file_listdir_bridge_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_file_listdir_argptr_stub (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	return(ret);
}
