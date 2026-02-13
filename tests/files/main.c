/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#define OC_NO_APP_LAYER
#include "orca.c"
#include "util/tests.c"

oc_str8 TEST_DIR;

void test_write(oc_test_info* info, oc_arena* arena)
{
    oc_test(info, "write")
    {
        oc_str8 path = oc_path_append(arena, TEST_DIR, OC_STR8("data/write_test.txt"));
        oc_str8 test_string = OC_STR8("Hello from write_test.txt");

        oc_file f = oc_catch(oc_file_open(path,
                                          OC_FILE_ACCESS_WRITE,
                                          &(oc_file_open_options){
                                              .flags = OC_FILE_OPEN_CREATE | OC_FILE_OPEN_TRUNCATE,
                                          }))
        {
            oc_test_fail(info, "Can't create/open file %.*s for writing", (int)path.len, path.ptr);
        }
        else
        {
            oc_file_write(f, test_string.len, test_string.ptr);
            if(oc_file_last_error(f))
            {

                oc_test_fail(info, "Error while writing %.*s", (int)path.len, path.ptr);
            }
            else
            {
                char* pathCStr = oc_str8_to_cstring(arena, path);
                FILE* file = fopen(pathCStr, "r");
                if(!file)
                {
                    oc_test_fail(info, "File %.*s not found while checking", (int)path.len, path.ptr);
                }
                else
                {
                    char buffer[256];
                    int n = fread(buffer, 1, 256, file);
                    if(n != test_string.len || strncmp(buffer, test_string.ptr, test_string.len))
                    {
                        oc_test_fail(info, "Didn't recover test string");
                    }
                    fclose(file);
                }
            }
            oc_file_close(f);
            remove(path.ptr);
        }
    }
}

int check_string(oc_test_info* info, oc_file f, oc_str8 test_string)
{
    char buffer[256];
    i64 n = oc_file_read(f, 256, buffer);
    if(oc_file_last_error(f))
    {
        oc_test_fail(info, "Error while reading test string");
        return -1;
    }
    else if(oc_str8_cmp(test_string, oc_str8_from_buffer(n, buffer)))
    {
        oc_test_fail(info, "read string doesn't match");
        return (-1);
    }
    else
    {
        return 0;
    }
}

void test_read(oc_test_info* info, oc_arena* arena)
{
    oc_test(info, "read")
    {
        oc_str8 path = oc_path_append(arena, TEST_DIR, OC_STR8("data/regular.txt"));
        oc_str8 test_string = OC_STR8("Hello from regular.txt");

        oc_file f = oc_catch(oc_file_open(path, OC_FILE_ACCESS_READ, 0))
        {
            oc_test_fail(info, "Can't open file %.*s for reading", oc_str8_ip(path));
        }
        else if(check_string(info, f, test_string))
        {
            oc_test_fail(info, "Check string failed");
        }
        oc_file_close(f);
    }
}

void test_stat_size(oc_test_info* info, oc_arena* arena)
{
    oc_test(info, "size")
    {
        oc_str8 path = oc_path_append(arena, TEST_DIR, OC_STR8("data/regular.txt"));
        oc_str8 test_string = OC_STR8("Hello from regular.txt");
        u64 size = test_string.len;

        oc_file f = oc_catch(oc_file_open(path, 0, 0))
        {
            oc_test_fail(info, "Can't open file");
        }
        else
        {
            oc_file_status status = oc_file_get_status(f);

            if(oc_file_last_error(f))
            {
                oc_test_fail(info, "Error while retrieving file status");
            }
            else if(status.size != size)
            {
                oc_test_fail(info, "size doesn't match");
            }
            oc_file_close(f);
        }
    }
}

void test_stat_type(oc_test_info* info, oc_arena* arena)
{
    oc_str8 regular = oc_path_append(arena, TEST_DIR, OC_STR8("data/regular.txt"));
    oc_str8 dir = oc_path_append(arena, TEST_DIR, OC_STR8("data/directory"));
    oc_str8 link = oc_path_append(arena, TEST_DIR, OC_STR8("data/symlink"));

    oc_test(info, "stat regular")
    {
        oc_file f = oc_catch(oc_file_open(regular, 0, 0))
        {
            oc_test_fail(info, "Can't open file");
        }
        else
        {
            oc_file_status status = oc_file_get_status(f);
            if(oc_file_last_error(f))
            {
                oc_test_fail(info, "Error while retrieving file status");
            }
            else if(status.type != OC_FILE_REGULAR)
            {
                oc_test_fail(info, "file type doesn't match");
            }
            oc_file_close(f);
        }
    }

    oc_test(info, "stat directory")
    {
        oc_file f = oc_catch(oc_file_open(dir, 0, 0))
        {
            oc_test_fail(info, "Can't open file");
        }
        else
        {
            oc_file_status status = oc_file_get_status(f);
            if(oc_file_last_error(f))
            {
                oc_test_fail(info, "Error while retrieving file status");
            }
            else if(status.type != OC_FILE_DIRECTORY)
            {
                oc_test_fail(info, "file type doesn't match");
            }
            oc_file_close(f);
        }
    }

#if !OC_PLATFORM_WINDOWS
    oc_test(info, "stat symlink")
    {
        oc_file f = oc_catch(oc_file_open(link,
                                          OC_FILE_ACCESS_NONE,
                                          &(oc_file_open_options){
                                              .resolve = OC_FILE_RESOLVE_SYMLINK_OPEN_LAST,
                                          }))
        {
            oc_test_fail(info, "Can't open file");
        }
        else
        {
            oc_file_status status = oc_file_get_status(f);
            if(oc_file_last_error(f))
            {
                oc_test_fail(info, "Error while retrieving file status");
            }

            if(status.type != OC_FILE_SYMLINK)
            {
                oc_test_fail(info, "file type doesn't match");
            }
            oc_file_close(f);
        }
    }
#endif
}

