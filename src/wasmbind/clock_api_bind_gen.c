void oc_clock_time_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	oc_clock_kind clock = (oc_clock_kind)*(i32*)&_params[0];
	*((f64*)&_returns[0]) = (f64)oc_clock_time(clock);
}

int bindgen_link_clock_api(oc_wasm* wasm)
{
	oc_wasm_status status;
	int ret = 0;

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_F64 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("oc_clock_time");
		binding.proc = oc_clock_time_stub;
		binding.countParams = 1;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function oc_clock_time (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	return(ret);
}
