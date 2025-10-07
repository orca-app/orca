#include <stdio.h>
#include <sys/stat.h>

#include "orca.h"
#include "ext/libzip/lib/zip.h"
#include "tool.c"

//TODO: this should be fleshed out and put in src/tool

int oc_tool_install(oc_tool_options* options) { return 0; }

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
                if(oc_file_makedir(dir, &(oc_file_makedir_options){ .flags = OC_FILE_MAKEDIR_CREATE_PARENTS }) != 0)
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

//TODO: put in common with runtime code
#if OC_PLATFORM_MACOS
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

#elif OC_PLATFORM_WINDOWS

oc_str8 get_orca_home_dir(oc_arena* arena)
{
    oc_arena_scope scratch = oc_scratch_begin_next(arena);

    char* home = getenv("USERPROFILE");

    oc_str8_list list = { 0 };
    oc_str8_list_push(scratch.arena, &list, OC_STR8(home));
    oc_str8_list_push(scratch.arena, &list, OC_STR8("AppData/orca"));

    oc_str8 path = oc_path_join(arena, list);

    oc_scratch_end(scratch);

    return path;
}

#endif

int oc_tool_run(oc_tool_options* options)
{
    int status = 0;
    oc_arena_scope scratch = oc_scratch_begin();

    //TODO: if options->app is a URL, download it here

    oc_str8 runtimeExe = oc_path_executable_relative(scratch.arena, OC_STR8("./orca_runtime"));

    //TODO: we don't want to capture ouput from that process...
    char* appCStr = oc_str8_to_cstring(scratch.arena, options->app);
    oc_subprocess_spawn_result result = oc_subprocess_spawn(2, (char*[]){ runtimeExe.ptr, appCStr }, 0);

    if(!oc_result_check(result))
    {
        oc_log_error("Couldn't launch the orca runtime (error = %i).\n", result.error);
        status = -1;
    }

    oc_scratch_end(scratch);
    return status;
}

typedef struct oc_launcher_item
{
    oc_list_elt listElt;
    oc_str8 name;
    oc_str8 path;
    oc_image thumbnail;

} oc_launcher_item;

typedef struct oc_launcher
{
    oc_window window;
    oc_surface surface;
    oc_canvas_renderer renderer;
    oc_canvas_context canvas;
    oc_font font;
    oc_ui_context* ui;

    oc_arena libraryArena;
    oc_list items;

    oc_arena searchBarArena;

} oc_launcher;

int launcher_load_apps(oc_launcher* launcher)
{
    oc_arena_scope scratch = oc_scratch_begin();

    oc_str8 home = get_orca_home_dir(scratch.arena);
    oc_str8 appsPath = oc_path_append(scratch.arena, home, OC_STR8("apps"));

    oc_file appsDir = oc_catch(oc_file_open(appsPath, OC_FILE_ACCESS_READ, 0))
    {
        oc_log_error("Could not load local apps library...");
        return -1;
    }

    oc_file_list list = oc_file_listdir(scratch.arena, appsDir);
    oc_file_close(appsDir);

    oc_file_list_for(list, elt)
    {
        oc_str8 ext = oc_path_slice_extension(elt->basename);
        if(!oc_str8_cmp(ext, OC_STR8(".orca")))
        {
            oc_launcher_item* item = oc_arena_push_type(&launcher->libraryArena, oc_launcher_item);
            item->path = oc_path_append(&launcher->libraryArena, appsPath, elt->basename);
            item->name = oc_path_slice_stem(item->path);

            //NOTE: load thumbnail
            zip_t* zip = zip_open(item->path.ptr, ZIP_RDONLY, 0);
            if(!zip)
            {
                oc_log_error("Couldn't open zip archive.\n");
            }
            else
            {
                //NOTE: try to find a jpg or png image for thumbnail
                const char* thumbnailPath = "data/thumbnail.jpg";
                zip_stat_t stat = { 0 };
                int r = zip_stat(zip, thumbnailPath, 0, &stat);
                if(r)
                {
                    if(zip_error_code_zip(zip_get_error(zip)) == ZIP_ER_NOENT)
                    {
                        thumbnailPath = "data/thumbnail.png";
                        r = zip_stat(zip, thumbnailPath, 0, &stat);
                    }
                }

                if(r)
                {
                    oc_log_error("Couldn't stat thumbnail file.\n");
                }
                else
                {
                    zip_file_t* zipFile = zip_fopen(zip, thumbnailPath, 0);
                    if(!zipFile)
                    {
                        oc_log_error("Couldn't open thumbnail file.\n");
                    }
                    else
                    {
                        oc_str8 data = { 0 };
                        data.len = stat.size;
                        data.ptr = oc_arena_push_array(scratch.arena, char, data.len);
                        data.len = zip_fread(zipFile, data.ptr, data.len);

                        item->thumbnail = oc_image_create_from_memory(launcher->renderer, data, false);

                        zip_fclose(zipFile);
                    }
                }
                zip_close(zip);
            }

            //TODO: load other metadata

            oc_list_push_back(&launcher->items, &item->listElt);
        }
    }

    oc_scratch_end(scratch);
    return 0;
}

