void glActiveTexture_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum texture = (GLenum)*(i32*)&_params[0];
	glActiveTexture(texture);
}

void glAttachShader_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLuint shader = (GLuint)*(i32*)&_params[1];
	glAttachShader(program, shader);
}

void glBindAttribLocation_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLuint index = (GLuint)*(i32*)&_params[1];
	const GLchar * name = (const GLchar *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)name >= (char*)_mem) && (((char*)name - (char*)_mem) < _memSize), "parameter 'name' is out of bounds");
		OC_ASSERT_DIALOG((char*)name + orca_glBindAttribLocation_name_length(instance, name)*sizeof(const GLchar ) <= ((char*)_mem + _memSize), "parameter 'name' is out of bounds");
	}
	glBindAttribLocation(program, index, name);
}

void glBindBuffer_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLuint buffer = (GLuint)*(i32*)&_params[1];
	glBindBuffer(target, buffer);
}

void glBindFramebuffer_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLuint framebuffer = (GLuint)*(i32*)&_params[1];
	glBindFramebuffer(target, framebuffer);
}

void glBindRenderbuffer_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLuint renderbuffer = (GLuint)*(i32*)&_params[1];
	glBindRenderbuffer(target, renderbuffer);
}

void glBindTexture_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLuint texture = (GLuint)*(i32*)&_params[1];
	glBindTexture(target, texture);
}

void glBlendColor_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLfloat red = (GLfloat)*(f32*)&_params[0];
	GLfloat green = (GLfloat)*(f32*)&_params[1];
	GLfloat blue = (GLfloat)*(f32*)&_params[2];
	GLfloat alpha = (GLfloat)*(f32*)&_params[3];
	glBlendColor(red, green, blue, alpha);
}

void glBlendEquation_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum mode = (GLenum)*(i32*)&_params[0];
	glBlendEquation(mode);
}

void glBlendEquationSeparate_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum modeRGB = (GLenum)*(i32*)&_params[0];
	GLenum modeAlpha = (GLenum)*(i32*)&_params[1];
	glBlendEquationSeparate(modeRGB, modeAlpha);
}

void glBlendFunc_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum sfactor = (GLenum)*(i32*)&_params[0];
	GLenum dfactor = (GLenum)*(i32*)&_params[1];
	glBlendFunc(sfactor, dfactor);
}

void glBlendFuncSeparate_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum sfactorRGB = (GLenum)*(i32*)&_params[0];
	GLenum dfactorRGB = (GLenum)*(i32*)&_params[1];
	GLenum sfactorAlpha = (GLenum)*(i32*)&_params[2];
	GLenum dfactorAlpha = (GLenum)*(i32*)&_params[3];
	glBlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
}

void glBufferData_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLsizeiptr size = (GLsizeiptr)*(i32*)&_params[1];
	const void * data = (const void *)((char*)_mem + *(u32*)&_params[2]);
	GLenum usage = (GLenum)*(i32*)&_params[3];
	{
		OC_ASSERT_DIALOG(((char*)data >= (char*)_mem) && (((char*)data - (char*)_mem) < _memSize), "parameter 'data' is out of bounds");
		OC_ASSERT_DIALOG((char*)data + size <= ((char*)_mem + _memSize), "parameter 'data' is out of bounds");
	}
	glBufferData(target, size, data, usage);
}

void glBufferSubData_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLintptr offset = (GLintptr)*(i32*)&_params[1];
	GLsizeiptr size = (GLsizeiptr)*(i32*)&_params[2];
	const void * data = (const void *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)data >= (char*)_mem) && (((char*)data - (char*)_mem) < _memSize), "parameter 'data' is out of bounds");
		OC_ASSERT_DIALOG((char*)data + size <= ((char*)_mem + _memSize), "parameter 'data' is out of bounds");
	}
	glBufferSubData(target, offset, size, data);
}

void glCheckFramebufferStatus_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glCheckFramebufferStatus(target);
}

void glClear_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLbitfield mask = (GLbitfield)*(i32*)&_params[0];
	glClear(mask);
}

void glClearColor_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLfloat red = (GLfloat)*(f32*)&_params[0];
	GLfloat green = (GLfloat)*(f32*)&_params[1];
	GLfloat blue = (GLfloat)*(f32*)&_params[2];
	GLfloat alpha = (GLfloat)*(f32*)&_params[3];
	glClearColor(red, green, blue, alpha);
}

void glClearDepthf_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLfloat d = (GLfloat)*(f32*)&_params[0];
	glClearDepthf(d);
}

void glClearStencil_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint s = (GLint)*(i32*)&_params[0];
	glClearStencil(s);
}

void glColorMask_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLboolean red = (GLboolean)*(i32*)&_params[0];
	GLboolean green = (GLboolean)*(i32*)&_params[1];
	GLboolean blue = (GLboolean)*(i32*)&_params[2];
	GLboolean alpha = (GLboolean)*(i32*)&_params[3];
	glColorMask(red, green, blue, alpha);
}

void glCompileShader_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint shader = (GLuint)*(i32*)&_params[0];
	glCompileShader(shader);
}

void glCompressedTexImage2D_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLint level = (GLint)*(i32*)&_params[1];
	GLenum internalformat = (GLenum)*(i32*)&_params[2];
	GLsizei width = (GLsizei)*(i32*)&_params[3];
	GLsizei height = (GLsizei)*(i32*)&_params[4];
	GLint border = (GLint)*(i32*)&_params[5];
	GLsizei imageSize = (GLsizei)*(i32*)&_params[6];
	const void * data = (const void *)((char*)_mem + *(u32*)&_params[7]);
	{
		OC_ASSERT_DIALOG(((char*)data >= (char*)_mem) && (((char*)data - (char*)_mem) < _memSize), "parameter 'data' is out of bounds");
		OC_ASSERT_DIALOG((char*)data + imageSize <= ((char*)_mem + _memSize), "parameter 'data' is out of bounds");
	}
	glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
}

void glCompressedTexSubImage2D_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLint level = (GLint)*(i32*)&_params[1];
	GLint xoffset = (GLint)*(i32*)&_params[2];
	GLint yoffset = (GLint)*(i32*)&_params[3];
	GLsizei width = (GLsizei)*(i32*)&_params[4];
	GLsizei height = (GLsizei)*(i32*)&_params[5];
	GLenum format = (GLenum)*(i32*)&_params[6];
	GLsizei imageSize = (GLsizei)*(i32*)&_params[7];
	const void * data = (const void *)((char*)_mem + *(u32*)&_params[8]);
	{
		OC_ASSERT_DIALOG(((char*)data >= (char*)_mem) && (((char*)data - (char*)_mem) < _memSize), "parameter 'data' is out of bounds");
		OC_ASSERT_DIALOG((char*)data + imageSize <= ((char*)_mem + _memSize), "parameter 'data' is out of bounds");
	}
	glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
}

void glCopyTexImage2D_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLint level = (GLint)*(i32*)&_params[1];
	GLenum internalformat = (GLenum)*(i32*)&_params[2];
	GLint x = (GLint)*(i32*)&_params[3];
	GLint y = (GLint)*(i32*)&_params[4];
	GLsizei width = (GLsizei)*(i32*)&_params[5];
	GLsizei height = (GLsizei)*(i32*)&_params[6];
	GLint border = (GLint)*(i32*)&_params[7];
	glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
}

void glCopyTexSubImage2D_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLint level = (GLint)*(i32*)&_params[1];
	GLint xoffset = (GLint)*(i32*)&_params[2];
	GLint yoffset = (GLint)*(i32*)&_params[3];
	GLint x = (GLint)*(i32*)&_params[4];
	GLint y = (GLint)*(i32*)&_params[5];
	GLsizei width = (GLsizei)*(i32*)&_params[6];
	GLsizei height = (GLsizei)*(i32*)&_params[7];
	glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
}

void glCreateProgram_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	*((i32*)&_returns[0]) = (i32)glCreateProgram();
}

void glCreateShader_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum type = (GLenum)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glCreateShader(type);
}

void glCullFace_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum mode = (GLenum)*(i32*)&_params[0];
	glCullFace(mode);
}

void glDeleteBuffers_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	const GLuint * buffers = (const GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)buffers >= (char*)_mem) && (((char*)buffers - (char*)_mem) < _memSize), "parameter 'buffers' is out of bounds");
		OC_ASSERT_DIALOG((char*)buffers + n*sizeof(const GLuint ) <= ((char*)_mem + _memSize), "parameter 'buffers' is out of bounds");
	}
	glDeleteBuffers(n, buffers);
}

void glDeleteFramebuffers_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	const GLuint * framebuffers = (const GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)framebuffers >= (char*)_mem) && (((char*)framebuffers - (char*)_mem) < _memSize), "parameter 'framebuffers' is out of bounds");
		OC_ASSERT_DIALOG((char*)framebuffers + n*sizeof(const GLuint ) <= ((char*)_mem + _memSize), "parameter 'framebuffers' is out of bounds");
	}
	glDeleteFramebuffers(n, framebuffers);
}

void glDeleteProgram_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	glDeleteProgram(program);
}

void glDeleteRenderbuffers_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	const GLuint * renderbuffers = (const GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)renderbuffers >= (char*)_mem) && (((char*)renderbuffers - (char*)_mem) < _memSize), "parameter 'renderbuffers' is out of bounds");
		OC_ASSERT_DIALOG((char*)renderbuffers + n*sizeof(const GLuint ) <= ((char*)_mem + _memSize), "parameter 'renderbuffers' is out of bounds");
	}
	glDeleteRenderbuffers(n, renderbuffers);
}

void glDeleteShader_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint shader = (GLuint)*(i32*)&_params[0];
	glDeleteShader(shader);
}

void glDeleteTextures_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	const GLuint * textures = (const GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)textures >= (char*)_mem) && (((char*)textures - (char*)_mem) < _memSize), "parameter 'textures' is out of bounds");
		OC_ASSERT_DIALOG((char*)textures + n*sizeof(const GLuint ) <= ((char*)_mem + _memSize), "parameter 'textures' is out of bounds");
	}
	glDeleteTextures(n, textures);
}

void glDepthFunc_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum func = (GLenum)*(i32*)&_params[0];
	glDepthFunc(func);
}

void glDepthMask_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLboolean flag = (GLboolean)*(i32*)&_params[0];
	glDepthMask(flag);
}

void glDepthRangef_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLfloat n = (GLfloat)*(f32*)&_params[0];
	GLfloat f = (GLfloat)*(f32*)&_params[1];
	glDepthRangef(n, f);
}

void glDetachShader_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLuint shader = (GLuint)*(i32*)&_params[1];
	glDetachShader(program, shader);
}

void glDisable_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum cap = (GLenum)*(i32*)&_params[0];
	glDisable(cap);
}

void glDisableVertexAttribArray_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint index = (GLuint)*(i32*)&_params[0];
	glDisableVertexAttribArray(index);
}

void glDrawArrays_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum mode = (GLenum)*(i32*)&_params[0];
	GLint first = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	glDrawArrays(mode, first, count);
}

void glDrawElements_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum mode = (GLenum)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	GLenum type = (GLenum)*(i32*)&_params[2];
	const void * indices = (const void *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)indices >= (char*)_mem) && (((char*)indices - (char*)_mem) < _memSize), "parameter 'indices' is out of bounds");
		OC_ASSERT_DIALOG((char*)indices + orca_glDrawElements_indices_length(instance, count, type) <= ((char*)_mem + _memSize), "parameter 'indices' is out of bounds");
	}
	glDrawElements(mode, count, type, indices);
}

void glEnable_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum cap = (GLenum)*(i32*)&_params[0];
	glEnable(cap);
}

void glEnableVertexAttribArray_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint index = (GLuint)*(i32*)&_params[0];
	glEnableVertexAttribArray(index);
}

void glFinish_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	glFinish();
}

void glFlush_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	glFlush();
}

void glFramebufferRenderbuffer_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum attachment = (GLenum)*(i32*)&_params[1];
	GLenum renderbuffertarget = (GLenum)*(i32*)&_params[2];
	GLuint renderbuffer = (GLuint)*(i32*)&_params[3];
	glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
}

void glFramebufferTexture2D_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum attachment = (GLenum)*(i32*)&_params[1];
	GLenum textarget = (GLenum)*(i32*)&_params[2];
	GLuint texture = (GLuint)*(i32*)&_params[3];
	GLint level = (GLint)*(i32*)&_params[4];
	glFramebufferTexture2D(target, attachment, textarget, texture, level);
}

void glFrontFace_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum mode = (GLenum)*(i32*)&_params[0];
	glFrontFace(mode);
}

void glGenBuffers_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	GLuint * buffers = (GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)buffers >= (char*)_mem) && (((char*)buffers - (char*)_mem) < _memSize), "parameter 'buffers' is out of bounds");
		OC_ASSERT_DIALOG((char*)buffers + n*sizeof(GLuint ) <= ((char*)_mem + _memSize), "parameter 'buffers' is out of bounds");
	}
	glGenBuffers(n, buffers);
}

void glGenerateMipmap_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	glGenerateMipmap(target);
}

void glGenFramebuffers_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	GLuint * framebuffers = (GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)framebuffers >= (char*)_mem) && (((char*)framebuffers - (char*)_mem) < _memSize), "parameter 'framebuffers' is out of bounds");
		OC_ASSERT_DIALOG((char*)framebuffers + n*sizeof(GLuint ) <= ((char*)_mem + _memSize), "parameter 'framebuffers' is out of bounds");
	}
	glGenFramebuffers(n, framebuffers);
}

void glGenRenderbuffers_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	GLuint * renderbuffers = (GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)renderbuffers >= (char*)_mem) && (((char*)renderbuffers - (char*)_mem) < _memSize), "parameter 'renderbuffers' is out of bounds");
		OC_ASSERT_DIALOG((char*)renderbuffers + n*sizeof(GLuint ) <= ((char*)_mem + _memSize), "parameter 'renderbuffers' is out of bounds");
	}
	glGenRenderbuffers(n, renderbuffers);
}

void glGenTextures_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	GLuint * textures = (GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)textures >= (char*)_mem) && (((char*)textures - (char*)_mem) < _memSize), "parameter 'textures' is out of bounds");
		OC_ASSERT_DIALOG((char*)textures + n*sizeof(GLuint ) <= ((char*)_mem + _memSize), "parameter 'textures' is out of bounds");
	}
	glGenTextures(n, textures);
}

