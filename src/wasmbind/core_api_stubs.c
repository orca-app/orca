void ORCA_IMPORT(oc_window_set_title_argptr_stub) (oc_str8* title);

void oc_window_set_title(oc_str8 title)
{
	oc_window_set_title_argptr_stub(&title);
}

void ORCA_IMPORT(oc_window_set_size_argptr_stub) (oc_vec2* size);

void oc_window_set_size(oc_vec2 size)
{
	oc_window_set_size_argptr_stub(&size);
}

void ORCA_IMPORT(oc_clipboard_get_string_argptr_stub) (oc_str8* __retArg, oc_arena* arena);

oc_str8 oc_clipboard_get_string(oc_arena* arena)
{
	oc_str8 __ret;
	oc_clipboard_get_string_argptr_stub(&__ret, arena);
	return(__ret);
}

void ORCA_IMPORT(oc_clipboard_set_string_argptr_stub) (oc_str8* value);

void oc_clipboard_set_string(oc_str8 value)
{
	oc_clipboard_set_string_argptr_stub(&value);
}

