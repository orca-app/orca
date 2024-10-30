/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "orca.h"
#include "flag.h"
#include "util.h"

typedef struct
{
    oc_str8 dir_name;
    bool found;
} oc_sdk_requirement;

typedef struct
{
    oc_str8 version;
    oc_list_elt node;
} oc_version;

int list(int argc, char** argv)
{
    oc_arena a;
    oc_arena_init(&a);

    Flag_Context c;
    flag_init_context(&c);

    flag_help(&c, "List the installed versions of the Orca SDK.");

    if(!flag_parse(&c, argc, argv))
    {
        flag_print_usage(&c, "orca list", stderr);
        if(flag_error_is_help(&c))
        {
            return 0;
        }
        flag_print_error(&c, stderr);
        return 1;
    }

    oc_list versions = {0};

    oc_str8 orca_dir = system_orca_dir(&a);
    oc_list candidate_dirs = oc_sys_read_dir(&a, orca_dir);
    oc_list_for(candidate_dirs, candidate_entry, oc_sys_dir_entry, node)
    {
        if(!candidate_entry->is_dir) continue;

        oc_sdk_requirement requirements[] =
        {
            {.dir_name = OC_STR8("bin"),       .found = false},
            {.dir_name = OC_STR8("orca-libc"), .found = false},
            {.dir_name = OC_STR8("resources"), .found = false},
            {.dir_name = OC_STR8("src"),       .found = false},
        };

        oc_arena_scope scratch = oc_scratch_begin();
        oc_str8 candidate_dir = oc_path_append(scratch.arena, candidate_entry->base, candidate_entry->name);

        oc_list entries = oc_sys_read_dir(scratch.arena, candidate_dir);
        oc_list_for(entries, entry, oc_sys_dir_entry, node)
        {
            if(!entry->is_dir) continue;

            for (i32 i = 0; i < oc_array_size(requirements); ++i)
            {
                if(0 == oc_str8_cmp(requirements[i].dir_name, entry->name))
                {
                    requirements[i].found = true;
                    break;
                }
            }
        }

        bool match = true;
        for (i32 i = 0; i < oc_array_size(requirements); ++i)
        {
            if(!requirements[i].found)
            {
                match = false;
                break;
            }
        }

        if(match)
        {
            oc_version *v = oc_arena_push_type(&a, oc_version);
            v->version = oc_str8_push_copy(&a, candidate_entry->name);
            oc_list_push_back(&versions, &v->node);
        }

        oc_scratch_end(scratch);
    }

    if(oc_list_empty(versions))
    {
        fprintf(stderr, "error: Failed to find any installed versions of the Orca SDK. If you just installed Orca for the first time, make sure to run `orca update`.\n");
        return 1;
    }

    oc_list_for(versions, v, oc_version, node)
    {
        printf("%.*s\n", oc_str8_ip(v->version));
    }

    return 0;
}