void glGetActiveAttrib_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLuint index = (GLuint)*(i32*)&_params[1];
	GLsizei bufSize = (GLsizei)*(i32*)&_params[2];
	GLsizei * length = (GLsizei *)((char*)_mem + *(u32*)&_params[3]);
	GLint * size = (GLint *)((char*)_mem + *(u32*)&_params[4]);
	GLenum * type = (GLenum *)((char*)_mem + *(u32*)&_params[5]);
	GLchar * name = (GLchar *)((char*)_mem + *(u32*)&_params[6]);
	{
		OC_ASSERT_DIALOG(((char*)length >= (char*)_mem) && (((char*)length - (char*)_mem) < _memSize), "parameter 'length' is out of bounds");
		OC_ASSERT_DIALOG((char*)length + 1*sizeof(GLsizei ) <= ((char*)_mem + _memSize), "parameter 'length' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)size >= (char*)_mem) && (((char*)size - (char*)_mem) < _memSize), "parameter 'size' is out of bounds");
		OC_ASSERT_DIALOG((char*)size + 1*sizeof(GLint ) <= ((char*)_mem + _memSize), "parameter 'size' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)type >= (char*)_mem) && (((char*)type - (char*)_mem) < _memSize), "parameter 'type' is out of bounds");
		OC_ASSERT_DIALOG((char*)type + 1*sizeof(GLenum ) <= ((char*)_mem + _memSize), "parameter 'type' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)name >= (char*)_mem) && (((char*)name - (char*)_mem) < _memSize), "parameter 'name' is out of bounds");
		OC_ASSERT_DIALOG((char*)name + bufSize*sizeof(GLchar ) <= ((char*)_mem + _memSize), "parameter 'name' is out of bounds");
	}
	glGetActiveAttrib(program, index, bufSize, length, size, type, name);
}

void glGetActiveUniform_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLuint index = (GLuint)*(i32*)&_params[1];
	GLsizei bufSize = (GLsizei)*(i32*)&_params[2];
	GLsizei * length = (GLsizei *)((char*)_mem + *(u32*)&_params[3]);
	GLint * size = (GLint *)((char*)_mem + *(u32*)&_params[4]);
	GLenum * type = (GLenum *)((char*)_mem + *(u32*)&_params[5]);
	GLchar * name = (GLchar *)((char*)_mem + *(u32*)&_params[6]);
	{
		OC_ASSERT_DIALOG(((char*)length >= (char*)_mem) && (((char*)length - (char*)_mem) < _memSize), "parameter 'length' is out of bounds");
		OC_ASSERT_DIALOG((char*)length + 1*sizeof(GLsizei ) <= ((char*)_mem + _memSize), "parameter 'length' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)size >= (char*)_mem) && (((char*)size - (char*)_mem) < _memSize), "parameter 'size' is out of bounds");
		OC_ASSERT_DIALOG((char*)size + 1*sizeof(GLint ) <= ((char*)_mem + _memSize), "parameter 'size' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)type >= (char*)_mem) && (((char*)type - (char*)_mem) < _memSize), "parameter 'type' is out of bounds");
		OC_ASSERT_DIALOG((char*)type + 1*sizeof(GLenum ) <= ((char*)_mem + _memSize), "parameter 'type' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)name >= (char*)_mem) && (((char*)name - (char*)_mem) < _memSize), "parameter 'name' is out of bounds");
		OC_ASSERT_DIALOG((char*)name + bufSize*sizeof(GLchar ) <= ((char*)_mem + _memSize), "parameter 'name' is out of bounds");
	}
	glGetActiveUniform(program, index, bufSize, length, size, type, name);
}

void glGetAttachedShaders_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLsizei maxCount = (GLsizei)*(i32*)&_params[1];
	GLsizei * count = (GLsizei *)((char*)_mem + *(u32*)&_params[2]);
	GLuint * shaders = (GLuint *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)count >= (char*)_mem) && (((char*)count - (char*)_mem) < _memSize), "parameter 'count' is out of bounds");
		OC_ASSERT_DIALOG((char*)count + 1*sizeof(GLsizei ) <= ((char*)_mem + _memSize), "parameter 'count' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)shaders >= (char*)_mem) && (((char*)shaders - (char*)_mem) < _memSize), "parameter 'shaders' is out of bounds");
		OC_ASSERT_DIALOG((char*)shaders + maxCount*sizeof(GLuint ) <= ((char*)_mem + _memSize), "parameter 'shaders' is out of bounds");
	}
	glGetAttachedShaders(program, maxCount, count, shaders);
}

void glGetAttribLocation_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	const GLchar * name = (const GLchar *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)name >= (char*)_mem) && (((char*)name - (char*)_mem) < _memSize), "parameter 'name' is out of bounds");
		OC_ASSERT_DIALOG((char*)name + orca_glGetAttribLocation_name_length(instance, name)*sizeof(const GLchar ) <= ((char*)_mem + _memSize), "parameter 'name' is out of bounds");
	}
	*((i32*)&_returns[0]) = (i32)glGetAttribLocation(program, name);
}

void glGetBooleanv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum pname = (GLenum)*(i32*)&_params[0];
	GLboolean * data = (GLboolean *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)data >= (char*)_mem) && (((char*)data - (char*)_mem) < _memSize), "parameter 'data' is out of bounds");
		OC_ASSERT_DIALOG((char*)data + orca_glGetBooleanv_data_length(instance, pname)*sizeof(GLboolean ) <= ((char*)_mem + _memSize), "parameter 'data' is out of bounds");
	}
	glGetBooleanv(pname, data);
}

void glGetBufferParameteriv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetBufferParameteriv_params_length(instance, pname)*sizeof(GLint ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glGetBufferParameteriv(target, pname, params);
}

void glGetError_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	*((i32*)&_returns[0]) = (i32)glGetError();
}

void glGetFloatv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum pname = (GLenum)*(i32*)&_params[0];
	GLfloat * data = (GLfloat *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)data >= (char*)_mem) && (((char*)data - (char*)_mem) < _memSize), "parameter 'data' is out of bounds");
		OC_ASSERT_DIALOG((char*)data + orca_glGetFloatv_data_length(instance, pname)*sizeof(GLfloat ) <= ((char*)_mem + _memSize), "parameter 'data' is out of bounds");
	}
	glGetFloatv(pname, data);
}

void glGetFramebufferAttachmentParameteriv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum attachment = (GLenum)*(i32*)&_params[1];
	GLenum pname = (GLenum)*(i32*)&_params[2];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetFramebufferAttachmentParameteriv_params_length(instance, pname)*sizeof(GLint ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
}

void glGetIntegerv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum pname = (GLenum)*(i32*)&_params[0];
	GLint * data = (GLint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)data >= (char*)_mem) && (((char*)data - (char*)_mem) < _memSize), "parameter 'data' is out of bounds");
		OC_ASSERT_DIALOG((char*)data + orca_glGetIntegerv_data_length(instance, pname)*sizeof(GLint ) <= ((char*)_mem + _memSize), "parameter 'data' is out of bounds");
	}
	glGetIntegerv(pname, data);
}

void glGetProgramiv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetProgramiv_params_length(instance, pname)*sizeof(GLint ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glGetProgramiv(program, pname, params);
}

void glGetProgramInfoLog_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLsizei bufSize = (GLsizei)*(i32*)&_params[1];
	GLsizei * length = (GLsizei *)((char*)_mem + *(u32*)&_params[2]);
	GLchar * infoLog = (GLchar *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)length >= (char*)_mem) && (((char*)length - (char*)_mem) < _memSize), "parameter 'length' is out of bounds");
		OC_ASSERT_DIALOG((char*)length + 1*sizeof(GLsizei ) <= ((char*)_mem + _memSize), "parameter 'length' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)infoLog >= (char*)_mem) && (((char*)infoLog - (char*)_mem) < _memSize), "parameter 'infoLog' is out of bounds");
		OC_ASSERT_DIALOG((char*)infoLog + bufSize*sizeof(GLchar ) <= ((char*)_mem + _memSize), "parameter 'infoLog' is out of bounds");
	}
	glGetProgramInfoLog(program, bufSize, length, infoLog);
}

void glGetRenderbufferParameteriv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetRenderbufferParameteriv_params_length(instance, pname)*sizeof(GLint ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glGetRenderbufferParameteriv(target, pname, params);
}

void glGetShaderiv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint shader = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetShaderiv_params_length(instance, pname)*sizeof(GLint ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glGetShaderiv(shader, pname, params);
}

void glGetShaderInfoLog_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint shader = (GLuint)*(i32*)&_params[0];
	GLsizei bufSize = (GLsizei)*(i32*)&_params[1];
	GLsizei * length = (GLsizei *)((char*)_mem + *(u32*)&_params[2]);
	GLchar * infoLog = (GLchar *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)length >= (char*)_mem) && (((char*)length - (char*)_mem) < _memSize), "parameter 'length' is out of bounds");
		OC_ASSERT_DIALOG((char*)length + 1*sizeof(GLsizei ) <= ((char*)_mem + _memSize), "parameter 'length' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)infoLog >= (char*)_mem) && (((char*)infoLog - (char*)_mem) < _memSize), "parameter 'infoLog' is out of bounds");
		OC_ASSERT_DIALOG((char*)infoLog + bufSize*sizeof(GLchar ) <= ((char*)_mem + _memSize), "parameter 'infoLog' is out of bounds");
	}
	glGetShaderInfoLog(shader, bufSize, length, infoLog);
}

void glGetShaderPrecisionFormat_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum shadertype = (GLenum)*(i32*)&_params[0];
	GLenum precisiontype = (GLenum)*(i32*)&_params[1];
	GLint * range = (GLint *)((char*)_mem + *(u32*)&_params[2]);
	GLint * precision = (GLint *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)range >= (char*)_mem) && (((char*)range - (char*)_mem) < _memSize), "parameter 'range' is out of bounds");
		OC_ASSERT_DIALOG((char*)range + 2*sizeof(GLint ) <= ((char*)_mem + _memSize), "parameter 'range' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)precision >= (char*)_mem) && (((char*)precision - (char*)_mem) < _memSize), "parameter 'precision' is out of bounds");
		OC_ASSERT_DIALOG((char*)precision + 1*sizeof(GLint ) <= ((char*)_mem + _memSize), "parameter 'precision' is out of bounds");
	}
	glGetShaderPrecisionFormat(shadertype, precisiontype, range, precision);
}

void glGetShaderSource_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint shader = (GLuint)*(i32*)&_params[0];
	GLsizei bufSize = (GLsizei)*(i32*)&_params[1];
	GLsizei * length = (GLsizei *)((char*)_mem + *(u32*)&_params[2]);
	GLchar * source = (GLchar *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)length >= (char*)_mem) && (((char*)length - (char*)_mem) < _memSize), "parameter 'length' is out of bounds");
		OC_ASSERT_DIALOG((char*)length + 1*sizeof(GLsizei ) <= ((char*)_mem + _memSize), "parameter 'length' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)source >= (char*)_mem) && (((char*)source - (char*)_mem) < _memSize), "parameter 'source' is out of bounds");
		OC_ASSERT_DIALOG((char*)source + bufSize*sizeof(GLchar ) <= ((char*)_mem + _memSize), "parameter 'source' is out of bounds");
	}
	glGetShaderSource(shader, bufSize, length, source);
}

void glGetTexParameterfv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLfloat * params = (GLfloat *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetTexParameterfv_params_length(instance, pname)*sizeof(GLfloat ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glGetTexParameterfv(target, pname, params);
}

void glGetTexParameteriv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetTexParameteriv_params_length(instance, pname)*sizeof(GLint ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glGetTexParameteriv(target, pname, params);
}

void glGetUniformfv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLfloat * params = (GLfloat *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetUniformfv_params_length(instance, program, location)*sizeof(GLfloat ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glGetUniformfv(program, location, params);
}

void glGetUniformiv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetUniformiv_params_length(instance, program, location)*sizeof(GLint ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glGetUniformiv(program, location, params);
}

void glGetUniformLocation_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	const GLchar * name = (const GLchar *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)name >= (char*)_mem) && (((char*)name - (char*)_mem) < _memSize), "parameter 'name' is out of bounds");
		OC_ASSERT_DIALOG((char*)name + orca_glGetUniformLocation_name_length(instance, name)*sizeof(const GLchar ) <= ((char*)_mem + _memSize), "parameter 'name' is out of bounds");
	}
	*((i32*)&_returns[0]) = (i32)glGetUniformLocation(program, name);
}

void glGetVertexAttribfv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint index = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLfloat * params = (GLfloat *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + 4*sizeof(GLfloat ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glGetVertexAttribfv(index, pname, params);
}

void glGetVertexAttribiv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint index = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + 4*sizeof(GLint ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glGetVertexAttribiv(index, pname, params);
}

void glHint_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum mode = (GLenum)*(i32*)&_params[1];
	glHint(target, mode);
}

void glIsBuffer_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint buffer = (GLuint)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glIsBuffer(buffer);
}

void glIsEnabled_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum cap = (GLenum)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glIsEnabled(cap);
}

void glIsFramebuffer_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint framebuffer = (GLuint)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glIsFramebuffer(framebuffer);
}

void glIsProgram_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glIsProgram(program);
}

void glIsRenderbuffer_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint renderbuffer = (GLuint)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glIsRenderbuffer(renderbuffer);
}

void glIsShader_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint shader = (GLuint)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glIsShader(shader);
}

void glIsTexture_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint texture = (GLuint)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glIsTexture(texture);
}

void glLineWidth_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLfloat width = (GLfloat)*(f32*)&_params[0];
	glLineWidth(width);
}

void glLinkProgram_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	glLinkProgram(program);
}

void glPixelStorei_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum pname = (GLenum)*(i32*)&_params[0];
	GLint param = (GLint)*(i32*)&_params[1];
	glPixelStorei(pname, param);
}

void glPolygonOffset_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLfloat factor = (GLfloat)*(f32*)&_params[0];
	GLfloat units = (GLfloat)*(f32*)&_params[1];
	glPolygonOffset(factor, units);
}

void glReadPixels_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint x = (GLint)*(i32*)&_params[0];
	GLint y = (GLint)*(i32*)&_params[1];
	GLsizei width = (GLsizei)*(i32*)&_params[2];
	GLsizei height = (GLsizei)*(i32*)&_params[3];
	GLenum format = (GLenum)*(i32*)&_params[4];
	GLenum type = (GLenum)*(i32*)&_params[5];
	void * pixels = (void *)((char*)_mem + *(u32*)&_params[6]);
	{
		OC_ASSERT_DIALOG(((char*)pixels >= (char*)_mem) && (((char*)pixels - (char*)_mem) < _memSize), "parameter 'pixels' is out of bounds");
		OC_ASSERT_DIALOG((char*)pixels + orca_glReadPixels_pixels_length(instance, format, type, width, height) <= ((char*)_mem + _memSize), "parameter 'pixels' is out of bounds");
	}
	glReadPixels(x, y, width, height, format, type, pixels);
}

void glReleaseShaderCompiler_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	glReleaseShaderCompiler();
}

void glRenderbufferStorage_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum internalformat = (GLenum)*(i32*)&_params[1];
	GLsizei width = (GLsizei)*(i32*)&_params[2];
	GLsizei height = (GLsizei)*(i32*)&_params[3];
	glRenderbufferStorage(target, internalformat, width, height);
}

void glSampleCoverage_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLfloat value = (GLfloat)*(f32*)&_params[0];
	GLboolean invert = (GLboolean)*(i32*)&_params[1];
	glSampleCoverage(value, invert);
}

void glScissor_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint x = (GLint)*(i32*)&_params[0];
	GLint y = (GLint)*(i32*)&_params[1];
	GLsizei width = (GLsizei)*(i32*)&_params[2];
	GLsizei height = (GLsizei)*(i32*)&_params[3];
	glScissor(x, y, width, height);
}

