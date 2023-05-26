/************************************************************//**
*
*	@file: win32_path.c
*	@author: Martin Fouilleul
*	@date: 24/05/2023
*
*****************************************************************/

#include"platform_path.c"

str8 path_executable(mem_arena* arena)
{
	//TODO use wide chars
	char* buffer = mem_arena_alloc_array(arena, char, MAX_PATH+1);
	int size = GetModuleFileName(NULL, buffer, MAX_PATH+1);
	//TODO: check for errors...
	for(int i=0; i<size; i++)
	{
		if(buffer[i] == '\\')
		{
			buffer[i] = '/';
		}
	}

	return(str8_from_buffer(size, buffer));
}

str8 path_canonical(mem_arena* arena, str8 path); //TODO
