/************************************************************//**
*
*	@file: io.h
*	@author: Martin Fouilleul
*	@date: 09/05/2023
*
*****************************************************************/
#ifndef __IO_H_
#define __IO_H_

#include"util/typedefs.h"
#include"util/strings.h"

#include"io_common.h"

file_handle file_open(str8 path, file_open_flags flags);
void file_close(file_handle file);

size_t file_size(file_handle file);
size_t file_pos(file_handle file);
size_t file_seek(file_handle file, long offset, file_whence whence);

size_t file_write(file_handle file, size_t size, char* buffer);
size_t file_read(file_handle file, size_t size, char* buffer);

int file_error(file_handle file);

#endif //__IO_H_