void glShaderBinary_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLsizei count = (GLsizei)*(i32*)&_params[0];
	const GLuint * shaders = (const GLuint *)((char*)_mem + *(u32*)&_params[1]);
	GLenum binaryFormat = (GLenum)*(i32*)&_params[2];
	const void * binary = (const void *)((char*)_mem + *(u32*)&_params[3]);
	GLsizei length = (GLsizei)*(i32*)&_params[4];
	{
		OC_ASSERT_DIALOG(((char*)shaders >= (char*)_mem) && (((char*)shaders - (char*)_mem) < _memSize), "parameter 'shaders' is out of bounds");
		OC_ASSERT_DIALOG((char*)shaders + count*sizeof(const GLuint ) <= ((char*)_mem + _memSize), "parameter 'shaders' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)binary >= (char*)_mem) && (((char*)binary - (char*)_mem) < _memSize), "parameter 'binary' is out of bounds");
		OC_ASSERT_DIALOG((char*)binary + length <= ((char*)_mem + _memSize), "parameter 'binary' is out of bounds");
	}
	glShaderBinary(count, shaders, binaryFormat, binary, length);
}

void glStencilFunc_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum func = (GLenum)*(i32*)&_params[0];
	GLint ref = (GLint)*(i32*)&_params[1];
	GLuint mask = (GLuint)*(i32*)&_params[2];
	glStencilFunc(func, ref, mask);
}

void glStencilFuncSeparate_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum face = (GLenum)*(i32*)&_params[0];
	GLenum func = (GLenum)*(i32*)&_params[1];
	GLint ref = (GLint)*(i32*)&_params[2];
	GLuint mask = (GLuint)*(i32*)&_params[3];
	glStencilFuncSeparate(face, func, ref, mask);
}

void glStencilMask_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint mask = (GLuint)*(i32*)&_params[0];
	glStencilMask(mask);
}

void glStencilMaskSeparate_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum face = (GLenum)*(i32*)&_params[0];
	GLuint mask = (GLuint)*(i32*)&_params[1];
	glStencilMaskSeparate(face, mask);
}

void glStencilOp_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum fail = (GLenum)*(i32*)&_params[0];
	GLenum zfail = (GLenum)*(i32*)&_params[1];
	GLenum zpass = (GLenum)*(i32*)&_params[2];
	glStencilOp(fail, zfail, zpass);
}

void glStencilOpSeparate_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum face = (GLenum)*(i32*)&_params[0];
	GLenum sfail = (GLenum)*(i32*)&_params[1];
	GLenum dpfail = (GLenum)*(i32*)&_params[2];
	GLenum dppass = (GLenum)*(i32*)&_params[3];
	glStencilOpSeparate(face, sfail, dpfail, dppass);
}

void glTexImage2D_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLint level = (GLint)*(i32*)&_params[1];
	GLint internalformat = (GLint)*(i32*)&_params[2];
	GLsizei width = (GLsizei)*(i32*)&_params[3];
	GLsizei height = (GLsizei)*(i32*)&_params[4];
	GLint border = (GLint)*(i32*)&_params[5];
	GLenum format = (GLenum)*(i32*)&_params[6];
	GLenum type = (GLenum)*(i32*)&_params[7];
	const void * pixels = (const void *)((char*)_mem + *(u32*)&_params[8]);
	{
		OC_ASSERT_DIALOG(((char*)pixels >= (char*)_mem) && (((char*)pixels - (char*)_mem) < _memSize), "parameter 'pixels' is out of bounds");
		OC_ASSERT_DIALOG((char*)pixels + orca_glTexImage2D_pixels_length(instance, format, type, width, height) <= ((char*)_mem + _memSize), "parameter 'pixels' is out of bounds");
	}
	glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
}

void glTexParameterf_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLfloat param = (GLfloat)*(f32*)&_params[2];
	glTexParameterf(target, pname, param);
}

void glTexParameterfv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	const GLfloat * params = (const GLfloat *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glTexParameterfv_params_length(instance, pname)*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glTexParameterfv(target, pname, params);
}

void glTexParameteri_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint param = (GLint)*(i32*)&_params[2];
	glTexParameteri(target, pname, param);
}

void glTexParameteriv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	const GLint * params = (const GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glTexParameteriv_params_length(instance, pname)*sizeof(const GLint ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glTexParameteriv(target, pname, params);
}

void glTexSubImage2D_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLint level = (GLint)*(i32*)&_params[1];
	GLint xoffset = (GLint)*(i32*)&_params[2];
	GLint yoffset = (GLint)*(i32*)&_params[3];
	GLsizei width = (GLsizei)*(i32*)&_params[4];
	GLsizei height = (GLsizei)*(i32*)&_params[5];
	GLenum format = (GLenum)*(i32*)&_params[6];
	GLenum type = (GLenum)*(i32*)&_params[7];
	const void * pixels = (const void *)((char*)_mem + *(u32*)&_params[8]);
	{
		OC_ASSERT_DIALOG(((char*)pixels >= (char*)_mem) && (((char*)pixels - (char*)_mem) < _memSize), "parameter 'pixels' is out of bounds");
		OC_ASSERT_DIALOG((char*)pixels + orca_glTexSubImage2D_pixels_length(instance, format, type, width, height) <= ((char*)_mem + _memSize), "parameter 'pixels' is out of bounds");
	}
	glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

void glUniform1f_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLfloat v0 = (GLfloat)*(f32*)&_params[1];
	glUniform1f(location, v0);
}

void glUniform1fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 1*count*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glUniform1fv(location, count, value);
}

void glUniform1i_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLint v0 = (GLint)*(i32*)&_params[1];
	glUniform1i(location, v0);
}

void glUniform1iv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	const GLint * value = (const GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 1*count*sizeof(const GLint ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glUniform1iv(location, count, value);
}

void glUniform2f_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLfloat v0 = (GLfloat)*(f32*)&_params[1];
	GLfloat v1 = (GLfloat)*(f32*)&_params[2];
	glUniform2f(location, v0, v1);
}

void glUniform2fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 2*count*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glUniform2fv(location, count, value);
}

void glUniform2i_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLint v0 = (GLint)*(i32*)&_params[1];
	GLint v1 = (GLint)*(i32*)&_params[2];
	glUniform2i(location, v0, v1);
}

void glUniform2iv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	const GLint * value = (const GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 2*count*sizeof(const GLint ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glUniform2iv(location, count, value);
}

void glUniform3f_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLfloat v0 = (GLfloat)*(f32*)&_params[1];
	GLfloat v1 = (GLfloat)*(f32*)&_params[2];
	GLfloat v2 = (GLfloat)*(f32*)&_params[3];
	glUniform3f(location, v0, v1, v2);
}

void glUniform3fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 3*count*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glUniform3fv(location, count, value);
}

void glUniform3i_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLint v0 = (GLint)*(i32*)&_params[1];
	GLint v1 = (GLint)*(i32*)&_params[2];
	GLint v2 = (GLint)*(i32*)&_params[3];
	glUniform3i(location, v0, v1, v2);
}

void glUniform3iv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	const GLint * value = (const GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 3*count*sizeof(const GLint ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glUniform3iv(location, count, value);
}

void glUniform4f_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLfloat v0 = (GLfloat)*(f32*)&_params[1];
	GLfloat v1 = (GLfloat)*(f32*)&_params[2];
	GLfloat v2 = (GLfloat)*(f32*)&_params[3];
	GLfloat v3 = (GLfloat)*(f32*)&_params[4];
	glUniform4f(location, v0, v1, v2, v3);
}

void glUniform4fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 4*count*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glUniform4fv(location, count, value);
}

void glUniform4i_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLint v0 = (GLint)*(i32*)&_params[1];
	GLint v1 = (GLint)*(i32*)&_params[2];
	GLint v2 = (GLint)*(i32*)&_params[3];
	GLint v3 = (GLint)*(i32*)&_params[4];
	glUniform4i(location, v0, v1, v2, v3);
}

void glUniform4iv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	const GLint * value = (const GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 4*count*sizeof(const GLint ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glUniform4iv(location, count, value);
}

void glUniformMatrix2fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	GLboolean transpose = (GLboolean)*(i32*)&_params[2];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 4*count*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glUniformMatrix2fv(location, count, transpose, value);
}

void glUniformMatrix3fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	GLboolean transpose = (GLboolean)*(i32*)&_params[2];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 9*count*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glUniformMatrix3fv(location, count, transpose, value);
}

void glUniformMatrix4fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	GLboolean transpose = (GLboolean)*(i32*)&_params[2];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 16*count*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glUniformMatrix4fv(location, count, transpose, value);
}

void glUseProgram_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	glUseProgram(program);
}

void glValidateProgram_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	glValidateProgram(program);
}

void glVertexAttrib1f_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint index = (GLuint)*(i32*)&_params[0];
	GLfloat x = (GLfloat)*(f32*)&_params[1];
	glVertexAttrib1f(index, x);
}

void glVertexAttrib1fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint index = (GLuint)*(i32*)&_params[0];
	const GLfloat * v = (const GLfloat *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)v >= (char*)_mem) && (((char*)v - (char*)_mem) < _memSize), "parameter 'v' is out of bounds");
		OC_ASSERT_DIALOG((char*)v + 1*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'v' is out of bounds");
	}
	glVertexAttrib1fv(index, v);
}

void glVertexAttrib2f_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint index = (GLuint)*(i32*)&_params[0];
	GLfloat x = (GLfloat)*(f32*)&_params[1];
	GLfloat y = (GLfloat)*(f32*)&_params[2];
	glVertexAttrib2f(index, x, y);
}

void glVertexAttrib2fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint index = (GLuint)*(i32*)&_params[0];
	const GLfloat * v = (const GLfloat *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)v >= (char*)_mem) && (((char*)v - (char*)_mem) < _memSize), "parameter 'v' is out of bounds");
		OC_ASSERT_DIALOG((char*)v + 2*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'v' is out of bounds");
	}
	glVertexAttrib2fv(index, v);
}

void glVertexAttrib3f_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint index = (GLuint)*(i32*)&_params[0];
	GLfloat x = (GLfloat)*(f32*)&_params[1];
	GLfloat y = (GLfloat)*(f32*)&_params[2];
	GLfloat z = (GLfloat)*(f32*)&_params[3];
	glVertexAttrib3f(index, x, y, z);
}

void glVertexAttrib3fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint index = (GLuint)*(i32*)&_params[0];
	const GLfloat * v = (const GLfloat *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)v >= (char*)_mem) && (((char*)v - (char*)_mem) < _memSize), "parameter 'v' is out of bounds");
		OC_ASSERT_DIALOG((char*)v + 3*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'v' is out of bounds");
	}
	glVertexAttrib3fv(index, v);
}

void glVertexAttrib4f_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint index = (GLuint)*(i32*)&_params[0];
	GLfloat x = (GLfloat)*(f32*)&_params[1];
	GLfloat y = (GLfloat)*(f32*)&_params[2];
	GLfloat z = (GLfloat)*(f32*)&_params[3];
	GLfloat w = (GLfloat)*(f32*)&_params[4];
	glVertexAttrib4f(index, x, y, z, w);
}

void glVertexAttrib4fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint index = (GLuint)*(i32*)&_params[0];
	const GLfloat * v = (const GLfloat *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)v >= (char*)_mem) && (((char*)v - (char*)_mem) < _memSize), "parameter 'v' is out of bounds");
		OC_ASSERT_DIALOG((char*)v + 4*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'v' is out of bounds");
	}
	glVertexAttrib4fv(index, v);
}

void glViewport_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint x = (GLint)*(i32*)&_params[0];
	GLint y = (GLint)*(i32*)&_params[1];
	GLsizei width = (GLsizei)*(i32*)&_params[2];
	GLsizei height = (GLsizei)*(i32*)&_params[3];
	glViewport(x, y, width, height);
}

void glReadBuffer_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum src = (GLenum)*(i32*)&_params[0];
	glReadBuffer(src);
}

void glDrawRangeElements_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum mode = (GLenum)*(i32*)&_params[0];
	GLuint start = (GLuint)*(i32*)&_params[1];
	GLuint end = (GLuint)*(i32*)&_params[2];
	GLsizei count = (GLsizei)*(i32*)&_params[3];
	GLenum type = (GLenum)*(i32*)&_params[4];
	const void * indices = (const void *)((char*)_mem + *(u32*)&_params[5]);
	{
		OC_ASSERT_DIALOG(((char*)indices >= (char*)_mem) && (((char*)indices - (char*)_mem) < _memSize), "parameter 'indices' is out of bounds");
		OC_ASSERT_DIALOG((char*)indices + orca_glDrawRangeElements_indices_length(instance, count, type) <= ((char*)_mem + _memSize), "parameter 'indices' is out of bounds");
	}
	glDrawRangeElements(mode, start, end, count, type, indices);
}

void glTexImage3D_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLint level = (GLint)*(i32*)&_params[1];
	GLint internalformat = (GLint)*(i32*)&_params[2];
	GLsizei width = (GLsizei)*(i32*)&_params[3];
	GLsizei height = (GLsizei)*(i32*)&_params[4];
	GLsizei depth = (GLsizei)*(i32*)&_params[5];
	GLint border = (GLint)*(i32*)&_params[6];
	GLenum format = (GLenum)*(i32*)&_params[7];
	GLenum type = (GLenum)*(i32*)&_params[8];
	const void * pixels = (const void *)((char*)_mem + *(u32*)&_params[9]);
	{
		OC_ASSERT_DIALOG(((char*)pixels >= (char*)_mem) && (((char*)pixels - (char*)_mem) < _memSize), "parameter 'pixels' is out of bounds");
		OC_ASSERT_DIALOG((char*)pixels + orca_glTexImage3D_pixels_length(instance, format, type, width, height, depth) <= ((char*)_mem + _memSize), "parameter 'pixels' is out of bounds");
	}
	glTexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels);
}

void glTexSubImage3D_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLint level = (GLint)*(i32*)&_params[1];
	GLint xoffset = (GLint)*(i32*)&_params[2];
	GLint yoffset = (GLint)*(i32*)&_params[3];
	GLint zoffset = (GLint)*(i32*)&_params[4];
	GLsizei width = (GLsizei)*(i32*)&_params[5];
	GLsizei height = (GLsizei)*(i32*)&_params[6];
	GLsizei depth = (GLsizei)*(i32*)&_params[7];
	GLenum format = (GLenum)*(i32*)&_params[8];
	GLenum type = (GLenum)*(i32*)&_params[9];
	const void * pixels = (const void *)((char*)_mem + *(u32*)&_params[10]);
	{
		OC_ASSERT_DIALOG(((char*)pixels >= (char*)_mem) && (((char*)pixels - (char*)_mem) < _memSize), "parameter 'pixels' is out of bounds");
		OC_ASSERT_DIALOG((char*)pixels + orca_glTexSubImage3D_pixels_length(instance, format, type, width, height, depth) <= ((char*)_mem + _memSize), "parameter 'pixels' is out of bounds");
	}
	glTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

