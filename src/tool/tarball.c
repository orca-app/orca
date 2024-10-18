/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#if OC_PLATFORM_MACOS
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif

#include "system.h"
#include "zlib.h"
#include "microtar.h"
#include "orca.h"

#define CHUNK_SIZE 262144

static int custom_file_write(mtar_t* tar, const void* data, unsigned size)
{
    size_t res = fwrite(data, 1, size, tar->stream);
    return (res == size) ? MTAR_ESUCCESS : MTAR_EWRITEFAIL;
}

static int custom_file_read(mtar_t* tar, void* data, unsigned size)
{
    size_t res = fread(data, 1, size, tar->stream);
    return (res == size) ? MTAR_ESUCCESS : MTAR_EREADFAIL;
}

static int custom_file_seek(mtar_t* tar, unsigned offset)
{
    int res = fseek(tar->stream, offset, SEEK_SET);
    return (res == 0) ? MTAR_ESUCCESS : MTAR_ESEEKFAIL;
}

static int custom_file_close(mtar_t* tar)
{
    fclose(tar->stream);
    return MTAR_ESUCCESS;
}

#if OC_PLATFORM_WINDOWS
static bool write_file_from_tar(oc_str8 path, mtar_header_t* header, char* buf, u64 size)
{
    (void)header;
    oc_file file = oc_file_open(path, OC_FILE_ACCESS_WRITE, OC_FILE_OPEN_CREATE);
    u64 written = oc_file_write(file, size, buf);
    oc_file_close(file);
    if(written != size)
    {
        return false;
    }
    return true;
}
#elif OC_PLATFORM_MACOS
// NOTE(shaw): open() is used here because some files extracted from an archive
// need execute permissions which is not possible with oc_file_open or fopen
static bool write_file_from_tar(oc_str8 path, mtar_header_t* header, char* buf, u64 size)
{
    oc_arena_scope scratch = oc_scratch_begin();
    char* cpath = oc_str8_to_cstring(scratch.arena, path);
    int fd = open(cpath, O_WRONLY | O_CREAT, header->mode);

    bool write_success = true;
    ssize_t bytes_written = 0;
    for(u64 off = 0; off < size; off += bytes_written)
    {
        bytes_written = write(fd, buf + off, size - off);
        if(bytes_written == 0 || bytes_written == -1)
        {
            write_success = false;
            break;
        }
    }

    int err = close(fd);
    oc_scratch_end(scratch);
    if(err == -1)
    {
        return false;
    }

    return write_success;
}
#else
    #error write_file_from_tar() not implemented on this platform.
#endif

int gzip_decompress(gzFile in_file, FILE* out_file)
{
    oc_arena_scope scratch = oc_scratch_begin();
    char* out = oc_arena_push(scratch.arena, CHUNK_SIZE);
    int uncompressed_bytes = 0;
    int bytes_written = 0;
    int result = Z_OK;

    for(;;)
    {
        uncompressed_bytes = gzread(in_file, out, CHUNK_SIZE);
        if(uncompressed_bytes == -1)
        {
            result = Z_STREAM_ERROR;
            break;
        }

        bytes_written = (int)fwrite(out, 1, uncompressed_bytes, out_file);
        if(bytes_written != uncompressed_bytes || ferror(out_file))
        {
            result = Z_ERRNO;
            break;
        }

        if(uncompressed_bytes < CHUNK_SIZE)
        {
            break;
        }
    }
    oc_scratch_end(scratch);
    return result;
}

int untar(FILE* file, oc_str8 out_dir)
{
    if(fseek(file, 0, SEEK_SET) != 0)
    {
        return MTAR_ESEEKFAIL;
    }

    mtar_header_t h;
    mtar_t tar = { 0 };
    tar.write = custom_file_write;
    tar.read = custom_file_read;
    tar.seek = custom_file_seek;
    tar.close = custom_file_close;
    tar.stream = file;

    int err = mtar_read_header(&tar, &h);
    while(err == MTAR_ESUCCESS)
    {
        if(h.type == MTAR_TDIR)
        {
            oc_arena_scope scratch = oc_scratch_begin();
            oc_str8 dir_path = oc_path_append(scratch.arena, out_dir, OC_STR8(h.name));
            bool ok = oc_sys_mkdirs(dir_path);
            oc_scratch_end(scratch);
            if(!ok)
            {
                err = MTAR_EWRITEFAIL;
                break;
            }
        }
        else if(h.type == MTAR_TREG)
        {
            oc_arena_scope scratch = oc_scratch_begin();
            char* buf = oc_arena_push(scratch.arena, h.size + 1);
            err = mtar_read_data(&tar, buf, h.size);
            if(err != MTAR_ESUCCESS)
            {
                oc_scratch_end(scratch);
                break;
            }

            oc_str8 path = oc_path_append(scratch.arena, out_dir, OC_STR8(h.name));
            bool ok = write_file_from_tar(path, &h, buf, h.size);
            oc_scratch_end(scratch);
            if(!ok)
            {
                err = MTAR_EWRITEFAIL;
                break;
            }
        }
        else
        {
            // skip unknown / unhandled file types
        }

        mtar_next(&tar);
        err = mtar_read_header(&tar, &h);
    }

    if(err == MTAR_ENULLRECORD)
    {
        err = MTAR_ESUCCESS;
    }

    return err;
}

bool tarball_extract(oc_str8 filepath, oc_str8 out_dir)
{
    bool result = true;
    oc_arena_scope scratch = oc_scratch_begin();

    oc_str8 filename = oc_path_slice_filename(filepath);
    oc_str8 gz_ext = OC_STR8(".gz");
    oc_str8 tar_path = { 0 };
    if(oc_str8_ends_with(filename, gz_ext))
    {
        oc_str8 tar_name = oc_str8_slice(filename, 0, filename.len - gz_ext.len);
        tar_path = oc_path_append(scratch.arena, out_dir, tar_name);
    }
    else
    {
        tar_path = oc_path_append(scratch.arena, out_dir, OC_STR8("temp.tar"));
    }

    char* tar_path_cstr = oc_str8_to_cstring(scratch.arena, tar_path);
    char* filepath_cstr = oc_str8_to_cstring(scratch.arena, filepath);
    FILE* tar_file = NULL;

    gzFile in_file = gzopen(filepath_cstr, "rb");
    if(!in_file)
    {
        result = false;
        goto cleanup;
    }

    if(gzbuffer(in_file, CHUNK_SIZE) == -1)
    {
        result = false;
        goto cleanup;
    }

    tar_file = fopen(tar_path_cstr, "wb+");
    if(!tar_file)
    {
        result = false;
        goto cleanup;
    }

    int err = gzip_decompress(in_file, tar_file);
    if(err != Z_OK)
    {
        result = false;
        goto cleanup;
    }

    err = untar(tar_file, out_dir);
    if(err != MTAR_ESUCCESS)
    {
        result = false;
    }

cleanup:
    if(in_file)
    {
        gzclose(in_file);
    }
    if(tar_file)
    {
        fclose(tar_file);
        remove(tar_path_cstr);
    }
    oc_scratch_end(scratch);
    return result;
}