void test_symlinks(oc_test_info* info, oc_arena* arena)
{
#if !OC_PLATFORM_WINDOWS
    oc_str8 path = oc_path_append(arena, TEST_DIR, OC_STR8("data/symlink"));

    oc_test(info, "open symlink target")
    {
        oc_file f = oc_catch(oc_file_open(path, OC_FILE_ACCESS_READ, 0))
        {
            oc_test_fail(info, "failed to open %s", path.ptr);
        }
        else
        {
            if(check_string(info, f, OC_STR8("Hello from regular.txt")))
            {
                oc_test_fail(info, "Check string failed");
            }
            oc_file_close(f);
        }
    }

    oc_test(info, "open symlink file")
    {
        oc_file f = oc_catch(oc_file_open(path,
                                          OC_FILE_ACCESS_READ,
                                          &(oc_file_open_options){
                                              .resolve = OC_FILE_RESOLVE_SYMLINK_OPEN_LAST,
                                          }))
        {
            oc_test_fail(info, "failed to open %s", path.ptr);
        }
        else
        {
            oc_file_status status = oc_file_get_status(f);
            if(oc_file_last_error(f))
            {
                oc_test_fail(info, "Error while retrieving file status");
            }
            else if(status.type != OC_FILE_SYMLINK)
            {
                oc_test_fail(info, "file type doesn't match");
            }
            else
            {
                char buffer[512];
                int n = oc_file_read(f, 512, buffer);

                if(n || (oc_file_last_error(f) == OC_IO_OK))
                {
                    oc_test_fail(info, "file read should fail on symlinks");
                }
            }
            oc_file_close(f);
        }
    }
#endif
}

void test_args(oc_test_info* info, oc_arena* arena)
{
    oc_str8 path = oc_path_append(arena, TEST_DIR, OC_STR8("data/regular.txt"));

    //NOTE: nil handle
    oc_test(info, "open_at with nil handle")
    {
        oc_file f = oc_catch(oc_file_open(path, OC_FILE_ACCESS_READ, 0))
        {
            oc_test_fail(info, "oc_file_open() with nil handle failed");
        }
        else
        {
            if(check_string(info, f, OC_STR8("Hello from regular.txt")))
            {
                oc_test_fail(info, "Check string failed");
            }
            oc_file_close(f);
        }
    }

    oc_test(info, "open_at with invalid handle")
    {
        oc_file wrongHandle = { .h = 123456789 };

        oc_file_result openRes = oc_file_open(path,
                                              OC_FILE_ACCESS_READ,
                                              &(oc_file_open_options){
                                                  .root = wrongHandle,
                                              });
        if(oc_result_check(openRes) || openRes.error != OC_IO_ERR_HANDLE)
        {
            oc_test_fail(info, "oc_file_open() with non-nil invalid handle should return OC_IO_ERR_HANDLE");
        }
        if(oc_result_check(openRes))
        {
            oc_file_close(openRes.value);
        }
    }

    oc_test(info, "open empty path")
    {
        oc_file_result openRes = oc_file_open(OC_STR8(""), OC_FILE_ACCESS_READ, 0);
        if(oc_result_check(openRes) || openRes.error != OC_IO_ERR_ARG)
        {
            oc_test_fail(info, "empty path should return OC_IO_ERR_ARG");
        }
        if(oc_result_check(openRes))
        {
            oc_file_close(openRes.value);
        }
    }
}

