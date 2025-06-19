/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include <orca.h>

ORCA_EXPORT void oc_on_init(void)
{
    oc_arena_scope scratch = oc_scratch_begin();

    oc_file_dialog_desc desc = {
        .kind = OC_FILE_DIALOG_SAVE,
        .flags = OC_FILE_DIALOG_FILES,
        .title = OC_STR8("Select Files"),
        .okLabel = OC_STR8("Select")
    };

    oc_file_open_with_dialog_result res = oc_file_open_with_dialog(scratch.arena, OC_FILE_ACCESS_WRITE, OC_FILE_OPEN_CREATE | OC_FILE_OPEN_TRUNCATE, &desc);

    oc_str8 buffer = OC_STR8("Test test");
    oc_file_write(res.file, buffer.len, buffer.ptr);
    oc_file_close(res.file);

    if(res.button == OC_FILE_DIALOG_CANCEL)
    {
        oc_log_error("Cancel\n");
    }
    else
    {
        oc_log_info("OK\n");
    }
}

ORCA_EXPORT void oc_on_frame_refresh(void)
{
    oc_request_quit();
}