i32 launcher_create(void* userData)
{
    oc_launcher* launcher = (oc_launcher*)userData;

    // create rendering resources
    oc_rect rect = { 0, 0, 1200, 900 };
    launcher->window = oc_window_create(rect, OC_STR8("Orca Launcher"), OC_WINDOW_STYLE_TRANSPARENT);
    launcher->renderer = oc_canvas_renderer_create();
    launcher->surface = oc_canvas_surface_create_for_window(launcher->renderer, launcher->window);

    launcher->canvas = oc_canvas_context_create();
    oc_canvas_context_select(oc_canvas_context_nil());

    // load font
    oc_arena_scope scratch = oc_scratch_begin();

    oc_str8 fontPath = oc_path_executable_relative(scratch.arena, OC_STR8("../resources/Menlo.ttf"));
    //TODO: get rid of that soon
    oc_unicode_range ranges[5] = { OC_UNICODE_BASIC_LATIN,
                                   OC_UNICODE_C1_CONTROLS_AND_LATIN_1_SUPPLEMENT,
                                   OC_UNICODE_LATIN_EXTENDED_A,
                                   OC_UNICODE_LATIN_EXTENDED_B,
                                   OC_UNICODE_SPECIALS };

    launcher->font = oc_font_create_from_path(fontPath, 5, ranges);

    oc_scratch_end(scratch);

    // create ui
    launcher->ui = oc_ui_context_create(launcher->font);
    oc_ui_set_context(0);

    // load library
    oc_arena_init(&launcher->libraryArena);
    launcher_load_apps(launcher);

    // init search bar arena
    oc_arena_init(&launcher->searchBarArena);

    // show window
    oc_window_center(launcher->window);
    oc_window_bring_to_front(launcher->window);
    oc_window_focus(launcher->window);

    return 0;
}

i32 launcher_destroy(void* userData)
{
    oc_launcher* launcher = (oc_launcher*)userData;

    oc_arena_cleanup(&launcher->searchBarArena);
    oc_arena_cleanup(&launcher->libraryArena);

    oc_ui_context_destroy(launcher->ui);
    oc_font_destroy(launcher->font);
    oc_canvas_context_destroy(launcher->canvas);
    oc_surface_destroy(launcher->surface);
    oc_canvas_renderer_destroy(launcher->renderer);
    oc_window_destroy(launcher->window);

    return 0;
}

void oc_launcher_item_draw_thumbnail(oc_ui_box* box, void* userData)
{
    oc_launcher_item* item = (oc_launcher_item*)userData;
    oc_image_draw(item->thumbnail, box->rect);

    if(oc_ui_box_get_sig(box).hover)
    {
        oc_set_color_rgba(1, 1, 1, 0.3);
        oc_rectangle_fill(box->rect.x, box->rect.y, box->rect.w, box->rect.h);
    }
}

size_t curl_write(void* ptr, size_t size, size_t nmemb, void* stream)
{
    oc_file dst = *(oc_file*)(stream);
    size_t written = oc_file_write(dst, size * nmemb, ptr);
    return written;
}

#include "curl/curl.h"