void test_jail(oc_test_info* info, oc_arena* arena)
{
    oc_str8 jailPath = oc_path_append(arena, TEST_DIR, OC_STR8("data/jail"));
    oc_file jail = oc_file_nil();
    oc_test(info, "open fail directory")
    {
        jail = oc_catch(oc_file_open(jailPath, OC_FILE_ACCESS_READ, 0))
        {
            oc_test_fail(info, "Can't open jail directory");
        }
    }
    //-----------------------------------------------------------
    //NOTE: Check escapes
    //-----------------------------------------------------------
    oc_test(info, "escape with absolute path")
    {
        //NOTE: escape with absolute path
        oc_file_result openRes = oc_file_open(OC_STR8("/tmp"),
                                              OC_FILE_ACCESS_READ,
                                              &(oc_file_open_options){
                                                  .root = jail,
                                              });

        if(oc_result_check(openRes) || openRes.error != OC_IO_ERR_NO_ENTRY)
        {
            oc_test_fail(info, "Escaped jail with absolute path /tmp");
        }
        if(oc_result_check(openRes))
        {
            oc_file_close(openRes.value);
        }
    }

    oc_test(info, "escape with ..")
    {
        //NOTE: escape with ..
        oc_file_result openRes = oc_file_open(OC_STR8(".."),
                                              OC_FILE_ACCESS_READ,
                                              &(oc_file_open_options){
                                                  .root = jail,
                                              });

        if(oc_result_check(openRes) || openRes.error != OC_IO_ERR_WALKOUT)
        {
            oc_test_fail(info, "Escaped jail with relative path ..");
        }
        if(oc_result_check(openRes))
        {
            oc_file_close(openRes.value);
        }
    }

    oc_test(info, "escape with dir/../..")
    {

        oc_file_result openRes = oc_file_open(OC_STR8("dir/../.."),
                                              OC_FILE_ACCESS_READ,
                                              &(oc_file_open_options){
                                                  .root = jail,
                                              });
        if(oc_result_check(openRes) || openRes.error != OC_IO_ERR_WALKOUT)
        {
            oc_test_fail(info, "Escaped jail with relative path dir/../..");
        }
        if(oc_result_check(openRes))
        {
            oc_file_close(openRes.value);
        }
    }

#if !OC_PLATFORM_WINDOWS
    oc_test(info, "escape with symlink to parent")
    {
        //NOTE: escape with symlink to parent
        oc_file_result openRes = oc_file_open(OC_STR8("/dir_escape"),
                                              OC_FILE_ACCESS_READ,
                                              &(oc_file_open_options){
                                                  .root = jail,
                                              });
        if(oc_result_check(openRes) || openRes.error != OC_IO_ERR_WALKOUT)
        {
            oc_test_fail(info, "Escaped jail with symlink to parent");
        }
        if(oc_result_check(openRes))
        {
            oc_file_close(openRes.value);
        }
    }

    oc_test(info, "escape to file with symlink to parent")
    {
        oc_file_result openRes = oc_file_open(OC_STR8("/dir_escape/regular.txt"),
                                              OC_FILE_ACCESS_READ,
                                              &(oc_file_open_options){
                                                  .root = jail,
                                              });
        if(oc_result_check(openRes) || openRes.error != OC_IO_ERR_WALKOUT)
        {
            oc_test_fail(info, "Escaped jail to regular.txt with symlink to parent");
        }
        if(oc_result_check(openRes))
        {
            oc_file_close(openRes.value);
        }
    }

    oc_test(info, "escape with symlink to file")
    {
        oc_file_result openRes = oc_file_open(OC_STR8("/file_escape"),
                                              OC_FILE_ACCESS_READ,
                                              &(oc_file_open_options){
                                                  .root = jail,
                                              });
        if(oc_result_check(openRes) || openRes.error != OC_IO_ERR_WALKOUT)
        {
            oc_test_fail(info, "Escaped jail with symlink to file regular.txt");
        }
        if(oc_result_check(openRes))
        {
            oc_file_close(openRes.value);
        }
    }
#endif
    /*
    //NOTE: escape with bad root handle
    oc_file wrong_handle = { 0 };
    oc_str8 regularPath = oc_path_append(arena, TEST_DIR, OC_STR8("data/regular.txt"));
    f = oc_file_open(wrong_handle, regularPath, OC_FILE_ACCESS_READ, OC_FILE_OPEN_RESTRICT);
    if(oc_file_last_error(f) == OC_IO_OK)
    {
        oc_test_fail(info, "Escaped jail with nil root handle");
    }
    if(oc_file_last_error(f) != OC_IO_ERR_HANDLE)
    {
        oc_test_fail(info, "OC_FILE_OPEN_RESTRICT with invalid root handle should return OC_IO_ERR_HANDLE");
    }
    oc_file_close(f);
*/
    //-----------------------------------------------------------
    //NOTE: empty path
    //-----------------------------------------------------------
    oc_test(info, "empty path")
    {
        oc_file_result openRes = oc_file_open(OC_STR8(""),
                                              OC_FILE_ACCESS_READ,
                                              &(oc_file_open_options){
                                                  .root = jail,
                                              });
        if(oc_result_check(openRes) || openRes.error != OC_IO_ERR_ARG)
        {
            oc_test_fail(info, "empty path should return OC_IO_ERR_ARG");
        }
        if(oc_result_check(openRes))
        {
            oc_file_close(openRes.value);
        }
    }

    //-----------------------------------------------------------
    //NOTE: Check legitimates open
    //-----------------------------------------------------------

    oc_test(info, "valid /test.txt")
    {
        oc_file_result openRes = oc_file_open(OC_STR8("/test.txt"),
                                              OC_FILE_ACCESS_READ,
                                              &(oc_file_open_options){
                                                  .root = jail,
                                              });
        oc_file f = oc_catch(openRes)
        {
            oc_test_fail(info, "Can't open jail/test.txt");
        }
        else
        {
            if(check_string(info, f, OC_STR8("Hello from jail/test.txt")))
            {
                oc_test_fail(info, "Check string failed");
            }
            oc_file_close(f);
        }
    }
    oc_test(info, "valid /dir/../test.txt")
    {
        oc_file_result openRes = oc_file_open(OC_STR8("/dir/../test.txt"),
                                              OC_FILE_ACCESS_READ,
                                              &(oc_file_open_options){
                                                  .root = jail,
                                              });
        oc_file f = oc_catch(openRes)
        {
            oc_test_fail(info, "Can't open jail/dir/../test.txt");
        }
        else
        {
            if(check_string(info, f, OC_STR8("Hello from jail/test.txt")))
            {
                oc_test_fail(info, "Check string failed");
            }
            oc_file_close(f);
        }
    }

    oc_test(info, "valid reopen .")
    {
        //NOTE: re-open root directory
        oc_file_result openRes = oc_file_open(OC_STR8("."),
                                              OC_FILE_ACCESS_READ,
                                              &(oc_file_open_options){
                                                  .root = jail,
                                              });
        oc_file f = oc_catch(openRes)
        {
            oc_test_fail(info, "Can't open jail/.");
        }
        else
        {
            //NOTE: access regular file test.txt inside reopened root
            oc_file f2 = oc_catch(oc_file_open(OC_STR8("test.txt"),
                                               OC_FILE_ACCESS_READ,
                                               &(oc_file_open_options){
                                                   .root = f,
                                               }))
            {
                oc_test_fail(info, "Couldn't open test.txt");
            }
            else
            {
                if(check_string(info, f2, OC_STR8("Hello from jail/test.txt")))
                {
                    oc_test_fail(info, "Check string failed");
                }
                oc_file_close(f2);
            }
            oc_file_close(f);
        }
    }
    oc_file_close(jail);
}

