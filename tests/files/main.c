/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#define OC_NO_APP_LAYER
#include "orca.c"

oc_str8 TEST_DIR;

int test_write(oc_arena* arena)
{
    oc_log_info("writing\n");

    oc_str8 path = oc_path_append(arena, TEST_DIR, OC_STR8("data/write_test.txt"));
    oc_str8 test_string = OC_STR8("Hello from write_test.txt");

    oc_file f = oc_file_open(path, OC_FILE_ACCESS_WRITE, OC_FILE_OPEN_CREATE | OC_FILE_OPEN_TRUNCATE);
    if(oc_file_last_error(f))
    {
        oc_log_error("Can't create/open file %.*s for writing\n", (int)path.len, path.ptr);
        return (-1);
    }

    oc_file_write(f, test_string.len, test_string.ptr);
    if(oc_file_last_error(f))
    {
        oc_log_error("Error while writing %.*s\n", (int)path.len, path.ptr);
        return (-1);
    }
    oc_file_close(f);

    // check
    char* pathCStr = oc_str8_to_cstring(arena, path);
    FILE* file = fopen(pathCStr, "r");
    if(!file)
    {
        oc_log_error("File %.*s not found while checking\n", (int)path.len, path.ptr);
        return (-1);
    }
    char buffer[256];
    int n = fread(buffer, 1, 256, file);
    if(n != test_string.len || strncmp(buffer, test_string.ptr, test_string.len))
    {
        oc_log_error("Didn't recover test string\n");
        return (-1);
    }
    fclose(file);

    remove(path.ptr);

    return (0);
}

int check_string(oc_file f, oc_str8 test_string)
{
    char buffer[256];
    i64 n = oc_file_read(f, 256, buffer);
    if(oc_file_last_error(f))
    {
        oc_log_error("Error while reading test string\n");
        return (-1);
    }

    if(oc_str8_cmp(test_string, oc_str8_from_buffer(n, buffer)))
    {
        return (-1);
    }

    return (0);
}

int test_read(oc_arena* arena)
{
    oc_log_info("reading\n");

    oc_str8 path = oc_path_append(arena, TEST_DIR, OC_STR8("data/regular.txt"));
    oc_str8 test_string = OC_STR8("Hello from regular.txt");

    oc_file f = oc_file_open(path, OC_FILE_ACCESS_READ, 0);
    if(oc_file_last_error(f))
    {
        oc_log_error("Can't open file %.*s for reading\n", (int)path.len, path.ptr);
        return (-1);
    }

    if(check_string(f, test_string))
    {
        oc_log_error("Check string failed\n");
        return (-1);
    }

    oc_file_close(f);
    return (0);
}

int test_stat_size(oc_arena* arena)
{
    oc_log_info("stat size\n");

    oc_str8 path = oc_path_append(arena, TEST_DIR, OC_STR8("data/regular.txt"));
    oc_str8 test_string = OC_STR8("Hello from regular.txt");
    u64 size = test_string.len;

    oc_file f = oc_file_open(path, 0, 0);
    if(oc_file_last_error(f))
    {
        oc_log_error("Can't open file\n");
        return (-1);
    }

    oc_file_status status = oc_file_get_status(f);

    if(oc_file_last_error(f))
    {
        oc_log_error("Error while retrieving file status\n");
        return (-1);
    }

    if(status.size != size)
    {
        oc_log_error("size doesn't match\n");
        return (-1);
    }

    return (0);
}

