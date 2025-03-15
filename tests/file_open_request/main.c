/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "orca.h"

oc_str8 parseTestDir(int argc, const char** argv, oc_arena* arena)
{
    const char* test_dir_arg_prefix = "--test-dir=";
    for (int i = 1; i < argc; ++i)
    {
        const char* arg = argv[i];
        const size_t arglen = strlen(arg);

        if (strstr(arg, test_dir_arg_prefix) == arg)
        {
            const char* slice = arg + strlen(test_dir_arg_prefix);
            return OC_STR8(slice);
        }
    }

    return OC_STR8("");
}

int main(int argc, const char** argv)
{
    oc_init();

    oc_arena_scope arena_scope = oc_scratch_begin();
    oc_str8 test_dir = parseTestDir(argc, argv, arena_scope.arena);
    oc_str8 test_txt_path = oc_path_append(arena_scope.arena, test_dir, OC_STR8("test.txt"));

    oc_file file = oc_file_open_with_request(test_txt_path, OC_FILE_ACCESS_READ, 0);
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

    oc_scratch_end(arena_scope);

    return (0);
}
