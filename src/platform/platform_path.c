/************************************************************/ /**
*
*	@file: platform_path.c
*	@author: Martin Fouilleul
*	@date: 24/05/2023
*
*****************************************************************/

#include "platform_path.h"

oc_str8 oc_path_slice_directory(oc_str8 fullPath)
{
    i64 lastSlashIndex = -1;

    for(i64 i = fullPath.len - 1; i >= 0; i--)
    {
        if(fullPath.ptr[i] == '/')
        {
            lastSlashIndex = i;
            break;
        }
    }
    oc_str8 directory = oc_str8_slice(fullPath, 0, lastSlashIndex + 1);
    return (directory);
}

oc_str8 oc_path_slice_filename(oc_str8 fullPath)
{
    i64 lastSlashIndex = -1;

    for(i64 i = fullPath.len - 1; i >= 0; i--)
    {
        if(fullPath.ptr[i] == '/')
        {
            lastSlashIndex = i;
            break;
        }
    }

    oc_str8 basename = oc_str8_slice(fullPath, lastSlashIndex + 1, fullPath.len);
    return (basename);
}

oc_str8_list oc_path_split(oc_arena* arena, oc_str8 path)
{
    oc_arena_scope tmp = oc_scratch_begin_next(arena);
    oc_str8_list split = { 0 };
    oc_str8_list_push(tmp.arena, &split, OC_STR8("/"));
    oc_str8_list res = oc_str8_split(arena, path, split);
    oc_scratch_end(tmp);
    return (res);
}

oc_str8 oc_path_join(oc_arena* arena, oc_str8_list elements)
{
    //TODO: check if elements have ending/begining '/' ?
    oc_str8 res = oc_str8_list_collate(arena, elements, OC_STR8(""), OC_STR8("/"), (oc_str8){ 0 });
    return (res);
}

oc_str8 oc_path_append(oc_arena* arena, oc_str8 parent, oc_str8 relPath)
{
    oc_str8 result = { 0 };

    if(parent.len == 0)
    {
        result = oc_str8_push_copy(arena, relPath);
    }
    else if(relPath.len == 0)
    {
        result = oc_str8_push_copy(arena, parent);
    }
    else
    {
        oc_arena_scope tmp = oc_scratch_begin_next(arena);

        oc_str8_list list = { 0 };
        oc_str8_list_push(tmp.arena, &list, parent);
        if((parent.ptr[parent.len - 1] != '/')
           && (relPath.ptr[relPath.len - 1] != '/'))
        {
            oc_str8_list_push(tmp.arena, &list, OC_STR8("/"));
        }
        oc_str8_list_push(tmp.arena, &list, relPath);

        result = oc_str8_list_join(arena, list);

        oc_scratch_end(tmp);
    }
    return (result);
}

oc_str8 oc_path_executable_relative(oc_arena* arena, oc_str8 relPath)
{
    oc_str8_list list = { 0 };
    oc_arena_scope scratch = oc_scratch_begin_next(arena);

    oc_str8 executablePath = oc_path_executable(scratch.arena);
    oc_str8 dirPath = oc_path_slice_directory(executablePath);

    oc_str8 path = oc_path_append(arena, dirPath, relPath);

    oc_scratch_end(scratch);
    return (path);
}
