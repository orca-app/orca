/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#define OC_NO_APP_LAYER 1
#include "orca.c"
#include "wasm/wasm.c"
#include "warm/warm_adapter.c"

int main(int argc, char** argv)
{
    oc_log_set_level(OC_LOG_LEVEL_ERROR);

    oc_arena arena = { 0 };
    oc_arena_init(&arena);

    oc_file file = oc_file_open(OC_STR8("./test_module.wasm"), OC_FILE_ACCESS_READ, OC_FILE_OPEN_NONE);
    oc_str8 contents = { 0 };
    contents.len = oc_file_size(file);
    contents.ptr = oc_arena_push(&arena, contents.len);
    oc_file_read(file, contents.len, contents.ptr);
    oc_file_close(file);

    wa_module* module = wa_module_create(&arena, contents);

    printf("test_module.wasm:\tfile format WASM\n");
    printf("\n");

    dw_print_line_info(module->debugInfo->line);

    return (0);
}
