/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include <stdio.h>

#include "curl/curl.h"
#include "flag.h"
#include "util.h"
#include "tarball.h"
#include "orca.h"

#if OC_PLATFORM_WINDOWS
    #define RELEASE_FILENAME OC_STR8("orca-windows")
    #define TOOL_NAME OC_STR8("orca.exe")
#elif OC_PLATFORM_MACOS
    #if OC_ARCH_ARM64
        #define RELEASE_FILENAME OC_STR8("orca-mac-arm64")
        #define TOOL_NAME OC_STR8("orca")
    #else
        #define RELEASE_FILENAME OC_STR8("orca-mac-x64")
        #define TOOL_NAME OC_STR8("orca")
    #endif
#else
    #error Unsupported platform
#endif

static char curl_errbuf[CURL_ERROR_SIZE]; // buffer for last curl error message

static size_t curl_callback_write_to_file(char* data, size_t size, size_t nmemb, void* userdata);
static const char* curl_last_error(CURLcode code);
static CURLcode download_file(CURL* handle, oc_str8 url, oc_str8 out_path);
static bool overwrite_current_version(oc_str8 new_version);
static int replace_yourself_and_update(CURL* curl, oc_str8 repo_url_base, oc_str8 orca_dir, oc_str8 old_version, oc_str8 new_version);

