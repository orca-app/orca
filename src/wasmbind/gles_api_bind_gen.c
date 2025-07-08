void glActiveTexture_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum texture = (GLenum)*(i32*)&_params[0];
	glActiveTexture(texture);
}

void glAttachShader_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLuint shader = (GLuint)*(i32*)&_params[1];
	glAttachShader(program, shader);
}

void glBindAttribLocation_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLuint index = (GLuint)*(i32*)&_params[1];
	const GLchar * name = (const GLchar *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)name >= (char*)_mem) && (((char*)name - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'name' is out of bounds");
		OC_ASSERT_DIALOG((char*)name + orca_glBindAttribLocation_name_length(wasm, name)*sizeof(const GLchar ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'name' is out of bounds");
	}
	glBindAttribLocation(program, index, name);
}

void glBindBuffer_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLuint buffer = (GLuint)*(i32*)&_params[1];
	glBindBuffer(target, buffer);
}

void glBindFramebuffer_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLuint framebuffer = (GLuint)*(i32*)&_params[1];
	glBindFramebuffer(target, framebuffer);
}

void glBindRenderbuffer_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLuint renderbuffer = (GLuint)*(i32*)&_params[1];
	glBindRenderbuffer(target, renderbuffer);
}

void glBindTexture_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLuint texture = (GLuint)*(i32*)&_params[1];
	glBindTexture(target, texture);
}

void glBlendColor_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLfloat red = (GLfloat)*(f32*)&_params[0];
	GLfloat green = (GLfloat)*(f32*)&_params[1];
	GLfloat blue = (GLfloat)*(f32*)&_params[2];
	GLfloat alpha = (GLfloat)*(f32*)&_params[3];
	glBlendColor(red, green, blue, alpha);
}

void glBlendEquation_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum mode = (GLenum)*(i32*)&_params[0];
	glBlendEquation(mode);
}

void glBlendEquationSeparate_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum modeRGB = (GLenum)*(i32*)&_params[0];
	GLenum modeAlpha = (GLenum)*(i32*)&_params[1];
	glBlendEquationSeparate(modeRGB, modeAlpha);
}

void glBlendFunc_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum sfactor = (GLenum)*(i32*)&_params[0];
	GLenum dfactor = (GLenum)*(i32*)&_params[1];
	glBlendFunc(sfactor, dfactor);
}

void glBlendFuncSeparate_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum sfactorRGB = (GLenum)*(i32*)&_params[0];
	GLenum dfactorRGB = (GLenum)*(i32*)&_params[1];
	GLenum sfactorAlpha = (GLenum)*(i32*)&_params[2];
	GLenum dfactorAlpha = (GLenum)*(i32*)&_params[3];
	glBlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
}

void glBufferData_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLsizeiptr size = (GLsizeiptr)*(i32*)&_params[1];
	const void * data = (const void *)((char*)_mem + *(u32*)&_params[2]);
	GLenum usage = (GLenum)*(i32*)&_params[3];
	{
		OC_ASSERT_DIALOG(((char*)data >= (char*)_mem) && (((char*)data - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'data' is out of bounds");
		OC_ASSERT_DIALOG((char*)data + size <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'data' is out of bounds");
	}
	glBufferData(target, size, data, usage);
}

void glBufferSubData_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLintptr offset = (GLintptr)*(i32*)&_params[1];
	GLsizeiptr size = (GLsizeiptr)*(i32*)&_params[2];
	const void * data = (const void *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)data >= (char*)_mem) && (((char*)data - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'data' is out of bounds");
		OC_ASSERT_DIALOG((char*)data + size <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'data' is out of bounds");
	}
	glBufferSubData(target, offset, size, data);
}

void glCheckFramebufferStatus_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glCheckFramebufferStatus(target);
}

void glClear_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLbitfield mask = (GLbitfield)*(i32*)&_params[0];
	glClear(mask);
}

void glClearColor_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLfloat red = (GLfloat)*(f32*)&_params[0];
	GLfloat green = (GLfloat)*(f32*)&_params[1];
	GLfloat blue = (GLfloat)*(f32*)&_params[2];
	GLfloat alpha = (GLfloat)*(f32*)&_params[3];
	glClearColor(red, green, blue, alpha);
}

void glClearDepthf_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLfloat d = (GLfloat)*(f32*)&_params[0];
	glClearDepthf(d);
}

void glClearStencil_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint s = (GLint)*(i32*)&_params[0];
	glClearStencil(s);
}

void glColorMask_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLboolean red = (GLboolean)*(i32*)&_params[0];
	GLboolean green = (GLboolean)*(i32*)&_params[1];
	GLboolean blue = (GLboolean)*(i32*)&_params[2];
	GLboolean alpha = (GLboolean)*(i32*)&_params[3];
	glColorMask(red, green, blue, alpha);
}

void glCompileShader_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint shader = (GLuint)*(i32*)&_params[0];
	glCompileShader(shader);
}

void glCompressedTexImage2D_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLint level = (GLint)*(i32*)&_params[1];
	GLenum internalformat = (GLenum)*(i32*)&_params[2];
	GLsizei width = (GLsizei)*(i32*)&_params[3];
	GLsizei height = (GLsizei)*(i32*)&_params[4];
	GLint border = (GLint)*(i32*)&_params[5];
	GLsizei imageSize = (GLsizei)*(i32*)&_params[6];
	const void * data = (const void *)((char*)_mem + *(u32*)&_params[7]);
	{
		OC_ASSERT_DIALOG(((char*)data >= (char*)_mem) && (((char*)data - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'data' is out of bounds");
		OC_ASSERT_DIALOG((char*)data + imageSize <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'data' is out of bounds");
	}
	glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
}

void glCompressedTexSubImage2D_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
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
		OC_ASSERT_DIALOG(((char*)data >= (char*)_mem) && (((char*)data - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'data' is out of bounds");
		OC_ASSERT_DIALOG((char*)data + imageSize <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'data' is out of bounds");
	}
	glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
}

void glCopyTexImage2D_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
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

void glCopyTexSubImage2D_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
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

void glCreateProgram_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	*((i32*)&_returns[0]) = (i32)glCreateProgram();
}

void glCreateShader_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum type = (GLenum)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glCreateShader(type);
}

void glCullFace_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum mode = (GLenum)*(i32*)&_params[0];
	glCullFace(mode);
}

void glDeleteBuffers_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	const GLuint * buffers = (const GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)buffers >= (char*)_mem) && (((char*)buffers - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'buffers' is out of bounds");
		OC_ASSERT_DIALOG((char*)buffers + n*sizeof(const GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'buffers' is out of bounds");
	}
	glDeleteBuffers(n, buffers);
}

void glDeleteFramebuffers_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	const GLuint * framebuffers = (const GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)framebuffers >= (char*)_mem) && (((char*)framebuffers - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'framebuffers' is out of bounds");
		OC_ASSERT_DIALOG((char*)framebuffers + n*sizeof(const GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'framebuffers' is out of bounds");
	}
	glDeleteFramebuffers(n, framebuffers);
}

void glDeleteProgram_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	glDeleteProgram(program);
}

void glDeleteRenderbuffers_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	const GLuint * renderbuffers = (const GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)renderbuffers >= (char*)_mem) && (((char*)renderbuffers - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'renderbuffers' is out of bounds");
		OC_ASSERT_DIALOG((char*)renderbuffers + n*sizeof(const GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'renderbuffers' is out of bounds");
	}
	glDeleteRenderbuffers(n, renderbuffers);
}

void glDeleteShader_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint shader = (GLuint)*(i32*)&_params[0];
	glDeleteShader(shader);
}

void glDeleteTextures_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	const GLuint * textures = (const GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)textures >= (char*)_mem) && (((char*)textures - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'textures' is out of bounds");
		OC_ASSERT_DIALOG((char*)textures + n*sizeof(const GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'textures' is out of bounds");
	}
	glDeleteTextures(n, textures);
}

void glDepthFunc_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum func = (GLenum)*(i32*)&_params[0];
	glDepthFunc(func);
}

void glDepthMask_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLboolean flag = (GLboolean)*(i32*)&_params[0];
	glDepthMask(flag);
}

void glDepthRangef_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLfloat n = (GLfloat)*(f32*)&_params[0];
	GLfloat f = (GLfloat)*(f32*)&_params[1];
	glDepthRangef(n, f);
}

void glDetachShader_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLuint shader = (GLuint)*(i32*)&_params[1];
	glDetachShader(program, shader);
}

void glDisable_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum cap = (GLenum)*(i32*)&_params[0];
	glDisable(cap);
}

void glDisableVertexAttribArray_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint index = (GLuint)*(i32*)&_params[0];
	glDisableVertexAttribArray(index);
}

void glDrawArrays_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum mode = (GLenum)*(i32*)&_params[0];
	GLint first = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	glDrawArrays(mode, first, count);
}

void glDrawElements_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum mode = (GLenum)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	GLenum type = (GLenum)*(i32*)&_params[2];
	const void * indices = (const void *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)indices >= (char*)_mem) && (((char*)indices - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'indices' is out of bounds");
		OC_ASSERT_DIALOG((char*)indices + orca_glDrawElements_indices_length(wasm, count, type) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'indices' is out of bounds");
	}
	glDrawElements(mode, count, type, indices);
}

void glEnable_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum cap = (GLenum)*(i32*)&_params[0];
	glEnable(cap);
}

void glEnableVertexAttribArray_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint index = (GLuint)*(i32*)&_params[0];
	glEnableVertexAttribArray(index);
}

void glFinish_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	glFinish();
}

void glFlush_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	glFlush();
}

void glFramebufferRenderbuffer_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum attachment = (GLenum)*(i32*)&_params[1];
	GLenum renderbuffertarget = (GLenum)*(i32*)&_params[2];
	GLuint renderbuffer = (GLuint)*(i32*)&_params[3];
	glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
}

void glFramebufferTexture2D_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum attachment = (GLenum)*(i32*)&_params[1];
	GLenum textarget = (GLenum)*(i32*)&_params[2];
	GLuint texture = (GLuint)*(i32*)&_params[3];
	GLint level = (GLint)*(i32*)&_params[4];
	glFramebufferTexture2D(target, attachment, textarget, texture, level);
}

void glFrontFace_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum mode = (GLenum)*(i32*)&_params[0];
	glFrontFace(mode);
}

