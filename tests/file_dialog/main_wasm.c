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
        .kind = OC_FILE_DIALOG_OPEN,
        .flags = OC_FILE_DIALOG_FILES,
        .title = OC_STR8("Select Files"),
        .okLabel = OC_STR8("Select")
    };

    oc_file_open_with_dialog_result res = oc_file_open_with_dialog(scratch.arena, OC_FILE_ACCESS_READ, OC_FILE_OPEN_NONE, &desc);

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
