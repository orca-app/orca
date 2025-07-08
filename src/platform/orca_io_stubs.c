#include "platform/platform_io_dialog.h"

void ORCA_IMPORT(oc_io_wait_single_req_argptr_stub) (oc_io_cmp* __retArg, oc_io_req* req);

oc_io_cmp oc_io_wait_single_req(oc_io_req* req)
{
	oc_io_cmp __ret;
	oc_io_wait_single_req_argptr_stub(&__ret, req);
	return(__ret);
}

void ORCA_IMPORT(oc_file_open_with_request_argptr_stub) (oc_file* __retArg, oc_str8* path, oc_file_access rights, oc_file_open_flags flags);

oc_file oc_file_open_with_request(oc_str8 path, oc_file_access rights, oc_file_open_flags flags)
{
	oc_file __ret;
	oc_file_open_with_request_argptr_stub(&__ret, &path, rights, flags);
	return(__ret);
}

void ORCA_IMPORT(oc_file_open_with_dialog_argptr_stub) (oc_file_open_with_dialog_result* __retArg, oc_arena* arena, oc_file_access rights, oc_file_open_flags flags, oc_file_dialog_desc* desc);

oc_file_open_with_dialog_result oc_file_open_with_dialog(oc_arena* arena, oc_file_access rights, oc_file_open_flags flags, oc_file_dialog_desc* desc)
{
	oc_file_open_with_dialog_result __ret;
	oc_file_open_with_dialog_argptr_stub(&__ret, arena, rights, flags, desc);
	return(__ret);
}