void launcher_load_app_from_url(oc_str8 url)
{
    oc_arena_scope scratch = oc_scratch_begin();

    oc_str8 basename = oc_path_slice_filename(url);
    oc_str8 ext = oc_path_slice_extension(url);
    if(oc_str8_cmp(ext, OC_STR8(".orca")))
    {
        //TODO
        oc_log_error("Not an orca file.\n");
        return;
    }

    oc_str8 home = get_orca_home_dir(scratch.arena);
    oc_str8 transientAppsPath = oc_path_append(scratch.arena, home, OC_STR8("apps/transient"));
    oc_str8 dstPath = oc_path_append(scratch.arena, transientAppsPath, basename);

    oc_io_error error = oc_file_makedir(transientAppsPath,
                                        &(oc_file_makedir_options){
                                            .flags = OC_FILE_MAKEDIR_CREATE_PARENTS | OC_FILE_MAKEDIR_IGNORE_EXISTING,
                                        });
    if(error)
    {
        //TODO error
        oc_log_error("Couldn't create transient apps dir.\n");
        return;
    }

    oc_file dst = oc_catch(oc_file_open(dstPath,
                                        OC_FILE_ACCESS_WRITE,
                                        &(oc_file_open_options){
                                            .flags = OC_FILE_OPEN_CREATE | OC_FILE_OPEN_TRUNCATE,
                                        }))
    {
        //TODO
        oc_log_error("Couldn't create download file.\n");
        return;
    }

    curl_global_init(CURL_GLOBAL_ALL);
    CURL* curl = curl_easy_init();
    const char* urlCString = oc_str8_to_cstring(scratch.arena, url);
    curl_easy_setopt(curl, CURLOPT_URL, urlCString);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &dst);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    oc_file_close(dst);

    oc_tool_run(&(oc_tool_options){ .app = dstPath });

    oc_scratch_end(scratch);
}