void glCopyTexSubImage3D_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLint level = (GLint)*(i32*)&_params[1];
	GLint xoffset = (GLint)*(i32*)&_params[2];
	GLint yoffset = (GLint)*(i32*)&_params[3];
	GLint zoffset = (GLint)*(i32*)&_params[4];
	GLint x = (GLint)*(i32*)&_params[5];
	GLint y = (GLint)*(i32*)&_params[6];
	GLsizei width = (GLsizei)*(i32*)&_params[7];
	GLsizei height = (GLsizei)*(i32*)&_params[8];
	glCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
}

void glCompressedTexImage3D_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLint level = (GLint)*(i32*)&_params[1];
	GLenum internalformat = (GLenum)*(i32*)&_params[2];
	GLsizei width = (GLsizei)*(i32*)&_params[3];
	GLsizei height = (GLsizei)*(i32*)&_params[4];
	GLsizei depth = (GLsizei)*(i32*)&_params[5];
	GLint border = (GLint)*(i32*)&_params[6];
	GLsizei imageSize = (GLsizei)*(i32*)&_params[7];
	const void * data = (const void *)((char*)_mem + *(u32*)&_params[8]);
	{
		OC_ASSERT_DIALOG(((char*)data >= (char*)_mem) && (((char*)data - (char*)_mem) < _memSize), "parameter 'data' is out of bounds");
		OC_ASSERT_DIALOG((char*)data + imageSize <= ((char*)_mem + _memSize), "parameter 'data' is out of bounds");
	}
	glCompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data);
}

void glCompressedTexSubImage3D_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLint level = (GLint)*(i32*)&_params[1];
	GLint xoffset = (GLint)*(i32*)&_params[2];
	GLint yoffset = (GLint)*(i32*)&_params[3];
	GLint zoffset = (GLint)*(i32*)&_params[4];
	GLsizei width = (GLsizei)*(i32*)&_params[5];
	GLsizei height = (GLsizei)*(i32*)&_params[6];
	GLsizei depth = (GLsizei)*(i32*)&_params[7];
	GLenum format = (GLenum)*(i32*)&_params[8];
	GLsizei imageSize = (GLsizei)*(i32*)&_params[9];
	const void * data = (const void *)((char*)_mem + *(u32*)&_params[10]);
	{
		OC_ASSERT_DIALOG(((char*)data >= (char*)_mem) && (((char*)data - (char*)_mem) < _memSize), "parameter 'data' is out of bounds");
		OC_ASSERT_DIALOG((char*)data + imageSize <= ((char*)_mem + _memSize), "parameter 'data' is out of bounds");
	}
	glCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
}

void glGenQueries_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	GLuint * ids = (GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)ids >= (char*)_mem) && (((char*)ids - (char*)_mem) < _memSize), "parameter 'ids' is out of bounds");
		OC_ASSERT_DIALOG((char*)ids + n*sizeof(GLuint ) <= ((char*)_mem + _memSize), "parameter 'ids' is out of bounds");
	}
	glGenQueries(n, ids);
}

void glDeleteQueries_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	const GLuint * ids = (const GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)ids >= (char*)_mem) && (((char*)ids - (char*)_mem) < _memSize), "parameter 'ids' is out of bounds");
		OC_ASSERT_DIALOG((char*)ids + n*sizeof(const GLuint ) <= ((char*)_mem + _memSize), "parameter 'ids' is out of bounds");
	}
	glDeleteQueries(n, ids);
}

void glIsQuery_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint id = (GLuint)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glIsQuery(id);
}

void glBeginQuery_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLuint id = (GLuint)*(i32*)&_params[1];
	glBeginQuery(target, id);
}

void glEndQuery_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	glEndQuery(target);
}

void glGetQueryiv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetQueryiv_params_length(instance, pname)*sizeof(GLint ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glGetQueryiv(target, pname, params);
}

void glGetQueryObjectuiv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint id = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLuint * params = (GLuint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetQueryObjectuiv_params_length(instance, pname)*sizeof(GLuint ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glGetQueryObjectuiv(id, pname, params);
}

void glDrawBuffers_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	const GLenum * bufs = (const GLenum *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)bufs >= (char*)_mem) && (((char*)bufs - (char*)_mem) < _memSize), "parameter 'bufs' is out of bounds");
		OC_ASSERT_DIALOG((char*)bufs + n*sizeof(const GLenum ) <= ((char*)_mem + _memSize), "parameter 'bufs' is out of bounds");
	}
	glDrawBuffers(n, bufs);
}

void glUniformMatrix2x3fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	GLboolean transpose = (GLboolean)*(i32*)&_params[2];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 6*count*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glUniformMatrix2x3fv(location, count, transpose, value);
}

void glUniformMatrix3x2fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	GLboolean transpose = (GLboolean)*(i32*)&_params[2];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 6*count*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glUniformMatrix3x2fv(location, count, transpose, value);
}

void glUniformMatrix2x4fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	GLboolean transpose = (GLboolean)*(i32*)&_params[2];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 8*count*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glUniformMatrix2x4fv(location, count, transpose, value);
}

void glUniformMatrix4x2fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	GLboolean transpose = (GLboolean)*(i32*)&_params[2];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 8*count*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glUniformMatrix4x2fv(location, count, transpose, value);
}

void glUniformMatrix3x4fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	GLboolean transpose = (GLboolean)*(i32*)&_params[2];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 12*count*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glUniformMatrix3x4fv(location, count, transpose, value);
}

void glUniformMatrix4x3fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	GLboolean transpose = (GLboolean)*(i32*)&_params[2];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 12*count*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glUniformMatrix4x3fv(location, count, transpose, value);
}

void glBlitFramebuffer_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint srcX0 = (GLint)*(i32*)&_params[0];
	GLint srcY0 = (GLint)*(i32*)&_params[1];
	GLint srcX1 = (GLint)*(i32*)&_params[2];
	GLint srcY1 = (GLint)*(i32*)&_params[3];
	GLint dstX0 = (GLint)*(i32*)&_params[4];
	GLint dstY0 = (GLint)*(i32*)&_params[5];
	GLint dstX1 = (GLint)*(i32*)&_params[6];
	GLint dstY1 = (GLint)*(i32*)&_params[7];
	GLbitfield mask = (GLbitfield)*(i32*)&_params[8];
	GLenum filter = (GLenum)*(i32*)&_params[9];
	glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

void glRenderbufferStorageMultisample_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLsizei samples = (GLsizei)*(i32*)&_params[1];
	GLenum internalformat = (GLenum)*(i32*)&_params[2];
	GLsizei width = (GLsizei)*(i32*)&_params[3];
	GLsizei height = (GLsizei)*(i32*)&_params[4];
	glRenderbufferStorageMultisample(target, samples, internalformat, width, height);
}

void glFramebufferTextureLayer_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum attachment = (GLenum)*(i32*)&_params[1];
	GLuint texture = (GLuint)*(i32*)&_params[2];
	GLint level = (GLint)*(i32*)&_params[3];
	GLint layer = (GLint)*(i32*)&_params[4];
	glFramebufferTextureLayer(target, attachment, texture, level, layer);
}

void glBindVertexArray_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint array = (GLuint)*(i32*)&_params[0];
	glBindVertexArray(array);
}

void glDeleteVertexArrays_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	const GLuint * arrays = (const GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)arrays >= (char*)_mem) && (((char*)arrays - (char*)_mem) < _memSize), "parameter 'arrays' is out of bounds");
		OC_ASSERT_DIALOG((char*)arrays + n*sizeof(const GLuint ) <= ((char*)_mem + _memSize), "parameter 'arrays' is out of bounds");
	}
	glDeleteVertexArrays(n, arrays);
}

void glGenVertexArrays_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	GLuint * arrays = (GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)arrays >= (char*)_mem) && (((char*)arrays - (char*)_mem) < _memSize), "parameter 'arrays' is out of bounds");
		OC_ASSERT_DIALOG((char*)arrays + n*sizeof(GLuint ) <= ((char*)_mem + _memSize), "parameter 'arrays' is out of bounds");
	}
	glGenVertexArrays(n, arrays);
}

void glIsVertexArray_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint array = (GLuint)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glIsVertexArray(array);
}

void glGetIntegeri_v_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLuint index = (GLuint)*(i32*)&_params[1];
	GLint * data = (GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)data >= (char*)_mem) && (((char*)data - (char*)_mem) < _memSize), "parameter 'data' is out of bounds");
		OC_ASSERT_DIALOG((char*)data + orca_glGetIntegeri_v_data_length(instance, target)*sizeof(GLint ) <= ((char*)_mem + _memSize), "parameter 'data' is out of bounds");
	}
	glGetIntegeri_v(target, index, data);
}

void glBeginTransformFeedback_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum primitiveMode = (GLenum)*(i32*)&_params[0];
	glBeginTransformFeedback(primitiveMode);
}

void glEndTransformFeedback_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	glEndTransformFeedback();
}

void glBindBufferRange_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLuint index = (GLuint)*(i32*)&_params[1];
	GLuint buffer = (GLuint)*(i32*)&_params[2];
	GLintptr offset = (GLintptr)*(i32*)&_params[3];
	GLsizeiptr size = (GLsizeiptr)*(i32*)&_params[4];
	glBindBufferRange(target, index, buffer, offset, size);
}

void glBindBufferBase_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLuint index = (GLuint)*(i32*)&_params[1];
	GLuint buffer = (GLuint)*(i32*)&_params[2];
	glBindBufferBase(target, index, buffer);
}

void glTransformFeedbackVaryings_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	const GLchar *const* varyings = (const GLchar *const*)((char*)_mem + *(u32*)&_params[2]);
	GLenum bufferMode = (GLenum)*(i32*)&_params[3];
	{
		OC_ASSERT_DIALOG(((char*)varyings >= (char*)_mem) && (((char*)varyings - (char*)_mem) < _memSize), "parameter 'varyings' is out of bounds");
		OC_ASSERT_DIALOG((char*)varyings + count*sizeof(const GLchar *const) <= ((char*)_mem + _memSize), "parameter 'varyings' is out of bounds");
	}
	glTransformFeedbackVaryings(program, count, varyings, bufferMode);
}

void glGetTransformFeedbackVarying_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLuint index = (GLuint)*(i32*)&_params[1];
	GLsizei bufSize = (GLsizei)*(i32*)&_params[2];
	GLsizei * length = (GLsizei *)((char*)_mem + *(u32*)&_params[3]);
	GLsizei * size = (GLsizei *)((char*)_mem + *(u32*)&_params[4]);
	GLenum * type = (GLenum *)((char*)_mem + *(u32*)&_params[5]);
	GLchar * name = (GLchar *)((char*)_mem + *(u32*)&_params[6]);
	{
		OC_ASSERT_DIALOG(((char*)length >= (char*)_mem) && (((char*)length - (char*)_mem) < _memSize), "parameter 'length' is out of bounds");
		OC_ASSERT_DIALOG((char*)length + 1*sizeof(GLsizei ) <= ((char*)_mem + _memSize), "parameter 'length' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)size >= (char*)_mem) && (((char*)size - (char*)_mem) < _memSize), "parameter 'size' is out of bounds");
		OC_ASSERT_DIALOG((char*)size + 1*sizeof(GLsizei ) <= ((char*)_mem + _memSize), "parameter 'size' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)type >= (char*)_mem) && (((char*)type - (char*)_mem) < _memSize), "parameter 'type' is out of bounds");
		OC_ASSERT_DIALOG((char*)type + 1*sizeof(GLenum ) <= ((char*)_mem + _memSize), "parameter 'type' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)name >= (char*)_mem) && (((char*)name - (char*)_mem) < _memSize), "parameter 'name' is out of bounds");
		OC_ASSERT_DIALOG((char*)name + bufSize*sizeof(GLchar ) <= ((char*)_mem + _memSize), "parameter 'name' is out of bounds");
	}
	glGetTransformFeedbackVarying(program, index, bufSize, length, size, type, name);
}

void glGetVertexAttribIiv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint index = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + 1*sizeof(GLint ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glGetVertexAttribIiv(index, pname, params);
}

void glGetVertexAttribIuiv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint index = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLuint * params = (GLuint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + 1*sizeof(GLuint ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glGetVertexAttribIuiv(index, pname, params);
}

void glVertexAttribI4i_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint index = (GLuint)*(i32*)&_params[0];
	GLint x = (GLint)*(i32*)&_params[1];
	GLint y = (GLint)*(i32*)&_params[2];
	GLint z = (GLint)*(i32*)&_params[3];
	GLint w = (GLint)*(i32*)&_params[4];
	glVertexAttribI4i(index, x, y, z, w);
}

void glVertexAttribI4ui_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint index = (GLuint)*(i32*)&_params[0];
	GLuint x = (GLuint)*(i32*)&_params[1];
	GLuint y = (GLuint)*(i32*)&_params[2];
	GLuint z = (GLuint)*(i32*)&_params[3];
	GLuint w = (GLuint)*(i32*)&_params[4];
	glVertexAttribI4ui(index, x, y, z, w);
}

void glVertexAttribI4iv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint index = (GLuint)*(i32*)&_params[0];
	const GLint * v = (const GLint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)v >= (char*)_mem) && (((char*)v - (char*)_mem) < _memSize), "parameter 'v' is out of bounds");
		OC_ASSERT_DIALOG((char*)v + 4*sizeof(const GLint ) <= ((char*)_mem + _memSize), "parameter 'v' is out of bounds");
	}
	glVertexAttribI4iv(index, v);
}

void glVertexAttribI4uiv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint index = (GLuint)*(i32*)&_params[0];
	const GLuint * v = (const GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)v >= (char*)_mem) && (((char*)v - (char*)_mem) < _memSize), "parameter 'v' is out of bounds");
		OC_ASSERT_DIALOG((char*)v + 4*sizeof(const GLuint ) <= ((char*)_mem + _memSize), "parameter 'v' is out of bounds");
	}
	glVertexAttribI4uiv(index, v);
}

void glGetUniformuiv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLuint * params = (GLuint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetUniformuiv_params_length(instance, program, location)*sizeof(GLuint ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glGetUniformuiv(program, location, params);
}

void glGetFragDataLocation_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	const GLchar * name = (const GLchar *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)name >= (char*)_mem) && (((char*)name - (char*)_mem) < _memSize), "parameter 'name' is out of bounds");
		OC_ASSERT_DIALOG((char*)name + orca_glGetFragDataLocation_name_length(instance, name)*sizeof(const GLchar ) <= ((char*)_mem + _memSize), "parameter 'name' is out of bounds");
	}
	*((i32*)&_returns[0]) = (i32)glGetFragDataLocation(program, name);
}

void glUniform1ui_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLuint v0 = (GLuint)*(i32*)&_params[1];
	glUniform1ui(location, v0);
}

void glUniform2ui_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLuint v0 = (GLuint)*(i32*)&_params[1];
	GLuint v1 = (GLuint)*(i32*)&_params[2];
	glUniform2ui(location, v0, v1);
}

void glUniform3ui_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLuint v0 = (GLuint)*(i32*)&_params[1];
	GLuint v1 = (GLuint)*(i32*)&_params[2];
	GLuint v2 = (GLuint)*(i32*)&_params[3];
	glUniform3ui(location, v0, v1, v2);
}

