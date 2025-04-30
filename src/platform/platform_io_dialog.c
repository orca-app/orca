/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "app/app.h"
#include "platform_io_dialog.h"
#include "platform_io_internal.h"

oc_file oc_file_open_with_request_for_table(oc_str8 path, oc_file_access rights, oc_file_open_flags flags, oc_file_table* table)
{
    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8 msg = oc_str8_pushf(scratch.arena, "Application wants to access file '%.*s'.", (int)path.len, path.ptr);

    oc_str8_list options = { 0 };
    oc_str8_list_push(scratch.arena, &options, OC_STR8("Deny"));
    oc_str8_list_push(scratch.arena, &options, OC_STR8("Accept"));

    int res = oc_alert_popup(OC_STR8("File Access"), msg, options);

    oc_file file = oc_file_nil();
    if(res == 1)
    {
        oc_io_cmp cmp = { 0 };

        oc_io_req req = {
            .op = OC_IO_OPEN_AT,
            .size = path.len,
            .buffer = path.ptr,
            .open.rights = rights,
            .open.flags = flags
        };
        cmp = oc_io_open_at(0, &req, table);
        if(cmp.error == OC_IO_OK)
        {
            file = cmp.handle;
        }
    }

    oc_scratch_end(scratch);

    return (file);
}

oc_file oc_file_open_with_request(oc_str8 path, oc_file_access rights, oc_file_open_flags flags)
{
    return (oc_file_open_with_request_for_table(path, rights, flags, &oc_globalFileTable));
}

oc_file_open_with_dialog_result oc_file_open_with_dialog_for_table(oc_arena* arena,
                                                                   oc_file_access rights,
                                                                   oc_file_open_flags flags,
                                                                   oc_file_dialog_desc* desc,
                                                                   oc_file_table* table)
{
    oc_arena_scope scratch = oc_scratch_begin_next(arena);

    oc_file_dialog_result dialogResult = oc_file_dialog_for_table(scratch.arena, desc, table);

    oc_file_open_with_dialog_result result = {
        .button = dialogResult.button
    };

    if(dialogResult.button == OC_FILE_DIALOG_OK)
    {
        int i = 0;
        oc_list_for(dialogResult.selection.list, elt, oc_str8_elt, listElt)
        {
            oc_file file = oc_file_nil();
            if(elt->string.len)
            {
                oc_io_req req = {
                    .op = OC_IO_OPEN_AT,
                    .size = elt->string.len,
                    .buffer = elt->string.ptr,
                    .open.rights = rights,
                    .open.flags = flags
                };

                oc_io_cmp cmp = oc_io_wait_single_req_for_table(&req, table);
                file = cmp.handle;
            }

            oc_file_open_with_dialog_elt* resElt = oc_arena_push_type(arena, oc_file_open_with_dialog_elt);
            resElt->file = file;
            oc_list_push_back(&result.selection, &resElt->listElt);

            if(i == 0)
            {
                result.file = file;
                i++;
            }
        }
    }

    oc_scratch_end(scratch);

    return (result);
}

oc_file_open_with_dialog_result oc_file_open_with_dialog(oc_arena* arena, oc_file_access rights, oc_file_open_flags flags, oc_file_dialog_desc* desc)
{
    return (oc_file_open_with_dialog_for_table(arena, rights, flags, desc, &oc_globalFileTable));
}