void test_rights(oc_test_info* info, oc_arena* arena)
{
    oc_str8 dirPath = oc_path_append(arena, TEST_DIR, OC_STR8("data"));

    //--------------------------------------------------------------------------------------
    // base dir with no access
    //--------------------------------------------------------------------------------------
    oc_test(info, "no access")
    {
        oc_file dir = oc_catch(oc_file_open(dirPath, OC_FILE_ACCESS_NONE, 0))
        {
            oc_test_fail(info, "Couldn't open data with no access rights");
        }
        else
        {
            oc_file_result openRes = oc_file_open(OC_STR8("./regular.txt"),
                                                  OC_FILE_ACCESS_READ,
                                                  &(oc_file_open_options){
                                                      .root = dir,
                                                  });
            if(oc_result_check(openRes) || openRes.error != OC_IO_ERR_PERM)
            {
                oc_test_fail(info, "Incorrect check when opening file with read access in dir with no access");
            }
            oc_file_close(dir);
        }
    }

    //--------------------------------------------------------------------------------------
    // base dir with read access
    //--------------------------------------------------------------------------------------

    oc_test(info, "read access")
    {
        oc_file dir = oc_catch(oc_file_open(dirPath, OC_FILE_ACCESS_READ, 0))
        {
            oc_test_fail(info, "Couldn't open ./data with read rights");
        }

        if(!oc_file_is_nil(dir))
        {
            // check that we _can't_ open a file with write access
            oc_file_result openRes = oc_file_open(OC_STR8("./regular.txt"),
                                                  OC_FILE_ACCESS_WRITE,
                                                  &(oc_file_open_options){
                                                      .root = dir,
                                                  });
            if(oc_result_check(openRes) || openRes.error != OC_IO_ERR_PERM)
            {
                oc_test_fail(info, "Incorrect check when opening file with write access in dir with read access");
            }

            // check that we _can_ open a file with read access
            oc_file f = oc_catch(oc_file_open(OC_STR8("./regular.txt"),
                                              OC_FILE_ACCESS_READ,
                                              &(oc_file_open_options){
                                                  .root = dir,
                                              }))
            {
                oc_test_fail(info, "Couldn't open file with read access in dir with read access");
            }
            else
            {

                // check that we _can't_ write to that file
                oc_str8 testWrite = OC_STR8("Hello, world!");
                if(oc_file_write(f, testWrite.len, testWrite.ptr) != 0)
                {
                    oc_test_fail(info, "Incorrectly wrote to read-only file");
                }
                if(oc_file_last_error(f) != OC_IO_ERR_PERM)
                {
                    oc_test_fail(info, "Incorrect error returned from writing to read-only file");
                }

                oc_file_close(f);
            }
            oc_file_close(dir);
        }
    }

    //--------------------------------------------------------------------------------------
    // base dir with write access
    //--------------------------------------------------------------------------------------
    oc_test(info, "write access")
    {
        oc_file dir = oc_catch(oc_file_open(dirPath, OC_FILE_ACCESS_WRITE, 0))
        {
            oc_test_fail(info, "Couldn't open %s with write rights", dirPath.ptr);
        }

        if(!oc_file_is_nil(dir))
        {
            // check that we _can't_ open a file with read access
            oc_file_result openRes = oc_file_open(OC_STR8("./regular.txt"),
                                                  OC_FILE_ACCESS_READ,
                                                  &(oc_file_open_options){
                                                      .root = dir,
                                                  });
            if(oc_result_check(openRes) || openRes.error != OC_IO_ERR_PERM)
            {
                oc_test_fail(info, "Incorrect check when opening file with read access in dir with write access");
            }

            // check that we _can_ open a file with write access
            oc_file f = oc_catch(oc_file_open(OC_STR8("./regular.txt"),
                                              OC_FILE_ACCESS_WRITE,
                                              &(oc_file_open_options){
                                                  .root = dir,
                                              }))
            {
                oc_test_fail(info, "Couldn't open file with write access in dir with write access");
            }

            if(!oc_file_is_nil(f))
            {
                // check that we _can't_ read that file
                char testRead[512];
                if(oc_file_read(f, 512, testRead) != 0)
                {
                    oc_test_fail(info, "Incorrectly read write-only file");
                }
                if(oc_file_last_error(f) != OC_IO_ERR_PERM)
                {
                    oc_test_fail(info, "Incorrect error returned from reading write-only file");
                }
                oc_file_close(f);
            }
            oc_file_close(dir);
        }
    }
    //--------------------------------------------------------------------------------------
    // base dir with read/write access
    //--------------------------------------------------------------------------------------
    oc_test(info, "read/write access")
    {
        oc_file dir = oc_catch(oc_file_open(dirPath, OC_FILE_ACCESS_READ | OC_FILE_ACCESS_WRITE, 0))
        {
            oc_test_fail(info, "Couldn't open ./data with read rights");
        }

        if(!oc_file_is_nil(dir))
        {
            // check that we can open file with read access
            oc_file f = oc_catch(oc_file_open(OC_STR8("./regular.txt"),
                                              OC_FILE_ACCESS_READ,
                                              &(oc_file_open_options){
                                                  .root = dir,
                                              }))
            {
                oc_test_fail(info, "Incorrect check when opening file with read access in dir with read/write access");
            }
            oc_file_close(f);

            // check that we can open file with write access
            f = oc_catch(oc_file_open(OC_STR8("./regular.txt"),
                                      OC_FILE_ACCESS_WRITE,
                                      &(oc_file_open_options){
                                          .root = dir,
                                      }))
            {
                oc_test_fail(info, "Couldn't open file with write access in dir with read/write access");
            }
            oc_file_close(f);
            oc_file_close(dir);
        }
    }
}