void glUniform4ui_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLuint v0 = (GLuint)*(i32*)&_params[1];
	GLuint v1 = (GLuint)*(i32*)&_params[2];
	GLuint v2 = (GLuint)*(i32*)&_params[3];
	GLuint v3 = (GLuint)*(i32*)&_params[4];
	glUniform4ui(location, v0, v1, v2, v3);
}

void glUniform1uiv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	const GLuint * value = (const GLuint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 1*count*sizeof(const GLuint ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glUniform1uiv(location, count, value);
}

void glUniform2uiv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	const GLuint * value = (const GLuint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 2*count*sizeof(const GLuint ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glUniform2uiv(location, count, value);
}

void glUniform3uiv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	const GLuint * value = (const GLuint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 3*count*sizeof(const GLuint ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glUniform3uiv(location, count, value);
}

void glUniform4uiv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	const GLuint * value = (const GLuint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 4*count*sizeof(const GLuint ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glUniform4uiv(location, count, value);
}

void glClearBufferiv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum buffer = (GLenum)*(i32*)&_params[0];
	GLint drawbuffer = (GLint)*(i32*)&_params[1];
	const GLint * value = (const GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + orca_glClearBufferiv_value_length(instance, buffer)*sizeof(const GLint ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glClearBufferiv(buffer, drawbuffer, value);
}

void glClearBufferuiv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum buffer = (GLenum)*(i32*)&_params[0];
	GLint drawbuffer = (GLint)*(i32*)&_params[1];
	const GLuint * value = (const GLuint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + orca_glClearBufferuiv_value_length(instance, buffer)*sizeof(const GLuint ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glClearBufferuiv(buffer, drawbuffer, value);
}

void glClearBufferfv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum buffer = (GLenum)*(i32*)&_params[0];
	GLint drawbuffer = (GLint)*(i32*)&_params[1];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + orca_glClearBufferfv_value_length(instance, buffer)*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glClearBufferfv(buffer, drawbuffer, value);
}

void glClearBufferfi_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum buffer = (GLenum)*(i32*)&_params[0];
	GLint drawbuffer = (GLint)*(i32*)&_params[1];
	GLfloat depth = (GLfloat)*(f32*)&_params[2];
	GLint stencil = (GLint)*(i32*)&_params[3];
	glClearBufferfi(buffer, drawbuffer, depth, stencil);
}

void glCopyBufferSubData_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum readTarget = (GLenum)*(i32*)&_params[0];
	GLenum writeTarget = (GLenum)*(i32*)&_params[1];
	GLintptr readOffset = (GLintptr)*(i32*)&_params[2];
	GLintptr writeOffset = (GLintptr)*(i32*)&_params[3];
	GLsizeiptr size = (GLsizeiptr)*(i32*)&_params[4];
	glCopyBufferSubData(readTarget, writeTarget, readOffset, writeOffset, size);
}

void glGetActiveUniformsiv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLsizei uniformCount = (GLsizei)*(i32*)&_params[1];
	const GLuint * uniformIndices = (const GLuint *)((char*)_mem + *(u32*)&_params[2]);
	GLenum pname = (GLenum)*(i32*)&_params[3];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)uniformIndices >= (char*)_mem) && (((char*)uniformIndices - (char*)_mem) < _memSize), "parameter 'uniformIndices' is out of bounds");
		OC_ASSERT_DIALOG((char*)uniformIndices + uniformCount*sizeof(const GLuint ) <= ((char*)_mem + _memSize), "parameter 'uniformIndices' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetActiveUniformsiv_params_length(instance, uniformCount, pname)*sizeof(GLint ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glGetActiveUniformsiv(program, uniformCount, uniformIndices, pname, params);
}

void glGetUniformBlockIndex_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	const GLchar * uniformBlockName = (const GLchar *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)uniformBlockName >= (char*)_mem) && (((char*)uniformBlockName - (char*)_mem) < _memSize), "parameter 'uniformBlockName' is out of bounds");
		OC_ASSERT_DIALOG((char*)uniformBlockName + orca_glGetUniformBlockIndex_uniformBlockName_length(instance, uniformBlockName)*sizeof(const GLchar ) <= ((char*)_mem + _memSize), "parameter 'uniformBlockName' is out of bounds");
	}
	*((i32*)&_returns[0]) = (i32)glGetUniformBlockIndex(program, uniformBlockName);
}

void glGetActiveUniformBlockiv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLuint uniformBlockIndex = (GLuint)*(i32*)&_params[1];
	GLenum pname = (GLenum)*(i32*)&_params[2];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetActiveUniformBlockiv_params_length(instance, program, uniformBlockIndex, pname)*sizeof(GLint ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glGetActiveUniformBlockiv(program, uniformBlockIndex, pname, params);
}

void glGetActiveUniformBlockName_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLuint uniformBlockIndex = (GLuint)*(i32*)&_params[1];
	GLsizei bufSize = (GLsizei)*(i32*)&_params[2];
	GLsizei * length = (GLsizei *)((char*)_mem + *(u32*)&_params[3]);
	GLchar * uniformBlockName = (GLchar *)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)length >= (char*)_mem) && (((char*)length - (char*)_mem) < _memSize), "parameter 'length' is out of bounds");
		OC_ASSERT_DIALOG((char*)length + 1*sizeof(GLsizei ) <= ((char*)_mem + _memSize), "parameter 'length' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)uniformBlockName >= (char*)_mem) && (((char*)uniformBlockName - (char*)_mem) < _memSize), "parameter 'uniformBlockName' is out of bounds");
		OC_ASSERT_DIALOG((char*)uniformBlockName + bufSize*sizeof(GLchar ) <= ((char*)_mem + _memSize), "parameter 'uniformBlockName' is out of bounds");
	}
	glGetActiveUniformBlockName(program, uniformBlockIndex, bufSize, length, uniformBlockName);
}

void glUniformBlockBinding_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLuint uniformBlockIndex = (GLuint)*(i32*)&_params[1];
	GLuint uniformBlockBinding = (GLuint)*(i32*)&_params[2];
	glUniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
}

void glDrawArraysInstanced_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum mode = (GLenum)*(i32*)&_params[0];
	GLint first = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	GLsizei instancecount = (GLsizei)*(i32*)&_params[3];
	glDrawArraysInstanced(mode, first, count, instancecount);
}

void glDrawElementsInstanced_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum mode = (GLenum)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	GLenum type = (GLenum)*(i32*)&_params[2];
	const void * indices = (const void *)((char*)_mem + *(u32*)&_params[3]);
	GLsizei instancecount = (GLsizei)*(i32*)&_params[4];
	{
		OC_ASSERT_DIALOG(((char*)indices >= (char*)_mem) && (((char*)indices - (char*)_mem) < _memSize), "parameter 'indices' is out of bounds");
		OC_ASSERT_DIALOG((char*)indices + orca_glDrawElementsInstanced_indices_length(instance, count, type) <= ((char*)_mem + _memSize), "parameter 'indices' is out of bounds");
	}
	glDrawElementsInstanced(mode, count, type, indices, instancecount);
}

void glFenceSync_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum condition = (GLenum)*(i32*)&_params[0];
	GLbitfield flags = (GLbitfield)*(i32*)&_params[1];
	*((i64*)&_returns[0]) = (i64)glFenceSync(condition, flags);
}

void glIsSync_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLsync sync = (GLsync)*(i64*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glIsSync(sync);
}

void glDeleteSync_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLsync sync = (GLsync)*(i64*)&_params[0];
	glDeleteSync(sync);
}

void glClientWaitSync_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLsync sync = (GLsync)*(i64*)&_params[0];
	GLbitfield flags = (GLbitfield)*(i32*)&_params[1];
	GLuint64 timeout = (GLuint64)*(i64*)&_params[2];
	*((i32*)&_returns[0]) = (i32)glClientWaitSync(sync, flags, timeout);
}

void glWaitSync_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLsync sync = (GLsync)*(i64*)&_params[0];
	GLbitfield flags = (GLbitfield)*(i32*)&_params[1];
	GLuint64 timeout = (GLuint64)*(i64*)&_params[2];
	glWaitSync(sync, flags, timeout);
}

void glGetInteger64v_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum pname = (GLenum)*(i32*)&_params[0];
	GLint64 * data = (GLint64 *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)data >= (char*)_mem) && (((char*)data - (char*)_mem) < _memSize), "parameter 'data' is out of bounds");
		OC_ASSERT_DIALOG((char*)data + orca_glGetInteger64v_data_length(instance, pname)*sizeof(GLint64 ) <= ((char*)_mem + _memSize), "parameter 'data' is out of bounds");
	}
	glGetInteger64v(pname, data);
}

void glGetSynciv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLsync sync = (GLsync)*(i64*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	GLsizei * length = (GLsizei *)((char*)_mem + *(u32*)&_params[3]);
	GLint * values = (GLint *)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)length >= (char*)_mem) && (((char*)length - (char*)_mem) < _memSize), "parameter 'length' is out of bounds");
		OC_ASSERT_DIALOG((char*)length + 1*sizeof(GLsizei ) <= ((char*)_mem + _memSize), "parameter 'length' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)values >= (char*)_mem) && (((char*)values - (char*)_mem) < _memSize), "parameter 'values' is out of bounds");
		OC_ASSERT_DIALOG((char*)values + count*sizeof(GLint ) <= ((char*)_mem + _memSize), "parameter 'values' is out of bounds");
	}
	glGetSynciv(sync, pname, count, length, values);
}

void glGetInteger64i_v_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLuint index = (GLuint)*(i32*)&_params[1];
	GLint64 * data = (GLint64 *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)data >= (char*)_mem) && (((char*)data - (char*)_mem) < _memSize), "parameter 'data' is out of bounds");
		OC_ASSERT_DIALOG((char*)data + orca_glGetInteger64i_v_data_length(instance, target)*sizeof(GLint64 ) <= ((char*)_mem + _memSize), "parameter 'data' is out of bounds");
	}
	glGetInteger64i_v(target, index, data);
}

void glGetBufferParameteri64v_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint64 * params = (GLint64 *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetBufferParameteri64v_params_length(instance, pname)*sizeof(GLint64 ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glGetBufferParameteri64v(target, pname, params);
}

void glGenSamplers_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLsizei count = (GLsizei)*(i32*)&_params[0];
	GLuint * samplers = (GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)samplers >= (char*)_mem) && (((char*)samplers - (char*)_mem) < _memSize), "parameter 'samplers' is out of bounds");
		OC_ASSERT_DIALOG((char*)samplers + count*sizeof(GLuint ) <= ((char*)_mem + _memSize), "parameter 'samplers' is out of bounds");
	}
	glGenSamplers(count, samplers);
}

void glDeleteSamplers_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLsizei count = (GLsizei)*(i32*)&_params[0];
	const GLuint * samplers = (const GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)samplers >= (char*)_mem) && (((char*)samplers - (char*)_mem) < _memSize), "parameter 'samplers' is out of bounds");
		OC_ASSERT_DIALOG((char*)samplers + count*sizeof(const GLuint ) <= ((char*)_mem + _memSize), "parameter 'samplers' is out of bounds");
	}
	glDeleteSamplers(count, samplers);
}

void glIsSampler_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint sampler = (GLuint)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glIsSampler(sampler);
}

void glBindSampler_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint unit = (GLuint)*(i32*)&_params[0];
	GLuint sampler = (GLuint)*(i32*)&_params[1];
	glBindSampler(unit, sampler);
}

void glSamplerParameteri_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint sampler = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint param = (GLint)*(i32*)&_params[2];
	glSamplerParameteri(sampler, pname, param);
}

void glSamplerParameteriv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint sampler = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	const GLint * param = (const GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)param >= (char*)_mem) && (((char*)param - (char*)_mem) < _memSize), "parameter 'param' is out of bounds");
		OC_ASSERT_DIALOG((char*)param + orca_glSamplerParameteriv_param_length(instance, pname)*sizeof(const GLint ) <= ((char*)_mem + _memSize), "parameter 'param' is out of bounds");
	}
	glSamplerParameteriv(sampler, pname, param);
}

void glSamplerParameterf_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint sampler = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLfloat param = (GLfloat)*(f32*)&_params[2];
	glSamplerParameterf(sampler, pname, param);
}

void glSamplerParameterfv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint sampler = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	const GLfloat * param = (const GLfloat *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)param >= (char*)_mem) && (((char*)param - (char*)_mem) < _memSize), "parameter 'param' is out of bounds");
		OC_ASSERT_DIALOG((char*)param + orca_glSamplerParameterfv_param_length(instance, pname)*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'param' is out of bounds");
	}
	glSamplerParameterfv(sampler, pname, param);
}

void glGetSamplerParameteriv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint sampler = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetSamplerParameteriv_params_length(instance, pname)*sizeof(GLint ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glGetSamplerParameteriv(sampler, pname, params);
}

void glGetSamplerParameterfv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint sampler = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLfloat * params = (GLfloat *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetSamplerParameterfv_params_length(instance, pname)*sizeof(GLfloat ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glGetSamplerParameterfv(sampler, pname, params);
}

void glVertexAttribDivisor_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint index = (GLuint)*(i32*)&_params[0];
	GLuint divisor = (GLuint)*(i32*)&_params[1];
	glVertexAttribDivisor(index, divisor);
}

void glBindTransformFeedback_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLuint id = (GLuint)*(i32*)&_params[1];
	glBindTransformFeedback(target, id);
}

void glDeleteTransformFeedbacks_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	const GLuint * ids = (const GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)ids >= (char*)_mem) && (((char*)ids - (char*)_mem) < _memSize), "parameter 'ids' is out of bounds");
		OC_ASSERT_DIALOG((char*)ids + n*sizeof(const GLuint ) <= ((char*)_mem + _memSize), "parameter 'ids' is out of bounds");
	}
	glDeleteTransformFeedbacks(n, ids);
}

void glGenTransformFeedbacks_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	GLuint * ids = (GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)ids >= (char*)_mem) && (((char*)ids - (char*)_mem) < _memSize), "parameter 'ids' is out of bounds");
		OC_ASSERT_DIALOG((char*)ids + n*sizeof(GLuint ) <= ((char*)_mem + _memSize), "parameter 'ids' is out of bounds");
	}
	glGenTransformFeedbacks(n, ids);
}

void glIsTransformFeedback_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint id = (GLuint)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glIsTransformFeedback(id);
}

void glPauseTransformFeedback_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	glPauseTransformFeedback();
}

void glResumeTransformFeedback_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	glResumeTransformFeedback();
}