int test_stat_type(oc_arena* arena)
{
    oc_str8 regular = oc_path_append(arena, TEST_DIR, OC_STR8("data/regular.txt"));
    oc_str8 dir = oc_path_append(arena, TEST_DIR, OC_STR8("data/directory"));
    oc_str8 link = oc_path_append(arena, TEST_DIR, OC_STR8("data/symlink"));

    oc_log_info("stat type, regular\n");

    oc_file f = oc_file_open(regular, 0, 0);
    if(oc_file_last_error(f))
    {
        oc_log_error("Can't open file\n");
        return (-1);
    }

    oc_file_status status = oc_file_get_status(f);
    if(oc_file_last_error(f))
    {
        oc_log_error("Error while retrieving file status\n");
        return (-1);
    }
    if(status.type != OC_FILE_REGULAR)
    {
        oc_log_error("file type doesn't match\n");
        return (-1);
    }
    oc_file_close(f);

    oc_log_info("stat type, directory\n");

    f = oc_file_open(dir, 0, 0);
    if(oc_file_last_error(f))
    {
        oc_log_error("Can't open file\n");
        return (-1);
    }

    status = oc_file_get_status(f);
    if(oc_file_last_error(f))
    {
        oc_log_error("Error while retrieving file status\n");
        return (-1);
    }
    if(status.type != OC_FILE_DIRECTORY)
    {
        oc_log_error("file type doesn't match\n");
        return (-1);
    }
    oc_file_close(f);

    oc_log_info("stat type, symlink\n");

    f = oc_file_open(link, OC_FILE_ACCESS_NONE, OC_FILE_OPEN_SYMLINK);
    if(oc_file_last_error(f))
    {
        oc_log_error("Can't open file\n");
        return (-1);
    }

    status = oc_file_get_status(f);
    if(oc_file_last_error(f))
    {
        oc_log_error("Error while retrieving file status\n");
        return (-1);
    }
#if !OC_PLATFORM_WINDOWS
    if(status.type != OC_FILE_SYMLINK)
    {
        oc_log_error("file type doesn't match\n");
        return (-1);
    }
#endif
    oc_file_close(f);

    return (0);
}

int test_symlinks(oc_arena* arena)
{
#if !OC_PLATFORM_WINDOWS
    oc_str8 path = oc_path_append(arena, TEST_DIR, OC_STR8("data/symlink"));

    // open symlink target
    oc_log_info("open symlink target\n");
    oc_file f = oc_file_open_at(oc_file_nil(), path, OC_FILE_ACCESS_READ, 0);
    if(oc_file_last_error(f))
    {
        oc_log_error("failed to open %s\n", path.ptr);
        return (-1);
    }
    if(check_string(f, OC_STR8("Hello from regular.txt")))
    {
        oc_log_error("Check string failed\n");
        return (-1);
    }
    oc_file_close(f);

    // open symlink file
    oc_log_info("open symlink file\n");
    f = oc_file_open_at(oc_file_nil(), path, OC_FILE_ACCESS_READ, OC_FILE_OPEN_SYMLINK);
    if(oc_file_last_error(f))
    {
        oc_log_error("failed to open %s\n", path.ptr);
        return (-1);
    }
    oc_file_status status = oc_file_get_status(f);
    if(oc_file_last_error(f))
    {
        oc_log_error("Error while retrieving file status\n");
        return (-1);
    }
    if(status.type != OC_FILE_SYMLINK)
    {
        oc_log_error("file type doesn't match\n");
        return (-1);
    }

    char buffer[512];
    int n = oc_file_read(f, 512, buffer);

    if(n || (oc_file_last_error(f) == OC_IO_OK))
    {
        oc_log_error("file read should fail on symlinks\n");
        return (-1);
    }

    oc_file_close(f);
#endif

    return (0);
}

int test_args(oc_arena* arena)
{
    oc_str8 path = oc_path_append(arena, TEST_DIR, OC_STR8("data/regular.txt"));

    //NOTE: nil handle
    oc_log_info("check open_at with nil handle\n");
    oc_file f = oc_file_open_at(oc_file_nil(), path, OC_FILE_ACCESS_READ, 0);
    if(oc_file_last_error(f))
    {
        oc_log_error("oc_file_open_at() with nil handle failed\n");
        return (-1);
    }
    if(check_string(f, OC_STR8("Hello from regular.txt")))
    {
        oc_log_error("Check string failed\n");
        return (-1);
    }
    oc_file_close(f);

    //NOTE: invalid handle
    oc_log_info("check open_at with nil handle\n");
    oc_file wrongHandle = { .h = 123456789 };

    f = oc_file_open_at(wrongHandle, path, OC_FILE_ACCESS_READ, 0);
    if(oc_file_last_error(f) != OC_IO_ERR_HANDLE)
    {
        oc_log_error("oc_file_open_at() with non-nil invalid handle should return OC_IO_ERR_HANDLE\n");
        return (-1);
    }
    oc_file_close(f);

    //NOTE: nil/wrong handle and OC_FILE_OPEN_RESTRICT
    oc_log_info("check open_at with nil handle and OC_FILE_OPEN_RESTRICT\n");

    f = oc_file_open_at(oc_file_nil(), path, OC_FILE_ACCESS_READ, OC_FILE_OPEN_RESTRICT);
    if(oc_file_last_error(f) != OC_IO_ERR_HANDLE)
    {
        oc_log_error("oc_file_open_at() with nil handle and OC_FILE_OPEN_RESTRICT should return OC_IO_ERR_HANDLE\n");
        return (-1);
    }
    oc_file_close(f);

    f = oc_file_open_at(wrongHandle, path, OC_FILE_ACCESS_READ, OC_FILE_OPEN_RESTRICT);
    if(oc_file_last_error(f) != OC_IO_ERR_HANDLE)
    {
        oc_log_error("oc_file_open_at() with invalid handle and OC_FILE_OPEN_RESTRICT should return OC_IO_ERR_HANDLE\n");
        return (-1);
    }
    oc_file_close(f);

    //NOTE: empty path
    oc_log_info("check empty path\n");

    f = oc_file_open_at(oc_file_nil(), OC_STR8(""), OC_FILE_ACCESS_READ, 0);
    if(oc_file_last_error(f) != OC_IO_ERR_ARG)
    {
        oc_log_error("empty path should return OC_IO_ERR_ARG\n");
        return (-1);
    }
    oc_file_close(f);

    return (0);
}

