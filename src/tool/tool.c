#include "util/typedefs.h"
#include "util/memory.h"
#include "util/strings.h"
#include "system.h"
#include "ext/libzip/lib/zip.h"
#include <sys/stat.h>
#include <unistd.h>

#include "system.c"

typedef struct oc_tool_options
{
    oc_str8 command;

    // bundle options
    bool standalone;
    oc_str8 name;
    oc_str8 icon;

    u32 resDirCount;
    oc_str8* resDirs;

    u32 resFileCount;
    oc_str8* resFiles;

    u32 moduleCount;
    oc_str8* modules;

    oc_str8 outDir;

    // install options
    oc_str8 installName;

    // install & run options
    oc_str8 app;
} oc_tool_options;

int add_to_archive(zip_t* zip, oc_str8 srcPath, oc_str8 dstPath)
{
    oc_file srcFile = oc_file_open(srcPath, OC_FILE_ACCESS_READ, 0);
    oc_file_status status = oc_file_get_status(srcFile);

    if(status.type == OC_FILE_DIRECTORY)
    {
        zip_dir_add(zip, dstPath.ptr, 0);

        oc_arena_scope scratch = oc_scratch_begin();
        oc_file_list files = oc_file_listdir(scratch.arena, srcFile);
        oc_file_list_for(files, elt)
        {
            oc_str8 fileSrcPath = oc_path_append(scratch.arena, srcPath, elt->basename);
            oc_str8 fileDstPath = oc_path_append(scratch.arena, dstPath, elt->basename);
            add_to_archive(zip, fileSrcPath, fileDstPath);
        }
        oc_scratch_end(scratch);
    }
    else
    {
        zip_source_t* source = zip_source_file(zip, srcPath.ptr, 0, ZIP_LENGTH_TO_END);
        zip_file_add(zip, dstPath.ptr, source, ZIP_FL_OVERWRITE);
    }
    return 0;
}

int oc_tool_bundle(oc_tool_options* options)
{
    //NOTE: bundle orca app file
    //TODO: make temp directory structure, then copy resources/binaries into it/compress

    oc_arena_scope scratch = oc_scratch_begin();

    oc_str8 template = oc_str8_pushf(scratch.arena, "/tmp/%.*s.XXXXXX", oc_str8_ip(options->name));
    //TODO: check error
    oc_str8 tmpDir = OC_STR8(mkdtemp(template.ptr));

    oc_str8 modDir = oc_path_append(scratch.arena, tmpDir, OC_STR8("modules/"));
    oc_sys_mkdirs(modDir);
    oc_str8 resDir = oc_path_append(scratch.arena, tmpDir, OC_STR8("data/"));
    oc_sys_mkdirs(resDir);

    //NOTE: copy modules
    bool foundMain = false;
    for(int i = 0; i < options->moduleCount; i++)
    {
        if(!oc_str8_cmp(oc_path_slice_filename(options->modules[i]), OC_STR8("main.wasm")))
        {
            foundMain = true;
        }
        oc_sys_copy(options->modules[i], modDir);
    }
    if(!foundMain)
    {
        printf("error: bundle must have at least one module named 'main.wasm'.\n");
        return -1;
    }

    //NOTE: copy resources
    for(int i = 0; i < options->resDirCount; i++)
    {
        oc_sys_copytree(options->resDirs[i], resDir);
    }

    for(int i = 0; i < options->resFileCount; i++)
    {
        oc_sys_copy(options->resFiles[i], resDir);
    }

    //NOTE: zip folder to out directory
    int status = 0;
    oc_str8 outFile = oc_str8_pushf(scratch.arena,
                                    "%.*s/%.*s.orca",
                                    oc_str8_ip(options->outDir),
                                    oc_str8_ip(options->name),
                                    OC_STR8(".orca"));

    zip_t* zip = zip_open(outFile.ptr, ZIP_CREATE | ZIP_TRUNCATE, 0);
    if(!zip)
    {
        status = -1;
    }
    else
    {
        add_to_archive(zip, modDir, OC_STR8("modules"));
        add_to_archive(zip, resDir, OC_STR8("data"));

        zip_close(zip);
    }

    oc_scratch_end(scratch);
    return status;
}

int oc_tool_sdk_path(oc_tool_options* options)
{
    oc_arena_scope scratch = oc_scratch_begin();

    oc_str8 sdkPath = oc_path_executable_relative(scratch.arena, OC_STR8("../SDK"));
    printf("%.*s\n", oc_str8_ip(sdkPath));

    oc_scratch_end(scratch);
    return 0;
}

#define _TOSTRING(s) #s
#define TOSTRING(s) _TOSTRING(s)

int oc_tool_version(oc_tool_options* options)
{
    printf("Orca CLI tools version: %s\n", TOSTRING(ORCA_TOOL_VERSION));
    return 0;
}
