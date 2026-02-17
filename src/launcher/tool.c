#include "util/typedefs.h"
#include "util/memory.h"
#include "util/strings.h"
#include "platform/io.h"
#include "ext/libzip/lib/zip.h"
#include <sys/stat.h>
#include <unistd.h>

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
    oc_file srcFile = oc_catch(oc_file_open(srcPath, OC_FILE_ACCESS_READ, 0))
    {
        oc_log_error("Couldn't open path %.*s\n", oc_str8_ip(srcPath));
        return -1;
    }
    oc_file_status status = oc_file_get_status(srcFile);

    if(status.type == OC_FILE_DIRECTORY)
    {
        if(zip_dir_add(zip, dstPath.ptr, 0) == -1)
        {
            oc_log_error("zip_dir_add failed adding directory %.*s: %s\n",
                         oc_str8_ip(dstPath),
                         zip_error_strerror(zip_get_error(zip)));
            return -1;
        }

        oc_arena_scope scratch = oc_scratch_begin();
        oc_file_list files = oc_file_listdir(scratch.arena, srcFile);
        oc_file_list_for(files, elt)
        {
            oc_str8 fileSrcPath = oc_path_append(scratch.arena, srcPath, elt->basename);
            oc_str8 fileDstPath = oc_path_append(scratch.arena, dstPath, elt->basename);
            if(add_to_archive(zip, fileSrcPath, fileDstPath))
            {
                return -1;
            }
        }
        oc_scratch_end(scratch);
    }
    else
    {
        zip_source_t* source = zip_source_file(zip, srcPath.ptr, 0, ZIP_LENGTH_TO_END);
        if(zip_file_add(zip, dstPath.ptr, source, ZIP_FL_OVERWRITE) == -1)
        {
            oc_log_error("zip_file_add failed adding file %.*s: %s\n",
                         oc_str8_ip(dstPath),
                         zip_error_strerror(zip_get_error(zip)));
            return -1;
        }
    }
    return 0;
}

int oc_tool_bundle_copy(oc_str8 src, oc_str8 dest)
{
    oc_io_error error = oc_file_copy(src, dest, 0);
    if(error)
    {
        oc_log_error("Could not copy %.*s to %.*s: %.*s\n",
                     oc_str8_ip(src),
                     oc_str8_ip(dest),
                     oc_str8_ip(oc_io_error_string(error)));
        return -1;
    }
    return 0;
}

#if OC_PLATFORM_WINDOWS

    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include "build/win32_icon.c"

