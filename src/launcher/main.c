#include <stdio.h>
#include <sys/stat.h>

#include "orca.h"
#include "ext/libzip/lib/zip.h"
#include "tool/tool.c"

//TODO: this should be fleshed out and put in src/tool

int oc_tool_install(oc_tool_options* options) { return 0; }

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

int oc_zip_extract(oc_str8 src, oc_str8 dst)
{
    oc_arena_scope scratch = oc_scratch_begin();
    const char* srcCStr = oc_str8_to_cstring(scratch.arena, src);

    zip_t* zip = zip_open(srcCStr, ZIP_RDONLY, 0);
    if(!zip)
    {
        goto error;
    }

    i64 count = zip_get_num_entries(zip, 0);
    for(i64 entryIndex = 0; entryIndex < count; entryIndex++)
    {
        oc_str8 name = OC_STR8(zip_get_name(zip, entryIndex, 0));
        if(!name.ptr)
        {
            goto error;
        }
        else
        {
            oc_str8_list list = { 0 };
            oc_str8_list_push(scratch.arena, &list, dst);
            oc_str8_list_push(scratch.arena, &list, name);
            oc_str8 dstPath = oc_path_join(scratch.arena, list);

            if(name.ptr[name.len - 1] == '/')
            {
                //NOTE: directory. Ignore since we create dirs when extracting files
            }
            else if(!oc_str8_cmp(oc_str8_slice(name, 0, 8), OC_STR8("__MACOSX")))
            {
                //NOTE: apple archiver's way of storing resource forks. Ignore
            }
            else
            {
                //NOTE: normal file
                // first make the leading directories if they don't exist
                oc_str8 dir = oc_path_slice_directory(dstPath);
                if(make_dirs(dir) != 0)
                {
                    goto error;
                }

                zip_file_t* srcFile = zip_fopen_index(zip, entryIndex, 0);
                if(!srcFile)
                {
                    goto error;
                }
                FILE* dstFile = fopen(dstPath.ptr, "w");
                if(!dstFile)
                {
                    goto error;
                }

                char chunk[1024];
                while(1)
                {
                    i64 n = zip_fread(srcFile, chunk, 1024);
                    if(n == 0)
                    {
                        break;
                    }
                    else if(n == -1)
                    {
                        fclose(dstFile);
                        zip_fclose(srcFile);
                        goto error;
                    }
                    else
                    {
                        fwrite(chunk, 1, n, dstFile);
                    }
                }

                fclose(dstFile);
                zip_fclose(srcFile);
            }
        }
    }

    zip_close(zip);
    oc_scratch_end(scratch);
    return 0;

error:
    if(zip)
    {
        zip_close(zip);
    }
    oc_scratch_end(scratch);
    return -1;
}

oc_str8 get_orca_home_dir(oc_arena* arena)
{
    oc_arena_scope scratch = oc_scratch_begin_next(arena);

    char* home = getenv("HOME");

    oc_str8_list list = { 0 };
    oc_str8_list_push(scratch.arena, &list, OC_STR8(home));
    oc_str8_list_push(scratch.arena, &list, OC_STR8(".orca"));

    oc_str8 path = oc_path_join(arena, list);

    oc_scratch_end(scratch);

    return path;
}

int oc_tool_run(oc_tool_options* options)
{
    int status = 0;
    oc_arena_scope scratch = oc_scratch_begin();

    //TODO: if options->app is a URL, download it here

    oc_str8 runtimeExe = oc_path_executable_relative(scratch.arena, OC_STR8("./orca_runtime"));

    //TODO: we don't want to capture ouput from that process...
    char* appCStr = oc_str8_to_cstring(scratch.arena, options->app);
    oc_subprocess_spawn_result result = oc_subprocess_spawn(2, (char*[]){ runtimeExe.ptr, appCStr }, 0);

    if(!oc_check(result))
    {
        oc_log_error("Couldn't launch the orca runtime (error = %i).\n", result.error);
        status = -1;
    }

    oc_scratch_end(scratch);
    return status;
}

