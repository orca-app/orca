/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "orca.h"

int main(int argc, char** argv)
{
    oc_init();

    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8 path = oc_path_executable_relative(scratch.arena, OC_STR8("../"));
    oc_file dir = oc_file_open(path, OC_FILE_ACCESS_READ, 0);

    oc_file_dialog_desc desc = {
        .kind = OC_FILE_DIALOG_OPEN,
        .flags = OC_FILE_DIALOG_FILES | OC_FILE_DIALOG_MULTIPLE,
        .title = OC_STR8("Select Files"),
        .okLabel = OC_STR8("Select"),
        .startAt = dir,
        .startPath = OC_STR8(".."),
    };

    oc_str8_list_push(scratch.arena, &desc.filters, OC_STR8("txt"));

    oc_file_dialog_result res = oc_file_dialog(scratch.arena, &desc);

    if(res.button == OC_FILE_DIALOG_CANCEL)
    {
        oc_log_error("Cancel\n");
    }
    else
    {
        oc_log_info("Selected files:\n");
        oc_list_for(res.selection.list, elt, oc_str8_elt, listElt)
        {
            oc_log_info("\t%.*s\n", (int)elt->string.len, elt->string.ptr);
        }
    }

    return (0);
}