void test_resolve(oc_test_info* info, oc_arena* arena)
{
    oc_test(info, "absolute path")
    {
#if OC_PLATFORM_MACOS
        oc_str8 path = OC_STR8("/usr/bin");
        oc_io_resolve_result r = oc_io_resolve(arena, oc_file_desc_nil(), path, 0);
        if(r.error || oc_str8_cmp(r.path, OC_STR8("/usr/bin")))
        {
            oc_test_fail(info, "Bad path resolution.");
        }
        oc_fd_close(r.fd);

#elif OC_PLATFORM_WINDOWS

        oc_str8 path = OC_STR8("C:\\Users");
        oc_io_resolve_result r = oc_io_resolve(arena, oc_file_desc_nil(), path, 0);
        if(r.error || oc_str8_cmp(r.path, OC_STR8("C:\\Users")))
        {
            oc_test_fail(info, "Bad path resolution.");
        }
        oc_fd_close(r.fd);
#endif
    }

    oc_test(info, "relative path")
    {
        oc_str8 path = OC_STR8("tests/files/data/regular.txt");

        oc_io_resolve_result r = oc_io_resolve(arena, oc_file_desc_nil(), path, 0);
        if(r.error || oc_str8_cmp(r.path, OC_STR8("tests/files/data/regular.txt")))
        {
            oc_test_fail(info, "Bad path resolution.");
        }
        oc_fd_close(r.fd);
    }

    oc_test(info, "relative path with ..")
    {
        oc_str8 path = OC_STR8("tests/files/data/directory/../regular.txt");
        oc_io_resolve_result r = oc_io_resolve(arena, oc_file_desc_nil(), path, 0);
        if(r.error || oc_str8_cmp(r.path, OC_STR8("tests/files/data/regular.txt")))
        {
            oc_test_fail(info, "Bad path resolution.");
        }
        oc_fd_close(r.fd);
    }

#ifndef OC_PLATFORM_WINDOWS
    oc_test(info, "relative path with symlink")
    {
        oc_str8 path = OC_STR8("tests/files/data/symlink");
        oc_io_resolve_result r = oc_io_resolve(arena, oc_file_desc_nil(), path, 0);
        if(r.error || oc_str8_cmp(r.path, OC_STR8("tests/files/data/regular.txt")))
        {
            oc_test_fail(info, "Bad path resolution.");
        }
        oc_fd_close(r.fd);
    }
#endif

    oc_test(info, "relative path wit non-existing end")
    {
        oc_str8 path = OC_STR8("tests/files/data/directory/foo/../bar");
        oc_io_resolve_result r = oc_io_resolve(arena, oc_file_desc_nil(), path, 0);
        if(r.error != OC_IO_ERR_NO_ENTRY)
        {
            oc_test_fail(info, "Bad path resolution.");
        }
        oc_fd_close(r.fd);
    }

    oc_test(info, "relative path with file inside the path")
    {
        oc_str8 path = OC_STR8("tests/files/data/regular.txt/foo/bar");
        oc_io_resolve_result r = oc_io_resolve(arena, oc_file_desc_nil(), path, 0);
        if(r.error != OC_IO_ERR_NOT_DIR)
        {
            oc_test_fail(info, "Bad path resolution.");
        }
        oc_fd_close(r.fd);
    }

    oc_file_slot* dirSlot = 0;
    oc_file dir = oc_file_nil();
    oc_test(info, "open root dir")
    {
        oc_str8 dirPath = oc_path_append(arena, TEST_DIR, OC_STR8("data"));
        dir = oc_catch(oc_file_open(dirPath, OC_FILE_ACCESS_READ, 0))
        {
            oc_test_fail(info, "Couldn't open data directory");
        }
        dirSlot = oc_file_slot_from_handle(&oc_globalFileTable, dir);
    }

    oc_test(info, "relative path inside root")
    {
        oc_io_resolve_result r = oc_io_resolve(arena, dirSlot->fd, OC_STR8("directory/test.txt"), 0);
        if(r.error || oc_str8_cmp(r.path, OC_STR8("directory/test.txt")))
        {
            oc_test_fail(info, "Bad path resolution.");
        }
        oc_fd_close(r.fd);
    }

    oc_test(info, "absolute path inside root")
    {
        oc_io_resolve_result r = oc_io_resolve(arena, dirSlot->fd, OC_STR8("/directory/test.txt"), 0);
        if(r.error || oc_str8_cmp(r.path, OC_STR8("directory/test.txt")))
        {
            oc_test_fail(info, "Bad path resolution.");
        }
        oc_fd_close(r.fd);
    }
#ifndef OC_PLATFORM_WINDOWS
    oc_test(info, "path with symlink inside root")
    {
        oc_io_resolve_result r = oc_io_resolve(arena, dirSlot->fd, OC_STR8("symlink"), 0);
        if(r.error || oc_str8_cmp(r.path, OC_STR8("regular.txt")))
        {
            oc_test_fail(info, "Bad path resolution.");
        }
        oc_fd_close(r.fd);
    }
#endif

    oc_test(info, "path with valid .. inside root")
    {
        oc_io_resolve_result r = oc_io_resolve(arena, dirSlot->fd, OC_STR8("directory/../regular.txt"), 0);
        if(r.error || oc_str8_cmp(r.path, OC_STR8("regular.txt")))
        {
            oc_test_fail(info, "Bad path resolution.");
        }
        oc_fd_close(r.fd);
    }

    oc_test(info, "path with non existing end inside root")
    {
        oc_io_resolve_result r = oc_io_resolve(arena, dirSlot->fd, OC_STR8("directory/foo/../test.txt"), 0);
        if(r.error != OC_IO_ERR_NO_ENTRY)
        {
            oc_test_fail(info, "Bad path resolution.");
        }
    }

    oc_test(info, "try escaping with ..")
    {
        oc_io_resolve_result r = oc_io_resolve(arena, dirSlot->fd, OC_STR8("directory/../../foo"), 0);
        if(r.error != OC_IO_ERR_WALKOUT)
        {
            oc_test_fail(info, "Bad path resolution.");
        }
    }

    oc_file jail = oc_file_nil();
    oc_file_slot* jailSlot = 0;
    oc_test(info, "open jail directory")
    {
        jail = oc_catch(oc_file_open(OC_STR8("jail"),
                                     OC_FILE_ACCESS_READ,
                                     &(oc_file_open_options){
                                         .root = dir,
                                     }))
        {
            oc_test_fail(info, "Couldn't open jail directory");
        }
        jailSlot = oc_file_slot_from_handle(&oc_globalFileTable, jail);
    }
#ifndef OC_PLATFORM_WINDOWS
    oc_test(info, "escape with symlink to file")
    {
        oc_io_resolve_result r = oc_io_resolve(arena, jailSlot->fd, OC_STR8("file_escape"), 0);
        if(r.error != OC_IO_ERR_WALKOUT)
        {
            oc_test_fail(info, "Bad path resolution.");
        }
    }

    oc_test(info, "escape with symlink to dir")
    {
        oc_io_resolve_result r = oc_io_resolve(arena, jailSlot->fd, OC_STR8("dir_escape"), 0);
        if(r.error != OC_IO_ERR_WALKOUT)
        {
            oc_test_fail(info, "Bad path resolution.");
        }
    }
#endif
}