int oc_tool_bundle_standalone_windows(oc_tool_options* options, oc_str8 appImage)
{
    //NOTE: bundle the app image into a windows app dir
    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8 appPath = oc_str8_pushf(scratch.arena,
                                    "%.*s/%.*s",
                                    oc_str8_ip(options->outDir),
                                    oc_str8_ip(options->name));

    oc_io_error error = oc_file_remove(appPath, &(oc_file_remove_options){ .flags = OC_FILE_REMOVE_RECURSIVE });
    if(error != OC_IO_OK && error != OC_IO_ERR_NO_ENTRY)
    {
        oc_log_error("Could not remove existing app directory...\n");
        return -1;
    }

    oc_str8 binPath = oc_path_append(scratch.arena, appPath, OC_STR8("bin"));
    oc_str8 resPath = oc_path_append(scratch.arena, appPath, OC_STR8("resources"));

    error = oc_file_makedir(binPath, &(oc_file_makedir_options){ .flags = OC_FILE_MAKEDIR_CREATE_PARENTS });
    if(error)
    {
        oc_log_error("Could not create directory %.*s: %.*s",
                     oc_str8_ip(binPath),
                     oc_str8_ip(oc_io_error_string(error)));
        return -1;
    }

    error = oc_file_makedir(resPath, &(oc_file_makedir_options){ .flags = OC_FILE_MAKEDIR_CREATE_PARENTS });
    if(error)
    {
        oc_log_error("Could not create directory %.*s: %.*s",
                     oc_str8_ip(resPath),
                     oc_str8_ip(oc_io_error_string(error)));
        return -1;
    }

    //NOTE: copy binaries to bin dir
    int status = 0;
    oc_str8 srcBin = oc_path_executable_relative(scratch.arena, OC_STR8("."));
    oc_str8 dstRuntimeName = oc_str8_pushf(scratch.arena, "%.*s.exe", oc_str8_ip(options->name));
    oc_str8 dstRuntimePath = oc_path_append(scratch.arena, binPath, dstRuntimeName);

    status |= oc_tool_bundle_copy(oc_path_append(scratch.arena, srcBin, OC_STR8("orca_runtime.exe")), dstRuntimePath);
    status |= oc_tool_bundle_copy(oc_path_append(scratch.arena, srcBin, OC_STR8("orca_platform.dll")), binPath);
    status |= oc_tool_bundle_copy(oc_path_append(scratch.arena, srcBin, OC_STR8("libEGL.dll")), binPath);
    status |= oc_tool_bundle_copy(oc_path_append(scratch.arena, srcBin, OC_STR8("libGLESv2.dll")), binPath);
    status |= oc_tool_bundle_copy(oc_path_append(scratch.arena, srcBin, OC_STR8("webgpu.dll")), binPath);

    //NOTE: copy resources
    oc_str8 srcRes = oc_path_executable_relative(scratch.arena, OC_STR8("../resources"));

    status |= oc_tool_bundle_copy(oc_path_append(scratch.arena, srcRes, OC_STR8("Menlo.ttf")), resPath);
    status |= oc_tool_bundle_copy(oc_path_append(scratch.arena, srcRes, OC_STR8("Menlo Bold.ttf")), resPath);

    //NOTE: copy app image
    status |= oc_tool_bundle_copy(appImage, resPath);

    if(status)
    {
        return -1;
    }

    //NOTE: set executable icon
    if(options->icon.len)
    {
        oc_str8 tmpDir = oc_path_append(scratch.arena, appPath, OC_STR8("tmp"));

        oc_io_error error = oc_file_remove(tmpDir, &(oc_file_remove_options){ .flags = OC_FILE_REMOVE_RECURSIVE });
        if(error != OC_IO_OK && error != OC_IO_ERR_NO_ENTRY)
        {
            oc_log_error("Could not remove existing directory %.*s: %.*s\n",
                         oc_str8_ip(tmpDir),
                         oc_str8_ip(oc_io_error_string(error)));
            return -1;
        }

        error = oc_file_makedir(tmpDir, 0);
        if(error)
        {
            oc_log_error("Could not create directory %.*s: %.*s\n",
                         oc_str8_ip(tmpDir),
                         oc_str8_ip(oc_io_error_string(error)));
            return -1;
        }

        oc_str8 icoPath = oc_path_append(scratch.arena, tmpDir, OC_STR8("icon.ico"));
        if(!icon_from_image(scratch.arena, options->icon, icoPath))
        {
            oc_log_error("failed to create windows icon for \"%.*s\"\n", oc_str8_ip(options->icon));
            return -1;
        }

        if(!embed_icon_into_exe(scratch.arena,
                                dstRuntimePath,
                                icoPath))
        {
            oc_log_error("failed to embed icon into exe file %.*s", oc_str8_ip(dstRuntimePath));
            return -1;
        }

        error = oc_file_remove(tmpDir, &(oc_file_remove_options){ .flags = OC_FILE_REMOVE_RECURSIVE });
        if(error != OC_IO_OK && error != OC_IO_ERR_NO_ENTRY)
        {
            oc_log_error("Could not remove directory %.*s: %.*s\n",
                         oc_str8_ip(tmpDir),
                         oc_str8_ip(oc_io_error_string(error)));
            return -1;
        }
    }

    oc_scratch_end(scratch);

    return 0;
}

#elif OC_PLATFORM_MACOS