void glGenBuffers_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	GLuint * buffers = (GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)buffers >= (char*)_mem) && (((char*)buffers - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'buffers' is out of bounds");
		OC_ASSERT_DIALOG((char*)buffers + n*sizeof(GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'buffers' is out of bounds");
	}
	glGenBuffers(n, buffers);
}

void glGenerateMipmap_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	glGenerateMipmap(target);
}

void glGenFramebuffers_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	GLuint * framebuffers = (GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)framebuffers >= (char*)_mem) && (((char*)framebuffers - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'framebuffers' is out of bounds");
		OC_ASSERT_DIALOG((char*)framebuffers + n*sizeof(GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'framebuffers' is out of bounds");
	}
	glGenFramebuffers(n, framebuffers);
}

void glGenRenderbuffers_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	GLuint * renderbuffers = (GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)renderbuffers >= (char*)_mem) && (((char*)renderbuffers - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'renderbuffers' is out of bounds");
		OC_ASSERT_DIALOG((char*)renderbuffers + n*sizeof(GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'renderbuffers' is out of bounds");
	}
	glGenRenderbuffers(n, renderbuffers);
}

void glGenTextures_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	GLuint * textures = (GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)textures >= (char*)_mem) && (((char*)textures - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'textures' is out of bounds");
		OC_ASSERT_DIALOG((char*)textures + n*sizeof(GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'textures' is out of bounds");
	}
	glGenTextures(n, textures);
}

void glGetActiveAttrib_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLuint index = (GLuint)*(i32*)&_params[1];
	GLsizei bufSize = (GLsizei)*(i32*)&_params[2];
	GLsizei * length = (GLsizei *)((char*)_mem + *(u32*)&_params[3]);
	GLint * size = (GLint *)((char*)_mem + *(u32*)&_params[4]);
	GLenum * type = (GLenum *)((char*)_mem + *(u32*)&_params[5]);
	GLchar * name = (GLchar *)((char*)_mem + *(u32*)&_params[6]);
	{
		OC_ASSERT_DIALOG(((char*)length >= (char*)_mem) && (((char*)length - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'length' is out of bounds");
		OC_ASSERT_DIALOG((char*)length + 1*sizeof(GLsizei ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'length' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)size >= (char*)_mem) && (((char*)size - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'size' is out of bounds");
		OC_ASSERT_DIALOG((char*)size + 1*sizeof(GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'size' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)type >= (char*)_mem) && (((char*)type - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'type' is out of bounds");
		OC_ASSERT_DIALOG((char*)type + 1*sizeof(GLenum ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'type' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)name >= (char*)_mem) && (((char*)name - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'name' is out of bounds");
		OC_ASSERT_DIALOG((char*)name + bufSize*sizeof(GLchar ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'name' is out of bounds");
	}
	glGetActiveAttrib(program, index, bufSize, length, size, type, name);
}

void glGetActiveUniform_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLuint index = (GLuint)*(i32*)&_params[1];
	GLsizei bufSize = (GLsizei)*(i32*)&_params[2];
	GLsizei * length = (GLsizei *)((char*)_mem + *(u32*)&_params[3]);
	GLint * size = (GLint *)((char*)_mem + *(u32*)&_params[4]);
	GLenum * type = (GLenum *)((char*)_mem + *(u32*)&_params[5]);
	GLchar * name = (GLchar *)((char*)_mem + *(u32*)&_params[6]);
	{
		OC_ASSERT_DIALOG(((char*)length >= (char*)_mem) && (((char*)length - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'length' is out of bounds");
		OC_ASSERT_DIALOG((char*)length + 1*sizeof(GLsizei ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'length' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)size >= (char*)_mem) && (((char*)size - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'size' is out of bounds");
		OC_ASSERT_DIALOG((char*)size + 1*sizeof(GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'size' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)type >= (char*)_mem) && (((char*)type - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'type' is out of bounds");
		OC_ASSERT_DIALOG((char*)type + 1*sizeof(GLenum ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'type' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)name >= (char*)_mem) && (((char*)name - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'name' is out of bounds");
		OC_ASSERT_DIALOG((char*)name + bufSize*sizeof(GLchar ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'name' is out of bounds");
	}
	glGetActiveUniform(program, index, bufSize, length, size, type, name);
}

void glGetAttachedShaders_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLsizei maxCount = (GLsizei)*(i32*)&_params[1];
	GLsizei * count = (GLsizei *)((char*)_mem + *(u32*)&_params[2]);
	GLuint * shaders = (GLuint *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)count >= (char*)_mem) && (((char*)count - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'count' is out of bounds");
		OC_ASSERT_DIALOG((char*)count + 1*sizeof(GLsizei ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'count' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)shaders >= (char*)_mem) && (((char*)shaders - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'shaders' is out of bounds");
		OC_ASSERT_DIALOG((char*)shaders + maxCount*sizeof(GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'shaders' is out of bounds");
	}
	glGetAttachedShaders(program, maxCount, count, shaders);
}

void glGetAttribLocation_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	const GLchar * name = (const GLchar *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)name >= (char*)_mem) && (((char*)name - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'name' is out of bounds");
		OC_ASSERT_DIALOG((char*)name + orca_glGetAttribLocation_name_length(wasm, name)*sizeof(const GLchar ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'name' is out of bounds");
	}
	*((i32*)&_returns[0]) = (i32)glGetAttribLocation(program, name);
}

void glGetBooleanv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum pname = (GLenum)*(i32*)&_params[0];
	GLboolean * data = (GLboolean *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)data >= (char*)_mem) && (((char*)data - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'data' is out of bounds");
		OC_ASSERT_DIALOG((char*)data + orca_glGetBooleanv_data_length(wasm, pname)*sizeof(GLboolean ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'data' is out of bounds");
	}
	glGetBooleanv(pname, data);
}

void glGetBufferParameteriv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetBufferParameteriv_params_length(wasm, pname)*sizeof(GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glGetBufferParameteriv(target, pname, params);
}

void glGetError_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	*((i32*)&_returns[0]) = (i32)glGetError();
}

void glGetFloatv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum pname = (GLenum)*(i32*)&_params[0];
	GLfloat * data = (GLfloat *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)data >= (char*)_mem) && (((char*)data - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'data' is out of bounds");
		OC_ASSERT_DIALOG((char*)data + orca_glGetFloatv_data_length(wasm, pname)*sizeof(GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'data' is out of bounds");
	}
	glGetFloatv(pname, data);
}

void glGetFramebufferAttachmentParameteriv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum attachment = (GLenum)*(i32*)&_params[1];
	GLenum pname = (GLenum)*(i32*)&_params[2];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetFramebufferAttachmentParameteriv_params_length(wasm, pname)*sizeof(GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
}

void glGetIntegerv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum pname = (GLenum)*(i32*)&_params[0];
	GLint * data = (GLint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)data >= (char*)_mem) && (((char*)data - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'data' is out of bounds");
		OC_ASSERT_DIALOG((char*)data + orca_glGetIntegerv_data_length(wasm, pname)*sizeof(GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'data' is out of bounds");
	}
	glGetIntegerv(pname, data);
}

void glGetProgramiv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetProgramiv_params_length(wasm, pname)*sizeof(GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glGetProgramiv(program, pname, params);
}

void glGetProgramInfoLog_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLsizei bufSize = (GLsizei)*(i32*)&_params[1];
	GLsizei * length = (GLsizei *)((char*)_mem + *(u32*)&_params[2]);
	GLchar * infoLog = (GLchar *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)length >= (char*)_mem) && (((char*)length - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'length' is out of bounds");
		OC_ASSERT_DIALOG((char*)length + 1*sizeof(GLsizei ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'length' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)infoLog >= (char*)_mem) && (((char*)infoLog - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'infoLog' is out of bounds");
		OC_ASSERT_DIALOG((char*)infoLog + bufSize*sizeof(GLchar ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'infoLog' is out of bounds");
	}
	glGetProgramInfoLog(program, bufSize, length, infoLog);
}

void glGetRenderbufferParameteriv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetRenderbufferParameteriv_params_length(wasm, pname)*sizeof(GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glGetRenderbufferParameteriv(target, pname, params);
}

void glGetShaderiv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint shader = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetShaderiv_params_length(wasm, pname)*sizeof(GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glGetShaderiv(shader, pname, params);
}

void glGetShaderInfoLog_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint shader = (GLuint)*(i32*)&_params[0];
	GLsizei bufSize = (GLsizei)*(i32*)&_params[1];
	GLsizei * length = (GLsizei *)((char*)_mem + *(u32*)&_params[2]);
	GLchar * infoLog = (GLchar *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)length >= (char*)_mem) && (((char*)length - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'length' is out of bounds");
		OC_ASSERT_DIALOG((char*)length + 1*sizeof(GLsizei ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'length' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)infoLog >= (char*)_mem) && (((char*)infoLog - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'infoLog' is out of bounds");
		OC_ASSERT_DIALOG((char*)infoLog + bufSize*sizeof(GLchar ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'infoLog' is out of bounds");
	}
	glGetShaderInfoLog(shader, bufSize, length, infoLog);
}

void glGetShaderPrecisionFormat_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum shadertype = (GLenum)*(i32*)&_params[0];
	GLenum precisiontype = (GLenum)*(i32*)&_params[1];
	GLint * range = (GLint *)((char*)_mem + *(u32*)&_params[2]);
	GLint * precision = (GLint *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)range >= (char*)_mem) && (((char*)range - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'range' is out of bounds");
		OC_ASSERT_DIALOG((char*)range + 2*sizeof(GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'range' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)precision >= (char*)_mem) && (((char*)precision - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'precision' is out of bounds");
		OC_ASSERT_DIALOG((char*)precision + 1*sizeof(GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'precision' is out of bounds");
	}
	glGetShaderPrecisionFormat(shadertype, precisiontype, range, precision);
}

void glGetShaderSource_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint shader = (GLuint)*(i32*)&_params[0];
	GLsizei bufSize = (GLsizei)*(i32*)&_params[1];
	GLsizei * length = (GLsizei *)((char*)_mem + *(u32*)&_params[2]);
	GLchar * source = (GLchar *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)length >= (char*)_mem) && (((char*)length - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'length' is out of bounds");
		OC_ASSERT_DIALOG((char*)length + 1*sizeof(GLsizei ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'length' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)source >= (char*)_mem) && (((char*)source - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'source' is out of bounds");
		OC_ASSERT_DIALOG((char*)source + bufSize*sizeof(GLchar ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'source' is out of bounds");
	}
	glGetShaderSource(shader, bufSize, length, source);
}

void glGetTexParameterfv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLfloat * params = (GLfloat *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetTexParameterfv_params_length(wasm, pname)*sizeof(GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glGetTexParameterfv(target, pname, params);
}

void glGetTexParameteriv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetTexParameteriv_params_length(wasm, pname)*sizeof(GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glGetTexParameteriv(target, pname, params);
}

void glGetUniformfv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLfloat * params = (GLfloat *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetUniformfv_params_length(wasm, program, location)*sizeof(GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glGetUniformfv(program, location, params);
}

void glGetUniformiv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetUniformiv_params_length(wasm, program, location)*sizeof(GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glGetUniformiv(program, location, params);
}

void glGetUniformLocation_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	const GLchar * name = (const GLchar *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)name >= (char*)_mem) && (((char*)name - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'name' is out of bounds");
		OC_ASSERT_DIALOG((char*)name + orca_glGetUniformLocation_name_length(wasm, name)*sizeof(const GLchar ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'name' is out of bounds");
	}
	*((i32*)&_returns[0]) = (i32)glGetUniformLocation(program, name);
}

void glGetVertexAttribfv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint index = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLfloat * params = (GLfloat *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + 4*sizeof(GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glGetVertexAttribfv(index, pname, params);
}

void glGetVertexAttribiv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint index = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + 4*sizeof(GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glGetVertexAttribiv(index, pname, params);
}

void glHint_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum mode = (GLenum)*(i32*)&_params[1];
	glHint(target, mode);
}

void glIsBuffer_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint buffer = (GLuint)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glIsBuffer(buffer);
}

void glIsEnabled_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum cap = (GLenum)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glIsEnabled(cap);
}

void glIsFramebuffer_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint framebuffer = (GLuint)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glIsFramebuffer(framebuffer);
}

void glIsProgram_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glIsProgram(program);
}

void glIsRenderbuffer_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint renderbuffer = (GLuint)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glIsRenderbuffer(renderbuffer);
}

void glIsShader_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint shader = (GLuint)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glIsShader(shader);
}

void glIsTexture_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint texture = (GLuint)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glIsTexture(texture);
}

void glLineWidth_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLfloat width = (GLfloat)*(f32*)&_params[0];
	glLineWidth(width);
}

void glLinkProgram_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	glLinkProgram(program);
}

void glPixelStorei_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum pname = (GLenum)*(i32*)&_params[0];
	GLint param = (GLint)*(i32*)&_params[1];
	glPixelStorei(pname, param);
}

void glPolygonOffset_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLfloat factor = (GLfloat)*(f32*)&_params[0];
	GLfloat units = (GLfloat)*(f32*)&_params[1];
	glPolygonOffset(factor, units);
}

void glReadPixels_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint x = (GLint)*(i32*)&_params[0];
	GLint y = (GLint)*(i32*)&_params[1];
	GLsizei width = (GLsizei)*(i32*)&_params[2];
	GLsizei height = (GLsizei)*(i32*)&_params[3];
	GLenum format = (GLenum)*(i32*)&_params[4];
	GLenum type = (GLenum)*(i32*)&_params[5];
	void * pixels = (void *)((char*)_mem + *(u32*)&_params[6]);
	{
		OC_ASSERT_DIALOG(((char*)pixels >= (char*)_mem) && (((char*)pixels - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'pixels' is out of bounds");
		OC_ASSERT_DIALOG((char*)pixels + orca_glReadPixels_pixels_length(wasm, format, type, width, height) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'pixels' is out of bounds");
	}
	glReadPixels(x, y, width, height, format, type, pixels);
}

void glReleaseShaderCompiler_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	glReleaseShaderCompiler();
}

void glRenderbufferStorage_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum internalformat = (GLenum)*(i32*)&_params[1];
	GLsizei width = (GLsizei)*(i32*)&_params[2];
	GLsizei height = (GLsizei)*(i32*)&_params[3];
	glRenderbufferStorage(target, internalformat, width, height);
}

void glSampleCoverage_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLfloat value = (GLfloat)*(f32*)&_params[0];
	GLboolean invert = (GLboolean)*(i32*)&_params[1];
	glSampleCoverage(value, invert);
}

void glScissor_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint x = (GLint)*(i32*)&_params[0];
	GLint y = (GLint)*(i32*)&_params[1];
	GLsizei width = (GLsizei)*(i32*)&_params[2];
	GLsizei height = (GLsizei)*(i32*)&_params[3];
	glScissor(x, y, width, height);
}

void glShaderBinary_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLsizei count = (GLsizei)*(i32*)&_params[0];
	const GLuint * shaders = (const GLuint *)((char*)_mem + *(u32*)&_params[1]);
	GLenum binaryFormat = (GLenum)*(i32*)&_params[2];
	const void * binary = (const void *)((char*)_mem + *(u32*)&_params[3]);
	GLsizei length = (GLsizei)*(i32*)&_params[4];
	{
		OC_ASSERT_DIALOG(((char*)shaders >= (char*)_mem) && (((char*)shaders - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'shaders' is out of bounds");
		OC_ASSERT_DIALOG((char*)shaders + count*sizeof(const GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'shaders' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)binary >= (char*)_mem) && (((char*)binary - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'binary' is out of bounds");
		OC_ASSERT_DIALOG((char*)binary + length <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'binary' is out of bounds");
	}
	glShaderBinary(count, shaders, binaryFormat, binary, length);
}

void glStencilFunc_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum func = (GLenum)*(i32*)&_params[0];
	GLint ref = (GLint)*(i32*)&_params[1];
	GLuint mask = (GLuint)*(i32*)&_params[2];
	glStencilFunc(func, ref, mask);
}

void glStencilFuncSeparate_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum face = (GLenum)*(i32*)&_params[0];
	GLenum func = (GLenum)*(i32*)&_params[1];
	GLint ref = (GLint)*(i32*)&_params[2];
	GLuint mask = (GLuint)*(i32*)&_params[3];
	glStencilFuncSeparate(face, func, ref, mask);
}

void glStencilMask_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint mask = (GLuint)*(i32*)&_params[0];
	glStencilMask(mask);
}

void glStencilMaskSeparate_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum face = (GLenum)*(i32*)&_params[0];
	GLuint mask = (GLuint)*(i32*)&_params[1];
	glStencilMaskSeparate(face, mask);
}

void glStencilOp_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum fail = (GLenum)*(i32*)&_params[0];
	GLenum zfail = (GLenum)*(i32*)&_params[1];
	GLenum zpass = (GLenum)*(i32*)&_params[2];
	glStencilOp(fail, zfail, zpass);
}

void glStencilOpSeparate_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum face = (GLenum)*(i32*)&_params[0];
	GLenum sfail = (GLenum)*(i32*)&_params[1];
	GLenum dpfail = (GLenum)*(i32*)&_params[2];
	GLenum dppass = (GLenum)*(i32*)&_params[3];
	glStencilOpSeparate(face, sfail, dpfail, dppass);
}

void glTexImage2D_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
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
		OC_ASSERT_DIALOG(((char*)pixels >= (char*)_mem) && (((char*)pixels - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'pixels' is out of bounds");
		OC_ASSERT_DIALOG((char*)pixels + orca_glTexImage2D_pixels_length(wasm, format, type, width, height) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'pixels' is out of bounds");
	}
	glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
}

void glTexParameterf_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLfloat param = (GLfloat)*(f32*)&_params[2];
	glTexParameterf(target, pname, param);
}

void glTexParameterfv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	const GLfloat * params = (const GLfloat *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glTexParameterfv_params_length(wasm, pname)*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glTexParameterfv(target, pname, params);
}

void glTexParameteri_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint param = (GLint)*(i32*)&_params[2];
	glTexParameteri(target, pname, param);
}

void glTexParameteriv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	const GLint * params = (const GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glTexParameteriv_params_length(wasm, pname)*sizeof(const GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glTexParameteriv(target, pname, params);
}

void glTexSubImage2D_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
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
		OC_ASSERT_DIALOG(((char*)pixels >= (char*)_mem) && (((char*)pixels - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'pixels' is out of bounds");
		OC_ASSERT_DIALOG((char*)pixels + orca_glTexSubImage2D_pixels_length(wasm, format, type, width, height) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'pixels' is out of bounds");
	}
	glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

void glUniform1f_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLfloat v0 = (GLfloat)*(f32*)&_params[1];
	glUniform1f(location, v0);
}

void glUniform1fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 1*count*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glUniform1fv(location, count, value);
}

void glUniform1i_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLint v0 = (GLint)*(i32*)&_params[1];
	glUniform1i(location, v0);
}

void glUniform1iv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	const GLint * value = (const GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 1*count*sizeof(const GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glUniform1iv(location, count, value);
}

void glUniform2f_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLfloat v0 = (GLfloat)*(f32*)&_params[1];
	GLfloat v1 = (GLfloat)*(f32*)&_params[2];
	glUniform2f(location, v0, v1);
}

void glUniform2fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 2*count*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glUniform2fv(location, count, value);
}

void glUniform2i_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLint v0 = (GLint)*(i32*)&_params[1];
	GLint v1 = (GLint)*(i32*)&_params[2];
	glUniform2i(location, v0, v1);
}

void glUniform2iv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	const GLint * value = (const GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 2*count*sizeof(const GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glUniform2iv(location, count, value);
}

void glUniform3f_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLfloat v0 = (GLfloat)*(f32*)&_params[1];
	GLfloat v1 = (GLfloat)*(f32*)&_params[2];
	GLfloat v2 = (GLfloat)*(f32*)&_params[3];
	glUniform3f(location, v0, v1, v2);
}

void glUniform3fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 3*count*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glUniform3fv(location, count, value);
}

void glUniform3i_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLint v0 = (GLint)*(i32*)&_params[1];
	GLint v1 = (GLint)*(i32*)&_params[2];
	GLint v2 = (GLint)*(i32*)&_params[3];
	glUniform3i(location, v0, v1, v2);
}

void glUniform3iv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	const GLint * value = (const GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 3*count*sizeof(const GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glUniform3iv(location, count, value);
}

void glUniform4f_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLfloat v0 = (GLfloat)*(f32*)&_params[1];
	GLfloat v1 = (GLfloat)*(f32*)&_params[2];
	GLfloat v2 = (GLfloat)*(f32*)&_params[3];
	GLfloat v3 = (GLfloat)*(f32*)&_params[4];
	glUniform4f(location, v0, v1, v2, v3);
}

void glUniform4fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 4*count*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glUniform4fv(location, count, value);
}

void glUniform4i_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLint v0 = (GLint)*(i32*)&_params[1];
	GLint v1 = (GLint)*(i32*)&_params[2];
	GLint v2 = (GLint)*(i32*)&_params[3];
	GLint v3 = (GLint)*(i32*)&_params[4];
	glUniform4i(location, v0, v1, v2, v3);
}

void glUniform4iv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	const GLint * value = (const GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 4*count*sizeof(const GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glUniform4iv(location, count, value);
}

void glUniformMatrix2fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	GLboolean transpose = (GLboolean)*(i32*)&_params[2];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 4*count*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glUniformMatrix2fv(location, count, transpose, value);
}

void glUniformMatrix3fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	GLboolean transpose = (GLboolean)*(i32*)&_params[2];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 9*count*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glUniformMatrix3fv(location, count, transpose, value);
}

void glUniformMatrix4fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	GLboolean transpose = (GLboolean)*(i32*)&_params[2];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 16*count*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glUniformMatrix4fv(location, count, transpose, value);
}

void glUseProgram_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	glUseProgram(program);
}

void glValidateProgram_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	glValidateProgram(program);
}

void glVertexAttrib1f_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint index = (GLuint)*(i32*)&_params[0];
	GLfloat x = (GLfloat)*(f32*)&_params[1];
	glVertexAttrib1f(index, x);
}

void glVertexAttrib1fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint index = (GLuint)*(i32*)&_params[0];
	const GLfloat * v = (const GLfloat *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)v >= (char*)_mem) && (((char*)v - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'v' is out of bounds");
		OC_ASSERT_DIALOG((char*)v + 1*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'v' is out of bounds");
	}
	glVertexAttrib1fv(index, v);
}

void glVertexAttrib2f_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint index = (GLuint)*(i32*)&_params[0];
	GLfloat x = (GLfloat)*(f32*)&_params[1];
	GLfloat y = (GLfloat)*(f32*)&_params[2];
	glVertexAttrib2f(index, x, y);
}

void glVertexAttrib2fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint index = (GLuint)*(i32*)&_params[0];
	const GLfloat * v = (const GLfloat *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)v >= (char*)_mem) && (((char*)v - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'v' is out of bounds");
		OC_ASSERT_DIALOG((char*)v + 2*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'v' is out of bounds");
	}
	glVertexAttrib2fv(index, v);
}

void glVertexAttrib3f_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint index = (GLuint)*(i32*)&_params[0];
	GLfloat x = (GLfloat)*(f32*)&_params[1];
	GLfloat y = (GLfloat)*(f32*)&_params[2];
	GLfloat z = (GLfloat)*(f32*)&_params[3];
	glVertexAttrib3f(index, x, y, z);
}

void glVertexAttrib3fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint index = (GLuint)*(i32*)&_params[0];
	const GLfloat * v = (const GLfloat *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)v >= (char*)_mem) && (((char*)v - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'v' is out of bounds");
		OC_ASSERT_DIALOG((char*)v + 3*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'v' is out of bounds");
	}
	glVertexAttrib3fv(index, v);
}

void glVertexAttrib4f_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint index = (GLuint)*(i32*)&_params[0];
	GLfloat x = (GLfloat)*(f32*)&_params[1];
	GLfloat y = (GLfloat)*(f32*)&_params[2];
	GLfloat z = (GLfloat)*(f32*)&_params[3];
	GLfloat w = (GLfloat)*(f32*)&_params[4];
	glVertexAttrib4f(index, x, y, z, w);
}

void glVertexAttrib4fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint index = (GLuint)*(i32*)&_params[0];
	const GLfloat * v = (const GLfloat *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)v >= (char*)_mem) && (((char*)v - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'v' is out of bounds");
		OC_ASSERT_DIALOG((char*)v + 4*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'v' is out of bounds");
	}
	glVertexAttrib4fv(index, v);
}

void glViewport_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint x = (GLint)*(i32*)&_params[0];
	GLint y = (GLint)*(i32*)&_params[1];
	GLsizei width = (GLsizei)*(i32*)&_params[2];
	GLsizei height = (GLsizei)*(i32*)&_params[3];
	glViewport(x, y, width, height);
}

void glReadBuffer_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum src = (GLenum)*(i32*)&_params[0];
	glReadBuffer(src);
}

void glDrawRangeElements_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum mode = (GLenum)*(i32*)&_params[0];
	GLuint start = (GLuint)*(i32*)&_params[1];
	GLuint end = (GLuint)*(i32*)&_params[2];
	GLsizei count = (GLsizei)*(i32*)&_params[3];
	GLenum type = (GLenum)*(i32*)&_params[4];
	const void * indices = (const void *)((char*)_mem + *(u32*)&_params[5]);
	{
		OC_ASSERT_DIALOG(((char*)indices >= (char*)_mem) && (((char*)indices - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'indices' is out of bounds");
		OC_ASSERT_DIALOG((char*)indices + orca_glDrawRangeElements_indices_length(wasm, count, type) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'indices' is out of bounds");
	}
	glDrawRangeElements(mode, start, end, count, type, indices);
}

void glTexImage3D_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
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
		OC_ASSERT_DIALOG(((char*)pixels >= (char*)_mem) && (((char*)pixels - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'pixels' is out of bounds");
		OC_ASSERT_DIALOG((char*)pixels + orca_glTexImage3D_pixels_length(wasm, format, type, width, height, depth) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'pixels' is out of bounds");
	}
	glTexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels);
}

void glTexSubImage3D_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
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
		OC_ASSERT_DIALOG(((char*)pixels >= (char*)_mem) && (((char*)pixels - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'pixels' is out of bounds");
		OC_ASSERT_DIALOG((char*)pixels + orca_glTexSubImage3D_pixels_length(wasm, format, type, width, height, depth) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'pixels' is out of bounds");
	}
	glTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

void glCopyTexSubImage3D_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
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

void glCompressedTexImage3D_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
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
		OC_ASSERT_DIALOG(((char*)data >= (char*)_mem) && (((char*)data - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'data' is out of bounds");
		OC_ASSERT_DIALOG((char*)data + imageSize <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'data' is out of bounds");
	}
	glCompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data);
}

void glCompressedTexSubImage3D_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
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
		OC_ASSERT_DIALOG(((char*)data >= (char*)_mem) && (((char*)data - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'data' is out of bounds");
		OC_ASSERT_DIALOG((char*)data + imageSize <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'data' is out of bounds");
	}
	glCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
}

void glGenQueries_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	GLuint * ids = (GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)ids >= (char*)_mem) && (((char*)ids - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'ids' is out of bounds");
		OC_ASSERT_DIALOG((char*)ids + n*sizeof(GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'ids' is out of bounds");
	}
	glGenQueries(n, ids);
}

void glDeleteQueries_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	const GLuint * ids = (const GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)ids >= (char*)_mem) && (((char*)ids - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'ids' is out of bounds");
		OC_ASSERT_DIALOG((char*)ids + n*sizeof(const GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'ids' is out of bounds");
	}
	glDeleteQueries(n, ids);
}

void glIsQuery_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint id = (GLuint)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glIsQuery(id);
}

void glBeginQuery_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLuint id = (GLuint)*(i32*)&_params[1];
	glBeginQuery(target, id);
}

void glEndQuery_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	glEndQuery(target);
}

void glGetQueryiv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetQueryiv_params_length(wasm, pname)*sizeof(GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glGetQueryiv(target, pname, params);
}

void glGetQueryObjectuiv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint id = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLuint * params = (GLuint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetQueryObjectuiv_params_length(wasm, pname)*sizeof(GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glGetQueryObjectuiv(id, pname, params);
}

void glDrawBuffers_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	const GLenum * bufs = (const GLenum *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)bufs >= (char*)_mem) && (((char*)bufs - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'bufs' is out of bounds");
		OC_ASSERT_DIALOG((char*)bufs + n*sizeof(const GLenum ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'bufs' is out of bounds");
	}
	glDrawBuffers(n, bufs);
}

void glUniformMatrix2x3fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	GLboolean transpose = (GLboolean)*(i32*)&_params[2];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 6*count*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glUniformMatrix2x3fv(location, count, transpose, value);
}

void glUniformMatrix3x2fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	GLboolean transpose = (GLboolean)*(i32*)&_params[2];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 6*count*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glUniformMatrix3x2fv(location, count, transpose, value);
}

void glUniformMatrix2x4fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	GLboolean transpose = (GLboolean)*(i32*)&_params[2];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 8*count*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glUniformMatrix2x4fv(location, count, transpose, value);
}

void glUniformMatrix4x2fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	GLboolean transpose = (GLboolean)*(i32*)&_params[2];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 8*count*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glUniformMatrix4x2fv(location, count, transpose, value);
}

void glUniformMatrix3x4fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	GLboolean transpose = (GLboolean)*(i32*)&_params[2];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 12*count*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glUniformMatrix3x4fv(location, count, transpose, value);
}

void glUniformMatrix4x3fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	GLboolean transpose = (GLboolean)*(i32*)&_params[2];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 12*count*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glUniformMatrix4x3fv(location, count, transpose, value);
}

void glBlitFramebuffer_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
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

void glRenderbufferStorageMultisample_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLsizei samples = (GLsizei)*(i32*)&_params[1];
	GLenum internalformat = (GLenum)*(i32*)&_params[2];
	GLsizei width = (GLsizei)*(i32*)&_params[3];
	GLsizei height = (GLsizei)*(i32*)&_params[4];
	glRenderbufferStorageMultisample(target, samples, internalformat, width, height);
}

void glFramebufferTextureLayer_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum attachment = (GLenum)*(i32*)&_params[1];
	GLuint texture = (GLuint)*(i32*)&_params[2];
	GLint level = (GLint)*(i32*)&_params[3];
	GLint layer = (GLint)*(i32*)&_params[4];
	glFramebufferTextureLayer(target, attachment, texture, level, layer);
}

void glBindVertexArray_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint array = (GLuint)*(i32*)&_params[0];
	glBindVertexArray(array);
}

void glDeleteVertexArrays_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	const GLuint * arrays = (const GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)arrays >= (char*)_mem) && (((char*)arrays - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'arrays' is out of bounds");
		OC_ASSERT_DIALOG((char*)arrays + n*sizeof(const GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'arrays' is out of bounds");
	}
	glDeleteVertexArrays(n, arrays);
}

void glGenVertexArrays_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	GLuint * arrays = (GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)arrays >= (char*)_mem) && (((char*)arrays - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'arrays' is out of bounds");
		OC_ASSERT_DIALOG((char*)arrays + n*sizeof(GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'arrays' is out of bounds");
	}
	glGenVertexArrays(n, arrays);
}

void glIsVertexArray_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint array = (GLuint)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glIsVertexArray(array);
}

void glGetIntegeri_v_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLuint index = (GLuint)*(i32*)&_params[1];
	GLint * data = (GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)data >= (char*)_mem) && (((char*)data - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'data' is out of bounds");
		OC_ASSERT_DIALOG((char*)data + orca_glGetIntegeri_v_data_length(wasm, target)*sizeof(GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'data' is out of bounds");
	}
	glGetIntegeri_v(target, index, data);
}

void glBeginTransformFeedback_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum primitiveMode = (GLenum)*(i32*)&_params[0];
	glBeginTransformFeedback(primitiveMode);
}

void glEndTransformFeedback_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	glEndTransformFeedback();
}

void glBindBufferRange_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLuint index = (GLuint)*(i32*)&_params[1];
	GLuint buffer = (GLuint)*(i32*)&_params[2];
	GLintptr offset = (GLintptr)*(i32*)&_params[3];
	GLsizeiptr size = (GLsizeiptr)*(i32*)&_params[4];
	glBindBufferRange(target, index, buffer, offset, size);
}

void glBindBufferBase_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLuint index = (GLuint)*(i32*)&_params[1];
	GLuint buffer = (GLuint)*(i32*)&_params[2];
	glBindBufferBase(target, index, buffer);
}

void glTransformFeedbackVaryings_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	const GLchar *const* varyings = (const GLchar *const*)((char*)_mem + *(u32*)&_params[2]);
	GLenum bufferMode = (GLenum)*(i32*)&_params[3];
	{
		OC_ASSERT_DIALOG(((char*)varyings >= (char*)_mem) && (((char*)varyings - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'varyings' is out of bounds");
		OC_ASSERT_DIALOG((char*)varyings + count*sizeof(const GLchar *const) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'varyings' is out of bounds");
	}
	glTransformFeedbackVaryings(program, count, varyings, bufferMode);
}

void glGetTransformFeedbackVarying_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLuint index = (GLuint)*(i32*)&_params[1];
	GLsizei bufSize = (GLsizei)*(i32*)&_params[2];
	GLsizei * length = (GLsizei *)((char*)_mem + *(u32*)&_params[3]);
	GLsizei * size = (GLsizei *)((char*)_mem + *(u32*)&_params[4]);
	GLenum * type = (GLenum *)((char*)_mem + *(u32*)&_params[5]);
	GLchar * name = (GLchar *)((char*)_mem + *(u32*)&_params[6]);
	{
		OC_ASSERT_DIALOG(((char*)length >= (char*)_mem) && (((char*)length - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'length' is out of bounds");
		OC_ASSERT_DIALOG((char*)length + 1*sizeof(GLsizei ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'length' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)size >= (char*)_mem) && (((char*)size - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'size' is out of bounds");
		OC_ASSERT_DIALOG((char*)size + 1*sizeof(GLsizei ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'size' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)type >= (char*)_mem) && (((char*)type - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'type' is out of bounds");
		OC_ASSERT_DIALOG((char*)type + 1*sizeof(GLenum ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'type' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)name >= (char*)_mem) && (((char*)name - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'name' is out of bounds");
		OC_ASSERT_DIALOG((char*)name + bufSize*sizeof(GLchar ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'name' is out of bounds");
	}
	glGetTransformFeedbackVarying(program, index, bufSize, length, size, type, name);
}

void glGetVertexAttribIiv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint index = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + 1*sizeof(GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glGetVertexAttribIiv(index, pname, params);
}

void glGetVertexAttribIuiv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint index = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLuint * params = (GLuint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + 1*sizeof(GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glGetVertexAttribIuiv(index, pname, params);
}

void glVertexAttribI4i_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint index = (GLuint)*(i32*)&_params[0];
	GLint x = (GLint)*(i32*)&_params[1];
	GLint y = (GLint)*(i32*)&_params[2];
	GLint z = (GLint)*(i32*)&_params[3];
	GLint w = (GLint)*(i32*)&_params[4];
	glVertexAttribI4i(index, x, y, z, w);
}

void glVertexAttribI4ui_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint index = (GLuint)*(i32*)&_params[0];
	GLuint x = (GLuint)*(i32*)&_params[1];
	GLuint y = (GLuint)*(i32*)&_params[2];
	GLuint z = (GLuint)*(i32*)&_params[3];
	GLuint w = (GLuint)*(i32*)&_params[4];
	glVertexAttribI4ui(index, x, y, z, w);
}

void glVertexAttribI4iv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint index = (GLuint)*(i32*)&_params[0];
	const GLint * v = (const GLint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)v >= (char*)_mem) && (((char*)v - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'v' is out of bounds");
		OC_ASSERT_DIALOG((char*)v + 4*sizeof(const GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'v' is out of bounds");
	}
	glVertexAttribI4iv(index, v);
}

void glVertexAttribI4uiv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint index = (GLuint)*(i32*)&_params[0];
	const GLuint * v = (const GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)v >= (char*)_mem) && (((char*)v - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'v' is out of bounds");
		OC_ASSERT_DIALOG((char*)v + 4*sizeof(const GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'v' is out of bounds");
	}
	glVertexAttribI4uiv(index, v);
}

void glGetUniformuiv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLuint * params = (GLuint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetUniformuiv_params_length(wasm, program, location)*sizeof(GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glGetUniformuiv(program, location, params);
}

void glGetFragDataLocation_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	const GLchar * name = (const GLchar *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)name >= (char*)_mem) && (((char*)name - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'name' is out of bounds");
		OC_ASSERT_DIALOG((char*)name + orca_glGetFragDataLocation_name_length(wasm, name)*sizeof(const GLchar ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'name' is out of bounds");
	}
	*((i32*)&_returns[0]) = (i32)glGetFragDataLocation(program, name);
}

void glUniform1ui_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLuint v0 = (GLuint)*(i32*)&_params[1];
	glUniform1ui(location, v0);
}

void glUniform2ui_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLuint v0 = (GLuint)*(i32*)&_params[1];
	GLuint v1 = (GLuint)*(i32*)&_params[2];
	glUniform2ui(location, v0, v1);
}

void glUniform3ui_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLuint v0 = (GLuint)*(i32*)&_params[1];
	GLuint v1 = (GLuint)*(i32*)&_params[2];
	GLuint v2 = (GLuint)*(i32*)&_params[3];
	glUniform3ui(location, v0, v1, v2);
}

void glUniform4ui_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLuint v0 = (GLuint)*(i32*)&_params[1];
	GLuint v1 = (GLuint)*(i32*)&_params[2];
	GLuint v2 = (GLuint)*(i32*)&_params[3];
	GLuint v3 = (GLuint)*(i32*)&_params[4];
	glUniform4ui(location, v0, v1, v2, v3);
}

void glUniform1uiv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	const GLuint * value = (const GLuint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 1*count*sizeof(const GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glUniform1uiv(location, count, value);
}

void glUniform2uiv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	const GLuint * value = (const GLuint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 2*count*sizeof(const GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glUniform2uiv(location, count, value);
}

void glUniform3uiv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	const GLuint * value = (const GLuint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 3*count*sizeof(const GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glUniform3uiv(location, count, value);
}

void glUniform4uiv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLint location = (GLint)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	const GLuint * value = (const GLuint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 4*count*sizeof(const GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glUniform4uiv(location, count, value);
}

void glClearBufferiv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum buffer = (GLenum)*(i32*)&_params[0];
	GLint drawbuffer = (GLint)*(i32*)&_params[1];
	const GLint * value = (const GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + orca_glClearBufferiv_value_length(wasm, buffer)*sizeof(const GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glClearBufferiv(buffer, drawbuffer, value);
}

void glClearBufferuiv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum buffer = (GLenum)*(i32*)&_params[0];
	GLint drawbuffer = (GLint)*(i32*)&_params[1];
	const GLuint * value = (const GLuint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + orca_glClearBufferuiv_value_length(wasm, buffer)*sizeof(const GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glClearBufferuiv(buffer, drawbuffer, value);
}

void glClearBufferfv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum buffer = (GLenum)*(i32*)&_params[0];
	GLint drawbuffer = (GLint)*(i32*)&_params[1];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + orca_glClearBufferfv_value_length(wasm, buffer)*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glClearBufferfv(buffer, drawbuffer, value);
}

void glClearBufferfi_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum buffer = (GLenum)*(i32*)&_params[0];
	GLint drawbuffer = (GLint)*(i32*)&_params[1];
	GLfloat depth = (GLfloat)*(f32*)&_params[2];
	GLint stencil = (GLint)*(i32*)&_params[3];
	glClearBufferfi(buffer, drawbuffer, depth, stencil);
}

void glCopyBufferSubData_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum readTarget = (GLenum)*(i32*)&_params[0];
	GLenum writeTarget = (GLenum)*(i32*)&_params[1];
	GLintptr readOffset = (GLintptr)*(i32*)&_params[2];
	GLintptr writeOffset = (GLintptr)*(i32*)&_params[3];
	GLsizeiptr size = (GLsizeiptr)*(i32*)&_params[4];
	glCopyBufferSubData(readTarget, writeTarget, readOffset, writeOffset, size);
}

void glGetActiveUniformsiv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLsizei uniformCount = (GLsizei)*(i32*)&_params[1];
	const GLuint * uniformIndices = (const GLuint *)((char*)_mem + *(u32*)&_params[2]);
	GLenum pname = (GLenum)*(i32*)&_params[3];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)uniformIndices >= (char*)_mem) && (((char*)uniformIndices - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'uniformIndices' is out of bounds");
		OC_ASSERT_DIALOG((char*)uniformIndices + uniformCount*sizeof(const GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'uniformIndices' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetActiveUniformsiv_params_length(wasm, uniformCount, pname)*sizeof(GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glGetActiveUniformsiv(program, uniformCount, uniformIndices, pname, params);
}

void glGetUniformBlockIndex_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	const GLchar * uniformBlockName = (const GLchar *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)uniformBlockName >= (char*)_mem) && (((char*)uniformBlockName - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'uniformBlockName' is out of bounds");
		OC_ASSERT_DIALOG((char*)uniformBlockName + orca_glGetUniformBlockIndex_uniformBlockName_length(wasm, uniformBlockName)*sizeof(const GLchar ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'uniformBlockName' is out of bounds");
	}
	*((i32*)&_returns[0]) = (i32)glGetUniformBlockIndex(program, uniformBlockName);
}

void glGetActiveUniformBlockiv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLuint uniformBlockIndex = (GLuint)*(i32*)&_params[1];
	GLenum pname = (GLenum)*(i32*)&_params[2];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetActiveUniformBlockiv_params_length(wasm, program, uniformBlockIndex, pname)*sizeof(GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glGetActiveUniformBlockiv(program, uniformBlockIndex, pname, params);
}

void glGetActiveUniformBlockName_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLuint uniformBlockIndex = (GLuint)*(i32*)&_params[1];
	GLsizei bufSize = (GLsizei)*(i32*)&_params[2];
	GLsizei * length = (GLsizei *)((char*)_mem + *(u32*)&_params[3]);
	GLchar * uniformBlockName = (GLchar *)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)length >= (char*)_mem) && (((char*)length - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'length' is out of bounds");
		OC_ASSERT_DIALOG((char*)length + 1*sizeof(GLsizei ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'length' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)uniformBlockName >= (char*)_mem) && (((char*)uniformBlockName - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'uniformBlockName' is out of bounds");
		OC_ASSERT_DIALOG((char*)uniformBlockName + bufSize*sizeof(GLchar ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'uniformBlockName' is out of bounds");
	}
	glGetActiveUniformBlockName(program, uniformBlockIndex, bufSize, length, uniformBlockName);
}

void glUniformBlockBinding_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLuint uniformBlockIndex = (GLuint)*(i32*)&_params[1];
	GLuint uniformBlockBinding = (GLuint)*(i32*)&_params[2];
	glUniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
}

void glDrawArraysInstanced_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum mode = (GLenum)*(i32*)&_params[0];
	GLint first = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	GLsizei instancecount = (GLsizei)*(i32*)&_params[3];
	glDrawArraysInstanced(mode, first, count, instancecount);
}

void glDrawElementsInstanced_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum mode = (GLenum)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	GLenum type = (GLenum)*(i32*)&_params[2];
	const void * indices = (const void *)((char*)_mem + *(u32*)&_params[3]);
	GLsizei instancecount = (GLsizei)*(i32*)&_params[4];
	{
		OC_ASSERT_DIALOG(((char*)indices >= (char*)_mem) && (((char*)indices - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'indices' is out of bounds");
		OC_ASSERT_DIALOG((char*)indices + orca_glDrawElementsInstanced_indices_length(wasm, count, type) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'indices' is out of bounds");
	}
	glDrawElementsInstanced(mode, count, type, indices, instancecount);
}

void glFenceSync_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum condition = (GLenum)*(i32*)&_params[0];
	GLbitfield flags = (GLbitfield)*(i32*)&_params[1];
	*((i64*)&_returns[0]) = (i64)glFenceSync(condition, flags);
}

void glIsSync_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLsync sync = (GLsync)*(i64*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glIsSync(sync);
}

void glDeleteSync_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLsync sync = (GLsync)*(i64*)&_params[0];
	glDeleteSync(sync);
}

void glClientWaitSync_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLsync sync = (GLsync)*(i64*)&_params[0];
	GLbitfield flags = (GLbitfield)*(i32*)&_params[1];
	GLuint64 timeout = (GLuint64)*(i64*)&_params[2];
	*((i32*)&_returns[0]) = (i32)glClientWaitSync(sync, flags, timeout);
}

void glWaitSync_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLsync sync = (GLsync)*(i64*)&_params[0];
	GLbitfield flags = (GLbitfield)*(i32*)&_params[1];
	GLuint64 timeout = (GLuint64)*(i64*)&_params[2];
	glWaitSync(sync, flags, timeout);
}

void glGetInteger64v_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum pname = (GLenum)*(i32*)&_params[0];
	GLint64 * data = (GLint64 *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)data >= (char*)_mem) && (((char*)data - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'data' is out of bounds");
		OC_ASSERT_DIALOG((char*)data + orca_glGetInteger64v_data_length(wasm, pname)*sizeof(GLint64 ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'data' is out of bounds");
	}
	glGetInteger64v(pname, data);
}

void glGetSynciv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLsync sync = (GLsync)*(i64*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	GLsizei * length = (GLsizei *)((char*)_mem + *(u32*)&_params[3]);
	GLint * values = (GLint *)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)length >= (char*)_mem) && (((char*)length - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'length' is out of bounds");
		OC_ASSERT_DIALOG((char*)length + 1*sizeof(GLsizei ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'length' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)values >= (char*)_mem) && (((char*)values - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'values' is out of bounds");
		OC_ASSERT_DIALOG((char*)values + count*sizeof(GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'values' is out of bounds");
	}
	glGetSynciv(sync, pname, count, length, values);
}

void glGetInteger64i_v_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLuint index = (GLuint)*(i32*)&_params[1];
	GLint64 * data = (GLint64 *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)data >= (char*)_mem) && (((char*)data - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'data' is out of bounds");
		OC_ASSERT_DIALOG((char*)data + orca_glGetInteger64i_v_data_length(wasm, target)*sizeof(GLint64 ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'data' is out of bounds");
	}
	glGetInteger64i_v(target, index, data);
}

void glGetBufferParameteri64v_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint64 * params = (GLint64 *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetBufferParameteri64v_params_length(wasm, pname)*sizeof(GLint64 ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glGetBufferParameteri64v(target, pname, params);
}

void glGenSamplers_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLsizei count = (GLsizei)*(i32*)&_params[0];
	GLuint * samplers = (GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)samplers >= (char*)_mem) && (((char*)samplers - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'samplers' is out of bounds");
		OC_ASSERT_DIALOG((char*)samplers + count*sizeof(GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'samplers' is out of bounds");
	}
	glGenSamplers(count, samplers);
}

void glDeleteSamplers_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLsizei count = (GLsizei)*(i32*)&_params[0];
	const GLuint * samplers = (const GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)samplers >= (char*)_mem) && (((char*)samplers - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'samplers' is out of bounds");
		OC_ASSERT_DIALOG((char*)samplers + count*sizeof(const GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'samplers' is out of bounds");
	}
	glDeleteSamplers(count, samplers);
}

void glIsSampler_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint sampler = (GLuint)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glIsSampler(sampler);
}

void glBindSampler_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint unit = (GLuint)*(i32*)&_params[0];
	GLuint sampler = (GLuint)*(i32*)&_params[1];
	glBindSampler(unit, sampler);
}

void glSamplerParameteri_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint sampler = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint param = (GLint)*(i32*)&_params[2];
	glSamplerParameteri(sampler, pname, param);
}

void glSamplerParameteriv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint sampler = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	const GLint * param = (const GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)param >= (char*)_mem) && (((char*)param - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'param' is out of bounds");
		OC_ASSERT_DIALOG((char*)param + orca_glSamplerParameteriv_param_length(wasm, pname)*sizeof(const GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'param' is out of bounds");
	}
	glSamplerParameteriv(sampler, pname, param);
}

void glSamplerParameterf_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint sampler = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLfloat param = (GLfloat)*(f32*)&_params[2];
	glSamplerParameterf(sampler, pname, param);
}

void glSamplerParameterfv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint sampler = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	const GLfloat * param = (const GLfloat *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)param >= (char*)_mem) && (((char*)param - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'param' is out of bounds");
		OC_ASSERT_DIALOG((char*)param + orca_glSamplerParameterfv_param_length(wasm, pname)*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'param' is out of bounds");
	}
	glSamplerParameterfv(sampler, pname, param);
}

void glGetSamplerParameteriv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint sampler = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetSamplerParameteriv_params_length(wasm, pname)*sizeof(GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glGetSamplerParameteriv(sampler, pname, params);
}

void glGetSamplerParameterfv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint sampler = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLfloat * params = (GLfloat *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetSamplerParameterfv_params_length(wasm, pname)*sizeof(GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glGetSamplerParameterfv(sampler, pname, params);
}

void glVertexAttribDivisor_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint index = (GLuint)*(i32*)&_params[0];
	GLuint divisor = (GLuint)*(i32*)&_params[1];
	glVertexAttribDivisor(index, divisor);
}

void glBindTransformFeedback_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLuint id = (GLuint)*(i32*)&_params[1];
	glBindTransformFeedback(target, id);
}

void glDeleteTransformFeedbacks_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	const GLuint * ids = (const GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)ids >= (char*)_mem) && (((char*)ids - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'ids' is out of bounds");
		OC_ASSERT_DIALOG((char*)ids + n*sizeof(const GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'ids' is out of bounds");
	}
	glDeleteTransformFeedbacks(n, ids);
}

void glGenTransformFeedbacks_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	GLuint * ids = (GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)ids >= (char*)_mem) && (((char*)ids - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'ids' is out of bounds");
		OC_ASSERT_DIALOG((char*)ids + n*sizeof(GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'ids' is out of bounds");
	}
	glGenTransformFeedbacks(n, ids);
}

void glIsTransformFeedback_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint id = (GLuint)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glIsTransformFeedback(id);
}

void glPauseTransformFeedback_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	glPauseTransformFeedback();
}

void glResumeTransformFeedback_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	glResumeTransformFeedback();
}

void glGetProgramBinary_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLsizei bufSize = (GLsizei)*(i32*)&_params[1];
	GLsizei * length = (GLsizei *)((char*)_mem + *(u32*)&_params[2]);
	GLenum * binaryFormat = (GLenum *)((char*)_mem + *(u32*)&_params[3]);
	void * binary = (void *)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)length >= (char*)_mem) && (((char*)length - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'length' is out of bounds");
		OC_ASSERT_DIALOG((char*)length + 1*sizeof(GLsizei ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'length' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)binaryFormat >= (char*)_mem) && (((char*)binaryFormat - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'binaryFormat' is out of bounds");
		OC_ASSERT_DIALOG((char*)binaryFormat + 1*sizeof(GLenum ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'binaryFormat' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)binary >= (char*)_mem) && (((char*)binary - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'binary' is out of bounds");
		OC_ASSERT_DIALOG((char*)binary + bufSize <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'binary' is out of bounds");
	}
	glGetProgramBinary(program, bufSize, length, binaryFormat, binary);
}

void glProgramBinary_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLenum binaryFormat = (GLenum)*(i32*)&_params[1];
	const void * binary = (const void *)((char*)_mem + *(u32*)&_params[2]);
	GLsizei length = (GLsizei)*(i32*)&_params[3];
	{
		OC_ASSERT_DIALOG(((char*)binary >= (char*)_mem) && (((char*)binary - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'binary' is out of bounds");
		OC_ASSERT_DIALOG((char*)binary + length <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'binary' is out of bounds");
	}
	glProgramBinary(program, binaryFormat, binary, length);
}

void glProgramParameteri_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint value = (GLint)*(i32*)&_params[2];
	glProgramParameteri(program, pname, value);
}

void glInvalidateFramebuffer_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLsizei numAttachments = (GLsizei)*(i32*)&_params[1];
	const GLenum * attachments = (const GLenum *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)attachments >= (char*)_mem) && (((char*)attachments - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'attachments' is out of bounds");
		OC_ASSERT_DIALOG((char*)attachments + numAttachments*sizeof(const GLenum ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'attachments' is out of bounds");
	}
	glInvalidateFramebuffer(target, numAttachments, attachments);
}

void glInvalidateSubFramebuffer_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLsizei numAttachments = (GLsizei)*(i32*)&_params[1];
	const GLenum * attachments = (const GLenum *)((char*)_mem + *(u32*)&_params[2]);
	GLint x = (GLint)*(i32*)&_params[3];
	GLint y = (GLint)*(i32*)&_params[4];
	GLsizei width = (GLsizei)*(i32*)&_params[5];
	GLsizei height = (GLsizei)*(i32*)&_params[6];
	{
		OC_ASSERT_DIALOG(((char*)attachments >= (char*)_mem) && (((char*)attachments - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'attachments' is out of bounds");
		OC_ASSERT_DIALOG((char*)attachments + numAttachments*sizeof(const GLenum ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'attachments' is out of bounds");
	}
	glInvalidateSubFramebuffer(target, numAttachments, attachments, x, y, width, height);
}

void glTexStorage2D_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLsizei levels = (GLsizei)*(i32*)&_params[1];
	GLenum internalformat = (GLenum)*(i32*)&_params[2];
	GLsizei width = (GLsizei)*(i32*)&_params[3];
	GLsizei height = (GLsizei)*(i32*)&_params[4];
	glTexStorage2D(target, levels, internalformat, width, height);
}

void glTexStorage3D_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLsizei levels = (GLsizei)*(i32*)&_params[1];
	GLenum internalformat = (GLenum)*(i32*)&_params[2];
	GLsizei width = (GLsizei)*(i32*)&_params[3];
	GLsizei height = (GLsizei)*(i32*)&_params[4];
	GLsizei depth = (GLsizei)*(i32*)&_params[5];
	glTexStorage3D(target, levels, internalformat, width, height, depth);
}

void glGetInternalformativ_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum internalformat = (GLenum)*(i32*)&_params[1];
	GLenum pname = (GLenum)*(i32*)&_params[2];
	GLsizei count = (GLsizei)*(i32*)&_params[3];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + count*sizeof(GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glGetInternalformativ(target, internalformat, pname, count, params);
}

void glDispatchCompute_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint num_groups_x = (GLuint)*(i32*)&_params[0];
	GLuint num_groups_y = (GLuint)*(i32*)&_params[1];
	GLuint num_groups_z = (GLuint)*(i32*)&_params[2];
	glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
}

void glDispatchComputeIndirect_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLintptr indirect = (GLintptr)*(i32*)&_params[0];
	glDispatchComputeIndirect(indirect);
}

void glDrawArraysIndirect_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum mode = (GLenum)*(i32*)&_params[0];
	const void * indirect = (const void *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)indirect >= (char*)_mem) && (((char*)indirect - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'indirect' is out of bounds");
		OC_ASSERT_DIALOG((char*)indirect + orca_glDrawArraysIndirect_indirect_length(wasm, indirect) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'indirect' is out of bounds");
	}
	glDrawArraysIndirect(mode, indirect);
}

void glDrawElementsIndirect_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum mode = (GLenum)*(i32*)&_params[0];
	GLenum type = (GLenum)*(i32*)&_params[1];
	const void * indirect = (const void *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)indirect >= (char*)_mem) && (((char*)indirect - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'indirect' is out of bounds");
		OC_ASSERT_DIALOG((char*)indirect + orca_glDrawElementsIndirect_indirect_length(wasm, indirect) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'indirect' is out of bounds");
	}
	glDrawElementsIndirect(mode, type, indirect);
}

void glFramebufferParameteri_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint param = (GLint)*(i32*)&_params[2];
	glFramebufferParameteri(target, pname, param);
}

void glGetFramebufferParameteriv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetFramebufferParameteriv_params_length(wasm, pname)*sizeof(GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glGetFramebufferParameteriv(target, pname, params);
}

void glGetProgramInterfaceiv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLenum programInterface = (GLenum)*(i32*)&_params[1];
	GLenum pname = (GLenum)*(i32*)&_params[2];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetProgramInterfaceiv_params_length(wasm, pname)*sizeof(GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glGetProgramInterfaceiv(program, programInterface, pname, params);
}

void glGetProgramResourceIndex_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLenum programInterface = (GLenum)*(i32*)&_params[1];
	const GLchar * name = (const GLchar *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)name >= (char*)_mem) && (((char*)name - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'name' is out of bounds");
		OC_ASSERT_DIALOG((char*)name + orca_glGetProgramResourceIndex_name_length(wasm, name)*sizeof(const GLchar ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'name' is out of bounds");
	}
	*((i32*)&_returns[0]) = (i32)glGetProgramResourceIndex(program, programInterface, name);
}

void glGetProgramResourceName_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLenum programInterface = (GLenum)*(i32*)&_params[1];
	GLuint index = (GLuint)*(i32*)&_params[2];
	GLsizei bufSize = (GLsizei)*(i32*)&_params[3];
	GLsizei * length = (GLsizei *)((char*)_mem + *(u32*)&_params[4]);
	GLchar * name = (GLchar *)((char*)_mem + *(u32*)&_params[5]);
	{
		OC_ASSERT_DIALOG(((char*)length >= (char*)_mem) && (((char*)length - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'length' is out of bounds");
		OC_ASSERT_DIALOG((char*)length + 1*sizeof(GLsizei ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'length' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)name >= (char*)_mem) && (((char*)name - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'name' is out of bounds");
		OC_ASSERT_DIALOG((char*)name + bufSize*sizeof(GLchar ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'name' is out of bounds");
	}
	glGetProgramResourceName(program, programInterface, index, bufSize, length, name);
}

void glGetProgramResourceiv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLenum programInterface = (GLenum)*(i32*)&_params[1];
	GLuint index = (GLuint)*(i32*)&_params[2];
	GLsizei propCount = (GLsizei)*(i32*)&_params[3];
	const GLenum * props = (const GLenum *)((char*)_mem + *(u32*)&_params[4]);
	GLsizei count = (GLsizei)*(i32*)&_params[5];
	GLsizei * length = (GLsizei *)((char*)_mem + *(u32*)&_params[6]);
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[7]);
	{
		OC_ASSERT_DIALOG(((char*)props >= (char*)_mem) && (((char*)props - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'props' is out of bounds");
		OC_ASSERT_DIALOG((char*)props + propCount*sizeof(const GLenum ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'props' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)length >= (char*)_mem) && (((char*)length - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'length' is out of bounds");
		OC_ASSERT_DIALOG((char*)length + 1*sizeof(GLsizei ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'length' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + count*sizeof(GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glGetProgramResourceiv(program, programInterface, index, propCount, props, count, length, params);
}

void glGetProgramResourceLocation_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLenum programInterface = (GLenum)*(i32*)&_params[1];
	const GLchar * name = (const GLchar *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)name >= (char*)_mem) && (((char*)name - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'name' is out of bounds");
		OC_ASSERT_DIALOG((char*)name + orca_glGetProgramResourceLocation_name_length(wasm, name)*sizeof(const GLchar ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'name' is out of bounds");
	}
	*((i32*)&_returns[0]) = (i32)glGetProgramResourceLocation(program, programInterface, name);
}

void glUseProgramStages_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint pipeline = (GLuint)*(i32*)&_params[0];
	GLbitfield stages = (GLbitfield)*(i32*)&_params[1];
	GLuint program = (GLuint)*(i32*)&_params[2];
	glUseProgramStages(pipeline, stages, program);
}

void glActiveShaderProgram_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint pipeline = (GLuint)*(i32*)&_params[0];
	GLuint program = (GLuint)*(i32*)&_params[1];
	glActiveShaderProgram(pipeline, program);
}

void glCreateShaderProgramv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum type = (GLenum)*(i32*)&_params[0];
	GLsizei count = (GLsizei)*(i32*)&_params[1];
	const GLchar *const* strings = (const GLchar *const*)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)strings >= (char*)_mem) && (((char*)strings - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'strings' is out of bounds");
		OC_ASSERT_DIALOG((char*)strings + count*sizeof(const GLchar *const) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'strings' is out of bounds");
	}
	*((i32*)&_returns[0]) = (i32)glCreateShaderProgramv(type, count, strings);
}

void glBindProgramPipeline_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint pipeline = (GLuint)*(i32*)&_params[0];
	glBindProgramPipeline(pipeline);
}

void glDeleteProgramPipelines_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	const GLuint * pipelines = (const GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)pipelines >= (char*)_mem) && (((char*)pipelines - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'pipelines' is out of bounds");
		OC_ASSERT_DIALOG((char*)pipelines + n*sizeof(const GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'pipelines' is out of bounds");
	}
	glDeleteProgramPipelines(n, pipelines);
}

void glGenProgramPipelines_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLsizei n = (GLsizei)*(i32*)&_params[0];
	GLuint * pipelines = (GLuint *)((char*)_mem + *(u32*)&_params[1]);
	{
		OC_ASSERT_DIALOG(((char*)pipelines >= (char*)_mem) && (((char*)pipelines - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'pipelines' is out of bounds");
		OC_ASSERT_DIALOG((char*)pipelines + n*sizeof(GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'pipelines' is out of bounds");
	}
	glGenProgramPipelines(n, pipelines);
}

void glIsProgramPipeline_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint pipeline = (GLuint)*(i32*)&_params[0];
	*((i32*)&_returns[0]) = (i32)glIsProgramPipeline(pipeline);
}

void glGetProgramPipelineiv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint pipeline = (GLuint)*(i32*)&_params[0];
	GLenum pname = (GLenum)*(i32*)&_params[1];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetProgramPipelineiv_params_length(wasm, pname)*sizeof(GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glGetProgramPipelineiv(pipeline, pname, params);
}

void glProgramUniform1i_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLint v0 = (GLint)*(i32*)&_params[2];
	glProgramUniform1i(program, location, v0);
}

void glProgramUniform2i_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLint v0 = (GLint)*(i32*)&_params[2];
	GLint v1 = (GLint)*(i32*)&_params[3];
	glProgramUniform2i(program, location, v0, v1);
}

void glProgramUniform3i_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLint v0 = (GLint)*(i32*)&_params[2];
	GLint v1 = (GLint)*(i32*)&_params[3];
	GLint v2 = (GLint)*(i32*)&_params[4];
	glProgramUniform3i(program, location, v0, v1, v2);
}

void glProgramUniform4i_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLint v0 = (GLint)*(i32*)&_params[2];
	GLint v1 = (GLint)*(i32*)&_params[3];
	GLint v2 = (GLint)*(i32*)&_params[4];
	GLint v3 = (GLint)*(i32*)&_params[5];
	glProgramUniform4i(program, location, v0, v1, v2, v3);
}

void glProgramUniform1ui_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLuint v0 = (GLuint)*(i32*)&_params[2];
	glProgramUniform1ui(program, location, v0);
}

void glProgramUniform2ui_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLuint v0 = (GLuint)*(i32*)&_params[2];
	GLuint v1 = (GLuint)*(i32*)&_params[3];
	glProgramUniform2ui(program, location, v0, v1);
}

void glProgramUniform3ui_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLuint v0 = (GLuint)*(i32*)&_params[2];
	GLuint v1 = (GLuint)*(i32*)&_params[3];
	GLuint v2 = (GLuint)*(i32*)&_params[4];
	glProgramUniform3ui(program, location, v0, v1, v2);
}

void glProgramUniform4ui_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLuint v0 = (GLuint)*(i32*)&_params[2];
	GLuint v1 = (GLuint)*(i32*)&_params[3];
	GLuint v2 = (GLuint)*(i32*)&_params[4];
	GLuint v3 = (GLuint)*(i32*)&_params[5];
	glProgramUniform4ui(program, location, v0, v1, v2, v3);
}

void glProgramUniform1f_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLfloat v0 = (GLfloat)*(f32*)&_params[2];
	glProgramUniform1f(program, location, v0);
}

void glProgramUniform2f_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLfloat v0 = (GLfloat)*(f32*)&_params[2];
	GLfloat v1 = (GLfloat)*(f32*)&_params[3];
	glProgramUniform2f(program, location, v0, v1);
}

void glProgramUniform3f_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLfloat v0 = (GLfloat)*(f32*)&_params[2];
	GLfloat v1 = (GLfloat)*(f32*)&_params[3];
	GLfloat v2 = (GLfloat)*(f32*)&_params[4];
	glProgramUniform3f(program, location, v0, v1, v2);
}

void glProgramUniform4f_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLfloat v0 = (GLfloat)*(f32*)&_params[2];
	GLfloat v1 = (GLfloat)*(f32*)&_params[3];
	GLfloat v2 = (GLfloat)*(f32*)&_params[4];
	GLfloat v3 = (GLfloat)*(f32*)&_params[5];
	glProgramUniform4f(program, location, v0, v1, v2, v3);
}

void glProgramUniform1iv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	const GLint * value = (const GLint *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + count*sizeof(const GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glProgramUniform1iv(program, location, count, value);
}

void glProgramUniform2iv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	const GLint * value = (const GLint *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 2*count*sizeof(const GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glProgramUniform2iv(program, location, count, value);
}

void glProgramUniform3iv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	const GLint * value = (const GLint *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 3*count*sizeof(const GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glProgramUniform3iv(program, location, count, value);
}

void glProgramUniform4iv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	const GLint * value = (const GLint *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 4*count*sizeof(const GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glProgramUniform4iv(program, location, count, value);
}

void glProgramUniform1uiv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	const GLuint * value = (const GLuint *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + count*sizeof(const GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glProgramUniform1uiv(program, location, count, value);
}

void glProgramUniform2uiv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	const GLuint * value = (const GLuint *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 2*count*sizeof(const GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glProgramUniform2uiv(program, location, count, value);
}

void glProgramUniform3uiv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	const GLuint * value = (const GLuint *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 3*count*sizeof(const GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glProgramUniform3uiv(program, location, count, value);
}

void glProgramUniform4uiv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	const GLuint * value = (const GLuint *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 4*count*sizeof(const GLuint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glProgramUniform4uiv(program, location, count, value);
}

void glProgramUniform1fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + count*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glProgramUniform1fv(program, location, count, value);
}

void glProgramUniform2fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 2*count*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glProgramUniform2fv(program, location, count, value);
}

void glProgramUniform3fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 3*count*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glProgramUniform3fv(program, location, count, value);
}

void glProgramUniform4fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 4*count*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glProgramUniform4fv(program, location, count, value);
}

void glProgramUniformMatrix2fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	GLboolean transpose = (GLboolean)*(i32*)&_params[3];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 4*count*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glProgramUniformMatrix2fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix3fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	GLboolean transpose = (GLboolean)*(i32*)&_params[3];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 9*count*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glProgramUniformMatrix3fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix4fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	GLboolean transpose = (GLboolean)*(i32*)&_params[3];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 16*count*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glProgramUniformMatrix4fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix2x3fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	GLboolean transpose = (GLboolean)*(i32*)&_params[3];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 6*count*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glProgramUniformMatrix2x3fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix3x2fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	GLboolean transpose = (GLboolean)*(i32*)&_params[3];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 6*count*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glProgramUniformMatrix3x2fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix2x4fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	GLboolean transpose = (GLboolean)*(i32*)&_params[3];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 8*count*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glProgramUniformMatrix2x4fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix4x2fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	GLboolean transpose = (GLboolean)*(i32*)&_params[3];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 8*count*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glProgramUniformMatrix4x2fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix3x4fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	GLboolean transpose = (GLboolean)*(i32*)&_params[3];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 12*count*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glProgramUniformMatrix3x4fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix4x3fv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint program = (GLuint)*(i32*)&_params[0];
	GLint location = (GLint)*(i32*)&_params[1];
	GLsizei count = (GLsizei)*(i32*)&_params[2];
	GLboolean transpose = (GLboolean)*(i32*)&_params[3];
	const GLfloat * value = (const GLfloat *)((char*)_mem + *(u32*)&_params[4]);
	{
		OC_ASSERT_DIALOG(((char*)value >= (char*)_mem) && (((char*)value - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
		OC_ASSERT_DIALOG((char*)value + 12*count*sizeof(const GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'value' is out of bounds");
	}
	glProgramUniformMatrix4x3fv(program, location, count, transpose, value);
}

void glValidateProgramPipeline_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint pipeline = (GLuint)*(i32*)&_params[0];
	glValidateProgramPipeline(pipeline);
}

void glGetProgramPipelineInfoLog_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint pipeline = (GLuint)*(i32*)&_params[0];
	GLsizei bufSize = (GLsizei)*(i32*)&_params[1];
	GLsizei * length = (GLsizei *)((char*)_mem + *(u32*)&_params[2]);
	GLchar * infoLog = (GLchar *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)length >= (char*)_mem) && (((char*)length - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'length' is out of bounds");
		OC_ASSERT_DIALOG((char*)length + 1*sizeof(GLsizei ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'length' is out of bounds");
	}
	{
		OC_ASSERT_DIALOG(((char*)infoLog >= (char*)_mem) && (((char*)infoLog - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'infoLog' is out of bounds");
		OC_ASSERT_DIALOG((char*)infoLog + bufSize*sizeof(GLchar ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'infoLog' is out of bounds");
	}
	glGetProgramPipelineInfoLog(pipeline, bufSize, length, infoLog);
}

void glBindImageTexture_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint unit = (GLuint)*(i32*)&_params[0];
	GLuint texture = (GLuint)*(i32*)&_params[1];
	GLint level = (GLint)*(i32*)&_params[2];
	GLboolean layered = (GLboolean)*(i32*)&_params[3];
	GLint layer = (GLint)*(i32*)&_params[4];
	GLenum access = (GLenum)*(i32*)&_params[5];
	GLenum format = (GLenum)*(i32*)&_params[6];
	glBindImageTexture(unit, texture, level, layered, layer, access, format);
}

void glGetBooleani_v_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLuint index = (GLuint)*(i32*)&_params[1];
	GLboolean * data = (GLboolean *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)data >= (char*)_mem) && (((char*)data - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'data' is out of bounds");
		OC_ASSERT_DIALOG((char*)data + orca_glGetBooleani_v_data_length(wasm, target)*sizeof(GLboolean ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'data' is out of bounds");
	}
	glGetBooleani_v(target, index, data);
}

void glMemoryBarrier_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLbitfield barriers = (GLbitfield)*(i32*)&_params[0];
	glMemoryBarrier(barriers);
}

void glMemoryBarrierByRegion_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLbitfield barriers = (GLbitfield)*(i32*)&_params[0];
	glMemoryBarrierByRegion(barriers);
}

void glTexStorage2DMultisample_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLsizei samples = (GLsizei)*(i32*)&_params[1];
	GLenum internalformat = (GLenum)*(i32*)&_params[2];
	GLsizei width = (GLsizei)*(i32*)&_params[3];
	GLsizei height = (GLsizei)*(i32*)&_params[4];
	GLboolean fixedsamplelocations = (GLboolean)*(i32*)&_params[5];
	glTexStorage2DMultisample(target, samples, internalformat, width, height, fixedsamplelocations);
}

void glGetMultisamplefv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum pname = (GLenum)*(i32*)&_params[0];
	GLuint index = (GLuint)*(i32*)&_params[1];
	GLfloat * val = (GLfloat *)((char*)_mem + *(u32*)&_params[2]);
	{
		OC_ASSERT_DIALOG(((char*)val >= (char*)_mem) && (((char*)val - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'val' is out of bounds");
		OC_ASSERT_DIALOG((char*)val + orca_glGetMultisamplefv_val_length(wasm, pname)*sizeof(GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'val' is out of bounds");
	}
	glGetMultisamplefv(pname, index, val);
}

void glSampleMaski_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint maskNumber = (GLuint)*(i32*)&_params[0];
	GLbitfield mask = (GLbitfield)*(i32*)&_params[1];
	glSampleMaski(maskNumber, mask);
}

void glGetTexLevelParameteriv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLint level = (GLint)*(i32*)&_params[1];
	GLenum pname = (GLenum)*(i32*)&_params[2];
	GLint * params = (GLint *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetTexLevelParameteriv_params_length(wasm, pname)*sizeof(GLint ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glGetTexLevelParameteriv(target, level, pname, params);
}

void glGetTexLevelParameterfv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLenum target = (GLenum)*(i32*)&_params[0];
	GLint level = (GLint)*(i32*)&_params[1];
	GLenum pname = (GLenum)*(i32*)&_params[2];
	GLfloat * params = (GLfloat *)((char*)_mem + *(u32*)&_params[3]);
	{
		OC_ASSERT_DIALOG(((char*)params >= (char*)_mem) && (((char*)params - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
		OC_ASSERT_DIALOG((char*)params + orca_glGetTexLevelParameterfv_params_length(wasm, pname)*sizeof(GLfloat ) <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter 'params' is out of bounds");
	}
	glGetTexLevelParameterfv(target, level, pname, params);
}

void glBindVertexBuffer_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint bindingindex = (GLuint)*(i32*)&_params[0];
	GLuint buffer = (GLuint)*(i32*)&_params[1];
	GLintptr offset = (GLintptr)*(i32*)&_params[2];
	GLsizei stride = (GLsizei)*(i32*)&_params[3];
	glBindVertexBuffer(bindingindex, buffer, offset, stride);
}

void glVertexAttribFormat_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint attribindex = (GLuint)*(i32*)&_params[0];
	GLint size = (GLint)*(i32*)&_params[1];
	GLenum type = (GLenum)*(i32*)&_params[2];
	GLboolean normalized = (GLboolean)*(i32*)&_params[3];
	GLuint relativeoffset = (GLuint)*(i32*)&_params[4];
	glVertexAttribFormat(attribindex, size, type, normalized, relativeoffset);
}

void glVertexAttribIFormat_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint attribindex = (GLuint)*(i32*)&_params[0];
	GLint size = (GLint)*(i32*)&_params[1];
	GLenum type = (GLenum)*(i32*)&_params[2];
	GLuint relativeoffset = (GLuint)*(i32*)&_params[3];
	glVertexAttribIFormat(attribindex, size, type, relativeoffset);
}

void glVertexAttribBinding_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint attribindex = (GLuint)*(i32*)&_params[0];
	GLuint bindingindex = (GLuint)*(i32*)&_params[1];
	glVertexAttribBinding(attribindex, bindingindex);
}

void glVertexBindingDivisor_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)
{
	GLuint bindingindex = (GLuint)*(i32*)&_params[0];
	GLuint divisor = (GLuint)*(i32*)&_params[1];
	glVertexBindingDivisor(bindingindex, divisor);
}

int bindgen_link_gles_api(oc_wasm* wasm)
{
	oc_wasm_status status;
	int ret = 0;

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glActiveTexture");
		binding.proc = glActiveTexture_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glActiveTexture (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glAttachShader");
		binding.proc = glAttachShader_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glAttachShader (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glBindAttribLocation");
		binding.proc = glBindAttribLocation_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glBindAttribLocation (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glBindBuffer");
		binding.proc = glBindBuffer_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glBindBuffer (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glBindFramebuffer");
		binding.proc = glBindFramebuffer_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glBindFramebuffer (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glBindRenderbuffer");
		binding.proc = glBindRenderbuffer_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glBindRenderbuffer (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glBindTexture");
		binding.proc = glBindTexture_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glBindTexture (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_F32, OC_WASM_VALTYPE_F32, OC_WASM_VALTYPE_F32, OC_WASM_VALTYPE_F32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glBlendColor");
		binding.proc = glBlendColor_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glBlendColor (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glBlendEquation");
		binding.proc = glBlendEquation_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glBlendEquation (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glBlendEquationSeparate");
		binding.proc = glBlendEquationSeparate_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glBlendEquationSeparate (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glBlendFunc");
		binding.proc = glBlendFunc_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glBlendFunc (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glBlendFuncSeparate");
		binding.proc = glBlendFuncSeparate_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glBlendFuncSeparate (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glBufferData");
		binding.proc = glBufferData_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glBufferData (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glBufferSubData");
		binding.proc = glBufferSubData_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glBufferSubData (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_I32 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glCheckFramebufferStatus");
		binding.proc = glCheckFramebufferStatus_stub;
		binding.countParams = 1;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glCheckFramebufferStatus (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glClear");
		binding.proc = glClear_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glClear (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_F32, OC_WASM_VALTYPE_F32, OC_WASM_VALTYPE_F32, OC_WASM_VALTYPE_F32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glClearColor");
		binding.proc = glClearColor_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glClearColor (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_F32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glClearDepthf");
		binding.proc = glClearDepthf_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glClearDepthf (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glClearStencil");
		binding.proc = glClearStencil_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glClearStencil (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glColorMask");
		binding.proc = glColorMask_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glColorMask (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glCompileShader");
		binding.proc = glCompileShader_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glCompileShader (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glCompressedTexImage2D");
		binding.proc = glCompressedTexImage2D_stub;
		binding.countParams = 8;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glCompressedTexImage2D (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glCompressedTexSubImage2D");
		binding.proc = glCompressedTexSubImage2D_stub;
		binding.countParams = 9;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glCompressedTexSubImage2D (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glCopyTexImage2D");
		binding.proc = glCopyTexImage2D_stub;
		binding.countParams = 8;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glCopyTexImage2D (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glCopyTexSubImage2D");
		binding.proc = glCopyTexSubImage2D_stub;
		binding.countParams = 8;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glCopyTexSubImage2D (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[1];
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_I32 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glCreateProgram");
		binding.proc = glCreateProgram_stub;
		binding.countParams = 0;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glCreateProgram (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_I32 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glCreateShader");
		binding.proc = glCreateShader_stub;
		binding.countParams = 1;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glCreateShader (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glCullFace");
		binding.proc = glCullFace_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glCullFace (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glDeleteBuffers");
		binding.proc = glDeleteBuffers_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glDeleteBuffers (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glDeleteFramebuffers");
		binding.proc = glDeleteFramebuffers_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glDeleteFramebuffers (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glDeleteProgram");
		binding.proc = glDeleteProgram_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glDeleteProgram (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glDeleteRenderbuffers");
		binding.proc = glDeleteRenderbuffers_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glDeleteRenderbuffers (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glDeleteShader");
		binding.proc = glDeleteShader_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glDeleteShader (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glDeleteTextures");
		binding.proc = glDeleteTextures_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glDeleteTextures (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glDepthFunc");
		binding.proc = glDepthFunc_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glDepthFunc (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glDepthMask");
		binding.proc = glDepthMask_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glDepthMask (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_F32, OC_WASM_VALTYPE_F32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glDepthRangef");
		binding.proc = glDepthRangef_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glDepthRangef (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glDetachShader");
		binding.proc = glDetachShader_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glDetachShader (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glDisable");
		binding.proc = glDisable_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glDisable (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glDisableVertexAttribArray");
		binding.proc = glDisableVertexAttribArray_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glDisableVertexAttribArray (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glDrawArrays");
		binding.proc = glDrawArrays_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glDrawArrays (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glDrawElements");
		binding.proc = glDrawElements_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glDrawElements (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glEnable");
		binding.proc = glEnable_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glEnable (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glEnableVertexAttribArray");
		binding.proc = glEnableVertexAttribArray_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glEnableVertexAttribArray (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[1];
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glFinish");
		binding.proc = glFinish_stub;
		binding.countParams = 0;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glFinish (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[1];
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glFlush");
		binding.proc = glFlush_stub;
		binding.countParams = 0;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glFlush (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glFramebufferRenderbuffer");
		binding.proc = glFramebufferRenderbuffer_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glFramebufferRenderbuffer (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glFramebufferTexture2D");
		binding.proc = glFramebufferTexture2D_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glFramebufferTexture2D (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glFrontFace");
		binding.proc = glFrontFace_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glFrontFace (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGenBuffers");
		binding.proc = glGenBuffers_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGenBuffers (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGenerateMipmap");
		binding.proc = glGenerateMipmap_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGenerateMipmap (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGenFramebuffers");
		binding.proc = glGenFramebuffers_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGenFramebuffers (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGenRenderbuffers");
		binding.proc = glGenRenderbuffers_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGenRenderbuffers (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGenTextures");
		binding.proc = glGenTextures_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGenTextures (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetActiveAttrib");
		binding.proc = glGetActiveAttrib_stub;
		binding.countParams = 7;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetActiveAttrib (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetActiveUniform");
		binding.proc = glGetActiveUniform_stub;
		binding.countParams = 7;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetActiveUniform (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetAttachedShaders");
		binding.proc = glGetAttachedShaders_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetAttachedShaders (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_I32 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetAttribLocation");
		binding.proc = glGetAttribLocation_stub;
		binding.countParams = 2;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetAttribLocation (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetBooleanv");
		binding.proc = glGetBooleanv_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetBooleanv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetBufferParameteriv");
		binding.proc = glGetBufferParameteriv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetBufferParameteriv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[1];
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_I32 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetError");
		binding.proc = glGetError_stub;
		binding.countParams = 0;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetError (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetFloatv");
		binding.proc = glGetFloatv_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetFloatv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetFramebufferAttachmentParameteriv");
		binding.proc = glGetFramebufferAttachmentParameteriv_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetFramebufferAttachmentParameteriv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetIntegerv");
		binding.proc = glGetIntegerv_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetIntegerv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetProgramiv");
		binding.proc = glGetProgramiv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetProgramiv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetProgramInfoLog");
		binding.proc = glGetProgramInfoLog_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetProgramInfoLog (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetRenderbufferParameteriv");
		binding.proc = glGetRenderbufferParameteriv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetRenderbufferParameteriv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetShaderiv");
		binding.proc = glGetShaderiv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetShaderiv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetShaderInfoLog");
		binding.proc = glGetShaderInfoLog_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetShaderInfoLog (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetShaderPrecisionFormat");
		binding.proc = glGetShaderPrecisionFormat_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetShaderPrecisionFormat (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetShaderSource");
		binding.proc = glGetShaderSource_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetShaderSource (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetTexParameterfv");
		binding.proc = glGetTexParameterfv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetTexParameterfv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetTexParameteriv");
		binding.proc = glGetTexParameteriv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetTexParameteriv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetUniformfv");
		binding.proc = glGetUniformfv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetUniformfv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetUniformiv");
		binding.proc = glGetUniformiv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetUniformiv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_I32 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetUniformLocation");
		binding.proc = glGetUniformLocation_stub;
		binding.countParams = 2;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetUniformLocation (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetVertexAttribfv");
		binding.proc = glGetVertexAttribfv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetVertexAttribfv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetVertexAttribiv");
		binding.proc = glGetVertexAttribiv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetVertexAttribiv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glHint");
		binding.proc = glHint_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glHint (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_I32 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glIsBuffer");
		binding.proc = glIsBuffer_stub;
		binding.countParams = 1;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glIsBuffer (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_I32 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glIsEnabled");
		binding.proc = glIsEnabled_stub;
		binding.countParams = 1;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glIsEnabled (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_I32 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glIsFramebuffer");
		binding.proc = glIsFramebuffer_stub;
		binding.countParams = 1;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glIsFramebuffer (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_I32 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glIsProgram");
		binding.proc = glIsProgram_stub;
		binding.countParams = 1;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glIsProgram (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_I32 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glIsRenderbuffer");
		binding.proc = glIsRenderbuffer_stub;
		binding.countParams = 1;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glIsRenderbuffer (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_I32 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glIsShader");
		binding.proc = glIsShader_stub;
		binding.countParams = 1;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glIsShader (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_I32 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glIsTexture");
		binding.proc = glIsTexture_stub;
		binding.countParams = 1;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glIsTexture (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_F32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glLineWidth");
		binding.proc = glLineWidth_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glLineWidth (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glLinkProgram");
		binding.proc = glLinkProgram_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glLinkProgram (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glPixelStorei");
		binding.proc = glPixelStorei_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glPixelStorei (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_F32, OC_WASM_VALTYPE_F32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glPolygonOffset");
		binding.proc = glPolygonOffset_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glPolygonOffset (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glReadPixels");
		binding.proc = glReadPixels_stub;
		binding.countParams = 7;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glReadPixels (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[1];
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glReleaseShaderCompiler");
		binding.proc = glReleaseShaderCompiler_stub;
		binding.countParams = 0;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glReleaseShaderCompiler (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glRenderbufferStorage");
		binding.proc = glRenderbufferStorage_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glRenderbufferStorage (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_F32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glSampleCoverage");
		binding.proc = glSampleCoverage_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glSampleCoverage (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glScissor");
		binding.proc = glScissor_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glScissor (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glShaderBinary");
		binding.proc = glShaderBinary_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glShaderBinary (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glStencilFunc");
		binding.proc = glStencilFunc_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glStencilFunc (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glStencilFuncSeparate");
		binding.proc = glStencilFuncSeparate_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glStencilFuncSeparate (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glStencilMask");
		binding.proc = glStencilMask_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glStencilMask (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glStencilMaskSeparate");
		binding.proc = glStencilMaskSeparate_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glStencilMaskSeparate (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glStencilOp");
		binding.proc = glStencilOp_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glStencilOp (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glStencilOpSeparate");
		binding.proc = glStencilOpSeparate_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glStencilOpSeparate (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glTexImage2D");
		binding.proc = glTexImage2D_stub;
		binding.countParams = 9;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glTexImage2D (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_F32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glTexParameterf");
		binding.proc = glTexParameterf_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glTexParameterf (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glTexParameterfv");
		binding.proc = glTexParameterfv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glTexParameterfv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glTexParameteri");
		binding.proc = glTexParameteri_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glTexParameteri (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glTexParameteriv");
		binding.proc = glTexParameteriv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glTexParameteriv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glTexSubImage2D");
		binding.proc = glTexSubImage2D_stub;
		binding.countParams = 9;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glTexSubImage2D (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_F32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniform1f");
		binding.proc = glUniform1f_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniform1f (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniform1fv");
		binding.proc = glUniform1fv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniform1fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniform1i");
		binding.proc = glUniform1i_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniform1i (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniform1iv");
		binding.proc = glUniform1iv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniform1iv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_F32, OC_WASM_VALTYPE_F32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniform2f");
		binding.proc = glUniform2f_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniform2f (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniform2fv");
		binding.proc = glUniform2fv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniform2fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniform2i");
		binding.proc = glUniform2i_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniform2i (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniform2iv");
		binding.proc = glUniform2iv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniform2iv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_F32, OC_WASM_VALTYPE_F32, OC_WASM_VALTYPE_F32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniform3f");
		binding.proc = glUniform3f_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniform3f (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniform3fv");
		binding.proc = glUniform3fv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniform3fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniform3i");
		binding.proc = glUniform3i_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniform3i (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniform3iv");
		binding.proc = glUniform3iv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniform3iv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_F32, OC_WASM_VALTYPE_F32, OC_WASM_VALTYPE_F32, OC_WASM_VALTYPE_F32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniform4f");
		binding.proc = glUniform4f_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniform4f (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniform4fv");
		binding.proc = glUniform4fv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniform4fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniform4i");
		binding.proc = glUniform4i_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniform4i (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniform4iv");
		binding.proc = glUniform4iv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniform4iv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniformMatrix2fv");
		binding.proc = glUniformMatrix2fv_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniformMatrix2fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniformMatrix3fv");
		binding.proc = glUniformMatrix3fv_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniformMatrix3fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniformMatrix4fv");
		binding.proc = glUniformMatrix4fv_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniformMatrix4fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUseProgram");
		binding.proc = glUseProgram_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUseProgram (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glValidateProgram");
		binding.proc = glValidateProgram_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glValidateProgram (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_F32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glVertexAttrib1f");
		binding.proc = glVertexAttrib1f_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glVertexAttrib1f (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glVertexAttrib1fv");
		binding.proc = glVertexAttrib1fv_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glVertexAttrib1fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_F32, OC_WASM_VALTYPE_F32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glVertexAttrib2f");
		binding.proc = glVertexAttrib2f_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glVertexAttrib2f (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glVertexAttrib2fv");
		binding.proc = glVertexAttrib2fv_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glVertexAttrib2fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_F32, OC_WASM_VALTYPE_F32, OC_WASM_VALTYPE_F32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glVertexAttrib3f");
		binding.proc = glVertexAttrib3f_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glVertexAttrib3f (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glVertexAttrib3fv");
		binding.proc = glVertexAttrib3fv_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glVertexAttrib3fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_F32, OC_WASM_VALTYPE_F32, OC_WASM_VALTYPE_F32, OC_WASM_VALTYPE_F32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glVertexAttrib4f");
		binding.proc = glVertexAttrib4f_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glVertexAttrib4f (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glVertexAttrib4fv");
		binding.proc = glVertexAttrib4fv_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glVertexAttrib4fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glViewport");
		binding.proc = glViewport_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glViewport (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glReadBuffer");
		binding.proc = glReadBuffer_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glReadBuffer (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glDrawRangeElements");
		binding.proc = glDrawRangeElements_stub;
		binding.countParams = 6;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glDrawRangeElements (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glTexImage3D");
		binding.proc = glTexImage3D_stub;
		binding.countParams = 10;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glTexImage3D (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glTexSubImage3D");
		binding.proc = glTexSubImage3D_stub;
		binding.countParams = 11;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glTexSubImage3D (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glCopyTexSubImage3D");
		binding.proc = glCopyTexSubImage3D_stub;
		binding.countParams = 9;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glCopyTexSubImage3D (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glCompressedTexImage3D");
		binding.proc = glCompressedTexImage3D_stub;
		binding.countParams = 9;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glCompressedTexImage3D (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glCompressedTexSubImage3D");
		binding.proc = glCompressedTexSubImage3D_stub;
		binding.countParams = 11;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glCompressedTexSubImage3D (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGenQueries");
		binding.proc = glGenQueries_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGenQueries (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glDeleteQueries");
		binding.proc = glDeleteQueries_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glDeleteQueries (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_I32 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glIsQuery");
		binding.proc = glIsQuery_stub;
		binding.countParams = 1;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glIsQuery (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glBeginQuery");
		binding.proc = glBeginQuery_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glBeginQuery (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glEndQuery");
		binding.proc = glEndQuery_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glEndQuery (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetQueryiv");
		binding.proc = glGetQueryiv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetQueryiv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetQueryObjectuiv");
		binding.proc = glGetQueryObjectuiv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetQueryObjectuiv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glDrawBuffers");
		binding.proc = glDrawBuffers_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glDrawBuffers (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniformMatrix2x3fv");
		binding.proc = glUniformMatrix2x3fv_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniformMatrix2x3fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniformMatrix3x2fv");
		binding.proc = glUniformMatrix3x2fv_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniformMatrix3x2fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniformMatrix2x4fv");
		binding.proc = glUniformMatrix2x4fv_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniformMatrix2x4fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniformMatrix4x2fv");
		binding.proc = glUniformMatrix4x2fv_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniformMatrix4x2fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniformMatrix3x4fv");
		binding.proc = glUniformMatrix3x4fv_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniformMatrix3x4fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniformMatrix4x3fv");
		binding.proc = glUniformMatrix4x3fv_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniformMatrix4x3fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glBlitFramebuffer");
		binding.proc = glBlitFramebuffer_stub;
		binding.countParams = 10;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glBlitFramebuffer (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glRenderbufferStorageMultisample");
		binding.proc = glRenderbufferStorageMultisample_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glRenderbufferStorageMultisample (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glFramebufferTextureLayer");
		binding.proc = glFramebufferTextureLayer_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glFramebufferTextureLayer (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glBindVertexArray");
		binding.proc = glBindVertexArray_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glBindVertexArray (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glDeleteVertexArrays");
		binding.proc = glDeleteVertexArrays_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glDeleteVertexArrays (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGenVertexArrays");
		binding.proc = glGenVertexArrays_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGenVertexArrays (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_I32 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glIsVertexArray");
		binding.proc = glIsVertexArray_stub;
		binding.countParams = 1;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glIsVertexArray (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetIntegeri_v");
		binding.proc = glGetIntegeri_v_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetIntegeri_v (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glBeginTransformFeedback");
		binding.proc = glBeginTransformFeedback_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glBeginTransformFeedback (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[1];
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glEndTransformFeedback");
		binding.proc = glEndTransformFeedback_stub;
		binding.countParams = 0;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glEndTransformFeedback (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glBindBufferRange");
		binding.proc = glBindBufferRange_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glBindBufferRange (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glBindBufferBase");
		binding.proc = glBindBufferBase_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glBindBufferBase (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glTransformFeedbackVaryings");
		binding.proc = glTransformFeedbackVaryings_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glTransformFeedbackVaryings (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetTransformFeedbackVarying");
		binding.proc = glGetTransformFeedbackVarying_stub;
		binding.countParams = 7;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetTransformFeedbackVarying (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetVertexAttribIiv");
		binding.proc = glGetVertexAttribIiv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetVertexAttribIiv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetVertexAttribIuiv");
		binding.proc = glGetVertexAttribIuiv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetVertexAttribIuiv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glVertexAttribI4i");
		binding.proc = glVertexAttribI4i_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glVertexAttribI4i (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glVertexAttribI4ui");
		binding.proc = glVertexAttribI4ui_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glVertexAttribI4ui (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glVertexAttribI4iv");
		binding.proc = glVertexAttribI4iv_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glVertexAttribI4iv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glVertexAttribI4uiv");
		binding.proc = glVertexAttribI4uiv_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glVertexAttribI4uiv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetUniformuiv");
		binding.proc = glGetUniformuiv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetUniformuiv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_I32 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetFragDataLocation");
		binding.proc = glGetFragDataLocation_stub;
		binding.countParams = 2;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetFragDataLocation (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniform1ui");
		binding.proc = glUniform1ui_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniform1ui (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniform2ui");
		binding.proc = glUniform2ui_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniform2ui (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniform3ui");
		binding.proc = glUniform3ui_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniform3ui (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniform4ui");
		binding.proc = glUniform4ui_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniform4ui (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniform1uiv");
		binding.proc = glUniform1uiv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniform1uiv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniform2uiv");
		binding.proc = glUniform2uiv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniform2uiv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniform3uiv");
		binding.proc = glUniform3uiv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniform3uiv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniform4uiv");
		binding.proc = glUniform4uiv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniform4uiv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glClearBufferiv");
		binding.proc = glClearBufferiv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glClearBufferiv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glClearBufferuiv");
		binding.proc = glClearBufferuiv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glClearBufferuiv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glClearBufferfv");
		binding.proc = glClearBufferfv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glClearBufferfv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_F32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glClearBufferfi");
		binding.proc = glClearBufferfi_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glClearBufferfi (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glCopyBufferSubData");
		binding.proc = glCopyBufferSubData_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glCopyBufferSubData (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetActiveUniformsiv");
		binding.proc = glGetActiveUniformsiv_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetActiveUniformsiv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_I32 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetUniformBlockIndex");
		binding.proc = glGetUniformBlockIndex_stub;
		binding.countParams = 2;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetUniformBlockIndex (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetActiveUniformBlockiv");
		binding.proc = glGetActiveUniformBlockiv_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetActiveUniformBlockiv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetActiveUniformBlockName");
		binding.proc = glGetActiveUniformBlockName_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetActiveUniformBlockName (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUniformBlockBinding");
		binding.proc = glUniformBlockBinding_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUniformBlockBinding (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glDrawArraysInstanced");
		binding.proc = glDrawArraysInstanced_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glDrawArraysInstanced (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glDrawElementsInstanced");
		binding.proc = glDrawElementsInstanced_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glDrawElementsInstanced (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_I64 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glFenceSync");
		binding.proc = glFenceSync_stub;
		binding.countParams = 2;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glFenceSync (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I64, };
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_I32 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glIsSync");
		binding.proc = glIsSync_stub;
		binding.countParams = 1;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glIsSync (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I64, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glDeleteSync");
		binding.proc = glDeleteSync_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glDeleteSync (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I64, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I64, };
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_I32 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glClientWaitSync");
		binding.proc = glClientWaitSync_stub;
		binding.countParams = 3;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glClientWaitSync (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I64, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I64, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glWaitSync");
		binding.proc = glWaitSync_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glWaitSync (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetInteger64v");
		binding.proc = glGetInteger64v_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetInteger64v (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I64, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetSynciv");
		binding.proc = glGetSynciv_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetSynciv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetInteger64i_v");
		binding.proc = glGetInteger64i_v_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetInteger64i_v (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetBufferParameteri64v");
		binding.proc = glGetBufferParameteri64v_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetBufferParameteri64v (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGenSamplers");
		binding.proc = glGenSamplers_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGenSamplers (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glDeleteSamplers");
		binding.proc = glDeleteSamplers_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glDeleteSamplers (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_I32 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glIsSampler");
		binding.proc = glIsSampler_stub;
		binding.countParams = 1;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glIsSampler (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glBindSampler");
		binding.proc = glBindSampler_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glBindSampler (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glSamplerParameteri");
		binding.proc = glSamplerParameteri_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glSamplerParameteri (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glSamplerParameteriv");
		binding.proc = glSamplerParameteriv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glSamplerParameteriv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_F32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glSamplerParameterf");
		binding.proc = glSamplerParameterf_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glSamplerParameterf (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glSamplerParameterfv");
		binding.proc = glSamplerParameterfv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glSamplerParameterfv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetSamplerParameteriv");
		binding.proc = glGetSamplerParameteriv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetSamplerParameteriv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetSamplerParameterfv");
		binding.proc = glGetSamplerParameterfv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetSamplerParameterfv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glVertexAttribDivisor");
		binding.proc = glVertexAttribDivisor_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glVertexAttribDivisor (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glBindTransformFeedback");
		binding.proc = glBindTransformFeedback_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glBindTransformFeedback (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glDeleteTransformFeedbacks");
		binding.proc = glDeleteTransformFeedbacks_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glDeleteTransformFeedbacks (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGenTransformFeedbacks");
		binding.proc = glGenTransformFeedbacks_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGenTransformFeedbacks (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_I32 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glIsTransformFeedback");
		binding.proc = glIsTransformFeedback_stub;
		binding.countParams = 1;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glIsTransformFeedback (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[1];
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glPauseTransformFeedback");
		binding.proc = glPauseTransformFeedback_stub;
		binding.countParams = 0;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glPauseTransformFeedback (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[1];
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glResumeTransformFeedback");
		binding.proc = glResumeTransformFeedback_stub;
		binding.countParams = 0;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glResumeTransformFeedback (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetProgramBinary");
		binding.proc = glGetProgramBinary_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetProgramBinary (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramBinary");
		binding.proc = glProgramBinary_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramBinary (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramParameteri");
		binding.proc = glProgramParameteri_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramParameteri (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glInvalidateFramebuffer");
		binding.proc = glInvalidateFramebuffer_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glInvalidateFramebuffer (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glInvalidateSubFramebuffer");
		binding.proc = glInvalidateSubFramebuffer_stub;
		binding.countParams = 7;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glInvalidateSubFramebuffer (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glTexStorage2D");
		binding.proc = glTexStorage2D_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glTexStorage2D (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glTexStorage3D");
		binding.proc = glTexStorage3D_stub;
		binding.countParams = 6;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glTexStorage3D (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetInternalformativ");
		binding.proc = glGetInternalformativ_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetInternalformativ (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glDispatchCompute");
		binding.proc = glDispatchCompute_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glDispatchCompute (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glDispatchComputeIndirect");
		binding.proc = glDispatchComputeIndirect_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glDispatchComputeIndirect (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glDrawArraysIndirect");
		binding.proc = glDrawArraysIndirect_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glDrawArraysIndirect (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glDrawElementsIndirect");
		binding.proc = glDrawElementsIndirect_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glDrawElementsIndirect (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glFramebufferParameteri");
		binding.proc = glFramebufferParameteri_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glFramebufferParameteri (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetFramebufferParameteriv");
		binding.proc = glGetFramebufferParameteriv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetFramebufferParameteriv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetProgramInterfaceiv");
		binding.proc = glGetProgramInterfaceiv_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetProgramInterfaceiv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_I32 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetProgramResourceIndex");
		binding.proc = glGetProgramResourceIndex_stub;
		binding.countParams = 3;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetProgramResourceIndex (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetProgramResourceName");
		binding.proc = glGetProgramResourceName_stub;
		binding.countParams = 6;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetProgramResourceName (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetProgramResourceiv");
		binding.proc = glGetProgramResourceiv_stub;
		binding.countParams = 8;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetProgramResourceiv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_I32 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetProgramResourceLocation");
		binding.proc = glGetProgramResourceLocation_stub;
		binding.countParams = 3;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetProgramResourceLocation (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glUseProgramStages");
		binding.proc = glUseProgramStages_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glUseProgramStages (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glActiveShaderProgram");
		binding.proc = glActiveShaderProgram_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glActiveShaderProgram (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_I32 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glCreateShaderProgramv");
		binding.proc = glCreateShaderProgramv_stub;
		binding.countParams = 3;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glCreateShaderProgramv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glBindProgramPipeline");
		binding.proc = glBindProgramPipeline_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glBindProgramPipeline (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glDeleteProgramPipelines");
		binding.proc = glDeleteProgramPipelines_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glDeleteProgramPipelines (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGenProgramPipelines");
		binding.proc = glGenProgramPipelines_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGenProgramPipelines (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[] = { OC_WASM_VALTYPE_I32 };

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glIsProgramPipeline");
		binding.proc = glIsProgramPipeline_stub;
		binding.countParams = 1;
		binding.countReturns = 1;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glIsProgramPipeline (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetProgramPipelineiv");
		binding.proc = glGetProgramPipelineiv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetProgramPipelineiv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniform1i");
		binding.proc = glProgramUniform1i_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniform1i (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniform2i");
		binding.proc = glProgramUniform2i_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniform2i (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniform3i");
		binding.proc = glProgramUniform3i_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniform3i (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniform4i");
		binding.proc = glProgramUniform4i_stub;
		binding.countParams = 6;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniform4i (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniform1ui");
		binding.proc = glProgramUniform1ui_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniform1ui (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniform2ui");
		binding.proc = glProgramUniform2ui_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniform2ui (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniform3ui");
		binding.proc = glProgramUniform3ui_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniform3ui (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniform4ui");
		binding.proc = glProgramUniform4ui_stub;
		binding.countParams = 6;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniform4ui (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_F32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniform1f");
		binding.proc = glProgramUniform1f_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniform1f (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_F32, OC_WASM_VALTYPE_F32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniform2f");
		binding.proc = glProgramUniform2f_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniform2f (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_F32, OC_WASM_VALTYPE_F32, OC_WASM_VALTYPE_F32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniform3f");
		binding.proc = glProgramUniform3f_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniform3f (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_F32, OC_WASM_VALTYPE_F32, OC_WASM_VALTYPE_F32, OC_WASM_VALTYPE_F32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniform4f");
		binding.proc = glProgramUniform4f_stub;
		binding.countParams = 6;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniform4f (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniform1iv");
		binding.proc = glProgramUniform1iv_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniform1iv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniform2iv");
		binding.proc = glProgramUniform2iv_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniform2iv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniform3iv");
		binding.proc = glProgramUniform3iv_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniform3iv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniform4iv");
		binding.proc = glProgramUniform4iv_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniform4iv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniform1uiv");
		binding.proc = glProgramUniform1uiv_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniform1uiv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniform2uiv");
		binding.proc = glProgramUniform2uiv_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniform2uiv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniform3uiv");
		binding.proc = glProgramUniform3uiv_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniform3uiv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniform4uiv");
		binding.proc = glProgramUniform4uiv_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniform4uiv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniform1fv");
		binding.proc = glProgramUniform1fv_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniform1fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniform2fv");
		binding.proc = glProgramUniform2fv_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniform2fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniform3fv");
		binding.proc = glProgramUniform3fv_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniform3fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniform4fv");
		binding.proc = glProgramUniform4fv_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniform4fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniformMatrix2fv");
		binding.proc = glProgramUniformMatrix2fv_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniformMatrix2fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniformMatrix3fv");
		binding.proc = glProgramUniformMatrix3fv_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniformMatrix3fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniformMatrix4fv");
		binding.proc = glProgramUniformMatrix4fv_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniformMatrix4fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniformMatrix2x3fv");
		binding.proc = glProgramUniformMatrix2x3fv_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniformMatrix2x3fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniformMatrix3x2fv");
		binding.proc = glProgramUniformMatrix3x2fv_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniformMatrix3x2fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniformMatrix2x4fv");
		binding.proc = glProgramUniformMatrix2x4fv_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniformMatrix2x4fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniformMatrix4x2fv");
		binding.proc = glProgramUniformMatrix4x2fv_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniformMatrix4x2fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniformMatrix3x4fv");
		binding.proc = glProgramUniformMatrix3x4fv_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniformMatrix3x4fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glProgramUniformMatrix4x3fv");
		binding.proc = glProgramUniformMatrix4x3fv_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glProgramUniformMatrix4x3fv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glValidateProgramPipeline");
		binding.proc = glValidateProgramPipeline_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glValidateProgramPipeline (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetProgramPipelineInfoLog");
		binding.proc = glGetProgramPipelineInfoLog_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetProgramPipelineInfoLog (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glBindImageTexture");
		binding.proc = glBindImageTexture_stub;
		binding.countParams = 7;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glBindImageTexture (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetBooleani_v");
		binding.proc = glGetBooleani_v_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetBooleani_v (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glMemoryBarrier");
		binding.proc = glMemoryBarrier_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glMemoryBarrier (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glMemoryBarrierByRegion");
		binding.proc = glMemoryBarrierByRegion_stub;
		binding.countParams = 1;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glMemoryBarrierByRegion (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glTexStorage2DMultisample");
		binding.proc = glTexStorage2DMultisample_stub;
		binding.countParams = 6;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glTexStorage2DMultisample (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetMultisamplefv");
		binding.proc = glGetMultisamplefv_stub;
		binding.countParams = 3;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetMultisamplefv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glSampleMaski");
		binding.proc = glSampleMaski_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glSampleMaski (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetTexLevelParameteriv");
		binding.proc = glGetTexLevelParameteriv_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetTexLevelParameteriv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glGetTexLevelParameterfv");
		binding.proc = glGetTexLevelParameterfv_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glGetTexLevelParameterfv (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glBindVertexBuffer");
		binding.proc = glBindVertexBuffer_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glBindVertexBuffer (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glVertexAttribFormat");
		binding.proc = glVertexAttribFormat_stub;
		binding.countParams = 5;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glVertexAttribFormat (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glVertexAttribIFormat");
		binding.proc = glVertexAttribIFormat_stub;
		binding.countParams = 4;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glVertexAttribIFormat (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glVertexAttribBinding");
		binding.proc = glVertexAttribBinding_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glVertexAttribBinding (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	{
		oc_wasm_valtype paramTypes[] = {OC_WASM_VALTYPE_I32, OC_WASM_VALTYPE_I32, };
		oc_wasm_valtype returnTypes[1];

		oc_wasm_binding binding = {0};
		binding.importName = OC_STR8("glVertexBindingDivisor");
		binding.proc = glVertexBindingDivisor_stub;
		binding.countParams = 2;
		binding.countReturns = 0;
		binding.params = paramTypes;
		binding.returns = returnTypes;
		status = oc_wasm_add_binding(wasm, &binding);
		if(oc_wasm_status_is_fail(status))
		{
			oc_log_error("Couldn't link function glVertexBindingDivisor (%s)\n", oc_wasm_status_str8(status).ptr);
			ret = -1;
		}
	}

	return(ret);
}