int test_jail(oc_arena* arena)
{
    oc_log_info("test jail\n");

    oc_str8 jailPath = oc_path_append(arena, TEST_DIR, OC_STR8("data/jail"));

    oc_file jail = oc_file_open(jailPath, OC_FILE_ACCESS_READ, 0);
    if(oc_file_last_error(jail))
    {
        oc_log_error("Can't open jail directory\n");
        return (-1);
    }

    //-----------------------------------------------------------
    //NOTE: Check escapes
    //-----------------------------------------------------------
    oc_log_info("check potential escapes\n");

    //NOTE: escape with absolute path
    oc_file f = oc_file_open_at(jail, OC_STR8("/tmp"), OC_FILE_ACCESS_READ, OC_FILE_OPEN_RESTRICT);
    if(oc_file_last_error(f) != OC_IO_ERR_NO_ENTRY)
    {
        oc_log_error("Escaped jail with absolute path /tmp\n");
        return (-1);
    }
    oc_file_close(f);

    //NOTE: escape with ..
    f = oc_file_open_at(jail, OC_STR8(".."), OC_FILE_ACCESS_READ, OC_FILE_OPEN_RESTRICT);
    if(oc_file_last_error(f) != OC_IO_ERR_WALKOUT)
    {
        oc_log_error("Escaped jail with relative path ..\n");
        return (-1);
    }
    oc_file_close(f);

    //NOTE: escape with dir/../..
    f = oc_file_open_at(jail, OC_STR8("dir/../.."), OC_FILE_ACCESS_READ, OC_FILE_OPEN_RESTRICT);
    if(oc_file_last_error(f) != OC_IO_ERR_WALKOUT)
    {
        oc_log_error("Escaped jail with relative path dir/../..\n");
        return (-1);
    }
    oc_file_close(f);

    //NOTE: escape with symlink to parent
#if !OC_PLATFORM_WINDOWS
    f = oc_file_open_at(jail, OC_STR8("/dir_escape"), OC_FILE_ACCESS_READ, OC_FILE_OPEN_RESTRICT);
    if(oc_file_last_error(f) != OC_IO_ERR_WALKOUT)
    {
        oc_log_error("Escaped jail with symlink to parent\n");
        return (-1);
    }
    oc_file_close(f);

    //NOTE: escape to file with symlink to parent
    f = oc_file_open_at(jail, OC_STR8("/dir_escape/regular.txt"), OC_FILE_ACCESS_READ, OC_FILE_OPEN_RESTRICT);
    if(oc_file_last_error(f) != OC_IO_ERR_WALKOUT)
    {
        oc_log_error("Escaped jail to regular.txt with symlink to parent\n");
        return (-1);
    }
    oc_file_close(f);

    //NOTE: escape with symlink to file
    f = oc_file_open_at(jail, OC_STR8("/file_escape"), OC_FILE_ACCESS_READ, OC_FILE_OPEN_RESTRICT);
    if(oc_file_last_error(f) != OC_IO_ERR_WALKOUT)
    {
        oc_log_error("Escaped jail with symlink to file regular.txt\n");
        return (-1);
    }
    oc_file_close(f);
#endif

    //NOTE: escape with bad root handle
    oc_file wrong_handle = { 0 };
    oc_str8 regularPath = oc_path_append(arena, TEST_DIR, OC_STR8("data/regular.txt"));
    f = oc_file_open_at(wrong_handle, regularPath, OC_FILE_ACCESS_READ, OC_FILE_OPEN_RESTRICT);
    if(oc_file_last_error(f) == OC_IO_OK)
    {
        oc_log_error("Escaped jail with nil root handle\n");
        return (-1);
    }
    if(oc_file_last_error(f) != OC_IO_ERR_HANDLE)
    {
        oc_log_error("OC_FILE_OPEN_RESTRICT with invalid root handle should return OC_IO_ERR_HANDLE\n");
        return (-1);
    }
    oc_file_close(f);

    //-----------------------------------------------------------
    //NOTE: empty path
    //-----------------------------------------------------------
    oc_log_info("check empty path\n");

    f = oc_file_open_at(jail, OC_STR8(""), OC_FILE_ACCESS_READ, 0);
    if(oc_file_last_error(f) != OC_IO_ERR_ARG)
    {
        oc_log_error("empty path should return OC_IO_ERR_ARG\n");
        return (-1);
    }
    oc_file_close(f);

    //-----------------------------------------------------------
    //NOTE: Check legitimates open
    //-----------------------------------------------------------
    oc_log_info("check legitimates open\n");

    //NOTE: regular file jail/test.txt
    f = oc_file_open_at(jail, OC_STR8("/test.txt"), OC_FILE_ACCESS_READ, OC_FILE_OPEN_RESTRICT);
    if(oc_file_last_error(f) != OC_IO_OK)
    {
        oc_log_error("Can't open jail/test.txt\n");
        return (-1);
    }
    if(check_string(f, OC_STR8("Hello from jail/test.txt")))
    {
        oc_log_error("Check string failed\n");
        return (-1);
    }
    oc_file_close(f);

    //NOTE: valid file traversal to jail/test.txt
    f = oc_file_open_at(jail, OC_STR8("/dir/../test.txt"), OC_FILE_ACCESS_READ, OC_FILE_OPEN_RESTRICT);
    if(oc_file_last_error(f) != OC_IO_OK)
    {
        oc_log_error("Can't open jail/dir/../test.txt\n");
        return (-1);
    }
    if(check_string(f, OC_STR8("Hello from jail/test.txt")))
    {
        oc_log_error("Check string failed\n");
        return (-1);
    }
    oc_file_close(f);

    //NOTE: re-open root directory
    f = oc_file_open_at(jail, OC_STR8("."), OC_FILE_ACCESS_READ, OC_FILE_OPEN_RESTRICT);
    if(oc_file_last_error(f) != OC_IO_OK)
    {
        oc_log_error("Can't open jail/.\n");
        return (-1);
    }
    {
        //NOTE: access regular file test.txt inside reopened root
        oc_file f2 = oc_file_open_at(f, OC_STR8("test.txt"), OC_FILE_ACCESS_READ, 0);

        if(check_string(f2, OC_STR8("Hello from jail/test.txt")))
        {
            oc_log_error("Check string failed\n");
            return (-1);
        }
        oc_file_close(f2);
    }
    oc_file_close(f);

    return (0);
}