int oc_tool_bundle_standalone_macos(oc_tool_options* options, oc_str8 appImage)
{
    //NOTE: bundle the app image into a macos bundle
    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8 bundlePath = oc_str8_pushf(scratch.arena,
                                       "%.*s/%.*s.app",
                                       oc_str8_ip(options->outDir),
                                       oc_str8_ip(options->name));

    oc_io_error error = oc_file_remove(bundlePath, &(oc_file_remove_options){ .flags = OC_FILE_REMOVE_RECURSIVE });
    if(error != OC_IO_OK && error != OC_IO_ERR_NO_ENTRY)
    {
        oc_log_error("Could not remove existing app bundle...\n");
        return -1;
    }

    oc_str8 contentsPath = oc_path_append(scratch.arena, bundlePath, OC_STR8("Contents"));
    oc_str8 macosPath = oc_path_append(scratch.arena, contentsPath, OC_STR8("macOS/"));
    oc_str8 resPath = oc_path_append(scratch.arena, contentsPath, OC_STR8("resources/"));

    error = oc_file_makedir(macosPath, &(oc_file_makedir_options){ .flags = OC_FILE_MAKEDIR_CREATE_PARENTS });
    if(error)
    {
        oc_log_error("Could not create directory %.*s: %.*s\n",
                     oc_str8_ip(macosPath),
                     oc_str8_ip(oc_io_error_string(error)));
        return -1;
    }
    error = oc_file_makedir(resPath, &(oc_file_makedir_options){ .flags = OC_FILE_MAKEDIR_CREATE_PARENTS });
    if(error)
    {
        oc_log_error("Could not create directory %.*s: %.*s\n",
                     oc_str8_ip(resPath),
                     oc_str8_ip(oc_io_error_string(error)));
        return -1;
    }

    //NOTE: copy binaries to macOS dir
    int status = 0;
    oc_str8 srcBin = oc_path_executable_relative(scratch.arena, OC_STR8("."));

    status |= oc_tool_bundle_copy(oc_path_append(scratch.arena, srcBin, OC_STR8("orca_runtime")), macosPath);
    status |= oc_tool_bundle_copy(oc_path_append(scratch.arena, srcBin, OC_STR8("liborca_platform.dylib")), macosPath);
    status |= oc_tool_bundle_copy(oc_path_append(scratch.arena, srcBin, OC_STR8("libEGL.dylib")), macosPath);
    status |= oc_tool_bundle_copy(oc_path_append(scratch.arena, srcBin, OC_STR8("libGLESv2.dylib")), macosPath);
    status |= oc_tool_bundle_copy(oc_path_append(scratch.arena, srcBin, OC_STR8("libwebgpu.dylib")), macosPath);

    //NOTE: copy resources
    oc_str8 srcRes = oc_path_executable_relative(scratch.arena, OC_STR8("../resources"));

    status |= oc_tool_bundle_copy(oc_path_append(scratch.arena, srcRes, OC_STR8("Menlo.ttf")), resPath);
    status |= oc_tool_bundle_copy(oc_path_append(scratch.arena, srcRes, OC_STR8("Menlo Bold.ttf")), resPath);

    //NOTE: copy app image
    status |= oc_tool_bundle_copy(appImage, resPath);

    if(status)
    {
        return -1;
    }

    //-----------------------------------------------------------
    //NOTE make icon
    //-----------------------------------------------------------
    if(options->icon.len)
    {
        oc_str8 icon_dir = oc_path_slice_directory(options->icon);
        oc_str8 iconset = oc_path_append(scratch.arena, icon_dir, OC_STR8("icon.iconset"));

        oc_io_error error = oc_file_remove(iconset, &(oc_file_remove_options){ .flags = OC_FILE_REMOVE_RECURSIVE });
        if(error != OC_IO_OK && error != OC_IO_ERR_NO_ENTRY)
        {
            oc_log_error("Could not remove existing icon set folder.\n");
            return -1;
        }

        error = oc_file_makedir(iconset, &(oc_file_makedir_options){ .flags = OC_FILE_MAKEDIR_CREATE_PARENTS });
        if(error)
        {
            oc_log_error("Could not create directory %.*s: %.*s\n",
                         oc_str8_ip(iconset),
                         oc_str8_ip(oc_io_error_string(error)));
        }

        i32 size = 16;
        for(i32 i = 0; i < 7; ++i)
        {
            oc_str8 sized_icon = oc_path_append(scratch.arena, iconset, oc_str8_pushf(scratch.arena, "icon_%dx%d.png", size, size));
            oc_str8 cmd = oc_str8_pushf(scratch.arena, "sips -z %d %d %.*s --out %s >/dev/null 2>&1",
                                        size, size, oc_str8_ip(options->icon), sized_icon.ptr);
            i32 result = system(cmd.ptr);
            if(result)
            {
                oc_log_error("failed to generate %dx%d icon from %.*s\n", size, size, oc_str8_ip(options->icon));
                return -1;
            }

            i32 retina_size = size * 2;
            oc_str8 retina_icon = oc_path_append(scratch.arena, iconset, oc_str8_pushf(scratch.arena, "icon_%dx%d@2x.png", size, size));
            cmd = oc_str8_pushf(scratch.arena, "sips -z %d %d %.*s --out %s >/dev/null 2>&1",
                                retina_size, retina_size, oc_str8_ip(options->icon), sized_icon.ptr);
            result = system(cmd.ptr);
            if(result)
            {
                oc_log_error("failed to generate %dx%d@2x retina icon from %.*s\n", size, size, oc_str8_ip(options->icon));
                return -1;
            }

            size *= 2;
        }

        oc_str8 icon_out = oc_path_append(scratch.arena, resPath, OC_STR8("icon.icns"));
        oc_str8 cmd = oc_str8_pushf(scratch.arena, "iconutil -c icns -o %s %s", icon_out.ptr, iconset.ptr);
        i32 result = system(cmd.ptr);
        if(result)
        {
            oc_log_error("failed to generate app icon from %.*s", oc_str8_ip(options->icon));
            return -1;
        }

        error = oc_file_remove(iconset, &(oc_file_remove_options){ .flags = OC_FILE_REMOVE_RECURSIVE });
        if(error != OC_IO_OK)
        {
            oc_log_error("Could not cleanup icon set folder.\n");
            return -1;
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
    oc_file plist_file = oc_catch(oc_file_open(plist_path,
                                               OC_FILE_ACCESS_WRITE,
                                               &(oc_file_open_options){
                                                   .flags = OC_FILE_OPEN_CREATE,
                                               }))
    {
        fprintf(stderr, "Error: failed to create plist file \"%.*s\"\n",
                oc_str8_ip(plist_path));
        return -1;
    }

    oc_file_write(plist_file, plist_contents.len, plist_contents.ptr);
    error = oc_file_last_error(plist_file);
    if(error)
    {
        oc_log_error("Couldn't write plist file %.*s: %.*s\n",
                     oc_str8_ip(plist_path),
                     oc_str8_ip(oc_io_error_string(error)));
        return -1;
    }

    oc_file_close(plist_file);

    oc_scratch_end(scratch);
    return 0;
}

#endif

int oc_tool_bundle(oc_tool_options* options)
{
    //NOTE: bundle orca app file
    oc_arena_scope scratch = oc_scratch_begin();

    oc_str8 tmpPath = { 0 };
    {
        //TODO: this is a bit of an involved way to get a _path_ to a temp dir, because
        // orca apis are explicitly designed to not give a full path to a tmp file (and
        // instead give an anonymous _handle_ to the newly created tmp file/dir), whereas
        // libzip needs a _path_ to work with (unless we implement zip_source_t for oc_file
        // handles, which we could do later...)

        oc_file tmpDir = oc_catch(oc_file_maketmp(OC_FILE_MAKETMP_DIRECTORY))
        {
            oc_log_error("Couldn't create tmp directory.\n");
            return -1;
        }
        oc_str8 tmpName = oc_catch(oc_file_name(scratch.arena, tmpDir))
        {
            oc_log_error("Couldn't get name of tmp directory.\n");
            return -1;
        }
        oc_str8 tmpFilesPath = oc_file_tmp_directory_path(scratch.arena);
        tmpPath = oc_path_append(scratch.arena, tmpFilesPath, tmpName);
        oc_file_close(tmpDir);
    }

    oc_str8 modPath = oc_path_append(scratch.arena, tmpPath, OC_STR8("modules/"));
    oc_io_error error = oc_file_makedir(modPath, &(oc_file_makedir_options){ .flags = OC_FILE_MAKEDIR_CREATE_PARENTS });
    if(error != OC_IO_OK)
    {
        oc_log_error("Couldn't create modules directory: %.*s\n",
                     oc_str8_ip(oc_io_error_string(error)));
    }

    oc_str8 resPath = oc_path_append(scratch.arena, tmpPath, OC_STR8("data/"));
    error = oc_file_makedir(resPath, &(oc_file_makedir_options){ .flags = OC_FILE_MAKEDIR_CREATE_PARENTS });
    if(error != OC_IO_OK)
    {
        oc_log_error("Couldn't create data directory: %.*s\n",
                     oc_str8_ip(oc_io_error_string(error)));
    }

    //NOTE: copy modules
    bool foundMain = false;
    for(int i = 0; i < options->moduleCount; i++)
    {
        if(!oc_str8_cmp(oc_path_slice_filename(options->modules[i]), OC_STR8("main.wasm")))
        {
            foundMain = true;
        }
        error = oc_file_copy(options->modules[i], modPath, 0);
        if(error != OC_IO_OK)
        {
            oc_log_error("Couldn't copy module %.*s: %.*s\n",
                         oc_str8_ip(options->modules[i]),
                         oc_str8_ip(oc_io_error_string(error)));
            return -1;
        }
    }
    if(!foundMain)
    {
        oc_log_error("error: bundle must have at least one module named 'main.wasm'.\n");
        return -1;
    }

    //NOTE: copy resources
    for(int i = 0; i < options->resDirCount; i++)
    {
        oc_str8 dir = options->resDirs[i];
        if(dir.len && dir.ptr[dir.len - 1] != '/')
        {
            dir = oc_str8_pushf(scratch.arena, "%.*s/", oc_str8_ip(dir));
        }
        error = oc_file_copy(dir, resPath, 0);
        if(error != OC_IO_OK)
        {
            oc_log_error("Couldn't copy resource dir %.*s: %.*s\n",
                         oc_str8_ip(dir),
                         oc_str8_ip(oc_io_error_string(error)));
        }
    }

    for(int i = 0; i < options->resFileCount; i++)
    {
        error = oc_file_copy(options->resFiles[i], resPath, 0);
        if(error != OC_IO_OK)
        {
            oc_log_error("Couldn't copy resource file %.*s: %.*s\n",
                         oc_str8_ip(options->resFiles[i]),
                         oc_str8_ip(oc_io_error_string(error)));
        }
    }

    //NOTE: copy icon
    if(options->icon.len)
    {
        oc_str8 ext = oc_path_slice_extension(options->icon);
        oc_str8 name = oc_str8_pushf(scratch.arena, "thumbnail%.*s", oc_str8_ip(ext));
        oc_str8 path = oc_path_append(scratch.arena, resPath, name);
        error = oc_file_copy(options->icon, path, 0);

        if(error != OC_IO_OK)
        {
            oc_log_error("Couldn't copy thumbnail %*s: %.*s\n",
                         oc_str8_ip(options->icon),
                         oc_str8_ip(oc_io_error_string(error)));
        }
    }

    //NOTE: zip folder to out directory
    int status = 0;
    oc_str8 outDir = options->standalone ? tmpPath : options->outDir;
    oc_str8 outFile = oc_str8_pushf(scratch.arena,
                                    "%.*s/%.*s.orca",
                                    oc_str8_ip(outDir),
                                    oc_str8_ip(options->name),
                                    OC_STR8(".orca"));

    zip_t* zip = zip_open(outFile.ptr, ZIP_CREATE | ZIP_TRUNCATE, 0);
    if(!zip)
    {
        oc_log_error("Couldn't create zip file %.*s\n", oc_str8_ip(outFile));
        return -1;
    }
    else
    {
        if(add_to_archive(zip, modPath, OC_STR8("modules")))
        {
            return -1;
        }
        if(add_to_archive(zip, resPath, OC_STR8("data")))
        {
            return -1;
        }
        zip_close(zip);
    }

    if(!status && options->standalone)
    {
#if OC_PLATFORM_MACOS
        status = oc_tool_bundle_standalone_macos(options, outFile);
#elif OC_PLATFORM_WINDOWS
        status = oc_tool_bundle_standalone_windows(options, outFile);
#endif
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
