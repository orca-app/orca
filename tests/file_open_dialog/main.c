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

    oc_file_dialog_desc desc = {
        .kind = OC_FILE_DIALOG_OPEN,
        .flags = OC_FILE_DIALOG_FILES,
        .title = OC_STR8("Select Files"),
        .okLabel = OC_STR8("Select")
    };

    oc_str8_list_push(oc_scratch(), &desc.filters, OC_STR8("txt"));

    oc_file file = oc_file_open_with_dialog(OC_FILE_ACCESS_READ, 0, &desc);
    if(oc_file_is_nil(file))
    {
        oc_log_error("Couldn't open file\n");
    }
    else
    {
        char buffer[1024];
        u64 len = oc_file_read(file, 1024, buffer);
        oc_log_info("file contents: %.*s\n", (int)len, buffer);
    }

    return (0);
}
