#include "platform/platform_io_dialog.h"
#include "wasmbind/hostcalls.h"

void ORCA_IMPORT(oc_io_wait_single_req_argptr_stub)(oc_io_cmp* __retArg, oc_io_req* req);
void ORCA_IMPORT(oc_file_open_with_request_argptr_stub)(oc_file* __retArg, oc_str8* path, oc_file_access rights, oc_file_open_flags flags);
void ORCA_IMPORT(oc_file_open_with_dialog_argptr_stub)(oc_file_open_with_dialog_result* __retArg, oc_arena* arena, oc_file_access rights, oc_file_open_flags flags, oc_file_dialog_desc* desc);
void ORCA_IMPORT(oc_file_listdir_argptr_stub)(oc_file_list* __retArg, oc_arena* arena, oc_file* directory);

oc_io_cmp oc_io_wait_single_req(oc_io_req* req)
{
    oc_io_cmp __ret;
    oc_hostcall_io_wait_single_req(req, &__ret);
    return (__ret);
}

oc_file oc_file_open_with_request(oc_str8 path, oc_file_access rights, oc_file_open_flags flags)
{
    oc_file __ret;
    oc_hostcall_file_open_with_request(&path, rights, flags, &__ret);
    return (__ret);
}

oc_file_open_with_dialog_result oc_file_open_with_dialog(oc_arena* arena, oc_file_access rights, oc_file_open_flags flags, oc_file_dialog_desc* desc)
{
    oc_file_open_with_dialog_result __ret;
    oc_hostcall_file_open_with_dialog(arena, rights, flags, desc, &__ret);
    return (__ret);
}

oc_file_list oc_file_listdir(oc_arena* arena, oc_file directory)
{
    oc_file_list __ret;
    oc_hostcall_file_listdir(arena, &directory, &__ret);
    return (__ret);
}