void glGetProgramBinary_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLsizei bufSize = (GLsizei)*(i32*)&_params[1];
	GLsizei * length = (GLsizei *)((char*)_mem + *(u32*)&_params[2]);
	GLenum * binaryFormat = (GLenum *)((char*)_mem + *(u32*)&_params[3]);
	void * binary = (void *)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)length >= (char*)_mem) && (((char*)length - (char*)_mem) < _memSize), "parameter 'length' is out of bounds");
		OC_ASSERT_DIALOG((char*)length + 1*sizeof(GLsizei ) <= ((char*)_mem + _memSize), "parameter 'length' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)binaryFormat >= (char*)_mem) && (((char*)binaryFormat - (char*)_mem) < _memSize), "parameter 'binaryFormat' is out of bounds");
		OC_ASSERT_DIALOG((char*)binaryFormat + 1*sizeof(GLenum ) <= ((char*)_mem + _memSize), "parameter 'binaryFormat' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)binary >= (char*)_mem) && (((char*)binary - (char*)_mem) < _memSize), "parameter 'binary' is out of bounds");
		OC_ASSERT_DIALOG((char*)binary + bufSize <= ((char*)_mem + _memSize), "parameter 'binary' is out of bounds");
	}
	glGetProgramBinary(program, bufSize, length, binaryFormat, binary);
}

void glProgramBinary_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLenum binaryFormat = (GLenum)*(i32*)&_params[1];
	const void * binary = (const void *)((char*)_mem + *(u32*)&_params[2]);
	GLsizei length = (GLsizei)*(i32*)&_params[3];
	{
		OC_ASSERT_DIALOG(((char*)binary >= (char*)_mem) && (((char*)binary - (char*)_mem) < _memSize), "parameter 'binary' is out of bounds");
		OC_ASSERT_DIALOG((char*)binary + length <= ((char*)_mem + _memSize), "parameter 'binary' is out of bounds");
	}
	glProgramBinary(program, binaryFormat, binary, length);
}

void glProgramParameteri_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint value = (GLint)*(i32*)&_params[2];
	glProgramParameteri(program, pname, value);
}

void glInvalidateFramebuffer_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLsizei numAttachments = (GLsizei)*(i32*)&_params[1];
	const GLenum * attachments = (const GLenum *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)attachments >= (char*)_mem) && (((char*)attachments - (char*)_mem) < _memSize), "parameter 'attachments' is out of bounds");
		OC_ASSERT_DIALOG((char*)attachments + numAttachments*sizeof(const GLenum ) <= ((char*)_mem + _memSize), "parameter 'attachments' is out of bounds");
	}
	glInvalidateFramebuffer(target, numAttachments, attachments);
}

void glInvalidateSubFramebuffer_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLsizei numAttachments = (GLsizei)*(i32*)&_params[1];
	const GLenum * attachments = (const GLenum *)((char*)_mem + *(u32*)&_params[2]);
	GLint x = (GLint)*(i32*)&_params[3];
	GLint y = (GLint)*(i32*)&_params[4];
	GLsizei width = (GLsizei)*(i32*)&_params[5];
	GLsizei height = (GLsizei)*(i32*)&_params[6];
	{
		OC_ASSERT_DIALOG(((char*)attachments >= (char*)_mem) && (((char*)attachments - (char*)_mem) < _memSize), "parameter 'attachments' is out of bounds");
		OC_ASSERT_DIALOG((char*)attachments + numAttachments*sizeof(const GLenum ) <= ((char*)_mem + _memSize), "parameter 'attachments' is out of bounds");
	}
	glInvalidateSubFramebuffer(target, numAttachments, attachments, x, y, width, height);
}

void glTexStorage2D_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLsizei levels = (GLsizei)*(i32*)&_params[1];
	GLenum internalformat = (GLenum)*(i32*)&_params[2];
	GLsizei width = (GLsizei)*(i32*)&_params[3];
	GLsizei height = (GLsizei)*(i32*)&_params[4];
	glTexStorage2D(target, levels, internalformat, width, height);
}

void glTexStorage3D_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLsizei levels = (GLsizei)*(i32*)&_params[1];
	GLenum internalformat = (GLenum)*(i32*)&_params[2];
	GLsizei width = (GLsizei)*(i32*)&_params[3];
	GLsizei height = (GLsizei)*(i32*)&_params[4];
	GLsizei depth = (GLsizei)*(i32*)&_params[5];
	glTexStorage3D(target, levels, internalformat, width, height, depth);
}

void glGetInternalformativ_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum internalformat = (GLenum)*(i32*)&_params[1];
	GLenum pname = (GLenum)*(i32*)&_params[2];
	GLsizei count = (GLsizei)*(i32*)&_params[3];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + count*sizeof(GLint ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glGetInternalformativ(target, internalformat, pname, count, params);
}

void glDispatchCompute_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint num_groups_x = (GLuint)*(i32*)&_params[0];
	GLuint num_groups_y = (GLuint)*(i32*)&_params[1];
	GLuint num_groups_z = (GLuint)*(i32*)&_params[2];
	glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
}

void glDispatchComputeIndirect_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLintptr indirect = (GLintptr)*(i32*)&_params[0];
	glDispatchComputeIndirect(indirect);
}

void glDrawArraysIndirect_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum mode = (GLenum)*(i32*)&_params[0];
	const void * indirect = (const void *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)indirect >= (char*)_mem) && (((char*)indirect - (char*)_mem) < _memSize), "parameter 'indirect' is out of bounds");
		OC_ASSERT_DIALOG((char*)indirect + orca_glDrawArraysIndirect_indirect_length(instance, indirect) <= ((char*)_mem + _memSize), "parameter 'indirect' is out of bounds");
	}
	glDrawArraysIndirect(mode, indirect);
}

void glDrawElementsIndirect_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum mode = (GLenum)*(i32*)&_params[0];
	GLenum type = (GLenum)*(i32*)&_params[1];
	const void * indirect = (const void *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)indirect >= (char*)_mem) && (((char*)indirect - (char*)_mem) < _memSize), "parameter 'indirect' is out of bounds");
		OC_ASSERT_DIALOG((char*)indirect + orca_glDrawElementsIndirect_indirect_length(instance, indirect) <= ((char*)_mem + _memSize), "parameter 'indirect' is out of bounds");
	}
	glDrawElementsIndirect(mode, type, indirect);
}

void glFramebufferParameteri_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint param = (GLint)*(i32*)&_params[2];
	glFramebufferParameteri(target, pname, param);
}

void glGetFramebufferParameteriv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetFramebufferParameteriv_params_length(instance, pname)*sizeof(GLint ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glGetFramebufferParameteriv(target, pname, params);
}

void glGetProgramInterfaceiv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLenum programInterface = (GLenum)*(i32*)&_params[1];
	GLenum pname = (GLenum)*(i32*)&_params[2];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetProgramInterfaceiv_params_length(instance, pname)*sizeof(GLint ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glGetProgramInterfaceiv(program, programInterface, pname, params);
}

void glGetProgramResourceIndex_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLenum programInterface = (GLenum)*(i32*)&_params[1];
	const GLchar * name = (const GLchar *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)name >= (char*)_mem) && (((char*)name - (char*)_mem) < _memSize), "parameter 'name' is out of bounds");
		OC_ASSERT_DIALOG((char*)name + orca_glGetProgramResourceIndex_name_length(instance, name)*sizeof(const GLchar ) <= ((char*)_mem + _memSize), "parameter 'name' is out of bounds");
	}
	*((i32*)&_returns[0]) = (i32)glGetProgramResourceIndex(program, programInterface, name);
}

void glGetProgramResourceName_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLenum programInterface = (GLenum)*(i32*)&_params[1];
	GLuint index = (GLuint)*(i32*)&_params[2];
	GLsizei bufSize = (GLsizei)*(i32*)&_params[3];
	GLsizei * length = (GLsizei *)((char*)_mem + *(u32*)&_params[4]);
	GLchar * name = (GLchar *)((char*)_mem + *(u32*)&_params[5]);
	{
		OC_ASSERT_DIALOG(((char*)length >= (char*)_mem) && (((char*)length - (char*)_mem) < _memSize), "parameter 'length' is out of bounds");
		OC_ASSERT_DIALOG((char*)length + 1*sizeof(GLsizei ) <= ((char*)_mem + _memSize), "parameter 'length' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)name >= (char*)_mem) && (((char*)name - (char*)_mem) < _memSize), "parameter 'name' is out of bounds");
		OC_ASSERT_DIALOG((char*)name + bufSize*sizeof(GLchar ) <= ((char*)_mem + _memSize), "parameter 'name' is out of bounds");
	}
	glGetProgramResourceName(program, programInterface, index, bufSize, length, name);
}

void glGetProgramResourceiv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLenum programInterface = (GLenum)*(i32*)&_params[1];
	GLuint index = (GLuint)*(i32*)&_params[2];
	GLsizei propCount = (GLsizei)*(i32*)&_params[3];
	const GLenum * props = (const GLenum *)((char*)_mem + *(u32*)&_params[4]);
	GLsizei count = (GLsizei)*(i32*)&_params[5];
	GLsizei * length = (GLsizei *)((char*)_mem + *(u32*)&_params[6]);
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[7]);
	{
		OC_ASSERT_DIALOG(((char*)props >= (char*)_mem) && (((char*)props - (char*)_mem) < _memSize), "parameter 'props' is out of bounds");
		OC_ASSERT_DIALOG((char*)props + propCount*sizeof(const GLenum ) <= ((char*)_mem + _memSize), "parameter 'props' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)length >= (char*)_mem) && (((char*)length - (char*)_mem) < _memSize), "parameter 'length' is out of bounds");
		OC_ASSERT_DIALOG((char*)length + 1*sizeof(GLsizei ) <= ((char*)_mem + _memSize), "parameter 'length' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + count*sizeof(GLint ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glGetProgramResourceiv(program, programInterface, index, propCount, props, count, length, params);
}

void glGetProgramResourceLocation_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLenum programInterface = (GLenum)*(i32*)&_params[1];
	const GLchar * name = (const GLchar *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)name >= (char*)_mem) && (((char*)name - (char*)_mem) < _memSize), "parameter 'name' is out of bounds");
		OC_ASSERT_DIALOG((char*)name + orca_glGetProgramResourceLocation_name_length(instance, name)*sizeof(const GLchar ) <= ((char*)_mem + _memSize), "parameter 'name' is out of bounds");
	}
	*((i32*)&_returns[0]) = (i32)glGetProgramResourceLocation(program, programInterface, name);
}

void glUseProgramStages_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint pipeline = (GLuint)*(i32*)&_params[0];
	GLbitfield stages = (GLbitfield)*(i32*)&_params[1];
	GLuint program = (GLuint)*(i32*)&_params[2];
	glUseProgramStages(pipeline, stages, program);
}

void glActiveShaderProgram_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint pipeline = (GLuint)*(i32*)&_params[0];
	GLuint program = (GLuint)*(i32*)&_params[1];
	glActiveShaderProgram(pipeline, program);
}

void glCreateShaderProgramv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum type = (GLenum)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	const GLchar *const* strings = (const GLchar *const*)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)strings >= (char*)_mem) && (((char*)strings - (char*)_mem) < _memSize), "parameter 'strings' is out of bounds");
		OC_ASSERT_DIALOG((char*)strings + count*sizeof(const GLchar *const) <= ((char*)_mem + _memSize), "parameter 'strings' is out of bounds");
	}
	*((i32*)&_returns[0]) = (i32)glCreateShaderProgramv(type, count, strings);
}

void glBindProgramPipeline_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint pipeline = (GLuint)*(i32*)&_params[0];
	glBindProgramPipeline(pipeline);
}

void glDeleteProgramPipelines_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	const GLuint * pipelines = (const GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)pipelines >= (char*)_mem) && (((char*)pipelines - (char*)_mem) < _memSize), "parameter 'pipelines' is out of bounds");
		OC_ASSERT_DIALOG((char*)pipelines + n*sizeof(const GLuint ) <= ((char*)_mem + _memSize), "parameter 'pipelines' is out of bounds");
	}
	glDeleteProgramPipelines(n, pipelines);
}

void glGenProgramPipelines_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	GLuint * pipelines = (GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)pipelines >= (char*)_mem) && (((char*)pipelines - (char*)_mem) < _memSize), "parameter 'pipelines' is out of bounds");
		OC_ASSERT_DIALOG((char*)pipelines + n*sizeof(GLuint ) <= ((char*)_mem + _memSize), "parameter 'pipelines' is out of bounds");
	}
	glGenProgramPipelines(n, pipelines);
}

void glIsProgramPipeline_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint pipeline = (GLuint)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glIsProgramPipeline(pipeline);
}

void glGetProgramPipelineiv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint pipeline = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetProgramPipelineiv_params_length(instance, pname)*sizeof(GLint ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glGetProgramPipelineiv(pipeline, pname, params);
}

void glProgramUniform1i_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLint v0 = (GLint)*(i32*)&_params[2];
	glProgramUniform1i(program, location, v0);
}

void glProgramUniform2i_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLint v0 = (GLint)*(i32*)&_params[2];
	GLint v1 = (GLint)*(i32*)&_params[3];
	glProgramUniform2i(program, location, v0, v1);
}

void glProgramUniform3i_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLint v0 = (GLint)*(i32*)&_params[2];
	GLint v1 = (GLint)*(i32*)&_params[3];
	GLint v2 = (GLint)*(i32*)&_params[4];
	glProgramUniform3i(program, location, v0, v1, v2);
}

void glProgramUniform4i_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLint v0 = (GLint)*(i32*)&_params[2];
	GLint v1 = (GLint)*(i32*)&_params[3];
	GLint v2 = (GLint)*(i32*)&_params[4];
	GLint v3 = (GLint)*(i32*)&_params[5];
	glProgramUniform4i(program, location, v0, v1, v2, v3);
}

void glProgramUniform1ui_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLuint v0 = (GLuint)*(i32*)&_params[2];
	glProgramUniform1ui(program, location, v0);
}

void glProgramUniform2ui_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLuint v0 = (GLuint)*(i32*)&_params[2];
	GLuint v1 = (GLuint)*(i32*)&_params[3];
	glProgramUniform2ui(program, location, v0, v1);
}

void glProgramUniform3ui_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLuint v0 = (GLuint)*(i32*)&_params[2];
	GLuint v1 = (GLuint)*(i32*)&_params[3];
	GLuint v2 = (GLuint)*(i32*)&_params[4];
	glProgramUniform3ui(program, location, v0, v1, v2);
}

void glProgramUniform4ui_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLuint v0 = (GLuint)*(i32*)&_params[2];
	GLuint v1 = (GLuint)*(i32*)&_params[3];
	GLuint v2 = (GLuint)*(i32*)&_params[4];
	GLuint v3 = (GLuint)*(i32*)&_params[5];
	glProgramUniform4ui(program, location, v0, v1, v2, v3);
}

void glProgramUniform1f_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLfloat v0 = (GLfloat)*(f32*)&_params[2];
	glProgramUniform1f(program, location, v0);
}

void glProgramUniform2f_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLfloat v0 = (GLfloat)*(f32*)&_params[2];
	GLfloat v1 = (GLfloat)*(f32*)&_params[3];
	glProgramUniform2f(program, location, v0, v1);
}

void glProgramUniform3f_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLfloat v0 = (GLfloat)*(f32*)&_params[2];
	GLfloat v1 = (GLfloat)*(f32*)&_params[3];
	GLfloat v2 = (GLfloat)*(f32*)&_params[4];
	glProgramUniform3f(program, location, v0, v1, v2);
}

