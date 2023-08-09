/************************************************************//**
*
*	@file: main.c
*	@author: Martin Fouilleul
*	@date: 26/05/2023
*
*****************************************************************/

#include<stdio.h>
#include"milepost.h"

int test_write()
{
	log_info("writing\n");

	mem_arena* arena = mem_scratch();

	str8 path = STR8("./data/write_test.txt");
	str8 test_string = STR8("Hello from write_test.txt");

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

int check_string(file_handle f, str8 test_string)
{
	char buffer[256];
	i64 n = file_read(f, 256, buffer);
	if(file_last_error(f))
	{
		log_error("Error while reading test string\n");
		return(-1);
	}

	if(str8_cmp(test_string, str8_from_buffer(n, buffer)))
	{
		return(-1);
	}

	return(0);
}

int test_read()
{
	log_info("reading\n");

	str8 path = STR8("./data/regular.txt");
	str8 test_string = STR8("Hello from regular.txt");

	file_handle f = file_open(path, FILE_ACCESS_READ, 0);
	if(file_last_error(f))
	{
		log_error("Can't open file %.*s for reading\n", (int)path.len, path.ptr);
		return(-1);
	}

	if(check_string(f, test_string))
	{
		log_error("Check string failed\n");
		return(-1);
	}

	file_close(f);
	return(0);
}

int test_stat_size()
{
	log_info("stat size\n");

	str8 path = STR8("./data/regular.txt");
	str8 test_string = STR8("Hello from regular.txt");
	u64 size = test_string.len;

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

int test_stat_type()
{
	str8 regular = STR8("./data/regular.txt");
	str8 dir = STR8("./data/directory");
	str8 link = STR8("./data/symlink");

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

int test_symlinks()
{
	// open symlink target
	log_info("open symlink target\n");
	file_handle f = file_open_at(file_handle_nil(), STR8("./data/symlink"), FILE_ACCESS_READ, 0);
	if(file_last_error(f))
	{
		log_error("failed to open ./data/symlink\n");
		return(-1);
	}
	if(check_string(f, STR8("Hello from regular.txt")))
	{
		log_error("Check string failed\n");
		return(-1);
	}
	file_close(f);

	// open symlink file
	log_info("open symlink file\n");
	f = file_open_at(file_handle_nil(), STR8("./data/symlink"), FILE_ACCESS_READ, FILE_OPEN_SYMLINK);
	if(file_last_error(f))
	{
		log_error("failed to open ./data/symlink\n");
		return(-1);
	}
	file_status status = file_get_status(f);
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

	char buffer[512];
	int n = file_read(f, 512, buffer);

	if(n || (file_last_error(f) == IO_OK))
	{
		log_error("file read should fail on symlinks\n");
		return(-1);
	}

	file_close(f);


	return(0);
}

int test_args()
{
	//NOTE: nil handle
	log_info("check open_at with nil handle\n");
	file_handle f = file_open_at(file_handle_nil(), STR8("./data/regular.txt"), FILE_ACCESS_READ, 0);
	if(file_last_error(f))
	{
		log_error("file_open_at() with nil handle failed\n");
		return(-1);
	}
	if(check_string(f, STR8("Hello from regular.txt")))
	{
		log_error("Check string failed\n");
		return(-1);
	}
	file_close(f);

	//NOTE: invalid handle
	log_info("check open_at with nil handle\n");
	file_handle wrongHandle = {.h = 123456789 };

	f = file_open_at(wrongHandle, STR8("./data/regular.txt"), FILE_ACCESS_READ, 0);
	if(file_last_error(f) != IO_ERR_HANDLE)
	{
		log_error("file_open_at() with non-nil invalid handle should return IO_ERR_HANDLE\n");
		return(-1);
	}
	file_close(f);

	//NOTE: nil/wrong handle and FILE_OPEN_RESTRICT
	log_info("check open_at with nil handle and FILE_OPEN_RESTRICT\n");

	f = file_open_at(file_handle_nil(), STR8("./data/regular.txt"), FILE_ACCESS_READ, FILE_OPEN_RESTRICT);
	if(file_last_error(f) != IO_ERR_HANDLE)
	{
		log_error("file_open_at() with nil handle and FILE_OPEN_RESTRICT should return IO_ERR_HANDLE\n");
		return(-1);
	}
	file_close(f);

	f = file_open_at(wrongHandle, STR8("./data/regular.txt"), FILE_ACCESS_READ, FILE_OPEN_RESTRICT);
	if(file_last_error(f) != IO_ERR_HANDLE)
	{
		log_error("file_open_at() with invalid handle and FILE_OPEN_RESTRICT should return IO_ERR_HANDLE\n");
		return(-1);
	}
	file_close(f);

	//NOTE: empty path
	log_info("check empty path\n");

	f = file_open_at(file_handle_nil(), STR8(""), FILE_ACCESS_READ, 0);
	if(file_last_error(f) != IO_ERR_ARG)
	{
		log_error("empty path should return IO_ERR_ARG\n");
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

	//-----------------------------------------------------------
	//NOTE: Check escapes
	//-----------------------------------------------------------
	log_info("check potential escapes\n");

	//NOTE: escape with absolute path
	file_handle f = file_open_at(jail, STR8("/tmp"), FILE_ACCESS_READ, FILE_OPEN_RESTRICT);
	if(file_last_error(f) != IO_ERR_NO_ENTRY)
	{
		log_error("Escaped jail with absolute path /tmp\n");
		return(-1);
	}
	file_close(f);

	//NOTE: escape with ..
	f = file_open_at(jail, STR8(".."), FILE_ACCESS_READ, FILE_OPEN_RESTRICT);
	if(file_last_error(f) != IO_ERR_WALKOUT)
	{
		log_error("Escaped jail with relative path ..\n");
		return(-1);
	}
	file_close(f);

	//NOTE: escape with dir/../..
	f = file_open_at(jail, STR8("dir/../.."), FILE_ACCESS_READ, FILE_OPEN_RESTRICT);
	if(file_last_error(f) != IO_ERR_WALKOUT)
	{
		log_error("Escaped jail with relative path dir/../..\n");
		return(-1);
	}
	file_close(f);

	//NOTE: escape with symlink to parent
	f = file_open_at(jail, STR8("/dir_escape"), FILE_ACCESS_READ, FILE_OPEN_RESTRICT);
	if(file_last_error(f) != IO_ERR_WALKOUT)
	{
		log_error("Escaped jail with symlink to parent\n");
		return(-1);
	}
	file_close(f);

	//NOTE: escape to file with symlink to parent
	f = file_open_at(jail, STR8("/dir_escape/regular.txt"), FILE_ACCESS_READ, FILE_OPEN_RESTRICT);
	if(file_last_error(f) != IO_ERR_WALKOUT)
	{
		log_error("Escaped jail to regular.txt with symlink to parent\n");
		return(-1);
	}
	file_close(f);

	//NOTE: escape with symlink to file
	f = file_open_at(jail, STR8("/file_escape"), FILE_ACCESS_READ, FILE_OPEN_RESTRICT);
	if(file_last_error(f) != IO_ERR_WALKOUT)
	{
		log_error("Escaped jail with symlink to file regular.txt\n");
		return(-1);
	}
	file_close(f);

	//NOTE: escape with bad root handle
	file_handle wrong_handle = {0};
	f = file_open_at(wrong_handle, STR8("./data/regular.txt"), FILE_ACCESS_READ, FILE_OPEN_RESTRICT);
	if(file_last_error(f) == IO_OK)
	{
		log_error("Escaped jail with nil root handle\n");
		return(-1);
	}
	if(file_last_error(f) != IO_ERR_HANDLE)
	{
		log_error("FILE_OPEN_RESTRICT with invalid root handle should return IO_ERR_HANDLE\n");
		return(-1);
	}
	file_close(f);

	//-----------------------------------------------------------
	//NOTE: empty path
	//-----------------------------------------------------------
	log_info("check empty path\n");

	f = file_open_at(jail, STR8(""), FILE_ACCESS_READ, 0);
	if(file_last_error(f) != IO_ERR_ARG)
	{
		log_error("empty path should return IO_ERR_ARG\n");
		return(-1);
	}
	file_close(f);

	//-----------------------------------------------------------
	//NOTE: Check legitimates open
	//-----------------------------------------------------------
	log_info("check legitimates open\n");

	//NOTE: regular file jail/test.txt
	f = file_open_at(jail, STR8("/test.txt"), FILE_ACCESS_READ, FILE_OPEN_RESTRICT);
	if(file_last_error(f) != IO_OK)
	{
		log_error("Can't open jail/test.txt\n");
		return(-1);
	}
	if(check_string(f, STR8("Hello from jail/test.txt")))
	{
		log_error("Check string failed\n");
		return(-1);
	}
	file_close(f);

	//NOTE: valid file traversal to jail/test.txt
	f = file_open_at(jail, STR8("/dir/../test.txt"), FILE_ACCESS_READ, FILE_OPEN_RESTRICT);
	if(file_last_error(f) != IO_OK)
	{
		log_error("Can't open jail/dir/../test.txt\n");
		return(-1);
	}
	if(check_string(f, STR8("Hello from jail/test.txt")))
	{
		log_error("Check string failed\n");
		return(-1);
	}
	file_close(f);

	//NOTE: re-open root directory
	f = file_open_at(jail, STR8("."), FILE_ACCESS_READ, FILE_OPEN_RESTRICT);
	if(file_last_error(f) != IO_OK)
	{
		log_error("Can't open jail/.\n");
		return(-1);
	}
	{
		//NOTE: access regular file test.txt inside reopened root
		file_handle f2 = file_open_at(f, STR8("test.txt"), FILE_ACCESS_READ, 0);

		if(check_string(f2, STR8("Hello from jail/test.txt")))
		{
			log_error("Check string failed\n");
			return(-1);
		}
		file_close(f2);
	}
	file_close(f);



	return(0);
}

int test_rights()
{
	log_info("test rights\n");

	str8 dirPath = STR8("./data");

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

	if(test_write()) { return(-1); }
	if(test_read()) { return(-1); }
	if(test_stat_size()) { return(-1); }
	if(test_stat_type()) { return(-1); }
	if(test_args()) { return(-1); }
	if(test_symlinks()) { return(-1); }
	if(test_rights()) { return(-1); }
	if(test_jail()) { return(-1); }

	remove("./data/write_test.txt");

	log_info("OK\n");

	return(0);
}