int test_rights(oc_arena* arena)
{
    oc_log_info("test rights\n");

    oc_str8 dirPath = oc_path_append(arena, TEST_DIR, OC_STR8("data"));

    //--------------------------------------------------------------------------------------
    // base dir with no access
    //--------------------------------------------------------------------------------------
    {
        oc_file dir = oc_file_open(dirPath, OC_FILE_ACCESS_NONE, 0);
        if(oc_file_last_error(dir))
        {
            oc_log_error("Couldn't open data with no access rights\n");
            return (-1);
        }

        oc_file f = oc_file_open_at(dir, OC_STR8("./regular.txt"), OC_FILE_ACCESS_READ, 0);
        if(oc_file_last_error(f) != OC_IO_ERR_PERM)
        {
            oc_log_error("Incorrect check when opening file with read access in dir with no access\n");
            return (-1);
        }
        oc_file_close(f);
        oc_file_close(dir);
    }
    //--------------------------------------------------------------------------------------
    // base dir with read access
    //--------------------------------------------------------------------------------------
    {
        oc_file dir = oc_file_open(dirPath, OC_FILE_ACCESS_READ, 0);
        if(oc_file_last_error(dir))
        {
            oc_log_error("Couldn't open ./data with read rights\n");
            return (-1);
        }

        // check that we _can't_ open a file with write access
        oc_file f = oc_file_open_at(dir, OC_STR8("./regular.txt"), OC_FILE_ACCESS_WRITE, 0);
        if(oc_file_last_error(f) != OC_IO_ERR_PERM)
        {
            oc_log_error("Incorrect check when opening file with write access in dir with read access\n");
            return (-1);
        }
        oc_file_close(f);

        // check that we _can_ open a file with read access
        f = oc_file_open_at(dir, OC_STR8("./regular.txt"), OC_FILE_ACCESS_READ, 0);
        if(oc_file_last_error(f))
        {
            oc_log_error("Couldn't open file with read access in dir with read access\n");
            return (-1);
        }

        // check that we _can't_ write to that file
        oc_str8 testWrite = OC_STR8("Hello, world!\n");
        if(oc_file_write(f, testWrite.len, testWrite.ptr) != 0)
        {
            oc_log_error("Incorrectly wrote to read-only file\n");
            return (-1);
        }
        if(oc_file_last_error(f) != OC_IO_ERR_PERM)
        {
            oc_log_error("Incorrect error returned from writing to read-only file\n");
            return (-1);
        }

        oc_file_close(f);
        oc_file_close(dir);
    }
    //--------------------------------------------------------------------------------------
    // base dir with write access
    //--------------------------------------------------------------------------------------
    {
        oc_file dir = oc_file_open(dirPath, OC_FILE_ACCESS_WRITE, 0);
        if(oc_file_last_error(dir))
        {
            oc_log_error("Couldn't open %s with write rights\n", dirPath.ptr);
            return (-1);
        }

        // check that we _can't_ open a file with read access
        oc_file f = oc_file_open_at(dir, OC_STR8("./regular.txt"), OC_FILE_ACCESS_READ, 0);
        if(oc_file_last_error(f) != OC_IO_ERR_PERM)
        {
            oc_log_error("Incorrect check when opening file with read access in dir with write access\n");
            return (-1);
        }
        oc_file_close(f);

        // check that we _can_ open a file with write access
        f = oc_file_open_at(dir, OC_STR8("./regular.txt"), OC_FILE_ACCESS_WRITE, 0);
        if(oc_file_last_error(f))
        {
            oc_log_error("Couldn't open file with write access in dir with write access\n");
            return (-1);
        }

        // check that we _can't_ read that file
        char testRead[512];
        if(oc_file_read(f, 512, testRead) != 0)
        {
            oc_log_error("Incorrectly read write-only file\n");
            return (-1);
        }
        if(oc_file_last_error(f) != OC_IO_ERR_PERM)
        {
            oc_log_error("Incorrect error returned from reading write-only file\n");
            return (-1);
        }

        oc_file_close(f);
        oc_file_close(dir);
    }
    //--------------------------------------------------------------------------------------
    // base dir with read/write access
    //--------------------------------------------------------------------------------------
    {
        oc_file dir = oc_file_open(dirPath, OC_FILE_ACCESS_READ | OC_FILE_ACCESS_WRITE, 0);
        if(oc_file_last_error(dir))
        {
            oc_log_error("Couldn't open ./data with read rights\n");
            return (-1);
        }

        // check that we can open file with read access
        oc_file f = oc_file_open_at(dir, OC_STR8("./regular.txt"), OC_FILE_ACCESS_READ, 0);
        if(oc_file_last_error(f))
        {
            oc_log_error("Incorrect check when opening file with read access in dir with read/write access\n");
            return (-1);
        }
        oc_file_close(f);

        // check that we can open file with write access
        f = oc_file_open_at(dir, OC_STR8("./regular.txt"), OC_FILE_ACCESS_WRITE, 0);
        if(oc_file_last_error(f))
        {
            oc_log_error("Couldn't open file with write access in dir with read/write access\n");
            return (-1);
        }

        oc_file_close(f);
        oc_file_close(dir);
    }
    return (0);
}

