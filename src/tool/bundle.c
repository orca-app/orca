/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

//#include <processenv.h>
#include <stdio.h>

#include "flag.h"
#include "orca.h"
#include "util.h"
#include "system.h"

#if OC_PLATFORM_WINDOWS
    #include "win32_icon.c"
#endif

int winBundle(
    oc_arena* a,
    oc_str8 name,
    oc_str8 icon,
    oc_str8 version,
    oc_str8_list resource_files,
    oc_str8_list resource_dirs,
    oc_str8 app_version,
    oc_str8 outDir,
    oc_str8 module);

int macBundle(
    oc_arena* a,
    oc_str8 name,
    oc_str8 icon,
    oc_str8 version,
    oc_str8_list resource_files,
    oc_str8_list resource_dirs,
    oc_str8 app_version,
    oc_str8 outDir,
    oc_str8 module,
    bool mtlEnableCapture);

int bundle(int argc, char** argv)
{
    oc_arena a;
    oc_arena_init(&a);

    Flag_Context c;
    flag_init_context(&c);

    flag_help(&c, "Packages a WebAssembly module into a standalone Orca application, along with any required assets.");

    char** name = flag_str(&c, "n", "name", "out", "the app's name");
    char** icon = flag_str(&c, "i", "icon", NULL, "an image file to use as the application's icon");
    char** version = flag_str(&c, "v", "version", NULL, "select a specific version of the Orca SDK (default is latest version)");
    oc_str8_list* resource_files = flag_strs(&c, "d", "resource", "copy a file to the app's resource directory");
    oc_str8_list* resource_dirs = flag_strs(&c, "D", "resource-dir", "copy the contents of a folder to the app's resource directory");
    char** app_version = flag_str(&c, NULL, "app-version", "0.0.0", "a version number to embed in the application bundle");
    char** outDir = flag_str(&c, "C", "out-dir", NULL, "where to place the final application bundle (defaults to the current directory)");
    bool* mtlEnableCapture = flag_bool(&c, "M", "mtl-enable-capture", false, "enable Metal frame capture in Xcode for the application bundle (macOS only)");

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

#if OC_PLATFORM_WINDOWS
    return winBundle(
        &a,
        OC_STR8(*name),
        OC_STR8(*icon),
        OC_STR8(*version),
        *resource_files,
        *resource_dirs,
        OC_STR8(*app_version),
        OC_STR8(*outDir),
        OC_STR8(*module));
#elif OC_PLATFORM_MACOS
    return macBundle(
        &a,
        OC_STR8(*name),
        OC_STR8(*icon),
        OC_STR8(*version),
        *resource_files,
        *resource_dirs,
        OC_STR8(*app_version),
        OC_STR8(*outDir),
        OC_STR8(*module),
        *mtlEnableCapture);
#else
    #error Can't build the bundle script on this platform!
#endif
}

#if OC_PLATFORM_WINDOWS

