/************************************************************//**
*
*	@file: main.c
*	@author: Martin Fouilleul
*	@date: 26/05/2023
*
*****************************************************************/

#include<stdio.h>
#include"milepost.h"

int test_write(mem_arena* arena, str8 path, str8 test_string)
{
	log_info("writing\n");

	file_handle f = file_open(path, FILE_OPEN_CREATE|FILE_OPEN_WRITE|FILE_OPEN_TRUNCATE);
	if(file_last_error(f))
	{
		log_error("Can't create/open file %.*s for writing\n", (int)path.len, path.ptr);
		return(-1);
	}

	file_write(f, test_string.len, test_string.ptr);
	if(file_last_error(f))
	{
		log_error("Error while writing %.*s\n", (int)path.len, path.ptr);
		return(-1);
	}
	file_close(f);

	// check
	char* pathCStr = str8_to_cstring(arena, path);
	FILE* file = fopen(pathCStr, "r");
	if(!file)
	{
		log_error("File %.*s not found while checking\n", (int)path.len, path.ptr);
		return(-1);
	}
	char buffer[256];
	int n = fread(buffer, 1, 256, file);
	if(n != test_string.len || strncmp(buffer, test_string.ptr, test_string.len))
	{
		log_error("Didn't recover test string\n");
		return(-1);
	}
	fclose(file);

	return(0);
}

int test_read(mem_arena* arena, str8 path, str8 test_string)
{
	log_info("reading\n");

	file_handle f = file_open(path, FILE_OPEN_READ);
	if(file_last_error(f))
	{
		log_error("Can't open file %.*s for reading\n", (int)path.len, path.ptr);
		return(-1);
	}

	char buffer[256];
	i64 n = file_read(f, 256, buffer);
	if(file_last_error(f))
	{
		log_error("Error while reading %.*s\n", (int)path.len, path.ptr);
		return(-1);
	}
	file_close(f);

	if(str8_cmp(test_string, str8_from_buffer(n, buffer)))
	{
		log_error("Didn't recover test string\n");
		return(-1);
	}

	return(0);
}

int test_stat_size(str8 path, u64 size)
{
	log_info("stat size\n");

	file_handle f = file_open(path, 0);
	if(file_last_error(f))
	{
		log_error("Can't open file\n");
		return(-1);
	}

	file_status status = file_get_status(f);

	if(file_last_error(f))
	{
		log_error("Error while retrieving file status\n");
		return(-1);
	}

	if(status.size != size)
	{
		log_error("size doesn't match\n");
		return(-1);

	}

	return(0);
}

int test_stat_type(mem_arena* arena, str8 dataDir)
{
	str8 regular = path_append(arena, dataDir, STR8("regular.txt"));
	str8 dir = path_append(arena, dataDir, STR8("directory"));

	#if PLATFORM_WINDOWS
		str8 link = path_append(arena, dataDir, STR8("win32_symlink"));
	#else
		str8 link = path_append(arena, dataDir, STR8("posix_symlink"));
	#endif

	log_info("stat type, regular\n");

	file_handle f = file_open(regular, 0);
	if(file_last_error(f))
	{
		log_error("Can't open file\n");
		return(-1);
	}

	file_status status = file_get_status(f);
	if(file_last_error(f))
	{
		log_error("Error while retrieving file status\n");
		return(-1);
	}
	if(status.type != MP_FILE_REGULAR)
	{
		log_error("file type doesn't match\n");
		return(-1);
	}
	file_close(f);

	log_info("stat type, directory\n");

	f = file_open(dir, 0);
	if(file_last_error(f))
	{
		log_error("Can't open file\n");
		return(-1);
	}

	status = file_get_status(f);
	if(file_last_error(f))
	{
		log_error("Error while retrieving file status\n");
		return(-1);
	}
	if(status.type != MP_FILE_DIRECTORY)
	{
		log_error("file type doesn't match\n");
		return(-1);
	}
	file_close(f);

	log_info("stat type, symlink\n");

	f = file_open(link, FILE_OPEN_SYMLINK);
	if(file_last_error(f))
	{
		log_error("Can't open file\n");
		return(-1);
	}

	status = file_get_status(f);
	if(file_last_error(f))
	{
		log_error("Error while retrieving file status\n");
		return(-1);
	}
	if(status.type != MP_FILE_SYMLINK)
	{
		log_error("file type doesn't match\n");
		return(-1);
	}
	file_close(f);

	return(0);
}

int main(int argc, char** argv)
{
	mem_arena* arena = mem_scratch();

	str8 dataDir = STR8("./data");
	str8 path = STR8("./test.txt");

	str8 test_string = STR8("Hello, world!");

	if(test_write(arena, path, test_string)) { return(-1); }
	if(test_read(arena, path, test_string)) { return(-1); }
	if(test_stat_size(path, test_string.len)) { return(-1); }
	if(test_stat_type(arena, dataDir)) { return(-1); }

	log_info("OK\n");

	return(0);
}