int test_resolve(oc_arena* arena)
{
    //NOTE: abs path
    oc_str8 path = OC_STR8("/usr/bin");
    oc_io_resolve_result r = oc_io_resolve(arena, 0, path);
    if(r.error || oc_str8_cmp(r.path, OC_STR8("/usr/bin")))
    {
        oc_log_error("Bad path resolution.\n");
        return -1;
    }

    //NOTE: relative path
    path = OC_STR8("tests/files/data/regular.txt");

    r = oc_io_resolve(arena, 0, path);
    if(r.error || oc_str8_cmp(r.path, OC_STR8("tests/files/data/regular.txt")))
    {
        oc_log_error("Bad path resolution.\n");
        return -1;
    }

    //NOTE: relative path with ..
    path = OC_STR8("tests/files/data/directory/../regular.txt");
    r = oc_io_resolve(arena, 0, path);
    if(r.error || oc_str8_cmp(r.path, OC_STR8("tests/files/data/regular.txt")))
    {
        oc_log_error("Bad path resolution.\n");
        return -1;
    }

    //NOTE: relative path with symlink
    path = OC_STR8("tests/files/data/symlink");
    r = oc_io_resolve(arena, 0, path);
    if(r.error || oc_str8_cmp(r.path, OC_STR8("tests/files/data/regular.txt")))
    {
        oc_log_error("Bad path resolution.\n");
        return -1;
    }

    //NOTE: relative path with non-existing end
    path = OC_STR8("tests/files/data/directory/foo/../bar");
    r = oc_io_resolve(arena, 0, path);
    if(r.error || oc_str8_cmp(r.path, OC_STR8("tests/files/data/directory/bar")))
    {
        oc_log_error("Bad path resolution.\n");
        return -1;
    }

    //NOTE: tests with root
    oc_str8 dirPath = oc_path_append(arena, TEST_DIR, OC_STR8("data"));
    oc_file dir = oc_file_open(dirPath, OC_FILE_ACCESS_READ, 0);
    oc_file_slot* dirSlot = oc_file_slot_from_handle(&oc_globalFileTable, dir);

    //NOTE: relative path with root
    r = oc_io_resolve(arena, dirSlot, OC_STR8("directory/test.txt"));
    if(r.error || oc_str8_cmp(r.path, OC_STR8("directory/test.txt")))
    {
        oc_log_error("Bad path resolution.\n");
        return -1;
    }

    //NOTE: absolute path with root
    r = oc_io_resolve(arena, dirSlot, OC_STR8("/directory/test.txt"));
    if(r.error || oc_str8_cmp(r.path, OC_STR8("/directory/test.txt")))
    {
        oc_log_error("Bad path resolution.\n");
        return -1;
    }

    //NOTE: path with symlink
    r = oc_io_resolve(arena, dirSlot, OC_STR8("symlink"));
    if(r.error || oc_str8_cmp(r.path, OC_STR8("regular.txt")))
    {
        oc_log_error("Bad path resolution.\n");
        return -1;
    }

    //NOTE: path with valid '..'
    r = oc_io_resolve(arena, dirSlot, OC_STR8("directory/../regular.txt"));
    if(r.error || oc_str8_cmp(r.path, OC_STR8("regular.txt")))
    {
        oc_log_error("Bad path resolution.\n");
        return -1;
    }

    //NOTE: path with valid non-existing end
    r = oc_io_resolve(arena, dirSlot, OC_STR8("directory/foo/../test.txt"));
    if(r.error || oc_str8_cmp(r.path, OC_STR8("directory/test.txt")))
    {
        oc_log_error("Bad path resolution.\n");
        return -1;
    }

    //NOTE: test escapes

    //NOTE: path with escaping '..'
    r = oc_io_resolve(arena, dirSlot, OC_STR8("directory/../../foo"));
    if(r.error != OC_IO_ERR_WALKOUT)
    {
        oc_log_error("Bad path resolution.\n");
        return -1;
    }

    oc_file jail = oc_file_open_at(dir, OC_STR8("jail"), OC_FILE_ACCESS_READ, 0);
    oc_file_slot* jailSlot = oc_file_slot_from_handle(&oc_globalFileTable, jail);

    //NOTE: path with escaping symlink to file
    r = oc_io_resolve(arena, jailSlot, OC_STR8("file_escape"));
    if(r.error != OC_IO_ERR_WALKOUT)
    {
        oc_log_error("Bad path resolution.\n");
        return -1;
    }

    //NOTE: path with escaping symlink to dir
    r = oc_io_resolve(arena, jailSlot, OC_STR8("dir_escape"));
    if(r.error != OC_IO_ERR_WALKOUT)
    {
        oc_log_error("Bad path resolution.\n");
        return -1;
    }

    return 0;
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
    oc_arena* arena = scratch.arena;

    TEST_DIR = parseTestDir(argc, argv, arena);

    if(test_resolve(arena))
    {
        return (-1);
    }
    if(test_write(arena))
    {
        return (-1);
    }
    if(test_read(arena))
    {
        return (-1);
    }
    if(test_stat_size(arena))
    {
        return (-1);
    }
    if(test_stat_type(arena))
    {
        return (-1);
    }
    if(test_args(arena))
    {
        return (-1);
    }
    if(test_symlinks(arena))
    {
        return (-1);
    }
    if(test_rights(arena))
    {
        return (-1);
    }
    if(test_jail(arena))
    {
        return (-1);
    }

    oc_log_info("OK\n");

    return (0);
}
