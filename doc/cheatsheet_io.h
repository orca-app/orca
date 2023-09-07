/************************************************************/ /**
*
*	@file: cheatsheet_io.h
*	@author: Martin Fouilleul
*	@date: 05/09/2023
*
*****************************************************************/

//----------------------------------------------------------------
// Low-level File IO API
//----------------------------------------------------------------
oc_io_cmp oc_io_wait_single_req(oc_io_req* req);

//----------------------------------------------------------------
// High-level File IO API
//----------------------------------------------------------------
oc_file oc_file_nil();
bool oc_file_is_nil(oc_file handle);

oc_file oc_file_open(oc_str8 path, oc_file_access rights, oc_file_open_flags flags);
oc_file oc_file_open_at(oc_file dir, oc_str8 path, oc_file_access rights, oc_file_open_flags flags);
void oc_file_close(oc_file file);
oc_io_error oc_file_last_error(oc_file handle);

i64 oc_file_pos(oc_file file);
i64 oc_file_seek(oc_file file, i64 offset, oc_file_whence whence);
u64 oc_file_write(oc_file file, u64 size, char* buffer);
u64 oc_file_read(oc_file file, u64 size, char* buffer);

oc_file_status oc_file_get_status(oc_file file);
u64 oc_file_size(oc_file file);

//----------------------------------------------------------------
// Asking users for file capabilities
//----------------------------------------------------------------
oc_file oc_file_open_with_request(oc_str8 path, oc_file_access rights, oc_file_open_flags flags);
oc_file_open_with_dialog_result oc_file_open_with_dialog(oc_arena* arena, oc_file_access rights, oc_file_open_flags flags, oc_file_dialog_desc* desc);