int update(int argc, char** argv)
{
    oc_arena arena;
    oc_arena_init(&arena);

    Flag_Context c;
    flag_init_context(&c);
    flag_help(&c, "Downloads and installs the latest version of Orca.");
    char** tarball = flag_str(&c, NULL, "tarball", NULL, "update the Orca SDK from a tarball");

    if(!flag_parse(&c, argc, argv))
    {
        flag_print_usage(&c, "orca update", stderr);
        if(flag_error_is_help(&c))
        {
            return 0;
        }
        flag_print_error(&c, stderr);
        return 1;
    }

    CURL* curl = curl_easy_init();
    if(!curl)
    {
        fprintf(stderr, "error: failed to initialize curl\n");
        return 1;
    }

    oc_str8 orca_dir = system_orca_dir(&arena);
    oc_str8 version = { 0 };
    oc_str8 version_dir = { 0 };

    oc_str8 temp_dir = oc_path_append(&arena, orca_dir, OC_STR8("temporary"));
    if(oc_sys_exists(temp_dir))
    {
        TRY(oc_sys_rmdir(temp_dir));
    }
    TRY(oc_sys_mkdirs(temp_dir));

    if(!*tarball)
    {
        oc_str8 repo_url_base = OC_STR8("https://github.com/orca-app/orca");

        //-----------------------------------------------------------------------------
        // get the latest version number from github release url
        //-----------------------------------------------------------------------------
        oc_str8 latest_url = oc_path_append(&arena, repo_url_base, OC_STR8("/releases/latest"));
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_errbuf);
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1); // follow redirects
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
        curl_easy_setopt(curl, CURLOPT_URL, latest_url.ptr);
        CURLcode curl_code = curl_easy_perform(curl);
        if(curl_code != CURLE_OK)
        {
            fprintf(stderr, "error: failed to fetch latest version: %s\n", curl_easy_strerror(curl_code));
            return 1;
        }

        char* final_url_cstr = NULL;
        curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &final_url_cstr);
        oc_str8 final_url = oc_str8_push_cstring(&arena, final_url_cstr);

        version = oc_path_slice_filename(final_url);
        version_dir = oc_path_append(&arena, orca_dir, version);

        if(oc_sys_exists(version_dir))
        {
            printf("Already up to date with version %.*s\n", oc_str8_ip(version));
            goto end;
        }

        /*
        //-----------------------------------------------------------------------------
        // update cli tool executable
        //-----------------------------------------------------------------------------
        oc_str8 current_tool_version = OC_STR8(TOSTRING(ORCA_TOOL_VERSION));
        if(oc_str8_cmp(current_tool_version, version) != 0)
        {
            return replace_yourself_and_update(curl, repo_url_base, orca_dir, current_tool_version, version);
        }
        */

        //-----------------------------------------------------------------------------
        // download and extract latest version
        //-----------------------------------------------------------------------------
        {
            printf("Downloading Orca SDK version %.*s...\n", oc_str8_ip(version));

            oc_str8 release_tarname = oc_str8_pushf(&arena, "%.*s.tar.gz",
                                                    oc_str8_ip(RELEASE_FILENAME));

            oc_str8 release_url = oc_str8_pushf(&arena, "/releases/latest/download/%.*s",
                                                oc_str8_ip(release_tarname));

            release_url = oc_path_append(&arena, repo_url_base, release_url);
            oc_str8 release_filepath = oc_path_append(&arena, temp_dir, release_tarname);

            curl_code = download_file(curl, release_url, release_filepath);
            if(curl_code != CURLE_OK)
            {
                fprintf(stderr, "error: failed to download file %s: %s\n",
                        release_url.ptr, curl_last_error(curl_code));
                return 1;
            }

            printf("Extracting Orca SDK...\n");
            if(!tarball_extract(release_filepath, temp_dir))
            {
                fprintf(stderr, "error: failed to extract files from %s\n", release_filepath.ptr);
                return 1;
            }
        }
    }
    else
    {
        oc_str8 tarballPath = OC_STR8(*tarball);

        printf("Extracting Orca SDK...\n");
        if(!tarball_extract(tarballPath, temp_dir))
        {
            fprintf(stderr, "error: failed to extract files from %s\n", *tarball);
            return 1;
        }

        oc_str8 extracted_release = oc_path_append(&arena, temp_dir, OC_STR8("orca"));
        oc_str8 versionPath = oc_path_append(&arena, extracted_release, OC_STR8("current_version"));
        oc_file versionFile = oc_file_open(versionPath, OC_FILE_ACCESS_READ, OC_FILE_OPEN_NONE);
        if(oc_file_is_nil(versionFile))
        {
            fprintf(stderr, "error: failed to read version file %s\n", versionPath.ptr);
            return 1;
        }

        version.len = oc_file_size(versionFile),
        version.ptr = oc_arena_push(&arena, version.len + 1);
        oc_file_read(versionFile, version.len, version.ptr);
        version.ptr[version.len] = '\0';

        version_dir = oc_path_append(&arena, orca_dir, version);
        if(oc_sys_exists(version_dir))
        {
            printf("Already up to date with version %.*s\n", oc_str8_ip(version));
            return 0;
        }
    }

    //-----------------------------------------------------------------------------
    // move files to the install dir
    //-----------------------------------------------------------------------------
    {
        oc_str8 extracted_release = oc_path_append(&arena, temp_dir, OC_STR8("orca"));
        oc_str8 release_sdk = oc_path_append(&arena, extracted_release, version);

        if(!oc_sys_move(release_sdk, version_dir))
        {
            fprintf(stderr, "error: failed to move %s to %s\n",
                    release_sdk.ptr, version_dir.ptr);
            return 1;
        }

        oc_str8 release_tool = oc_path_append(&arena, extracted_release, TOOL_NAME);
        if(!oc_sys_move(release_tool, version_dir))
        {
            fprintf(stderr, "error: failed to move %s to %s\n",
                    release_tool.ptr, version_dir.ptr);
            return 1;
        }
    }
    //-----------------------------------------------------------------------------
    // record checksum and update current_version file
    //-----------------------------------------------------------------------------
    {
        /*
        oc_str8 checksum = { 0 };
        oc_str8 checksum_path = oc_path_append(&arena, temp_dir, OC_STR8("sha1.sum"));
        oc_file checksum_file = oc_file_open(checksum_path, OC_FILE_ACCESS_READ, OC_FILE_OPEN_NONE);
        if(!oc_file_is_nil(checksum_file))
        {
            checksum.len = oc_file_size(checksum_file);
            checksum.ptr = oc_arena_push(&arena, checksum.len + 1);
            oc_file_read(checksum_file, checksum.len, checksum.ptr);
            if(oc_file_last_error(checksum_file))
            {
                fprintf(stderr, "error: failed to read checksum file %s\n", checksum_path.ptr);
            }
        }
        oc_file_close(checksum_file);

        if(checksum.len)
        */
        {
            oc_str8 all_versions = oc_path_append(&arena, orca_dir, OC_STR8("all_versions"));
            oc_file_open_flags open_flags = oc_sys_exists(all_versions)
                                              ? OC_FILE_OPEN_APPEND
                                              : OC_FILE_OPEN_CREATE;
            oc_file file = oc_file_open(all_versions, OC_FILE_ACCESS_WRITE, open_flags);
            if(!oc_file_is_nil(file))
            {
                oc_file_seek(file, 0, OC_FILE_SEEK_END);
                /*
                oc_str8 version_and_checksum = oc_str8_pushf(&arena, "%.*s %.*s\n",
                                                             oc_str8_ip(version), oc_str8_ip(checksum));
                oc_file_write(file, version_and_checksum.len, version_and_checksum.ptr);
                */
                oc_file_write(file, oc_str8_ip(version));
            }
            else
            {
                fprintf(stderr, "error: failed to open file %s\n", all_versions.ptr);
            }
            oc_file_close(file);
        }
    }

    if(!overwrite_current_version(version))
    {
        fprintf(stderr, "error: failed to update current version file\n");
    }

