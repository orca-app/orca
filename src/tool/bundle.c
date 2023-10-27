/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include <processenv.h>
#include <stdio.h>

#include "flag.h"
#include "platform/platform_path.h"
#include "util/strings.h"
#include "util/memory.h"
#include "system.h"

int winBundle(
    oc_arena* a,
    oc_str8 name,
    oc_str8 icon,
    oc_str8 version,
    oc_str8_list resourceFiles,
    oc_str8_list resourceDirs,
    oc_str8 outDir,
    oc_str8 orcaDir);

int bundle(int argc, char** argv)
{
    oc_arena a;
    oc_arena_init(&a);

    Flag_Context c;
    flag_init_context(&c);

    char** name = flag_str(&c, "n", "name", "out", "the app's name");
    char** icon = flag_str(&c, "i", "icon", NULL, "an image file to use as the application's icon");
    char** version = flag_str(&c, NULL, "version", "0.0.0", "a version number to embed in the application bundle");
    oc_str8_list* resourceFiles = flag_strs(&c, "d", "resource", "copy a file to the app's resource directory");
    oc_str8_list* resourceDirs = flag_strs(&c, "D", "resource-dir", "copy a directory to the app's resource directory");
    char** outDir = flag_str(&c, "C", "out-dir", NULL, "where to place the final application bundle (defaults to the current directory)");
    char** orcaDir = flag_str(&c, "O", "orca-dir", ".", NULL);
    // TODO: mtl-enable-capture

    char** module = flag_pos(&c, "module", "a .wasm file containing the application's wasm module");

    if(!flag_parse(&c, argc, argv))
    {
        flag_print_usage(&c, "orca bundle", stderr);
        if(flag_error_is_help(&c))
        {
            return 0;
        }
        flag_print_error(&c, stderr);
        return 1;
    }

    if(!flag_parse_positional(&c))
    {
        flag_print_usage(&c, "orca bundle", stderr);
        flag_print_error(&c, stderr);
        return 1;
    }

#ifdef OC_PLATFORM_WINDOWS
    return winBundle(
        &a,
        OC_STR8(*name),
        OC_STR8(*icon),
        OC_STR8(*version),
        *resourceFiles,
        *resourceDirs,
        OC_STR8(*outDir),
        OC_STR8(*orcaDir));
#else
    #error Can't build the bundle script on this platform!
#endif
}

#define TRY(cmd)                                                                              \
    {                                                                                         \
        bool __result = cmd;                                                                  \
        if(!__result)                                                                         \
        {                                                                                     \
            int code = oc_sys_err.code;                                                       \
            if(code == 0)                                                                     \
            {                                                                                 \
                code = 1;                                                                     \
            }                                                                                 \
            fprintf(stderr, "ERROR (code %d): %.*s\n", code, oc_str8_printf(oc_sys_err.msg)); \
            return code;                                                                      \
        }                                                                                     \
    }

#ifdef OC_PLATFORM_WINDOWS

int winBundle(
    oc_arena* a,
    oc_str8 name,
    oc_str8 icon,
    oc_str8 version,
    oc_str8_list resourceFiles,
    oc_str8_list resourceDirs,
    oc_str8 outDir,
    oc_str8 orcaDir)
{
    if(!outDir.ptr)
    {
        outDir = oc_sys_getcwd(a);
    }

    //-----------------------------------------------------------
    //NOTE: make bundle directory structure
    //-----------------------------------------------------------
    oc_str8 bundleDir = oc_path_append(a, outDir, name);
    oc_str8 exeDir = oc_path_append(a, bundleDir, OC_STR8("bin"));
    oc_str8 resDir = oc_path_append(a, bundleDir, OC_STR8("resources"));
    oc_str8 guestDir = oc_path_append(a, bundleDir, OC_STR8("app"));
    oc_str8 wasmDir = oc_path_append(a, bundleDir, OC_STR8("wasm"));
    oc_str8 dataDir = oc_path_append(a, bundleDir, OC_STR8("data"));

    if(oc_sys_path_exists(bundleDir))
    {
        TRY(oc_sys_rmdir(bundleDir));
    }
    TRY(oc_sys_mkdirs(bundleDir));
    TRY(oc_sys_mkdirs(exeDir));
    TRY(oc_sys_mkdirs(resDir));
    TRY(oc_sys_mkdirs(guestDir));
    TRY(oc_sys_mkdirs(wasmDir));
    TRY(oc_sys_mkdirs(dataDir));

    printf("u good");

    return 0;
}

#endif // OC_PLATFORM_WINDOWS