void glProgramUniform4f_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLfloat v0 = (GLfloat)*(f32*)&_params[2];
	GLfloat v1 = (GLfloat)*(f32*)&_params[3];
	GLfloat v2 = (GLfloat)*(f32*)&_params[4];
	GLfloat v3 = (GLfloat)*(f32*)&_params[5];
	glProgramUniform4f(program, location, v0, v1, v2, v3);
}

void glProgramUniform1iv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	const GLint * value = (const GLint *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + count*sizeof(const GLint ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glProgramUniform1iv(program, location, count, value);
}

void glProgramUniform2iv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	const GLint * value = (const GLint *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 2*count*sizeof(const GLint ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glProgramUniform2iv(program, location, count, value);
}

void glProgramUniform3iv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	const GLint * value = (const GLint *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 3*count*sizeof(const GLint ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glProgramUniform3iv(program, location, count, value);
}

void glProgramUniform4iv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	const GLint * value = (const GLint *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 4*count*sizeof(const GLint ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glProgramUniform4iv(program, location, count, value);
}

void glProgramUniform1uiv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	const GLuint * value = (const GLuint *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + count*sizeof(const GLuint ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glProgramUniform1uiv(program, location, count, value);
}

void glProgramUniform2uiv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	const GLuint * value = (const GLuint *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 2*count*sizeof(const GLuint ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glProgramUniform2uiv(program, location, count, value);
}

void glProgramUniform3uiv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	const GLuint * value = (const GLuint *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 3*count*sizeof(const GLuint ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glProgramUniform3uiv(program, location, count, value);
}

void glProgramUniform4uiv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	const GLuint * value = (const GLuint *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 4*count*sizeof(const GLuint ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glProgramUniform4uiv(program, location, count, value);
}

void glProgramUniform1fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + count*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glProgramUniform1fv(program, location, count, value);
}

void glProgramUniform2fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 2*count*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glProgramUniform2fv(program, location, count, value);
}

void glProgramUniform3fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 3*count*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glProgramUniform3fv(program, location, count, value);
}

void glProgramUniform4fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 4*count*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glProgramUniform4fv(program, location, count, value);
}

void glProgramUniformMatrix2fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	GLboolean transpose = (GLboolean)*(i32*)&_params[3];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 4*count*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glProgramUniformMatrix2fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix3fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	GLboolean transpose = (GLboolean)*(i32*)&_params[3];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 9*count*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glProgramUniformMatrix3fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix4fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	GLboolean transpose = (GLboolean)*(i32*)&_params[3];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 16*count*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glProgramUniformMatrix4fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix2x3fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	GLboolean transpose = (GLboolean)*(i32*)&_params[3];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 6*count*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glProgramUniformMatrix2x3fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix3x2fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	GLboolean transpose = (GLboolean)*(i32*)&_params[3];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 6*count*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glProgramUniformMatrix3x2fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix2x4fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	GLboolean transpose = (GLboolean)*(i32*)&_params[3];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 8*count*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glProgramUniformMatrix2x4fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix4x2fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	GLboolean transpose = (GLboolean)*(i32*)&_params[3];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 8*count*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glProgramUniformMatrix4x2fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix3x4fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	GLboolean transpose = (GLboolean)*(i32*)&_params[3];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 12*count*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glProgramUniformMatrix3x4fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix4x3fv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	GLboolean transpose = (GLboolean)*(i32*)&_params[3];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < _memSize), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 12*count*sizeof(const GLfloat ) <= ((char*)_mem + _memSize), "parameter 'value' is out of bounds");
	}
	glProgramUniformMatrix4x3fv(program, location, count, transpose, value);
}

void glValidateProgramPipeline_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint pipeline = (GLuint)*(i32*)&_params[0];
	glValidateProgramPipeline(pipeline);
}

void glGetProgramPipelineInfoLog_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint pipeline = (GLuint)*(i32*)&_params[0];
	GLsizei bufSize = (GLsizei)*(i32*)&_params[1];
	GLsizei * length = (GLsizei *)((char*)_mem + *(u32*)&_params[2]);
	GLchar * infoLog = (GLchar *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)length >= (char*)_mem) && (((char*)length - (char*)_mem) < _memSize), "parameter 'length' is out of bounds");
		OC_ASSERT_DIALOG((char*)length + 1*sizeof(GLsizei ) <= ((char*)_mem + _memSize), "parameter 'length' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)infoLog >= (char*)_mem) && (((char*)infoLog - (char*)_mem) < _memSize), "parameter 'infoLog' is out of bounds");
		OC_ASSERT_DIALOG((char*)infoLog + bufSize*sizeof(GLchar ) <= ((char*)_mem + _memSize), "parameter 'infoLog' is out of bounds");
	}
	glGetProgramPipelineInfoLog(pipeline, bufSize, length, infoLog);
}

void glBindImageTexture_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint unit = (GLuint)*(i32*)&_params[0];
	GLuint texture = (GLuint)*(i32*)&_params[1];
	GLint level = (GLint)*(i32*)&_params[2];
	GLboolean layered = (GLboolean)*(i32*)&_params[3];
	GLint layer = (GLint)*(i32*)&_params[4];
	GLenum access = (GLenum)*(i32*)&_params[5];
	GLenum format = (GLenum)*(i32*)&_params[6];
	glBindImageTexture(unit, texture, level, layered, layer, access, format);
}

void glGetBooleani_v_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLuint index = (GLuint)*(i32*)&_params[1];
	GLboolean * data = (GLboolean *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)data >= (char*)_mem) && (((char*)data - (char*)_mem) < _memSize), "parameter 'data' is out of bounds");
		OC_ASSERT_DIALOG((char*)data + orca_glGetBooleani_v_data_length(instance, target)*sizeof(GLboolean ) <= ((char*)_mem + _memSize), "parameter 'data' is out of bounds");
	}
	glGetBooleani_v(target, index, data);
}

void glMemoryBarrier_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLbitfield barriers = (GLbitfield)*(i32*)&_params[0];
	glMemoryBarrier(barriers);
}

void glMemoryBarrierByRegion_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLbitfield barriers = (GLbitfield)*(i32*)&_params[0];
	glMemoryBarrierByRegion(barriers);
}

void glTexStorage2DMultisample_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLsizei samples = (GLsizei)*(i32*)&_params[1];
	GLenum internalformat = (GLenum)*(i32*)&_params[2];
	GLsizei width = (GLsizei)*(i32*)&_params[3];
	GLsizei height = (GLsizei)*(i32*)&_params[4];
	GLboolean fixedsamplelocations = (GLboolean)*(i32*)&_params[5];
	glTexStorage2DMultisample(target, samples, internalformat, width, height, fixedsamplelocations);
}

void glGetMultisamplefv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum pname = (GLenum)*(i32*)&_params[0];
	GLuint index = (GLuint)*(i32*)&_params[1];
	GLfloat * val = (GLfloat *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)val >= (char*)_mem) && (((char*)val - (char*)_mem) < _memSize), "parameter 'val' is out of bounds");
		OC_ASSERT_DIALOG((char*)val + orca_glGetMultisamplefv_val_length(instance, pname)*sizeof(GLfloat ) <= ((char*)_mem + _memSize), "parameter 'val' is out of bounds");
	}
	glGetMultisamplefv(pname, index, val);
}

void glSampleMaski_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint maskNumber = (GLuint)*(i32*)&_params[0];
	GLbitfield mask = (GLbitfield)*(i32*)&_params[1];
	glSampleMaski(maskNumber, mask);
}

void glGetTexLevelParameteriv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLint level = (GLint)*(i32*)&_params[1];
	GLenum pname = (GLenum)*(i32*)&_params[2];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetTexLevelParameteriv_params_length(instance, pname)*sizeof(GLint ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glGetTexLevelParameteriv(target, level, pname, params);
}

void glGetTexLevelParameterfv_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLint level = (GLint)*(i32*)&_params[1];
	GLenum pname = (GLenum)*(i32*)&_params[2];
	GLfloat * params = (GLfloat *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < _memSize), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetTexLevelParameterfv_params_length(instance, pname)*sizeof(GLfloat ) <= ((char*)_mem + _memSize), "parameter 'params' is out of bounds");
	}
	glGetTexLevelParameterfv(target, level, pname, params);
}

void glBindVertexBuffer_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint bindingindex = (GLuint)*(i32*)&_params[0];
	GLuint buffer = (GLuint)*(i32*)&_params[1];
	GLintptr offset = (GLintptr)*(i32*)&_params[2];
	GLsizei stride = (GLsizei)*(i32*)&_params[3];
	glBindVertexBuffer(bindingindex, buffer, offset, stride);
}

void glVertexAttribFormat_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint attribindex = (GLuint)*(i32*)&_params[0];
	GLint size = (GLint)*(i32*)&_params[1];
	GLenum type = (GLenum)*(i32*)&_params[2];
	GLboolean normalized = (GLboolean)*(i32*)&_params[3];
	GLuint relativeoffset = (GLuint)*(i32*)&_params[4];
	glVertexAttribFormat(attribindex, size, type, normalized, relativeoffset);
}

void glVertexAttribIFormat_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint attribindex = (GLuint)*(i32*)&_params[0];
	GLint size = (GLint)*(i32*)&_params[1];
	GLenum type = (GLenum)*(i32*)&_params[2];
	GLuint relativeoffset = (GLuint)*(i32*)&_params[3];
	glVertexAttribIFormat(attribindex, size, type, relativeoffset);
}

void glVertexAttribBinding_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint attribindex = (GLuint)*(i32*)&_params[0];
	GLuint bindingindex = (GLuint)*(i32*)&_params[1];
	glVertexAttribBinding(attribindex, bindingindex);
}

void glVertexBindingDivisor_stub(wa_interpreter* interpreter, wa_value* _params, wa_value* _returns, void* user)
{
	wa_instance* instance = wa_interpreter_current_instance(interpreter);
	oc_str8 memStr8 = wa_instance_get_memory_str8(instance);
 	char* _mem = memStr8.ptr;
 	u32 _memSize = memStr8.len;
	GLuint bindingindex = (GLuint)*(i32*)&_params[0];
	GLuint divisor = (GLuint)*(i32*)&_params[1];
	glVertexBindingDivisor(bindingindex, divisor);
}