void test_maketmp(oc_test_info* info, oc_arena* arena)
{
    oc_test(info, "tmp file")
    {
        oc_file tmpFile = oc_catch(oc_file_maketmp(OC_FILE_MAKETMP_FILE))
        {
            oc_test_fail(info, "Can't make tmp file.");
        }
        oc_file_close(tmpFile);
    }

    oc_test(info, "tmp directory")
    {
        oc_file tmpDir = oc_catch(oc_file_maketmp(OC_FILE_MAKETMP_DIRECTORY))
        {
            oc_test_fail(info, "Can't make tmp directory.");
        }
        oc_file_close(tmpDir);
    }
}

void test_makedir(oc_test_info* info, oc_arena* arena)
{
    oc_file tmpDir = oc_file_nil();
    oc_test(info, "make tmp dir")
    {
        tmpDir = oc_catch(oc_file_maketmp(OC_FILE_MAKETMP_DIRECTORY))
        {
            oc_test_fail(info, "Can't make tmp directory.");
        }
    }

    oc_test(info, "makedir")
    {
        oc_io_error error = oc_file_makedir(OC_STR8("test"),
                                            &(oc_file_makedir_options){
                                                .root = tmpDir,
                                            });
        if(error != OC_IO_OK)
        {
            oc_test_fail(info, "Can't create directory.");
        }
        oc_file dir = oc_catch(oc_file_open(OC_STR8("test"),
                                            OC_FILE_ACCESS_READ | OC_FILE_ACCESS_WRITE,
                                            &(oc_file_open_options){
                                                .root = tmpDir,
                                            }))
        {
            oc_test_fail(info, "Can't open created directory.");
        }
        else
        {
            oc_file_close(dir);
        }
    }

    oc_test(info, "create existing dir without ignore exiting flag")
    {
        oc_io_error error = oc_file_makedir(OC_STR8("test"),
                                            &(oc_file_makedir_options){
                                                .root = tmpDir,
                                            });
        if(error != OC_IO_ERR_EXISTS)
        {
            oc_test_fail(info, "Creating existing directory without OC_FILE_MAKEDIR_IGNORE_EXISTING should error with OC_IO_ERR_EXISTS.");
        }
    }

    oc_test(info, "create existing dir with ignore existing flag")
    {
        oc_io_error error = oc_file_makedir(OC_STR8("test"),
                                            &(oc_file_makedir_options){
                                                .root = tmpDir,
                                                .flags = OC_FILE_MAKEDIR_IGNORE_EXISTING,
                                            });
        if(error != OC_IO_OK)
        {
            oc_test_fail(info, "Creating existing directory with OC_FILE_MAKEDIR_IGNORE_EXISTING failed.");
        }
    }

    oc_test(info, "create dir with non-existent path")
    {
        oc_io_error error = oc_file_makedir(OC_STR8("foo/bar/baz"),
                                            &(oc_file_makedir_options){
                                                .root = tmpDir,
                                            });
        if(error != OC_IO_ERR_NO_ENTRY)
        {
            oc_test_fail(info, "Creating directory with non-existent path should error with OC_IO_ERR_NO_ENTRY.");
        }
    }

    oc_test(info, "create parents")
    {
        oc_io_error error = oc_file_makedir(OC_STR8("foo/bar/baz"),
                                            &(oc_file_makedir_options){
                                                .root = tmpDir,
                                                .flags = OC_FILE_MAKEDIR_CREATE_PARENTS,
                                            });
        if(error != OC_IO_OK)
        {
            oc_test_fail(info, "Creating directory with OC_IO_FILE_MAKEDIR_CREATE_PARENTS failed.");
        }
    }

    oc_test(info, "try create in read-only root")
    {
        oc_file dir = oc_catch(oc_file_open(OC_STR8("test"),
                                            OC_FILE_ACCESS_READ,
                                            &(oc_file_open_options){
                                                .root = tmpDir,
                                            }))
        {
            oc_test_fail(info, "Can't open directory in read-only mode.");
        }

        oc_io_error error = oc_file_makedir(OC_STR8("a"),
                                            &(oc_file_makedir_options){
                                                .root = dir,
                                            });
        if(error != OC_IO_ERR_PERM)
        {
            oc_test_fail(info, "oc_file_makedir in read-only root should error with OC_IO_ERR_PERM.");
        }
        oc_file_close(dir);
    }

    oc_test(info, "create in write-only root")
    {
        oc_file dir = oc_catch(oc_file_open(OC_STR8("test"),
                                            OC_FILE_ACCESS_WRITE,
                                            &(oc_file_open_options){
                                                .root = tmpDir,
                                            }))
        {
            oc_test_fail(info, "Can't open directory in write-only mode.");
        }

        oc_io_error error = oc_file_makedir(OC_STR8("b"),
                                            &(oc_file_makedir_options){
                                                .root = dir,
                                            });
        if(error != OC_IO_OK)
        {
            oc_test_fail(info, "failed to create directory in write-only root");
        }
        oc_file_close(dir);
    }

    oc_test(info, "create parents in write-only root")
    {
        oc_file dir = oc_catch(oc_file_open(OC_STR8("test"),
                                            OC_FILE_ACCESS_WRITE,
                                            &(oc_file_open_options){
                                                .root = tmpDir,
                                            }))
        {
            oc_test_fail(info, "Can't open directory in write-only mode.");
        }

        oc_io_error error = oc_file_makedir(OC_STR8("c/d/e"),
                                            &(oc_file_makedir_options){
                                                .root = dir,
                                                .flags = OC_FILE_MAKEDIR_CREATE_PARENTS,
                                            });
        if(error != OC_IO_OK)
        {
            oc_test_fail(info, "failed to create directory with OC_FILE_MAKEDIR_CREATE_PARENTS in write-only root");
        }
        oc_file_close(dir);
    }

    oc_file_close(tmpDir);
}

