/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#if OC_PLATFORM_WINDOWS
    #include <combaseapi.h>
    #include <knownfolders.h>
    #include <shlobj_core.h>
    #include <winerror.h>
#endif

#include "orca.h"
#include "flag.h"
#include "util.h"

oc_str8 getOrcaDir(oc_arena* a);
oc_str8 getCurrentVersionDir(oc_arena* a, oc_str8 orcaDir);

int sdkPath(int argc, char** argv)
{
    oc_arena a;
    oc_arena_init(&a);

    Flag_Context c;
    flag_init_context(&c);

    // TODO: version selection

    if(!flag_parse(&c, argc, argv))
    {
        flag_print_usage(&c, "orca sdk-path", stderr);
        if(flag_error_is_help(&c))
        {
            return 0;
        }
        flag_print_error(&c, stderr);
        return 1;
    }

    oc_str8 orcaDir = getOrcaDir(&a);
    oc_str8 sdkDir = getCurrentVersionDir(&a, orcaDir);

    printf("%.*s", oc_str8_printf(sdkDir));

    return 0;
}

#if OC_PLATFORM_WINDOWS
oc_str8 getOrcaDir(oc_arena* a)
{
    PWSTR pathWCStr;
    HRESULT res = SHGetKnownFolderPath(&FOLDERID_LocalAppData, 0, NULL, &pathWCStr);
    if(!SUCCEEDED(res))
    {
        fprintf(stderr, "Failed to locate system orca directory.\n");
        exit(1);
    }

    oc_str16 pathWide = oc_str16_from_buffer(lstrlenW(pathWCStr), pathWCStr);
    oc_str8 path = oc_win32_wide_to_utf8(a, pathWide);
    CoTaskMemFree(pathWCStr);

    path = oc_path_append(a, path, OC_STR8("orca"));
    oc_win32_path_normalize_slash_in_place(path);
    return path;
}
#elif OC_PLATFORM_MACOS
oc_str8 getOrcaDir(oc_arena* a)
{
    oc_str8 path = OC_STR8(getenv("HOME"));
    path = oc_path_append(a, path, OC_STR8(".orca"));
    return path;
}
#else
    #error "Unknown platform for sdk-path"
#endif

oc_str8 getCurrentVersionDir(oc_arena* a, oc_str8 orcaDir)
{
    oc_str8 currentFilePath = oc_path_append(a, orcaDir, OC_STR8("versions/current"));
    FILE* current = fopen(currentFilePath.ptr, "r");
    if(!current)
    {
        fprintf(stderr, "Failed to determine current Orca SDK version.\n");
        exit(1);
    }

    char buf[64];
    fgets(buf, sizeof(buf), current);
    oc_str8 currentVersion = OC_STR8(buf);
    currentVersion = oc_str8_trim_space(currentVersion);

    oc_str8 res = orcaDir;
    res = oc_path_append(a, res, OC_STR8("versions"));
    res = oc_path_append(a, res, currentVersion);
    return res;
}