int bindgen_link_gles_api(oc_arena* arena, wa_import_package* package)
{
	wa_status status;
	int ret = 0;

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glActiveTexture");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glActiveTexture_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glAttachShader");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glAttachShader_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glBindAttribLocation");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glBindAttribLocation_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glBindBuffer");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glBindBuffer_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glBindFramebuffer");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glBindFramebuffer_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glBindRenderbuffer");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glBindRenderbuffer_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glBindTexture");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glBindTexture_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_F32, WA_TYPE_F32, WA_TYPE_F32, WA_TYPE_F32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glBlendColor");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glBlendColor_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glBlendEquation");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glBlendEquation_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glBlendEquationSeparate");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glBlendEquationSeparate_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glBlendFunc");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glBlendFunc_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glBlendFuncSeparate");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glBlendFuncSeparate_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glBufferData");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glBufferData_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glBufferSubData");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glBufferSubData_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[] = { WA_TYPE_I32 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glCheckFramebufferStatus");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glCheckFramebufferStatus_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glClear");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glClear_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_F32, WA_TYPE_F32, WA_TYPE_F32, WA_TYPE_F32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glClearColor");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glClearColor_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_F32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glClearDepthf");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glClearDepthf_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glClearStencil");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glClearStencil_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glColorMask");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glColorMask_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glCompileShader");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glCompileShader_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glCompressedTexImage2D");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glCompressedTexImage2D_stub;
		binding.hostFunction.type.paramCount = 8;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glCompressedTexSubImage2D");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glCompressedTexSubImage2D_stub;
		binding.hostFunction.type.paramCount = 9;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glCopyTexImage2D");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glCopyTexImage2D_stub;
		binding.hostFunction.type.paramCount = 8;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glCopyTexSubImage2D");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glCopyTexSubImage2D_stub;
		binding.hostFunction.type.paramCount = 8;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[1];
		wa_value_type returnTypes[] = { WA_TYPE_I32 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glCreateProgram");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glCreateProgram_stub;
		binding.hostFunction.type.paramCount = 0;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[] = { WA_TYPE_I32 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glCreateShader");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glCreateShader_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glCullFace");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glCullFace_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glDeleteBuffers");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glDeleteBuffers_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glDeleteFramebuffers");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glDeleteFramebuffers_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glDeleteProgram");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glDeleteProgram_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glDeleteRenderbuffers");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glDeleteRenderbuffers_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glDeleteShader");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glDeleteShader_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glDeleteTextures");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glDeleteTextures_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glDepthFunc");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glDepthFunc_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glDepthMask");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glDepthMask_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_F32, WA_TYPE_F32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glDepthRangef");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glDepthRangef_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glDetachShader");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glDetachShader_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glDisable");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glDisable_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glDisableVertexAttribArray");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glDisableVertexAttribArray_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glDrawArrays");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glDrawArrays_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glDrawElements");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glDrawElements_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glEnable");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glEnable_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glEnableVertexAttribArray");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glEnableVertexAttribArray_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[1];
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glFinish");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glFinish_stub;
		binding.hostFunction.type.paramCount = 0;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[1];
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glFlush");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glFlush_stub;
		binding.hostFunction.type.paramCount = 0;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glFramebufferRenderbuffer");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glFramebufferRenderbuffer_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glFramebufferTexture2D");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glFramebufferTexture2D_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glFrontFace");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glFrontFace_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGenBuffers");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGenBuffers_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGenerateMipmap");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGenerateMipmap_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGenFramebuffers");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGenFramebuffers_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGenRenderbuffers");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGenRenderbuffers_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGenTextures");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGenTextures_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetActiveAttrib");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetActiveAttrib_stub;
		binding.hostFunction.type.paramCount = 7;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetActiveUniform");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetActiveUniform_stub;
		binding.hostFunction.type.paramCount = 7;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetAttachedShaders");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetAttachedShaders_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[] = { WA_TYPE_I32 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetAttribLocation");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetAttribLocation_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetBooleanv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetBooleanv_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetBufferParameteriv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetBufferParameteriv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[1];
		wa_value_type returnTypes[] = { WA_TYPE_I32 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetError");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetError_stub;
		binding.hostFunction.type.paramCount = 0;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetFloatv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetFloatv_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetFramebufferAttachmentParameteriv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetFramebufferAttachmentParameteriv_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetIntegerv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetIntegerv_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetProgramiv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetProgramiv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetProgramInfoLog");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetProgramInfoLog_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetRenderbufferParameteriv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetRenderbufferParameteriv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetShaderiv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetShaderiv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetShaderInfoLog");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetShaderInfoLog_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetShaderPrecisionFormat");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetShaderPrecisionFormat_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetShaderSource");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetShaderSource_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetTexParameterfv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetTexParameterfv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetTexParameteriv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetTexParameteriv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetUniformfv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetUniformfv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetUniformiv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetUniformiv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[] = { WA_TYPE_I32 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetUniformLocation");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetUniformLocation_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetVertexAttribfv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetVertexAttribfv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetVertexAttribiv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetVertexAttribiv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glHint");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glHint_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[] = { WA_TYPE_I32 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glIsBuffer");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glIsBuffer_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[] = { WA_TYPE_I32 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glIsEnabled");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glIsEnabled_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[] = { WA_TYPE_I32 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glIsFramebuffer");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glIsFramebuffer_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[] = { WA_TYPE_I32 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glIsProgram");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glIsProgram_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[] = { WA_TYPE_I32 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glIsRenderbuffer");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glIsRenderbuffer_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[] = { WA_TYPE_I32 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glIsShader");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glIsShader_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[] = { WA_TYPE_I32 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glIsTexture");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glIsTexture_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_F32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glLineWidth");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glLineWidth_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glLinkProgram");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glLinkProgram_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glPixelStorei");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glPixelStorei_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_F32, WA_TYPE_F32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glPolygonOffset");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glPolygonOffset_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glReadPixels");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glReadPixels_stub;
		binding.hostFunction.type.paramCount = 7;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[1];
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glReleaseShaderCompiler");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glReleaseShaderCompiler_stub;
		binding.hostFunction.type.paramCount = 0;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glRenderbufferStorage");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glRenderbufferStorage_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_F32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glSampleCoverage");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glSampleCoverage_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glScissor");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glScissor_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glShaderBinary");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glShaderBinary_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glStencilFunc");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glStencilFunc_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glStencilFuncSeparate");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glStencilFuncSeparate_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glStencilMask");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glStencilMask_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glStencilMaskSeparate");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glStencilMaskSeparate_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glStencilOp");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glStencilOp_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glStencilOpSeparate");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glStencilOpSeparate_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glTexImage2D");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glTexImage2D_stub;
		binding.hostFunction.type.paramCount = 9;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_F32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glTexParameterf");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glTexParameterf_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glTexParameterfv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glTexParameterfv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glTexParameteri");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glTexParameteri_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glTexParameteriv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glTexParameteriv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glTexSubImage2D");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glTexSubImage2D_stub;
		binding.hostFunction.type.paramCount = 9;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_F32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniform1f");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniform1f_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniform1fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniform1fv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniform1i");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniform1i_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniform1iv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniform1iv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_F32, WA_TYPE_F32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniform2f");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniform2f_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniform2fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniform2fv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniform2i");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniform2i_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniform2iv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniform2iv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_F32, WA_TYPE_F32, WA_TYPE_F32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniform3f");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniform3f_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniform3fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniform3fv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniform3i");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniform3i_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniform3iv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniform3iv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_F32, WA_TYPE_F32, WA_TYPE_F32, WA_TYPE_F32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniform4f");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniform4f_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniform4fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniform4fv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniform4i");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniform4i_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniform4iv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniform4iv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniformMatrix2fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniformMatrix2fv_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniformMatrix3fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniformMatrix3fv_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniformMatrix4fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniformMatrix4fv_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUseProgram");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUseProgram_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glValidateProgram");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glValidateProgram_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_F32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glVertexAttrib1f");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glVertexAttrib1f_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glVertexAttrib1fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glVertexAttrib1fv_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_F32, WA_TYPE_F32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glVertexAttrib2f");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glVertexAttrib2f_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glVertexAttrib2fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glVertexAttrib2fv_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_F32, WA_TYPE_F32, WA_TYPE_F32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glVertexAttrib3f");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glVertexAttrib3f_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glVertexAttrib3fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glVertexAttrib3fv_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_F32, WA_TYPE_F32, WA_TYPE_F32, WA_TYPE_F32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glVertexAttrib4f");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glVertexAttrib4f_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glVertexAttrib4fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glVertexAttrib4fv_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glViewport");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glViewport_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glReadBuffer");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glReadBuffer_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glDrawRangeElements");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glDrawRangeElements_stub;
		binding.hostFunction.type.paramCount = 6;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glTexImage3D");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glTexImage3D_stub;
		binding.hostFunction.type.paramCount = 10;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glTexSubImage3D");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glTexSubImage3D_stub;
		binding.hostFunction.type.paramCount = 11;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glCopyTexSubImage3D");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glCopyTexSubImage3D_stub;
		binding.hostFunction.type.paramCount = 9;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glCompressedTexImage3D");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glCompressedTexImage3D_stub;
		binding.hostFunction.type.paramCount = 9;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glCompressedTexSubImage3D");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glCompressedTexSubImage3D_stub;
		binding.hostFunction.type.paramCount = 11;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGenQueries");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGenQueries_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glDeleteQueries");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glDeleteQueries_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[] = { WA_TYPE_I32 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glIsQuery");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glIsQuery_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glBeginQuery");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glBeginQuery_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glEndQuery");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glEndQuery_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetQueryiv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetQueryiv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetQueryObjectuiv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetQueryObjectuiv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glDrawBuffers");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glDrawBuffers_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniformMatrix2x3fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniformMatrix2x3fv_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniformMatrix3x2fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniformMatrix3x2fv_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniformMatrix2x4fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniformMatrix2x4fv_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniformMatrix4x2fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniformMatrix4x2fv_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniformMatrix3x4fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniformMatrix3x4fv_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniformMatrix4x3fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniformMatrix4x3fv_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glBlitFramebuffer");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glBlitFramebuffer_stub;
		binding.hostFunction.type.paramCount = 10;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glRenderbufferStorageMultisample");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glRenderbufferStorageMultisample_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glFramebufferTextureLayer");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glFramebufferTextureLayer_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glBindVertexArray");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glBindVertexArray_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glDeleteVertexArrays");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glDeleteVertexArrays_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGenVertexArrays");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGenVertexArrays_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[] = { WA_TYPE_I32 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glIsVertexArray");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glIsVertexArray_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetIntegeri_v");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetIntegeri_v_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glBeginTransformFeedback");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glBeginTransformFeedback_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[1];
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glEndTransformFeedback");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glEndTransformFeedback_stub;
		binding.hostFunction.type.paramCount = 0;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glBindBufferRange");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glBindBufferRange_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glBindBufferBase");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glBindBufferBase_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glTransformFeedbackVaryings");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glTransformFeedbackVaryings_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetTransformFeedbackVarying");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetTransformFeedbackVarying_stub;
		binding.hostFunction.type.paramCount = 7;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetVertexAttribIiv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetVertexAttribIiv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetVertexAttribIuiv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetVertexAttribIuiv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glVertexAttribI4i");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glVertexAttribI4i_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glVertexAttribI4ui");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glVertexAttribI4ui_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glVertexAttribI4iv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glVertexAttribI4iv_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glVertexAttribI4uiv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glVertexAttribI4uiv_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetUniformuiv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetUniformuiv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[] = { WA_TYPE_I32 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetFragDataLocation");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetFragDataLocation_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniform1ui");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniform1ui_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniform2ui");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniform2ui_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniform3ui");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniform3ui_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniform4ui");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniform4ui_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniform1uiv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniform1uiv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniform2uiv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniform2uiv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniform3uiv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniform3uiv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniform4uiv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniform4uiv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glClearBufferiv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glClearBufferiv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glClearBufferuiv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glClearBufferuiv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glClearBufferfv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glClearBufferfv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_F32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glClearBufferfi");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glClearBufferfi_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glCopyBufferSubData");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glCopyBufferSubData_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetActiveUniformsiv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetActiveUniformsiv_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[] = { WA_TYPE_I32 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetUniformBlockIndex");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetUniformBlockIndex_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetActiveUniformBlockiv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetActiveUniformBlockiv_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetActiveUniformBlockName");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetActiveUniformBlockName_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUniformBlockBinding");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUniformBlockBinding_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glDrawArraysInstanced");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glDrawArraysInstanced_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glDrawElementsInstanced");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glDrawElementsInstanced_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[] = { WA_TYPE_I64 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glFenceSync");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glFenceSync_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I64, };
		wa_value_type returnTypes[] = { WA_TYPE_I32 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glIsSync");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glIsSync_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I64, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glDeleteSync");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glDeleteSync_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I64, WA_TYPE_I32, WA_TYPE_I64, };
		wa_value_type returnTypes[] = { WA_TYPE_I32 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glClientWaitSync");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glClientWaitSync_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I64, WA_TYPE_I32, WA_TYPE_I64, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glWaitSync");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glWaitSync_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetInteger64v");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetInteger64v_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I64, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetSynciv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetSynciv_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetInteger64i_v");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetInteger64i_v_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetBufferParameteri64v");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetBufferParameteri64v_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGenSamplers");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGenSamplers_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glDeleteSamplers");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glDeleteSamplers_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[] = { WA_TYPE_I32 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glIsSampler");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glIsSampler_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glBindSampler");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glBindSampler_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glSamplerParameteri");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glSamplerParameteri_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glSamplerParameteriv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glSamplerParameteriv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_F32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glSamplerParameterf");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glSamplerParameterf_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glSamplerParameterfv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glSamplerParameterfv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetSamplerParameteriv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetSamplerParameteriv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetSamplerParameterfv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetSamplerParameterfv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glVertexAttribDivisor");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glVertexAttribDivisor_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glBindTransformFeedback");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glBindTransformFeedback_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glDeleteTransformFeedbacks");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glDeleteTransformFeedbacks_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGenTransformFeedbacks");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGenTransformFeedbacks_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[] = { WA_TYPE_I32 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glIsTransformFeedback");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glIsTransformFeedback_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[1];
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glPauseTransformFeedback");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glPauseTransformFeedback_stub;
		binding.hostFunction.type.paramCount = 0;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[1];
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glResumeTransformFeedback");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glResumeTransformFeedback_stub;
		binding.hostFunction.type.paramCount = 0;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetProgramBinary");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetProgramBinary_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramBinary");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramBinary_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramParameteri");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramParameteri_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glInvalidateFramebuffer");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glInvalidateFramebuffer_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glInvalidateSubFramebuffer");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glInvalidateSubFramebuffer_stub;
		binding.hostFunction.type.paramCount = 7;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glTexStorage2D");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glTexStorage2D_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glTexStorage3D");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glTexStorage3D_stub;
		binding.hostFunction.type.paramCount = 6;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetInternalformativ");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetInternalformativ_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glDispatchCompute");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glDispatchCompute_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glDispatchComputeIndirect");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glDispatchComputeIndirect_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glDrawArraysIndirect");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glDrawArraysIndirect_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glDrawElementsIndirect");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glDrawElementsIndirect_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glFramebufferParameteri");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glFramebufferParameteri_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetFramebufferParameteriv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetFramebufferParameteriv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetProgramInterfaceiv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetProgramInterfaceiv_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[] = { WA_TYPE_I32 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetProgramResourceIndex");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetProgramResourceIndex_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetProgramResourceName");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetProgramResourceName_stub;
		binding.hostFunction.type.paramCount = 6;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetProgramResourceiv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetProgramResourceiv_stub;
		binding.hostFunction.type.paramCount = 8;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[] = { WA_TYPE_I32 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetProgramResourceLocation");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetProgramResourceLocation_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glUseProgramStages");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glUseProgramStages_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glActiveShaderProgram");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glActiveShaderProgram_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[] = { WA_TYPE_I32 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glCreateShaderProgramv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glCreateShaderProgramv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glBindProgramPipeline");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glBindProgramPipeline_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glDeleteProgramPipelines");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glDeleteProgramPipelines_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGenProgramPipelines");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGenProgramPipelines_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[] = { WA_TYPE_I32 };

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glIsProgramPipeline");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glIsProgramPipeline_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 1;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetProgramPipelineiv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetProgramPipelineiv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniform1i");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniform1i_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniform2i");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniform2i_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniform3i");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniform3i_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniform4i");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniform4i_stub;
		binding.hostFunction.type.paramCount = 6;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniform1ui");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniform1ui_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniform2ui");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniform2ui_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniform3ui");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniform3ui_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniform4ui");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniform4ui_stub;
		binding.hostFunction.type.paramCount = 6;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_F32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniform1f");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniform1f_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_F32, WA_TYPE_F32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniform2f");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniform2f_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_F32, WA_TYPE_F32, WA_TYPE_F32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniform3f");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniform3f_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_F32, WA_TYPE_F32, WA_TYPE_F32, WA_TYPE_F32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniform4f");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniform4f_stub;
		binding.hostFunction.type.paramCount = 6;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniform1iv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniform1iv_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniform2iv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniform2iv_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniform3iv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniform3iv_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniform4iv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniform4iv_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniform1uiv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniform1uiv_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniform2uiv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniform2uiv_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniform3uiv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniform3uiv_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniform4uiv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniform4uiv_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniform1fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniform1fv_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniform2fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniform2fv_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniform3fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniform3fv_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniform4fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniform4fv_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniformMatrix2fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniformMatrix2fv_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniformMatrix3fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniformMatrix3fv_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniformMatrix4fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniformMatrix4fv_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniformMatrix2x3fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniformMatrix2x3fv_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniformMatrix3x2fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniformMatrix3x2fv_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniformMatrix2x4fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniformMatrix2x4fv_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniformMatrix4x2fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniformMatrix4x2fv_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniformMatrix3x4fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniformMatrix3x4fv_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glProgramUniformMatrix4x3fv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glProgramUniformMatrix4x3fv_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glValidateProgramPipeline");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glValidateProgramPipeline_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetProgramPipelineInfoLog");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetProgramPipelineInfoLog_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glBindImageTexture");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glBindImageTexture_stub;
		binding.hostFunction.type.paramCount = 7;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetBooleani_v");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetBooleani_v_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glMemoryBarrier");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glMemoryBarrier_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glMemoryBarrierByRegion");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glMemoryBarrierByRegion_stub;
		binding.hostFunction.type.paramCount = 1;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glTexStorage2DMultisample");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glTexStorage2DMultisample_stub;
		binding.hostFunction.type.paramCount = 6;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetMultisamplefv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetMultisamplefv_stub;
		binding.hostFunction.type.paramCount = 3;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glSampleMaski");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glSampleMaski_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetTexLevelParameteriv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetTexLevelParameteriv_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glGetTexLevelParameterfv");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glGetTexLevelParameterfv_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glBindVertexBuffer");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glBindVertexBuffer_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glVertexAttribFormat");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glVertexAttribFormat_stub;
		binding.hostFunction.type.paramCount = 5;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glVertexAttribIFormat");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glVertexAttribIFormat_stub;
		binding.hostFunction.type.paramCount = 4;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glVertexAttribBinding");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glVertexAttribBinding_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	{
		wa_value_type paramTypes[] = {WA_TYPE_I32, WA_TYPE_I32, };
		wa_value_type returnTypes[1];

		wa_import_binding binding = {0};
		binding.name = OC_STR8("glVertexBindingDivisor");
		binding.kind = WA_BINDING_HOST_FUNCTION;
		binding.hostFunction.proc = glVertexBindingDivisor_stub;
		binding.hostFunction.type.paramCount = 2;
		binding.hostFunction.type.returnCount = 0;
		binding.hostFunction.type.params = paramTypes;
		binding.hostFunction.type.returns = returnTypes;
		wa_import_package_push_binding(arena, package, &binding);
	}

	return(ret);
}
