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

	file_handle f = file_open(path, FILE_ACCESS_WRITE, FILE_OPEN_CREATE|FILE_OPEN_TRUNCATE);
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

	file_handle f = file_open(path, FILE_ACCESS_READ, 0);
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

	file_handle f = file_open(path, 0, 0);
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

	file_handle f = file_open(regular, 0, 0);
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

	f = file_open(dir, 0, 0);
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

	f = file_open(link, FILE_ACCESS_NONE, FILE_OPEN_SYMLINK);
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

int test_jail()
{
	log_info("test jail\n");

	file_handle jail = file_open(STR8("./data/jail"), FILE_ACCESS_READ, 0);
	if(file_last_error(jail))
	{
		log_error("Can't open jail directory\n");
		return(-1);
	}

	// Check escapes
	file_handle f = file_open_at(jail, STR8(".."), FILE_ACCESS_READ, FILE_OPEN_RESTRICT);
	if(file_last_error(f) != IO_ERR_WALKOUT)
	{
		log_error("Escaped jail with relative path ..\n");
		return(-1);
	}
	file_close(f);

	f = file_open_at(jail, STR8(".."), FILE_ACCESS_READ, FILE_OPEN_RESTRICT);
	if(file_last_error(f) != IO_ERR_WALKOUT)
	{
		log_error("Escaped jail with relative path dir1/../..\n");
		return(-1);
	}
	file_close(f);

	f = file_open_at(jail, STR8("/escape"), FILE_ACCESS_READ, FILE_OPEN_RESTRICT);
	if(file_last_error(f) != IO_ERR_WALKOUT)
	{
		log_error("Escaped jail with symlink\n");
		return(-1);
	}
	file_close(f);

	// Check legitimates open
	f = file_open_at(jail, STR8("/test.txt"), FILE_ACCESS_READ, FILE_OPEN_RESTRICT);
	if(file_last_error(f) != IO_OK)
	{
		log_error("Can't open jail/test.txt\n");
		return(-1);
	}
	file_close(f);

	f = file_open_at(jail, STR8("/dir1/../test.txt"), FILE_ACCESS_READ, FILE_OPEN_RESTRICT);
	if(file_last_error(f) != IO_OK)
	{
		log_error("Can't open jail/dir1/../test.txt\n");
		return(-1);
	}
	file_close(f);


	return(0);
}

int test_rights(mem_arena* arena, str8 dirPath)
{
	log_info("test rights\n");

	//--------------------------------------------------------------------------------------
	// base dir with no access
	//--------------------------------------------------------------------------------------
	{
		file_handle dir = file_open(dirPath, FILE_ACCESS_NONE, 0);
		if(file_last_error(dir))
		{
			log_error("Couldn't open ./data with no access rights\n");
			return(-1);
		}

		file_handle f = file_open_at(dir, STR8("./regular.txt"), FILE_ACCESS_READ, 0);
		if(file_last_error(f) != IO_ERR_PERM)
		{
			log_error("Incorrect check when opening file with read access in dir with no access\n");
			return(-1);
		}
		file_close(f);
		file_close(dir);
	}
	//--------------------------------------------------------------------------------------
	// base dir with read access
	//--------------------------------------------------------------------------------------
	{
		file_handle dir = file_open(dirPath, FILE_ACCESS_READ, 0);
		if(file_last_error(dir))
		{
			log_error("Couldn't open ./data with read rights\n");
			return(-1);
		}

		// check that we _can't_ open a file with write access
		file_handle f = file_open_at(dir, STR8("./regular.txt"), FILE_ACCESS_WRITE, 0);
		if(file_last_error(f) != IO_ERR_PERM)
		{
			log_error("Incorrect check when opening file with write access in dir with read access\n");
			return(-1);
		}
		file_close(f);

		// check that we _can_ open a file with read access
		f = file_open_at(dir, STR8("./regular.txt"), FILE_ACCESS_READ, 0);
		if(file_last_error(f))
		{
			log_error("Couldn't open file with read access in dir with read access\n");
			return(-1);
		}

		// check that we _can't_ write to that file
		str8 testWrite = STR8("Hello, world!\n");
		if(file_write(f, testWrite.len, testWrite.ptr) != 0)
		{
			log_error("Incorrectly wrote to read-only file\n");
			return(-1);
		}
		if(file_last_error(f) != IO_ERR_PERM)
		{
			log_error("Incorrect error returned from writing to read-only file\n");
			return(-1);
		}

		file_close(f);
		file_close(dir);
	}
	//--------------------------------------------------------------------------------------
	// base dir with write access
	//--------------------------------------------------------------------------------------
	{
		file_handle dir = file_open(dirPath, FILE_ACCESS_WRITE, 0);
		if(file_last_error(dir))
		{
			log_error("Couldn't open ./data with write rights\n");
			return(-1);
		}

		// check that we _can't_ open a file with read access
		file_handle f = file_open_at(dir, STR8("./regular.txt"), FILE_ACCESS_READ, 0);
		if(file_last_error(f) != IO_ERR_PERM)
		{
			log_error("Incorrect check when opening file with read access in dir with write access\n");
			return(-1);
		}
		file_close(f);

		// check that we _can_ open a file with write access
		f = file_open_at(dir, STR8("./regular.txt"), FILE_ACCESS_WRITE, 0);
		if(file_last_error(f))
		{
			log_error("Couldn't open file with write access in dir with write access\n");
			return(-1);
		}

		// check that we _can't_ read that file
		char testRead[512];
		if(file_read(f, 512, testRead) != 0)
		{
			log_error("Incorrectly read write-only file\n");
			return(-1);
		}
		if(file_last_error(f) != IO_ERR_PERM)
		{
			log_error("Incorrect error returned from reading write-only file\n");
			return(-1);
		}

		file_close(f);
		file_close(dir);
	}
	//--------------------------------------------------------------------------------------
	// base dir with read/write access
	//--------------------------------------------------------------------------------------
	{
		file_handle dir = file_open(dirPath, FILE_ACCESS_READ|FILE_ACCESS_WRITE, 0);
		if(file_last_error(dir))
		{
			log_error("Couldn't open ./data with read rights\n");
			return(-1);
		}

		// check that we can open file with read access
		file_handle f = file_open_at(dir, STR8("./regular.txt"), FILE_ACCESS_READ, 0);
		if(file_last_error(f))
		{
			log_error("Incorrect check when opening file with read access in dir with read/write access\n");
			return(-1);
		}
		file_close(f);

		// check that we can open file with write access
		f = file_open_at(dir, STR8("./regular.txt"), FILE_ACCESS_WRITE, 0);
		if(file_last_error(f))
		{
			log_error("Couldn't open file with write access in dir with read/write access\n");
			return(-1);
		}

		file_close(f);
		file_close(dir);
	}
	return(0);
}

int main(int argc, char** argv)
{
	mp_init();

	mem_arena* arena = mem_scratch();

	str8 dataDir = STR8("./data");
	str8 path = STR8("./test.txt");

	str8 test_string = STR8("Hello, world!");

	if(test_write(arena, path, test_string)) { return(-1); }
	if(test_read(arena, path, test_string)) { return(-1); }
	if(test_stat_size(path, test_string.len)) { return(-1); }
	if(test_stat_type(arena, dataDir)) { return(-1); }
	if(test_rights(arena, dataDir)) { return(-1); }
	if(test_jail()) { return(-1); }

	remove("./test.txt");

	log_info("OK\n");

	return(0);
}
