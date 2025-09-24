#define OC_NO_APP_LAYER 1
#include "orca.c"
#include "tool/system.c"

int make_dirs(oc_str8 path)
{
    int result = 0;
    oc_arena_scope scratch = oc_scratch_begin();

    oc_str8_list elements = oc_path_split(scratch.arena, path);

    oc_str8_list acc = { 0 };
    if(path.len && path.ptr[0] == '/')
    {
        ////////////////////////////////////////////////////
        //NOTE: should oc_path_split return root? otherwise we can't
        // differentiate between abs an relative path...
        ////////////////////////////////////////////////////
        oc_str8_list_push(scratch.arena, &acc, OC_STR8("/"));
    }

    oc_str8_list_for(elements, elt)
    {
        oc_str8_list_push(scratch.arena, &acc, elt->string);
        oc_str8 accPath = oc_path_join(scratch.arena, acc);

        struct stat st = { 0 };
        if(stat(accPath.ptr, &st) != 0)
        {
            // create the directory
            mkdir(accPath.ptr, 0700);
        }
        else if(!(st.st_mode & S_IFDIR))
        {
            // file exists, but not a directory. Error out.
            result = -1;
            goto end;
        }
    }

end:
    oc_scratch_end(scratch);
    return result;
}

void copy_headers(oc_str8 src, oc_str8 dst, oc_str8_list ignore)
{
    oc_arena_scope scratch = oc_scratch_begin();

    oc_file srcDir = oc_file_open(src, OC_FILE_ACCESS_READ, OC_FILE_OPEN_NONE);
    oc_file_list files = oc_file_listdir(scratch.arena, srcDir);
    oc_file_close(srcDir);

    oc_file_list_for(files, elt)
    {
        oc_str8 ext = oc_path_slice_extension(elt->basename);
        if(elt->type == OC_FILE_REGULAR && !oc_str8_cmp(ext, OC_STR8(".h")))
        {
            oc_str8 srcFile = oc_path_append(scratch.arena, src, elt->basename);
            oc_str8 dstFile = oc_path_append(scratch.arena, dst, elt->basename);

            make_dirs(dst);

            oc_sys_copy(srcFile, dstFile);
        }
        else if(elt->type == OC_FILE_DIRECTORY)
        {
            oc_str8 srcDir = oc_path_append(scratch.arena, src, elt->basename);
            oc_str8 dstDir = oc_path_append(scratch.arena, dst, elt->basename);

            bool skip = false;
            oc_str8_list_for(ignore, ignoreElt)
            {
                if(!oc_str8_cmp(ignoreElt->string, srcDir))
                {
                    skip = true;
                    break;
                }
            }
            if(!skip)
            {
                copy_headers(srcDir, dstDir, ignore);
            }
        }
    }

    oc_scratch_end(scratch);
}

