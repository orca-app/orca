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
    oc_str8 appVersion;
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

int oc_tool_bundle_standalone_macos(oc_tool_options* options, oc_str8 appImage)
{
    //NOTE: bundle the app image into a macos bundle
    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8 bundlePath = oc_str8_pushf(scratch.arena,
                                       "%.*s/%.*s.app",
                                       oc_str8_ip(options->outDir),
                                       oc_str8_ip(options->name));
    if(oc_sys_exists(bundlePath))
    {
        TRY(oc_sys_rmdir(bundlePath));
    }

    oc_str8 contentsPath = oc_path_append(scratch.arena, bundlePath, OC_STR8("Contents"));

    oc_str8 macosPath = oc_path_append(scratch.arena, contentsPath, OC_STR8("macOS/"));
    oc_str8 resPath = oc_path_append(scratch.arena, contentsPath, OC_STR8("resources/"));

    oc_sys_mkdirs(macosPath);
    oc_sys_mkdirs(resPath);

    //NOTE: copy binaries to macOS dir
    oc_str8 srcBin = oc_path_executable_relative(scratch.arena, OC_STR8("."));

    oc_sys_copy(oc_path_append(scratch.arena, srcBin, OC_STR8("orca_runtime")), macosPath);
    oc_sys_copy(oc_path_append(scratch.arena, srcBin, OC_STR8("liborca_platform.dylib")), macosPath);
    oc_sys_copy(oc_path_append(scratch.arena, srcBin, OC_STR8("libEGL.dylib")), macosPath);
    oc_sys_copy(oc_path_append(scratch.arena, srcBin, OC_STR8("libGLESv2.dylib")), macosPath);
    oc_sys_copy(oc_path_append(scratch.arena, srcBin, OC_STR8("libwebgpu.dylib")), macosPath);

    //NOTE: copy resources
    oc_str8 srcRes = oc_path_executable_relative(scratch.arena, OC_STR8("../resources"));

    oc_sys_copy(oc_path_append(scratch.arena, srcRes, OC_STR8("Menlo.ttf")), resPath);
    oc_sys_copy(oc_path_append(scratch.arena, srcRes, OC_STR8("Menlo Bold.ttf")), resPath);

    //NOTE: copy app image
    oc_sys_copy(appImage, resPath);

    //-----------------------------------------------------------
    //NOTE make icon
    //-----------------------------------------------------------
    if(options->icon.len)
    {
        if(oc_sys_exists(options->icon))
        {
            oc_str8 icon_dir = oc_path_slice_directory(options->icon);
            oc_str8 iconset = oc_path_append(scratch.arena, icon_dir, OC_STR8("icon.iconset"));
            if(oc_sys_exists(iconset))
            {
                TRY(oc_sys_rmdir(iconset));
            }
            TRY(oc_sys_mkdirs(iconset));

            i32 size = 16;
            for(i32 i = 0; i < 7; ++i)
            {
                oc_str8 sized_icon = oc_path_append(scratch.arena, iconset, oc_str8_pushf(scratch.arena, "icon_%dx%d.png", size, size));
                oc_str8 cmd = oc_str8_pushf(scratch.arena, "sips -z %d %d %.*s --out %s >/dev/null 2>&1",
                                            size, size, oc_str8_ip(options->icon), sized_icon.ptr);
                i32 result = system(cmd.ptr);
                if(result)
                {
                    fprintf(stderr, "failed to generate %dx%d icon from %.*s\n", size, size, oc_str8_ip(options->icon));
                }

                i32 retina_size = size * 2;
                oc_str8 retina_icon = oc_path_append(scratch.arena, iconset, oc_str8_pushf(scratch.arena, "icon_%dx%d@2x.png", size, size));
                cmd = oc_str8_pushf(scratch.arena, "sips -z %d %d %.*s --out %s >/dev/null 2>&1",
                                    retina_size, retina_size, oc_str8_ip(options->icon), sized_icon.ptr);
                result = system(cmd.ptr);
                if(result)
                {
                    fprintf(stderr, "failed to generate %dx%d@2x retina icon from %.*s\n", size, size, oc_str8_ip(options->icon));
                }

                size *= 2;
            }

            oc_str8 icon_out = oc_path_append(scratch.arena, resPath, OC_STR8("icon.icns"));
            oc_str8 cmd = oc_str8_pushf(scratch.arena, "iconutil -c icns -o %s %s", icon_out.ptr, iconset.ptr);
            i32 result = system(cmd.ptr);
            if(result)
            {
                fprintf(stderr, "failed to generate app icon from %.*s", oc_str8_ip(options->icon));
            }
            TRY(oc_sys_rmdir(iconset));
        }
        else
        {
            fprintf(stderr, "warning: failed to find icon file \"%.*s\"\n", oc_str8_ip(options->icon));
        }
    }

    //-----------------------------------------------------------
    //NOTE: write plist file
    //-----------------------------------------------------------
    oc_str8 bundle_sig = OC_STR8("????");

    oc_str8 plist_contents = oc_str8_pushf(scratch.arena,
                                           "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                           "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">"
                                           "<plist version=\"1.0\">"
                                           "<dict>"
                                           "<key>CFBundleName</key>"
                                           "<string>%.*s</string>"
                                           "<key>CFBundleDisplayName</key>"
                                           "<string>%.*s</string>"
                                           "<key>CFBundleIdentifier</key>"
                                           "<string>%.*s</string>"
                                           "<key>CFBundleVersion</key>"
                                           "<string>%.*s</string>"
                                           "<key>CFBundlePackageType</key>"
                                           "<string>APPL</string>"
                                           "<key>CFBundleSignature</key>"
                                           "<string>%.*s</string>"
                                           "<key>CFBundleExecutable</key>"
                                           "<string>orca_runtime</string>"
                                           "<key>CFBundleIconFile</key>"
                                           "<string>icon.icns</string>"
                                           "<key>NSHighResolutionCapable</key>"
                                           "<string>True</string>"
                                           "</dict>"
                                           "</plist>",
                                           oc_str8_ip(options->name),
                                           oc_str8_ip(options->name),
                                           oc_str8_ip(options->name),
                                           oc_str8_ip(options->appVersion),
                                           oc_str8_ip(bundle_sig));

    oc_str8 plist_path = oc_path_append(scratch.arena, contentsPath, OC_STR8("Info.plist"));
    oc_file plist_file = oc_file_open(plist_path, OC_FILE_ACCESS_WRITE, OC_FILE_OPEN_CREATE);
    if(oc_file_is_nil(plist_file))
    {
        fprintf(stderr, "Error: failed to create plist file \"%.*s\"\n",
                oc_str8_ip(plist_path));
        oc_file_close(plist_file);
        return 1;
    }
    oc_file_write(plist_file, plist_contents.len, plist_contents.ptr);
    oc_file_close(plist_file);

    oc_scratch_end(scratch);
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
    oc_str8 outDir = options->standalone ? tmpDir : options->outDir;
    oc_str8 outFile = oc_str8_pushf(scratch.arena,
                                    "%.*s/%.*s.orca",
                                    oc_str8_ip(outDir),
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

    if(options->standalone)
    {
        status = oc_tool_bundle_standalone_macos(options, outFile);
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