end:
    TRY(oc_sys_rmdir(temp_dir));

    // NOTE(shaw): assuming that the cli tool will always just call update and
    // exit so no cleanup is done, i.e. curl_easy_cleanup(curl)

    printf("Successfully updated Orca SDK to version %.*s.\n", oc_str8_ip(version));
    return 0;
}

static size_t curl_callback_write_to_file(char* data, size_t size, size_t nmemb, void* userdata)
{
    oc_file* file = (oc_file*)userdata;
    return oc_file_write(*file, size * nmemb, data);
}

static const char* curl_last_error(CURLcode code)
{
    // if there is no message in curl_errbuf, then fall back to the less
    // detailed error message from the CURLcode
    u64 len = strlen(curl_errbuf);
    return len ? curl_errbuf : curl_easy_strerror(code);
}

static CURLcode download_file(CURL* handle, oc_str8 url, oc_str8 out_path)
{
    oc_file file = oc_file_open(out_path, OC_FILE_ACCESS_WRITE, OC_FILE_OPEN_CREATE | OC_FILE_OPEN_TRUNCATE);
    if(oc_file_is_nil(file))
    {
        oc_file_close(file);
        return CURLE_WRITE_ERROR;
    }

    curl_easy_reset(handle);
    curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, curl_errbuf);
    curl_easy_setopt(handle, CURLOPT_FAILONERROR, 1);
    curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, curl_callback_write_to_file);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &file);
    curl_easy_setopt(handle, CURLOPT_URL, url.ptr);
    CURLcode err = curl_easy_perform(handle);
    oc_file_close(file);
    return err;
}

static bool overwrite_current_version(oc_str8 new_version)
{
    bool result = true;
    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8 orca_dir = system_orca_dir(scratch.arena);
    oc_str8 current_version_path = oc_path_append(scratch.arena, orca_dir, OC_STR8("current_version"));
    oc_file file = oc_file_open(current_version_path, OC_FILE_ACCESS_WRITE,
                                OC_FILE_OPEN_CREATE | OC_FILE_OPEN_TRUNCATE);
    if(oc_file_is_nil(file))
    {
        result = false;
        goto cleanup;
    }

    oc_file_write(file, new_version.len, new_version.ptr);
    if(oc_file_last_error(file) != OC_IO_OK)
    {
        result = false;
        goto cleanup;
    }

cleanup:
    oc_file_close(file);
    oc_scratch_end(scratch);
    return result;
}