int winBundle(
    oc_arena* a,
    oc_str8 name,
    oc_str8 icon,
    oc_str8 version,
    oc_str8_list resource_files,
    oc_str8_list resource_dirs,
    oc_str8 app_version,
    oc_str8 outDir,
    oc_str8 module)
{
    //-----------------------------------------------------------
    //NOTE: make bundle directory structure
    //-----------------------------------------------------------
    oc_str8 bundleDir = oc_path_append(a, outDir, name);
    oc_str8 exeDir = oc_path_append(a, bundleDir, OC_STR8("bin"));
    oc_str8 resDir = oc_path_append(a, bundleDir, OC_STR8("resources"));
    oc_str8 guestDir = oc_path_append(a, bundleDir, OC_STR8("app"));
    oc_str8 wasmDir = oc_path_append(a, guestDir, OC_STR8("wasm"));
    oc_str8 dataDir = oc_path_append(a, guestDir, OC_STR8("data"));

    if(oc_sys_exists(bundleDir))
    {
        TRY(oc_sys_rmdir(bundleDir));
    }
    TRY(oc_sys_mkdirs(bundleDir));
    TRY(oc_sys_mkdirs(exeDir));
    TRY(oc_sys_mkdirs(resDir));
    TRY(oc_sys_mkdirs(guestDir));
    TRY(oc_sys_mkdirs(wasmDir));
    TRY(oc_sys_mkdirs(dataDir));

    oc_str8 sdk_dir = version.len > 0
        ? get_version_dir(a, version, true)
        : current_version_dir(a, true);

    //-----------------------------------------------------------
    //NOTE: link runtime objects and application icon into exe
    //-----------------------------------------------------------
    {
        oc_str8 temp_dir = oc_path_append(a, outDir, OC_STR8("temporary"));
        oc_str8 res_path = { 0 };
        if(icon.len > 0)
        {
            if (oc_sys_exists(icon))
            {
                if(oc_sys_exists(temp_dir))
                {
                    TRY(oc_sys_rmdir(temp_dir));
                }
                TRY(oc_sys_mkdirs(temp_dir));
                oc_str8 ico_path = oc_path_append(a, temp_dir, OC_STR8("icon.ico"));
                if(!icon_from_image(a, icon, ico_path))
                {
                    fprintf(stderr, "failed to create windows icon for \"%.*s\"\n", oc_str8_ip(icon));
                }

                res_path = oc_path_append(a, temp_dir, OC_STR8("icon.res"));
                if(!resource_file_from_icon(a, ico_path, res_path))
                {
                    fprintf(stderr, "failed to create windows resource file for \"%.*s\"\n", oc_str8_ip(ico_path));
                    res_path.ptr = 0;
                    res_path.len = 0;
                }

            }
            else
            {
                fprintf(stderr, "warning: failed to find icon file \"%.*s\"\n", oc_str8_ip(icon));
            }
        }

        oc_str8 exe_out = oc_path_append(a, exeDir, oc_str8_pushf(a, "%.*s.exe", oc_str8_ip(name)));
        oc_str8 libpath = oc_path_append(a, sdk_dir, OC_STR8("bin"));
        oc_str8 cmd = oc_str8_pushf(a, "link.exe /nologo /LIBPATH:%s runtime.obj orca.dll.lib wasm3.lib %.*s /out:%s",
                                    libpath.ptr, oc_str8_ip(res_path), exe_out.ptr);
        i32 result = system(cmd.ptr);
        oc_sys_rmdir(temp_dir);
        if(result)
        {
            fprintf(stderr, "failed to link application executable\n");
            return result;
        }
    }

    //-----------------------------------------------------------
    //NOTE: copy orca libraries
    //-----------------------------------------------------------
    oc_str8 orcaLib = oc_path_append(a, sdk_dir, OC_STR8("bin/orca.dll"));
    oc_str8 glesLib = oc_path_append(a, sdk_dir, OC_STR8("bin/libGLESv2.dll"));
    oc_str8 eglLib = oc_path_append(a, sdk_dir, OC_STR8("bin/libEGL.dll"));

    TRY(oc_sys_copy(orcaLib, exeDir));
    TRY(oc_sys_copy(glesLib, exeDir));
    TRY(oc_sys_copy(eglLib, exeDir));

    //-----------------------------------------------------------
    //NOTE: copy wasm module and data
    //-----------------------------------------------------------
    TRY(oc_sys_copy(module, oc_path_append(a, wasmDir, OC_STR8("/module.wasm"))));

    oc_str8_list_for(resource_files, it)
    {
        oc_str8 resource_file = it->string;
        if(oc_sys_isdir(resource_file))
        {
            printf("Error: Got %.*s as a resource file, but it is a directory. Ignoring.",
                   oc_str8_ip(resource_file));
        }
        else
        {
            TRY(oc_sys_copy(resource_file, dataDir));
        }
    }

    oc_str8_list_for(resource_dirs, it)
    {
        oc_str8 resource_dir = it->string;
        if(oc_sys_isdir(resource_dir))
        {
            TRY(oc_sys_copytree(resource_dir, dataDir));
        }
        else
        {
            printf("Error: Got %.*s as a resource dir, but it is not a directory. Ignoring.",
                   oc_str8_ip(resource_dir));
        }
    }

    //-----------------------------------------------------------
    //NOTE: copy runtime resources
    //-----------------------------------------------------------
    TRY(oc_sys_copy(oc_path_append(a, sdk_dir, OC_STR8("resources/Menlo.ttf")), resDir));
    TRY(oc_sys_copy(oc_path_append(a, sdk_dir, OC_STR8("resources/Menlo Bold.ttf")), resDir));

    return 0;
}
#elif OC_PLATFORM_MACOS

