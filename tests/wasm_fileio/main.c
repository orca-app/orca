/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include <orca.h>

#include <stdlib.h>
#include <string.h>

#define TEST_FAIL_ERR(err)                              \
    if((err) != OC_IO_OK)                               \
    {                                                   \
        oc_log_error("unexpected IO error: %d\n", err); \
        return (-1);                                    \
    }

typedef struct expected_entry
{
    oc_str8 basename;
    oc_file_type type;
} expected_entry;

int index_of_entry(oc_str8 basename, const expected_entry* entries, size_t count)
{
    for(size_t i = 0; i < count; ++i)
    {
        if(!oc_str8_cmp(basename, entries[i].basename))
        {
            return (int)i;
        }
    }
    return -1;
}

int test_listdir_recursive(oc_arena* arena, oc_str8 path, const expected_entry* expected, size_t expected_count, size_t* found_count)
{
    int success = 0;

    oc_file dir = oc_file_open(path, OC_FILE_ACCESS_READ, 0);
    TEST_FAIL_ERR(oc_file_last_error(dir));

    oc_file_list entries = oc_file_listdir(arena, dir);
    TEST_FAIL_ERR(oc_file_last_error(dir));

    oc_file_list_for(entries, elt)
    {
        oc_str8 entry_path = oc_path_append(arena, path, elt->basename);

        int index = index_of_entry(entry_path, expected, expected_count);
        if(index == -1)
        {
            oc_log_info("unexpected entry %p %zu\n", oc_str8_ip(entry_path));
            success = -1;
        }

        if(expected[index].type != elt->type)
        {
            oc_log_info("entry %.*s has the wrong type. expected %d but got %d\n",
                        oc_str8_ip(entry_path), expected[index].type, elt->type);
            success = -1;
        }

        ++*found_count;

        if(!success && elt->type == OC_FILE_DIRECTORY)
        {
            success = test_listdir_recursive(arena, entry_path, expected, expected_count, found_count);
        }
    }

    oc_file_close(dir);

    return success;
}

int test_listdir(void)
{
    oc_log_info("listdir");

    oc_arena_scope arena_scope = oc_scratch_begin();

    const expected_entry expected[] = {
        { OC_STR8("./nested1"), OC_FILE_DIRECTORY },
        { OC_STR8("./root"), OC_FILE_REGULAR },
        { OC_STR8("./nested1/nested2"), OC_FILE_DIRECTORY },
        { OC_STR8("./nested1/a"), OC_FILE_REGULAR },
        { OC_STR8("./nested1/b"), OC_FILE_REGULAR },
        { OC_STR8("./nested1/nested2/leaf"), OC_FILE_REGULAR },
    };

    size_t found = 0;
    size_t expected_count = oc_array_size(expected);
    int success = test_listdir_recursive(arena_scope.arena, OC_STR8("."), expected, expected_count, &found);
    if(success != -1 && found != expected_count)
    {
        oc_log_error("Enumerated %zu items but expected %zu\n", found, expected_count);
        success = -1;
    }

    oc_scratch_end(arena_scope);

    return success;
}

ORCA_EXPORT i32 oc_on_test(void)
{
    oc_log_info("testing: wasm_fileio\n");
    if(test_listdir())
    {
        return (-1);
    }

    oc_log_info("OK\n");

    return (0);
}