int main(int argc, char** argv)
{
    oc_arena_scope scratch = oc_scratch_begin();

    oc_tool_options options = { 0 };
    oc_arg_parser parser = { 0 };

    oc_arg_parser_init(&parser,
                       scratch.arena,
                       OC_STR8("orca"),
                       &(oc_arg_parser_options){
                           .desc = OC_STR8("The Orca command line interface provides a number of devtools to make,"
                                           " install, and run Orca apps from a terminal."),
                       });

    //NOTE: version subparser
    oc_arg_parser* version = oc_arg_parser_subparser(&parser,
                                                     OC_STR8("version"),
                                                     &options.command,
                                                     &(oc_arg_parser_options){
                                                         .desc = OC_STR8("Print the version of the Orca dev tools."),
                                                     });
    //NOTE: sdk-path subparser
    oc_arg_parser* sdk_path = oc_arg_parser_subparser(&parser,
                                                      OC_STR8("sdk-path"),
                                                      &options.command,
                                                      &(oc_arg_parser_options){
                                                          .desc = OC_STR8("Print the path of the Orca SDK."),
                                                      });

    //NOTE: bundle subparser
    oc_arg_parser* bundle = oc_arg_parser_subparser(&parser,
                                                    OC_STR8("bundle"),
                                                    &options.command,
                                                    &(oc_arg_parser_options){
                                                        .desc = OC_STR8("Bundle wasm modules and resources into an Orca applications."),
                                                    });

    oc_arg_parser_add_flag(bundle,
                           OC_STR8("standalone"),
                           &options.standalone,
                           &(oc_arg_parser_arg_options){
                               .desc = OC_STR8("Make a standalone application."),
                           });
    oc_arg_parser_add_named_str8(bundle,
                                 OC_STR8("name"),
                                 &options.name,
                                 &(oc_arg_parser_arg_options){
                                     .desc = OC_STR8("Specifies the name of the application."),
                                     .required = true,
                                 });
    oc_arg_parser_add_named_str8(bundle,
                                 OC_STR8("app-version"),
                                 &options.appVersion,
                                 &(oc_arg_parser_arg_options){
                                     .desc = OC_STR8("Specifies a version number for the application."),
                                     .defaultValue.valStr8 = OC_STR8("0.0.0"),
                                 });
    oc_arg_parser_add_named_str8(bundle,
                                 OC_STR8("icon"),
                                 &options.icon,
                                 &(oc_arg_parser_arg_options){
                                     .desc = OC_STR8("Specifies the path to an image to use as the application's icon."),
                                 });
    oc_arg_parser_add_named_str8_array(bundle,
                                       OC_STR8("resource-file"),
                                       &options.resFileCount,
                                       &options.resFiles,
                                       &(oc_arg_parser_arg_options){
                                           .valueName = OC_STR8("file"),
                                           .desc = OC_STR8("Copy file to the application's data folder."),
                                           .allowRepeat = true,
                                       });
    oc_arg_parser_add_named_str8_array(bundle,
                                       OC_STR8("resource-dir"),
                                       &options.resDirCount,
                                       &options.resDirs,
                                       &(oc_arg_parser_arg_options){
                                           .valueName = OC_STR8("dir"),
                                           .desc = OC_STR8("Copy the contents of dir to the application's data folder."),
                                           .allowRepeat = true,
                                       });
    oc_arg_parser_add_named_str8(bundle,
                                 OC_STR8("out-dir"),
                                 &options.outDir,
                                 &(oc_arg_parser_arg_options){
                                     .valueName = OC_STR8("dir"),
                                     .desc = OC_STR8("Place the application in dir (defaults to the working directory.)"),
                                     .defaultValue.valStr8 = OC_STR8("."),
                                 });

    oc_arg_parser_add_positional_str8_array(bundle,
                                            OC_STR8("module"),
                                            &options.moduleCount,
                                            &options.modules,
                                            &(oc_arg_parser_arg_options){
                                                .desc = OC_STR8("Copy webassembly modules to the application's modules folder."),
                                                .required = true,
                                                .nargs = -1,
                                            });
    //NOTE: install subparser
    oc_arg_parser* install = oc_arg_parser_subparser(&parser,
                                                     OC_STR8("install"),
                                                     &options.command,
                                                     &(oc_arg_parser_options){
                                                         .desc = OC_STR8("Install an Orca app into the local app library."),
                                                     });

    oc_arg_parser_add_named_str8(install,
                                 OC_STR8("name"),
                                 &options.installName,
                                 &(oc_arg_parser_arg_options){
                                     .valueName = OC_STR8("name"),
                                     .desc = OC_STR8("Specifies the name under which to install the app."),
                                 });

    oc_arg_parser_add_positional_str8(install,
                                      OC_STR8("app"),
                                      &options.app,
                                      &(oc_arg_parser_arg_options){
                                          .required = true,
                                          .desc = OC_STR8("The app to install. <app> can be a path to an application file,"
                                                          " or an orca URL."),
                                      });

    //NOTE: run subparser
    oc_arg_parser* run = oc_arg_parser_subparser(&parser,
                                                 OC_STR8("run"),
                                                 &options.command,
                                                 &(oc_arg_parser_options){
                                                     .desc = OC_STR8("Install an Orca app into the local app library."),
                                                 });

    oc_arg_parser_add_positional_str8(run,
                                      OC_STR8("app"),
                                      &options.app,
                                      &(oc_arg_parser_arg_options){
                                          .required = true,
                                          .desc = OC_STR8("The app to run. <app> can be the name of an installed app,"
                                                          " a path to an application file, or an orca URL."),
                                      });

    if(oc_arg_parser_parse(&parser, argc, argv) == 0)
    {
        if(!oc_str8_cmp(options.command, OC_STR8("version")))
        {
            return oc_tool_version(&options);
        }
        else if(!oc_str8_cmp(options.command, OC_STR8("sdk-path")))
        {
            return oc_tool_sdk_path(&options);
        }
        else if(!oc_str8_cmp(options.command, OC_STR8("bundle")))
        {
            return oc_tool_bundle(&options);
        }
        else if(!oc_str8_cmp(options.command, OC_STR8("install")))
        {
            return oc_tool_install(&options);
        }
        else if(!oc_str8_cmp(options.command, OC_STR8("run")))
        {
            return oc_tool_run(&options);
        }
    }
    else
    {
        return -1;
    }

    //NOTE: if we didn't have any command, start the launcher

    printf("starting in launcher mode...\n");

    oc_init();
    while(!oc_should_quit())
    {
        oc_pump_events(-1);

        oc_arena_scope scratch = oc_scratch_begin();
        oc_event* event = 0;
        while((event = oc_next_event(scratch.arena)) != 0)
        {
            switch(event->type)
            {
                case OC_EVENT_QUIT:
                {
                    oc_request_quit();
                }
                break;
                case OC_EVENT_PATHDROP:
                {
                    oc_str8 path = { 0 };
                    oc_str8_list_for(event->paths, elt)
                    {
                        path = elt->string;
                        break;
                    }
                    //NOTE: run app
                    return oc_tool_run(&(oc_tool_options){ .app = path });
                }
                break;

                default:
                    break;
            }
        }
        oc_scratch_end(scratch);
    }
    oc_terminate();
    return 0;
}