/*
#if OC_PLATFORM_WINDOWS
static int replace_yourself_and_update(CURL* curl, oc_str8 repo_url_base, oc_str8 orca_dir,
                                       oc_str8 old_version, oc_str8 new_version)
{
    int result = 0;
    oc_arena_scope scratch = oc_scratch_begin();

    // download latest orca cli tool
    printf("Downloading cli tool version %.*s...\n", oc_str8_ip(new_version));
    oc_str8 tool_url = oc_path_append(scratch.arena, repo_url_base,
                                      OC_STR8("/releases/latest/download/orca.exe"));
    oc_str8 new_tool_path = oc_path_append(scratch.arena, orca_dir, OC_STR8("latest_orca.exe"));
    CURLcode curl_code = download_file(curl, tool_url, new_tool_path);
    if(curl_code != CURLE_OK)
    {
        fprintf(stderr, "error: failed to download file %s: %s\n",
                tool_url.ptr, curl_last_error(curl_code));
        result = 1;
        goto cleanup;
    }

    // execute orca update with newer cli tool
    oc_str8 cmd = oc_str8_pushf(scratch.arena, "\"%s\" update", new_tool_path.ptr);
    result = system(cmd.ptr);
    if(result)
    {
        goto cleanup;
    }

    // replace old tool with new
    // NOTE(shaw): the currently executing cli tool can only be renamed, not deleted
    // so it is renamed to old_orca.exe, and subsequent updates will delete it
    oc_str8 old_rename_path = oc_path_append(scratch.arena, orca_dir, OC_STR8("old_orca.exe"));
    remove(old_rename_path.ptr);
    oc_str8 old_tool_path = oc_path_append(scratch.arena, orca_dir, OC_STR8("orca.exe"));
    rename(old_tool_path.ptr, old_rename_path.ptr);
    result = rename(new_tool_path.ptr, old_tool_path.ptr);
    if(result)
    {
        fprintf(stderr, "error: failed to replace Orca cli tool with latest version\n");
        goto cleanup;
    }

    printf("Successfully updated cli tool to version %.*s.\n", oc_str8_ip(new_version));

cleanup:
    oc_scratch_end(scratch);
    return result;
}

#elif OC_PLATFORM_MACOS

static int replace_yourself_and_update(CURL* curl, oc_str8 repo_url_base, oc_str8 orca_dir,
                                       oc_str8 old_version, oc_str8 new_version)
{
    int result = 0;
    oc_arena_scope scratch = oc_scratch_begin();

    oc_str8 temp_dir = oc_path_append(scratch.arena, orca_dir, OC_STR8("temporary"));
    if(!oc_sys_exists(temp_dir))
    {
        TRY(oc_sys_mkdirs(temp_dir));
    }

    // download latest orca cli tool
    printf("Downloading cli tool version %.*s...\n", oc_str8_ip(new_version));
    oc_str8 tool_url = oc_path_append(scratch.arena, repo_url_base,
                                      OC_STR8("/releases/latest/download/orca-mac.tar.gz"));
    oc_str8 tarball_path = oc_path_append(scratch.arena, temp_dir, OC_STR8("tool.tar.gz"));
    CURLcode curl_code = download_file(curl, tool_url, tarball_path);
    if(curl_code != CURLE_OK)
    {
        fprintf(stderr, "error: failed to download file %s: %s\n",
                tool_url.ptr, curl_last_error(curl_code));
        result = 1;
        goto cleanup;
    }

    printf("Extracting cli tool ...\n");
    if(!tarball_extract(tarball_path, temp_dir))
    {
        fprintf(stderr, "error: failed to extract orca cli tool from tarball\n");
    }

    oc_str8 temp_dir_orca = oc_path_append(scratch.arena, temp_dir, OC_STR8("orca"));
    oc_str8 new_tool_path = oc_path_append(scratch.arena, orca_dir, OC_STR8("latest_orca"));
    TRY(oc_sys_move(temp_dir_orca, new_tool_path));

    // execute orca update with newer cli tool
    oc_str8 cmd = oc_str8_pushf(scratch.arena, "\"%s\" update", new_tool_path.ptr);
    result = system(cmd.ptr);
    if(result)
    {
        goto cleanup;
    }

    // replace old tool with new
    // NOTE(shaw): the currently executing cli tool can only be renamed, not deleted
    // so it is renamed to old_orca, and subsequent updates will delete it
    oc_str8 old_rename_path = oc_path_append(scratch.arena, orca_dir, OC_STR8("old_orca"));
    remove(old_rename_path.ptr);
    oc_str8 old_tool_path = oc_path_append(scratch.arena, orca_dir, OC_STR8("orca"));
    rename(old_tool_path.ptr, old_rename_path.ptr);
    result = rename(new_tool_path.ptr, old_tool_path.ptr);
    if(result)
    {
        fprintf(stderr, "error: failed to replace Orca cli tool with latest version\n");
        goto cleanup;
    }

    printf("Successfully updated cli tool to version %.*s.\n", oc_str8_ip(new_version));

cleanup:
    if(oc_sys_exists(temp_dir))
    {
        oc_sys_rmdir(temp_dir);
    }
    oc_scratch_end(scratch);
    return result;
}
#else
    #error replace_yourself_and_update() not implemented on this platform
#endif
*/
