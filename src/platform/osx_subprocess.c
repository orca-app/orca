/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "platform_subprocess.h"

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

oc_str8 oc_run_cmd(oc_arena* arena, oc_str8 cmd)
{
    const char* cmdC = oc_str8_to_cstring(arena, cmd);
    FILE* p = popen(cmdC, "r+");
    if(!p)
    {
        oc_log_error("failed to launch process");
        return OC_STR8("");
    }

    oc_str8_list chunks = { 0 };
    char buffer[1024];
    while(true)
    {
        ssize_t nRead = read(fileno(p), buffer, sizeof(buffer));
        if(nRead < 0)
        {
            oc_log_error("failed to read from process's stdout\n");
            return OC_STR8("");
        }
        if(nRead == 0)
        {
            break;
        }

        oc_str8 chunk = oc_str8_from_buffer(nRead, buffer);
        oc_str8_list_push(arena, &chunks, oc_str8_push_copy(arena, chunk));
    }

    int status = pclose(p);
    if(status == -1)
    {
        oc_log_warning("failed to close subprocess\n");
    }
    else if(!WIFEXITED(status))
    {
        oc_log_warning("subprocess did not exit correctly\n");
    }

    return oc_str8_list_join(arena, chunks);
}

#ifdef __cplusplus
} // extern "C"
#endif
