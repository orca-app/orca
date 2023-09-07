/************************************************************/ /**
*
*	@file: main.c
*	@author: Martin Fouilleul
*	@date: 26/05/2023
*
*****************************************************************/

#include "orca.h"

int main(int argc, char** argv)
{
    oc_init();

    oc_str8 path = oc_path_executable_relative(oc_scratch(), OC_STR8("../"));
    oc_file dir = oc_file_open(path, OC_FILE_ACCESS_READ, 0);

    oc_file_dialog_desc desc = {
        .kind = OC_FILE_DIALOG_OPEN,
        .flags = OC_FILE_DIALOG_FILES | OC_FILE_DIALOG_MULTIPLE,
        .title = OC_STR8("Select Files"),
        .okLabel = OC_STR8("Select"),
        .startAt = dir,
        .startPath = OC_STR8(".."),
    };

    oc_str8_list_push(oc_scratch(), &desc.filters, OC_STR8("txt"));

    oc_file_dialog_result res = oc_file_dialog(oc_scratch(), &desc);

    if(res.button == OC_FILE_DIALOG_CANCEL)
    {
        oc_log_error("Cancel\n");
    }
    else
    {
        oc_log_info("Selected files:\n");
        oc_list_for(&res.selection.list, elt, oc_str8_elt, listElt)
        {
            oc_log_info("\t%.*s\n", (int)elt->string.len, elt->string.ptr);
        }
    }

    return (0);
}
