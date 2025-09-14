void oc_clock_time_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	oc_clock_kind clock = (oc_clock_kind)*(i32*)&_params[0];
	*((f64*)&_returns[0]) = (f64)oc_clock_time(clock);
}

int bindgen_link_clock_api(oc_arena* arena, wa_import_package* package)
{
	wa_status status;
	int ret = 0;

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[] = { WA_TYPE_F64 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("oc_clock_time");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = oc_clock_time_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	return(ret);
}
