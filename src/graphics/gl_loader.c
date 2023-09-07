/********************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
*********************************************************/
#include "gl_loader.h"
#include "platform/platform.h"

oc_thread_local oc_gl_api* oc_glAPI = 0;
oc_gl_api oc_glNoAPI;

void oc_glGetFloatv_noimpl(GLenum pname, GLfloat* data)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetFloatv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glTexBufferRange_noimpl(GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glTexBufferRange is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLboolean oc_glIsBuffer_noimpl(GLuint buffer)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glIsBuffer is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLboolean)0);
}

GLboolean oc_glIsTexture_noimpl(GLuint texture)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glIsTexture is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLboolean)0);
}

void oc_glDepthRangef_noimpl(GLfloat n, GLfloat f)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDepthRangef is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glEndConditionalRender_noimpl(void)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glEndConditionalRender is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBlendFunci_noimpl(GLuint buf, GLenum src, GLenum dst)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBlendFunci is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetProgramPipelineiv_noimpl(GLuint pipeline, GLenum pname, GLint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetProgramPipelineiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glWaitSync_noimpl(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glWaitSync is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniformMatrix2fv_noimpl(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniformMatrix2fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniformMatrix4x3dv_noimpl(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniformMatrix4x3dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib1dv_noimpl(GLuint index, const GLdouble* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib1dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glSamplerParameteri_noimpl(GLuint sampler, GLenum pname, GLint param)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glSamplerParameteri is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetVertexAttribIiv_noimpl(GLuint index, GLenum pname, GLint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetVertexAttribIiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetSamplerParameterfv_noimpl(GLuint sampler, GLenum pname, GLfloat* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetSamplerParameterfv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib1d_noimpl(GLuint index, GLdouble x)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib1d is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glTexBuffer_noimpl(GLenum target, GLenum internalformat, GLuint buffer)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glTexBuffer is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glInvalidateBufferData_noimpl(GLuint buffer)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glInvalidateBufferData is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform2i_noimpl(GLuint program, GLint location, GLint v0, GLint v1)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform2i is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniform4dv_noimpl(GLint location, GLsizei count, const GLdouble* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform4dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUseProgram_noimpl(GLuint program)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUseProgram is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribI3iv_noimpl(GLuint index, const GLint* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribI3iv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDrawElementsIndirect_noimpl(GLenum mode, GLenum type, const void* indirect)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDrawElementsIndirect is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib4uiv_noimpl(GLuint index, const GLuint* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib4uiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetQueryObjectiv_noimpl(GLuint id, GLenum pname, GLint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetQueryObjectiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glFramebufferRenderbuffer_noimpl(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glFramebufferRenderbuffer is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBlendEquationi_noimpl(GLuint buf, GLenum mode)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBlendEquationi is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetActiveSubroutineName_noimpl(GLuint program, GLenum shadertype, GLuint index, GLsizei bufSize, GLsizei* length, GLchar* name)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetActiveSubroutineName is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib2s_noimpl(GLuint index, GLshort x, GLshort y)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib2s is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribL1d_noimpl(GLuint index, GLdouble x)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribL1d is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBindTextures_noimpl(GLuint first, GLsizei count, const GLuint* textures)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBindTextures is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib3sv_noimpl(GLuint index, const GLshort* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib3sv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetFloati_v_noimpl(GLenum target, GLuint index, GLfloat* data)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetFloati_v is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBeginTransformFeedback_noimpl(GLenum primitiveMode)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBeginTransformFeedback is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glClearStencil_noimpl(GLint s)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glClearStencil is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniform3i_noimpl(GLint location, GLint v0, GLint v1, GLint v2)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform3i is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glValidateProgramPipeline_noimpl(GLuint pipeline)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glValidateProgramPipeline is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniformMatrix4x2fv_noimpl(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniformMatrix4x2fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribI4ui_noimpl(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribI4ui is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetShaderiv_noimpl(GLuint shader, GLenum pname, GLint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetShaderiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glReadnPixels_noimpl(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void* data)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glReadnPixels is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniformMatrix4x2fv_noimpl(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniformMatrix4x2fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetShaderPrecisionFormat_noimpl(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetShaderPrecisionFormat is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniformMatrix2x3fv_noimpl(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniformMatrix2x3fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glTexSubImage3D_noimpl(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glTexSubImage3D is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLint oc_glGetProgramResourceLocationIndex_noimpl(GLuint program, GLenum programInterface, const GLchar* name)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetProgramResourceLocationIndex is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLint)0);
}

void oc_glBlendFunc_noimpl(GLenum sfactor, GLenum dfactor)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBlendFunc is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniformMatrix3x4fv_noimpl(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniformMatrix3x4fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniform3d_noimpl(GLint location, GLdouble x, GLdouble y, GLdouble z)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform3d is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib1sv_noimpl(GLuint index, const GLshort* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib1sv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBindFragDataLocation_noimpl(GLuint program, GLuint color, const GLchar* name)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBindFragDataLocation is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib4bv_noimpl(GLuint index, const GLbyte* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib4bv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniform4iv_noimpl(GLint location, GLsizei count, const GLint* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform4iv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform2ui_noimpl(GLuint program, GLint location, GLuint v0, GLuint v1)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform2ui is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDrawArrays_noimpl(GLenum mode, GLint first, GLsizei count)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDrawArrays is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramBinary_noimpl(GLuint program, GLenum binaryFormat, const void* binary, GLsizei length)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramBinary is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib4f_noimpl(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib4f is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribP2uiv_noimpl(GLuint index, GLenum type, GLboolean normalized, const GLuint* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribP2uiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniformMatrix3fv_noimpl(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniformMatrix3fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniform2i_noimpl(GLint location, GLint v0, GLint v1)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform2i is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetQueryObjectuiv_noimpl(GLuint id, GLenum pname, GLuint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetQueryObjectuiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniformBlockBinding_noimpl(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniformBlockBinding is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glSampleCoverage_noimpl(GLfloat value, GLboolean invert)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glSampleCoverage is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib4Nusv_noimpl(GLuint index, const GLushort* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib4Nusv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniformMatrix2x4dv_noimpl(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniformMatrix2x4dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniform3uiv_noimpl(GLint location, GLsizei count, const GLuint* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform3uiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib1s_noimpl(GLuint index, GLshort x)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib1s is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetVertexAttribPointerv_noimpl(GLuint index, GLenum pname, void** pointer)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetVertexAttribPointerv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBlendBarrier_noimpl(void)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBlendBarrier is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDrawRangeElements_noimpl(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDrawRangeElements is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glTexStorage3D_noimpl(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glTexStorage3D is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetInternalformati64v_noimpl(GLenum target, GLenum internalformat, GLenum pname, GLsizei count, GLint64* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetInternalformati64v is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetQueryObjecti64v_noimpl(GLuint id, GLenum pname, GLint64* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetQueryObjecti64v is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glCompressedTexSubImage1D_noimpl(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* data)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glCompressedTexSubImage1D is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib3dv_noimpl(GLuint index, const GLdouble* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib3dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexBindingDivisor_noimpl(GLuint bindingindex, GLuint divisor)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexBindingDivisor is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUseProgramStages_noimpl(GLuint pipeline, GLbitfield stages, GLuint program)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUseProgramStages is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribBinding_noimpl(GLuint attribindex, GLuint bindingindex)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribBinding is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDebugMessageInsert_noimpl(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDebugMessageInsert is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetTexParameteriv_noimpl(GLenum target, GLenum pname, GLint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetTexParameteriv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glMultiDrawArraysIndirect_noimpl(GLenum mode, const void* indirect, GLsizei drawcount, GLsizei stride)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glMultiDrawArraysIndirect is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetTexParameterfv_noimpl(GLenum target, GLenum pname, GLfloat* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetTexParameterfv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetProgramPipelineInfoLog_noimpl(GLuint pipeline, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetProgramPipelineInfoLog is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glEndQuery_noimpl(GLenum target)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glEndQuery is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLint oc_glGetProgramResourceLocation_noimpl(GLuint program, GLenum programInterface, const GLchar* name)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetProgramResourceLocation is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLint)0);
}

void oc_glCompressedTexImage2D_noimpl(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glCompressedTexImage2D is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribP2ui_noimpl(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribP2ui is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLboolean oc_glIsEnabledi_noimpl(GLenum target, GLuint index)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glIsEnabledi is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLboolean)0);
}

void oc_glGetActiveAtomicCounterBufferiv_noimpl(GLuint program, GLuint bufferIndex, GLenum pname, GLint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetActiveAtomicCounterBufferiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLboolean oc_glIsProgram_noimpl(GLuint program)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glIsProgram is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLboolean)0);
}

void oc_glUniform1dv_noimpl(GLint location, GLsizei count, const GLdouble* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform1dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glTexParameteriv_noimpl(GLenum target, GLenum pname, const GLint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glTexParameteriv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniform2fv_noimpl(GLint location, GLsizei count, const GLfloat* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform2fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glReleaseShaderCompiler_noimpl(void)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glReleaseShaderCompiler is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glCullFace_noimpl(GLenum mode)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glCullFace is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribI4i_noimpl(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribI4i is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLuint oc_glGetProgramResourceIndex_noimpl(GLuint program, GLenum programInterface, const GLchar* name)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetProgramResourceIndex is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLuint)0);
}

void oc_glShaderBinary_noimpl(GLsizei count, const GLuint* shaders, GLenum binaryFormat, const void* binary, GLsizei length)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glShaderBinary is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniformMatrix3x2dv_noimpl(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniformMatrix3x2dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glInvalidateFramebuffer_noimpl(GLenum target, GLsizei numAttachments, const GLenum* attachments)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glInvalidateFramebuffer is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glAttachShader_noimpl(GLuint program, GLuint shader)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glAttachShader is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glFlushMappedBufferRange_noimpl(GLenum target, GLintptr offset, GLsizeiptr length)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glFlushMappedBufferRange is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribP3uiv_noimpl(GLuint index, GLenum type, GLboolean normalized, const GLuint* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribP3uiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetActiveUniformName_noimpl(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformName)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetActiveUniformName is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void* oc_glMapBuffer_noimpl(GLenum target, GLenum access)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glMapBuffer is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((void*)0);
}

void oc_glDrawBuffers_noimpl(GLsizei n, const GLenum* bufs)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDrawBuffers is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetSynciv_noimpl(GLsync sync, GLenum pname, GLsizei count, GLsizei* length, GLint* values)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetSynciv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glCopyTexSubImage2D_noimpl(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glCopyTexSubImage2D is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glObjectLabel_noimpl(GLenum identifier, GLuint name, GLsizei length, const GLchar* label)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glObjectLabel is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBufferSubData_noimpl(GLenum target, GLintptr offset, GLsizeiptr size, const void* data)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBufferSubData is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniform2f_noimpl(GLint location, GLfloat v0, GLfloat v1)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform2f is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDebugMessageCallback_noimpl(GLDEBUGPROC callback, const void* userParam)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDebugMessageCallback is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribL4dv_noimpl(GLuint index, const GLdouble* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribL4dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLboolean oc_glIsProgramPipeline_noimpl(GLuint pipeline)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glIsProgramPipeline is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLboolean)0);
}

void oc_glResumeTransformFeedback_noimpl(void)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glResumeTransformFeedback is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribI4iv_noimpl(GLuint index, const GLint* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribI4iv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetShaderInfoLog_noimpl(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetShaderInfoLog is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetIntegeri_v_noimpl(GLenum target, GLuint index, GLint* data)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetIntegeri_v is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBindVertexBuffer_noimpl(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBindVertexBuffer is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBlendEquation_noimpl(GLenum mode)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBlendEquation is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribL2dv_noimpl(GLuint index, const GLdouble* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribL2dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribI1ui_noimpl(GLuint index, GLuint x)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribI1ui is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib4Nsv_noimpl(GLuint index, const GLshort* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib4Nsv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribL4d_noimpl(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribL4d is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glCopyImageSubData_noimpl(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glCopyImageSubData is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetFramebufferAttachmentParameteriv_noimpl(GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetFramebufferAttachmentParameteriv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribL2d_noimpl(GLuint index, GLdouble x, GLdouble y)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribL2d is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLuint oc_glGetSubroutineIndex_noimpl(GLuint program, GLenum shadertype, const GLchar* name)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetSubroutineIndex is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLuint)0);
}

void oc_glVertexAttribI3uiv_noimpl(GLuint index, const GLuint* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribI3uiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib4iv_noimpl(GLuint index, const GLint* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib4iv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBindVertexBuffers_noimpl(GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizei* strides)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBindVertexBuffers is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniformMatrix2x3dv_noimpl(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniformMatrix2x3dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glPrimitiveBoundingBox_noimpl(GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glPrimitiveBoundingBox is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glScissor_noimpl(GLint x, GLint y, GLsizei width, GLsizei height)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glScissor is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLenum oc_glClientWaitSync_noimpl(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glClientWaitSync is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLenum)0);
}

void oc_glUniform3ui_noimpl(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform3ui is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribP3ui_noimpl(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribP3ui is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glEnable_noimpl(GLenum cap)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glEnable is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glStencilOpSeparate_noimpl(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glStencilOpSeparate is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniformMatrix2x3dv_noimpl(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniformMatrix2x3dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniformMatrix3dv_noimpl(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniformMatrix3dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glTexImage2DMultisample_noimpl(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glTexImage2DMultisample is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib4Nbv_noimpl(GLuint index, const GLbyte* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib4Nbv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetTexImage_noimpl(GLenum target, GLint level, GLenum format, GLenum type, void* pixels)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetTexImage is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib4sv_noimpl(GLuint index, const GLshort* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib4sv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glPixelStorei_noimpl(GLenum pname, GLint param)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glPixelStorei is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDepthMask_noimpl(GLboolean flag)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDepthMask is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glTexStorage2D_noimpl(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glTexStorage2D is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glClear_noimpl(GLbitfield mask)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glClear is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniformMatrix3x4dv_noimpl(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniformMatrix3x4dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDeleteTransformFeedbacks_noimpl(GLsizei n, const GLuint* ids)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDeleteTransformFeedbacks is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void* oc_glMapBufferRange_noimpl(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glMapBufferRange is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((void*)0);
}

void oc_glMemoryBarrier_noimpl(GLbitfield barriers)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glMemoryBarrier is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glViewportIndexedf_noimpl(GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glViewportIndexedf is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib3fv_noimpl(GLuint index, const GLfloat* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib3fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glObjectPtrLabel_noimpl(const void* ptr, GLsizei length, const GLchar* label)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glObjectPtrLabel is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glTexStorage1D_noimpl(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glTexStorage1D is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glCompressedTexImage3D_noimpl(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* data)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glCompressedTexImage3D is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib1fv_noimpl(GLuint index, const GLfloat* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib1fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribPointer_noimpl(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribPointer is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetQueryIndexediv_noimpl(GLenum target, GLuint index, GLenum pname, GLint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetQueryIndexediv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glCompileShader_noimpl(GLuint shader)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glCompileShader is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform1i_noimpl(GLuint program, GLint location, GLint v0)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform1i is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetQueryiv_noimpl(GLenum target, GLenum pname, GLint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetQueryiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribI1iv_noimpl(GLuint index, const GLint* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribI1iv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glCopyTexImage2D_noimpl(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glCopyTexImage2D is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetQueryObjectui64v_noimpl(GLuint id, GLenum pname, GLuint64* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetQueryObjectui64v is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glPointSize_noimpl(GLfloat size)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glPointSize is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDisablei_noimpl(GLenum target, GLuint index)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDisablei is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribL1dv_noimpl(GLuint index, const GLdouble* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribL1dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLuint oc_glCreateShader_noimpl(GLenum type)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glCreateShader is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLuint)0);
}

const GLubyte* oc_glGetString_noimpl(GLenum name)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetString is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((const GLubyte*)0);
}

void oc_glViewportArrayv_noimpl(GLuint first, GLsizei count, const GLfloat* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glViewportArrayv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform3d_noimpl(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform3d is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib4Nubv_noimpl(GLuint index, const GLubyte* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib4Nubv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glTexParameteri_noimpl(GLenum target, GLenum pname, GLint param)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glTexParameteri is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform4fv_noimpl(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform4fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGenerateMipmap_noimpl(GLenum target)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGenerateMipmap is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glCompressedTexSubImage3D_noimpl(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glCompressedTexSubImage3D is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniform3f_noimpl(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform3f is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetUniformIndices_noimpl(GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetUniformIndices is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribLPointer_noimpl(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribLPointer is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribI2uiv_noimpl(GLuint index, const GLuint* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribI2uiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glQueryCounter_noimpl(GLuint id, GLenum target)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glQueryCounter is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glActiveShaderProgram_noimpl(GLuint pipeline, GLuint program)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glActiveShaderProgram is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniform1ui_noimpl(GLint location, GLuint v0)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform1ui is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribI1i_noimpl(GLuint index, GLint x)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribI1i is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetTexParameterIiv_noimpl(GLenum target, GLenum pname, GLint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetTexParameterIiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetUniformfv_noimpl(GLuint program, GLint location, GLfloat* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetUniformfv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform2uiv_noimpl(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform2uiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLenum oc_glGetError_noimpl(void)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetError is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLenum)0);
}

void oc_glGetActiveUniformBlockName_noimpl(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetActiveUniformBlockName is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glTextureView_noimpl(GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glTextureView is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetnUniformiv_noimpl(GLuint program, GLint location, GLsizei bufSize, GLint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetnUniformiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform4dv_noimpl(GLuint program, GLint location, GLsizei count, const GLdouble* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform4dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glViewportIndexedfv_noimpl(GLuint index, const GLfloat* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glViewportIndexedfv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glHint_noimpl(GLenum target, GLenum mode)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glHint is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetShaderSource_noimpl(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* source)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetShaderSource is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniformMatrix4x3fv_noimpl(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniformMatrix4x3fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniform1iv_noimpl(GLint location, GLsizei count, const GLint* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform1iv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribI4bv_noimpl(GLuint index, const GLbyte* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribI4bv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniformMatrix4x2dv_noimpl(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniformMatrix4x2dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBufferStorage_noimpl(GLenum target, GLsizeiptr size, const void* data, GLbitfield flags)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBufferStorage is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLboolean oc_glIsRenderbuffer_noimpl(GLuint renderbuffer)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glIsRenderbuffer is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLboolean)0);
}

void oc_glGetActiveSubroutineUniformName_noimpl(GLuint program, GLenum shadertype, GLuint index, GLsizei bufSize, GLsizei* length, GLchar* name)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetActiveSubroutineUniformName is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glLinkProgram_noimpl(GLuint program)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glLinkProgram is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetActiveUniformsiv_noimpl(GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetActiveUniformsiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLuint oc_glGetDebugMessageLog_noimpl(GLuint count, GLsizei bufSize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetDebugMessageLog is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLuint)0);
}

void oc_glCopyTexSubImage3D_noimpl(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glCopyTexSubImage3D is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glPointParameteri_noimpl(GLenum pname, GLint param)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glPointParameteri is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform3dv_noimpl(GLuint program, GLint location, GLsizei count, const GLdouble* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform3dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glCompressedTexImage1D_noimpl(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void* data)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glCompressedTexImage1D is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniformMatrix3x4fv_noimpl(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniformMatrix3x4fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGenSamplers_noimpl(GLsizei count, GLuint* samplers)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGenSamplers is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetCompressedTexImage_noimpl(GLenum target, GLint level, void* img)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetCompressedTexImage is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDeleteQueries_noimpl(GLsizei n, const GLuint* ids)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDeleteQueries is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGenProgramPipelines_noimpl(GLsizei n, GLuint* pipelines)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGenProgramPipelines is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDispatchComputeIndirect_noimpl(GLintptr indirect)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDispatchComputeIndirect is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribIPointer_noimpl(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribIPointer is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLuint oc_glCreateProgram_noimpl(void)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glCreateProgram is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLuint)0);
}

void oc_glClearTexSubImage_noimpl(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* data)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glClearTexSubImage is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib4d_noimpl(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib4d is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glFrontFace_noimpl(GLenum mode)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glFrontFace is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBindTransformFeedback_noimpl(GLenum target, GLuint id)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBindTransformFeedback is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetProgramStageiv_noimpl(GLuint program, GLenum shadertype, GLenum pname, GLint* values)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetProgramStageiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glSamplerParameterIiv_noimpl(GLuint sampler, GLenum pname, const GLint* param)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glSamplerParameterIiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetInteger64v_noimpl(GLenum pname, GLint64* data)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetInteger64v is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLuint oc_glCreateShaderProgramv_noimpl(GLenum type, GLsizei count, const GLchar* const* strings)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glCreateShaderProgramv is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLuint)0);
}

void oc_glBindBuffersRange_noimpl(GLenum target, GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizeiptr* sizes)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBindBuffersRange is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniform3fv_noimpl(GLint location, GLsizei count, const GLfloat* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform3fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniformMatrix4fv_noimpl(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniformMatrix4fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBindBuffersBase_noimpl(GLenum target, GLuint first, GLsizei count, const GLuint* buffers)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBindBuffersBase is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glClearBufferfi_noimpl(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glClearBufferfi is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glFramebufferTexture3D_noimpl(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glFramebufferTexture3D is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDisable_noimpl(GLenum cap)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDisable is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform1iv_noimpl(GLuint program, GLint location, GLsizei count, const GLint* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform1iv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribI2iv_noimpl(GLuint index, const GLint* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribI2iv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDepthRangeIndexed_noimpl(GLuint index, GLdouble n, GLdouble f)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDepthRangeIndexed is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glPatchParameteri_noimpl(GLenum pname, GLint value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glPatchParameteri is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLuint oc_glGetUniformBlockIndex_noimpl(GLuint program, const GLchar* uniformBlockName)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetUniformBlockIndex is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLuint)0);
}

void oc_glMultiDrawArrays_noimpl(GLenum mode, const GLint* first, const GLsizei* count, GLsizei drawcount)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glMultiDrawArrays is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribI4ubv_noimpl(GLuint index, const GLubyte* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribI4ubv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBindBuffer_noimpl(GLenum target, GLuint buffer)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBindBuffer is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribI3i_noimpl(GLuint index, GLint x, GLint y, GLint z)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribI3i is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetDoublev_noimpl(GLenum pname, GLdouble* data)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetDoublev is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDrawTransformFeedbackStream_noimpl(GLenum mode, GLuint id, GLuint stream)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDrawTransformFeedbackStream is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribI4uiv_noimpl(GLuint index, const GLuint* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribI4uiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glRenderbufferStorageMultisample_noimpl(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glRenderbufferStorageMultisample is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribL3dv_noimpl(GLuint index, const GLdouble* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribL3dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glStencilMaskSeparate_noimpl(GLenum face, GLuint mask)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glStencilMaskSeparate is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform1d_noimpl(GLuint program, GLint location, GLdouble v0)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform1d is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glViewport_noimpl(GLint x, GLint y, GLsizei width, GLsizei height)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glViewport is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribP1ui_noimpl(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribP1ui is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib4dv_noimpl(GLuint index, const GLdouble* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib4dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGenQueries_noimpl(GLsizei n, GLuint* ids)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGenQueries is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glTexParameterIiv_noimpl(GLenum target, GLenum pname, const GLint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glTexParameterIiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform2d_noimpl(GLuint program, GLint location, GLdouble v0, GLdouble v1)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform2d is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform1uiv_noimpl(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform1uiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib4Nub_noimpl(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib4Nub is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLboolean oc_glIsVertexArray_noimpl(GLuint array)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glIsVertexArray is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLboolean)0);
}

void oc_glProgramUniform3f_noimpl(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform3f is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform3iv_noimpl(GLuint program, GLint location, GLsizei count, const GLint* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform3iv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetProgramBinary_noimpl(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, void* binary)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetProgramBinary is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBindRenderbuffer_noimpl(GLenum target, GLuint renderbuffer)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBindRenderbuffer is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBindFragDataLocationIndexed_noimpl(GLuint program, GLuint colorNumber, GLuint index, const GLchar* name)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBindFragDataLocationIndexed is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetSamplerParameterIiv_noimpl(GLuint sampler, GLenum pname, GLint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetSamplerParameterIiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribDivisor_noimpl(GLuint index, GLuint divisor)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribDivisor is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniformMatrix3x2dv_noimpl(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniformMatrix3x2dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glFramebufferParameteri_noimpl(GLenum target, GLenum pname, GLint param)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glFramebufferParameteri is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGenTransformFeedbacks_noimpl(GLsizei n, GLuint* ids)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGenTransformFeedbacks is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDeleteSync_noimpl(GLsync sync)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDeleteSync is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform1ui_noimpl(GLuint program, GLint location, GLuint v0)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform1ui is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glTexSubImage1D_noimpl(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glTexSubImage1D is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glClearDepthf_noimpl(GLfloat d)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glClearDepthf is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glReadPixels_noimpl(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glReadPixels is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribI2i_noimpl(GLuint index, GLint x, GLint y)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribI2i is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glFinish_noimpl(void)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glFinish is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glLineWidth_noimpl(GLfloat width)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glLineWidth is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDeleteShader_noimpl(GLuint shader)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDeleteShader is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLboolean oc_glIsSampler_noimpl(GLuint sampler)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glIsSampler is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLboolean)0);
}

void oc_glProgramUniformMatrix4dv_noimpl(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniformMatrix4dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glTransformFeedbackVaryings_noimpl(GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glTransformFeedbackVaryings is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBeginConditionalRender_noimpl(GLuint id, GLenum mode)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBeginConditionalRender is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBindSamplers_noimpl(GLuint first, GLsizei count, const GLuint* samplers)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBindSamplers is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDeleteProgramPipelines_noimpl(GLsizei n, const GLuint* pipelines)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDeleteProgramPipelines is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glColorMask_noimpl(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glColorMask is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glTexParameterfv_noimpl(GLenum target, GLenum pname, const GLfloat* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glTexParameterfv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glPushDebugGroup_noimpl(GLenum source, GLuint id, GLsizei length, const GLchar* message)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glPushDebugGroup is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glClearBufferfv_noimpl(GLenum buffer, GLint drawbuffer, const GLfloat* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glClearBufferfv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLboolean oc_glIsEnabled_noimpl(GLenum cap)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glIsEnabled is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLboolean)0);
}

void oc_glVertexAttrib2f_noimpl(GLuint index, GLfloat x, GLfloat y)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib2f is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform2f_noimpl(GLuint program, GLint location, GLfloat v0, GLfloat v1)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform2f is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetSamplerParameterIuiv_noimpl(GLuint sampler, GLenum pname, GLuint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetSamplerParameterIuiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetInteger64i_v_noimpl(GLenum target, GLuint index, GLint64* data)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetInteger64i_v is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniform2dv_noimpl(GLint location, GLsizei count, const GLdouble* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform2dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetBufferSubData_noimpl(GLenum target, GLintptr offset, GLsizeiptr size, void* data)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetBufferSubData is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glMultiDrawElementsIndirect_noimpl(GLenum mode, GLenum type, const void* indirect, GLsizei drawcount, GLsizei stride)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glMultiDrawElementsIndirect is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramParameteri_noimpl(GLuint program, GLenum pname, GLint value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramParameteri is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribP4ui_noimpl(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribP4ui is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glSamplerParameterfv_noimpl(GLuint sampler, GLenum pname, const GLfloat* param)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glSamplerParameterfv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glPointParameterf_noimpl(GLenum pname, GLfloat param)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glPointParameterf is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniformMatrix2x4fv_noimpl(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniformMatrix2x4fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGenBuffers_noimpl(GLsizei n, GLuint* buffers)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGenBuffers is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform2dv_noimpl(GLuint program, GLint location, GLsizei count, const GLdouble* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform2dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribFormat_noimpl(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribFormat is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glTexSubImage2D_noimpl(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glTexSubImage2D is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib4ubv_noimpl(GLuint index, const GLubyte* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib4ubv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLenum oc_glGetGraphicsResetStatus_noimpl(void)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetGraphicsResetStatus is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLenum)0);
}

void oc_glGetProgramInterfaceiv_noimpl(GLuint program, GLenum programInterface, GLenum pname, GLint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetProgramInterfaceiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribIFormat_noimpl(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribIFormat is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetnUniformfv_noimpl(GLuint program, GLint location, GLsizei bufSize, GLfloat* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetnUniformfv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDeleteProgram_noimpl(GLuint program)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDeleteProgram is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glClampColor_noimpl(GLenum target, GLenum clamp)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glClampColor is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDrawElementsInstancedBaseVertexBaseInstance_noimpl(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDrawElementsInstancedBaseVertexBaseInstance is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDrawElements_noimpl(GLenum mode, GLsizei count, GLenum type, const void* indices)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDrawElements is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDebugMessageControl_noimpl(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDebugMessageControl is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetRenderbufferParameteriv_noimpl(GLenum target, GLenum pname, GLint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetRenderbufferParameteriv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDetachShader_noimpl(GLuint program, GLuint shader)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDetachShader is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGenFramebuffers_noimpl(GLsizei n, GLuint* framebuffers)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGenFramebuffers is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProvokingVertex_noimpl(GLenum mode)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProvokingVertex is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glSampleMaski_noimpl(GLuint maskNumber, GLbitfield mask)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glSampleMaski is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glEndQueryIndexed_noimpl(GLenum target, GLuint index)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glEndQueryIndexed is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform1f_noimpl(GLuint program, GLint location, GLfloat v0)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform1f is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBindFramebuffer_noimpl(GLenum target, GLuint framebuffer)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBindFramebuffer is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBeginQueryIndexed_noimpl(GLenum target, GLuint index, GLuint id)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBeginQueryIndexed is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniformSubroutinesuiv_noimpl(GLenum shadertype, GLsizei count, const GLuint* indices)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniformSubroutinesuiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetUniformiv_noimpl(GLuint program, GLint location, GLint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetUniformiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glFramebufferTexture_noimpl(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glFramebufferTexture is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glPointParameterfv_noimpl(GLenum pname, const GLfloat* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glPointParameterfv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLboolean oc_glIsTransformFeedback_noimpl(GLuint id)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glIsTransformFeedback is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLboolean)0);
}

GLenum oc_glCheckFramebufferStatus_noimpl(GLenum target)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glCheckFramebufferStatus is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLenum)0);
}

void oc_glShaderSource_noimpl(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glShaderSource is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniformMatrix2x4dv_noimpl(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniformMatrix2x4dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBindImageTextures_noimpl(GLuint first, GLsizei count, const GLuint* textures)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBindImageTextures is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glCopyTexImage1D_noimpl(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glCopyTexImage1D is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniformMatrix3dv_noimpl(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniformMatrix3dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform1dv_noimpl(GLuint program, GLint location, GLsizei count, const GLdouble* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform1dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBlitFramebuffer_noimpl(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBlitFramebuffer is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glPopDebugGroup_noimpl(void)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glPopDebugGroup is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glTexParameterIuiv_noimpl(GLenum target, GLenum pname, const GLuint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glTexParameterIuiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib2d_noimpl(GLuint index, GLdouble x, GLdouble y)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib2d is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glTexImage1D_noimpl(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void* pixels)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glTexImage1D is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetObjectPtrLabel_noimpl(const void* ptr, GLsizei bufSize, GLsizei* length, GLchar* label)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetObjectPtrLabel is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glStencilMask_noimpl(GLuint mask)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glStencilMask is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBeginQuery_noimpl(GLenum target, GLuint id)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBeginQuery is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniformMatrix4fv_noimpl(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniformMatrix4fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLboolean oc_glIsSync_noimpl(GLsync sync)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glIsSync is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLboolean)0);
}

void oc_glUniform3dv_noimpl(GLint location, GLsizei count, const GLdouble* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform3dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform2fv_noimpl(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform2fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribI4sv_noimpl(GLuint index, const GLshort* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribI4sv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glScissorArrayv_noimpl(GLuint first, GLsizei count, const GLint* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glScissorArrayv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribP1uiv_noimpl(GLuint index, GLenum type, GLboolean normalized, const GLuint* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribP1uiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniform2uiv_noimpl(GLint location, GLsizei count, const GLuint* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform2uiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDeleteBuffers_noimpl(GLsizei n, const GLuint* buffers)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDeleteBuffers is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform3ui_noimpl(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform3ui is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glFramebufferTextureLayer_noimpl(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glFramebufferTextureLayer is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glEndTransformFeedback_noimpl(void)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glEndTransformFeedback is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBlendFuncSeparatei_noimpl(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBlendFuncSeparatei is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDrawTransformFeedbackInstanced_noimpl(GLenum mode, GLuint id, GLsizei instancecount)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDrawTransformFeedbackInstanced is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDrawRangeElementsBaseVertex_noimpl(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices, GLint basevertex)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDrawRangeElementsBaseVertex is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib1f_noimpl(GLuint index, GLfloat x)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib1f is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetUniformSubroutineuiv_noimpl(GLenum shadertype, GLint location, GLuint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetUniformSubroutineuiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDisableVertexAttribArray_noimpl(GLuint index)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDisableVertexAttribArray is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniformMatrix3x2fv_noimpl(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniformMatrix3x2fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribI4usv_noimpl(GLuint index, const GLushort* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribI4usv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetObjectLabel_noimpl(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei* length, GLchar* label)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetObjectLabel is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBindAttribLocation_noimpl(GLuint program, GLuint index, const GLchar* name)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBindAttribLocation is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniform1f_noimpl(GLint location, GLfloat v0)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform1f is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetUniformdv_noimpl(GLuint program, GLint location, GLdouble* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetUniformdv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLint oc_glGetUniformLocation_noimpl(GLuint program, const GLchar* name)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetUniformLocation is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLint)0);
}

GLint oc_glGetSubroutineUniformLocation_noimpl(GLuint program, GLenum shadertype, const GLchar* name)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetSubroutineUniformLocation is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLint)0);
}

void oc_glGetTexParameterIuiv_noimpl(GLenum target, GLenum pname, GLuint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetTexParameterIuiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glSamplerParameterf_noimpl(GLuint sampler, GLenum pname, GLfloat param)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glSamplerParameterf is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribL3d_noimpl(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribL3d is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glTexImage3DMultisample_noimpl(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glTexImage3DMultisample is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glTexImage3D_noimpl(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glTexImage3D is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glRenderbufferStorage_noimpl(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glRenderbufferStorage is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glEnableVertexAttribArray_noimpl(GLuint index)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glEnableVertexAttribArray is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribP4uiv_noimpl(GLuint index, GLenum type, GLboolean normalized, const GLuint* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribP4uiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniform4d_noimpl(GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform4d is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib4s_noimpl(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib4s is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDrawElementsInstancedBaseVertex_noimpl(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLint basevertex)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDrawElementsInstancedBaseVertex is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib3s_noimpl(GLuint index, GLshort x, GLshort y, GLshort z)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib3s is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform2iv_noimpl(GLuint program, GLint location, GLsizei count, const GLint* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform2iv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glStencilFuncSeparate_noimpl(GLenum face, GLenum func, GLint ref, GLuint mask)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glStencilFuncSeparate is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDeleteFramebuffers_noimpl(GLsizei n, const GLuint* framebuffers)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDeleteFramebuffers is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDepthRange_noimpl(GLdouble n, GLdouble f)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDepthRange is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniformMatrix3x2fv_noimpl(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniformMatrix3x2fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniformMatrix2dv_noimpl(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniformMatrix2dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glShaderStorageBlockBinding_noimpl(GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glShaderStorageBlockBinding is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glClearDepth_noimpl(GLdouble depth)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glClearDepth is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib2dv_noimpl(GLuint index, const GLdouble* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib2dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glSamplerParameterIuiv_noimpl(GLuint sampler, GLenum pname, const GLuint* param)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glSamplerParameterIuiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetVertexAttribLdv_noimpl(GLuint index, GLenum pname, GLdouble* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetVertexAttribLdv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniformMatrix3x4dv_noimpl(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniformMatrix3x4dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDepthRangeArrayv_noimpl(GLuint first, GLsizei count, const GLdouble* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDepthRangeArrayv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetActiveUniform_noimpl(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetActiveUniform is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glPatchParameterfv_noimpl(GLenum pname, const GLfloat* values)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glPatchParameterfv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glInvalidateTexImage_noimpl(GLuint texture, GLint level)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glInvalidateTexImage is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib3f_noimpl(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib3f is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform4iv_noimpl(GLuint program, GLint location, GLsizei count, const GLint* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform4iv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform4d_noimpl(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform4d is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLboolean oc_glIsFramebuffer_noimpl(GLuint framebuffer)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glIsFramebuffer is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLboolean)0);
}

void oc_glPixelStoref_noimpl(GLenum pname, GLfloat param)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glPixelStoref is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform4uiv_noimpl(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform4uiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniformMatrix4x2dv_noimpl(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniformMatrix4x2dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLsync oc_glFenceSync_noimpl(GLenum condition, GLbitfield flags)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glFenceSync is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLsync)0);
}

void oc_glGetBufferParameteri64v_noimpl(GLenum target, GLenum pname, GLint64* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetBufferParameteri64v is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glStencilOp_noimpl(GLenum fail, GLenum zfail, GLenum zpass)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glStencilOp is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glClearBufferData_noimpl(GLenum target, GLenum internalformat, GLenum format, GLenum type, const void* data)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glClearBufferData is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetnUniformuiv_noimpl(GLuint program, GLint location, GLsizei bufSize, GLuint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetnUniformuiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetProgramResourceiv_noimpl(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum* props, GLsizei count, GLsizei* length, GLint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetProgramResourceiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetVertexAttribdv_noimpl(GLuint index, GLenum pname, GLdouble* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetVertexAttribdv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetTransformFeedbackVarying_noimpl(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetTransformFeedbackVarying is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib2fv_noimpl(GLuint index, const GLfloat* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib2fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetBooleani_v_noimpl(GLenum target, GLuint index, GLboolean* data)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetBooleani_v is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glColorMaski_noimpl(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glColorMaski is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glInvalidateBufferSubData_noimpl(GLuint buffer, GLintptr offset, GLsizeiptr length)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glInvalidateBufferSubData is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniformMatrix4dv_noimpl(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniformMatrix4dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLboolean oc_glIsQuery_noimpl(GLuint id)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glIsQuery is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLboolean)0);
}

void oc_glUniform4ui_noimpl(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform4ui is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniform4i_noimpl(GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform4i is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetSamplerParameteriv_noimpl(GLuint sampler, GLenum pname, GLint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetSamplerParameteriv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glMultiDrawElementsBaseVertex_noimpl(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices, GLsizei drawcount, const GLint* basevertex)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glMultiDrawElementsBaseVertex is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribI1uiv_noimpl(GLuint index, const GLuint* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribI1uiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetIntegerv_noimpl(GLenum pname, GLint* data)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetIntegerv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniformMatrix2x3fv_noimpl(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniformMatrix2x3fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glTexImage2D_noimpl(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glTexImage2D is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetAttachedShaders_noimpl(GLuint program, GLsizei maxCount, GLsizei* count, GLuint* shaders)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetAttachedShaders is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniform2d_noimpl(GLint location, GLdouble x, GLdouble y)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform2d is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glMemoryBarrierByRegion_noimpl(GLbitfield barriers)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glMemoryBarrierByRegion is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniformMatrix2fv_noimpl(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniformMatrix2fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glPrimitiveRestartIndex_noimpl(GLuint index)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glPrimitiveRestartIndex is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetVertexAttribiv_noimpl(GLuint index, GLenum pname, GLint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetVertexAttribiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLint oc_glGetAttribLocation_noimpl(GLuint program, const GLchar* name)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetAttribLocation is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLint)0);
}

void oc_glTexStorage2DMultisample_noimpl(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glTexStorage2DMultisample is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glCompressedTexSubImage2D_noimpl(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glCompressedTexSubImage2D is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetVertexAttribfv_noimpl(GLuint index, GLenum pname, GLfloat* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetVertexAttribfv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetBufferParameteriv_noimpl(GLenum target, GLenum pname, GLint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetBufferParameteriv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glTexParameterf_noimpl(GLenum target, GLenum pname, GLfloat param)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glTexParameterf is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glFramebufferTexture2D_noimpl(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glFramebufferTexture2D is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetActiveAttrib_noimpl(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetActiveAttrib is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glInvalidateTexSubImage_noimpl(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glInvalidateTexSubImage is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDeleteVertexArrays_noimpl(GLsizei n, const GLuint* arrays)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDeleteVertexArrays is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribI2ui_noimpl(GLuint index, GLuint x, GLuint y)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribI2ui is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glPointParameteriv_noimpl(GLenum pname, const GLint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glPointParameteriv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetPointerv_noimpl(GLenum pname, void** params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetPointerv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glEnablei_noimpl(GLenum target, GLuint index)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glEnablei is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBindBufferRange_noimpl(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBindBufferRange is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDrawArraysInstanced_noimpl(GLenum mode, GLint first, GLsizei count, GLsizei instancecount)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDrawArraysInstanced is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDeleteTextures_noimpl(GLsizei n, const GLuint* textures)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDeleteTextures is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib4Niv_noimpl(GLuint index, const GLint* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib4Niv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glMultiDrawElements_noimpl(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices, GLsizei drawcount)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glMultiDrawElements is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetProgramiv_noimpl(GLuint program, GLenum pname, GLint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetProgramiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDepthFunc_noimpl(GLenum func)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDepthFunc is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGenTextures_noimpl(GLsizei n, GLuint* textures)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGenTextures is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetInternalformativ_noimpl(GLenum target, GLenum internalformat, GLenum pname, GLsizei count, GLint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetInternalformativ is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform3i_noimpl(GLuint program, GLint location, GLint v0, GLint v1, GLint v2)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform3i is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glScissorIndexed_noimpl(GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glScissorIndexed is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib2sv_noimpl(GLuint index, const GLshort* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib2sv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glTexStorage3DMultisample_noimpl(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glTexStorage3DMultisample is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniform2iv_noimpl(GLint location, GLsizei count, const GLint* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform2iv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDrawArraysInstancedBaseInstance_noimpl(GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDrawArraysInstancedBaseInstance is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttribI3ui_noimpl(GLuint index, GLuint x, GLuint y, GLuint z)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribI3ui is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDeleteSamplers_noimpl(GLsizei count, const GLuint* samplers)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDeleteSamplers is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGenVertexArrays_noimpl(GLsizei n, GLuint* arrays)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGenVertexArrays is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetFramebufferParameteriv_noimpl(GLenum target, GLenum pname, GLint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetFramebufferParameteriv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glPolygonMode_noimpl(GLenum face, GLenum mode)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glPolygonMode is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniformMatrix2x4fv_noimpl(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniformMatrix2x4fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetProgramResourceName_noimpl(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei* length, GLchar* name)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetProgramResourceName is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glSamplerParameteriv_noimpl(GLuint sampler, GLenum pname, const GLint* param)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glSamplerParameteriv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetActiveSubroutineUniformiv_noimpl(GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint* values)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetActiveSubroutineUniformiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

const GLubyte* oc_glGetStringi_noimpl(GLenum name, GLuint index)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetStringi is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((const GLubyte*)0);
}

void oc_glVertexAttribLFormat_noimpl(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttribLFormat is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib3d_noimpl(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib3d is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBindVertexArray_noimpl(GLuint array)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBindVertexArray is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLboolean oc_glUnmapBuffer_noimpl(GLenum target)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUnmapBuffer is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLboolean)0);
}

void oc_glDrawElementsInstancedBaseInstance_noimpl(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLuint baseinstance)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDrawElementsInstancedBaseInstance is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniform4uiv_noimpl(GLint location, GLsizei count, const GLuint* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform4uiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glFramebufferTexture1D_noimpl(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glFramebufferTexture1D is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDrawTransformFeedbackStreamInstanced_noimpl(GLenum mode, GLuint id, GLuint stream, GLsizei instancecount)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDrawTransformFeedbackStreamInstanced is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glStencilFunc_noimpl(GLenum func, GLint ref, GLuint mask)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glStencilFunc is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glValidateProgram_noimpl(GLuint program)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glValidateProgram is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glFlush_noimpl(void)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glFlush is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform3uiv_noimpl(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform3uiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDeleteRenderbuffers_noimpl(GLsizei n, const GLuint* renderbuffers)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDeleteRenderbuffers is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib4fv_noimpl(GLuint index, const GLfloat* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib4fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniformMatrix2dv_noimpl(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniformMatrix2dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLint oc_glGetFragDataIndex_noimpl(GLuint program, const GLchar* name)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetFragDataIndex is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLint)0);
}

void oc_glUniform3iv_noimpl(GLint location, GLsizei count, const GLint* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform3iv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glMinSampleShading_noimpl(GLfloat value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glMinSampleShading is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetBooleanv_noimpl(GLenum pname, GLboolean* data)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetBooleanv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetMultisamplefv_noimpl(GLenum pname, GLuint index, GLfloat* val)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetMultisamplefv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetVertexAttribIuiv_noimpl(GLuint index, GLenum pname, GLuint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetVertexAttribIuiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetProgramInfoLog_noimpl(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetProgramInfoLog is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniform4fv_noimpl(GLint location, GLsizei count, const GLfloat* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform4fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDrawBuffer_noimpl(GLenum buf)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDrawBuffer is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniform1i_noimpl(GLint location, GLint v0)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform1i is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform4ui_noimpl(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform4ui is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniformMatrix3fv_noimpl(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniformMatrix3fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBlendEquationSeparate_noimpl(GLenum modeRGB, GLenum modeAlpha)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBlendEquationSeparate is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBindProgramPipeline_noimpl(GLuint pipeline)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBindProgramPipeline is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetDoublei_v_noimpl(GLenum target, GLuint index, GLdouble* data)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetDoublei_v is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBufferData_noimpl(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBufferData is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glClearColor_noimpl(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glClearColor is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform4i_noimpl(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform4i is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetTexLevelParameteriv_noimpl(GLenum target, GLint level, GLenum pname, GLint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetTexLevelParameteriv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetActiveUniformBlockiv_noimpl(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetActiveUniformBlockiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform1fv_noimpl(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform1fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glPauseTransformFeedback_noimpl(void)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glPauseTransformFeedback is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetBufferPointerv_noimpl(GLenum target, GLenum pname, void** params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetBufferPointerv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glInvalidateSubFramebuffer_noimpl(GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glInvalidateSubFramebuffer is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glScissorIndexedv_noimpl(GLuint index, const GLint* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glScissorIndexedv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniform2ui_noimpl(GLint location, GLuint v0, GLuint v1)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform2ui is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBindTexture_noimpl(GLenum target, GLuint texture)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBindTexture is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDrawElementsInstanced_noimpl(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDrawElementsInstanced is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform4f_noimpl(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform4f is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBindBufferBase_noimpl(GLenum target, GLuint index, GLuint buffer)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBindBufferBase is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLboolean oc_glIsShader_noimpl(GLuint shader)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glIsShader is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLboolean)0);
}

void oc_glClearBufferSubData_noimpl(GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void* data)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glClearBufferSubData is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib4Nuiv_noimpl(GLuint index, const GLuint* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib4Nuiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDrawArraysIndirect_noimpl(GLenum mode, const void* indirect)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDrawArraysIndirect is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glVertexAttrib4usv_noimpl(GLuint index, const GLushort* v)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glVertexAttrib4usv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniform1d_noimpl(GLint location, GLdouble x)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform1d is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glClearTexImage_noimpl(GLuint texture, GLint level, GLenum format, GLenum type, const void* data)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glClearTexImage is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniform1uiv_noimpl(GLint location, GLsizei count, const GLuint* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform1uiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBindSampler_noimpl(GLuint unit, GLuint sampler)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBindSampler is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetTexLevelParameterfv_noimpl(GLenum target, GLint level, GLenum pname, GLfloat* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetTexLevelParameterfv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glClearBufferiv_noimpl(GLenum buffer, GLint drawbuffer, const GLint* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glClearBufferiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glLogicOp_noimpl(GLenum opcode)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glLogicOp is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glActiveTexture_noimpl(GLenum texture)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glActiveTexture is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

GLint oc_glGetFragDataLocation_noimpl(GLuint program, const GLchar* name)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetFragDataLocation is not part of currently selected %s API\n", oc_glAPI->name);
    }
    return ((GLint)0);
}

void oc_glBlendColor_noimpl(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBlendColor is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniformMatrix4x3fv_noimpl(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniformMatrix4x3fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glProgramUniform3fv_noimpl(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glProgramUniform3fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniform1fv_noimpl(GLint location, GLsizei count, const GLfloat* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform1fv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDrawElementsBaseVertex_noimpl(GLenum mode, GLsizei count, GLenum type, const void* indices, GLint basevertex)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDrawElementsBaseVertex is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniform4f_noimpl(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniform4f is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBlendEquationSeparatei_noimpl(GLuint buf, GLenum modeRGB, GLenum modeAlpha)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBlendEquationSeparatei is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBlendFuncSeparate_noimpl(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBlendFuncSeparate is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glClearBufferuiv_noimpl(GLenum buffer, GLint drawbuffer, const GLuint* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glClearBufferuiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glCopyTexSubImage1D_noimpl(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glCopyTexSubImage1D is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDrawTransformFeedback_noimpl(GLenum mode, GLuint id)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDrawTransformFeedback is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glReadBuffer_noimpl(GLenum src)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glReadBuffer is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glCopyBufferSubData_noimpl(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glCopyBufferSubData is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGetUniformuiv_noimpl(GLuint program, GLint location, GLuint* params)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGetUniformuiv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glPolygonOffset_noimpl(GLfloat factor, GLfloat units)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glPolygonOffset is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glDispatchCompute_noimpl(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glDispatchCompute is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glBindImageTexture_noimpl(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glBindImageTexture is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glUniformMatrix4x3dv_noimpl(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glUniformMatrix4x3dv is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

void oc_glGenRenderbuffers_noimpl(GLsizei n, GLuint* renderbuffers)
{
    if(oc_glAPI == &oc_glNoAPI)
    {
        oc_log_error("No GL or GLES API is selected. Make sure you call oc_surface_select() before calling OpenGL API functions.\n");
    }
    else
    {
        oc_log_error("glGenRenderbuffers is not part of currently selected %s API\n", oc_glAPI->name);
    }
}

oc_gl_api oc_glNoAPI = {
    .GetFloatv = oc_glGetFloatv_noimpl,
    .TexBufferRange = oc_glTexBufferRange_noimpl,
    .IsBuffer = oc_glIsBuffer_noimpl,
    .IsTexture = oc_glIsTexture_noimpl,
    .DepthRangef = oc_glDepthRangef_noimpl,
    .EndConditionalRender = oc_glEndConditionalRender_noimpl,
    .BlendFunci = oc_glBlendFunci_noimpl,
    .GetProgramPipelineiv = oc_glGetProgramPipelineiv_noimpl,
    .WaitSync = oc_glWaitSync_noimpl,
    .ProgramUniformMatrix2fv = oc_glProgramUniformMatrix2fv_noimpl,
    .ProgramUniformMatrix4x3dv = oc_glProgramUniformMatrix4x3dv_noimpl,
    .VertexAttrib1dv = oc_glVertexAttrib1dv_noimpl,
    .SamplerParameteri = oc_glSamplerParameteri_noimpl,
    .GetVertexAttribIiv = oc_glGetVertexAttribIiv_noimpl,
    .GetSamplerParameterfv = oc_glGetSamplerParameterfv_noimpl,
    .VertexAttrib1d = oc_glVertexAttrib1d_noimpl,
    .TexBuffer = oc_glTexBuffer_noimpl,
    .InvalidateBufferData = oc_glInvalidateBufferData_noimpl,
    .ProgramUniform2i = oc_glProgramUniform2i_noimpl,
    .Uniform4dv = oc_glUniform4dv_noimpl,
    .UseProgram = oc_glUseProgram_noimpl,
    .VertexAttribI3iv = oc_glVertexAttribI3iv_noimpl,
    .DrawElementsIndirect = oc_glDrawElementsIndirect_noimpl,
    .VertexAttrib4uiv = oc_glVertexAttrib4uiv_noimpl,
    .GetQueryObjectiv = oc_glGetQueryObjectiv_noimpl,
    .FramebufferRenderbuffer = oc_glFramebufferRenderbuffer_noimpl,
    .BlendEquationi = oc_glBlendEquationi_noimpl,
    .GetActiveSubroutineName = oc_glGetActiveSubroutineName_noimpl,
    .VertexAttrib2s = oc_glVertexAttrib2s_noimpl,
    .VertexAttribL1d = oc_glVertexAttribL1d_noimpl,
    .BindTextures = oc_glBindTextures_noimpl,
    .VertexAttrib3sv = oc_glVertexAttrib3sv_noimpl,
    .GetFloati_v = oc_glGetFloati_v_noimpl,
    .BeginTransformFeedback = oc_glBeginTransformFeedback_noimpl,
    .ClearStencil = oc_glClearStencil_noimpl,
    .Uniform3i = oc_glUniform3i_noimpl,
    .ValidateProgramPipeline = oc_glValidateProgramPipeline_noimpl,
    .ProgramUniformMatrix4x2fv = oc_glProgramUniformMatrix4x2fv_noimpl,
    .VertexAttribI4ui = oc_glVertexAttribI4ui_noimpl,
    .GetShaderiv = oc_glGetShaderiv_noimpl,
    .ReadnPixels = oc_glReadnPixels_noimpl,
    .UniformMatrix4x2fv = oc_glUniformMatrix4x2fv_noimpl,
    .GetShaderPrecisionFormat = oc_glGetShaderPrecisionFormat_noimpl,
    .ProgramUniformMatrix2x3fv = oc_glProgramUniformMatrix2x3fv_noimpl,
    .TexSubImage3D = oc_glTexSubImage3D_noimpl,
    .GetProgramResourceLocationIndex = oc_glGetProgramResourceLocationIndex_noimpl,
    .BlendFunc = oc_glBlendFunc_noimpl,
    .ProgramUniformMatrix3x4fv = oc_glProgramUniformMatrix3x4fv_noimpl,
    .Uniform3d = oc_glUniform3d_noimpl,
    .VertexAttrib1sv = oc_glVertexAttrib1sv_noimpl,
    .BindFragDataLocation = oc_glBindFragDataLocation_noimpl,
    .VertexAttrib4bv = oc_glVertexAttrib4bv_noimpl,
    .Uniform4iv = oc_glUniform4iv_noimpl,
    .ProgramUniform2ui = oc_glProgramUniform2ui_noimpl,
    .DrawArrays = oc_glDrawArrays_noimpl,
    .ProgramBinary = oc_glProgramBinary_noimpl,
    .VertexAttrib4f = oc_glVertexAttrib4f_noimpl,
    .VertexAttribP2uiv = oc_glVertexAttribP2uiv_noimpl,
    .UniformMatrix3fv = oc_glUniformMatrix3fv_noimpl,
    .Uniform2i = oc_glUniform2i_noimpl,
    .GetQueryObjectuiv = oc_glGetQueryObjectuiv_noimpl,
    .UniformBlockBinding = oc_glUniformBlockBinding_noimpl,
    .SampleCoverage = oc_glSampleCoverage_noimpl,
    .VertexAttrib4Nusv = oc_glVertexAttrib4Nusv_noimpl,
    .ProgramUniformMatrix2x4dv = oc_glProgramUniformMatrix2x4dv_noimpl,
    .Uniform3uiv = oc_glUniform3uiv_noimpl,
    .VertexAttrib1s = oc_glVertexAttrib1s_noimpl,
    .GetVertexAttribPointerv = oc_glGetVertexAttribPointerv_noimpl,
    .BlendBarrier = oc_glBlendBarrier_noimpl,
    .DrawRangeElements = oc_glDrawRangeElements_noimpl,
    .TexStorage3D = oc_glTexStorage3D_noimpl,
    .GetInternalformati64v = oc_glGetInternalformati64v_noimpl,
    .GetQueryObjecti64v = oc_glGetQueryObjecti64v_noimpl,
    .CompressedTexSubImage1D = oc_glCompressedTexSubImage1D_noimpl,
    .VertexAttrib3dv = oc_glVertexAttrib3dv_noimpl,
    .VertexBindingDivisor = oc_glVertexBindingDivisor_noimpl,
    .UseProgramStages = oc_glUseProgramStages_noimpl,
    .VertexAttribBinding = oc_glVertexAttribBinding_noimpl,
    .DebugMessageInsert = oc_glDebugMessageInsert_noimpl,
    .GetTexParameteriv = oc_glGetTexParameteriv_noimpl,
    .MultiDrawArraysIndirect = oc_glMultiDrawArraysIndirect_noimpl,
    .GetTexParameterfv = oc_glGetTexParameterfv_noimpl,
    .GetProgramPipelineInfoLog = oc_glGetProgramPipelineInfoLog_noimpl,
    .EndQuery = oc_glEndQuery_noimpl,
    .GetProgramResourceLocation = oc_glGetProgramResourceLocation_noimpl,
    .CompressedTexImage2D = oc_glCompressedTexImage2D_noimpl,
    .VertexAttribP2ui = oc_glVertexAttribP2ui_noimpl,
    .IsEnabledi = oc_glIsEnabledi_noimpl,
    .GetActiveAtomicCounterBufferiv = oc_glGetActiveAtomicCounterBufferiv_noimpl,
    .IsProgram = oc_glIsProgram_noimpl,
    .Uniform1dv = oc_glUniform1dv_noimpl,
    .TexParameteriv = oc_glTexParameteriv_noimpl,
    .Uniform2fv = oc_glUniform2fv_noimpl,
    .ReleaseShaderCompiler = oc_glReleaseShaderCompiler_noimpl,
    .CullFace = oc_glCullFace_noimpl,
    .VertexAttribI4i = oc_glVertexAttribI4i_noimpl,
    .GetProgramResourceIndex = oc_glGetProgramResourceIndex_noimpl,
    .ShaderBinary = oc_glShaderBinary_noimpl,
    .UniformMatrix3x2dv = oc_glUniformMatrix3x2dv_noimpl,
    .InvalidateFramebuffer = oc_glInvalidateFramebuffer_noimpl,
    .AttachShader = oc_glAttachShader_noimpl,
    .FlushMappedBufferRange = oc_glFlushMappedBufferRange_noimpl,
    .VertexAttribP3uiv = oc_glVertexAttribP3uiv_noimpl,
    .GetActiveUniformName = oc_glGetActiveUniformName_noimpl,
    .MapBuffer = oc_glMapBuffer_noimpl,
    .DrawBuffers = oc_glDrawBuffers_noimpl,
    .GetSynciv = oc_glGetSynciv_noimpl,
    .CopyTexSubImage2D = oc_glCopyTexSubImage2D_noimpl,
    .ObjectLabel = oc_glObjectLabel_noimpl,
    .BufferSubData = oc_glBufferSubData_noimpl,
    .Uniform2f = oc_glUniform2f_noimpl,
    .DebugMessageCallback = oc_glDebugMessageCallback_noimpl,
    .VertexAttribL4dv = oc_glVertexAttribL4dv_noimpl,
    .IsProgramPipeline = oc_glIsProgramPipeline_noimpl,
    .ResumeTransformFeedback = oc_glResumeTransformFeedback_noimpl,
    .VertexAttribI4iv = oc_glVertexAttribI4iv_noimpl,
    .GetShaderInfoLog = oc_glGetShaderInfoLog_noimpl,
    .GetIntegeri_v = oc_glGetIntegeri_v_noimpl,
    .BindVertexBuffer = oc_glBindVertexBuffer_noimpl,
    .BlendEquation = oc_glBlendEquation_noimpl,
    .VertexAttribL2dv = oc_glVertexAttribL2dv_noimpl,
    .VertexAttribI1ui = oc_glVertexAttribI1ui_noimpl,
    .VertexAttrib4Nsv = oc_glVertexAttrib4Nsv_noimpl,
    .VertexAttribL4d = oc_glVertexAttribL4d_noimpl,
    .CopyImageSubData = oc_glCopyImageSubData_noimpl,
    .GetFramebufferAttachmentParameteriv = oc_glGetFramebufferAttachmentParameteriv_noimpl,
    .VertexAttribL2d = oc_glVertexAttribL2d_noimpl,
    .GetSubroutineIndex = oc_glGetSubroutineIndex_noimpl,
    .VertexAttribI3uiv = oc_glVertexAttribI3uiv_noimpl,
    .VertexAttrib4iv = oc_glVertexAttrib4iv_noimpl,
    .BindVertexBuffers = oc_glBindVertexBuffers_noimpl,
    .ProgramUniformMatrix2x3dv = oc_glProgramUniformMatrix2x3dv_noimpl,
    .PrimitiveBoundingBox = oc_glPrimitiveBoundingBox_noimpl,
    .Scissor = oc_glScissor_noimpl,
    .ClientWaitSync = oc_glClientWaitSync_noimpl,
    .Uniform3ui = oc_glUniform3ui_noimpl,
    .VertexAttribP3ui = oc_glVertexAttribP3ui_noimpl,
    .Enable = oc_glEnable_noimpl,
    .StencilOpSeparate = oc_glStencilOpSeparate_noimpl,
    .UniformMatrix2x3dv = oc_glUniformMatrix2x3dv_noimpl,
    .ProgramUniformMatrix3dv = oc_glProgramUniformMatrix3dv_noimpl,
    .TexImage2DMultisample = oc_glTexImage2DMultisample_noimpl,
    .VertexAttrib4Nbv = oc_glVertexAttrib4Nbv_noimpl,
    .GetTexImage = oc_glGetTexImage_noimpl,
    .VertexAttrib4sv = oc_glVertexAttrib4sv_noimpl,
    .PixelStorei = oc_glPixelStorei_noimpl,
    .DepthMask = oc_glDepthMask_noimpl,
    .TexStorage2D = oc_glTexStorage2D_noimpl,
    .Clear = oc_glClear_noimpl,
    .UniformMatrix3x4dv = oc_glUniformMatrix3x4dv_noimpl,
    .DeleteTransformFeedbacks = oc_glDeleteTransformFeedbacks_noimpl,
    .MapBufferRange = oc_glMapBufferRange_noimpl,
    .MemoryBarrier = oc_glMemoryBarrier_noimpl,
    .ViewportIndexedf = oc_glViewportIndexedf_noimpl,
    .VertexAttrib3fv = oc_glVertexAttrib3fv_noimpl,
    .ObjectPtrLabel = oc_glObjectPtrLabel_noimpl,
    .TexStorage1D = oc_glTexStorage1D_noimpl,
    .CompressedTexImage3D = oc_glCompressedTexImage3D_noimpl,
    .VertexAttrib1fv = oc_glVertexAttrib1fv_noimpl,
    .VertexAttribPointer = oc_glVertexAttribPointer_noimpl,
    .GetQueryIndexediv = oc_glGetQueryIndexediv_noimpl,
    .CompileShader = oc_glCompileShader_noimpl,
    .ProgramUniform1i = oc_glProgramUniform1i_noimpl,
    .GetQueryiv = oc_glGetQueryiv_noimpl,
    .VertexAttribI1iv = oc_glVertexAttribI1iv_noimpl,
    .CopyTexImage2D = oc_glCopyTexImage2D_noimpl,
    .GetQueryObjectui64v = oc_glGetQueryObjectui64v_noimpl,
    .PointSize = oc_glPointSize_noimpl,
    .Disablei = oc_glDisablei_noimpl,
    .VertexAttribL1dv = oc_glVertexAttribL1dv_noimpl,
    .CreateShader = oc_glCreateShader_noimpl,
    .GetString = oc_glGetString_noimpl,
    .ViewportArrayv = oc_glViewportArrayv_noimpl,
    .ProgramUniform3d = oc_glProgramUniform3d_noimpl,
    .VertexAttrib4Nubv = oc_glVertexAttrib4Nubv_noimpl,
    .TexParameteri = oc_glTexParameteri_noimpl,
    .ProgramUniform4fv = oc_glProgramUniform4fv_noimpl,
    .GenerateMipmap = oc_glGenerateMipmap_noimpl,
    .CompressedTexSubImage3D = oc_glCompressedTexSubImage3D_noimpl,
    .Uniform3f = oc_glUniform3f_noimpl,
    .GetUniformIndices = oc_glGetUniformIndices_noimpl,
    .VertexAttribLPointer = oc_glVertexAttribLPointer_noimpl,
    .VertexAttribI2uiv = oc_glVertexAttribI2uiv_noimpl,
    .QueryCounter = oc_glQueryCounter_noimpl,
    .ActiveShaderProgram = oc_glActiveShaderProgram_noimpl,
    .Uniform1ui = oc_glUniform1ui_noimpl,
    .VertexAttribI1i = oc_glVertexAttribI1i_noimpl,
    .GetTexParameterIiv = oc_glGetTexParameterIiv_noimpl,
    .GetUniformfv = oc_glGetUniformfv_noimpl,
    .ProgramUniform2uiv = oc_glProgramUniform2uiv_noimpl,
    .GetError = oc_glGetError_noimpl,
    .GetActiveUniformBlockName = oc_glGetActiveUniformBlockName_noimpl,
    .TextureView = oc_glTextureView_noimpl,
    .GetnUniformiv = oc_glGetnUniformiv_noimpl,
    .ProgramUniform4dv = oc_glProgramUniform4dv_noimpl,
    .ViewportIndexedfv = oc_glViewportIndexedfv_noimpl,
    .Hint = oc_glHint_noimpl,
    .GetShaderSource = oc_glGetShaderSource_noimpl,
    .ProgramUniformMatrix4x3fv = oc_glProgramUniformMatrix4x3fv_noimpl,
    .Uniform1iv = oc_glUniform1iv_noimpl,
    .VertexAttribI4bv = oc_glVertexAttribI4bv_noimpl,
    .UniformMatrix4x2dv = oc_glUniformMatrix4x2dv_noimpl,
    .BufferStorage = oc_glBufferStorage_noimpl,
    .IsRenderbuffer = oc_glIsRenderbuffer_noimpl,
    .GetActiveSubroutineUniformName = oc_glGetActiveSubroutineUniformName_noimpl,
    .LinkProgram = oc_glLinkProgram_noimpl,
    .GetActiveUniformsiv = oc_glGetActiveUniformsiv_noimpl,
    .GetDebugMessageLog = oc_glGetDebugMessageLog_noimpl,
    .CopyTexSubImage3D = oc_glCopyTexSubImage3D_noimpl,
    .PointParameteri = oc_glPointParameteri_noimpl,
    .ProgramUniform3dv = oc_glProgramUniform3dv_noimpl,
    .CompressedTexImage1D = oc_glCompressedTexImage1D_noimpl,
    .UniformMatrix3x4fv = oc_glUniformMatrix3x4fv_noimpl,
    .GenSamplers = oc_glGenSamplers_noimpl,
    .GetCompressedTexImage = oc_glGetCompressedTexImage_noimpl,
    .DeleteQueries = oc_glDeleteQueries_noimpl,
    .GenProgramPipelines = oc_glGenProgramPipelines_noimpl,
    .DispatchComputeIndirect = oc_glDispatchComputeIndirect_noimpl,
    .VertexAttribIPointer = oc_glVertexAttribIPointer_noimpl,
    .CreateProgram = oc_glCreateProgram_noimpl,
    .ClearTexSubImage = oc_glClearTexSubImage_noimpl,
    .VertexAttrib4d = oc_glVertexAttrib4d_noimpl,
    .FrontFace = oc_glFrontFace_noimpl,
    .BindTransformFeedback = oc_glBindTransformFeedback_noimpl,
    .GetProgramStageiv = oc_glGetProgramStageiv_noimpl,
    .SamplerParameterIiv = oc_glSamplerParameterIiv_noimpl,
    .GetInteger64v = oc_glGetInteger64v_noimpl,
    .CreateShaderProgramv = oc_glCreateShaderProgramv_noimpl,
    .BindBuffersRange = oc_glBindBuffersRange_noimpl,
    .Uniform3fv = oc_glUniform3fv_noimpl,
    .ProgramUniformMatrix4fv = oc_glProgramUniformMatrix4fv_noimpl,
    .BindBuffersBase = oc_glBindBuffersBase_noimpl,
    .ClearBufferfi = oc_glClearBufferfi_noimpl,
    .FramebufferTexture3D = oc_glFramebufferTexture3D_noimpl,
    .Disable = oc_glDisable_noimpl,
    .ProgramUniform1iv = oc_glProgramUniform1iv_noimpl,
    .VertexAttribI2iv = oc_glVertexAttribI2iv_noimpl,
    .DepthRangeIndexed = oc_glDepthRangeIndexed_noimpl,
    .PatchParameteri = oc_glPatchParameteri_noimpl,
    .GetUniformBlockIndex = oc_glGetUniformBlockIndex_noimpl,
    .MultiDrawArrays = oc_glMultiDrawArrays_noimpl,
    .VertexAttribI4ubv = oc_glVertexAttribI4ubv_noimpl,
    .BindBuffer = oc_glBindBuffer_noimpl,
    .VertexAttribI3i = oc_glVertexAttribI3i_noimpl,
    .GetDoublev = oc_glGetDoublev_noimpl,
    .DrawTransformFeedbackStream = oc_glDrawTransformFeedbackStream_noimpl,
    .VertexAttribI4uiv = oc_glVertexAttribI4uiv_noimpl,
    .RenderbufferStorageMultisample = oc_glRenderbufferStorageMultisample_noimpl,
    .VertexAttribL3dv = oc_glVertexAttribL3dv_noimpl,
    .StencilMaskSeparate = oc_glStencilMaskSeparate_noimpl,
    .ProgramUniform1d = oc_glProgramUniform1d_noimpl,
    .Viewport = oc_glViewport_noimpl,
    .VertexAttribP1ui = oc_glVertexAttribP1ui_noimpl,
    .VertexAttrib4dv = oc_glVertexAttrib4dv_noimpl,
    .GenQueries = oc_glGenQueries_noimpl,
    .TexParameterIiv = oc_glTexParameterIiv_noimpl,
    .ProgramUniform2d = oc_glProgramUniform2d_noimpl,
    .ProgramUniform1uiv = oc_glProgramUniform1uiv_noimpl,
    .VertexAttrib4Nub = oc_glVertexAttrib4Nub_noimpl,
    .IsVertexArray = oc_glIsVertexArray_noimpl,
    .ProgramUniform3f = oc_glProgramUniform3f_noimpl,
    .ProgramUniform3iv = oc_glProgramUniform3iv_noimpl,
    .GetProgramBinary = oc_glGetProgramBinary_noimpl,
    .BindRenderbuffer = oc_glBindRenderbuffer_noimpl,
    .BindFragDataLocationIndexed = oc_glBindFragDataLocationIndexed_noimpl,
    .GetSamplerParameterIiv = oc_glGetSamplerParameterIiv_noimpl,
    .VertexAttribDivisor = oc_glVertexAttribDivisor_noimpl,
    .ProgramUniformMatrix3x2dv = oc_glProgramUniformMatrix3x2dv_noimpl,
    .FramebufferParameteri = oc_glFramebufferParameteri_noimpl,
    .GenTransformFeedbacks = oc_glGenTransformFeedbacks_noimpl,
    .DeleteSync = oc_glDeleteSync_noimpl,
    .ProgramUniform1ui = oc_glProgramUniform1ui_noimpl,
    .TexSubImage1D = oc_glTexSubImage1D_noimpl,
    .ClearDepthf = oc_glClearDepthf_noimpl,
    .ReadPixels = oc_glReadPixels_noimpl,
    .VertexAttribI2i = oc_glVertexAttribI2i_noimpl,
    .Finish = oc_glFinish_noimpl,
    .LineWidth = oc_glLineWidth_noimpl,
    .DeleteShader = oc_glDeleteShader_noimpl,
    .IsSampler = oc_glIsSampler_noimpl,
    .ProgramUniformMatrix4dv = oc_glProgramUniformMatrix4dv_noimpl,
    .TransformFeedbackVaryings = oc_glTransformFeedbackVaryings_noimpl,
    .BeginConditionalRender = oc_glBeginConditionalRender_noimpl,
    .BindSamplers = oc_glBindSamplers_noimpl,
    .DeleteProgramPipelines = oc_glDeleteProgramPipelines_noimpl,
    .ColorMask = oc_glColorMask_noimpl,
    .TexParameterfv = oc_glTexParameterfv_noimpl,
    .PushDebugGroup = oc_glPushDebugGroup_noimpl,
    .ClearBufferfv = oc_glClearBufferfv_noimpl,
    .IsEnabled = oc_glIsEnabled_noimpl,
    .VertexAttrib2f = oc_glVertexAttrib2f_noimpl,
    .ProgramUniform2f = oc_glProgramUniform2f_noimpl,
    .GetSamplerParameterIuiv = oc_glGetSamplerParameterIuiv_noimpl,
    .GetInteger64i_v = oc_glGetInteger64i_v_noimpl,
    .Uniform2dv = oc_glUniform2dv_noimpl,
    .GetBufferSubData = oc_glGetBufferSubData_noimpl,
    .MultiDrawElementsIndirect = oc_glMultiDrawElementsIndirect_noimpl,
    .ProgramParameteri = oc_glProgramParameteri_noimpl,
    .VertexAttribP4ui = oc_glVertexAttribP4ui_noimpl,
    .SamplerParameterfv = oc_glSamplerParameterfv_noimpl,
    .PointParameterf = oc_glPointParameterf_noimpl,
    .UniformMatrix2x4fv = oc_glUniformMatrix2x4fv_noimpl,
    .GenBuffers = oc_glGenBuffers_noimpl,
    .ProgramUniform2dv = oc_glProgramUniform2dv_noimpl,
    .VertexAttribFormat = oc_glVertexAttribFormat_noimpl,
    .TexSubImage2D = oc_glTexSubImage2D_noimpl,
    .VertexAttrib4ubv = oc_glVertexAttrib4ubv_noimpl,
    .GetGraphicsResetStatus = oc_glGetGraphicsResetStatus_noimpl,
    .GetProgramInterfaceiv = oc_glGetProgramInterfaceiv_noimpl,
    .VertexAttribIFormat = oc_glVertexAttribIFormat_noimpl,
    .GetnUniformfv = oc_glGetnUniformfv_noimpl,
    .DeleteProgram = oc_glDeleteProgram_noimpl,
    .ClampColor = oc_glClampColor_noimpl,
    .DrawElementsInstancedBaseVertexBaseInstance = oc_glDrawElementsInstancedBaseVertexBaseInstance_noimpl,
    .DrawElements = oc_glDrawElements_noimpl,
    .DebugMessageControl = oc_glDebugMessageControl_noimpl,
    .GetRenderbufferParameteriv = oc_glGetRenderbufferParameteriv_noimpl,
    .DetachShader = oc_glDetachShader_noimpl,
    .GenFramebuffers = oc_glGenFramebuffers_noimpl,
    .ProvokingVertex = oc_glProvokingVertex_noimpl,
    .SampleMaski = oc_glSampleMaski_noimpl,
    .EndQueryIndexed = oc_glEndQueryIndexed_noimpl,
    .ProgramUniform1f = oc_glProgramUniform1f_noimpl,
    .BindFramebuffer = oc_glBindFramebuffer_noimpl,
    .BeginQueryIndexed = oc_glBeginQueryIndexed_noimpl,
    .UniformSubroutinesuiv = oc_glUniformSubroutinesuiv_noimpl,
    .GetUniformiv = oc_glGetUniformiv_noimpl,
    .FramebufferTexture = oc_glFramebufferTexture_noimpl,
    .PointParameterfv = oc_glPointParameterfv_noimpl,
    .IsTransformFeedback = oc_glIsTransformFeedback_noimpl,
    .CheckFramebufferStatus = oc_glCheckFramebufferStatus_noimpl,
    .ShaderSource = oc_glShaderSource_noimpl,
    .UniformMatrix2x4dv = oc_glUniformMatrix2x4dv_noimpl,
    .BindImageTextures = oc_glBindImageTextures_noimpl,
    .CopyTexImage1D = oc_glCopyTexImage1D_noimpl,
    .UniformMatrix3dv = oc_glUniformMatrix3dv_noimpl,
    .ProgramUniform1dv = oc_glProgramUniform1dv_noimpl,
    .BlitFramebuffer = oc_glBlitFramebuffer_noimpl,
    .PopDebugGroup = oc_glPopDebugGroup_noimpl,
    .TexParameterIuiv = oc_glTexParameterIuiv_noimpl,
    .VertexAttrib2d = oc_glVertexAttrib2d_noimpl,
    .TexImage1D = oc_glTexImage1D_noimpl,
    .GetObjectPtrLabel = oc_glGetObjectPtrLabel_noimpl,
    .StencilMask = oc_glStencilMask_noimpl,
    .BeginQuery = oc_glBeginQuery_noimpl,
    .UniformMatrix4fv = oc_glUniformMatrix4fv_noimpl,
    .IsSync = oc_glIsSync_noimpl,
    .Uniform3dv = oc_glUniform3dv_noimpl,
    .ProgramUniform2fv = oc_glProgramUniform2fv_noimpl,
    .VertexAttribI4sv = oc_glVertexAttribI4sv_noimpl,
    .ScissorArrayv = oc_glScissorArrayv_noimpl,
    .VertexAttribP1uiv = oc_glVertexAttribP1uiv_noimpl,
    .Uniform2uiv = oc_glUniform2uiv_noimpl,
    .DeleteBuffers = oc_glDeleteBuffers_noimpl,
    .ProgramUniform3ui = oc_glProgramUniform3ui_noimpl,
    .FramebufferTextureLayer = oc_glFramebufferTextureLayer_noimpl,
    .EndTransformFeedback = oc_glEndTransformFeedback_noimpl,
    .BlendFuncSeparatei = oc_glBlendFuncSeparatei_noimpl,
    .DrawTransformFeedbackInstanced = oc_glDrawTransformFeedbackInstanced_noimpl,
    .DrawRangeElementsBaseVertex = oc_glDrawRangeElementsBaseVertex_noimpl,
    .VertexAttrib1f = oc_glVertexAttrib1f_noimpl,
    .GetUniformSubroutineuiv = oc_glGetUniformSubroutineuiv_noimpl,
    .DisableVertexAttribArray = oc_glDisableVertexAttribArray_noimpl,
    .ProgramUniformMatrix3x2fv = oc_glProgramUniformMatrix3x2fv_noimpl,
    .VertexAttribI4usv = oc_glVertexAttribI4usv_noimpl,
    .GetObjectLabel = oc_glGetObjectLabel_noimpl,
    .BindAttribLocation = oc_glBindAttribLocation_noimpl,
    .Uniform1f = oc_glUniform1f_noimpl,
    .GetUniformdv = oc_glGetUniformdv_noimpl,
    .GetUniformLocation = oc_glGetUniformLocation_noimpl,
    .GetSubroutineUniformLocation = oc_glGetSubroutineUniformLocation_noimpl,
    .GetTexParameterIuiv = oc_glGetTexParameterIuiv_noimpl,
    .SamplerParameterf = oc_glSamplerParameterf_noimpl,
    .VertexAttribL3d = oc_glVertexAttribL3d_noimpl,
    .TexImage3DMultisample = oc_glTexImage3DMultisample_noimpl,
    .TexImage3D = oc_glTexImage3D_noimpl,
    .RenderbufferStorage = oc_glRenderbufferStorage_noimpl,
    .EnableVertexAttribArray = oc_glEnableVertexAttribArray_noimpl,
    .VertexAttribP4uiv = oc_glVertexAttribP4uiv_noimpl,
    .Uniform4d = oc_glUniform4d_noimpl,
    .VertexAttrib4s = oc_glVertexAttrib4s_noimpl,
    .DrawElementsInstancedBaseVertex = oc_glDrawElementsInstancedBaseVertex_noimpl,
    .VertexAttrib3s = oc_glVertexAttrib3s_noimpl,
    .ProgramUniform2iv = oc_glProgramUniform2iv_noimpl,
    .StencilFuncSeparate = oc_glStencilFuncSeparate_noimpl,
    .DeleteFramebuffers = oc_glDeleteFramebuffers_noimpl,
    .DepthRange = oc_glDepthRange_noimpl,
    .UniformMatrix3x2fv = oc_glUniformMatrix3x2fv_noimpl,
    .ProgramUniformMatrix2dv = oc_glProgramUniformMatrix2dv_noimpl,
    .ShaderStorageBlockBinding = oc_glShaderStorageBlockBinding_noimpl,
    .ClearDepth = oc_glClearDepth_noimpl,
    .VertexAttrib2dv = oc_glVertexAttrib2dv_noimpl,
    .SamplerParameterIuiv = oc_glSamplerParameterIuiv_noimpl,
    .GetVertexAttribLdv = oc_glGetVertexAttribLdv_noimpl,
    .ProgramUniformMatrix3x4dv = oc_glProgramUniformMatrix3x4dv_noimpl,
    .DepthRangeArrayv = oc_glDepthRangeArrayv_noimpl,
    .GetActiveUniform = oc_glGetActiveUniform_noimpl,
    .PatchParameterfv = oc_glPatchParameterfv_noimpl,
    .InvalidateTexImage = oc_glInvalidateTexImage_noimpl,
    .VertexAttrib3f = oc_glVertexAttrib3f_noimpl,
    .ProgramUniform4iv = oc_glProgramUniform4iv_noimpl,
    .ProgramUniform4d = oc_glProgramUniform4d_noimpl,
    .IsFramebuffer = oc_glIsFramebuffer_noimpl,
    .PixelStoref = oc_glPixelStoref_noimpl,
    .ProgramUniform4uiv = oc_glProgramUniform4uiv_noimpl,
    .ProgramUniformMatrix4x2dv = oc_glProgramUniformMatrix4x2dv_noimpl,
    .FenceSync = oc_glFenceSync_noimpl,
    .GetBufferParameteri64v = oc_glGetBufferParameteri64v_noimpl,
    .StencilOp = oc_glStencilOp_noimpl,
    .ClearBufferData = oc_glClearBufferData_noimpl,
    .GetnUniformuiv = oc_glGetnUniformuiv_noimpl,
    .GetProgramResourceiv = oc_glGetProgramResourceiv_noimpl,
    .GetVertexAttribdv = oc_glGetVertexAttribdv_noimpl,
    .GetTransformFeedbackVarying = oc_glGetTransformFeedbackVarying_noimpl,
    .VertexAttrib2fv = oc_glVertexAttrib2fv_noimpl,
    .GetBooleani_v = oc_glGetBooleani_v_noimpl,
    .ColorMaski = oc_glColorMaski_noimpl,
    .InvalidateBufferSubData = oc_glInvalidateBufferSubData_noimpl,
    .UniformMatrix4dv = oc_glUniformMatrix4dv_noimpl,
    .IsQuery = oc_glIsQuery_noimpl,
    .Uniform4ui = oc_glUniform4ui_noimpl,
    .Uniform4i = oc_glUniform4i_noimpl,
    .GetSamplerParameteriv = oc_glGetSamplerParameteriv_noimpl,
    .MultiDrawElementsBaseVertex = oc_glMultiDrawElementsBaseVertex_noimpl,
    .VertexAttribI1uiv = oc_glVertexAttribI1uiv_noimpl,
    .GetIntegerv = oc_glGetIntegerv_noimpl,
    .UniformMatrix2x3fv = oc_glUniformMatrix2x3fv_noimpl,
    .TexImage2D = oc_glTexImage2D_noimpl,
    .GetAttachedShaders = oc_glGetAttachedShaders_noimpl,
    .Uniform2d = oc_glUniform2d_noimpl,
    .MemoryBarrierByRegion = oc_glMemoryBarrierByRegion_noimpl,
    .UniformMatrix2fv = oc_glUniformMatrix2fv_noimpl,
    .PrimitiveRestartIndex = oc_glPrimitiveRestartIndex_noimpl,
    .GetVertexAttribiv = oc_glGetVertexAttribiv_noimpl,
    .GetAttribLocation = oc_glGetAttribLocation_noimpl,
    .TexStorage2DMultisample = oc_glTexStorage2DMultisample_noimpl,
    .CompressedTexSubImage2D = oc_glCompressedTexSubImage2D_noimpl,
    .GetVertexAttribfv = oc_glGetVertexAttribfv_noimpl,
    .GetBufferParameteriv = oc_glGetBufferParameteriv_noimpl,
    .TexParameterf = oc_glTexParameterf_noimpl,
    .FramebufferTexture2D = oc_glFramebufferTexture2D_noimpl,
    .GetActiveAttrib = oc_glGetActiveAttrib_noimpl,
    .InvalidateTexSubImage = oc_glInvalidateTexSubImage_noimpl,
    .DeleteVertexArrays = oc_glDeleteVertexArrays_noimpl,
    .VertexAttribI2ui = oc_glVertexAttribI2ui_noimpl,
    .PointParameteriv = oc_glPointParameteriv_noimpl,
    .GetPointerv = oc_glGetPointerv_noimpl,
    .Enablei = oc_glEnablei_noimpl,
    .BindBufferRange = oc_glBindBufferRange_noimpl,
    .DrawArraysInstanced = oc_glDrawArraysInstanced_noimpl,
    .DeleteTextures = oc_glDeleteTextures_noimpl,
    .VertexAttrib4Niv = oc_glVertexAttrib4Niv_noimpl,
    .MultiDrawElements = oc_glMultiDrawElements_noimpl,
    .GetProgramiv = oc_glGetProgramiv_noimpl,
    .DepthFunc = oc_glDepthFunc_noimpl,
    .GenTextures = oc_glGenTextures_noimpl,
    .GetInternalformativ = oc_glGetInternalformativ_noimpl,
    .ProgramUniform3i = oc_glProgramUniform3i_noimpl,
    .ScissorIndexed = oc_glScissorIndexed_noimpl,
    .VertexAttrib2sv = oc_glVertexAttrib2sv_noimpl,
    .TexStorage3DMultisample = oc_glTexStorage3DMultisample_noimpl,
    .Uniform2iv = oc_glUniform2iv_noimpl,
    .DrawArraysInstancedBaseInstance = oc_glDrawArraysInstancedBaseInstance_noimpl,
    .VertexAttribI3ui = oc_glVertexAttribI3ui_noimpl,
    .DeleteSamplers = oc_glDeleteSamplers_noimpl,
    .GenVertexArrays = oc_glGenVertexArrays_noimpl,
    .GetFramebufferParameteriv = oc_glGetFramebufferParameteriv_noimpl,
    .PolygonMode = oc_glPolygonMode_noimpl,
    .ProgramUniformMatrix2x4fv = oc_glProgramUniformMatrix2x4fv_noimpl,
    .GetProgramResourceName = oc_glGetProgramResourceName_noimpl,
    .SamplerParameteriv = oc_glSamplerParameteriv_noimpl,
    .GetActiveSubroutineUniformiv = oc_glGetActiveSubroutineUniformiv_noimpl,
    .GetStringi = oc_glGetStringi_noimpl,
    .VertexAttribLFormat = oc_glVertexAttribLFormat_noimpl,
    .VertexAttrib3d = oc_glVertexAttrib3d_noimpl,
    .BindVertexArray = oc_glBindVertexArray_noimpl,
    .UnmapBuffer = oc_glUnmapBuffer_noimpl,
    .DrawElementsInstancedBaseInstance = oc_glDrawElementsInstancedBaseInstance_noimpl,
    .Uniform4uiv = oc_glUniform4uiv_noimpl,
    .FramebufferTexture1D = oc_glFramebufferTexture1D_noimpl,
    .DrawTransformFeedbackStreamInstanced = oc_glDrawTransformFeedbackStreamInstanced_noimpl,
    .StencilFunc = oc_glStencilFunc_noimpl,
    .ValidateProgram = oc_glValidateProgram_noimpl,
    .Flush = oc_glFlush_noimpl,
    .ProgramUniform3uiv = oc_glProgramUniform3uiv_noimpl,
    .DeleteRenderbuffers = oc_glDeleteRenderbuffers_noimpl,
    .VertexAttrib4fv = oc_glVertexAttrib4fv_noimpl,
    .UniformMatrix2dv = oc_glUniformMatrix2dv_noimpl,
    .GetFragDataIndex = oc_glGetFragDataIndex_noimpl,
    .Uniform3iv = oc_glUniform3iv_noimpl,
    .MinSampleShading = oc_glMinSampleShading_noimpl,
    .GetBooleanv = oc_glGetBooleanv_noimpl,
    .GetMultisamplefv = oc_glGetMultisamplefv_noimpl,
    .GetVertexAttribIuiv = oc_glGetVertexAttribIuiv_noimpl,
    .GetProgramInfoLog = oc_glGetProgramInfoLog_noimpl,
    .Uniform4fv = oc_glUniform4fv_noimpl,
    .DrawBuffer = oc_glDrawBuffer_noimpl,
    .Uniform1i = oc_glUniform1i_noimpl,
    .ProgramUniform4ui = oc_glProgramUniform4ui_noimpl,
    .ProgramUniformMatrix3fv = oc_glProgramUniformMatrix3fv_noimpl,
    .BlendEquationSeparate = oc_glBlendEquationSeparate_noimpl,
    .BindProgramPipeline = oc_glBindProgramPipeline_noimpl,
    .GetDoublei_v = oc_glGetDoublei_v_noimpl,
    .BufferData = oc_glBufferData_noimpl,
    .ClearColor = oc_glClearColor_noimpl,
    .ProgramUniform4i = oc_glProgramUniform4i_noimpl,
    .GetTexLevelParameteriv = oc_glGetTexLevelParameteriv_noimpl,
    .GetActiveUniformBlockiv = oc_glGetActiveUniformBlockiv_noimpl,
    .ProgramUniform1fv = oc_glProgramUniform1fv_noimpl,
    .PauseTransformFeedback = oc_glPauseTransformFeedback_noimpl,
    .GetBufferPointerv = oc_glGetBufferPointerv_noimpl,
    .InvalidateSubFramebuffer = oc_glInvalidateSubFramebuffer_noimpl,
    .ScissorIndexedv = oc_glScissorIndexedv_noimpl,
    .Uniform2ui = oc_glUniform2ui_noimpl,
    .BindTexture = oc_glBindTexture_noimpl,
    .DrawElementsInstanced = oc_glDrawElementsInstanced_noimpl,
    .ProgramUniform4f = oc_glProgramUniform4f_noimpl,
    .BindBufferBase = oc_glBindBufferBase_noimpl,
    .IsShader = oc_glIsShader_noimpl,
    .ClearBufferSubData = oc_glClearBufferSubData_noimpl,
    .VertexAttrib4Nuiv = oc_glVertexAttrib4Nuiv_noimpl,
    .DrawArraysIndirect = oc_glDrawArraysIndirect_noimpl,
    .VertexAttrib4usv = oc_glVertexAttrib4usv_noimpl,
    .Uniform1d = oc_glUniform1d_noimpl,
    .ClearTexImage = oc_glClearTexImage_noimpl,
    .Uniform1uiv = oc_glUniform1uiv_noimpl,
    .BindSampler = oc_glBindSampler_noimpl,
    .GetTexLevelParameterfv = oc_glGetTexLevelParameterfv_noimpl,
    .ClearBufferiv = oc_glClearBufferiv_noimpl,
    .LogicOp = oc_glLogicOp_noimpl,
    .ActiveTexture = oc_glActiveTexture_noimpl,
    .GetFragDataLocation = oc_glGetFragDataLocation_noimpl,
    .BlendColor = oc_glBlendColor_noimpl,
    .UniformMatrix4x3fv = oc_glUniformMatrix4x3fv_noimpl,
    .ProgramUniform3fv = oc_glProgramUniform3fv_noimpl,
    .Uniform1fv = oc_glUniform1fv_noimpl,
    .DrawElementsBaseVertex = oc_glDrawElementsBaseVertex_noimpl,
    .Uniform4f = oc_glUniform4f_noimpl,
    .BlendEquationSeparatei = oc_glBlendEquationSeparatei_noimpl,
    .BlendFuncSeparate = oc_glBlendFuncSeparate_noimpl,
    .ClearBufferuiv = oc_glClearBufferuiv_noimpl,
    .CopyTexSubImage1D = oc_glCopyTexSubImage1D_noimpl,
    .DrawTransformFeedback = oc_glDrawTransformFeedback_noimpl,
    .ReadBuffer = oc_glReadBuffer_noimpl,
    .CopyBufferSubData = oc_glCopyBufferSubData_noimpl,
    .GetUniformuiv = oc_glGetUniformuiv_noimpl,
    .PolygonOffset = oc_glPolygonOffset_noimpl,
    .DispatchCompute = oc_glDispatchCompute_noimpl,
    .BindImageTexture = oc_glBindImageTexture_noimpl,
    .UniformMatrix4x3dv = oc_glUniformMatrix4x3dv_noimpl,
    .GenRenderbuffers = oc_glGenRenderbuffers_noimpl,
};

void oc_gl_load_gl41(oc_gl_api* api, oc_gl_load_proc loadProc)
{
    api->name = "gl41";
    api->GetFloatv = loadProc("glGetFloatv");
    api->TexBufferRange = oc_glTexBufferRange_noimpl;
    api->IsBuffer = loadProc("glIsBuffer");
    api->IsTexture = loadProc("glIsTexture");
    api->DepthRangef = loadProc("glDepthRangef");
    api->EndConditionalRender = loadProc("glEndConditionalRender");
    api->BlendFunci = loadProc("glBlendFunci");
    api->GetProgramPipelineiv = loadProc("glGetProgramPipelineiv");
    api->WaitSync = loadProc("glWaitSync");
    api->ProgramUniformMatrix2fv = loadProc("glProgramUniformMatrix2fv");
    api->ProgramUniformMatrix4x3dv = loadProc("glProgramUniformMatrix4x3dv");
    api->VertexAttrib1dv = loadProc("glVertexAttrib1dv");
    api->SamplerParameteri = loadProc("glSamplerParameteri");
    api->GetVertexAttribIiv = loadProc("glGetVertexAttribIiv");
    api->GetSamplerParameterfv = loadProc("glGetSamplerParameterfv");
    api->VertexAttrib1d = loadProc("glVertexAttrib1d");
    api->TexBuffer = loadProc("glTexBuffer");
    api->InvalidateBufferData = oc_glInvalidateBufferData_noimpl;
    api->ProgramUniform2i = loadProc("glProgramUniform2i");
    api->Uniform4dv = loadProc("glUniform4dv");
    api->UseProgram = loadProc("glUseProgram");
    api->VertexAttribI3iv = loadProc("glVertexAttribI3iv");
    api->DrawElementsIndirect = loadProc("glDrawElementsIndirect");
    api->VertexAttrib4uiv = loadProc("glVertexAttrib4uiv");
    api->GetQueryObjectiv = loadProc("glGetQueryObjectiv");
    api->FramebufferRenderbuffer = loadProc("glFramebufferRenderbuffer");
    api->BlendEquationi = loadProc("glBlendEquationi");
    api->GetActiveSubroutineName = loadProc("glGetActiveSubroutineName");
    api->VertexAttrib2s = loadProc("glVertexAttrib2s");
    api->VertexAttribL1d = loadProc("glVertexAttribL1d");
    api->BindTextures = oc_glBindTextures_noimpl;
    api->VertexAttrib3sv = loadProc("glVertexAttrib3sv");
    api->GetFloati_v = loadProc("glGetFloati_v");
    api->BeginTransformFeedback = loadProc("glBeginTransformFeedback");
    api->ClearStencil = loadProc("glClearStencil");
    api->Uniform3i = loadProc("glUniform3i");
    api->ValidateProgramPipeline = loadProc("glValidateProgramPipeline");
    api->ProgramUniformMatrix4x2fv = loadProc("glProgramUniformMatrix4x2fv");
    api->VertexAttribI4ui = loadProc("glVertexAttribI4ui");
    api->GetShaderiv = loadProc("glGetShaderiv");
    api->ReadnPixels = oc_glReadnPixels_noimpl;
    api->UniformMatrix4x2fv = loadProc("glUniformMatrix4x2fv");
    api->GetShaderPrecisionFormat = loadProc("glGetShaderPrecisionFormat");
    api->ProgramUniformMatrix2x3fv = loadProc("glProgramUniformMatrix2x3fv");
    api->TexSubImage3D = loadProc("glTexSubImage3D");
    api->GetProgramResourceLocationIndex = oc_glGetProgramResourceLocationIndex_noimpl;
    api->BlendFunc = loadProc("glBlendFunc");
    api->ProgramUniformMatrix3x4fv = loadProc("glProgramUniformMatrix3x4fv");
    api->Uniform3d = loadProc("glUniform3d");
    api->VertexAttrib1sv = loadProc("glVertexAttrib1sv");
    api->BindFragDataLocation = loadProc("glBindFragDataLocation");
    api->VertexAttrib4bv = loadProc("glVertexAttrib4bv");
    api->Uniform4iv = loadProc("glUniform4iv");
    api->ProgramUniform2ui = loadProc("glProgramUniform2ui");
    api->DrawArrays = loadProc("glDrawArrays");
    api->ProgramBinary = loadProc("glProgramBinary");
    api->VertexAttrib4f = loadProc("glVertexAttrib4f");
    api->VertexAttribP2uiv = loadProc("glVertexAttribP2uiv");
    api->UniformMatrix3fv = loadProc("glUniformMatrix3fv");
    api->Uniform2i = loadProc("glUniform2i");
    api->GetQueryObjectuiv = loadProc("glGetQueryObjectuiv");
    api->UniformBlockBinding = loadProc("glUniformBlockBinding");
    api->SampleCoverage = loadProc("glSampleCoverage");
    api->VertexAttrib4Nusv = loadProc("glVertexAttrib4Nusv");
    api->ProgramUniformMatrix2x4dv = loadProc("glProgramUniformMatrix2x4dv");
    api->Uniform3uiv = loadProc("glUniform3uiv");
    api->VertexAttrib1s = loadProc("glVertexAttrib1s");
    api->GetVertexAttribPointerv = loadProc("glGetVertexAttribPointerv");
    api->BlendBarrier = oc_glBlendBarrier_noimpl;
    api->DrawRangeElements = loadProc("glDrawRangeElements");
    api->TexStorage3D = oc_glTexStorage3D_noimpl;
    api->GetInternalformati64v = oc_glGetInternalformati64v_noimpl;
    api->GetQueryObjecti64v = loadProc("glGetQueryObjecti64v");
    api->CompressedTexSubImage1D = loadProc("glCompressedTexSubImage1D");
    api->VertexAttrib3dv = loadProc("glVertexAttrib3dv");
    api->VertexBindingDivisor = oc_glVertexBindingDivisor_noimpl;
    api->UseProgramStages = loadProc("glUseProgramStages");
    api->VertexAttribBinding = oc_glVertexAttribBinding_noimpl;
    api->DebugMessageInsert = oc_glDebugMessageInsert_noimpl;
    api->GetTexParameteriv = loadProc("glGetTexParameteriv");
    api->MultiDrawArraysIndirect = oc_glMultiDrawArraysIndirect_noimpl;
    api->GetTexParameterfv = loadProc("glGetTexParameterfv");
    api->GetProgramPipelineInfoLog = loadProc("glGetProgramPipelineInfoLog");
    api->EndQuery = loadProc("glEndQuery");
    api->GetProgramResourceLocation = oc_glGetProgramResourceLocation_noimpl;
    api->CompressedTexImage2D = loadProc("glCompressedTexImage2D");
    api->VertexAttribP2ui = loadProc("glVertexAttribP2ui");
    api->IsEnabledi = loadProc("glIsEnabledi");
    api->GetActiveAtomicCounterBufferiv = oc_glGetActiveAtomicCounterBufferiv_noimpl;
    api->IsProgram = loadProc("glIsProgram");
    api->Uniform1dv = loadProc("glUniform1dv");
    api->TexParameteriv = loadProc("glTexParameteriv");
    api->Uniform2fv = loadProc("glUniform2fv");
    api->ReleaseShaderCompiler = loadProc("glReleaseShaderCompiler");
    api->CullFace = loadProc("glCullFace");
    api->VertexAttribI4i = loadProc("glVertexAttribI4i");
    api->GetProgramResourceIndex = oc_glGetProgramResourceIndex_noimpl;
    api->ShaderBinary = loadProc("glShaderBinary");
    api->UniformMatrix3x2dv = loadProc("glUniformMatrix3x2dv");
    api->InvalidateFramebuffer = oc_glInvalidateFramebuffer_noimpl;
    api->AttachShader = loadProc("glAttachShader");
    api->FlushMappedBufferRange = loadProc("glFlushMappedBufferRange");
    api->VertexAttribP3uiv = loadProc("glVertexAttribP3uiv");
    api->GetActiveUniformName = loadProc("glGetActiveUniformName");
    api->MapBuffer = loadProc("glMapBuffer");
    api->DrawBuffers = loadProc("glDrawBuffers");
    api->GetSynciv = loadProc("glGetSynciv");
    api->CopyTexSubImage2D = loadProc("glCopyTexSubImage2D");
    api->ObjectLabel = oc_glObjectLabel_noimpl;
    api->BufferSubData = loadProc("glBufferSubData");
    api->Uniform2f = loadProc("glUniform2f");
    api->DebugMessageCallback = oc_glDebugMessageCallback_noimpl;
    api->VertexAttribL4dv = loadProc("glVertexAttribL4dv");
    api->IsProgramPipeline = loadProc("glIsProgramPipeline");
    api->ResumeTransformFeedback = loadProc("glResumeTransformFeedback");
    api->VertexAttribI4iv = loadProc("glVertexAttribI4iv");
    api->GetShaderInfoLog = loadProc("glGetShaderInfoLog");
    api->GetIntegeri_v = loadProc("glGetIntegeri_v");
    api->BindVertexBuffer = oc_glBindVertexBuffer_noimpl;
    api->BlendEquation = loadProc("glBlendEquation");
    api->VertexAttribL2dv = loadProc("glVertexAttribL2dv");
    api->VertexAttribI1ui = loadProc("glVertexAttribI1ui");
    api->VertexAttrib4Nsv = loadProc("glVertexAttrib4Nsv");
    api->VertexAttribL4d = loadProc("glVertexAttribL4d");
    api->CopyImageSubData = oc_glCopyImageSubData_noimpl;
    api->GetFramebufferAttachmentParameteriv = loadProc("glGetFramebufferAttachmentParameteriv");
    api->VertexAttribL2d = loadProc("glVertexAttribL2d");
    api->GetSubroutineIndex = loadProc("glGetSubroutineIndex");
    api->VertexAttribI3uiv = loadProc("glVertexAttribI3uiv");
    api->VertexAttrib4iv = loadProc("glVertexAttrib4iv");
    api->BindVertexBuffers = oc_glBindVertexBuffers_noimpl;
    api->ProgramUniformMatrix2x3dv = loadProc("glProgramUniformMatrix2x3dv");
    api->PrimitiveBoundingBox = oc_glPrimitiveBoundingBox_noimpl;
    api->Scissor = loadProc("glScissor");
    api->ClientWaitSync = loadProc("glClientWaitSync");
    api->Uniform3ui = loadProc("glUniform3ui");
    api->VertexAttribP3ui = loadProc("glVertexAttribP3ui");
    api->Enable = loadProc("glEnable");
    api->StencilOpSeparate = loadProc("glStencilOpSeparate");
    api->UniformMatrix2x3dv = loadProc("glUniformMatrix2x3dv");
    api->ProgramUniformMatrix3dv = loadProc("glProgramUniformMatrix3dv");
    api->TexImage2DMultisample = loadProc("glTexImage2DMultisample");
    api->VertexAttrib4Nbv = loadProc("glVertexAttrib4Nbv");
    api->GetTexImage = loadProc("glGetTexImage");
    api->VertexAttrib4sv = loadProc("glVertexAttrib4sv");
    api->PixelStorei = loadProc("glPixelStorei");
    api->DepthMask = loadProc("glDepthMask");
    api->TexStorage2D = oc_glTexStorage2D_noimpl;
    api->Clear = loadProc("glClear");
    api->UniformMatrix3x4dv = loadProc("glUniformMatrix3x4dv");
    api->DeleteTransformFeedbacks = loadProc("glDeleteTransformFeedbacks");
    api->MapBufferRange = loadProc("glMapBufferRange");
    api->MemoryBarrier = oc_glMemoryBarrier_noimpl;
    api->ViewportIndexedf = loadProc("glViewportIndexedf");
    api->VertexAttrib3fv = loadProc("glVertexAttrib3fv");
    api->ObjectPtrLabel = oc_glObjectPtrLabel_noimpl;
    api->TexStorage1D = oc_glTexStorage1D_noimpl;
    api->CompressedTexImage3D = loadProc("glCompressedTexImage3D");
    api->VertexAttrib1fv = loadProc("glVertexAttrib1fv");
    api->VertexAttribPointer = loadProc("glVertexAttribPointer");
    api->GetQueryIndexediv = loadProc("glGetQueryIndexediv");
    api->CompileShader = loadProc("glCompileShader");
    api->ProgramUniform1i = loadProc("glProgramUniform1i");
    api->GetQueryiv = loadProc("glGetQueryiv");
    api->VertexAttribI1iv = loadProc("glVertexAttribI1iv");
    api->CopyTexImage2D = loadProc("glCopyTexImage2D");
    api->GetQueryObjectui64v = loadProc("glGetQueryObjectui64v");
    api->PointSize = loadProc("glPointSize");
    api->Disablei = loadProc("glDisablei");
    api->VertexAttribL1dv = loadProc("glVertexAttribL1dv");
    api->CreateShader = loadProc("glCreateShader");
    api->GetString = loadProc("glGetString");
    api->ViewportArrayv = loadProc("glViewportArrayv");
    api->ProgramUniform3d = loadProc("glProgramUniform3d");
    api->VertexAttrib4Nubv = loadProc("glVertexAttrib4Nubv");
    api->TexParameteri = loadProc("glTexParameteri");
    api->ProgramUniform4fv = loadProc("glProgramUniform4fv");
    api->GenerateMipmap = loadProc("glGenerateMipmap");
    api->CompressedTexSubImage3D = loadProc("glCompressedTexSubImage3D");
    api->Uniform3f = loadProc("glUniform3f");
    api->GetUniformIndices = loadProc("glGetUniformIndices");
    api->VertexAttribLPointer = loadProc("glVertexAttribLPointer");
    api->VertexAttribI2uiv = loadProc("glVertexAttribI2uiv");
    api->QueryCounter = loadProc("glQueryCounter");
    api->ActiveShaderProgram = loadProc("glActiveShaderProgram");
    api->Uniform1ui = loadProc("glUniform1ui");
    api->VertexAttribI1i = loadProc("glVertexAttribI1i");
    api->GetTexParameterIiv = loadProc("glGetTexParameterIiv");
    api->GetUniformfv = loadProc("glGetUniformfv");
    api->ProgramUniform2uiv = loadProc("glProgramUniform2uiv");
    api->GetError = loadProc("glGetError");
    api->GetActiveUniformBlockName = loadProc("glGetActiveUniformBlockName");
    api->TextureView = oc_glTextureView_noimpl;
    api->GetnUniformiv = oc_glGetnUniformiv_noimpl;
    api->ProgramUniform4dv = loadProc("glProgramUniform4dv");
    api->ViewportIndexedfv = loadProc("glViewportIndexedfv");
    api->Hint = loadProc("glHint");
    api->GetShaderSource = loadProc("glGetShaderSource");
    api->ProgramUniformMatrix4x3fv = loadProc("glProgramUniformMatrix4x3fv");
    api->Uniform1iv = loadProc("glUniform1iv");
    api->VertexAttribI4bv = loadProc("glVertexAttribI4bv");
    api->UniformMatrix4x2dv = loadProc("glUniformMatrix4x2dv");
    api->BufferStorage = oc_glBufferStorage_noimpl;
    api->IsRenderbuffer = loadProc("glIsRenderbuffer");
    api->GetActiveSubroutineUniformName = loadProc("glGetActiveSubroutineUniformName");
    api->LinkProgram = loadProc("glLinkProgram");
    api->GetActiveUniformsiv = loadProc("glGetActiveUniformsiv");
    api->GetDebugMessageLog = oc_glGetDebugMessageLog_noimpl;
    api->CopyTexSubImage3D = loadProc("glCopyTexSubImage3D");
    api->PointParameteri = loadProc("glPointParameteri");
    api->ProgramUniform3dv = loadProc("glProgramUniform3dv");
    api->CompressedTexImage1D = loadProc("glCompressedTexImage1D");
    api->UniformMatrix3x4fv = loadProc("glUniformMatrix3x4fv");
    api->GenSamplers = loadProc("glGenSamplers");
    api->GetCompressedTexImage = loadProc("glGetCompressedTexImage");
    api->DeleteQueries = loadProc("glDeleteQueries");
    api->GenProgramPipelines = loadProc("glGenProgramPipelines");
    api->DispatchComputeIndirect = oc_glDispatchComputeIndirect_noimpl;
    api->VertexAttribIPointer = loadProc("glVertexAttribIPointer");
    api->CreateProgram = loadProc("glCreateProgram");
    api->ClearTexSubImage = oc_glClearTexSubImage_noimpl;
    api->VertexAttrib4d = loadProc("glVertexAttrib4d");
    api->FrontFace = loadProc("glFrontFace");
    api->BindTransformFeedback = loadProc("glBindTransformFeedback");
    api->GetProgramStageiv = loadProc("glGetProgramStageiv");
    api->SamplerParameterIiv = loadProc("glSamplerParameterIiv");
    api->GetInteger64v = loadProc("glGetInteger64v");
    api->CreateShaderProgramv = loadProc("glCreateShaderProgramv");
    api->BindBuffersRange = oc_glBindBuffersRange_noimpl;
    api->Uniform3fv = loadProc("glUniform3fv");
    api->ProgramUniformMatrix4fv = loadProc("glProgramUniformMatrix4fv");
    api->BindBuffersBase = oc_glBindBuffersBase_noimpl;
    api->ClearBufferfi = loadProc("glClearBufferfi");
    api->FramebufferTexture3D = loadProc("glFramebufferTexture3D");
    api->Disable = loadProc("glDisable");
    api->ProgramUniform1iv = loadProc("glProgramUniform1iv");
    api->VertexAttribI2iv = loadProc("glVertexAttribI2iv");
    api->DepthRangeIndexed = loadProc("glDepthRangeIndexed");
    api->PatchParameteri = loadProc("glPatchParameteri");
    api->GetUniformBlockIndex = loadProc("glGetUniformBlockIndex");
    api->MultiDrawArrays = loadProc("glMultiDrawArrays");
    api->VertexAttribI4ubv = loadProc("glVertexAttribI4ubv");
    api->BindBuffer = loadProc("glBindBuffer");
    api->VertexAttribI3i = loadProc("glVertexAttribI3i");
    api->GetDoublev = loadProc("glGetDoublev");
    api->DrawTransformFeedbackStream = loadProc("glDrawTransformFeedbackStream");
    api->VertexAttribI4uiv = loadProc("glVertexAttribI4uiv");
    api->RenderbufferStorageMultisample = loadProc("glRenderbufferStorageMultisample");
    api->VertexAttribL3dv = loadProc("glVertexAttribL3dv");
    api->StencilMaskSeparate = loadProc("glStencilMaskSeparate");
    api->ProgramUniform1d = loadProc("glProgramUniform1d");
    api->Viewport = loadProc("glViewport");
    api->VertexAttribP1ui = loadProc("glVertexAttribP1ui");
    api->VertexAttrib4dv = loadProc("glVertexAttrib4dv");
    api->GenQueries = loadProc("glGenQueries");
    api->TexParameterIiv = loadProc("glTexParameterIiv");
    api->ProgramUniform2d = loadProc("glProgramUniform2d");
    api->ProgramUniform1uiv = loadProc("glProgramUniform1uiv");
    api->VertexAttrib4Nub = loadProc("glVertexAttrib4Nub");
    api->IsVertexArray = loadProc("glIsVertexArray");
    api->ProgramUniform3f = loadProc("glProgramUniform3f");
    api->ProgramUniform3iv = loadProc("glProgramUniform3iv");
    api->GetProgramBinary = loadProc("glGetProgramBinary");
    api->BindRenderbuffer = loadProc("glBindRenderbuffer");
    api->BindFragDataLocationIndexed = loadProc("glBindFragDataLocationIndexed");
    api->GetSamplerParameterIiv = loadProc("glGetSamplerParameterIiv");
    api->VertexAttribDivisor = loadProc("glVertexAttribDivisor");
    api->ProgramUniformMatrix3x2dv = loadProc("glProgramUniformMatrix3x2dv");
    api->FramebufferParameteri = oc_glFramebufferParameteri_noimpl;
    api->GenTransformFeedbacks = loadProc("glGenTransformFeedbacks");
    api->DeleteSync = loadProc("glDeleteSync");
    api->ProgramUniform1ui = loadProc("glProgramUniform1ui");
    api->TexSubImage1D = loadProc("glTexSubImage1D");
    api->ClearDepthf = loadProc("glClearDepthf");
    api->ReadPixels = loadProc("glReadPixels");
    api->VertexAttribI2i = loadProc("glVertexAttribI2i");
    api->Finish = loadProc("glFinish");
    api->LineWidth = loadProc("glLineWidth");
    api->DeleteShader = loadProc("glDeleteShader");
    api->IsSampler = loadProc("glIsSampler");
    api->ProgramUniformMatrix4dv = loadProc("glProgramUniformMatrix4dv");
    api->TransformFeedbackVaryings = loadProc("glTransformFeedbackVaryings");
    api->BeginConditionalRender = loadProc("glBeginConditionalRender");
    api->BindSamplers = oc_glBindSamplers_noimpl;
    api->DeleteProgramPipelines = loadProc("glDeleteProgramPipelines");
    api->ColorMask = loadProc("glColorMask");
    api->TexParameterfv = loadProc("glTexParameterfv");
    api->PushDebugGroup = oc_glPushDebugGroup_noimpl;
    api->ClearBufferfv = loadProc("glClearBufferfv");
    api->IsEnabled = loadProc("glIsEnabled");
    api->VertexAttrib2f = loadProc("glVertexAttrib2f");
    api->ProgramUniform2f = loadProc("glProgramUniform2f");
    api->GetSamplerParameterIuiv = loadProc("glGetSamplerParameterIuiv");
    api->GetInteger64i_v = loadProc("glGetInteger64i_v");
    api->Uniform2dv = loadProc("glUniform2dv");
    api->GetBufferSubData = loadProc("glGetBufferSubData");
    api->MultiDrawElementsIndirect = oc_glMultiDrawElementsIndirect_noimpl;
    api->ProgramParameteri = loadProc("glProgramParameteri");
    api->VertexAttribP4ui = loadProc("glVertexAttribP4ui");
    api->SamplerParameterfv = loadProc("glSamplerParameterfv");
    api->PointParameterf = loadProc("glPointParameterf");
    api->UniformMatrix2x4fv = loadProc("glUniformMatrix2x4fv");
    api->GenBuffers = loadProc("glGenBuffers");
    api->ProgramUniform2dv = loadProc("glProgramUniform2dv");
    api->VertexAttribFormat = oc_glVertexAttribFormat_noimpl;
    api->TexSubImage2D = loadProc("glTexSubImage2D");
    api->VertexAttrib4ubv = loadProc("glVertexAttrib4ubv");
    api->GetGraphicsResetStatus = oc_glGetGraphicsResetStatus_noimpl;
    api->GetProgramInterfaceiv = oc_glGetProgramInterfaceiv_noimpl;
    api->VertexAttribIFormat = oc_glVertexAttribIFormat_noimpl;
    api->GetnUniformfv = oc_glGetnUniformfv_noimpl;
    api->DeleteProgram = loadProc("glDeleteProgram");
    api->ClampColor = loadProc("glClampColor");
    api->DrawElementsInstancedBaseVertexBaseInstance = oc_glDrawElementsInstancedBaseVertexBaseInstance_noimpl;
    api->DrawElements = loadProc("glDrawElements");
    api->DebugMessageControl = oc_glDebugMessageControl_noimpl;
    api->GetRenderbufferParameteriv = loadProc("glGetRenderbufferParameteriv");
    api->DetachShader = loadProc("glDetachShader");
    api->GenFramebuffers = loadProc("glGenFramebuffers");
    api->ProvokingVertex = loadProc("glProvokingVertex");
    api->SampleMaski = loadProc("glSampleMaski");
    api->EndQueryIndexed = loadProc("glEndQueryIndexed");
    api->ProgramUniform1f = loadProc("glProgramUniform1f");
    api->BindFramebuffer = loadProc("glBindFramebuffer");
    api->BeginQueryIndexed = loadProc("glBeginQueryIndexed");
    api->UniformSubroutinesuiv = loadProc("glUniformSubroutinesuiv");
    api->GetUniformiv = loadProc("glGetUniformiv");
    api->FramebufferTexture = loadProc("glFramebufferTexture");
    api->PointParameterfv = loadProc("glPointParameterfv");
    api->IsTransformFeedback = loadProc("glIsTransformFeedback");
    api->CheckFramebufferStatus = loadProc("glCheckFramebufferStatus");
    api->ShaderSource = loadProc("glShaderSource");
    api->UniformMatrix2x4dv = loadProc("glUniformMatrix2x4dv");
    api->BindImageTextures = oc_glBindImageTextures_noimpl;
    api->CopyTexImage1D = loadProc("glCopyTexImage1D");
    api->UniformMatrix3dv = loadProc("glUniformMatrix3dv");
    api->ProgramUniform1dv = loadProc("glProgramUniform1dv");
    api->BlitFramebuffer = loadProc("glBlitFramebuffer");
    api->PopDebugGroup = oc_glPopDebugGroup_noimpl;
    api->TexParameterIuiv = loadProc("glTexParameterIuiv");
    api->VertexAttrib2d = loadProc("glVertexAttrib2d");
    api->TexImage1D = loadProc("glTexImage1D");
    api->GetObjectPtrLabel = oc_glGetObjectPtrLabel_noimpl;
    api->StencilMask = loadProc("glStencilMask");
    api->BeginQuery = loadProc("glBeginQuery");
    api->UniformMatrix4fv = loadProc("glUniformMatrix4fv");
    api->IsSync = loadProc("glIsSync");
    api->Uniform3dv = loadProc("glUniform3dv");
    api->ProgramUniform2fv = loadProc("glProgramUniform2fv");
    api->VertexAttribI4sv = loadProc("glVertexAttribI4sv");
    api->ScissorArrayv = loadProc("glScissorArrayv");
    api->VertexAttribP1uiv = loadProc("glVertexAttribP1uiv");
    api->Uniform2uiv = loadProc("glUniform2uiv");
    api->DeleteBuffers = loadProc("glDeleteBuffers");
    api->ProgramUniform3ui = loadProc("glProgramUniform3ui");
    api->FramebufferTextureLayer = loadProc("glFramebufferTextureLayer");
    api->EndTransformFeedback = loadProc("glEndTransformFeedback");
    api->BlendFuncSeparatei = loadProc("glBlendFuncSeparatei");
    api->DrawTransformFeedbackInstanced = oc_glDrawTransformFeedbackInstanced_noimpl;
    api->DrawRangeElementsBaseVertex = loadProc("glDrawRangeElementsBaseVertex");
    api->VertexAttrib1f = loadProc("glVertexAttrib1f");
    api->GetUniformSubroutineuiv = loadProc("glGetUniformSubroutineuiv");
    api->DisableVertexAttribArray = loadProc("glDisableVertexAttribArray");
    api->ProgramUniformMatrix3x2fv = loadProc("glProgramUniformMatrix3x2fv");
    api->VertexAttribI4usv = loadProc("glVertexAttribI4usv");
    api->GetObjectLabel = oc_glGetObjectLabel_noimpl;
    api->BindAttribLocation = loadProc("glBindAttribLocation");
    api->Uniform1f = loadProc("glUniform1f");
    api->GetUniformdv = loadProc("glGetUniformdv");
    api->GetUniformLocation = loadProc("glGetUniformLocation");
    api->GetSubroutineUniformLocation = loadProc("glGetSubroutineUniformLocation");
    api->GetTexParameterIuiv = loadProc("glGetTexParameterIuiv");
    api->SamplerParameterf = loadProc("glSamplerParameterf");
    api->VertexAttribL3d = loadProc("glVertexAttribL3d");
    api->TexImage3DMultisample = loadProc("glTexImage3DMultisample");
    api->TexImage3D = loadProc("glTexImage3D");
    api->RenderbufferStorage = loadProc("glRenderbufferStorage");
    api->EnableVertexAttribArray = loadProc("glEnableVertexAttribArray");
    api->VertexAttribP4uiv = loadProc("glVertexAttribP4uiv");
    api->Uniform4d = loadProc("glUniform4d");
    api->VertexAttrib4s = loadProc("glVertexAttrib4s");
    api->DrawElementsInstancedBaseVertex = loadProc("glDrawElementsInstancedBaseVertex");
    api->VertexAttrib3s = loadProc("glVertexAttrib3s");
    api->ProgramUniform2iv = loadProc("glProgramUniform2iv");
    api->StencilFuncSeparate = loadProc("glStencilFuncSeparate");
    api->DeleteFramebuffers = loadProc("glDeleteFramebuffers");
    api->DepthRange = loadProc("glDepthRange");
    api->UniformMatrix3x2fv = loadProc("glUniformMatrix3x2fv");
    api->ProgramUniformMatrix2dv = loadProc("glProgramUniformMatrix2dv");
    api->ShaderStorageBlockBinding = oc_glShaderStorageBlockBinding_noimpl;
    api->ClearDepth = loadProc("glClearDepth");
    api->VertexAttrib2dv = loadProc("glVertexAttrib2dv");
    api->SamplerParameterIuiv = loadProc("glSamplerParameterIuiv");
    api->GetVertexAttribLdv = loadProc("glGetVertexAttribLdv");
    api->ProgramUniformMatrix3x4dv = loadProc("glProgramUniformMatrix3x4dv");
    api->DepthRangeArrayv = loadProc("glDepthRangeArrayv");
    api->GetActiveUniform = loadProc("glGetActiveUniform");
    api->PatchParameterfv = loadProc("glPatchParameterfv");
    api->InvalidateTexImage = oc_glInvalidateTexImage_noimpl;
    api->VertexAttrib3f = loadProc("glVertexAttrib3f");
    api->ProgramUniform4iv = loadProc("glProgramUniform4iv");
    api->ProgramUniform4d = loadProc("glProgramUniform4d");
    api->IsFramebuffer = loadProc("glIsFramebuffer");
    api->PixelStoref = loadProc("glPixelStoref");
    api->ProgramUniform4uiv = loadProc("glProgramUniform4uiv");
    api->ProgramUniformMatrix4x2dv = loadProc("glProgramUniformMatrix4x2dv");
    api->FenceSync = loadProc("glFenceSync");
    api->GetBufferParameteri64v = loadProc("glGetBufferParameteri64v");
    api->StencilOp = loadProc("glStencilOp");
    api->ClearBufferData = oc_glClearBufferData_noimpl;
    api->GetnUniformuiv = oc_glGetnUniformuiv_noimpl;
    api->GetProgramResourceiv = oc_glGetProgramResourceiv_noimpl;
    api->GetVertexAttribdv = loadProc("glGetVertexAttribdv");
    api->GetTransformFeedbackVarying = loadProc("glGetTransformFeedbackVarying");
    api->VertexAttrib2fv = loadProc("glVertexAttrib2fv");
    api->GetBooleani_v = loadProc("glGetBooleani_v");
    api->ColorMaski = loadProc("glColorMaski");
    api->InvalidateBufferSubData = oc_glInvalidateBufferSubData_noimpl;
    api->UniformMatrix4dv = loadProc("glUniformMatrix4dv");
    api->IsQuery = loadProc("glIsQuery");
    api->Uniform4ui = loadProc("glUniform4ui");
    api->Uniform4i = loadProc("glUniform4i");
    api->GetSamplerParameteriv = loadProc("glGetSamplerParameteriv");
    api->MultiDrawElementsBaseVertex = loadProc("glMultiDrawElementsBaseVertex");
    api->VertexAttribI1uiv = loadProc("glVertexAttribI1uiv");
    api->GetIntegerv = loadProc("glGetIntegerv");
    api->UniformMatrix2x3fv = loadProc("glUniformMatrix2x3fv");
    api->TexImage2D = loadProc("glTexImage2D");
    api->GetAttachedShaders = loadProc("glGetAttachedShaders");
    api->Uniform2d = loadProc("glUniform2d");
    api->MemoryBarrierByRegion = oc_glMemoryBarrierByRegion_noimpl;
    api->UniformMatrix2fv = loadProc("glUniformMatrix2fv");
    api->PrimitiveRestartIndex = loadProc("glPrimitiveRestartIndex");
    api->GetVertexAttribiv = loadProc("glGetVertexAttribiv");
    api->GetAttribLocation = loadProc("glGetAttribLocation");
    api->TexStorage2DMultisample = oc_glTexStorage2DMultisample_noimpl;
    api->CompressedTexSubImage2D = loadProc("glCompressedTexSubImage2D");
    api->GetVertexAttribfv = loadProc("glGetVertexAttribfv");
    api->GetBufferParameteriv = loadProc("glGetBufferParameteriv");
    api->TexParameterf = loadProc("glTexParameterf");
    api->FramebufferTexture2D = loadProc("glFramebufferTexture2D");
    api->GetActiveAttrib = loadProc("glGetActiveAttrib");
    api->InvalidateTexSubImage = oc_glInvalidateTexSubImage_noimpl;
    api->DeleteVertexArrays = loadProc("glDeleteVertexArrays");
    api->VertexAttribI2ui = loadProc("glVertexAttribI2ui");
    api->PointParameteriv = loadProc("glPointParameteriv");
    api->GetPointerv = oc_glGetPointerv_noimpl;
    api->Enablei = loadProc("glEnablei");
    api->BindBufferRange = loadProc("glBindBufferRange");
    api->DrawArraysInstanced = loadProc("glDrawArraysInstanced");
    api->DeleteTextures = loadProc("glDeleteTextures");
    api->VertexAttrib4Niv = loadProc("glVertexAttrib4Niv");
    api->MultiDrawElements = loadProc("glMultiDrawElements");
    api->GetProgramiv = loadProc("glGetProgramiv");
    api->DepthFunc = loadProc("glDepthFunc");
    api->GenTextures = loadProc("glGenTextures");
    api->GetInternalformativ = oc_glGetInternalformativ_noimpl;
    api->ProgramUniform3i = loadProc("glProgramUniform3i");
    api->ScissorIndexed = loadProc("glScissorIndexed");
    api->VertexAttrib2sv = loadProc("glVertexAttrib2sv");
    api->TexStorage3DMultisample = oc_glTexStorage3DMultisample_noimpl;
    api->Uniform2iv = loadProc("glUniform2iv");
    api->DrawArraysInstancedBaseInstance = oc_glDrawArraysInstancedBaseInstance_noimpl;
    api->VertexAttribI3ui = loadProc("glVertexAttribI3ui");
    api->DeleteSamplers = loadProc("glDeleteSamplers");
    api->GenVertexArrays = loadProc("glGenVertexArrays");
    api->GetFramebufferParameteriv = oc_glGetFramebufferParameteriv_noimpl;
    api->PolygonMode = loadProc("glPolygonMode");
    api->ProgramUniformMatrix2x4fv = loadProc("glProgramUniformMatrix2x4fv");
    api->GetProgramResourceName = oc_glGetProgramResourceName_noimpl;
    api->SamplerParameteriv = loadProc("glSamplerParameteriv");
    api->GetActiveSubroutineUniformiv = loadProc("glGetActiveSubroutineUniformiv");
    api->GetStringi = loadProc("glGetStringi");
    api->VertexAttribLFormat = oc_glVertexAttribLFormat_noimpl;
    api->VertexAttrib3d = loadProc("glVertexAttrib3d");
    api->BindVertexArray = loadProc("glBindVertexArray");
    api->UnmapBuffer = loadProc("glUnmapBuffer");
    api->DrawElementsInstancedBaseInstance = oc_glDrawElementsInstancedBaseInstance_noimpl;
    api->Uniform4uiv = loadProc("glUniform4uiv");
    api->FramebufferTexture1D = loadProc("glFramebufferTexture1D");
    api->DrawTransformFeedbackStreamInstanced = oc_glDrawTransformFeedbackStreamInstanced_noimpl;
    api->StencilFunc = loadProc("glStencilFunc");
    api->ValidateProgram = loadProc("glValidateProgram");
    api->Flush = loadProc("glFlush");
    api->ProgramUniform3uiv = loadProc("glProgramUniform3uiv");
    api->DeleteRenderbuffers = loadProc("glDeleteRenderbuffers");
    api->VertexAttrib4fv = loadProc("glVertexAttrib4fv");
    api->UniformMatrix2dv = loadProc("glUniformMatrix2dv");
    api->GetFragDataIndex = loadProc("glGetFragDataIndex");
    api->Uniform3iv = loadProc("glUniform3iv");
    api->MinSampleShading = loadProc("glMinSampleShading");
    api->GetBooleanv = loadProc("glGetBooleanv");
    api->GetMultisamplefv = loadProc("glGetMultisamplefv");
    api->GetVertexAttribIuiv = loadProc("glGetVertexAttribIuiv");
    api->GetProgramInfoLog = loadProc("glGetProgramInfoLog");
    api->Uniform4fv = loadProc("glUniform4fv");
    api->DrawBuffer = loadProc("glDrawBuffer");
    api->Uniform1i = loadProc("glUniform1i");
    api->ProgramUniform4ui = loadProc("glProgramUniform4ui");
    api->ProgramUniformMatrix3fv = loadProc("glProgramUniformMatrix3fv");
    api->BlendEquationSeparate = loadProc("glBlendEquationSeparate");
    api->BindProgramPipeline = loadProc("glBindProgramPipeline");
    api->GetDoublei_v = loadProc("glGetDoublei_v");
    api->BufferData = loadProc("glBufferData");
    api->ClearColor = loadProc("glClearColor");
    api->ProgramUniform4i = loadProc("glProgramUniform4i");
    api->GetTexLevelParameteriv = loadProc("glGetTexLevelParameteriv");
    api->GetActiveUniformBlockiv = loadProc("glGetActiveUniformBlockiv");
    api->ProgramUniform1fv = loadProc("glProgramUniform1fv");
    api->PauseTransformFeedback = loadProc("glPauseTransformFeedback");
    api->GetBufferPointerv = loadProc("glGetBufferPointerv");
    api->InvalidateSubFramebuffer = oc_glInvalidateSubFramebuffer_noimpl;
    api->ScissorIndexedv = loadProc("glScissorIndexedv");
    api->Uniform2ui = loadProc("glUniform2ui");
    api->BindTexture = loadProc("glBindTexture");
    api->DrawElementsInstanced = loadProc("glDrawElementsInstanced");
    api->ProgramUniform4f = loadProc("glProgramUniform4f");
    api->BindBufferBase = loadProc("glBindBufferBase");
    api->IsShader = loadProc("glIsShader");
    api->ClearBufferSubData = oc_glClearBufferSubData_noimpl;
    api->VertexAttrib4Nuiv = loadProc("glVertexAttrib4Nuiv");
    api->DrawArraysIndirect = loadProc("glDrawArraysIndirect");
    api->VertexAttrib4usv = loadProc("glVertexAttrib4usv");
    api->Uniform1d = loadProc("glUniform1d");
    api->ClearTexImage = oc_glClearTexImage_noimpl;
    api->Uniform1uiv = loadProc("glUniform1uiv");
    api->BindSampler = loadProc("glBindSampler");
    api->GetTexLevelParameterfv = loadProc("glGetTexLevelParameterfv");
    api->ClearBufferiv = loadProc("glClearBufferiv");
    api->LogicOp = loadProc("glLogicOp");
    api->ActiveTexture = loadProc("glActiveTexture");
    api->GetFragDataLocation = loadProc("glGetFragDataLocation");
    api->BlendColor = loadProc("glBlendColor");
    api->UniformMatrix4x3fv = loadProc("glUniformMatrix4x3fv");
    api->ProgramUniform3fv = loadProc("glProgramUniform3fv");
    api->Uniform1fv = loadProc("glUniform1fv");
    api->DrawElementsBaseVertex = loadProc("glDrawElementsBaseVertex");
    api->Uniform4f = loadProc("glUniform4f");
    api->BlendEquationSeparatei = loadProc("glBlendEquationSeparatei");
    api->BlendFuncSeparate = loadProc("glBlendFuncSeparate");
    api->ClearBufferuiv = loadProc("glClearBufferuiv");
    api->CopyTexSubImage1D = loadProc("glCopyTexSubImage1D");
    api->DrawTransformFeedback = loadProc("glDrawTransformFeedback");
    api->ReadBuffer = loadProc("glReadBuffer");
    api->CopyBufferSubData = loadProc("glCopyBufferSubData");
    api->GetUniformuiv = loadProc("glGetUniformuiv");
    api->PolygonOffset = loadProc("glPolygonOffset");
    api->DispatchCompute = oc_glDispatchCompute_noimpl;
    api->BindImageTexture = oc_glBindImageTexture_noimpl;
    api->UniformMatrix4x3dv = loadProc("glUniformMatrix4x3dv");
    api->GenRenderbuffers = loadProc("glGenRenderbuffers");
}

void oc_gl_load_gl43(oc_gl_api* api, oc_gl_load_proc loadProc)
{
    api->name = "gl43";
    api->GetFloatv = loadProc("glGetFloatv");
    api->TexBufferRange = loadProc("glTexBufferRange");
    api->IsBuffer = loadProc("glIsBuffer");
    api->IsTexture = loadProc("glIsTexture");
    api->DepthRangef = loadProc("glDepthRangef");
    api->EndConditionalRender = loadProc("glEndConditionalRender");
    api->BlendFunci = loadProc("glBlendFunci");
    api->GetProgramPipelineiv = loadProc("glGetProgramPipelineiv");
    api->WaitSync = loadProc("glWaitSync");
    api->ProgramUniformMatrix2fv = loadProc("glProgramUniformMatrix2fv");
    api->ProgramUniformMatrix4x3dv = loadProc("glProgramUniformMatrix4x3dv");
    api->VertexAttrib1dv = loadProc("glVertexAttrib1dv");
    api->SamplerParameteri = loadProc("glSamplerParameteri");
    api->GetVertexAttribIiv = loadProc("glGetVertexAttribIiv");
    api->GetSamplerParameterfv = loadProc("glGetSamplerParameterfv");
    api->VertexAttrib1d = loadProc("glVertexAttrib1d");
    api->TexBuffer = loadProc("glTexBuffer");
    api->InvalidateBufferData = loadProc("glInvalidateBufferData");
    api->ProgramUniform2i = loadProc("glProgramUniform2i");
    api->Uniform4dv = loadProc("glUniform4dv");
    api->UseProgram = loadProc("glUseProgram");
    api->VertexAttribI3iv = loadProc("glVertexAttribI3iv");
    api->DrawElementsIndirect = loadProc("glDrawElementsIndirect");
    api->VertexAttrib4uiv = loadProc("glVertexAttrib4uiv");
    api->GetQueryObjectiv = loadProc("glGetQueryObjectiv");
    api->FramebufferRenderbuffer = loadProc("glFramebufferRenderbuffer");
    api->BlendEquationi = loadProc("glBlendEquationi");
    api->GetActiveSubroutineName = loadProc("glGetActiveSubroutineName");
    api->VertexAttrib2s = loadProc("glVertexAttrib2s");
    api->VertexAttribL1d = loadProc("glVertexAttribL1d");
    api->BindTextures = oc_glBindTextures_noimpl;
    api->VertexAttrib3sv = loadProc("glVertexAttrib3sv");
    api->GetFloati_v = loadProc("glGetFloati_v");
    api->BeginTransformFeedback = loadProc("glBeginTransformFeedback");
    api->ClearStencil = loadProc("glClearStencil");
    api->Uniform3i = loadProc("glUniform3i");
    api->ValidateProgramPipeline = loadProc("glValidateProgramPipeline");
    api->ProgramUniformMatrix4x2fv = loadProc("glProgramUniformMatrix4x2fv");
    api->VertexAttribI4ui = loadProc("glVertexAttribI4ui");
    api->GetShaderiv = loadProc("glGetShaderiv");
    api->ReadnPixels = oc_glReadnPixels_noimpl;
    api->UniformMatrix4x2fv = loadProc("glUniformMatrix4x2fv");
    api->GetShaderPrecisionFormat = loadProc("glGetShaderPrecisionFormat");
    api->ProgramUniformMatrix2x3fv = loadProc("glProgramUniformMatrix2x3fv");
    api->TexSubImage3D = loadProc("glTexSubImage3D");
    api->GetProgramResourceLocationIndex = loadProc("glGetProgramResourceLocationIndex");
    api->BlendFunc = loadProc("glBlendFunc");
    api->ProgramUniformMatrix3x4fv = loadProc("glProgramUniformMatrix3x4fv");
    api->Uniform3d = loadProc("glUniform3d");
    api->VertexAttrib1sv = loadProc("glVertexAttrib1sv");
    api->BindFragDataLocation = loadProc("glBindFragDataLocation");
    api->VertexAttrib4bv = loadProc("glVertexAttrib4bv");
    api->Uniform4iv = loadProc("glUniform4iv");
    api->ProgramUniform2ui = loadProc("glProgramUniform2ui");
    api->DrawArrays = loadProc("glDrawArrays");
    api->ProgramBinary = loadProc("glProgramBinary");
    api->VertexAttrib4f = loadProc("glVertexAttrib4f");
    api->VertexAttribP2uiv = loadProc("glVertexAttribP2uiv");
    api->UniformMatrix3fv = loadProc("glUniformMatrix3fv");
    api->Uniform2i = loadProc("glUniform2i");
    api->GetQueryObjectuiv = loadProc("glGetQueryObjectuiv");
    api->UniformBlockBinding = loadProc("glUniformBlockBinding");
    api->SampleCoverage = loadProc("glSampleCoverage");
    api->VertexAttrib4Nusv = loadProc("glVertexAttrib4Nusv");
    api->ProgramUniformMatrix2x4dv = loadProc("glProgramUniformMatrix2x4dv");
    api->Uniform3uiv = loadProc("glUniform3uiv");
    api->VertexAttrib1s = loadProc("glVertexAttrib1s");
    api->GetVertexAttribPointerv = loadProc("glGetVertexAttribPointerv");
    api->BlendBarrier = oc_glBlendBarrier_noimpl;
    api->DrawRangeElements = loadProc("glDrawRangeElements");
    api->TexStorage3D = loadProc("glTexStorage3D");
    api->GetInternalformati64v = loadProc("glGetInternalformati64v");
    api->GetQueryObjecti64v = loadProc("glGetQueryObjecti64v");
    api->CompressedTexSubImage1D = loadProc("glCompressedTexSubImage1D");
    api->VertexAttrib3dv = loadProc("glVertexAttrib3dv");
    api->VertexBindingDivisor = loadProc("glVertexBindingDivisor");
    api->UseProgramStages = loadProc("glUseProgramStages");
    api->VertexAttribBinding = loadProc("glVertexAttribBinding");
    api->DebugMessageInsert = loadProc("glDebugMessageInsert");
    api->GetTexParameteriv = loadProc("glGetTexParameteriv");
    api->MultiDrawArraysIndirect = loadProc("glMultiDrawArraysIndirect");
    api->GetTexParameterfv = loadProc("glGetTexParameterfv");
    api->GetProgramPipelineInfoLog = loadProc("glGetProgramPipelineInfoLog");
    api->EndQuery = loadProc("glEndQuery");
    api->GetProgramResourceLocation = loadProc("glGetProgramResourceLocation");
    api->CompressedTexImage2D = loadProc("glCompressedTexImage2D");
    api->VertexAttribP2ui = loadProc("glVertexAttribP2ui");
    api->IsEnabledi = loadProc("glIsEnabledi");
    api->GetActiveAtomicCounterBufferiv = loadProc("glGetActiveAtomicCounterBufferiv");
    api->IsProgram = loadProc("glIsProgram");
    api->Uniform1dv = loadProc("glUniform1dv");
    api->TexParameteriv = loadProc("glTexParameteriv");
    api->Uniform2fv = loadProc("glUniform2fv");
    api->ReleaseShaderCompiler = loadProc("glReleaseShaderCompiler");
    api->CullFace = loadProc("glCullFace");
    api->VertexAttribI4i = loadProc("glVertexAttribI4i");
    api->GetProgramResourceIndex = loadProc("glGetProgramResourceIndex");
    api->ShaderBinary = loadProc("glShaderBinary");
    api->UniformMatrix3x2dv = loadProc("glUniformMatrix3x2dv");
    api->InvalidateFramebuffer = loadProc("glInvalidateFramebuffer");
    api->AttachShader = loadProc("glAttachShader");
    api->FlushMappedBufferRange = loadProc("glFlushMappedBufferRange");
    api->VertexAttribP3uiv = loadProc("glVertexAttribP3uiv");
    api->GetActiveUniformName = loadProc("glGetActiveUniformName");
    api->MapBuffer = loadProc("glMapBuffer");
    api->DrawBuffers = loadProc("glDrawBuffers");
    api->GetSynciv = loadProc("glGetSynciv");
    api->CopyTexSubImage2D = loadProc("glCopyTexSubImage2D");
    api->ObjectLabel = loadProc("glObjectLabel");
    api->BufferSubData = loadProc("glBufferSubData");
    api->Uniform2f = loadProc("glUniform2f");
    api->DebugMessageCallback = loadProc("glDebugMessageCallback");
    api->VertexAttribL4dv = loadProc("glVertexAttribL4dv");
    api->IsProgramPipeline = loadProc("glIsProgramPipeline");
    api->ResumeTransformFeedback = loadProc("glResumeTransformFeedback");
    api->VertexAttribI4iv = loadProc("glVertexAttribI4iv");
    api->GetShaderInfoLog = loadProc("glGetShaderInfoLog");
    api->GetIntegeri_v = loadProc("glGetIntegeri_v");
    api->BindVertexBuffer = loadProc("glBindVertexBuffer");
    api->BlendEquation = loadProc("glBlendEquation");
    api->VertexAttribL2dv = loadProc("glVertexAttribL2dv");
    api->VertexAttribI1ui = loadProc("glVertexAttribI1ui");
    api->VertexAttrib4Nsv = loadProc("glVertexAttrib4Nsv");
    api->VertexAttribL4d = loadProc("glVertexAttribL4d");
    api->CopyImageSubData = loadProc("glCopyImageSubData");
    api->GetFramebufferAttachmentParameteriv = loadProc("glGetFramebufferAttachmentParameteriv");
    api->VertexAttribL2d = loadProc("glVertexAttribL2d");
    api->GetSubroutineIndex = loadProc("glGetSubroutineIndex");
    api->VertexAttribI3uiv = loadProc("glVertexAttribI3uiv");
    api->VertexAttrib4iv = loadProc("glVertexAttrib4iv");
    api->BindVertexBuffers = oc_glBindVertexBuffers_noimpl;
    api->ProgramUniformMatrix2x3dv = loadProc("glProgramUniformMatrix2x3dv");
    api->PrimitiveBoundingBox = oc_glPrimitiveBoundingBox_noimpl;
    api->Scissor = loadProc("glScissor");
    api->ClientWaitSync = loadProc("glClientWaitSync");
    api->Uniform3ui = loadProc("glUniform3ui");
    api->VertexAttribP3ui = loadProc("glVertexAttribP3ui");
    api->Enable = loadProc("glEnable");
    api->StencilOpSeparate = loadProc("glStencilOpSeparate");
    api->UniformMatrix2x3dv = loadProc("glUniformMatrix2x3dv");
    api->ProgramUniformMatrix3dv = loadProc("glProgramUniformMatrix3dv");
    api->TexImage2DMultisample = loadProc("glTexImage2DMultisample");
    api->VertexAttrib4Nbv = loadProc("glVertexAttrib4Nbv");
    api->GetTexImage = loadProc("glGetTexImage");
    api->VertexAttrib4sv = loadProc("glVertexAttrib4sv");
    api->PixelStorei = loadProc("glPixelStorei");
    api->DepthMask = loadProc("glDepthMask");
    api->TexStorage2D = loadProc("glTexStorage2D");
    api->Clear = loadProc("glClear");
    api->UniformMatrix3x4dv = loadProc("glUniformMatrix3x4dv");
    api->DeleteTransformFeedbacks = loadProc("glDeleteTransformFeedbacks");
    api->MapBufferRange = loadProc("glMapBufferRange");
    api->MemoryBarrier = loadProc("glMemoryBarrier");
    api->ViewportIndexedf = loadProc("glViewportIndexedf");
    api->VertexAttrib3fv = loadProc("glVertexAttrib3fv");
    api->ObjectPtrLabel = loadProc("glObjectPtrLabel");
    api->TexStorage1D = loadProc("glTexStorage1D");
    api->CompressedTexImage3D = loadProc("glCompressedTexImage3D");
    api->VertexAttrib1fv = loadProc("glVertexAttrib1fv");
    api->VertexAttribPointer = loadProc("glVertexAttribPointer");
    api->GetQueryIndexediv = loadProc("glGetQueryIndexediv");
    api->CompileShader = loadProc("glCompileShader");
    api->ProgramUniform1i = loadProc("glProgramUniform1i");
    api->GetQueryiv = loadProc("glGetQueryiv");
    api->VertexAttribI1iv = loadProc("glVertexAttribI1iv");
    api->CopyTexImage2D = loadProc("glCopyTexImage2D");
    api->GetQueryObjectui64v = loadProc("glGetQueryObjectui64v");
    api->PointSize = loadProc("glPointSize");
    api->Disablei = loadProc("glDisablei");
    api->VertexAttribL1dv = loadProc("glVertexAttribL1dv");
    api->CreateShader = loadProc("glCreateShader");
    api->GetString = loadProc("glGetString");
    api->ViewportArrayv = loadProc("glViewportArrayv");
    api->ProgramUniform3d = loadProc("glProgramUniform3d");
    api->VertexAttrib4Nubv = loadProc("glVertexAttrib4Nubv");
    api->TexParameteri = loadProc("glTexParameteri");
    api->ProgramUniform4fv = loadProc("glProgramUniform4fv");
    api->GenerateMipmap = loadProc("glGenerateMipmap");
    api->CompressedTexSubImage3D = loadProc("glCompressedTexSubImage3D");
    api->Uniform3f = loadProc("glUniform3f");
    api->GetUniformIndices = loadProc("glGetUniformIndices");
    api->VertexAttribLPointer = loadProc("glVertexAttribLPointer");
    api->VertexAttribI2uiv = loadProc("glVertexAttribI2uiv");
    api->QueryCounter = loadProc("glQueryCounter");
    api->ActiveShaderProgram = loadProc("glActiveShaderProgram");
    api->Uniform1ui = loadProc("glUniform1ui");
    api->VertexAttribI1i = loadProc("glVertexAttribI1i");
    api->GetTexParameterIiv = loadProc("glGetTexParameterIiv");
    api->GetUniformfv = loadProc("glGetUniformfv");
    api->ProgramUniform2uiv = loadProc("glProgramUniform2uiv");
    api->GetError = loadProc("glGetError");
    api->GetActiveUniformBlockName = loadProc("glGetActiveUniformBlockName");
    api->TextureView = loadProc("glTextureView");
    api->GetnUniformiv = oc_glGetnUniformiv_noimpl;
    api->ProgramUniform4dv = loadProc("glProgramUniform4dv");
    api->ViewportIndexedfv = loadProc("glViewportIndexedfv");
    api->Hint = loadProc("glHint");
    api->GetShaderSource = loadProc("glGetShaderSource");
    api->ProgramUniformMatrix4x3fv = loadProc("glProgramUniformMatrix4x3fv");
    api->Uniform1iv = loadProc("glUniform1iv");
    api->VertexAttribI4bv = loadProc("glVertexAttribI4bv");
    api->UniformMatrix4x2dv = loadProc("glUniformMatrix4x2dv");
    api->BufferStorage = oc_glBufferStorage_noimpl;
    api->IsRenderbuffer = loadProc("glIsRenderbuffer");
    api->GetActiveSubroutineUniformName = loadProc("glGetActiveSubroutineUniformName");
    api->LinkProgram = loadProc("glLinkProgram");
    api->GetActiveUniformsiv = loadProc("glGetActiveUniformsiv");
    api->GetDebugMessageLog = loadProc("glGetDebugMessageLog");
    api->CopyTexSubImage3D = loadProc("glCopyTexSubImage3D");
    api->PointParameteri = loadProc("glPointParameteri");
    api->ProgramUniform3dv = loadProc("glProgramUniform3dv");
    api->CompressedTexImage1D = loadProc("glCompressedTexImage1D");
    api->UniformMatrix3x4fv = loadProc("glUniformMatrix3x4fv");
    api->GenSamplers = loadProc("glGenSamplers");
    api->GetCompressedTexImage = loadProc("glGetCompressedTexImage");
    api->DeleteQueries = loadProc("glDeleteQueries");
    api->GenProgramPipelines = loadProc("glGenProgramPipelines");
    api->DispatchComputeIndirect = loadProc("glDispatchComputeIndirect");
    api->VertexAttribIPointer = loadProc("glVertexAttribIPointer");
    api->CreateProgram = loadProc("glCreateProgram");
    api->ClearTexSubImage = oc_glClearTexSubImage_noimpl;
    api->VertexAttrib4d = loadProc("glVertexAttrib4d");
    api->FrontFace = loadProc("glFrontFace");
    api->BindTransformFeedback = loadProc("glBindTransformFeedback");
    api->GetProgramStageiv = loadProc("glGetProgramStageiv");
    api->SamplerParameterIiv = loadProc("glSamplerParameterIiv");
    api->GetInteger64v = loadProc("glGetInteger64v");
    api->CreateShaderProgramv = loadProc("glCreateShaderProgramv");
    api->BindBuffersRange = oc_glBindBuffersRange_noimpl;
    api->Uniform3fv = loadProc("glUniform3fv");
    api->ProgramUniformMatrix4fv = loadProc("glProgramUniformMatrix4fv");
    api->BindBuffersBase = oc_glBindBuffersBase_noimpl;
    api->ClearBufferfi = loadProc("glClearBufferfi");
    api->FramebufferTexture3D = loadProc("glFramebufferTexture3D");
    api->Disable = loadProc("glDisable");
    api->ProgramUniform1iv = loadProc("glProgramUniform1iv");
    api->VertexAttribI2iv = loadProc("glVertexAttribI2iv");
    api->DepthRangeIndexed = loadProc("glDepthRangeIndexed");
    api->PatchParameteri = loadProc("glPatchParameteri");
    api->GetUniformBlockIndex = loadProc("glGetUniformBlockIndex");
    api->MultiDrawArrays = loadProc("glMultiDrawArrays");
    api->VertexAttribI4ubv = loadProc("glVertexAttribI4ubv");
    api->BindBuffer = loadProc("glBindBuffer");
    api->VertexAttribI3i = loadProc("glVertexAttribI3i");
    api->GetDoublev = loadProc("glGetDoublev");
    api->DrawTransformFeedbackStream = loadProc("glDrawTransformFeedbackStream");
    api->VertexAttribI4uiv = loadProc("glVertexAttribI4uiv");
    api->RenderbufferStorageMultisample = loadProc("glRenderbufferStorageMultisample");
    api->VertexAttribL3dv = loadProc("glVertexAttribL3dv");
    api->StencilMaskSeparate = loadProc("glStencilMaskSeparate");
    api->ProgramUniform1d = loadProc("glProgramUniform1d");
    api->Viewport = loadProc("glViewport");
    api->VertexAttribP1ui = loadProc("glVertexAttribP1ui");
    api->VertexAttrib4dv = loadProc("glVertexAttrib4dv");
    api->GenQueries = loadProc("glGenQueries");
    api->TexParameterIiv = loadProc("glTexParameterIiv");
    api->ProgramUniform2d = loadProc("glProgramUniform2d");
    api->ProgramUniform1uiv = loadProc("glProgramUniform1uiv");
    api->VertexAttrib4Nub = loadProc("glVertexAttrib4Nub");
    api->IsVertexArray = loadProc("glIsVertexArray");
    api->ProgramUniform3f = loadProc("glProgramUniform3f");
    api->ProgramUniform3iv = loadProc("glProgramUniform3iv");
    api->GetProgramBinary = loadProc("glGetProgramBinary");
    api->BindRenderbuffer = loadProc("glBindRenderbuffer");
    api->BindFragDataLocationIndexed = loadProc("glBindFragDataLocationIndexed");
    api->GetSamplerParameterIiv = loadProc("glGetSamplerParameterIiv");
    api->VertexAttribDivisor = loadProc("glVertexAttribDivisor");
    api->ProgramUniformMatrix3x2dv = loadProc("glProgramUniformMatrix3x2dv");
    api->FramebufferParameteri = loadProc("glFramebufferParameteri");
    api->GenTransformFeedbacks = loadProc("glGenTransformFeedbacks");
    api->DeleteSync = loadProc("glDeleteSync");
    api->ProgramUniform1ui = loadProc("glProgramUniform1ui");
    api->TexSubImage1D = loadProc("glTexSubImage1D");
    api->ClearDepthf = loadProc("glClearDepthf");
    api->ReadPixels = loadProc("glReadPixels");
    api->VertexAttribI2i = loadProc("glVertexAttribI2i");
    api->Finish = loadProc("glFinish");
    api->LineWidth = loadProc("glLineWidth");
    api->DeleteShader = loadProc("glDeleteShader");
    api->IsSampler = loadProc("glIsSampler");
    api->ProgramUniformMatrix4dv = loadProc("glProgramUniformMatrix4dv");
    api->TransformFeedbackVaryings = loadProc("glTransformFeedbackVaryings");
    api->BeginConditionalRender = loadProc("glBeginConditionalRender");
    api->BindSamplers = oc_glBindSamplers_noimpl;
    api->DeleteProgramPipelines = loadProc("glDeleteProgramPipelines");
    api->ColorMask = loadProc("glColorMask");
    api->TexParameterfv = loadProc("glTexParameterfv");
    api->PushDebugGroup = loadProc("glPushDebugGroup");
    api->ClearBufferfv = loadProc("glClearBufferfv");
    api->IsEnabled = loadProc("glIsEnabled");
    api->VertexAttrib2f = loadProc("glVertexAttrib2f");
    api->ProgramUniform2f = loadProc("glProgramUniform2f");
    api->GetSamplerParameterIuiv = loadProc("glGetSamplerParameterIuiv");
    api->GetInteger64i_v = loadProc("glGetInteger64i_v");
    api->Uniform2dv = loadProc("glUniform2dv");
    api->GetBufferSubData = loadProc("glGetBufferSubData");
    api->MultiDrawElementsIndirect = loadProc("glMultiDrawElementsIndirect");
    api->ProgramParameteri = loadProc("glProgramParameteri");
    api->VertexAttribP4ui = loadProc("glVertexAttribP4ui");
    api->SamplerParameterfv = loadProc("glSamplerParameterfv");
    api->PointParameterf = loadProc("glPointParameterf");
    api->UniformMatrix2x4fv = loadProc("glUniformMatrix2x4fv");
    api->GenBuffers = loadProc("glGenBuffers");
    api->ProgramUniform2dv = loadProc("glProgramUniform2dv");
    api->VertexAttribFormat = loadProc("glVertexAttribFormat");
    api->TexSubImage2D = loadProc("glTexSubImage2D");
    api->VertexAttrib4ubv = loadProc("glVertexAttrib4ubv");
    api->GetGraphicsResetStatus = oc_glGetGraphicsResetStatus_noimpl;
    api->GetProgramInterfaceiv = loadProc("glGetProgramInterfaceiv");
    api->VertexAttribIFormat = loadProc("glVertexAttribIFormat");
    api->GetnUniformfv = oc_glGetnUniformfv_noimpl;
    api->DeleteProgram = loadProc("glDeleteProgram");
    api->ClampColor = loadProc("glClampColor");
    api->DrawElementsInstancedBaseVertexBaseInstance = loadProc("glDrawElementsInstancedBaseVertexBaseInstance");
    api->DrawElements = loadProc("glDrawElements");
    api->DebugMessageControl = loadProc("glDebugMessageControl");
    api->GetRenderbufferParameteriv = loadProc("glGetRenderbufferParameteriv");
    api->DetachShader = loadProc("glDetachShader");
    api->GenFramebuffers = loadProc("glGenFramebuffers");
    api->ProvokingVertex = loadProc("glProvokingVertex");
    api->SampleMaski = loadProc("glSampleMaski");
    api->EndQueryIndexed = loadProc("glEndQueryIndexed");
    api->ProgramUniform1f = loadProc("glProgramUniform1f");
    api->BindFramebuffer = loadProc("glBindFramebuffer");
    api->BeginQueryIndexed = loadProc("glBeginQueryIndexed");
    api->UniformSubroutinesuiv = loadProc("glUniformSubroutinesuiv");
    api->GetUniformiv = loadProc("glGetUniformiv");
    api->FramebufferTexture = loadProc("glFramebufferTexture");
    api->PointParameterfv = loadProc("glPointParameterfv");
    api->IsTransformFeedback = loadProc("glIsTransformFeedback");
    api->CheckFramebufferStatus = loadProc("glCheckFramebufferStatus");
    api->ShaderSource = loadProc("glShaderSource");
    api->UniformMatrix2x4dv = loadProc("glUniformMatrix2x4dv");
    api->BindImageTextures = oc_glBindImageTextures_noimpl;
    api->CopyTexImage1D = loadProc("glCopyTexImage1D");
    api->UniformMatrix3dv = loadProc("glUniformMatrix3dv");
    api->ProgramUniform1dv = loadProc("glProgramUniform1dv");
    api->BlitFramebuffer = loadProc("glBlitFramebuffer");
    api->PopDebugGroup = loadProc("glPopDebugGroup");
    api->TexParameterIuiv = loadProc("glTexParameterIuiv");
    api->VertexAttrib2d = loadProc("glVertexAttrib2d");
    api->TexImage1D = loadProc("glTexImage1D");
    api->GetObjectPtrLabel = loadProc("glGetObjectPtrLabel");
    api->StencilMask = loadProc("glStencilMask");
    api->BeginQuery = loadProc("glBeginQuery");
    api->UniformMatrix4fv = loadProc("glUniformMatrix4fv");
    api->IsSync = loadProc("glIsSync");
    api->Uniform3dv = loadProc("glUniform3dv");
    api->ProgramUniform2fv = loadProc("glProgramUniform2fv");
    api->VertexAttribI4sv = loadProc("glVertexAttribI4sv");
    api->ScissorArrayv = loadProc("glScissorArrayv");
    api->VertexAttribP1uiv = loadProc("glVertexAttribP1uiv");
    api->Uniform2uiv = loadProc("glUniform2uiv");
    api->DeleteBuffers = loadProc("glDeleteBuffers");
    api->ProgramUniform3ui = loadProc("glProgramUniform3ui");
    api->FramebufferTextureLayer = loadProc("glFramebufferTextureLayer");
    api->EndTransformFeedback = loadProc("glEndTransformFeedback");
    api->BlendFuncSeparatei = loadProc("glBlendFuncSeparatei");
    api->DrawTransformFeedbackInstanced = loadProc("glDrawTransformFeedbackInstanced");
    api->DrawRangeElementsBaseVertex = loadProc("glDrawRangeElementsBaseVertex");
    api->VertexAttrib1f = loadProc("glVertexAttrib1f");
    api->GetUniformSubroutineuiv = loadProc("glGetUniformSubroutineuiv");
    api->DisableVertexAttribArray = loadProc("glDisableVertexAttribArray");
    api->ProgramUniformMatrix3x2fv = loadProc("glProgramUniformMatrix3x2fv");
    api->VertexAttribI4usv = loadProc("glVertexAttribI4usv");
    api->GetObjectLabel = loadProc("glGetObjectLabel");
    api->BindAttribLocation = loadProc("glBindAttribLocation");
    api->Uniform1f = loadProc("glUniform1f");
    api->GetUniformdv = loadProc("glGetUniformdv");
    api->GetUniformLocation = loadProc("glGetUniformLocation");
    api->GetSubroutineUniformLocation = loadProc("glGetSubroutineUniformLocation");
    api->GetTexParameterIuiv = loadProc("glGetTexParameterIuiv");
    api->SamplerParameterf = loadProc("glSamplerParameterf");
    api->VertexAttribL3d = loadProc("glVertexAttribL3d");
    api->TexImage3DMultisample = loadProc("glTexImage3DMultisample");
    api->TexImage3D = loadProc("glTexImage3D");
    api->RenderbufferStorage = loadProc("glRenderbufferStorage");
    api->EnableVertexAttribArray = loadProc("glEnableVertexAttribArray");
    api->VertexAttribP4uiv = loadProc("glVertexAttribP4uiv");
    api->Uniform4d = loadProc("glUniform4d");
    api->VertexAttrib4s = loadProc("glVertexAttrib4s");
    api->DrawElementsInstancedBaseVertex = loadProc("glDrawElementsInstancedBaseVertex");
    api->VertexAttrib3s = loadProc("glVertexAttrib3s");
    api->ProgramUniform2iv = loadProc("glProgramUniform2iv");
    api->StencilFuncSeparate = loadProc("glStencilFuncSeparate");
    api->DeleteFramebuffers = loadProc("glDeleteFramebuffers");
    api->DepthRange = loadProc("glDepthRange");
    api->UniformMatrix3x2fv = loadProc("glUniformMatrix3x2fv");
    api->ProgramUniformMatrix2dv = loadProc("glProgramUniformMatrix2dv");
    api->ShaderStorageBlockBinding = loadProc("glShaderStorageBlockBinding");
    api->ClearDepth = loadProc("glClearDepth");
    api->VertexAttrib2dv = loadProc("glVertexAttrib2dv");
    api->SamplerParameterIuiv = loadProc("glSamplerParameterIuiv");
    api->GetVertexAttribLdv = loadProc("glGetVertexAttribLdv");
    api->ProgramUniformMatrix3x4dv = loadProc("glProgramUniformMatrix3x4dv");
    api->DepthRangeArrayv = loadProc("glDepthRangeArrayv");
    api->GetActiveUniform = loadProc("glGetActiveUniform");
    api->PatchParameterfv = loadProc("glPatchParameterfv");
    api->InvalidateTexImage = loadProc("glInvalidateTexImage");
    api->VertexAttrib3f = loadProc("glVertexAttrib3f");
    api->ProgramUniform4iv = loadProc("glProgramUniform4iv");
    api->ProgramUniform4d = loadProc("glProgramUniform4d");
    api->IsFramebuffer = loadProc("glIsFramebuffer");
    api->PixelStoref = loadProc("glPixelStoref");
    api->ProgramUniform4uiv = loadProc("glProgramUniform4uiv");
    api->ProgramUniformMatrix4x2dv = loadProc("glProgramUniformMatrix4x2dv");
    api->FenceSync = loadProc("glFenceSync");
    api->GetBufferParameteri64v = loadProc("glGetBufferParameteri64v");
    api->StencilOp = loadProc("glStencilOp");
    api->ClearBufferData = loadProc("glClearBufferData");
    api->GetnUniformuiv = oc_glGetnUniformuiv_noimpl;
    api->GetProgramResourceiv = loadProc("glGetProgramResourceiv");
    api->GetVertexAttribdv = loadProc("glGetVertexAttribdv");
    api->GetTransformFeedbackVarying = loadProc("glGetTransformFeedbackVarying");
    api->VertexAttrib2fv = loadProc("glVertexAttrib2fv");
    api->GetBooleani_v = loadProc("glGetBooleani_v");
    api->ColorMaski = loadProc("glColorMaski");
    api->InvalidateBufferSubData = loadProc("glInvalidateBufferSubData");
    api->UniformMatrix4dv = loadProc("glUniformMatrix4dv");
    api->IsQuery = loadProc("glIsQuery");
    api->Uniform4ui = loadProc("glUniform4ui");
    api->Uniform4i = loadProc("glUniform4i");
    api->GetSamplerParameteriv = loadProc("glGetSamplerParameteriv");
    api->MultiDrawElementsBaseVertex = loadProc("glMultiDrawElementsBaseVertex");
    api->VertexAttribI1uiv = loadProc("glVertexAttribI1uiv");
    api->GetIntegerv = loadProc("glGetIntegerv");
    api->UniformMatrix2x3fv = loadProc("glUniformMatrix2x3fv");
    api->TexImage2D = loadProc("glTexImage2D");
    api->GetAttachedShaders = loadProc("glGetAttachedShaders");
    api->Uniform2d = loadProc("glUniform2d");
    api->MemoryBarrierByRegion = oc_glMemoryBarrierByRegion_noimpl;
    api->UniformMatrix2fv = loadProc("glUniformMatrix2fv");
    api->PrimitiveRestartIndex = loadProc("glPrimitiveRestartIndex");
    api->GetVertexAttribiv = loadProc("glGetVertexAttribiv");
    api->GetAttribLocation = loadProc("glGetAttribLocation");
    api->TexStorage2DMultisample = loadProc("glTexStorage2DMultisample");
    api->CompressedTexSubImage2D = loadProc("glCompressedTexSubImage2D");
    api->GetVertexAttribfv = loadProc("glGetVertexAttribfv");
    api->GetBufferParameteriv = loadProc("glGetBufferParameteriv");
    api->TexParameterf = loadProc("glTexParameterf");
    api->FramebufferTexture2D = loadProc("glFramebufferTexture2D");
    api->GetActiveAttrib = loadProc("glGetActiveAttrib");
    api->InvalidateTexSubImage = loadProc("glInvalidateTexSubImage");
    api->DeleteVertexArrays = loadProc("glDeleteVertexArrays");
    api->VertexAttribI2ui = loadProc("glVertexAttribI2ui");
    api->PointParameteriv = loadProc("glPointParameteriv");
    api->GetPointerv = loadProc("glGetPointerv");
    api->Enablei = loadProc("glEnablei");
    api->BindBufferRange = loadProc("glBindBufferRange");
    api->DrawArraysInstanced = loadProc("glDrawArraysInstanced");
    api->DeleteTextures = loadProc("glDeleteTextures");
    api->VertexAttrib4Niv = loadProc("glVertexAttrib4Niv");
    api->MultiDrawElements = loadProc("glMultiDrawElements");
    api->GetProgramiv = loadProc("glGetProgramiv");
    api->DepthFunc = loadProc("glDepthFunc");
    api->GenTextures = loadProc("glGenTextures");
    api->GetInternalformativ = loadProc("glGetInternalformativ");
    api->ProgramUniform3i = loadProc("glProgramUniform3i");
    api->ScissorIndexed = loadProc("glScissorIndexed");
    api->VertexAttrib2sv = loadProc("glVertexAttrib2sv");
    api->TexStorage3DMultisample = loadProc("glTexStorage3DMultisample");
    api->Uniform2iv = loadProc("glUniform2iv");
    api->DrawArraysInstancedBaseInstance = loadProc("glDrawArraysInstancedBaseInstance");
    api->VertexAttribI3ui = loadProc("glVertexAttribI3ui");
    api->DeleteSamplers = loadProc("glDeleteSamplers");
    api->GenVertexArrays = loadProc("glGenVertexArrays");
    api->GetFramebufferParameteriv = loadProc("glGetFramebufferParameteriv");
    api->PolygonMode = loadProc("glPolygonMode");
    api->ProgramUniformMatrix2x4fv = loadProc("glProgramUniformMatrix2x4fv");
    api->GetProgramResourceName = loadProc("glGetProgramResourceName");
    api->SamplerParameteriv = loadProc("glSamplerParameteriv");
    api->GetActiveSubroutineUniformiv = loadProc("glGetActiveSubroutineUniformiv");
    api->GetStringi = loadProc("glGetStringi");
    api->VertexAttribLFormat = loadProc("glVertexAttribLFormat");
    api->VertexAttrib3d = loadProc("glVertexAttrib3d");
    api->BindVertexArray = loadProc("glBindVertexArray");
    api->UnmapBuffer = loadProc("glUnmapBuffer");
    api->DrawElementsInstancedBaseInstance = loadProc("glDrawElementsInstancedBaseInstance");
    api->Uniform4uiv = loadProc("glUniform4uiv");
    api->FramebufferTexture1D = loadProc("glFramebufferTexture1D");
    api->DrawTransformFeedbackStreamInstanced = loadProc("glDrawTransformFeedbackStreamInstanced");
    api->StencilFunc = loadProc("glStencilFunc");
    api->ValidateProgram = loadProc("glValidateProgram");
    api->Flush = loadProc("glFlush");
    api->ProgramUniform3uiv = loadProc("glProgramUniform3uiv");
    api->DeleteRenderbuffers = loadProc("glDeleteRenderbuffers");
    api->VertexAttrib4fv = loadProc("glVertexAttrib4fv");
    api->UniformMatrix2dv = loadProc("glUniformMatrix2dv");
    api->GetFragDataIndex = loadProc("glGetFragDataIndex");
    api->Uniform3iv = loadProc("glUniform3iv");
    api->MinSampleShading = loadProc("glMinSampleShading");
    api->GetBooleanv = loadProc("glGetBooleanv");
    api->GetMultisamplefv = loadProc("glGetMultisamplefv");
    api->GetVertexAttribIuiv = loadProc("glGetVertexAttribIuiv");
    api->GetProgramInfoLog = loadProc("glGetProgramInfoLog");
    api->Uniform4fv = loadProc("glUniform4fv");
    api->DrawBuffer = loadProc("glDrawBuffer");
    api->Uniform1i = loadProc("glUniform1i");
    api->ProgramUniform4ui = loadProc("glProgramUniform4ui");
    api->ProgramUniformMatrix3fv = loadProc("glProgramUniformMatrix3fv");
    api->BlendEquationSeparate = loadProc("glBlendEquationSeparate");
    api->BindProgramPipeline = loadProc("glBindProgramPipeline");
    api->GetDoublei_v = loadProc("glGetDoublei_v");
    api->BufferData = loadProc("glBufferData");
    api->ClearColor = loadProc("glClearColor");
    api->ProgramUniform4i = loadProc("glProgramUniform4i");
    api->GetTexLevelParameteriv = loadProc("glGetTexLevelParameteriv");
    api->GetActiveUniformBlockiv = loadProc("glGetActiveUniformBlockiv");
    api->ProgramUniform1fv = loadProc("glProgramUniform1fv");
    api->PauseTransformFeedback = loadProc("glPauseTransformFeedback");
    api->GetBufferPointerv = loadProc("glGetBufferPointerv");
    api->InvalidateSubFramebuffer = loadProc("glInvalidateSubFramebuffer");
    api->ScissorIndexedv = loadProc("glScissorIndexedv");
    api->Uniform2ui = loadProc("glUniform2ui");
    api->BindTexture = loadProc("glBindTexture");
    api->DrawElementsInstanced = loadProc("glDrawElementsInstanced");
    api->ProgramUniform4f = loadProc("glProgramUniform4f");
    api->BindBufferBase = loadProc("glBindBufferBase");
    api->IsShader = loadProc("glIsShader");
    api->ClearBufferSubData = loadProc("glClearBufferSubData");
    api->VertexAttrib4Nuiv = loadProc("glVertexAttrib4Nuiv");
    api->DrawArraysIndirect = loadProc("glDrawArraysIndirect");
    api->VertexAttrib4usv = loadProc("glVertexAttrib4usv");
    api->Uniform1d = loadProc("glUniform1d");
    api->ClearTexImage = oc_glClearTexImage_noimpl;
    api->Uniform1uiv = loadProc("glUniform1uiv");
    api->BindSampler = loadProc("glBindSampler");
    api->GetTexLevelParameterfv = loadProc("glGetTexLevelParameterfv");
    api->ClearBufferiv = loadProc("glClearBufferiv");
    api->LogicOp = loadProc("glLogicOp");
    api->ActiveTexture = loadProc("glActiveTexture");
    api->GetFragDataLocation = loadProc("glGetFragDataLocation");
    api->BlendColor = loadProc("glBlendColor");
    api->UniformMatrix4x3fv = loadProc("glUniformMatrix4x3fv");
    api->ProgramUniform3fv = loadProc("glProgramUniform3fv");
    api->Uniform1fv = loadProc("glUniform1fv");
    api->DrawElementsBaseVertex = loadProc("glDrawElementsBaseVertex");
    api->Uniform4f = loadProc("glUniform4f");
    api->BlendEquationSeparatei = loadProc("glBlendEquationSeparatei");
    api->BlendFuncSeparate = loadProc("glBlendFuncSeparate");
    api->ClearBufferuiv = loadProc("glClearBufferuiv");
    api->CopyTexSubImage1D = loadProc("glCopyTexSubImage1D");
    api->DrawTransformFeedback = loadProc("glDrawTransformFeedback");
    api->ReadBuffer = loadProc("glReadBuffer");
    api->CopyBufferSubData = loadProc("glCopyBufferSubData");
    api->GetUniformuiv = loadProc("glGetUniformuiv");
    api->PolygonOffset = loadProc("glPolygonOffset");
    api->DispatchCompute = loadProc("glDispatchCompute");
    api->BindImageTexture = loadProc("glBindImageTexture");
    api->UniformMatrix4x3dv = loadProc("glUniformMatrix4x3dv");
    api->GenRenderbuffers = loadProc("glGenRenderbuffers");
}

void oc_gl_load_gl44(oc_gl_api* api, oc_gl_load_proc loadProc)
{
    api->name = "gl44";
    api->GetFloatv = loadProc("glGetFloatv");
    api->TexBufferRange = loadProc("glTexBufferRange");
    api->IsBuffer = loadProc("glIsBuffer");
    api->IsTexture = loadProc("glIsTexture");
    api->DepthRangef = loadProc("glDepthRangef");
    api->EndConditionalRender = loadProc("glEndConditionalRender");
    api->BlendFunci = loadProc("glBlendFunci");
    api->GetProgramPipelineiv = loadProc("glGetProgramPipelineiv");
    api->WaitSync = loadProc("glWaitSync");
    api->ProgramUniformMatrix2fv = loadProc("glProgramUniformMatrix2fv");
    api->ProgramUniformMatrix4x3dv = loadProc("glProgramUniformMatrix4x3dv");
    api->VertexAttrib1dv = loadProc("glVertexAttrib1dv");
    api->SamplerParameteri = loadProc("glSamplerParameteri");
    api->GetVertexAttribIiv = loadProc("glGetVertexAttribIiv");
    api->GetSamplerParameterfv = loadProc("glGetSamplerParameterfv");
    api->VertexAttrib1d = loadProc("glVertexAttrib1d");
    api->TexBuffer = loadProc("glTexBuffer");
    api->InvalidateBufferData = loadProc("glInvalidateBufferData");
    api->ProgramUniform2i = loadProc("glProgramUniform2i");
    api->Uniform4dv = loadProc("glUniform4dv");
    api->UseProgram = loadProc("glUseProgram");
    api->VertexAttribI3iv = loadProc("glVertexAttribI3iv");
    api->DrawElementsIndirect = loadProc("glDrawElementsIndirect");
    api->VertexAttrib4uiv = loadProc("glVertexAttrib4uiv");
    api->GetQueryObjectiv = loadProc("glGetQueryObjectiv");
    api->FramebufferRenderbuffer = loadProc("glFramebufferRenderbuffer");
    api->BlendEquationi = loadProc("glBlendEquationi");
    api->GetActiveSubroutineName = loadProc("glGetActiveSubroutineName");
    api->VertexAttrib2s = loadProc("glVertexAttrib2s");
    api->VertexAttribL1d = loadProc("glVertexAttribL1d");
    api->BindTextures = loadProc("glBindTextures");
    api->VertexAttrib3sv = loadProc("glVertexAttrib3sv");
    api->GetFloati_v = loadProc("glGetFloati_v");
    api->BeginTransformFeedback = loadProc("glBeginTransformFeedback");
    api->ClearStencil = loadProc("glClearStencil");
    api->Uniform3i = loadProc("glUniform3i");
    api->ValidateProgramPipeline = loadProc("glValidateProgramPipeline");
    api->ProgramUniformMatrix4x2fv = loadProc("glProgramUniformMatrix4x2fv");
    api->VertexAttribI4ui = loadProc("glVertexAttribI4ui");
    api->GetShaderiv = loadProc("glGetShaderiv");
    api->ReadnPixels = oc_glReadnPixels_noimpl;
    api->UniformMatrix4x2fv = loadProc("glUniformMatrix4x2fv");
    api->GetShaderPrecisionFormat = loadProc("glGetShaderPrecisionFormat");
    api->ProgramUniformMatrix2x3fv = loadProc("glProgramUniformMatrix2x3fv");
    api->TexSubImage3D = loadProc("glTexSubImage3D");
    api->GetProgramResourceLocationIndex = loadProc("glGetProgramResourceLocationIndex");
    api->BlendFunc = loadProc("glBlendFunc");
    api->ProgramUniformMatrix3x4fv = loadProc("glProgramUniformMatrix3x4fv");
    api->Uniform3d = loadProc("glUniform3d");
    api->VertexAttrib1sv = loadProc("glVertexAttrib1sv");
    api->BindFragDataLocation = loadProc("glBindFragDataLocation");
    api->VertexAttrib4bv = loadProc("glVertexAttrib4bv");
    api->Uniform4iv = loadProc("glUniform4iv");
    api->ProgramUniform2ui = loadProc("glProgramUniform2ui");
    api->DrawArrays = loadProc("glDrawArrays");
    api->ProgramBinary = loadProc("glProgramBinary");
    api->VertexAttrib4f = loadProc("glVertexAttrib4f");
    api->VertexAttribP2uiv = loadProc("glVertexAttribP2uiv");
    api->UniformMatrix3fv = loadProc("glUniformMatrix3fv");
    api->Uniform2i = loadProc("glUniform2i");
    api->GetQueryObjectuiv = loadProc("glGetQueryObjectuiv");
    api->UniformBlockBinding = loadProc("glUniformBlockBinding");
    api->SampleCoverage = loadProc("glSampleCoverage");
    api->VertexAttrib4Nusv = loadProc("glVertexAttrib4Nusv");
    api->ProgramUniformMatrix2x4dv = loadProc("glProgramUniformMatrix2x4dv");
    api->Uniform3uiv = loadProc("glUniform3uiv");
    api->VertexAttrib1s = loadProc("glVertexAttrib1s");
    api->GetVertexAttribPointerv = loadProc("glGetVertexAttribPointerv");
    api->BlendBarrier = oc_glBlendBarrier_noimpl;
    api->DrawRangeElements = loadProc("glDrawRangeElements");
    api->TexStorage3D = loadProc("glTexStorage3D");
    api->GetInternalformati64v = loadProc("glGetInternalformati64v");
    api->GetQueryObjecti64v = loadProc("glGetQueryObjecti64v");
    api->CompressedTexSubImage1D = loadProc("glCompressedTexSubImage1D");
    api->VertexAttrib3dv = loadProc("glVertexAttrib3dv");
    api->VertexBindingDivisor = loadProc("glVertexBindingDivisor");
    api->UseProgramStages = loadProc("glUseProgramStages");
    api->VertexAttribBinding = loadProc("glVertexAttribBinding");
    api->DebugMessageInsert = loadProc("glDebugMessageInsert");
    api->GetTexParameteriv = loadProc("glGetTexParameteriv");
    api->MultiDrawArraysIndirect = loadProc("glMultiDrawArraysIndirect");
    api->GetTexParameterfv = loadProc("glGetTexParameterfv");
    api->GetProgramPipelineInfoLog = loadProc("glGetProgramPipelineInfoLog");
    api->EndQuery = loadProc("glEndQuery");
    api->GetProgramResourceLocation = loadProc("glGetProgramResourceLocation");
    api->CompressedTexImage2D = loadProc("glCompressedTexImage2D");
    api->VertexAttribP2ui = loadProc("glVertexAttribP2ui");
    api->IsEnabledi = loadProc("glIsEnabledi");
    api->GetActiveAtomicCounterBufferiv = loadProc("glGetActiveAtomicCounterBufferiv");
    api->IsProgram = loadProc("glIsProgram");
    api->Uniform1dv = loadProc("glUniform1dv");
    api->TexParameteriv = loadProc("glTexParameteriv");
    api->Uniform2fv = loadProc("glUniform2fv");
    api->ReleaseShaderCompiler = loadProc("glReleaseShaderCompiler");
    api->CullFace = loadProc("glCullFace");
    api->VertexAttribI4i = loadProc("glVertexAttribI4i");
    api->GetProgramResourceIndex = loadProc("glGetProgramResourceIndex");
    api->ShaderBinary = loadProc("glShaderBinary");
    api->UniformMatrix3x2dv = loadProc("glUniformMatrix3x2dv");
    api->InvalidateFramebuffer = loadProc("glInvalidateFramebuffer");
    api->AttachShader = loadProc("glAttachShader");
    api->FlushMappedBufferRange = loadProc("glFlushMappedBufferRange");
    api->VertexAttribP3uiv = loadProc("glVertexAttribP3uiv");
    api->GetActiveUniformName = loadProc("glGetActiveUniformName");
    api->MapBuffer = loadProc("glMapBuffer");
    api->DrawBuffers = loadProc("glDrawBuffers");
    api->GetSynciv = loadProc("glGetSynciv");
    api->CopyTexSubImage2D = loadProc("glCopyTexSubImage2D");
    api->ObjectLabel = loadProc("glObjectLabel");
    api->BufferSubData = loadProc("glBufferSubData");
    api->Uniform2f = loadProc("glUniform2f");
    api->DebugMessageCallback = loadProc("glDebugMessageCallback");
    api->VertexAttribL4dv = loadProc("glVertexAttribL4dv");
    api->IsProgramPipeline = loadProc("glIsProgramPipeline");
    api->ResumeTransformFeedback = loadProc("glResumeTransformFeedback");
    api->VertexAttribI4iv = loadProc("glVertexAttribI4iv");
    api->GetShaderInfoLog = loadProc("glGetShaderInfoLog");
    api->GetIntegeri_v = loadProc("glGetIntegeri_v");
    api->BindVertexBuffer = loadProc("glBindVertexBuffer");
    api->BlendEquation = loadProc("glBlendEquation");
    api->VertexAttribL2dv = loadProc("glVertexAttribL2dv");
    api->VertexAttribI1ui = loadProc("glVertexAttribI1ui");
    api->VertexAttrib4Nsv = loadProc("glVertexAttrib4Nsv");
    api->VertexAttribL4d = loadProc("glVertexAttribL4d");
    api->CopyImageSubData = loadProc("glCopyImageSubData");
    api->GetFramebufferAttachmentParameteriv = loadProc("glGetFramebufferAttachmentParameteriv");
    api->VertexAttribL2d = loadProc("glVertexAttribL2d");
    api->GetSubroutineIndex = loadProc("glGetSubroutineIndex");
    api->VertexAttribI3uiv = loadProc("glVertexAttribI3uiv");
    api->VertexAttrib4iv = loadProc("glVertexAttrib4iv");
    api->BindVertexBuffers = loadProc("glBindVertexBuffers");
    api->ProgramUniformMatrix2x3dv = loadProc("glProgramUniformMatrix2x3dv");
    api->PrimitiveBoundingBox = oc_glPrimitiveBoundingBox_noimpl;
    api->Scissor = loadProc("glScissor");
    api->ClientWaitSync = loadProc("glClientWaitSync");
    api->Uniform3ui = loadProc("glUniform3ui");
    api->VertexAttribP3ui = loadProc("glVertexAttribP3ui");
    api->Enable = loadProc("glEnable");
    api->StencilOpSeparate = loadProc("glStencilOpSeparate");
    api->UniformMatrix2x3dv = loadProc("glUniformMatrix2x3dv");
    api->ProgramUniformMatrix3dv = loadProc("glProgramUniformMatrix3dv");
    api->TexImage2DMultisample = loadProc("glTexImage2DMultisample");
    api->VertexAttrib4Nbv = loadProc("glVertexAttrib4Nbv");
    api->GetTexImage = loadProc("glGetTexImage");
    api->VertexAttrib4sv = loadProc("glVertexAttrib4sv");
    api->PixelStorei = loadProc("glPixelStorei");
    api->DepthMask = loadProc("glDepthMask");
    api->TexStorage2D = loadProc("glTexStorage2D");
    api->Clear = loadProc("glClear");
    api->UniformMatrix3x4dv = loadProc("glUniformMatrix3x4dv");
    api->DeleteTransformFeedbacks = loadProc("glDeleteTransformFeedbacks");
    api->MapBufferRange = loadProc("glMapBufferRange");
    api->MemoryBarrier = loadProc("glMemoryBarrier");
    api->ViewportIndexedf = loadProc("glViewportIndexedf");
    api->VertexAttrib3fv = loadProc("glVertexAttrib3fv");
    api->ObjectPtrLabel = loadProc("glObjectPtrLabel");
    api->TexStorage1D = loadProc("glTexStorage1D");
    api->CompressedTexImage3D = loadProc("glCompressedTexImage3D");
    api->VertexAttrib1fv = loadProc("glVertexAttrib1fv");
    api->VertexAttribPointer = loadProc("glVertexAttribPointer");
    api->GetQueryIndexediv = loadProc("glGetQueryIndexediv");
    api->CompileShader = loadProc("glCompileShader");
    api->ProgramUniform1i = loadProc("glProgramUniform1i");
    api->GetQueryiv = loadProc("glGetQueryiv");
    api->VertexAttribI1iv = loadProc("glVertexAttribI1iv");
    api->CopyTexImage2D = loadProc("glCopyTexImage2D");
    api->GetQueryObjectui64v = loadProc("glGetQueryObjectui64v");
    api->PointSize = loadProc("glPointSize");
    api->Disablei = loadProc("glDisablei");
    api->VertexAttribL1dv = loadProc("glVertexAttribL1dv");
    api->CreateShader = loadProc("glCreateShader");
    api->GetString = loadProc("glGetString");
    api->ViewportArrayv = loadProc("glViewportArrayv");
    api->ProgramUniform3d = loadProc("glProgramUniform3d");
    api->VertexAttrib4Nubv = loadProc("glVertexAttrib4Nubv");
    api->TexParameteri = loadProc("glTexParameteri");
    api->ProgramUniform4fv = loadProc("glProgramUniform4fv");
    api->GenerateMipmap = loadProc("glGenerateMipmap");
    api->CompressedTexSubImage3D = loadProc("glCompressedTexSubImage3D");
    api->Uniform3f = loadProc("glUniform3f");
    api->GetUniformIndices = loadProc("glGetUniformIndices");
    api->VertexAttribLPointer = loadProc("glVertexAttribLPointer");
    api->VertexAttribI2uiv = loadProc("glVertexAttribI2uiv");
    api->QueryCounter = loadProc("glQueryCounter");
    api->ActiveShaderProgram = loadProc("glActiveShaderProgram");
    api->Uniform1ui = loadProc("glUniform1ui");
    api->VertexAttribI1i = loadProc("glVertexAttribI1i");
    api->GetTexParameterIiv = loadProc("glGetTexParameterIiv");
    api->GetUniformfv = loadProc("glGetUniformfv");
    api->ProgramUniform2uiv = loadProc("glProgramUniform2uiv");
    api->GetError = loadProc("glGetError");
    api->GetActiveUniformBlockName = loadProc("glGetActiveUniformBlockName");
    api->TextureView = loadProc("glTextureView");
    api->GetnUniformiv = oc_glGetnUniformiv_noimpl;
    api->ProgramUniform4dv = loadProc("glProgramUniform4dv");
    api->ViewportIndexedfv = loadProc("glViewportIndexedfv");
    api->Hint = loadProc("glHint");
    api->GetShaderSource = loadProc("glGetShaderSource");
    api->ProgramUniformMatrix4x3fv = loadProc("glProgramUniformMatrix4x3fv");
    api->Uniform1iv = loadProc("glUniform1iv");
    api->VertexAttribI4bv = loadProc("glVertexAttribI4bv");
    api->UniformMatrix4x2dv = loadProc("glUniformMatrix4x2dv");
    api->BufferStorage = loadProc("glBufferStorage");
    api->IsRenderbuffer = loadProc("glIsRenderbuffer");
    api->GetActiveSubroutineUniformName = loadProc("glGetActiveSubroutineUniformName");
    api->LinkProgram = loadProc("glLinkProgram");
    api->GetActiveUniformsiv = loadProc("glGetActiveUniformsiv");
    api->GetDebugMessageLog = loadProc("glGetDebugMessageLog");
    api->CopyTexSubImage3D = loadProc("glCopyTexSubImage3D");
    api->PointParameteri = loadProc("glPointParameteri");
    api->ProgramUniform3dv = loadProc("glProgramUniform3dv");
    api->CompressedTexImage1D = loadProc("glCompressedTexImage1D");
    api->UniformMatrix3x4fv = loadProc("glUniformMatrix3x4fv");
    api->GenSamplers = loadProc("glGenSamplers");
    api->GetCompressedTexImage = loadProc("glGetCompressedTexImage");
    api->DeleteQueries = loadProc("glDeleteQueries");
    api->GenProgramPipelines = loadProc("glGenProgramPipelines");
    api->DispatchComputeIndirect = loadProc("glDispatchComputeIndirect");
    api->VertexAttribIPointer = loadProc("glVertexAttribIPointer");
    api->CreateProgram = loadProc("glCreateProgram");
    api->ClearTexSubImage = loadProc("glClearTexSubImage");
    api->VertexAttrib4d = loadProc("glVertexAttrib4d");
    api->FrontFace = loadProc("glFrontFace");
    api->BindTransformFeedback = loadProc("glBindTransformFeedback");
    api->GetProgramStageiv = loadProc("glGetProgramStageiv");
    api->SamplerParameterIiv = loadProc("glSamplerParameterIiv");
    api->GetInteger64v = loadProc("glGetInteger64v");
    api->CreateShaderProgramv = loadProc("glCreateShaderProgramv");
    api->BindBuffersRange = loadProc("glBindBuffersRange");
    api->Uniform3fv = loadProc("glUniform3fv");
    api->ProgramUniformMatrix4fv = loadProc("glProgramUniformMatrix4fv");
    api->BindBuffersBase = loadProc("glBindBuffersBase");
    api->ClearBufferfi = loadProc("glClearBufferfi");
    api->FramebufferTexture3D = loadProc("glFramebufferTexture3D");
    api->Disable = loadProc("glDisable");
    api->ProgramUniform1iv = loadProc("glProgramUniform1iv");
    api->VertexAttribI2iv = loadProc("glVertexAttribI2iv");
    api->DepthRangeIndexed = loadProc("glDepthRangeIndexed");
    api->PatchParameteri = loadProc("glPatchParameteri");
    api->GetUniformBlockIndex = loadProc("glGetUniformBlockIndex");
    api->MultiDrawArrays = loadProc("glMultiDrawArrays");
    api->VertexAttribI4ubv = loadProc("glVertexAttribI4ubv");
    api->BindBuffer = loadProc("glBindBuffer");
    api->VertexAttribI3i = loadProc("glVertexAttribI3i");
    api->GetDoublev = loadProc("glGetDoublev");
    api->DrawTransformFeedbackStream = loadProc("glDrawTransformFeedbackStream");
    api->VertexAttribI4uiv = loadProc("glVertexAttribI4uiv");
    api->RenderbufferStorageMultisample = loadProc("glRenderbufferStorageMultisample");
    api->VertexAttribL3dv = loadProc("glVertexAttribL3dv");
    api->StencilMaskSeparate = loadProc("glStencilMaskSeparate");
    api->ProgramUniform1d = loadProc("glProgramUniform1d");
    api->Viewport = loadProc("glViewport");
    api->VertexAttribP1ui = loadProc("glVertexAttribP1ui");
    api->VertexAttrib4dv = loadProc("glVertexAttrib4dv");
    api->GenQueries = loadProc("glGenQueries");
    api->TexParameterIiv = loadProc("glTexParameterIiv");
    api->ProgramUniform2d = loadProc("glProgramUniform2d");
    api->ProgramUniform1uiv = loadProc("glProgramUniform1uiv");
    api->VertexAttrib4Nub = loadProc("glVertexAttrib4Nub");
    api->IsVertexArray = loadProc("glIsVertexArray");
    api->ProgramUniform3f = loadProc("glProgramUniform3f");
    api->ProgramUniform3iv = loadProc("glProgramUniform3iv");
    api->GetProgramBinary = loadProc("glGetProgramBinary");
    api->BindRenderbuffer = loadProc("glBindRenderbuffer");
    api->BindFragDataLocationIndexed = loadProc("glBindFragDataLocationIndexed");
    api->GetSamplerParameterIiv = loadProc("glGetSamplerParameterIiv");
    api->VertexAttribDivisor = loadProc("glVertexAttribDivisor");
    api->ProgramUniformMatrix3x2dv = loadProc("glProgramUniformMatrix3x2dv");
    api->FramebufferParameteri = loadProc("glFramebufferParameteri");
    api->GenTransformFeedbacks = loadProc("glGenTransformFeedbacks");
    api->DeleteSync = loadProc("glDeleteSync");
    api->ProgramUniform1ui = loadProc("glProgramUniform1ui");
    api->TexSubImage1D = loadProc("glTexSubImage1D");
    api->ClearDepthf = loadProc("glClearDepthf");
    api->ReadPixels = loadProc("glReadPixels");
    api->VertexAttribI2i = loadProc("glVertexAttribI2i");
    api->Finish = loadProc("glFinish");
    api->LineWidth = loadProc("glLineWidth");
    api->DeleteShader = loadProc("glDeleteShader");
    api->IsSampler = loadProc("glIsSampler");
    api->ProgramUniformMatrix4dv = loadProc("glProgramUniformMatrix4dv");
    api->TransformFeedbackVaryings = loadProc("glTransformFeedbackVaryings");
    api->BeginConditionalRender = loadProc("glBeginConditionalRender");
    api->BindSamplers = loadProc("glBindSamplers");
    api->DeleteProgramPipelines = loadProc("glDeleteProgramPipelines");
    api->ColorMask = loadProc("glColorMask");
    api->TexParameterfv = loadProc("glTexParameterfv");
    api->PushDebugGroup = loadProc("glPushDebugGroup");
    api->ClearBufferfv = loadProc("glClearBufferfv");
    api->IsEnabled = loadProc("glIsEnabled");
    api->VertexAttrib2f = loadProc("glVertexAttrib2f");
    api->ProgramUniform2f = loadProc("glProgramUniform2f");
    api->GetSamplerParameterIuiv = loadProc("glGetSamplerParameterIuiv");
    api->GetInteger64i_v = loadProc("glGetInteger64i_v");
    api->Uniform2dv = loadProc("glUniform2dv");
    api->GetBufferSubData = loadProc("glGetBufferSubData");
    api->MultiDrawElementsIndirect = loadProc("glMultiDrawElementsIndirect");
    api->ProgramParameteri = loadProc("glProgramParameteri");
    api->VertexAttribP4ui = loadProc("glVertexAttribP4ui");
    api->SamplerParameterfv = loadProc("glSamplerParameterfv");
    api->PointParameterf = loadProc("glPointParameterf");
    api->UniformMatrix2x4fv = loadProc("glUniformMatrix2x4fv");
    api->GenBuffers = loadProc("glGenBuffers");
    api->ProgramUniform2dv = loadProc("glProgramUniform2dv");
    api->VertexAttribFormat = loadProc("glVertexAttribFormat");
    api->TexSubImage2D = loadProc("glTexSubImage2D");
    api->VertexAttrib4ubv = loadProc("glVertexAttrib4ubv");
    api->GetGraphicsResetStatus = oc_glGetGraphicsResetStatus_noimpl;
    api->GetProgramInterfaceiv = loadProc("glGetProgramInterfaceiv");
    api->VertexAttribIFormat = loadProc("glVertexAttribIFormat");
    api->GetnUniformfv = oc_glGetnUniformfv_noimpl;
    api->DeleteProgram = loadProc("glDeleteProgram");
    api->ClampColor = loadProc("glClampColor");
    api->DrawElementsInstancedBaseVertexBaseInstance = loadProc("glDrawElementsInstancedBaseVertexBaseInstance");
    api->DrawElements = loadProc("glDrawElements");
    api->DebugMessageControl = loadProc("glDebugMessageControl");
    api->GetRenderbufferParameteriv = loadProc("glGetRenderbufferParameteriv");
    api->DetachShader = loadProc("glDetachShader");
    api->GenFramebuffers = loadProc("glGenFramebuffers");
    api->ProvokingVertex = loadProc("glProvokingVertex");
    api->SampleMaski = loadProc("glSampleMaski");
    api->EndQueryIndexed = loadProc("glEndQueryIndexed");
    api->ProgramUniform1f = loadProc("glProgramUniform1f");
    api->BindFramebuffer = loadProc("glBindFramebuffer");
    api->BeginQueryIndexed = loadProc("glBeginQueryIndexed");
    api->UniformSubroutinesuiv = loadProc("glUniformSubroutinesuiv");
    api->GetUniformiv = loadProc("glGetUniformiv");
    api->FramebufferTexture = loadProc("glFramebufferTexture");
    api->PointParameterfv = loadProc("glPointParameterfv");
    api->IsTransformFeedback = loadProc("glIsTransformFeedback");
    api->CheckFramebufferStatus = loadProc("glCheckFramebufferStatus");
    api->ShaderSource = loadProc("glShaderSource");
    api->UniformMatrix2x4dv = loadProc("glUniformMatrix2x4dv");
    api->BindImageTextures = loadProc("glBindImageTextures");
    api->CopyTexImage1D = loadProc("glCopyTexImage1D");
    api->UniformMatrix3dv = loadProc("glUniformMatrix3dv");
    api->ProgramUniform1dv = loadProc("glProgramUniform1dv");
    api->BlitFramebuffer = loadProc("glBlitFramebuffer");
    api->PopDebugGroup = loadProc("glPopDebugGroup");
    api->TexParameterIuiv = loadProc("glTexParameterIuiv");
    api->VertexAttrib2d = loadProc("glVertexAttrib2d");
    api->TexImage1D = loadProc("glTexImage1D");
    api->GetObjectPtrLabel = loadProc("glGetObjectPtrLabel");
    api->StencilMask = loadProc("glStencilMask");
    api->BeginQuery = loadProc("glBeginQuery");
    api->UniformMatrix4fv = loadProc("glUniformMatrix4fv");
    api->IsSync = loadProc("glIsSync");
    api->Uniform3dv = loadProc("glUniform3dv");
    api->ProgramUniform2fv = loadProc("glProgramUniform2fv");
    api->VertexAttribI4sv = loadProc("glVertexAttribI4sv");
    api->ScissorArrayv = loadProc("glScissorArrayv");
    api->VertexAttribP1uiv = loadProc("glVertexAttribP1uiv");
    api->Uniform2uiv = loadProc("glUniform2uiv");
    api->DeleteBuffers = loadProc("glDeleteBuffers");
    api->ProgramUniform3ui = loadProc("glProgramUniform3ui");
    api->FramebufferTextureLayer = loadProc("glFramebufferTextureLayer");
    api->EndTransformFeedback = loadProc("glEndTransformFeedback");
    api->BlendFuncSeparatei = loadProc("glBlendFuncSeparatei");
    api->DrawTransformFeedbackInstanced = loadProc("glDrawTransformFeedbackInstanced");
    api->DrawRangeElementsBaseVertex = loadProc("glDrawRangeElementsBaseVertex");
    api->VertexAttrib1f = loadProc("glVertexAttrib1f");
    api->GetUniformSubroutineuiv = loadProc("glGetUniformSubroutineuiv");
    api->DisableVertexAttribArray = loadProc("glDisableVertexAttribArray");
    api->ProgramUniformMatrix3x2fv = loadProc("glProgramUniformMatrix3x2fv");
    api->VertexAttribI4usv = loadProc("glVertexAttribI4usv");
    api->GetObjectLabel = loadProc("glGetObjectLabel");
    api->BindAttribLocation = loadProc("glBindAttribLocation");
    api->Uniform1f = loadProc("glUniform1f");
    api->GetUniformdv = loadProc("glGetUniformdv");
    api->GetUniformLocation = loadProc("glGetUniformLocation");
    api->GetSubroutineUniformLocation = loadProc("glGetSubroutineUniformLocation");
    api->GetTexParameterIuiv = loadProc("glGetTexParameterIuiv");
    api->SamplerParameterf = loadProc("glSamplerParameterf");
    api->VertexAttribL3d = loadProc("glVertexAttribL3d");
    api->TexImage3DMultisample = loadProc("glTexImage3DMultisample");
    api->TexImage3D = loadProc("glTexImage3D");
    api->RenderbufferStorage = loadProc("glRenderbufferStorage");
    api->EnableVertexAttribArray = loadProc("glEnableVertexAttribArray");
    api->VertexAttribP4uiv = loadProc("glVertexAttribP4uiv");
    api->Uniform4d = loadProc("glUniform4d");
    api->VertexAttrib4s = loadProc("glVertexAttrib4s");
    api->DrawElementsInstancedBaseVertex = loadProc("glDrawElementsInstancedBaseVertex");
    api->VertexAttrib3s = loadProc("glVertexAttrib3s");
    api->ProgramUniform2iv = loadProc("glProgramUniform2iv");
    api->StencilFuncSeparate = loadProc("glStencilFuncSeparate");
    api->DeleteFramebuffers = loadProc("glDeleteFramebuffers");
    api->DepthRange = loadProc("glDepthRange");
    api->UniformMatrix3x2fv = loadProc("glUniformMatrix3x2fv");
    api->ProgramUniformMatrix2dv = loadProc("glProgramUniformMatrix2dv");
    api->ShaderStorageBlockBinding = loadProc("glShaderStorageBlockBinding");
    api->ClearDepth = loadProc("glClearDepth");
    api->VertexAttrib2dv = loadProc("glVertexAttrib2dv");
    api->SamplerParameterIuiv = loadProc("glSamplerParameterIuiv");
    api->GetVertexAttribLdv = loadProc("glGetVertexAttribLdv");
    api->ProgramUniformMatrix3x4dv = loadProc("glProgramUniformMatrix3x4dv");
    api->DepthRangeArrayv = loadProc("glDepthRangeArrayv");
    api->GetActiveUniform = loadProc("glGetActiveUniform");
    api->PatchParameterfv = loadProc("glPatchParameterfv");
    api->InvalidateTexImage = loadProc("glInvalidateTexImage");
    api->VertexAttrib3f = loadProc("glVertexAttrib3f");
    api->ProgramUniform4iv = loadProc("glProgramUniform4iv");
    api->ProgramUniform4d = loadProc("glProgramUniform4d");
    api->IsFramebuffer = loadProc("glIsFramebuffer");
    api->PixelStoref = loadProc("glPixelStoref");
    api->ProgramUniform4uiv = loadProc("glProgramUniform4uiv");
    api->ProgramUniformMatrix4x2dv = loadProc("glProgramUniformMatrix4x2dv");
    api->FenceSync = loadProc("glFenceSync");
    api->GetBufferParameteri64v = loadProc("glGetBufferParameteri64v");
    api->StencilOp = loadProc("glStencilOp");
    api->ClearBufferData = loadProc("glClearBufferData");
    api->GetnUniformuiv = oc_glGetnUniformuiv_noimpl;
    api->GetProgramResourceiv = loadProc("glGetProgramResourceiv");
    api->GetVertexAttribdv = loadProc("glGetVertexAttribdv");
    api->GetTransformFeedbackVarying = loadProc("glGetTransformFeedbackVarying");
    api->VertexAttrib2fv = loadProc("glVertexAttrib2fv");
    api->GetBooleani_v = loadProc("glGetBooleani_v");
    api->ColorMaski = loadProc("glColorMaski");
    api->InvalidateBufferSubData = loadProc("glInvalidateBufferSubData");
    api->UniformMatrix4dv = loadProc("glUniformMatrix4dv");
    api->IsQuery = loadProc("glIsQuery");
    api->Uniform4ui = loadProc("glUniform4ui");
    api->Uniform4i = loadProc("glUniform4i");
    api->GetSamplerParameteriv = loadProc("glGetSamplerParameteriv");
    api->MultiDrawElementsBaseVertex = loadProc("glMultiDrawElementsBaseVertex");
    api->VertexAttribI1uiv = loadProc("glVertexAttribI1uiv");
    api->GetIntegerv = loadProc("glGetIntegerv");
    api->UniformMatrix2x3fv = loadProc("glUniformMatrix2x3fv");
    api->TexImage2D = loadProc("glTexImage2D");
    api->GetAttachedShaders = loadProc("glGetAttachedShaders");
    api->Uniform2d = loadProc("glUniform2d");
    api->MemoryBarrierByRegion = oc_glMemoryBarrierByRegion_noimpl;
    api->UniformMatrix2fv = loadProc("glUniformMatrix2fv");
    api->PrimitiveRestartIndex = loadProc("glPrimitiveRestartIndex");
    api->GetVertexAttribiv = loadProc("glGetVertexAttribiv");
    api->GetAttribLocation = loadProc("glGetAttribLocation");
    api->TexStorage2DMultisample = loadProc("glTexStorage2DMultisample");
    api->CompressedTexSubImage2D = loadProc("glCompressedTexSubImage2D");
    api->GetVertexAttribfv = loadProc("glGetVertexAttribfv");
    api->GetBufferParameteriv = loadProc("glGetBufferParameteriv");
    api->TexParameterf = loadProc("glTexParameterf");
    api->FramebufferTexture2D = loadProc("glFramebufferTexture2D");
    api->GetActiveAttrib = loadProc("glGetActiveAttrib");
    api->InvalidateTexSubImage = loadProc("glInvalidateTexSubImage");
    api->DeleteVertexArrays = loadProc("glDeleteVertexArrays");
    api->VertexAttribI2ui = loadProc("glVertexAttribI2ui");
    api->PointParameteriv = loadProc("glPointParameteriv");
    api->GetPointerv = loadProc("glGetPointerv");
    api->Enablei = loadProc("glEnablei");
    api->BindBufferRange = loadProc("glBindBufferRange");
    api->DrawArraysInstanced = loadProc("glDrawArraysInstanced");
    api->DeleteTextures = loadProc("glDeleteTextures");
    api->VertexAttrib4Niv = loadProc("glVertexAttrib4Niv");
    api->MultiDrawElements = loadProc("glMultiDrawElements");
    api->GetProgramiv = loadProc("glGetProgramiv");
    api->DepthFunc = loadProc("glDepthFunc");
    api->GenTextures = loadProc("glGenTextures");
    api->GetInternalformativ = loadProc("glGetInternalformativ");
    api->ProgramUniform3i = loadProc("glProgramUniform3i");
    api->ScissorIndexed = loadProc("glScissorIndexed");
    api->VertexAttrib2sv = loadProc("glVertexAttrib2sv");
    api->TexStorage3DMultisample = loadProc("glTexStorage3DMultisample");
    api->Uniform2iv = loadProc("glUniform2iv");
    api->DrawArraysInstancedBaseInstance = loadProc("glDrawArraysInstancedBaseInstance");
    api->VertexAttribI3ui = loadProc("glVertexAttribI3ui");
    api->DeleteSamplers = loadProc("glDeleteSamplers");
    api->GenVertexArrays = loadProc("glGenVertexArrays");
    api->GetFramebufferParameteriv = loadProc("glGetFramebufferParameteriv");
    api->PolygonMode = loadProc("glPolygonMode");
    api->ProgramUniformMatrix2x4fv = loadProc("glProgramUniformMatrix2x4fv");
    api->GetProgramResourceName = loadProc("glGetProgramResourceName");
    api->SamplerParameteriv = loadProc("glSamplerParameteriv");
    api->GetActiveSubroutineUniformiv = loadProc("glGetActiveSubroutineUniformiv");
    api->GetStringi = loadProc("glGetStringi");
    api->VertexAttribLFormat = loadProc("glVertexAttribLFormat");
    api->VertexAttrib3d = loadProc("glVertexAttrib3d");
    api->BindVertexArray = loadProc("glBindVertexArray");
    api->UnmapBuffer = loadProc("glUnmapBuffer");
    api->DrawElementsInstancedBaseInstance = loadProc("glDrawElementsInstancedBaseInstance");
    api->Uniform4uiv = loadProc("glUniform4uiv");
    api->FramebufferTexture1D = loadProc("glFramebufferTexture1D");
    api->DrawTransformFeedbackStreamInstanced = loadProc("glDrawTransformFeedbackStreamInstanced");
    api->StencilFunc = loadProc("glStencilFunc");
    api->ValidateProgram = loadProc("glValidateProgram");
    api->Flush = loadProc("glFlush");
    api->ProgramUniform3uiv = loadProc("glProgramUniform3uiv");
    api->DeleteRenderbuffers = loadProc("glDeleteRenderbuffers");
    api->VertexAttrib4fv = loadProc("glVertexAttrib4fv");
    api->UniformMatrix2dv = loadProc("glUniformMatrix2dv");
    api->GetFragDataIndex = loadProc("glGetFragDataIndex");
    api->Uniform3iv = loadProc("glUniform3iv");
    api->MinSampleShading = loadProc("glMinSampleShading");
    api->GetBooleanv = loadProc("glGetBooleanv");
    api->GetMultisamplefv = loadProc("glGetMultisamplefv");
    api->GetVertexAttribIuiv = loadProc("glGetVertexAttribIuiv");
    api->GetProgramInfoLog = loadProc("glGetProgramInfoLog");
    api->Uniform4fv = loadProc("glUniform4fv");
    api->DrawBuffer = loadProc("glDrawBuffer");
    api->Uniform1i = loadProc("glUniform1i");
    api->ProgramUniform4ui = loadProc("glProgramUniform4ui");
    api->ProgramUniformMatrix3fv = loadProc("glProgramUniformMatrix3fv");
    api->BlendEquationSeparate = loadProc("glBlendEquationSeparate");
    api->BindProgramPipeline = loadProc("glBindProgramPipeline");
    api->GetDoublei_v = loadProc("glGetDoublei_v");
    api->BufferData = loadProc("glBufferData");
    api->ClearColor = loadProc("glClearColor");
    api->ProgramUniform4i = loadProc("glProgramUniform4i");
    api->GetTexLevelParameteriv = loadProc("glGetTexLevelParameteriv");
    api->GetActiveUniformBlockiv = loadProc("glGetActiveUniformBlockiv");
    api->ProgramUniform1fv = loadProc("glProgramUniform1fv");
    api->PauseTransformFeedback = loadProc("glPauseTransformFeedback");
    api->GetBufferPointerv = loadProc("glGetBufferPointerv");
    api->InvalidateSubFramebuffer = loadProc("glInvalidateSubFramebuffer");
    api->ScissorIndexedv = loadProc("glScissorIndexedv");
    api->Uniform2ui = loadProc("glUniform2ui");
    api->BindTexture = loadProc("glBindTexture");
    api->DrawElementsInstanced = loadProc("glDrawElementsInstanced");
    api->ProgramUniform4f = loadProc("glProgramUniform4f");
    api->BindBufferBase = loadProc("glBindBufferBase");
    api->IsShader = loadProc("glIsShader");
    api->ClearBufferSubData = loadProc("glClearBufferSubData");
    api->VertexAttrib4Nuiv = loadProc("glVertexAttrib4Nuiv");
    api->DrawArraysIndirect = loadProc("glDrawArraysIndirect");
    api->VertexAttrib4usv = loadProc("glVertexAttrib4usv");
    api->Uniform1d = loadProc("glUniform1d");
    api->ClearTexImage = loadProc("glClearTexImage");
    api->Uniform1uiv = loadProc("glUniform1uiv");
    api->BindSampler = loadProc("glBindSampler");
    api->GetTexLevelParameterfv = loadProc("glGetTexLevelParameterfv");
    api->ClearBufferiv = loadProc("glClearBufferiv");
    api->LogicOp = loadProc("glLogicOp");
    api->ActiveTexture = loadProc("glActiveTexture");
    api->GetFragDataLocation = loadProc("glGetFragDataLocation");
    api->BlendColor = loadProc("glBlendColor");
    api->UniformMatrix4x3fv = loadProc("glUniformMatrix4x3fv");
    api->ProgramUniform3fv = loadProc("glProgramUniform3fv");
    api->Uniform1fv = loadProc("glUniform1fv");
    api->DrawElementsBaseVertex = loadProc("glDrawElementsBaseVertex");
    api->Uniform4f = loadProc("glUniform4f");
    api->BlendEquationSeparatei = loadProc("glBlendEquationSeparatei");
    api->BlendFuncSeparate = loadProc("glBlendFuncSeparate");
    api->ClearBufferuiv = loadProc("glClearBufferuiv");
    api->CopyTexSubImage1D = loadProc("glCopyTexSubImage1D");
    api->DrawTransformFeedback = loadProc("glDrawTransformFeedback");
    api->ReadBuffer = loadProc("glReadBuffer");
    api->CopyBufferSubData = loadProc("glCopyBufferSubData");
    api->GetUniformuiv = loadProc("glGetUniformuiv");
    api->PolygonOffset = loadProc("glPolygonOffset");
    api->DispatchCompute = loadProc("glDispatchCompute");
    api->BindImageTexture = loadProc("glBindImageTexture");
    api->UniformMatrix4x3dv = loadProc("glUniformMatrix4x3dv");
    api->GenRenderbuffers = loadProc("glGenRenderbuffers");
}

void oc_gl_load_gles31(oc_gl_api* api, oc_gl_load_proc loadProc)
{
    api->name = "gles31";
    api->GetFloatv = loadProc("glGetFloatv");
    api->TexBufferRange = oc_glTexBufferRange_noimpl;
    api->IsBuffer = loadProc("glIsBuffer");
    api->IsTexture = loadProc("glIsTexture");
    api->DepthRangef = loadProc("glDepthRangef");
    api->EndConditionalRender = oc_glEndConditionalRender_noimpl;
    api->BlendFunci = oc_glBlendFunci_noimpl;
    api->GetProgramPipelineiv = loadProc("glGetProgramPipelineiv");
    api->WaitSync = loadProc("glWaitSync");
    api->ProgramUniformMatrix2fv = loadProc("glProgramUniformMatrix2fv");
    api->ProgramUniformMatrix4x3dv = oc_glProgramUniformMatrix4x3dv_noimpl;
    api->VertexAttrib1dv = oc_glVertexAttrib1dv_noimpl;
    api->SamplerParameteri = loadProc("glSamplerParameteri");
    api->GetVertexAttribIiv = loadProc("glGetVertexAttribIiv");
    api->GetSamplerParameterfv = loadProc("glGetSamplerParameterfv");
    api->VertexAttrib1d = oc_glVertexAttrib1d_noimpl;
    api->TexBuffer = oc_glTexBuffer_noimpl;
    api->InvalidateBufferData = oc_glInvalidateBufferData_noimpl;
    api->ProgramUniform2i = loadProc("glProgramUniform2i");
    api->Uniform4dv = oc_glUniform4dv_noimpl;
    api->UseProgram = loadProc("glUseProgram");
    api->VertexAttribI3iv = oc_glVertexAttribI3iv_noimpl;
    api->DrawElementsIndirect = loadProc("glDrawElementsIndirect");
    api->VertexAttrib4uiv = oc_glVertexAttrib4uiv_noimpl;
    api->GetQueryObjectiv = oc_glGetQueryObjectiv_noimpl;
    api->FramebufferRenderbuffer = loadProc("glFramebufferRenderbuffer");
    api->BlendEquationi = oc_glBlendEquationi_noimpl;
    api->GetActiveSubroutineName = oc_glGetActiveSubroutineName_noimpl;
    api->VertexAttrib2s = oc_glVertexAttrib2s_noimpl;
    api->VertexAttribL1d = oc_glVertexAttribL1d_noimpl;
    api->BindTextures = oc_glBindTextures_noimpl;
    api->VertexAttrib3sv = oc_glVertexAttrib3sv_noimpl;
    api->GetFloati_v = oc_glGetFloati_v_noimpl;
    api->BeginTransformFeedback = loadProc("glBeginTransformFeedback");
    api->ClearStencil = loadProc("glClearStencil");
    api->Uniform3i = loadProc("glUniform3i");
    api->ValidateProgramPipeline = loadProc("glValidateProgramPipeline");
    api->ProgramUniformMatrix4x2fv = loadProc("glProgramUniformMatrix4x2fv");
    api->VertexAttribI4ui = loadProc("glVertexAttribI4ui");
    api->GetShaderiv = loadProc("glGetShaderiv");
    api->ReadnPixels = oc_glReadnPixels_noimpl;
    api->UniformMatrix4x2fv = loadProc("glUniformMatrix4x2fv");
    api->GetShaderPrecisionFormat = loadProc("glGetShaderPrecisionFormat");
    api->ProgramUniformMatrix2x3fv = loadProc("glProgramUniformMatrix2x3fv");
    api->TexSubImage3D = loadProc("glTexSubImage3D");
    api->GetProgramResourceLocationIndex = oc_glGetProgramResourceLocationIndex_noimpl;
    api->BlendFunc = loadProc("glBlendFunc");
    api->ProgramUniformMatrix3x4fv = loadProc("glProgramUniformMatrix3x4fv");
    api->Uniform3d = oc_glUniform3d_noimpl;
    api->VertexAttrib1sv = oc_glVertexAttrib1sv_noimpl;
    api->BindFragDataLocation = oc_glBindFragDataLocation_noimpl;
    api->VertexAttrib4bv = oc_glVertexAttrib4bv_noimpl;
    api->Uniform4iv = loadProc("glUniform4iv");
    api->ProgramUniform2ui = loadProc("glProgramUniform2ui");
    api->DrawArrays = loadProc("glDrawArrays");
    api->ProgramBinary = loadProc("glProgramBinary");
    api->VertexAttrib4f = loadProc("glVertexAttrib4f");
    api->VertexAttribP2uiv = oc_glVertexAttribP2uiv_noimpl;
    api->UniformMatrix3fv = loadProc("glUniformMatrix3fv");
    api->Uniform2i = loadProc("glUniform2i");
    api->GetQueryObjectuiv = loadProc("glGetQueryObjectuiv");
    api->UniformBlockBinding = loadProc("glUniformBlockBinding");
    api->SampleCoverage = loadProc("glSampleCoverage");
    api->VertexAttrib4Nusv = oc_glVertexAttrib4Nusv_noimpl;
    api->ProgramUniformMatrix2x4dv = oc_glProgramUniformMatrix2x4dv_noimpl;
    api->Uniform3uiv = loadProc("glUniform3uiv");
    api->VertexAttrib1s = oc_glVertexAttrib1s_noimpl;
    api->GetVertexAttribPointerv = loadProc("glGetVertexAttribPointerv");
    api->BlendBarrier = oc_glBlendBarrier_noimpl;
    api->DrawRangeElements = loadProc("glDrawRangeElements");
    api->TexStorage3D = loadProc("glTexStorage3D");
    api->GetInternalformati64v = oc_glGetInternalformati64v_noimpl;
    api->GetQueryObjecti64v = oc_glGetQueryObjecti64v_noimpl;
    api->CompressedTexSubImage1D = oc_glCompressedTexSubImage1D_noimpl;
    api->VertexAttrib3dv = oc_glVertexAttrib3dv_noimpl;
    api->VertexBindingDivisor = loadProc("glVertexBindingDivisor");
    api->UseProgramStages = loadProc("glUseProgramStages");
    api->VertexAttribBinding = loadProc("glVertexAttribBinding");
    api->DebugMessageInsert = oc_glDebugMessageInsert_noimpl;
    api->GetTexParameteriv = loadProc("glGetTexParameteriv");
    api->MultiDrawArraysIndirect = oc_glMultiDrawArraysIndirect_noimpl;
    api->GetTexParameterfv = loadProc("glGetTexParameterfv");
    api->GetProgramPipelineInfoLog = loadProc("glGetProgramPipelineInfoLog");
    api->EndQuery = loadProc("glEndQuery");
    api->GetProgramResourceLocation = loadProc("glGetProgramResourceLocation");
    api->CompressedTexImage2D = loadProc("glCompressedTexImage2D");
    api->VertexAttribP2ui = oc_glVertexAttribP2ui_noimpl;
    api->IsEnabledi = oc_glIsEnabledi_noimpl;
    api->GetActiveAtomicCounterBufferiv = oc_glGetActiveAtomicCounterBufferiv_noimpl;
    api->IsProgram = loadProc("glIsProgram");
    api->Uniform1dv = oc_glUniform1dv_noimpl;
    api->TexParameteriv = loadProc("glTexParameteriv");
    api->Uniform2fv = loadProc("glUniform2fv");
    api->ReleaseShaderCompiler = loadProc("glReleaseShaderCompiler");
    api->CullFace = loadProc("glCullFace");
    api->VertexAttribI4i = loadProc("glVertexAttribI4i");
    api->GetProgramResourceIndex = loadProc("glGetProgramResourceIndex");
    api->ShaderBinary = loadProc("glShaderBinary");
    api->UniformMatrix3x2dv = oc_glUniformMatrix3x2dv_noimpl;
    api->InvalidateFramebuffer = loadProc("glInvalidateFramebuffer");
    api->AttachShader = loadProc("glAttachShader");
    api->FlushMappedBufferRange = loadProc("glFlushMappedBufferRange");
    api->VertexAttribP3uiv = oc_glVertexAttribP3uiv_noimpl;
    api->GetActiveUniformName = oc_glGetActiveUniformName_noimpl;
    api->MapBuffer = oc_glMapBuffer_noimpl;
    api->DrawBuffers = loadProc("glDrawBuffers");
    api->GetSynciv = loadProc("glGetSynciv");
    api->CopyTexSubImage2D = loadProc("glCopyTexSubImage2D");
    api->ObjectLabel = oc_glObjectLabel_noimpl;
    api->BufferSubData = loadProc("glBufferSubData");
    api->Uniform2f = loadProc("glUniform2f");
    api->DebugMessageCallback = oc_glDebugMessageCallback_noimpl;
    api->VertexAttribL4dv = oc_glVertexAttribL4dv_noimpl;
    api->IsProgramPipeline = loadProc("glIsProgramPipeline");
    api->ResumeTransformFeedback = loadProc("glResumeTransformFeedback");
    api->VertexAttribI4iv = loadProc("glVertexAttribI4iv");
    api->GetShaderInfoLog = loadProc("glGetShaderInfoLog");
    api->GetIntegeri_v = loadProc("glGetIntegeri_v");
    api->BindVertexBuffer = loadProc("glBindVertexBuffer");
    api->BlendEquation = loadProc("glBlendEquation");
    api->VertexAttribL2dv = oc_glVertexAttribL2dv_noimpl;
    api->VertexAttribI1ui = oc_glVertexAttribI1ui_noimpl;
    api->VertexAttrib4Nsv = oc_glVertexAttrib4Nsv_noimpl;
    api->VertexAttribL4d = oc_glVertexAttribL4d_noimpl;
    api->CopyImageSubData = oc_glCopyImageSubData_noimpl;
    api->GetFramebufferAttachmentParameteriv = loadProc("glGetFramebufferAttachmentParameteriv");
    api->VertexAttribL2d = oc_glVertexAttribL2d_noimpl;
    api->GetSubroutineIndex = oc_glGetSubroutineIndex_noimpl;
    api->VertexAttribI3uiv = oc_glVertexAttribI3uiv_noimpl;
    api->VertexAttrib4iv = oc_glVertexAttrib4iv_noimpl;
    api->BindVertexBuffers = oc_glBindVertexBuffers_noimpl;
    api->ProgramUniformMatrix2x3dv = oc_glProgramUniformMatrix2x3dv_noimpl;
    api->PrimitiveBoundingBox = oc_glPrimitiveBoundingBox_noimpl;
    api->Scissor = loadProc("glScissor");
    api->ClientWaitSync = loadProc("glClientWaitSync");
    api->Uniform3ui = loadProc("glUniform3ui");
    api->VertexAttribP3ui = oc_glVertexAttribP3ui_noimpl;
    api->Enable = loadProc("glEnable");
    api->StencilOpSeparate = loadProc("glStencilOpSeparate");
    api->UniformMatrix2x3dv = oc_glUniformMatrix2x3dv_noimpl;
    api->ProgramUniformMatrix3dv = oc_glProgramUniformMatrix3dv_noimpl;
    api->TexImage2DMultisample = oc_glTexImage2DMultisample_noimpl;
    api->VertexAttrib4Nbv = oc_glVertexAttrib4Nbv_noimpl;
    api->GetTexImage = oc_glGetTexImage_noimpl;
    api->VertexAttrib4sv = oc_glVertexAttrib4sv_noimpl;
    api->PixelStorei = loadProc("glPixelStorei");
    api->DepthMask = loadProc("glDepthMask");
    api->TexStorage2D = loadProc("glTexStorage2D");
    api->Clear = loadProc("glClear");
    api->UniformMatrix3x4dv = oc_glUniformMatrix3x4dv_noimpl;
    api->DeleteTransformFeedbacks = loadProc("glDeleteTransformFeedbacks");
    api->MapBufferRange = loadProc("glMapBufferRange");
    api->MemoryBarrier = loadProc("glMemoryBarrier");
    api->ViewportIndexedf = oc_glViewportIndexedf_noimpl;
    api->VertexAttrib3fv = loadProc("glVertexAttrib3fv");
    api->ObjectPtrLabel = oc_glObjectPtrLabel_noimpl;
    api->TexStorage1D = oc_glTexStorage1D_noimpl;
    api->CompressedTexImage3D = loadProc("glCompressedTexImage3D");
    api->VertexAttrib1fv = loadProc("glVertexAttrib1fv");
    api->VertexAttribPointer = loadProc("glVertexAttribPointer");
    api->GetQueryIndexediv = oc_glGetQueryIndexediv_noimpl;
    api->CompileShader = loadProc("glCompileShader");
    api->ProgramUniform1i = loadProc("glProgramUniform1i");
    api->GetQueryiv = loadProc("glGetQueryiv");
    api->VertexAttribI1iv = oc_glVertexAttribI1iv_noimpl;
    api->CopyTexImage2D = loadProc("glCopyTexImage2D");
    api->GetQueryObjectui64v = oc_glGetQueryObjectui64v_noimpl;
    api->PointSize = oc_glPointSize_noimpl;
    api->Disablei = oc_glDisablei_noimpl;
    api->VertexAttribL1dv = oc_glVertexAttribL1dv_noimpl;
    api->CreateShader = loadProc("glCreateShader");
    api->GetString = loadProc("glGetString");
    api->ViewportArrayv = oc_glViewportArrayv_noimpl;
    api->ProgramUniform3d = oc_glProgramUniform3d_noimpl;
    api->VertexAttrib4Nubv = oc_glVertexAttrib4Nubv_noimpl;
    api->TexParameteri = loadProc("glTexParameteri");
    api->ProgramUniform4fv = loadProc("glProgramUniform4fv");
    api->GenerateMipmap = loadProc("glGenerateMipmap");
    api->CompressedTexSubImage3D = loadProc("glCompressedTexSubImage3D");
    api->Uniform3f = loadProc("glUniform3f");
    api->GetUniformIndices = loadProc("glGetUniformIndices");
    api->VertexAttribLPointer = oc_glVertexAttribLPointer_noimpl;
    api->VertexAttribI2uiv = oc_glVertexAttribI2uiv_noimpl;
    api->QueryCounter = oc_glQueryCounter_noimpl;
    api->ActiveShaderProgram = loadProc("glActiveShaderProgram");
    api->Uniform1ui = loadProc("glUniform1ui");
    api->VertexAttribI1i = oc_glVertexAttribI1i_noimpl;
    api->GetTexParameterIiv = oc_glGetTexParameterIiv_noimpl;
    api->GetUniformfv = loadProc("glGetUniformfv");
    api->ProgramUniform2uiv = loadProc("glProgramUniform2uiv");
    api->GetError = loadProc("glGetError");
    api->GetActiveUniformBlockName = loadProc("glGetActiveUniformBlockName");
    api->TextureView = oc_glTextureView_noimpl;
    api->GetnUniformiv = oc_glGetnUniformiv_noimpl;
    api->ProgramUniform4dv = oc_glProgramUniform4dv_noimpl;
    api->ViewportIndexedfv = oc_glViewportIndexedfv_noimpl;
    api->Hint = loadProc("glHint");
    api->GetShaderSource = loadProc("glGetShaderSource");
    api->ProgramUniformMatrix4x3fv = loadProc("glProgramUniformMatrix4x3fv");
    api->Uniform1iv = loadProc("glUniform1iv");
    api->VertexAttribI4bv = oc_glVertexAttribI4bv_noimpl;
    api->UniformMatrix4x2dv = oc_glUniformMatrix4x2dv_noimpl;
    api->BufferStorage = oc_glBufferStorage_noimpl;
    api->IsRenderbuffer = loadProc("glIsRenderbuffer");
    api->GetActiveSubroutineUniformName = oc_glGetActiveSubroutineUniformName_noimpl;
    api->LinkProgram = loadProc("glLinkProgram");
    api->GetActiveUniformsiv = loadProc("glGetActiveUniformsiv");
    api->GetDebugMessageLog = oc_glGetDebugMessageLog_noimpl;
    api->CopyTexSubImage3D = loadProc("glCopyTexSubImage3D");
    api->PointParameteri = oc_glPointParameteri_noimpl;
    api->ProgramUniform3dv = oc_glProgramUniform3dv_noimpl;
    api->CompressedTexImage1D = oc_glCompressedTexImage1D_noimpl;
    api->UniformMatrix3x4fv = loadProc("glUniformMatrix3x4fv");
    api->GenSamplers = loadProc("glGenSamplers");
    api->GetCompressedTexImage = oc_glGetCompressedTexImage_noimpl;
    api->DeleteQueries = loadProc("glDeleteQueries");
    api->GenProgramPipelines = loadProc("glGenProgramPipelines");
    api->DispatchComputeIndirect = loadProc("glDispatchComputeIndirect");
    api->VertexAttribIPointer = loadProc("glVertexAttribIPointer");
    api->CreateProgram = loadProc("glCreateProgram");
    api->ClearTexSubImage = oc_glClearTexSubImage_noimpl;
    api->VertexAttrib4d = oc_glVertexAttrib4d_noimpl;
    api->FrontFace = loadProc("glFrontFace");
    api->BindTransformFeedback = loadProc("glBindTransformFeedback");
    api->GetProgramStageiv = oc_glGetProgramStageiv_noimpl;
    api->SamplerParameterIiv = oc_glSamplerParameterIiv_noimpl;
    api->GetInteger64v = loadProc("glGetInteger64v");
    api->CreateShaderProgramv = loadProc("glCreateShaderProgramv");
    api->BindBuffersRange = oc_glBindBuffersRange_noimpl;
    api->Uniform3fv = loadProc("glUniform3fv");
    api->ProgramUniformMatrix4fv = loadProc("glProgramUniformMatrix4fv");
    api->BindBuffersBase = oc_glBindBuffersBase_noimpl;
    api->ClearBufferfi = loadProc("glClearBufferfi");
    api->FramebufferTexture3D = oc_glFramebufferTexture3D_noimpl;
    api->Disable = loadProc("glDisable");
    api->ProgramUniform1iv = loadProc("glProgramUniform1iv");
    api->VertexAttribI2iv = oc_glVertexAttribI2iv_noimpl;
    api->DepthRangeIndexed = oc_glDepthRangeIndexed_noimpl;
    api->PatchParameteri = oc_glPatchParameteri_noimpl;
    api->GetUniformBlockIndex = loadProc("glGetUniformBlockIndex");
    api->MultiDrawArrays = oc_glMultiDrawArrays_noimpl;
    api->VertexAttribI4ubv = oc_glVertexAttribI4ubv_noimpl;
    api->BindBuffer = loadProc("glBindBuffer");
    api->VertexAttribI3i = oc_glVertexAttribI3i_noimpl;
    api->GetDoublev = oc_glGetDoublev_noimpl;
    api->DrawTransformFeedbackStream = oc_glDrawTransformFeedbackStream_noimpl;
    api->VertexAttribI4uiv = loadProc("glVertexAttribI4uiv");
    api->RenderbufferStorageMultisample = loadProc("glRenderbufferStorageMultisample");
    api->VertexAttribL3dv = oc_glVertexAttribL3dv_noimpl;
    api->StencilMaskSeparate = loadProc("glStencilMaskSeparate");
    api->ProgramUniform1d = oc_glProgramUniform1d_noimpl;
    api->Viewport = loadProc("glViewport");
    api->VertexAttribP1ui = oc_glVertexAttribP1ui_noimpl;
    api->VertexAttrib4dv = oc_glVertexAttrib4dv_noimpl;
    api->GenQueries = loadProc("glGenQueries");
    api->TexParameterIiv = oc_glTexParameterIiv_noimpl;
    api->ProgramUniform2d = oc_glProgramUniform2d_noimpl;
    api->ProgramUniform1uiv = loadProc("glProgramUniform1uiv");
    api->VertexAttrib4Nub = oc_glVertexAttrib4Nub_noimpl;
    api->IsVertexArray = loadProc("glIsVertexArray");
    api->ProgramUniform3f = loadProc("glProgramUniform3f");
    api->ProgramUniform3iv = loadProc("glProgramUniform3iv");
    api->GetProgramBinary = loadProc("glGetProgramBinary");
    api->BindRenderbuffer = loadProc("glBindRenderbuffer");
    api->BindFragDataLocationIndexed = oc_glBindFragDataLocationIndexed_noimpl;
    api->GetSamplerParameterIiv = oc_glGetSamplerParameterIiv_noimpl;
    api->VertexAttribDivisor = loadProc("glVertexAttribDivisor");
    api->ProgramUniformMatrix3x2dv = oc_glProgramUniformMatrix3x2dv_noimpl;
    api->FramebufferParameteri = loadProc("glFramebufferParameteri");
    api->GenTransformFeedbacks = loadProc("glGenTransformFeedbacks");
    api->DeleteSync = loadProc("glDeleteSync");
    api->ProgramUniform1ui = loadProc("glProgramUniform1ui");
    api->TexSubImage1D = oc_glTexSubImage1D_noimpl;
    api->ClearDepthf = loadProc("glClearDepthf");
    api->ReadPixels = loadProc("glReadPixels");
    api->VertexAttribI2i = oc_glVertexAttribI2i_noimpl;
    api->Finish = loadProc("glFinish");
    api->LineWidth = loadProc("glLineWidth");
    api->DeleteShader = loadProc("glDeleteShader");
    api->IsSampler = loadProc("glIsSampler");
    api->ProgramUniformMatrix4dv = oc_glProgramUniformMatrix4dv_noimpl;
    api->TransformFeedbackVaryings = loadProc("glTransformFeedbackVaryings");
    api->BeginConditionalRender = oc_glBeginConditionalRender_noimpl;
    api->BindSamplers = oc_glBindSamplers_noimpl;
    api->DeleteProgramPipelines = loadProc("glDeleteProgramPipelines");
    api->ColorMask = loadProc("glColorMask");
    api->TexParameterfv = loadProc("glTexParameterfv");
    api->PushDebugGroup = oc_glPushDebugGroup_noimpl;
    api->ClearBufferfv = loadProc("glClearBufferfv");
    api->IsEnabled = loadProc("glIsEnabled");
    api->VertexAttrib2f = loadProc("glVertexAttrib2f");
    api->ProgramUniform2f = loadProc("glProgramUniform2f");
    api->GetSamplerParameterIuiv = oc_glGetSamplerParameterIuiv_noimpl;
    api->GetInteger64i_v = loadProc("glGetInteger64i_v");
    api->Uniform2dv = oc_glUniform2dv_noimpl;
    api->GetBufferSubData = oc_glGetBufferSubData_noimpl;
    api->MultiDrawElementsIndirect = oc_glMultiDrawElementsIndirect_noimpl;
    api->ProgramParameteri = loadProc("glProgramParameteri");
    api->VertexAttribP4ui = oc_glVertexAttribP4ui_noimpl;
    api->SamplerParameterfv = loadProc("glSamplerParameterfv");
    api->PointParameterf = oc_glPointParameterf_noimpl;
    api->UniformMatrix2x4fv = loadProc("glUniformMatrix2x4fv");
    api->GenBuffers = loadProc("glGenBuffers");
    api->ProgramUniform2dv = oc_glProgramUniform2dv_noimpl;
    api->VertexAttribFormat = loadProc("glVertexAttribFormat");
    api->TexSubImage2D = loadProc("glTexSubImage2D");
    api->VertexAttrib4ubv = oc_glVertexAttrib4ubv_noimpl;
    api->GetGraphicsResetStatus = oc_glGetGraphicsResetStatus_noimpl;
    api->GetProgramInterfaceiv = loadProc("glGetProgramInterfaceiv");
    api->VertexAttribIFormat = loadProc("glVertexAttribIFormat");
    api->GetnUniformfv = oc_glGetnUniformfv_noimpl;
    api->DeleteProgram = loadProc("glDeleteProgram");
    api->ClampColor = oc_glClampColor_noimpl;
    api->DrawElementsInstancedBaseVertexBaseInstance = oc_glDrawElementsInstancedBaseVertexBaseInstance_noimpl;
    api->DrawElements = loadProc("glDrawElements");
    api->DebugMessageControl = oc_glDebugMessageControl_noimpl;
    api->GetRenderbufferParameteriv = loadProc("glGetRenderbufferParameteriv");
    api->DetachShader = loadProc("glDetachShader");
    api->GenFramebuffers = loadProc("glGenFramebuffers");
    api->ProvokingVertex = oc_glProvokingVertex_noimpl;
    api->SampleMaski = loadProc("glSampleMaski");
    api->EndQueryIndexed = oc_glEndQueryIndexed_noimpl;
    api->ProgramUniform1f = loadProc("glProgramUniform1f");
    api->BindFramebuffer = loadProc("glBindFramebuffer");
    api->BeginQueryIndexed = oc_glBeginQueryIndexed_noimpl;
    api->UniformSubroutinesuiv = oc_glUniformSubroutinesuiv_noimpl;
    api->GetUniformiv = loadProc("glGetUniformiv");
    api->FramebufferTexture = oc_glFramebufferTexture_noimpl;
    api->PointParameterfv = oc_glPointParameterfv_noimpl;
    api->IsTransformFeedback = loadProc("glIsTransformFeedback");
    api->CheckFramebufferStatus = loadProc("glCheckFramebufferStatus");
    api->ShaderSource = loadProc("glShaderSource");
    api->UniformMatrix2x4dv = oc_glUniformMatrix2x4dv_noimpl;
    api->BindImageTextures = oc_glBindImageTextures_noimpl;
    api->CopyTexImage1D = oc_glCopyTexImage1D_noimpl;
    api->UniformMatrix3dv = oc_glUniformMatrix3dv_noimpl;
    api->ProgramUniform1dv = oc_glProgramUniform1dv_noimpl;
    api->BlitFramebuffer = loadProc("glBlitFramebuffer");
    api->PopDebugGroup = oc_glPopDebugGroup_noimpl;
    api->TexParameterIuiv = oc_glTexParameterIuiv_noimpl;
    api->VertexAttrib2d = oc_glVertexAttrib2d_noimpl;
    api->TexImage1D = oc_glTexImage1D_noimpl;
    api->GetObjectPtrLabel = oc_glGetObjectPtrLabel_noimpl;
    api->StencilMask = loadProc("glStencilMask");
    api->BeginQuery = loadProc("glBeginQuery");
    api->UniformMatrix4fv = loadProc("glUniformMatrix4fv");
    api->IsSync = loadProc("glIsSync");
    api->Uniform3dv = oc_glUniform3dv_noimpl;
    api->ProgramUniform2fv = loadProc("glProgramUniform2fv");
    api->VertexAttribI4sv = oc_glVertexAttribI4sv_noimpl;
    api->ScissorArrayv = oc_glScissorArrayv_noimpl;
    api->VertexAttribP1uiv = oc_glVertexAttribP1uiv_noimpl;
    api->Uniform2uiv = loadProc("glUniform2uiv");
    api->DeleteBuffers = loadProc("glDeleteBuffers");
    api->ProgramUniform3ui = loadProc("glProgramUniform3ui");
    api->FramebufferTextureLayer = loadProc("glFramebufferTextureLayer");
    api->EndTransformFeedback = loadProc("glEndTransformFeedback");
    api->BlendFuncSeparatei = oc_glBlendFuncSeparatei_noimpl;
    api->DrawTransformFeedbackInstanced = oc_glDrawTransformFeedbackInstanced_noimpl;
    api->DrawRangeElementsBaseVertex = oc_glDrawRangeElementsBaseVertex_noimpl;
    api->VertexAttrib1f = loadProc("glVertexAttrib1f");
    api->GetUniformSubroutineuiv = oc_glGetUniformSubroutineuiv_noimpl;
    api->DisableVertexAttribArray = loadProc("glDisableVertexAttribArray");
    api->ProgramUniformMatrix3x2fv = loadProc("glProgramUniformMatrix3x2fv");
    api->VertexAttribI4usv = oc_glVertexAttribI4usv_noimpl;
    api->GetObjectLabel = oc_glGetObjectLabel_noimpl;
    api->BindAttribLocation = loadProc("glBindAttribLocation");
    api->Uniform1f = loadProc("glUniform1f");
    api->GetUniformdv = oc_glGetUniformdv_noimpl;
    api->GetUniformLocation = loadProc("glGetUniformLocation");
    api->GetSubroutineUniformLocation = oc_glGetSubroutineUniformLocation_noimpl;
    api->GetTexParameterIuiv = oc_glGetTexParameterIuiv_noimpl;
    api->SamplerParameterf = loadProc("glSamplerParameterf");
    api->VertexAttribL3d = oc_glVertexAttribL3d_noimpl;
    api->TexImage3DMultisample = oc_glTexImage3DMultisample_noimpl;
    api->TexImage3D = loadProc("glTexImage3D");
    api->RenderbufferStorage = loadProc("glRenderbufferStorage");
    api->EnableVertexAttribArray = loadProc("glEnableVertexAttribArray");
    api->VertexAttribP4uiv = oc_glVertexAttribP4uiv_noimpl;
    api->Uniform4d = oc_glUniform4d_noimpl;
    api->VertexAttrib4s = oc_glVertexAttrib4s_noimpl;
    api->DrawElementsInstancedBaseVertex = oc_glDrawElementsInstancedBaseVertex_noimpl;
    api->VertexAttrib3s = oc_glVertexAttrib3s_noimpl;
    api->ProgramUniform2iv = loadProc("glProgramUniform2iv");
    api->StencilFuncSeparate = loadProc("glStencilFuncSeparate");
    api->DeleteFramebuffers = loadProc("glDeleteFramebuffers");
    api->DepthRange = oc_glDepthRange_noimpl;
    api->UniformMatrix3x2fv = loadProc("glUniformMatrix3x2fv");
    api->ProgramUniformMatrix2dv = oc_glProgramUniformMatrix2dv_noimpl;
    api->ShaderStorageBlockBinding = oc_glShaderStorageBlockBinding_noimpl;
    api->ClearDepth = oc_glClearDepth_noimpl;
    api->VertexAttrib2dv = oc_glVertexAttrib2dv_noimpl;
    api->SamplerParameterIuiv = oc_glSamplerParameterIuiv_noimpl;
    api->GetVertexAttribLdv = oc_glGetVertexAttribLdv_noimpl;
    api->ProgramUniformMatrix3x4dv = oc_glProgramUniformMatrix3x4dv_noimpl;
    api->DepthRangeArrayv = oc_glDepthRangeArrayv_noimpl;
    api->GetActiveUniform = loadProc("glGetActiveUniform");
    api->PatchParameterfv = oc_glPatchParameterfv_noimpl;
    api->InvalidateTexImage = oc_glInvalidateTexImage_noimpl;
    api->VertexAttrib3f = loadProc("glVertexAttrib3f");
    api->ProgramUniform4iv = loadProc("glProgramUniform4iv");
    api->ProgramUniform4d = oc_glProgramUniform4d_noimpl;
    api->IsFramebuffer = loadProc("glIsFramebuffer");
    api->PixelStoref = oc_glPixelStoref_noimpl;
    api->ProgramUniform4uiv = loadProc("glProgramUniform4uiv");
    api->ProgramUniformMatrix4x2dv = oc_glProgramUniformMatrix4x2dv_noimpl;
    api->FenceSync = loadProc("glFenceSync");
    api->GetBufferParameteri64v = loadProc("glGetBufferParameteri64v");
    api->StencilOp = loadProc("glStencilOp");
    api->ClearBufferData = oc_glClearBufferData_noimpl;
    api->GetnUniformuiv = oc_glGetnUniformuiv_noimpl;
    api->GetProgramResourceiv = loadProc("glGetProgramResourceiv");
    api->GetVertexAttribdv = oc_glGetVertexAttribdv_noimpl;
    api->GetTransformFeedbackVarying = loadProc("glGetTransformFeedbackVarying");
    api->VertexAttrib2fv = loadProc("glVertexAttrib2fv");
    api->GetBooleani_v = loadProc("glGetBooleani_v");
    api->ColorMaski = oc_glColorMaski_noimpl;
    api->InvalidateBufferSubData = oc_glInvalidateBufferSubData_noimpl;
    api->UniformMatrix4dv = oc_glUniformMatrix4dv_noimpl;
    api->IsQuery = loadProc("glIsQuery");
    api->Uniform4ui = loadProc("glUniform4ui");
    api->Uniform4i = loadProc("glUniform4i");
    api->GetSamplerParameteriv = loadProc("glGetSamplerParameteriv");
    api->MultiDrawElementsBaseVertex = oc_glMultiDrawElementsBaseVertex_noimpl;
    api->VertexAttribI1uiv = oc_glVertexAttribI1uiv_noimpl;
    api->GetIntegerv = loadProc("glGetIntegerv");
    api->UniformMatrix2x3fv = loadProc("glUniformMatrix2x3fv");
    api->TexImage2D = loadProc("glTexImage2D");
    api->GetAttachedShaders = loadProc("glGetAttachedShaders");
    api->Uniform2d = oc_glUniform2d_noimpl;
    api->MemoryBarrierByRegion = loadProc("glMemoryBarrierByRegion");
    api->UniformMatrix2fv = loadProc("glUniformMatrix2fv");
    api->PrimitiveRestartIndex = oc_glPrimitiveRestartIndex_noimpl;
    api->GetVertexAttribiv = loadProc("glGetVertexAttribiv");
    api->GetAttribLocation = loadProc("glGetAttribLocation");
    api->TexStorage2DMultisample = loadProc("glTexStorage2DMultisample");
    api->CompressedTexSubImage2D = loadProc("glCompressedTexSubImage2D");
    api->GetVertexAttribfv = loadProc("glGetVertexAttribfv");
    api->GetBufferParameteriv = loadProc("glGetBufferParameteriv");
    api->TexParameterf = loadProc("glTexParameterf");
    api->FramebufferTexture2D = loadProc("glFramebufferTexture2D");
    api->GetActiveAttrib = loadProc("glGetActiveAttrib");
    api->InvalidateTexSubImage = oc_glInvalidateTexSubImage_noimpl;
    api->DeleteVertexArrays = loadProc("glDeleteVertexArrays");
    api->VertexAttribI2ui = oc_glVertexAttribI2ui_noimpl;
    api->PointParameteriv = oc_glPointParameteriv_noimpl;
    api->GetPointerv = oc_glGetPointerv_noimpl;
    api->Enablei = oc_glEnablei_noimpl;
    api->BindBufferRange = loadProc("glBindBufferRange");
    api->DrawArraysInstanced = loadProc("glDrawArraysInstanced");
    api->DeleteTextures = loadProc("glDeleteTextures");
    api->VertexAttrib4Niv = oc_glVertexAttrib4Niv_noimpl;
    api->MultiDrawElements = oc_glMultiDrawElements_noimpl;
    api->GetProgramiv = loadProc("glGetProgramiv");
    api->DepthFunc = loadProc("glDepthFunc");
    api->GenTextures = loadProc("glGenTextures");
    api->GetInternalformativ = loadProc("glGetInternalformativ");
    api->ProgramUniform3i = loadProc("glProgramUniform3i");
    api->ScissorIndexed = oc_glScissorIndexed_noimpl;
    api->VertexAttrib2sv = oc_glVertexAttrib2sv_noimpl;
    api->TexStorage3DMultisample = oc_glTexStorage3DMultisample_noimpl;
    api->Uniform2iv = loadProc("glUniform2iv");
    api->DrawArraysInstancedBaseInstance = oc_glDrawArraysInstancedBaseInstance_noimpl;
    api->VertexAttribI3ui = oc_glVertexAttribI3ui_noimpl;
    api->DeleteSamplers = loadProc("glDeleteSamplers");
    api->GenVertexArrays = loadProc("glGenVertexArrays");
    api->GetFramebufferParameteriv = loadProc("glGetFramebufferParameteriv");
    api->PolygonMode = oc_glPolygonMode_noimpl;
    api->ProgramUniformMatrix2x4fv = loadProc("glProgramUniformMatrix2x4fv");
    api->GetProgramResourceName = loadProc("glGetProgramResourceName");
    api->SamplerParameteriv = loadProc("glSamplerParameteriv");
    api->GetActiveSubroutineUniformiv = oc_glGetActiveSubroutineUniformiv_noimpl;
    api->GetStringi = loadProc("glGetStringi");
    api->VertexAttribLFormat = oc_glVertexAttribLFormat_noimpl;
    api->VertexAttrib3d = oc_glVertexAttrib3d_noimpl;
    api->BindVertexArray = loadProc("glBindVertexArray");
    api->UnmapBuffer = loadProc("glUnmapBuffer");
    api->DrawElementsInstancedBaseInstance = oc_glDrawElementsInstancedBaseInstance_noimpl;
    api->Uniform4uiv = loadProc("glUniform4uiv");
    api->FramebufferTexture1D = oc_glFramebufferTexture1D_noimpl;
    api->DrawTransformFeedbackStreamInstanced = oc_glDrawTransformFeedbackStreamInstanced_noimpl;
    api->StencilFunc = loadProc("glStencilFunc");
    api->ValidateProgram = loadProc("glValidateProgram");
    api->Flush = loadProc("glFlush");
    api->ProgramUniform3uiv = loadProc("glProgramUniform3uiv");
    api->DeleteRenderbuffers = loadProc("glDeleteRenderbuffers");
    api->VertexAttrib4fv = loadProc("glVertexAttrib4fv");
    api->UniformMatrix2dv = oc_glUniformMatrix2dv_noimpl;
    api->GetFragDataIndex = oc_glGetFragDataIndex_noimpl;
    api->Uniform3iv = loadProc("glUniform3iv");
    api->MinSampleShading = oc_glMinSampleShading_noimpl;
    api->GetBooleanv = loadProc("glGetBooleanv");
    api->GetMultisamplefv = loadProc("glGetMultisamplefv");
    api->GetVertexAttribIuiv = loadProc("glGetVertexAttribIuiv");
    api->GetProgramInfoLog = loadProc("glGetProgramInfoLog");
    api->Uniform4fv = loadProc("glUniform4fv");
    api->DrawBuffer = oc_glDrawBuffer_noimpl;
    api->Uniform1i = loadProc("glUniform1i");
    api->ProgramUniform4ui = loadProc("glProgramUniform4ui");
    api->ProgramUniformMatrix3fv = loadProc("glProgramUniformMatrix3fv");
    api->BlendEquationSeparate = loadProc("glBlendEquationSeparate");
    api->BindProgramPipeline = loadProc("glBindProgramPipeline");
    api->GetDoublei_v = oc_glGetDoublei_v_noimpl;
    api->BufferData = loadProc("glBufferData");
    api->ClearColor = loadProc("glClearColor");
    api->ProgramUniform4i = loadProc("glProgramUniform4i");
    api->GetTexLevelParameteriv = loadProc("glGetTexLevelParameteriv");
    api->GetActiveUniformBlockiv = loadProc("glGetActiveUniformBlockiv");
    api->ProgramUniform1fv = loadProc("glProgramUniform1fv");
    api->PauseTransformFeedback = loadProc("glPauseTransformFeedback");
    api->GetBufferPointerv = loadProc("glGetBufferPointerv");
    api->InvalidateSubFramebuffer = loadProc("glInvalidateSubFramebuffer");
    api->ScissorIndexedv = oc_glScissorIndexedv_noimpl;
    api->Uniform2ui = loadProc("glUniform2ui");
    api->BindTexture = loadProc("glBindTexture");
    api->DrawElementsInstanced = loadProc("glDrawElementsInstanced");
    api->ProgramUniform4f = loadProc("glProgramUniform4f");
    api->BindBufferBase = loadProc("glBindBufferBase");
    api->IsShader = loadProc("glIsShader");
    api->ClearBufferSubData = oc_glClearBufferSubData_noimpl;
    api->VertexAttrib4Nuiv = oc_glVertexAttrib4Nuiv_noimpl;
    api->DrawArraysIndirect = loadProc("glDrawArraysIndirect");
    api->VertexAttrib4usv = oc_glVertexAttrib4usv_noimpl;
    api->Uniform1d = oc_glUniform1d_noimpl;
    api->ClearTexImage = oc_glClearTexImage_noimpl;
    api->Uniform1uiv = loadProc("glUniform1uiv");
    api->BindSampler = loadProc("glBindSampler");
    api->GetTexLevelParameterfv = loadProc("glGetTexLevelParameterfv");
    api->ClearBufferiv = loadProc("glClearBufferiv");
    api->LogicOp = oc_glLogicOp_noimpl;
    api->ActiveTexture = loadProc("glActiveTexture");
    api->GetFragDataLocation = loadProc("glGetFragDataLocation");
    api->BlendColor = loadProc("glBlendColor");
    api->UniformMatrix4x3fv = loadProc("glUniformMatrix4x3fv");
    api->ProgramUniform3fv = loadProc("glProgramUniform3fv");
    api->Uniform1fv = loadProc("glUniform1fv");
    api->DrawElementsBaseVertex = oc_glDrawElementsBaseVertex_noimpl;
    api->Uniform4f = loadProc("glUniform4f");
    api->BlendEquationSeparatei = oc_glBlendEquationSeparatei_noimpl;
    api->BlendFuncSeparate = loadProc("glBlendFuncSeparate");
    api->ClearBufferuiv = loadProc("glClearBufferuiv");
    api->CopyTexSubImage1D = oc_glCopyTexSubImage1D_noimpl;
    api->DrawTransformFeedback = oc_glDrawTransformFeedback_noimpl;
    api->ReadBuffer = loadProc("glReadBuffer");
    api->CopyBufferSubData = loadProc("glCopyBufferSubData");
    api->GetUniformuiv = loadProc("glGetUniformuiv");
    api->PolygonOffset = loadProc("glPolygonOffset");
    api->DispatchCompute = loadProc("glDispatchCompute");
    api->BindImageTexture = loadProc("glBindImageTexture");
    api->UniformMatrix4x3dv = oc_glUniformMatrix4x3dv_noimpl;
    api->GenRenderbuffers = loadProc("glGenRenderbuffers");
}

void oc_gl_load_gles32(oc_gl_api* api, oc_gl_load_proc loadProc)
{
    api->name = "gles32";
    api->GetFloatv = loadProc("glGetFloatv");
    api->TexBufferRange = loadProc("glTexBufferRange");
    api->IsBuffer = loadProc("glIsBuffer");
    api->IsTexture = loadProc("glIsTexture");
    api->DepthRangef = loadProc("glDepthRangef");
    api->EndConditionalRender = oc_glEndConditionalRender_noimpl;
    api->BlendFunci = loadProc("glBlendFunci");
    api->GetProgramPipelineiv = loadProc("glGetProgramPipelineiv");
    api->WaitSync = loadProc("glWaitSync");
    api->ProgramUniformMatrix2fv = loadProc("glProgramUniformMatrix2fv");
    api->ProgramUniformMatrix4x3dv = oc_glProgramUniformMatrix4x3dv_noimpl;
    api->VertexAttrib1dv = oc_glVertexAttrib1dv_noimpl;
    api->SamplerParameteri = loadProc("glSamplerParameteri");
    api->GetVertexAttribIiv = loadProc("glGetVertexAttribIiv");
    api->GetSamplerParameterfv = loadProc("glGetSamplerParameterfv");
    api->VertexAttrib1d = oc_glVertexAttrib1d_noimpl;
    api->TexBuffer = loadProc("glTexBuffer");
    api->InvalidateBufferData = oc_glInvalidateBufferData_noimpl;
    api->ProgramUniform2i = loadProc("glProgramUniform2i");
    api->Uniform4dv = oc_glUniform4dv_noimpl;
    api->UseProgram = loadProc("glUseProgram");
    api->VertexAttribI3iv = oc_glVertexAttribI3iv_noimpl;
    api->DrawElementsIndirect = loadProc("glDrawElementsIndirect");
    api->VertexAttrib4uiv = oc_glVertexAttrib4uiv_noimpl;
    api->GetQueryObjectiv = oc_glGetQueryObjectiv_noimpl;
    api->FramebufferRenderbuffer = loadProc("glFramebufferRenderbuffer");
    api->BlendEquationi = loadProc("glBlendEquationi");
    api->GetActiveSubroutineName = oc_glGetActiveSubroutineName_noimpl;
    api->VertexAttrib2s = oc_glVertexAttrib2s_noimpl;
    api->VertexAttribL1d = oc_glVertexAttribL1d_noimpl;
    api->BindTextures = oc_glBindTextures_noimpl;
    api->VertexAttrib3sv = oc_glVertexAttrib3sv_noimpl;
    api->GetFloati_v = oc_glGetFloati_v_noimpl;
    api->BeginTransformFeedback = loadProc("glBeginTransformFeedback");
    api->ClearStencil = loadProc("glClearStencil");
    api->Uniform3i = loadProc("glUniform3i");
    api->ValidateProgramPipeline = loadProc("glValidateProgramPipeline");
    api->ProgramUniformMatrix4x2fv = loadProc("glProgramUniformMatrix4x2fv");
    api->VertexAttribI4ui = loadProc("glVertexAttribI4ui");
    api->GetShaderiv = loadProc("glGetShaderiv");
    api->ReadnPixels = loadProc("glReadnPixels");
    api->UniformMatrix4x2fv = loadProc("glUniformMatrix4x2fv");
    api->GetShaderPrecisionFormat = loadProc("glGetShaderPrecisionFormat");
    api->ProgramUniformMatrix2x3fv = loadProc("glProgramUniformMatrix2x3fv");
    api->TexSubImage3D = loadProc("glTexSubImage3D");
    api->GetProgramResourceLocationIndex = oc_glGetProgramResourceLocationIndex_noimpl;
    api->BlendFunc = loadProc("glBlendFunc");
    api->ProgramUniformMatrix3x4fv = loadProc("glProgramUniformMatrix3x4fv");
    api->Uniform3d = oc_glUniform3d_noimpl;
    api->VertexAttrib1sv = oc_glVertexAttrib1sv_noimpl;
    api->BindFragDataLocation = oc_glBindFragDataLocation_noimpl;
    api->VertexAttrib4bv = oc_glVertexAttrib4bv_noimpl;
    api->Uniform4iv = loadProc("glUniform4iv");
    api->ProgramUniform2ui = loadProc("glProgramUniform2ui");
    api->DrawArrays = loadProc("glDrawArrays");
    api->ProgramBinary = loadProc("glProgramBinary");
    api->VertexAttrib4f = loadProc("glVertexAttrib4f");
    api->VertexAttribP2uiv = oc_glVertexAttribP2uiv_noimpl;
    api->UniformMatrix3fv = loadProc("glUniformMatrix3fv");
    api->Uniform2i = loadProc("glUniform2i");
    api->GetQueryObjectuiv = loadProc("glGetQueryObjectuiv");
    api->UniformBlockBinding = loadProc("glUniformBlockBinding");
    api->SampleCoverage = loadProc("glSampleCoverage");
    api->VertexAttrib4Nusv = oc_glVertexAttrib4Nusv_noimpl;
    api->ProgramUniformMatrix2x4dv = oc_glProgramUniformMatrix2x4dv_noimpl;
    api->Uniform3uiv = loadProc("glUniform3uiv");
    api->VertexAttrib1s = oc_glVertexAttrib1s_noimpl;
    api->GetVertexAttribPointerv = loadProc("glGetVertexAttribPointerv");
    api->BlendBarrier = loadProc("glBlendBarrier");
    api->DrawRangeElements = loadProc("glDrawRangeElements");
    api->TexStorage3D = loadProc("glTexStorage3D");
    api->GetInternalformati64v = oc_glGetInternalformati64v_noimpl;
    api->GetQueryObjecti64v = oc_glGetQueryObjecti64v_noimpl;
    api->CompressedTexSubImage1D = oc_glCompressedTexSubImage1D_noimpl;
    api->VertexAttrib3dv = oc_glVertexAttrib3dv_noimpl;
    api->VertexBindingDivisor = loadProc("glVertexBindingDivisor");
    api->UseProgramStages = loadProc("glUseProgramStages");
    api->VertexAttribBinding = loadProc("glVertexAttribBinding");
    api->DebugMessageInsert = loadProc("glDebugMessageInsert");
    api->GetTexParameteriv = loadProc("glGetTexParameteriv");
    api->MultiDrawArraysIndirect = oc_glMultiDrawArraysIndirect_noimpl;
    api->GetTexParameterfv = loadProc("glGetTexParameterfv");
    api->GetProgramPipelineInfoLog = loadProc("glGetProgramPipelineInfoLog");
    api->EndQuery = loadProc("glEndQuery");
    api->GetProgramResourceLocation = loadProc("glGetProgramResourceLocation");
    api->CompressedTexImage2D = loadProc("glCompressedTexImage2D");
    api->VertexAttribP2ui = oc_glVertexAttribP2ui_noimpl;
    api->IsEnabledi = loadProc("glIsEnabledi");
    api->GetActiveAtomicCounterBufferiv = oc_glGetActiveAtomicCounterBufferiv_noimpl;
    api->IsProgram = loadProc("glIsProgram");
    api->Uniform1dv = oc_glUniform1dv_noimpl;
    api->TexParameteriv = loadProc("glTexParameteriv");
    api->Uniform2fv = loadProc("glUniform2fv");
    api->ReleaseShaderCompiler = loadProc("glReleaseShaderCompiler");
    api->CullFace = loadProc("glCullFace");
    api->VertexAttribI4i = loadProc("glVertexAttribI4i");
    api->GetProgramResourceIndex = loadProc("glGetProgramResourceIndex");
    api->ShaderBinary = loadProc("glShaderBinary");
    api->UniformMatrix3x2dv = oc_glUniformMatrix3x2dv_noimpl;
    api->InvalidateFramebuffer = loadProc("glInvalidateFramebuffer");
    api->AttachShader = loadProc("glAttachShader");
    api->FlushMappedBufferRange = loadProc("glFlushMappedBufferRange");
    api->VertexAttribP3uiv = oc_glVertexAttribP3uiv_noimpl;
    api->GetActiveUniformName = oc_glGetActiveUniformName_noimpl;
    api->MapBuffer = oc_glMapBuffer_noimpl;
    api->DrawBuffers = loadProc("glDrawBuffers");
    api->GetSynciv = loadProc("glGetSynciv");
    api->CopyTexSubImage2D = loadProc("glCopyTexSubImage2D");
    api->ObjectLabel = loadProc("glObjectLabel");
    api->BufferSubData = loadProc("glBufferSubData");
    api->Uniform2f = loadProc("glUniform2f");
    api->DebugMessageCallback = loadProc("glDebugMessageCallback");
    api->VertexAttribL4dv = oc_glVertexAttribL4dv_noimpl;
    api->IsProgramPipeline = loadProc("glIsProgramPipeline");
    api->ResumeTransformFeedback = loadProc("glResumeTransformFeedback");
    api->VertexAttribI4iv = loadProc("glVertexAttribI4iv");
    api->GetShaderInfoLog = loadProc("glGetShaderInfoLog");
    api->GetIntegeri_v = loadProc("glGetIntegeri_v");
    api->BindVertexBuffer = loadProc("glBindVertexBuffer");
    api->BlendEquation = loadProc("glBlendEquation");
    api->VertexAttribL2dv = oc_glVertexAttribL2dv_noimpl;
    api->VertexAttribI1ui = oc_glVertexAttribI1ui_noimpl;
    api->VertexAttrib4Nsv = oc_glVertexAttrib4Nsv_noimpl;
    api->VertexAttribL4d = oc_glVertexAttribL4d_noimpl;
    api->CopyImageSubData = loadProc("glCopyImageSubData");
    api->GetFramebufferAttachmentParameteriv = loadProc("glGetFramebufferAttachmentParameteriv");
    api->VertexAttribL2d = oc_glVertexAttribL2d_noimpl;
    api->GetSubroutineIndex = oc_glGetSubroutineIndex_noimpl;
    api->VertexAttribI3uiv = oc_glVertexAttribI3uiv_noimpl;
    api->VertexAttrib4iv = oc_glVertexAttrib4iv_noimpl;
    api->BindVertexBuffers = oc_glBindVertexBuffers_noimpl;
    api->ProgramUniformMatrix2x3dv = oc_glProgramUniformMatrix2x3dv_noimpl;
    api->PrimitiveBoundingBox = loadProc("glPrimitiveBoundingBox");
    api->Scissor = loadProc("glScissor");
    api->ClientWaitSync = loadProc("glClientWaitSync");
    api->Uniform3ui = loadProc("glUniform3ui");
    api->VertexAttribP3ui = oc_glVertexAttribP3ui_noimpl;
    api->Enable = loadProc("glEnable");
    api->StencilOpSeparate = loadProc("glStencilOpSeparate");
    api->UniformMatrix2x3dv = oc_glUniformMatrix2x3dv_noimpl;
    api->ProgramUniformMatrix3dv = oc_glProgramUniformMatrix3dv_noimpl;
    api->TexImage2DMultisample = oc_glTexImage2DMultisample_noimpl;
    api->VertexAttrib4Nbv = oc_glVertexAttrib4Nbv_noimpl;
    api->GetTexImage = oc_glGetTexImage_noimpl;
    api->VertexAttrib4sv = oc_glVertexAttrib4sv_noimpl;
    api->PixelStorei = loadProc("glPixelStorei");
    api->DepthMask = loadProc("glDepthMask");
    api->TexStorage2D = loadProc("glTexStorage2D");
    api->Clear = loadProc("glClear");
    api->UniformMatrix3x4dv = oc_glUniformMatrix3x4dv_noimpl;
    api->DeleteTransformFeedbacks = loadProc("glDeleteTransformFeedbacks");
    api->MapBufferRange = loadProc("glMapBufferRange");
    api->MemoryBarrier = loadProc("glMemoryBarrier");
    api->ViewportIndexedf = oc_glViewportIndexedf_noimpl;
    api->VertexAttrib3fv = loadProc("glVertexAttrib3fv");
    api->ObjectPtrLabel = loadProc("glObjectPtrLabel");
    api->TexStorage1D = oc_glTexStorage1D_noimpl;
    api->CompressedTexImage3D = loadProc("glCompressedTexImage3D");
    api->VertexAttrib1fv = loadProc("glVertexAttrib1fv");
    api->VertexAttribPointer = loadProc("glVertexAttribPointer");
    api->GetQueryIndexediv = oc_glGetQueryIndexediv_noimpl;
    api->CompileShader = loadProc("glCompileShader");
    api->ProgramUniform1i = loadProc("glProgramUniform1i");
    api->GetQueryiv = loadProc("glGetQueryiv");
    api->VertexAttribI1iv = oc_glVertexAttribI1iv_noimpl;
    api->CopyTexImage2D = loadProc("glCopyTexImage2D");
    api->GetQueryObjectui64v = oc_glGetQueryObjectui64v_noimpl;
    api->PointSize = oc_glPointSize_noimpl;
    api->Disablei = loadProc("glDisablei");
    api->VertexAttribL1dv = oc_glVertexAttribL1dv_noimpl;
    api->CreateShader = loadProc("glCreateShader");
    api->GetString = loadProc("glGetString");
    api->ViewportArrayv = oc_glViewportArrayv_noimpl;
    api->ProgramUniform3d = oc_glProgramUniform3d_noimpl;
    api->VertexAttrib4Nubv = oc_glVertexAttrib4Nubv_noimpl;
    api->TexParameteri = loadProc("glTexParameteri");
    api->ProgramUniform4fv = loadProc("glProgramUniform4fv");
    api->GenerateMipmap = loadProc("glGenerateMipmap");
    api->CompressedTexSubImage3D = loadProc("glCompressedTexSubImage3D");
    api->Uniform3f = loadProc("glUniform3f");
    api->GetUniformIndices = loadProc("glGetUniformIndices");
    api->VertexAttribLPointer = oc_glVertexAttribLPointer_noimpl;
    api->VertexAttribI2uiv = oc_glVertexAttribI2uiv_noimpl;
    api->QueryCounter = oc_glQueryCounter_noimpl;
    api->ActiveShaderProgram = loadProc("glActiveShaderProgram");
    api->Uniform1ui = loadProc("glUniform1ui");
    api->VertexAttribI1i = oc_glVertexAttribI1i_noimpl;
    api->GetTexParameterIiv = loadProc("glGetTexParameterIiv");
    api->GetUniformfv = loadProc("glGetUniformfv");
    api->ProgramUniform2uiv = loadProc("glProgramUniform2uiv");
    api->GetError = loadProc("glGetError");
    api->GetActiveUniformBlockName = loadProc("glGetActiveUniformBlockName");
    api->TextureView = oc_glTextureView_noimpl;
    api->GetnUniformiv = loadProc("glGetnUniformiv");
    api->ProgramUniform4dv = oc_glProgramUniform4dv_noimpl;
    api->ViewportIndexedfv = oc_glViewportIndexedfv_noimpl;
    api->Hint = loadProc("glHint");
    api->GetShaderSource = loadProc("glGetShaderSource");
    api->ProgramUniformMatrix4x3fv = loadProc("glProgramUniformMatrix4x3fv");
    api->Uniform1iv = loadProc("glUniform1iv");
    api->VertexAttribI4bv = oc_glVertexAttribI4bv_noimpl;
    api->UniformMatrix4x2dv = oc_glUniformMatrix4x2dv_noimpl;
    api->BufferStorage = oc_glBufferStorage_noimpl;
    api->IsRenderbuffer = loadProc("glIsRenderbuffer");
    api->GetActiveSubroutineUniformName = oc_glGetActiveSubroutineUniformName_noimpl;
    api->LinkProgram = loadProc("glLinkProgram");
    api->GetActiveUniformsiv = loadProc("glGetActiveUniformsiv");
    api->GetDebugMessageLog = loadProc("glGetDebugMessageLog");
    api->CopyTexSubImage3D = loadProc("glCopyTexSubImage3D");
    api->PointParameteri = oc_glPointParameteri_noimpl;
    api->ProgramUniform3dv = oc_glProgramUniform3dv_noimpl;
    api->CompressedTexImage1D = oc_glCompressedTexImage1D_noimpl;
    api->UniformMatrix3x4fv = loadProc("glUniformMatrix3x4fv");
    api->GenSamplers = loadProc("glGenSamplers");
    api->GetCompressedTexImage = oc_glGetCompressedTexImage_noimpl;
    api->DeleteQueries = loadProc("glDeleteQueries");
    api->GenProgramPipelines = loadProc("glGenProgramPipelines");
    api->DispatchComputeIndirect = loadProc("glDispatchComputeIndirect");
    api->VertexAttribIPointer = loadProc("glVertexAttribIPointer");
    api->CreateProgram = loadProc("glCreateProgram");
    api->ClearTexSubImage = oc_glClearTexSubImage_noimpl;
    api->VertexAttrib4d = oc_glVertexAttrib4d_noimpl;
    api->FrontFace = loadProc("glFrontFace");
    api->BindTransformFeedback = loadProc("glBindTransformFeedback");
    api->GetProgramStageiv = oc_glGetProgramStageiv_noimpl;
    api->SamplerParameterIiv = loadProc("glSamplerParameterIiv");
    api->GetInteger64v = loadProc("glGetInteger64v");
    api->CreateShaderProgramv = loadProc("glCreateShaderProgramv");
    api->BindBuffersRange = oc_glBindBuffersRange_noimpl;
    api->Uniform3fv = loadProc("glUniform3fv");
    api->ProgramUniformMatrix4fv = loadProc("glProgramUniformMatrix4fv");
    api->BindBuffersBase = oc_glBindBuffersBase_noimpl;
    api->ClearBufferfi = loadProc("glClearBufferfi");
    api->FramebufferTexture3D = oc_glFramebufferTexture3D_noimpl;
    api->Disable = loadProc("glDisable");
    api->ProgramUniform1iv = loadProc("glProgramUniform1iv");
    api->VertexAttribI2iv = oc_glVertexAttribI2iv_noimpl;
    api->DepthRangeIndexed = oc_glDepthRangeIndexed_noimpl;
    api->PatchParameteri = loadProc("glPatchParameteri");
    api->GetUniformBlockIndex = loadProc("glGetUniformBlockIndex");
    api->MultiDrawArrays = oc_glMultiDrawArrays_noimpl;
    api->VertexAttribI4ubv = oc_glVertexAttribI4ubv_noimpl;
    api->BindBuffer = loadProc("glBindBuffer");
    api->VertexAttribI3i = oc_glVertexAttribI3i_noimpl;
    api->GetDoublev = oc_glGetDoublev_noimpl;
    api->DrawTransformFeedbackStream = oc_glDrawTransformFeedbackStream_noimpl;
    api->VertexAttribI4uiv = loadProc("glVertexAttribI4uiv");
    api->RenderbufferStorageMultisample = loadProc("glRenderbufferStorageMultisample");
    api->VertexAttribL3dv = oc_glVertexAttribL3dv_noimpl;
    api->StencilMaskSeparate = loadProc("glStencilMaskSeparate");
    api->ProgramUniform1d = oc_glProgramUniform1d_noimpl;
    api->Viewport = loadProc("glViewport");
    api->VertexAttribP1ui = oc_glVertexAttribP1ui_noimpl;
    api->VertexAttrib4dv = oc_glVertexAttrib4dv_noimpl;
    api->GenQueries = loadProc("glGenQueries");
    api->TexParameterIiv = loadProc("glTexParameterIiv");
    api->ProgramUniform2d = oc_glProgramUniform2d_noimpl;
    api->ProgramUniform1uiv = loadProc("glProgramUniform1uiv");
    api->VertexAttrib4Nub = oc_glVertexAttrib4Nub_noimpl;
    api->IsVertexArray = loadProc("glIsVertexArray");
    api->ProgramUniform3f = loadProc("glProgramUniform3f");
    api->ProgramUniform3iv = loadProc("glProgramUniform3iv");
    api->GetProgramBinary = loadProc("glGetProgramBinary");
    api->BindRenderbuffer = loadProc("glBindRenderbuffer");
    api->BindFragDataLocationIndexed = oc_glBindFragDataLocationIndexed_noimpl;
    api->GetSamplerParameterIiv = loadProc("glGetSamplerParameterIiv");
    api->VertexAttribDivisor = loadProc("glVertexAttribDivisor");
    api->ProgramUniformMatrix3x2dv = oc_glProgramUniformMatrix3x2dv_noimpl;
    api->FramebufferParameteri = loadProc("glFramebufferParameteri");
    api->GenTransformFeedbacks = loadProc("glGenTransformFeedbacks");
    api->DeleteSync = loadProc("glDeleteSync");
    api->ProgramUniform1ui = loadProc("glProgramUniform1ui");
    api->TexSubImage1D = oc_glTexSubImage1D_noimpl;
    api->ClearDepthf = loadProc("glClearDepthf");
    api->ReadPixels = loadProc("glReadPixels");
    api->VertexAttribI2i = oc_glVertexAttribI2i_noimpl;
    api->Finish = loadProc("glFinish");
    api->LineWidth = loadProc("glLineWidth");
    api->DeleteShader = loadProc("glDeleteShader");
    api->IsSampler = loadProc("glIsSampler");
    api->ProgramUniformMatrix4dv = oc_glProgramUniformMatrix4dv_noimpl;
    api->TransformFeedbackVaryings = loadProc("glTransformFeedbackVaryings");
    api->BeginConditionalRender = oc_glBeginConditionalRender_noimpl;
    api->BindSamplers = oc_glBindSamplers_noimpl;
    api->DeleteProgramPipelines = loadProc("glDeleteProgramPipelines");
    api->ColorMask = loadProc("glColorMask");
    api->TexParameterfv = loadProc("glTexParameterfv");
    api->PushDebugGroup = loadProc("glPushDebugGroup");
    api->ClearBufferfv = loadProc("glClearBufferfv");
    api->IsEnabled = loadProc("glIsEnabled");
    api->VertexAttrib2f = loadProc("glVertexAttrib2f");
    api->ProgramUniform2f = loadProc("glProgramUniform2f");
    api->GetSamplerParameterIuiv = loadProc("glGetSamplerParameterIuiv");
    api->GetInteger64i_v = loadProc("glGetInteger64i_v");
    api->Uniform2dv = oc_glUniform2dv_noimpl;
    api->GetBufferSubData = oc_glGetBufferSubData_noimpl;
    api->MultiDrawElementsIndirect = oc_glMultiDrawElementsIndirect_noimpl;
    api->ProgramParameteri = loadProc("glProgramParameteri");
    api->VertexAttribP4ui = oc_glVertexAttribP4ui_noimpl;
    api->SamplerParameterfv = loadProc("glSamplerParameterfv");
    api->PointParameterf = oc_glPointParameterf_noimpl;
    api->UniformMatrix2x4fv = loadProc("glUniformMatrix2x4fv");
    api->GenBuffers = loadProc("glGenBuffers");
    api->ProgramUniform2dv = oc_glProgramUniform2dv_noimpl;
    api->VertexAttribFormat = loadProc("glVertexAttribFormat");
    api->TexSubImage2D = loadProc("glTexSubImage2D");
    api->VertexAttrib4ubv = oc_glVertexAttrib4ubv_noimpl;
    api->GetGraphicsResetStatus = loadProc("glGetGraphicsResetStatus");
    api->GetProgramInterfaceiv = loadProc("glGetProgramInterfaceiv");
    api->VertexAttribIFormat = loadProc("glVertexAttribIFormat");
    api->GetnUniformfv = loadProc("glGetnUniformfv");
    api->DeleteProgram = loadProc("glDeleteProgram");
    api->ClampColor = oc_glClampColor_noimpl;
    api->DrawElementsInstancedBaseVertexBaseInstance = oc_glDrawElementsInstancedBaseVertexBaseInstance_noimpl;
    api->DrawElements = loadProc("glDrawElements");
    api->DebugMessageControl = loadProc("glDebugMessageControl");
    api->GetRenderbufferParameteriv = loadProc("glGetRenderbufferParameteriv");
    api->DetachShader = loadProc("glDetachShader");
    api->GenFramebuffers = loadProc("glGenFramebuffers");
    api->ProvokingVertex = oc_glProvokingVertex_noimpl;
    api->SampleMaski = loadProc("glSampleMaski");
    api->EndQueryIndexed = oc_glEndQueryIndexed_noimpl;
    api->ProgramUniform1f = loadProc("glProgramUniform1f");
    api->BindFramebuffer = loadProc("glBindFramebuffer");
    api->BeginQueryIndexed = oc_glBeginQueryIndexed_noimpl;
    api->UniformSubroutinesuiv = oc_glUniformSubroutinesuiv_noimpl;
    api->GetUniformiv = loadProc("glGetUniformiv");
    api->FramebufferTexture = loadProc("glFramebufferTexture");
    api->PointParameterfv = oc_glPointParameterfv_noimpl;
    api->IsTransformFeedback = loadProc("glIsTransformFeedback");
    api->CheckFramebufferStatus = loadProc("glCheckFramebufferStatus");
    api->ShaderSource = loadProc("glShaderSource");
    api->UniformMatrix2x4dv = oc_glUniformMatrix2x4dv_noimpl;
    api->BindImageTextures = oc_glBindImageTextures_noimpl;
    api->CopyTexImage1D = oc_glCopyTexImage1D_noimpl;
    api->UniformMatrix3dv = oc_glUniformMatrix3dv_noimpl;
    api->ProgramUniform1dv = oc_glProgramUniform1dv_noimpl;
    api->BlitFramebuffer = loadProc("glBlitFramebuffer");
    api->PopDebugGroup = loadProc("glPopDebugGroup");
    api->TexParameterIuiv = loadProc("glTexParameterIuiv");
    api->VertexAttrib2d = oc_glVertexAttrib2d_noimpl;
    api->TexImage1D = oc_glTexImage1D_noimpl;
    api->GetObjectPtrLabel = loadProc("glGetObjectPtrLabel");
    api->StencilMask = loadProc("glStencilMask");
    api->BeginQuery = loadProc("glBeginQuery");
    api->UniformMatrix4fv = loadProc("glUniformMatrix4fv");
    api->IsSync = loadProc("glIsSync");
    api->Uniform3dv = oc_glUniform3dv_noimpl;
    api->ProgramUniform2fv = loadProc("glProgramUniform2fv");
    api->VertexAttribI4sv = oc_glVertexAttribI4sv_noimpl;
    api->ScissorArrayv = oc_glScissorArrayv_noimpl;
    api->VertexAttribP1uiv = oc_glVertexAttribP1uiv_noimpl;
    api->Uniform2uiv = loadProc("glUniform2uiv");
    api->DeleteBuffers = loadProc("glDeleteBuffers");
    api->ProgramUniform3ui = loadProc("glProgramUniform3ui");
    api->FramebufferTextureLayer = loadProc("glFramebufferTextureLayer");
    api->EndTransformFeedback = loadProc("glEndTransformFeedback");
    api->BlendFuncSeparatei = loadProc("glBlendFuncSeparatei");
    api->DrawTransformFeedbackInstanced = oc_glDrawTransformFeedbackInstanced_noimpl;
    api->DrawRangeElementsBaseVertex = loadProc("glDrawRangeElementsBaseVertex");
    api->VertexAttrib1f = loadProc("glVertexAttrib1f");
    api->GetUniformSubroutineuiv = oc_glGetUniformSubroutineuiv_noimpl;
    api->DisableVertexAttribArray = loadProc("glDisableVertexAttribArray");
    api->ProgramUniformMatrix3x2fv = loadProc("glProgramUniformMatrix3x2fv");
    api->VertexAttribI4usv = oc_glVertexAttribI4usv_noimpl;
    api->GetObjectLabel = loadProc("glGetObjectLabel");
    api->BindAttribLocation = loadProc("glBindAttribLocation");
    api->Uniform1f = loadProc("glUniform1f");
    api->GetUniformdv = oc_glGetUniformdv_noimpl;
    api->GetUniformLocation = loadProc("glGetUniformLocation");
    api->GetSubroutineUniformLocation = oc_glGetSubroutineUniformLocation_noimpl;
    api->GetTexParameterIuiv = loadProc("glGetTexParameterIuiv");
    api->SamplerParameterf = loadProc("glSamplerParameterf");
    api->VertexAttribL3d = oc_glVertexAttribL3d_noimpl;
    api->TexImage3DMultisample = oc_glTexImage3DMultisample_noimpl;
    api->TexImage3D = loadProc("glTexImage3D");
    api->RenderbufferStorage = loadProc("glRenderbufferStorage");
    api->EnableVertexAttribArray = loadProc("glEnableVertexAttribArray");
    api->VertexAttribP4uiv = oc_glVertexAttribP4uiv_noimpl;
    api->Uniform4d = oc_glUniform4d_noimpl;
    api->VertexAttrib4s = oc_glVertexAttrib4s_noimpl;
    api->DrawElementsInstancedBaseVertex = loadProc("glDrawElementsInstancedBaseVertex");
    api->VertexAttrib3s = oc_glVertexAttrib3s_noimpl;
    api->ProgramUniform2iv = loadProc("glProgramUniform2iv");
    api->StencilFuncSeparate = loadProc("glStencilFuncSeparate");
    api->DeleteFramebuffers = loadProc("glDeleteFramebuffers");
    api->DepthRange = oc_glDepthRange_noimpl;
    api->UniformMatrix3x2fv = loadProc("glUniformMatrix3x2fv");
    api->ProgramUniformMatrix2dv = oc_glProgramUniformMatrix2dv_noimpl;
    api->ShaderStorageBlockBinding = oc_glShaderStorageBlockBinding_noimpl;
    api->ClearDepth = oc_glClearDepth_noimpl;
    api->VertexAttrib2dv = oc_glVertexAttrib2dv_noimpl;
    api->SamplerParameterIuiv = loadProc("glSamplerParameterIuiv");
    api->GetVertexAttribLdv = oc_glGetVertexAttribLdv_noimpl;
    api->ProgramUniformMatrix3x4dv = oc_glProgramUniformMatrix3x4dv_noimpl;
    api->DepthRangeArrayv = oc_glDepthRangeArrayv_noimpl;
    api->GetActiveUniform = loadProc("glGetActiveUniform");
    api->PatchParameterfv = oc_glPatchParameterfv_noimpl;
    api->InvalidateTexImage = oc_glInvalidateTexImage_noimpl;
    api->VertexAttrib3f = loadProc("glVertexAttrib3f");
    api->ProgramUniform4iv = loadProc("glProgramUniform4iv");
    api->ProgramUniform4d = oc_glProgramUniform4d_noimpl;
    api->IsFramebuffer = loadProc("glIsFramebuffer");
    api->PixelStoref = oc_glPixelStoref_noimpl;
    api->ProgramUniform4uiv = loadProc("glProgramUniform4uiv");
    api->ProgramUniformMatrix4x2dv = oc_glProgramUniformMatrix4x2dv_noimpl;
    api->FenceSync = loadProc("glFenceSync");
    api->GetBufferParameteri64v = loadProc("glGetBufferParameteri64v");
    api->StencilOp = loadProc("glStencilOp");
    api->ClearBufferData = oc_glClearBufferData_noimpl;
    api->GetnUniformuiv = loadProc("glGetnUniformuiv");
    api->GetProgramResourceiv = loadProc("glGetProgramResourceiv");
    api->GetVertexAttribdv = oc_glGetVertexAttribdv_noimpl;
    api->GetTransformFeedbackVarying = loadProc("glGetTransformFeedbackVarying");
    api->VertexAttrib2fv = loadProc("glVertexAttrib2fv");
    api->GetBooleani_v = loadProc("glGetBooleani_v");
    api->ColorMaski = loadProc("glColorMaski");
    api->InvalidateBufferSubData = oc_glInvalidateBufferSubData_noimpl;
    api->UniformMatrix4dv = oc_glUniformMatrix4dv_noimpl;
    api->IsQuery = loadProc("glIsQuery");
    api->Uniform4ui = loadProc("glUniform4ui");
    api->Uniform4i = loadProc("glUniform4i");
    api->GetSamplerParameteriv = loadProc("glGetSamplerParameteriv");
    api->MultiDrawElementsBaseVertex = oc_glMultiDrawElementsBaseVertex_noimpl;
    api->VertexAttribI1uiv = oc_glVertexAttribI1uiv_noimpl;
    api->GetIntegerv = loadProc("glGetIntegerv");
    api->UniformMatrix2x3fv = loadProc("glUniformMatrix2x3fv");
    api->TexImage2D = loadProc("glTexImage2D");
    api->GetAttachedShaders = loadProc("glGetAttachedShaders");
    api->Uniform2d = oc_glUniform2d_noimpl;
    api->MemoryBarrierByRegion = loadProc("glMemoryBarrierByRegion");
    api->UniformMatrix2fv = loadProc("glUniformMatrix2fv");
    api->PrimitiveRestartIndex = oc_glPrimitiveRestartIndex_noimpl;
    api->GetVertexAttribiv = loadProc("glGetVertexAttribiv");
    api->GetAttribLocation = loadProc("glGetAttribLocation");
    api->TexStorage2DMultisample = loadProc("glTexStorage2DMultisample");
    api->CompressedTexSubImage2D = loadProc("glCompressedTexSubImage2D");
    api->GetVertexAttribfv = loadProc("glGetVertexAttribfv");
    api->GetBufferParameteriv = loadProc("glGetBufferParameteriv");
    api->TexParameterf = loadProc("glTexParameterf");
    api->FramebufferTexture2D = loadProc("glFramebufferTexture2D");
    api->GetActiveAttrib = loadProc("glGetActiveAttrib");
    api->InvalidateTexSubImage = oc_glInvalidateTexSubImage_noimpl;
    api->DeleteVertexArrays = loadProc("glDeleteVertexArrays");
    api->VertexAttribI2ui = oc_glVertexAttribI2ui_noimpl;
    api->PointParameteriv = oc_glPointParameteriv_noimpl;
    api->GetPointerv = loadProc("glGetPointerv");
    api->Enablei = loadProc("glEnablei");
    api->BindBufferRange = loadProc("glBindBufferRange");
    api->DrawArraysInstanced = loadProc("glDrawArraysInstanced");
    api->DeleteTextures = loadProc("glDeleteTextures");
    api->VertexAttrib4Niv = oc_glVertexAttrib4Niv_noimpl;
    api->MultiDrawElements = oc_glMultiDrawElements_noimpl;
    api->GetProgramiv = loadProc("glGetProgramiv");
    api->DepthFunc = loadProc("glDepthFunc");
    api->GenTextures = loadProc("glGenTextures");
    api->GetInternalformativ = loadProc("glGetInternalformativ");
    api->ProgramUniform3i = loadProc("glProgramUniform3i");
    api->ScissorIndexed = oc_glScissorIndexed_noimpl;
    api->VertexAttrib2sv = oc_glVertexAttrib2sv_noimpl;
    api->TexStorage3DMultisample = loadProc("glTexStorage3DMultisample");
    api->Uniform2iv = loadProc("glUniform2iv");
    api->DrawArraysInstancedBaseInstance = oc_glDrawArraysInstancedBaseInstance_noimpl;
    api->VertexAttribI3ui = oc_glVertexAttribI3ui_noimpl;
    api->DeleteSamplers = loadProc("glDeleteSamplers");
    api->GenVertexArrays = loadProc("glGenVertexArrays");
    api->GetFramebufferParameteriv = loadProc("glGetFramebufferParameteriv");
    api->PolygonMode = oc_glPolygonMode_noimpl;
    api->ProgramUniformMatrix2x4fv = loadProc("glProgramUniformMatrix2x4fv");
    api->GetProgramResourceName = loadProc("glGetProgramResourceName");
    api->SamplerParameteriv = loadProc("glSamplerParameteriv");
    api->GetActiveSubroutineUniformiv = oc_glGetActiveSubroutineUniformiv_noimpl;
    api->GetStringi = loadProc("glGetStringi");
    api->VertexAttribLFormat = oc_glVertexAttribLFormat_noimpl;
    api->VertexAttrib3d = oc_glVertexAttrib3d_noimpl;
    api->BindVertexArray = loadProc("glBindVertexArray");
    api->UnmapBuffer = loadProc("glUnmapBuffer");
    api->DrawElementsInstancedBaseInstance = oc_glDrawElementsInstancedBaseInstance_noimpl;
    api->Uniform4uiv = loadProc("glUniform4uiv");
    api->FramebufferTexture1D = oc_glFramebufferTexture1D_noimpl;
    api->DrawTransformFeedbackStreamInstanced = oc_glDrawTransformFeedbackStreamInstanced_noimpl;
    api->StencilFunc = loadProc("glStencilFunc");
    api->ValidateProgram = loadProc("glValidateProgram");
    api->Flush = loadProc("glFlush");
    api->ProgramUniform3uiv = loadProc("glProgramUniform3uiv");
    api->DeleteRenderbuffers = loadProc("glDeleteRenderbuffers");
    api->VertexAttrib4fv = loadProc("glVertexAttrib4fv");
    api->UniformMatrix2dv = oc_glUniformMatrix2dv_noimpl;
    api->GetFragDataIndex = oc_glGetFragDataIndex_noimpl;
    api->Uniform3iv = loadProc("glUniform3iv");
    api->MinSampleShading = loadProc("glMinSampleShading");
    api->GetBooleanv = loadProc("glGetBooleanv");
    api->GetMultisamplefv = loadProc("glGetMultisamplefv");
    api->GetVertexAttribIuiv = loadProc("glGetVertexAttribIuiv");
    api->GetProgramInfoLog = loadProc("glGetProgramInfoLog");
    api->Uniform4fv = loadProc("glUniform4fv");
    api->DrawBuffer = oc_glDrawBuffer_noimpl;
    api->Uniform1i = loadProc("glUniform1i");
    api->ProgramUniform4ui = loadProc("glProgramUniform4ui");
    api->ProgramUniformMatrix3fv = loadProc("glProgramUniformMatrix3fv");
    api->BlendEquationSeparate = loadProc("glBlendEquationSeparate");
    api->BindProgramPipeline = loadProc("glBindProgramPipeline");
    api->GetDoublei_v = oc_glGetDoublei_v_noimpl;
    api->BufferData = loadProc("glBufferData");
    api->ClearColor = loadProc("glClearColor");
    api->ProgramUniform4i = loadProc("glProgramUniform4i");
    api->GetTexLevelParameteriv = loadProc("glGetTexLevelParameteriv");
    api->GetActiveUniformBlockiv = loadProc("glGetActiveUniformBlockiv");
    api->ProgramUniform1fv = loadProc("glProgramUniform1fv");
    api->PauseTransformFeedback = loadProc("glPauseTransformFeedback");
    api->GetBufferPointerv = loadProc("glGetBufferPointerv");
    api->InvalidateSubFramebuffer = loadProc("glInvalidateSubFramebuffer");
    api->ScissorIndexedv = oc_glScissorIndexedv_noimpl;
    api->Uniform2ui = loadProc("glUniform2ui");
    api->BindTexture = loadProc("glBindTexture");
    api->DrawElementsInstanced = loadProc("glDrawElementsInstanced");
    api->ProgramUniform4f = loadProc("glProgramUniform4f");
    api->BindBufferBase = loadProc("glBindBufferBase");
    api->IsShader = loadProc("glIsShader");
    api->ClearBufferSubData = oc_glClearBufferSubData_noimpl;
    api->VertexAttrib4Nuiv = oc_glVertexAttrib4Nuiv_noimpl;
    api->DrawArraysIndirect = loadProc("glDrawArraysIndirect");
    api->VertexAttrib4usv = oc_glVertexAttrib4usv_noimpl;
    api->Uniform1d = oc_glUniform1d_noimpl;
    api->ClearTexImage = oc_glClearTexImage_noimpl;
    api->Uniform1uiv = loadProc("glUniform1uiv");
    api->BindSampler = loadProc("glBindSampler");
    api->GetTexLevelParameterfv = loadProc("glGetTexLevelParameterfv");
    api->ClearBufferiv = loadProc("glClearBufferiv");
    api->LogicOp = oc_glLogicOp_noimpl;
    api->ActiveTexture = loadProc("glActiveTexture");
    api->GetFragDataLocation = loadProc("glGetFragDataLocation");
    api->BlendColor = loadProc("glBlendColor");
    api->UniformMatrix4x3fv = loadProc("glUniformMatrix4x3fv");
    api->ProgramUniform3fv = loadProc("glProgramUniform3fv");
    api->Uniform1fv = loadProc("glUniform1fv");
    api->DrawElementsBaseVertex = loadProc("glDrawElementsBaseVertex");
    api->Uniform4f = loadProc("glUniform4f");
    api->BlendEquationSeparatei = loadProc("glBlendEquationSeparatei");
    api->BlendFuncSeparate = loadProc("glBlendFuncSeparate");
    api->ClearBufferuiv = loadProc("glClearBufferuiv");
    api->CopyTexSubImage1D = oc_glCopyTexSubImage1D_noimpl;
    api->DrawTransformFeedback = oc_glDrawTransformFeedback_noimpl;
    api->ReadBuffer = loadProc("glReadBuffer");
    api->CopyBufferSubData = loadProc("glCopyBufferSubData");
    api->GetUniformuiv = loadProc("glGetUniformuiv");
    api->PolygonOffset = loadProc("glPolygonOffset");
    api->DispatchCompute = loadProc("glDispatchCompute");
    api->BindImageTexture = loadProc("glBindImageTexture");
    api->UniformMatrix4x3dv = oc_glUniformMatrix4x3dv_noimpl;
    api->GenRenderbuffers = loadProc("glGenRenderbuffers");
}

void oc_gl_select_api(oc_gl_api* api) { oc_glAPI = api; }

void oc_gl_deselect_api() { oc_glAPI = &oc_glNoAPI; }

oc_gl_api* oc_gl_get_api(void) { return (oc_glAPI); }
