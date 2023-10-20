/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include <orca.h>

#include <stdio.h>
#include <errno.h>

const oc_str8 REGULAR_TXT_CONTENTS = OC_STR8("Hello from regular.txt");

oc_file oc_file_from_filep(FILE* f)
{
    unsigned long long handle = *(unsigned long long*)f;
    return (oc_file){ .h = handle };
}

void force_close_file_no_flush(FILE* file)
{
    oc_file_close(oc_file_from_filep(file));
    free(file);
}

int check_string(FILE* f, oc_str8 test_string)
{
    char buffer[256];
    size_t n = fread(buffer, 1, 256, f);
    if(ferror(f))
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

int test_read(void)
{
    {
        oc_str8 path = OC_STR8("regular.txt");
        FILE* f = fopen(path.ptr, "r");
        if(f == NULL || ferror(f))
        {
            oc_log_error("Can't open file %.*s for reading\n", (int)path.len, path.ptr);
            return (-1);
        }

        if(check_string(f, REGULAR_TXT_CONTENTS))
        {
            oc_log_error("Check string failed\n");
            return (-1);
        }

        fclose(f);
    }

    {
        oc_str8 path = OC_STR8("directory/test.txt");
        oc_str8 test_string = OC_STR8("Hello from directory/test.txt");

        FILE* f = fopen(path.ptr, "r");
        if(f == NULL || ferror(f))
        {
            oc_log_error("Can't open file %.*s for reading\n", (int)path.len, path.ptr);
            return (-1);
        }

        if(check_string(f, test_string))
        {
            oc_log_error("Check string failed\n");
            return (-1);
        }

        fclose(f);
    }

    {
        FILE* f = fopen("does_not_exist.txt", "r");
        if(f != NULL)
        {
            oc_log_error("Somehow opened a file that doesn't exist\n");
            return (-1);
        }
    }

    return (0);
}

int test_write(void)
{
    oc_arena_scope scratch = oc_scratch_begin();
    oc_arena* arena = scratch.arena;

    oc_str8 path = OC_STR8("write_test.txt");
    oc_str8 test_string = OC_STR8("Hello from write_test.txt");

    FILE* f = fopen(path.ptr, "w");
    if(ferror(f))
    {
        oc_log_error("Can't create/open file %.*s for writing\n", (int)path.len, path.ptr);
        return (-1);
    }

    size_t written = fwrite(test_string.ptr, 1, test_string.len, f);
    if(ferror(f))
    {
        oc_log_error("Error while writing %.*s\n", (int)path.len, path.ptr);
        return (-1);
    }
    if(written != test_string.len)
    {
        oc_log_error("Failed to write the entire string to file. written: %zu, expected: %zu", written, test_string.len);
        return (-1);
    }
    fclose(f);

    char* pathCStr = oc_str8_to_cstring(arena, path);
    FILE* file = fopen(pathCStr, "r");
    if(!file)
    {
        oc_log_error("File %.*s not found while checking\n", (int)path.len, path.ptr);
        return (-1);
    }
    char buffer[256];
    size_t n = fread(buffer, 1, 256, file);
    if(n != test_string.len || strncmp(buffer, test_string.ptr, test_string.len))
    {
        oc_log_error("Didn't recover test string\n");
        return (-1);
    }
    fclose(file);

    return (0);
}

int test_error(void)
{
    {
        FILE* f = fopen("regular.txt", "r");
        oc_str8 test_string = OC_STR8("this shouldn't get written since the file is in read mode");
        size_t written = fwrite(test_string.ptr, 1, test_string.len, f);
        if(!ferror(f))
        {
            oc_log_error("File should be in error state");
            return (-1);
        }
        if(written > 0)
        {
            oc_log_error("Wrote %d bytes but shouldn't have written any.", (int)written);
            return (-1);
        }

        clearerr(f);
        if(ferror(f))
        {
            oc_log_error("File error state should be cleared");
            return (-1);
        }
    }

    {
        FILE* f = fopen("error_test.txt", "w");

        char buffer[256];
        size_t n = fread(buffer, 1, sizeof(buffer), f);

        if(!ferror(f))
        {
            oc_log_error("File should be in error state");
            return (-1);
        }

        clearerr(f);
        if(ferror(f))
        {
            oc_log_error("File error state should be cleared");
            return (-1);
        }
    }

    return (0);
}

int test_eof(void)
{
    FILE* f = fopen("regular.txt", "r");
    char buffer[1024];
    size_t num_read = fread(buffer, 1, sizeof(buffer), f);
    if(num_read == 0)
    {
        oc_log_error("Should have read at least some data");
        return (-1);
    }

    if(!feof(f))
    {
        oc_log_error("Should be at end of file by now");
        return (-1);
    }

    char data = 0;
    num_read = fread(&data, 1, 1, f);
    if(num_read != 0)
    {
        oc_log_error("Should be at end of file - no data should be read");
        return (-1);
    }

    return (0);
}

int test_getputc(void)
{
    const oc_str8 filename = OC_STR8("putc_test.txt");
    const oc_str8 test_file_contents = OC_STR8("The quick brown fox jumped over the lazy dog!@#$%^&*()\n");

    {
        FILE* f = fopen(filename.ptr, "w");
        for(int i = 0; i < test_file_contents.len; ++i)
        {
            int c = test_file_contents.ptr[i];
            if(putc(c, f) != c)
            {
                oc_log_error("Failed to put character to file");
                return (-1);
            }
            if(ferror(f))
            {
                oc_log_error("Caught error putting character to file");
                return (-1);
            }
        }
        fclose(f);
    }

    {
        FILE* f = fopen(filename.ptr, "r");
        char buffer[256];
        int total_read = 0;
        for(int i = 0; i < sizeof(buffer) && !feof(f); ++i)
        {
            int character = getc(f);
            if(character == EOF)
            {
                break;
            }
            if(ferror(f))
            {
                oc_log_error("Failed to read character");
                return (-1);
            }
            buffer[i] = (char)character;

            total_read = i + 1;
        }

        if(ferror(f) && !feof(f))
        {
            oc_log_error("File in error state");
            return (-1);
        }

        if(oc_str8_cmp(test_file_contents, oc_str8_from_buffer(total_read, buffer)))
        {
            oc_log_error("Failed to read correct file contents, got: '%.*s'", (int)total_read, buffer);
            return (-1);
        }
    }

    return (0);
}

int test_getputs(void)
{
    const oc_str8 filename = OC_STR8("getputs_test.txt");
    const oc_str8 test_file_contents = OC_STR8("hello from getputs_test\n");

    {
        FILE* f = fopen("getputs_test.txt", "w");
        if(fputs(test_file_contents.ptr, f))
        {
            oc_log_error("Failed to fputs");
            return (-1);
        }
        fclose(f);
    }

    {
        FILE* f = fopen("getputs_test.txt", "r");
        char buffer[256];
        if(fgets(buffer, sizeof(buffer), f) == NULL)
        {
            oc_log_error("Failed to fgets");
            return (-1);
        }
        oc_str8 read_contents = OC_STR8(buffer);
        if(oc_str8_cmp(test_file_contents, read_contents))
        {
            oc_log_error("Didn't read expected output from file, got: %s", buffer);
            return (-1);
        }
        fclose(f);
    }

    return (0);
}

int test_printf_scanf(void)
{
    const oc_str8 filename = OC_STR8("printf_scanf_test.txt");
    const oc_str8 format_string = OC_STR8("long int test: %llu\none more: %d %u");
    const long long unsigned value_lld = -1;
    const int value_d = 424242;
    const unsigned value_u = 0xBEEFBEEF;

    {
        FILE* f = fopen(filename.ptr, "w");
        if(f == NULL)
        {
            oc_log_error("Failed to open file");
            return (-1);
        }

        if(fprintf(f, format_string.ptr, value_lld, value_d, value_u) < 0)
        {
            oc_log_error("Failed to fprintf");
            return (-1);
        }

        fclose(f);
    }

    {
        FILE* f = fopen(filename.ptr, "r");
        if(f == NULL)
        {
            oc_log_error("Failed to open file");
            return (-1);
        }

        long long unsigned read_value_lld;
        int read_value_d;
        unsigned read_value_u;

        fscanf(f, format_string.ptr, &read_value_lld, &read_value_d, &read_value_u);
        if(ferror(f))
        {
            oc_log_error("Caught error after fscanf");
            return (-1);
        }

        if(read_value_lld != value_lld || read_value_d != value_d || read_value_u != value_u)
        {
            oc_log_error("Read incorrect values: %lld, %d, %u", read_value_lld, read_value_d, read_value_u);
            return (-1);
        }

        fclose(f);
    }

    return (0);
}

int test_getsetpos(void)
{
    FILE* f = fopen("getsetpos_test.txt", "w+");
    if(f == NULL)
    {
        oc_log_error("failed to open getsetpos_test.txt");
        return (-1);
    }

    if(fputc('A', f) != 'A')
    {
        oc_log_error("failed to fputc");
        return (-1);
    }

    fpos_t pos;
    if(fgetpos(f, &pos))
    {
        oc_log_error("fgetpos failed");
        return (-1);
    }

    fputc('B', f);

    if(fsetpos(f, &pos))
    {
        oc_log_error("fsetpos failed");
        return (-1);
    }

    for(int i = 'C'; i <= 'Z'; ++i)
    {
        fputc((char)i, f);
    }

    if(fsetpos(f, &pos))
    {
        oc_log_error("fsetpos failed");
        return (-1);
    }

    int character = fgetc(f);
    if(character == EOF)
    {
        oc_log_error("Got unexpected EOF");
        return (-1);
    }
    if(character != 'C')
    {
        oc_log_error("Failed to get 'C', got '%c'", (char)character);
        return (-1);
    }

    return (0);
}

int test_seek(void)
{
    const char* filename = "temp_big_file.bin";
    FILE* f = fopen(filename, "w+");
    if(ferror(f))
    {
        return (-1);
    }

    // should only write 256 bytes of zeroes to the file since we're resetting the head position every time
    const unsigned char MAX_LOOPS = 0xFF;
    for(unsigned char i = 0; i < MAX_LOOPS; ++i)
    {
        unsigned char empty_data[16];
        memset(empty_data, i + 1, sizeof(empty_data));

        if(i > 0)
        {
            int err = fseek(f, -((long int)sizeof(empty_data)), SEEK_CUR);
            if(err)
            {
                oc_log_error("Failed to SEEK_CUR\n");
                return (-1);
            }
        }

        size_t n = fwrite(empty_data, 1, sizeof(empty_data), f);
        if(ferror(f))
        {
            oc_log_error("Caught error writing to %s\n", filename);
            return (-1);
        }
        if(n != sizeof(empty_data))
        {
            oc_log_error("Failed to write all bytes to %s\n", filename);
            return (-1);
        }
    }

    fflush(f);
    if(ferror(f))
    {
        oc_log_error("fflush failed: %d\n", errno);
        return (-1);
    }

    {
        int err = fseek(f, 0, SEEK_SET);
        if(err)
        {
            oc_log_error("Failed to SEEK_SET\n");
            return (-1);
        }

        unsigned char written_data[16];
        size_t n = fread(&written_data, 1, sizeof(written_data), f);
        if(ferror(f))
        {
            oc_log_error("Caught error reading data: %d\n", errno);
            return (-1);
        }

        if(n != sizeof(written_data))
        {
            oc_log_error("Failed to read enough data\n");
            return (-1);
        }

        for(i32 i = 0; i < sizeof(written_data) / sizeof(*written_data); ++i)
        {
            if(written_data[i] != MAX_LOOPS)
            {
                oc_log_error("Read %d but expected %d\n", written_data[i], MAX_LOOPS);
                return (-1);
            }
        }
    }

    {
        int err = fseek(f, 0, SEEK_END);
        if(err)
        {
            oc_log_error("Failed to SEEK_END\n");
            return (-1);
        }
    }

    fclose(f);

    return (0);
}

int test_ftell(void)
{
    FILE* f = fopen("regular.txt", "r");
    int err = fseek(f, 0, SEEK_END);
    if(err)
    {
        oc_log_error("Failed to SEEK_END\n");
        return (-1);
    }

    int pos = ftell(f);
    if(pos <= 0)
    {
        oc_log_error("Failed to ftell");
        return (-1);
    }

    if(ferror(f))
    {
        oc_log_error("Caught error running ftell");
        return (-1);
    }

    return (0);
}

int test_rewind(void)
{
    FILE* f = fopen("regular.txt", "r");
    int err = fseek(f, 0, SEEK_END);
    if(err)
    {
        oc_log_error("Failed to SEEK_END\n");
        return (-1);
    }

    rewind(f);
    if(ferror(f))
    {
        oc_log_error("Caught error running rewind");
        return (-1);
    }

    int pos = ftell(f);
    if(pos != 0)
    {
        oc_log_error("rewind didn't work");
        return (-1);
    }

    return (0);
}

int test_freopen(void)
{
    {
        oc_str8 filename = OC_STR8("freopen_test1.txt");
        oc_str8 test_string = OC_STR8("hello world from a file");

        FILE* f = fopen(filename.ptr, "w");
        fprintf(f, "%s", test_string.ptr);

        if(ferror(f))
        {
            oc_log_error("failed writing to file");
            return (-1);
        }

        if(freopen(filename.ptr, "r", f) == NULL)
        {
            oc_log_error("freopen failed with err: %d, %s", errno, strerror(errno));
            return (-1);
        }

        if(check_string(f, test_string))
        {
            return (-1);
        }
        fclose(f);
    }

    {
        oc_str8 filename = OC_STR8("freopen_test2.txt");
        FILE* f = fopen(filename.ptr, "w");
        if(freopen(NULL, "r", f))
        {
            oc_log_error("orca shouldn't support reopening files without specifying the filename");
            return (-1);
        }
    }

    return (0);
}

int test_setbuf(void)
{
    {
        oc_str8 filename = OC_STR8("setbuf_test1.txt");
        oc_str8 test_string = OC_STR8("We shouldn't see this be written to disk because we'll close the file without flushing.");

        FILE* f = fopen(filename.ptr, "w");
        fwrite(test_string.ptr, 1, test_string.len, f);

        force_close_file_no_flush(f);

        f = fopen(filename.ptr, "r");
        fseek(f, 0, SEEK_END);
        long int pos = ftell(f);
        if(pos > 0)
        {
            oc_log_error("Shouldn't see any data written to disk, but got size: %d", (int)pos);
            return (-1);
        }

        // BUFSIZ ensures we don't see any data written to disk because it's exactly the size of the default buffer
        char test_bulk_data[BUFSIZ];
        memset(test_bulk_data, 'A', sizeof(test_bulk_data));
        f = fopen(filename.ptr, "w");
        fwrite(test_bulk_data, 1, sizeof(test_bulk_data), f);

        force_close_file_no_flush(f);

        f = fopen(filename.ptr, "r");
        fseek(f, 0, SEEK_END);
        pos = ftell(f);
        if(pos > 0)
        {
            oc_log_error("Shouldn't see any data written to disk, but got size: %d", (int)pos);
            return (-1);
        }
    }

    {
        oc_str8 filename = OC_STR8("setbuf_test2.txt");
        oc_str8 test_string = OC_STR8("We SHOULD see this be written to disk because even though we'll close the file without flushing, it has no buffering.");

        FILE* f = fopen(filename.ptr, "w");
        setbuf(f, NULL);
        fwrite(test_string.ptr, 1, test_string.len, f);
        force_close_file_no_flush(f);

        f = fopen(filename.ptr, "r");
        if(check_string(f, test_string))
        {
            oc_log_error("Unbuffered FILE should forward all writes immediately to disk");
            return (-1);
        }

        // BUFSIZ + 1 should trigger a write to disk
        char test_bulk_data[BUFSIZ + 1];
        memset(test_bulk_data, 'A', sizeof(test_bulk_data));
        f = fopen(filename.ptr, "w");
        fwrite(test_bulk_data, 1, sizeof(test_bulk_data), f);

        force_close_file_no_flush(f);

        f = fopen(filename.ptr, "r");
        fseek(f, 0, SEEK_END);
        long int pos = ftell(f);
        if(pos == 0)
        {
            oc_log_error("Should have flushed some data to disk");
            return (-1);
        }
    }

    {
        oc_str8 filename = OC_STR8("setbuf_test3.txt");
        oc_str8 test_string = OC_STR8("We SHOULD see at least some of this data be written to disk because even though we'll close the file without flushing, this string is longer than the buffer.");

        char file_buffer[32] = { 0 };

        FILE* f = fopen(filename.ptr, "w");
        setvbuf(f, file_buffer, _IOFBF, sizeof(file_buffer));

        // write to the file in chunks smaller than the buffer to ensure it gets used, and eventually flushed
        for(int pos = 0; pos < test_string.len;)
        {
            int chunk_size = 10;
            int chunk_size_clamped = (test_string.len > pos + chunk_size) ? chunk_size : (pos + chunk_size) - test_string.len;
            int written = (int)fwrite(test_string.ptr + written, 1, chunk_size_clamped, f);
            if(written <= 0)
            {
                oc_log_error("error writing to file");
                return (-1);
            }
            pos += written;
        }

        char zeroed_file_buffer[sizeof(file_buffer)] = { 0 };

        if(memcmp(file_buffer, zeroed_file_buffer, sizeof(file_buffer)) == 0)
        {
            oc_log_error("the file should have used our custom buffer");
            return (-1);
        }
        force_close_file_no_flush(f);

        f = fopen(filename.ptr, "r");
        fseek(f, 0, SEEK_END);
        long int pos = ftell(f);
        if(pos <= 0)
        {
            oc_log_error("Should see some data because of the short buffer that should get flushed when full");
            return (-1);
        }
    }

    return (0);
}

int test_ungetc(void)
{
    oc_str8 path = OC_STR8("regular.txt");
    FILE* f = fopen(path.ptr, "r");

    // Making some assumptions here that the first character is 'H' and the second character is 'e'
    int c1 = fgetc(f);
    int c2 = 'Y';

    if(ungetc(c2, f) == EOF)
    {
        oc_log_error("failed to ungetc");
        return (-1);
    }

    int c3 = fgetc(f);
    if(c3 != c2)
    {
        oc_log_error("expected to get char %c but got %c", c2, c3);
        return (-1);
    }

    return (0);
}

int test_std_handles(void)
{
    FILE* handles[] = { stdout, stderr, stdin };
    const char* redirect_mode[] = { "w", "w", "r" };
    const char* names[] = { "stdout", "stderr", "stdin" };
    for(int i = 0; i < 3; ++i)
    {
        FILE* f = handles[i];
        if(f != stdin)
        {
            if(fprintf(f, "printing this should result in an error") > 0 || !ferror(f))
            {
                oc_log_error("%s initial state should error on any operation", names[i]);
                return (-1);
            }
            clearerr(f);

            if(fputc('a', f) != EOF || !ferror(f))
            {
                oc_log_error("%s initial state should error on any operation", names[i]);
                return (-1);
            }
            clearerr(f);
        }
        else
        {
            char in_buffer[64];
            if(scanf("%s", in_buffer) > 0 || !ferror(f))
            {
                oc_log_error("%s initial state should error on any operation", names[i]);
                return (-1);
            }
            clearerr(f);

            if(getchar() != EOF || !ferror(f))
            {
                oc_log_error("%s initial state should error on any operation", names[i]);
                return (-1);
            }
            clearerr(f);
        }

        f = freopen("std_handles_test.txt", redirect_mode[i], f);
        if(f == NULL)
        {
            oc_log_error("failed reopening std handle %s", names[i]);
            return (-1);
        }

        // test_std_handles.txt gets written to first so reading from stdin should be fine
        oc_str8 test_string = OC_STR8("this_string_should_go_in_a_file.note_lack_of_whitespace");
        char test_char = '!';

        if(f != stdin)
        {

            if(fprintf(f, "%s\n", test_string.ptr) <= 0 || ferror(f))
            {
                oc_log_error("caught error printing data to %s after reopen", names[i]);
                return (-1);
            }

            if(fputc(test_char, f) == EOF || ferror(f))
            {
                oc_log_error("%s initial state should error on any operation", names[i]);
                return (-1);
            }

            fflush(f);
        }
        else
        {
            char in_buffer[64] = { 0 };
            if(scanf("%s\n", in_buffer) <= 0 || ferror(f))
            {
                oc_log_error("%s should be able to read from file. got: '%s'", names[i], in_buffer);
                return (-1);
            }

            if(oc_str8_cmp(test_string, oc_str8_from_buffer(strlen(in_buffer), in_buffer)))
            {
                oc_log_error("didn't read expected string from test file, got '%s' but expected '%s'", in_buffer, test_string.ptr);
                return (-1);
            }

            int read_char = getchar();
            if(read_char == EOF || ferror(f))
            {
                oc_log_error("caught error calling getchar() for handle %s. read_char: %d", names[i], read_char);
                return (-1);
            }
            if(read_char != test_char)
            {
                oc_log_error("%s didn't read expected char from file, got %c but expected %c", names[i], read_char, test_char);
                return (-1);
            }
        }
    }

    return (0);
}

int test_perror(void)
{
    oc_str8 filename = OC_STR8("perror_test.txt");
    {
        freopen(filename.ptr, "w", stderr);
        fflush(stderr);
        errno = ECANCELED;
        perror(NULL);
        errno = ECANCELED;
        perror("test msg");
        fflush(stderr);
    }

    {
        oc_str8 expected = OC_STR8("Operation canceled\ntest msg: Operation canceled\n");
        FILE* f = fopen(filename.ptr, "r");
        if(f == NULL)
        {
            oc_log_error("failed to open perror file");
            return (-1);
        }

        if(check_string(f, expected))
        {
            oc_log_error("didn't read expected string from perror file");
            return (-1);
        }
    }

    return (0);
}

int test_jail(void)
{
    FILE* f = fopen("../out_of_data_dir.txt", "w");
    if(f)
    {
        oc_log_error("Shouldn't be able to write to files outside the data dir");
        return (-1);
    }

    f = fopen("../wasm/module.wasm", "r");
    if(f)
    {
        oc_log_error("Shouldn't be able to read files outside the data dir");
        return (-1);
    }

    return (0);
}

ORCA_EXPORT i32 oc_on_test(void)
{
    if(test_read())
    {
        return (-1);
    }
    if(test_write())
    {
        return (-1);
    }
    if(test_error())
    {
        return (-1);
    }
    if(test_eof())
    {
        return (-1);
    }
    if(test_getputc())
    {
        return (-1);
    }
    if(test_getputs())
    {
        return (-1);
    }
    if(test_printf_scanf())
    {
        return (-1);
    }
    if(test_getsetpos())
    {
        return (-1);
    }
    if(test_seek())
    {
        return (-1);
    }
    if(test_ftell())
    {
        return (-1);
    }
    if(test_rewind())
    {
        return (-1);
    }
    if(test_freopen())
    {
        return (-1);
    }
    if(test_setbuf())
    {
        return (-1);
    }
    if(test_ungetc())
    {
        return (-1);
    }
    if(test_std_handles())
    {
        return (-1);
    }
    if(test_perror())
    {
        return (-1);
    }
    if(test_jail())
    {
        return (-1);
    }

    oc_log_info("OK\n");

    return (0);
}