i32 launcher_runloop(void* data)
{
    oc_launcher launcher = { 0 };
    bool finishedLaunching = false; //TODO: put in launcher struct as "initialized"?
    bool quit = false;

    while(!quit)
    {
        oc_arena_scope scratch = oc_scratch_begin();
        oc_event* event = 0;
        while((event = oc_next_event(scratch.arena)) != 0)
        {
            if(finishedLaunching)
            {
                oc_ui_set_context(launcher.ui);
                oc_ui_process_event(event);
            }
            switch(event->type)
            {
                case OC_EVENT_QUIT:
                case OC_EVENT_WINDOW_CLOSE:
                {
                    quit = true;
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
                    if(!finishedLaunching)
                    {
                        //NOTE: run app
                        return oc_tool_run(&(oc_tool_options){ .app = path });
                    }
                }
                break;

                case OC_EVENT_FINISH_LAUNCHING:
                {
                    finishedLaunching = true;
                    //NOTE: if we got here, we were not opened from an "open with" action,
                    // so we can create the window etc.
                    oc_dispatch_on_main_thread_sync(launcher_create, &launcher);
                }
                break;

                default:
                    break;
            }
        }
        oc_scratch_end(scratch);

        if(finishedLaunching)
        {
            oc_canvas_context_select(launcher.canvas);
            //            oc_set_color_rgba(1, 0, 1, 1);
            //            oc_clear();

            oc_rect rect = oc_window_get_content_rect(launcher.window);

            const int APP_THUMBNAIL_SIZE = 200;
            const int APP_ITEM_WIDTH = 200;

            oc_ui_frame(rect.wh)
            {
                oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);

                oc_ui_box("search-bar-frame")
                {
                    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
                    oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
                    oc_ui_style_set_f32(OC_UI_MARGIN_Y, 10);
                    oc_ui_style_set_i32(OC_UI_ALIGN_X, OC_UI_ALIGN_CENTER);

                    oc_ui_style_rule("search-bar")
                    {
                        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 0.7 });
                    }

                    static oc_ui_text_box_info searchBar = {
                        .defaultText = OC_STR8_LIT("Enter app URL here..."),
                    };
                    oc_ui_text_box_result result = oc_ui_text_box("search-bar", scratch.arena, &searchBar);
                    if(result.changed)
                    {
                        oc_arena_clear(&launcher.searchBarArena);
                        searchBar.text = oc_str8_push_copy(&launcher.searchBarArena, result.text);
                    }
                    if(result.accepted)
                    {
                        launcher_load_app_from_url(searchBar.text);
                        searchBar.text = (oc_str8){ 0 };
                        oc_arena_clear(&launcher.searchBarArena);
                    }
                }
                oc_ui_box("library")
                {
                    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
                    oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PARENT, 1, 1 });

                    oc_vec2 containerSize = rect.wh;

                    u32 maxThumbnailsPerRow = (u32)containerSize.x / APP_THUMBNAIL_SIZE;
                    f32 thumbnailSpacing = (containerSize.x - maxThumbnailsPerRow * APP_THUMBNAIL_SIZE) / (maxThumbnailsPerRow + 1);

                    oc_vec2 pos = { thumbnailSpacing, 0 };
                    oc_arena_scope scratch = oc_scratch_begin();
                    int colIndex = 0;

                    oc_list_for_indexed(launcher.items, it, oc_launcher_item, listElt)
                    {
                        oc_launcher_item* item = it.elt;

                        if(colIndex >= maxThumbnailsPerRow)
                        {
                            colIndex = 0;
                            pos.x = thumbnailSpacing;
                            pos.y += APP_THUMBNAIL_SIZE + 30;
                        }

                        oc_str8 idStr = oc_str8_pushf(scratch.arena, "item-%i", it.index);
                        oc_ui_box_str8(idStr)
                        {
                            oc_ui_style_set_i32(OC_UI_FLOATING_X, 1);
                            oc_ui_style_set_i32(OC_UI_FLOATING_Y, 1);
                            oc_ui_style_set_f32(OC_UI_FLOAT_TARGET_X, pos.x);
                            oc_ui_style_set_f32(OC_UI_FLOAT_TARGET_Y, pos.y);
                            oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
                            oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_CHILDREN });
                            oc_ui_style_set_f32(OC_UI_MARGIN_X, 10);
                            oc_ui_style_set_f32(OC_UI_MARGIN_Y, 10);

                            oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
                            oc_ui_style_set_i32(OC_UI_ALIGN_X, OC_UI_ALIGN_CENTER);

                            oc_ui_style_set_i32(OC_UI_ANIMATION_MASK,
                                                OC_UI_MASK_SIZE_WIDTH
                                                    | OC_UI_MASK_SIZE_HEIGHT
                                                    | OC_UI_MASK_FLOAT_TARGET_X
                                                    | OC_UI_MASK_FLOAT_TARGET_Y);

                            oc_ui_style_set_f32(OC_UI_ANIMATION_TIME, 0.4);

                            oc_ui_tag("item");
                            oc_ui_style_rule(".item.hover")
                            {
                                oc_ui_style_set_f32(OC_UI_FLOAT_TARGET_X, pos.x - 5);
                                oc_ui_style_set_f32(OC_UI_FLOAT_TARGET_Y, pos.y - 5);
                            }
                            oc_ui_style_rule(".hover thumbnail-frame")
                            {
                                oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, APP_THUMBNAIL_SIZE + 10 });
                                oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, APP_THUMBNAIL_SIZE + 10 });
                            }

                            oc_ui_box("thumbnail-frame")
                            {
                                oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, APP_THUMBNAIL_SIZE });
                                oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, APP_THUMBNAIL_SIZE });
                                oc_ui_style_set_f32(OC_UI_MARGIN_X, 10);
                                oc_ui_style_set_f32(OC_UI_MARGIN_Y, 10);

                                oc_ui_style_set_i32(OC_UI_ANIMATION_MASK,
                                                    OC_UI_MASK_SIZE_WIDTH
                                                        | OC_UI_MASK_SIZE_HEIGHT
                                                        | OC_UI_MASK_FLOAT_TARGET_X
                                                        | OC_UI_MASK_FLOAT_TARGET_Y);

                                oc_ui_style_set_f32(OC_UI_ANIMATION_TIME, 0.4);

                                oc_ui_box("thumbnail")
                                {
                                    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
                                    oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
                                    oc_ui_box_set_draw_proc(oc_ui_box_top(), oc_launcher_item_draw_thumbnail, item);
                                }
                            }

                            oc_ui_label_str8(OC_STR8("app-name"), item->name);

                            if(oc_ui_get_sig().doubleClicked)
                            {
                                oc_tool_run(&(oc_tool_options){
                                    .app = item->path,
                                });
                            }
                        }

                        pos.x += APP_THUMBNAIL_SIZE + thumbnailSpacing;
                        colIndex++;
                    }
                }
                oc_scratch_end(scratch);
            }
            oc_ui_draw();

            oc_canvas_render(launcher.renderer, launcher.canvas, launcher.surface);
            oc_canvas_present(launcher.renderer, launcher.surface);
        }
    }

    oc_dispatch_on_main_thread_sync(launcher_destroy, &launcher);
    oc_request_quit();

    return 0;
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
    oc_init();

    oc_thread* runloopThread = oc_thread_create(launcher_runloop, 0);

    while(!oc_should_quit())
    {
        oc_pump_events(0);
    }

    i64 exitCode = 0;
    oc_thread_join(runloopThread, &exitCode);

    oc_terminate();
    return exitCode;
}