int macBundle(
    oc_arena* a,
    oc_str8 name,
    oc_str8 icon,
    oc_str8 version,
    oc_str8_list resource_files,
    oc_str8_list resource_dirs,
    oc_str8 app_version,
    oc_str8 outDir,
    oc_str8 module,
    bool mtlEnableCapture)
{
    //-----------------------------------------------------------
    //NOTE: make bundle directory structure
    //-----------------------------------------------------------
    oc_str8_list list = { 0 };
    oc_str8_list_push(a, &list, name);
    oc_str8_list_push(a, &list, OC_STR8(".app"));
    name = oc_str8_list_join(a, list);

    oc_str8 bundleDir = oc_path_append(a, outDir, name);
    oc_str8 contentsDir = oc_path_append(a, bundleDir, OC_STR8("Contents"));
    oc_str8 exeDir = oc_path_append(a, contentsDir, OC_STR8("MacOS"));
    oc_str8 resDir = oc_path_append(a, contentsDir, OC_STR8("resources"));
    oc_str8 guestDir = oc_path_append(a, contentsDir, OC_STR8("app"));
    oc_str8 wasmDir = oc_path_append(a, guestDir, OC_STR8("wasm"));
    oc_str8 dataDir = oc_path_append(a, guestDir, OC_STR8("data"));

    if(oc_sys_exists(bundleDir))
    {
        TRY(oc_sys_rmdir(bundleDir));
    }
    TRY(oc_sys_mkdirs(bundleDir));
    TRY(oc_sys_mkdirs(contentsDir));
    TRY(oc_sys_mkdirs(exeDir));
    TRY(oc_sys_mkdirs(resDir));
    TRY(oc_sys_mkdirs(guestDir));
    TRY(oc_sys_mkdirs(wasmDir));
    TRY(oc_sys_mkdirs(dataDir));

    oc_str8 sdk_dir = version.len > 0
        ? get_version_dir(a, version, true)
        : current_version_dir(a, true);

    //-----------------------------------------------------------
    //NOTE: copy orca runtime executable and libraries
    //-----------------------------------------------------------
    oc_str8 orcaExe = oc_path_append(a, sdk_dir, OC_STR8("bin/orca_runtime"));
    oc_str8 orcaLib = oc_path_append(a, sdk_dir, OC_STR8("bin/liborca.dylib"));
    oc_str8 glesLib = oc_path_append(a, sdk_dir, OC_STR8("bin/libGLESv2.dylib"));
    oc_str8 eglLib = oc_path_append(a, sdk_dir, OC_STR8("bin/libEGL.dylib"));
    oc_str8 renderer_lib = oc_path_append(a, sdk_dir, OC_STR8("bin/mtl_renderer.metallib"));

    TRY(oc_sys_copy(orcaExe, exeDir));
    TRY(oc_sys_copy(orcaLib, exeDir));
    TRY(oc_sys_copy(glesLib, exeDir));
    TRY(oc_sys_copy(eglLib, exeDir));
    TRY(oc_sys_copy(renderer_lib, exeDir));

    //-----------------------------------------------------------
    //NOTE: copy wasm module and data
    //-----------------------------------------------------------
    TRY(oc_sys_copy(module, oc_path_append(a, wasmDir, OC_STR8("/module.wasm"))));

    oc_str8_list_for(resource_files, it)
    {
        oc_str8 resource_file = it->string;
        if(oc_sys_isdir(resource_file))
        {
            printf("Error: Got %.*s as a resource file, but it is a directory. Ignoring.",
                   oc_str8_ip(resource_file));
        }
        else
        {
            TRY(oc_sys_copy(resource_file, dataDir));
        }
    }

    oc_str8_list_for(resource_dirs, it)
    {
        oc_str8 resource_dir = it->string;
        if(oc_sys_isdir(resource_dir))
        {
            // NOTE(shaw): trailing slash means that contents are copied rather
            // than the directory itself
            oc_str8 resource_dir_slash = oc_path_append(a, resource_dir, OC_STR8("/"));
            TRY(oc_sys_copytree(resource_dir_slash, dataDir));
        }
        else
        {
            printf("Error: Got %.*s as a resource dir, but it is not a directory. Ignoring.",
                   oc_str8_ip(resource_dir));
        }
    }

    //-----------------------------------------------------------
    //NOTE: copy runtime resources
    //-----------------------------------------------------------
    TRY(oc_sys_copy(oc_path_append(a, sdk_dir, OC_STR8("resources/Menlo.ttf")), resDir));
    TRY(oc_sys_copy(oc_path_append(a, sdk_dir, OC_STR8("resources/Menlo Bold.ttf")), resDir));

    //-----------------------------------------------------------
    //NOTE make icon
    //-----------------------------------------------------------
    if(icon.len)
    {
        if (oc_sys_exists(icon))
        {
            oc_str8 icon_dir = oc_path_slice_directory(icon);
            oc_str8 iconset = oc_path_append(a, icon_dir, OC_STR8("icon.iconset"));
            if(oc_sys_exists(iconset))
            {
                TRY(oc_sys_rmdir(iconset));
            }
            TRY(oc_sys_mkdirs(iconset));

            i32 size = 16;
            for(i32 i = 0; i < 7; ++i)
            {
                oc_str8 sized_icon = oc_path_append(a, iconset, oc_str8_pushf(a, "icon_%dx%d.png", size, size));
                oc_str8 cmd = oc_str8_pushf(a, "sips -z %d %d %.*s --out %s >/dev/null 2>&1",
                                            size, size, oc_str8_ip(icon), sized_icon.ptr);
                i32 result = system(cmd.ptr);
                if(result)
                {
                    fprintf(stderr, "failed to generate %dx%d icon from %.*s\n", size, size, oc_str8_ip(icon));
                }

                i32 retina_size = size * 2;
                oc_str8 retina_icon = oc_path_append(a, iconset, oc_str8_pushf(a, "icon_%dx%d@2x.png", size, size));
                cmd = oc_str8_pushf(a, "sips -z %d %d %.*s --out %s >/dev/null 2>&1",
                                    retina_size, retina_size, oc_str8_ip(icon), sized_icon.ptr);
                result = system(cmd.ptr);
                if(result)
                {
                    fprintf(stderr, "failed to generate %dx%d@2x retina icon from %.*s\n", size, size, oc_str8_ip(icon));
                }

                size *= 2;
            }

            oc_str8 icon_out = oc_path_append(a, resDir, OC_STR8("icon.icns"));
            oc_str8 cmd = oc_str8_pushf(a, "iconutil -c icns -o %s %s", icon_out.ptr, iconset.ptr);
            i32 result = system(cmd.ptr);
            if(result)
            {
                fprintf(stderr, "failed to generate app icon from %.*s", oc_str8_ip(icon));
            }
            TRY(oc_sys_rmdir(iconset));
        }
        else
        {
            fprintf(stderr, "warning: failed to find icon file \"%.*s\"\n", oc_str8_ip(icon));
        }
    }

    //-----------------------------------------------------------
    //NOTE: write plist file
    //-----------------------------------------------------------
    oc_str8 bundle_sig = OC_STR8("????");

    oc_str8 plist_contents = oc_str8_pushf(a,
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
                                           "%s"
                                           "</dict>"
                                           "</plist>",
                                           oc_str8_ip(name),
                                           oc_str8_ip(name),
                                           oc_str8_ip(name),
                                           oc_str8_ip(app_version),
                                           oc_str8_ip(bundle_sig),
                                           mtlEnableCapture ? "<key>MetalCaptureEnabled</key><true/>" : "");

    oc_str8 plist_path = oc_path_append(a, contentsDir, OC_STR8("Info.plist"));
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

    return 0;
}

#endif