void test_remove(oc_test_info* info, oc_arena* arena)
{
    oc_file tmpDir = oc_file_nil();
    oc_test(info, "make some dirs and files")
    {
        tmpDir = oc_catch(oc_file_maketmp(OC_FILE_MAKETMP_DIRECTORY))
        {
            oc_test_fail(info, "Can't make tmp directory.");
        }

        /*
            tmpDir
                a.txt
                b.txt
                foo
                    bar
                        c.txt
                        d.txt
                        baz
        */
        oc_io_error error = oc_file_makedir(OC_STR8("foo/bar/baz"),
                                            &(oc_file_makedir_options){
                                                .root = tmpDir,
                                                .flags = OC_FILE_MAKEDIR_CREATE_PARENTS,
                                            });
        if(error != OC_IO_OK)
        {
            oc_test_fail(info, "Can't make test directories.");
        }

        oc_file f = oc_catch(oc_file_open(OC_STR8("a.txt"),
                                          OC_FILE_ACCESS_NONE,
                                          &(oc_file_open_options){
                                              .root = tmpDir,
                                              .flags = OC_FILE_OPEN_CREATE,
                                          }))
        {
            oc_test_fail(info, "Can't create test files.");
        }
        oc_file_close(f);

        f = oc_catch(oc_file_open(OC_STR8("b.txt"),
                                  OC_FILE_ACCESS_NONE,
                                  &(oc_file_open_options){
                                      .root = tmpDir,
                                      .flags = OC_FILE_OPEN_CREATE,
                                  }))
        {
            oc_test_fail(info, "Can't create test files.");
        }
        oc_file_close(f);

        f = oc_catch(oc_file_open(OC_STR8("foo/bar/c.txt"),
                                  OC_FILE_ACCESS_NONE,
                                  &(oc_file_open_options){
                                      .root = tmpDir,
                                      .flags = OC_FILE_OPEN_CREATE,
                                  }))
        {
            oc_test_fail(info, "Can't create test files.");
        }

        f = oc_catch(oc_file_open(OC_STR8("foo/bar/d.txt"),
                                  OC_FILE_ACCESS_NONE,
                                  &(oc_file_open_options){
                                      .root = tmpDir,
                                      .flags = OC_FILE_OPEN_CREATE,
                                  }))
        {
            oc_test_fail(info, "Can't create test files.");
        }

        oc_file_close(f);
    }

    oc_test(info, "try removing file in read-only dir")
    {
        oc_file dir = oc_catch(oc_file_open(OC_STR8("."),
                                            OC_FILE_ACCESS_READ,
                                            &(oc_file_open_options){
                                                .root = tmpDir,
                                            }))
        {
            oc_test_fail(info, "Can't open tmpDir in read-only access.");
        }

        oc_io_error error = oc_file_remove(OC_STR8("a.txt"),
                                           &(oc_file_remove_options){
                                               .root = dir,
                                           });
        if(error != OC_IO_ERR_PERM)
        {
            oc_test_fail(info, "Removing in a root with read-only access should error with OC_IO_ERR_PERM.");
        }
        oc_file_close(dir);
    }

    oc_test(info, "remove file in write-only dir")
    {
        oc_file dir = oc_catch(oc_file_open(OC_STR8("."),
                                            OC_FILE_ACCESS_WRITE,
                                            &(oc_file_open_options){
                                                .root = tmpDir,
                                            }))
        {
            oc_test_fail(info, "Can't open tmpDir in write-only access.");
        }

        oc_io_error error = oc_file_remove(OC_STR8("b.txt"),
                                           &(oc_file_remove_options){
                                               .root = dir,
                                           });

        if(error != OC_IO_OK)
        {
            oc_test_fail(info, "Removing in a root with write-only access failed.");
        }

        oc_result_if(oc_file_open(OC_STR8("b.txt"), OC_FILE_ACCESS_NONE, &(oc_file_open_options){ .root = tmpDir }))
        {
            oc_test_fail(info, "File wasn't actually removed.");
        }
        else if(oc_last_error() != OC_IO_ERR_NO_ENTRY)
        {
            oc_test_fail(info, "Trying to open file that was previously removed should error with OC_IO_ERR_NO_ENTRY.");
        }
        oc_file_close(dir);
    }

    oc_test(info, "remove file")
    {
        oc_io_error error = oc_file_remove(OC_STR8("a.txt"),
                                           &(oc_file_remove_options){
                                               .root = tmpDir,
                                           });
        if(error != OC_IO_OK)
        {
            oc_test_fail(info, "Failed to remove file.");
        }
        oc_result_if(oc_file_open(OC_STR8("a.txt"), OC_FILE_ACCESS_NONE, &(oc_file_open_options){ .root = tmpDir }))
        {
            oc_test_fail(info, "File wasn't actually removed.");
        }
        else if(oc_last_error() != OC_IO_ERR_NO_ENTRY)
        {
            oc_test_fail(info, "Trying to open file that was previously removed should error with OC_IO_ERR_NO_ENTRY.");
        }
    }

    oc_test(info, "remove file with remove dir flag") // should be allowed
    {
        oc_io_error error = oc_file_remove(OC_STR8("foo/bar/c.txt"),
                                           &(oc_file_remove_options){
                                               .root = tmpDir,
                                               .flags = OC_FILE_REMOVE_DIR,
                                           });
        if(error != OC_IO_OK)
        {
            oc_test_fail(info, "Failed to remove file.");
        }
        oc_result_if(oc_file_open(OC_STR8("foo/bar/c.txt"), OC_FILE_ACCESS_NONE, &(oc_file_open_options){ .root = tmpDir }))
        {
            oc_test_fail(info, "File wasn't actually removed.");
        }
        else if(oc_last_error() != OC_IO_ERR_NO_ENTRY)
        {
            oc_test_fail(info, "Trying to open file that was previously removed should error with OC_IO_ERR_NO_ENTRY.");
        }
    }

    oc_test(info, "try removing dir without remove dir flag")
    {
        oc_io_error error = oc_file_remove(OC_STR8("foo/bar/baz"),
                                           &(oc_file_remove_options){
                                               .root = tmpDir,
                                           });
        if(error != OC_IO_ERR_DIR)
        {
            oc_test_fail(info, "Removing directory without OC_FILE_REMOVE_DIR should error with OC_IO_ERR_DIR.");
        }
    }

    oc_test(info, "remove empty dir")
    {
        oc_io_error error = oc_file_remove(OC_STR8("foo/bar/baz"),
                                           &(oc_file_remove_options){
                                               .root = tmpDir,
                                               .flags = OC_FILE_REMOVE_DIR,
                                           });
        if(error != OC_IO_OK)
        {
            oc_test_fail(info, "Failed to remove directory.");
        }
        oc_result_if(oc_file_open(OC_STR8("foo/bar/baz"), OC_FILE_ACCESS_NONE, &(oc_file_open_options){ .root = tmpDir }))
        {
            oc_test_fail(info, "Directory wasn't actually removed.");
        }
        else if(oc_last_error() != OC_IO_ERR_NO_ENTRY)
        {
            oc_test_fail(info, "Trying to open directory that was previously removed should error with OC_IO_ERR_NO_ENTRY.");
        }
    }

    oc_test(info, "try removing non-empty dir")
    {
        oc_io_error error = oc_file_remove(OC_STR8("foo"),
                                           &(oc_file_remove_options){
                                               .root = tmpDir,
                                               .flags = OC_FILE_REMOVE_DIR,
                                           });
        if(error != OC_IO_ERR_NOT_EMPTY)
        {
            oc_test_fail(info, "Removing non-empty directory should error with OC_IO_ERR_NOT_EMPTY.");
        }
    }

    oc_test(info, "remove recursive")
    {
        oc_io_error error = oc_file_remove(OC_STR8("foo"),
                                           &(oc_file_remove_options){
                                               .root = tmpDir,
                                               .flags = OC_FILE_REMOVE_DIR | OC_FILE_REMOVE_RECURSIVE,
                                           });
        if(error != OC_IO_OK)
        {
            oc_test_fail(info, "Failed to remove directory.");
        }
        oc_result_if(oc_file_open(OC_STR8("foo"), OC_FILE_ACCESS_NONE, &(oc_file_open_options){ .root = tmpDir }))
        {
            oc_test_fail(info, "Directory wasn't actually removed.");
        }
        else if(oc_last_error() != OC_IO_ERR_NO_ENTRY)
        {
            oc_test_fail(info, "Trying to open directory that was previously removed should error with OC_IO_ERR_NO_ENTRY.");
        }
    }

    oc_file_close(tmpDir);
}