int make_app_macos(void)
{
    oc_arena_scope scratch = oc_scratch_begin();

    oc_str8 name = OC_STR8("Orca");
    oc_str8 exec = OC_STR8("orca");
    oc_str8 icon = OC_STR8("resources/orca_icon.png");
    oc_str8 version = OC_STR8("0.0.1");
    oc_str8 outDir = OC_STR8("zig-out");

    //-----------------------------------------------------------
    //NOTE: make bundle directory structure
    //-----------------------------------------------------------
    oc_str8_list list = { 0 };
    oc_str8_list_push(scratch.arena, &list, name);
    oc_str8_list_push(scratch.arena, &list, OC_STR8(".app"));
    name = oc_str8_list_join(scratch.arena, list);

    oc_str8 bundleDir = oc_path_append(scratch.arena, outDir, name);
    oc_str8 contentsDir = oc_path_append(scratch.arena, bundleDir, OC_STR8("Contents"));
    oc_str8 exeDir = oc_path_append(scratch.arena, contentsDir, OC_STR8("MacOS"));
    oc_str8 resDir = oc_path_append(scratch.arena, contentsDir, OC_STR8("resources"));
    oc_str8 sdkDir = oc_path_append(scratch.arena, contentsDir, OC_STR8("SDK"));
    oc_str8 sdkSrcDir = oc_path_append(scratch.arena, sdkDir, OC_STR8("src"));
    oc_str8 sdkLibCDir = oc_path_append(scratch.arena, sdkDir, OC_STR8("orca-libc"));
    oc_str8 sdkLibDir = oc_path_append(scratch.arena, sdkDir, OC_STR8("lib"));

    if(oc_sys_exists(bundleDir))
    {
        TRY(oc_sys_rmdir(bundleDir));
    }
    TRY(oc_sys_mkdirs(bundleDir));
    TRY(oc_sys_mkdirs(contentsDir));
    TRY(oc_sys_mkdirs(exeDir));
    TRY(oc_sys_mkdirs(resDir));

    //-----------------------------------------------------------
    //NOTE: copy binaries
    //-----------------------------------------------------------
    TRY(oc_sys_copy(OC_STR8("./zig-out/bin/orca"), exeDir));
    TRY(oc_sys_copy(OC_STR8("./zig-out/bin/orca_runtime"), exeDir));
    TRY(oc_sys_copy(OC_STR8("./zig-out/bin/liborca_platform.dylib"), exeDir));
    TRY(oc_sys_copy(OC_STR8("./zig-out/bin/libEGL.dylib"), exeDir));
    TRY(oc_sys_copy(OC_STR8("./zig-out/bin/libGLESv2.dylib"), exeDir));
    TRY(oc_sys_copy(OC_STR8("./zig-out/bin/libwebgpu.dylib"), exeDir));

    //-----------------------------------------------------------
    //NOTE: copy data
    //-----------------------------------------------------------
    TRY(oc_sys_copy(OC_STR8("resources/Menlo.ttf"), resDir));
    TRY(oc_sys_copy(OC_STR8("resources/Menlo Bold.ttf"), resDir));

    //-----------------------------------------------------------
    //NOTE copy SDK
    //-----------------------------------------------------------
    oc_str8_list ignore = { 0 };

    copy_headers(OC_STR8("./src/app"), oc_path_append(scratch.arena, sdkSrcDir, OC_STR8("app")), ignore);
    copy_headers(OC_STR8("./src/graphics"), oc_path_append(scratch.arena, sdkSrcDir, OC_STR8("graphics")), ignore);
    copy_headers(OC_STR8("./src/platform"), oc_path_append(scratch.arena, sdkSrcDir, OC_STR8("platform")), ignore);
    copy_headers(OC_STR8("./src/ui"), oc_path_append(scratch.arena, sdkSrcDir, OC_STR8("ui")), ignore);
    copy_headers(OC_STR8("./src/util"), oc_path_append(scratch.arena, sdkSrcDir, OC_STR8("util")), ignore);

    copy_headers(OC_STR8("./src/ext/angle"), oc_path_append(scratch.arena, sdkSrcDir, OC_STR8("ext/angle")), ignore);
    copy_headers(OC_STR8("./src/ext/dawn"), oc_path_append(scratch.arena, sdkSrcDir, OC_STR8("ext/dawn")), ignore);
    copy_headers(OC_STR8("./src/ext/GL"), oc_path_append(scratch.arena, sdkSrcDir, OC_STR8("ext/GL")), ignore);
    copy_headers(OC_STR8("./src/ext/KHR"), oc_path_append(scratch.arena, sdkSrcDir, OC_STR8("ext/KHR")), ignore);
    copy_headers(OC_STR8("./src/ext/stb"), oc_path_append(scratch.arena, sdkSrcDir, OC_STR8("ext/stb")), ignore);

    oc_sys_copy(OC_STR8("./src/orca.h"), oc_path_append(scratch.arena, sdkSrcDir, OC_STR8("orca.h")));

    oc_sys_copytree(OC_STR8("./zig-out/orca-libc"), sdkLibCDir);

    oc_sys_mkdirs(sdkLibDir);
    TRY(oc_sys_copy(OC_STR8("./zig-out/bin/liborca_wasm.a"), oc_path_append(scratch.arena, sdkLibDir, OC_STR8("liborca_wasm.a"))));
    //-----------------------------------------------------------
    //NOTE make icon
    //-----------------------------------------------------------
    if(icon.len)
    {
        if(oc_sys_exists(icon))
        {
            oc_str8 icon_dir = oc_path_slice_directory(icon);
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
                                            size, size, oc_str8_ip(icon), sized_icon.ptr);
                i32 result = system(cmd.ptr);
                if(result)
                {
                    fprintf(stderr, "failed to generate %dx%d icon from %.*s\n", size, size, oc_str8_ip(icon));
                }

                i32 retina_size = size * 2;
                oc_str8 retina_icon = oc_path_append(scratch.arena, iconset, oc_str8_pushf(scratch.arena, "icon_%dx%d@2x.png", size, size));
                cmd = oc_str8_pushf(scratch.arena, "sips -z %d %d %.*s --out %s >/dev/null 2>&1",
                                    retina_size, retina_size, oc_str8_ip(icon), sized_icon.ptr);
                result = system(cmd.ptr);
                if(result)
                {
                    fprintf(stderr, "failed to generate %dx%d@2x retina icon from %.*s\n", size, size, oc_str8_ip(icon));
                }

                size *= 2;
            }

            oc_str8 icon_out = oc_path_append(scratch.arena, resDir, OC_STR8("icon.icns"));
            oc_str8 cmd = oc_str8_pushf(scratch.arena, "iconutil -c icns -o %s %s", icon_out.ptr, iconset.ptr);
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
                                           "<string>%.*s</string>"
                                           "<key>CFBundleIconFile</key>"
                                           "<string>icon.icns</string>"
                                           "<key>NSHighResolutionCapable</key>"
                                           "<string>True</string>"
                                           "<key>CFBundleDocumentTypes</key>"
                                           "<array>"
                                           "<dict>"
                                           "    <key>CFBundleTypeRole</key>"
                                           "        <string>Viewer</string>"
                                           "    <key>CFBundleTypeExtensions</key>"
                                           "    <array>"
                                           "        <string>orca</string>"
                                           "    </array>"
                                           "    <key>CFBundleTypeIconFile</key>"
                                           "        <string></string>"
                                           "    <key>CFBundleTypeMIMETypes</key>"
                                           "        <string>application/octet-stream</string>"
                                           "    <key>CFBundleTypeName</key>"
                                           "        <string>Orca Application</string>"
                                           "    <key>CFBundleTypeOSTypes</key>"
                                           "    <array>"
                                           "        <string>****</string>"
                                           "    </array>"
                                           "</dict>"
                                           "</array>"
                                           "</dict>"
                                           "</plist>",
                                           oc_str8_ip(name),
                                           oc_str8_ip(name),
                                           oc_str8_ip(name),
                                           oc_str8_ip(version),
                                           oc_str8_ip(bundle_sig),
                                           oc_str8_ip(exec));

    oc_str8 plist_path = oc_path_append(scratch.arena, contentsDir, OC_STR8("Info.plist"));
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

int main(int argc, char** argv)
{
    return make_app_macos();
}
