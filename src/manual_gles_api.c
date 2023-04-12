
const void* glShaderSource_stub(IM3Runtime runtime, IM3ImportContext _ctx, uint64_t * _sp, void * _mem)
{
	i32 shader = *(i32*)&_sp[0];
	i32 count = *(i32*)&_sp[1];
	i32 stringArrayOffset = *(i32*)&_sp[2];
	i32 lengthArrayOffset = *(i32*)&_sp[3];

	int* stringOffsetArray = (int*)((char*)_mem + stringArrayOffset);
	const char** stringArray = (const char**)mem_arena_alloc_array(mem_scratch(), char*, count);
	for(int i=0; i<count; i++)
	{
		stringArray[i] = (char*)_mem + stringOffsetArray[i];
	}

	int* lengthArray = lengthArrayOffset ? (int*)((char*)_mem + lengthArrayOffset) : 0;

	glShaderSource(shader, count, stringArray, lengthArray);
	return(0);
}

int manual_link_gles_api(IM3Module module)
{
	M3Result res;
	res = m3_LinkRawFunction(module, "*", "glShaderSource", "v(iiii)", glShaderSource_stub);
	if(res) { LOG_ERROR("error: %s\n", res); return(-1); }

	return(0);
}