void test_copy(oc_test_info* info, oc_arena* arena)
{
    oc_str8 dirPath = oc_path_append(arena, TEST_DIR, OC_STR8("data"));

    oc_file dataDir = oc_file_nil();
    oc_file tmpDir = oc_file_nil();

    oc_test(info, "open source and dest dirs")
    {
        dataDir = oc_catch(oc_file_open(dirPath, OC_FILE_ACCESS_READ, 0))
        {
            oc_test_fail(info, "Couldn't open data directory");
        }

        tmpDir = oc_catch(oc_file_maketmp(OC_FILE_MAKETMP_DIRECTORY))
        {
            oc_test_fail(info, "Can't make tmp directory.");
        }
    }

    oc_test(info, "copy recursive")
    {
        oc_io_error error = oc_file_copy(OC_STR8("foo"),
                                         OC_STR8("foo"),
                                         &(oc_file_copy_options){
                                             .srcRoot = dataDir,
                                             .dstRoot = tmpDir,
                                         });

        if(error != OC_IO_OK)
        {
            oc_test_fail(info, "copy recursive failed.");
        }

        oc_file test = oc_catch(oc_file_open(OC_STR8("foo/bar/test.txt"),
                                             OC_FILE_ACCESS_READ,
                                             &(oc_file_open_options){
                                                 .root = tmpDir,
                                             }))
        {
            oc_test_fail(info, "Couldn't open copied file.");
        }
        int r = check_string(info, test, OC_STR8("Hello from foo/bar/test.txt"));
        if(r)
        {
            oc_test_fail(info, "Mismatched contents for copied file.");
        }
        oc_file_close(test);
    }

    oc_file_close(dataDir);
    oc_file_close(tmpDir);
}

oc_str8 parseTestDir(int argc, const char** argv, oc_arena* arena)
{
    const char* test_dir_arg_prefix = "--test-dir=";
    for(int i = 1; i < argc; ++i)
    {
        const char* arg = argv[i];
        const size_t arglen = strlen(arg);

        if(strstr(arg, test_dir_arg_prefix) == arg)
        {
            const char* slice = arg + strlen(test_dir_arg_prefix);
            return OC_STR8(slice);
        }
    }

    return OC_STR8("");
}

int main(int argc, const char** argv)
{
    oc_arena_scope scratch = oc_scratch_begin();

    TEST_DIR = parseTestDir(argc, argv, scratch.arena);

    oc_test_info info = { 0 };
    oc_test_init(&info, "files", OC_TEST_PRINT_ALL);

    oc_test_group(&info, "resolve")
    {
        test_resolve(&info, scratch.arena);
    }
    oc_test_group(&info, "write")
    {
        test_write(&info, scratch.arena);
    }
    oc_test_group(&info, "read")
    {
        test_read(&info, scratch.arena);
    }
    oc_test_group(&info, "stat")
    {
        test_stat_size(&info, scratch.arena);
    }
    oc_test_group(&info, "stat")
    {
        test_stat_type(&info, scratch.arena);
    }
    oc_test_group(&info, "args")
    {
        test_args(&info, scratch.arena);
    }
    oc_test_group(&info, "symlinks")
    {
        test_symlinks(&info, scratch.arena);
    }
    oc_test_group(&info, "rights")
    {
        test_rights(&info, scratch.arena);
    }
    oc_test_group(&info, "jail")
    {
        test_jail(&info, scratch.arena);
    }
    oc_test_group(&info, "maketmp")
    {
        test_maketmp(&info, scratch.arena);
    }
    oc_test_group(&info, "makedir")
    {
        test_makedir(&info, scratch.arena);
    }
    oc_test_group(&info, "remove")
    {
        test_remove(&info, scratch.arena);
    }
    oc_test_group(&info, "copy")
    {
        test_copy(&info, scratch.arena);
    }

    oc_test_summary(&info);
    return info.totalFailed ? -1 : 0;
}
