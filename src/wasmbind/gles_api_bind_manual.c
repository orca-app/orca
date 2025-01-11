/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

//------------------------------------------------------------------------
// Manual pointer size checking functions
//------------------------------------------------------------------------

u64 orca_gl_type_size(GLenum type)
{
    u64 size = 8;
    switch(type)
    {
        case GL_UNSIGNED_BYTE:
        case GL_BYTE:
            size = sizeof(GLbyte);
            break;

        case GL_UNSIGNED_SHORT:
        case GL_SHORT:
        case GL_HALF_FLOAT:
            size = sizeof(GLshort);
            break;

        case GL_UNSIGNED_INT:
        case GL_INT:
        case GL_FIXED:
        case GL_INT_2_10_10_10_REV:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
            size = sizeof(GLint);
            break;

        case GL_FLOAT:
            size = sizeof(GLfloat);
            break;

        case GL_DOUBLE:
            size = sizeof(GLdouble);
            break;

        default:
            OC_ASSERT(0, "unknown GLenum type %i", type);
    }

    return (size);
}

u64 orca_gl_format_count(GLenum format)
{
    u64 count = 4;
    switch(format)
    {
        case GL_RED:
        case GL_RED_INTEGER:
        case GL_DEPTH_COMPONENT:
        case GL_STENCIL_INDEX:
        case GL_LUMINANCE:
        case GL_ALPHA:
            count = 1;
            break;

        case GL_RG:
        case GL_RG_INTEGER:
        case GL_DEPTH_STENCIL:
        case GL_LUMINANCE_ALPHA:
            count = 2;
            break;

        case GL_RGB:
        case GL_RGB_INTEGER:
            count = 3;
            break;

        case GL_RGBA:
        case GL_RGBA_INTEGER:
            count = 4;
            break;

        default:
            OC_ASSERT(0, "unknow GLenum format %i", format);
    }

    return (count);
}

typedef struct orca_gles_impl_limits
{
    bool init;
    int maxDrawBuffers;
    int numCompressedTextureFormats;
    int numProgramBinaryFormats;
    int numShaderBinaryFormats;
    //...
} orca_gles_impl_limits;

orca_gles_impl_limits __orcaGLESImplLimits = { 0 };

u64 orca_glGet_data_length(GLenum pname)
{
    if(!__orcaGLESImplLimits.init)
    {
        glGetIntegerv(GL_MAX_DRAW_BUFFERS, &__orcaGLESImplLimits.maxDrawBuffers);
        glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &__orcaGLESImplLimits.numCompressedTextureFormats);
        glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &__orcaGLESImplLimits.numProgramBinaryFormats);
        glGetIntegerv(GL_NUM_SHADER_BINARY_FORMATS, &__orcaGLESImplLimits.numShaderBinaryFormats);
    }
    u64 count = 8;

    if(pname >= GL_DRAW_BUFFER0 && pname < GL_DRAW_BUFFER0 + __orcaGLESImplLimits.maxDrawBuffers)
    {
        count = 1;
    }
    else
    {
        switch(pname)
        {
            case GL_ACTIVE_TEXTURE:
            case GL_ALPHA_BITS:
            case GL_ARRAY_BUFFER_BINDING:
            case GL_BLEND:
            case GL_BLEND_DST_ALPHA:
            case GL_BLEND_DST_RGB:
            case GL_BLEND_EQUATION_ALPHA:
            case GL_BLEND_EQUATION_RGB:
            case GL_BLEND_SRC_ALPHA:
            case GL_BLEND_SRC_RGB:
            case GL_BLUE_BITS:
            case GL_CONTEXT_FLAGS:
            case GL_CONTEXT_ROBUST_ACCESS:
            case GL_COPY_READ_BUFFER_BINDING:
            case GL_COPY_WRITE_BUFFER_BINDING:
            case GL_CULL_FACE:
            case GL_CULL_FACE_MODE:
            case GL_CURRENT_PROGRAM:
            case GL_DEBUG_GROUP_STACK_DEPTH:
            case GL_DEBUG_LOGGED_MESSAGES:
            case GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH:
            case GL_DEPTH_BITS:
            case GL_DEPTH_CLEAR_VALUE:
            case GL_DEPTH_FUNC:
            case GL_DEPTH_TEST:
            case GL_DEPTH_WRITEMASK:
            case GL_DISPATCH_INDIRECT_BUFFER_BINDING:
            case GL_DITHER:
            case GL_DRAW_FRAMEBUFFER_BINDING:
            case GL_ELEMENT_ARRAY_BUFFER_BINDING:
            case GL_FRAGMENT_INTERPOLATION_OFFSET_BITS:
            case GL_FRAGMENT_SHADER_DERIVATIVE_HINT:
            case GL_FRONT_FACE:
            case GL_GENERATE_MIPMAP_HINT:
            case GL_GREEN_BITS:
            case GL_IMAGE_BINDING_LAYERED:
            case GL_IMPLEMENTATION_COLOR_READ_FORMAT:
            case GL_IMPLEMENTATION_COLOR_READ_TYPE:
            case GL_LAYER_PROVOKING_VERTEX:
            case GL_LINE_WIDTH:
            case GL_MAJOR_VERSION:
            case GL_MAX_3D_TEXTURE_SIZE:
            case GL_MAX_ARRAY_TEXTURE_LAYERS:
            case GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS:
            case GL_MAX_COLOR_ATTACHMENTS:

            case GL_MAX_COMBINED_ATOMIC_COUNTERS:
            case GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS:
            case GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS:
            case GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS:
            case GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS:
            case GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS:
            case GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS:
            case GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS:
            case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:
            case GL_MAX_COMBINED_UNIFORM_BLOCKS:
            case GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS:
            case GL_MAX_COMPUTE_ATOMIC_COUNTERS:
            case GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS:
            case GL_MAX_COMPUTE_IMAGE_UNIFORMS:
            case GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS:
            case GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS:
            case GL_MAX_COMPUTE_UNIFORM_BLOCKS:
            case GL_MAX_COMPUTE_UNIFORM_COMPONENTS:
            case GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS:
            case GL_MAX_COMPUTE_WORK_GROUP_COUNT:
            case GL_MAX_COMPUTE_WORK_GROUP_SIZE:
            case GL_MAX_CUBE_MAP_TEXTURE_SIZE:
            case GL_MAX_DEBUG_GROUP_STACK_DEPTH:
            case GL_MAX_DEBUG_LOGGED_MESSAGES:
            case GL_MAX_DEBUG_MESSAGE_LENGTH:
            case GL_MAX_DEPTH_TEXTURE_SAMPLES:
            case GL_MAX_DRAW_BUFFERS:
            case GL_MAX_ELEMENT_INDEX:
            case GL_MAX_ELEMENTS_INDICES:
            case GL_MAX_ELEMENTS_VERTICES:
            case GL_MAX_FRAGMENT_ATOMIC_COUNTERS:
            case GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS:
            case GL_MAX_FRAGMENT_IMAGE_UNIFORMS:
            case GL_MAX_FRAGMENT_INPUT_COMPONENTS:
            case GL_MAX_FRAGMENT_INTERPOLATION_OFFSET:
            case GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS:
            case GL_MAX_FRAGMENT_UNIFORM_BLOCKS:
            case GL_MAX_FRAGMENT_UNIFORM_COMPONENTS:
            case GL_MAX_FRAGMENT_UNIFORM_VECTORS:
            case GL_MAX_FRAMEBUFFER_HEIGHT:
            case GL_MAX_FRAMEBUFFER_LAYERS:
            case GL_MAX_FRAMEBUFFER_SAMPLES:
            case GL_MAX_FRAMEBUFFER_WIDTH:
            case GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS:
            case GL_MAX_GEOMETRY_ATOMIC_COUNTERS:
            case GL_MAX_GEOMETRY_IMAGE_UNIFORMS:
            case GL_MAX_GEOMETRY_INPUT_COMPONENTS:
            case GL_MAX_GEOMETRY_OUTPUT_COMPONENTS:
            case GL_MAX_GEOMETRY_OUTPUT_VERTICES:
            case GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS:
            case GL_MAX_GEOMETRY_SHADER_INVOCATIONS:
            case GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS:
            case GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS:
            case GL_MAX_GEOMETRY_UNIFORM_BLOCKS:
            case GL_MAX_GEOMETRY_UNIFORM_COMPONENTS:
            case GL_MAX_INTEGER_SAMPLES:
            case GL_MAX_LABEL_LENGTH:
            case GL_MAX_PROGRAM_TEXEL_OFFSET:
            case GL_MAX_RENDERBUFFER_SIZE:
            case GL_MAX_SAMPLE_MASK_WORDS:
            case GL_MAX_SAMPLES:
            case GL_MAX_SERVER_WAIT_TIMEOUT:
            case GL_MAX_SHADER_STORAGE_BLOCK_SIZE:
            case GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS:
            case GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS:
            case GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS:
            case GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS:
            case GL_MAX_TESS_CONTROL_INPUT_COMPONENTS:
            case GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS:
            case GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS:
            case GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS:
            case GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS:
            case GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS:
            case GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS:
            case GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS:
            case GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS:
            case GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS:
            case GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS:
            case GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS:
            case GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS:
            case GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS:
            case GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS:
            case GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS:
            case GL_MAX_TESS_GEN_LEVEL:
            case GL_MAX_TESS_PATCH_COMPONENTS:
            case GL_MAX_TEXTURE_BUFFER_SIZE:
            case GL_MAX_TEXTURE_IMAGE_UNITS:
            case GL_MAX_TEXTURE_LOD_BIAS:
            case GL_MAX_TEXTURE_SIZE:
            case GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS:
            case GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS:
            case GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS:
            case GL_MAX_UNIFORM_BLOCK_SIZE:
            case GL_MAX_UNIFORM_BUFFER_BINDINGS:
            case GL_MAX_UNIFORM_LOCATIONS:
            case GL_MAX_VARYING_COMPONENTS:
            case GL_MAX_VARYING_VECTORS:
            case GL_MAX_VERTEX_ATOMIC_COUNTERS:
            case GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS:
            case GL_MAX_VERTEX_ATTRIB_BINDINGS:
            case GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET:
            case GL_MAX_VERTEX_ATTRIBS:
            case GL_MAX_VERTEX_IMAGE_UNIFORMS:
            case GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS:
            case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS:
            case GL_MAX_VERTEX_OUTPUT_COMPONENTS:
            case GL_MAX_VERTEX_UNIFORM_BLOCKS:
            case GL_MAX_VERTEX_UNIFORM_COMPONENTS:
            case GL_MAX_VERTEX_UNIFORM_VECTORS:
            case GL_MIN_FRAGMENT_INTERPOLATION_OFFSET:
            case GL_MIN_PROGRAM_TEXEL_OFFSET:
            case GL_MIN_SAMPLE_SHADING_VALUE:
            case GL_MINOR_VERSION:
            case GL_NUM_COMPRESSED_TEXTURE_FORMATS:
            case GL_NUM_EXTENSIONS:
            case GL_NUM_PROGRAM_BINARY_FORMATS:
            case GL_NUM_SHADER_BINARY_FORMATS:
            case GL_PACK_ALIGNMENT:
            case GL_PACK_ROW_LENGTH:
            case GL_PACK_SKIP_PIXELS:
            case GL_PACK_SKIP_ROWS:
            case GL_PATCH_VERTICES:
            case GL_PIXEL_PACK_BUFFER_BINDING:
            case GL_PIXEL_UNPACK_BUFFER_BINDING:
            case GL_POLYGON_OFFSET_FACTOR:
            case GL_POLYGON_OFFSET_FILL:
            case GL_POLYGON_OFFSET_UNITS:
            case GL_PRIMITIVE_RESTART_FIXED_INDEX:
            case GL_PROGRAM_PIPELINE_BINDING:
            case GL_RASTERIZER_DISCARD:
            case GL_READ_BUFFER:
            case GL_READ_FRAMEBUFFER_BINDING:
            case GL_RED_BITS:
            case GL_RENDERBUFFER_BINDING:
            case GL_RESET_NOTIFICATION_STRATEGY:
            case GL_SAMPLE_ALPHA_TO_COVERAGE:
            case GL_SAMPLE_BUFFERS:
            case GL_SAMPLE_COVERAGE:
            case GL_SAMPLE_COVERAGE_INVERT:
            case GL_SAMPLE_COVERAGE_VALUE:
            case GL_SAMPLE_MASK_VALUE:
            case GL_SAMPLE_SHADING:
            case GL_SAMPLER_BINDING:
            case GL_SAMPLES:
            case GL_SCISSOR_TEST:
            case GL_SHADER_COMPILER:
            case GL_SHADER_STORAGE_BUFFER_BINDING:
            case GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT:
            case GL_SHADER_STORAGE_BUFFER_SIZE:
            case GL_SHADER_STORAGE_BUFFER_START:
            case GL_STENCIL_BACK_FAIL:
            case GL_STENCIL_BACK_FUNC:
            case GL_STENCIL_BACK_PASS_DEPTH_FAIL:
            case GL_STENCIL_BACK_PASS_DEPTH_PASS:
            case GL_STENCIL_BACK_REF:
            case GL_STENCIL_BACK_VALUE_MASK:
            case GL_STENCIL_BACK_WRITEMASK:
            case GL_STENCIL_BITS:
            case GL_STENCIL_CLEAR_VALUE:
            case GL_STENCIL_FAIL:
            case GL_STENCIL_FUNC:
            case GL_STENCIL_PASS_DEPTH_FAIL:
            case GL_STENCIL_PASS_DEPTH_PASS:
            case GL_STENCIL_REF:
            case GL_STENCIL_TEST:
            case GL_STENCIL_VALUE_MASK:
            case GL_STENCIL_WRITEMASK:
            case GL_SUBPIXEL_BITS:
            case GL_TEXTURE_BINDING_2D:
            case GL_TEXTURE_BINDING_2D_ARRAY:
            case GL_TEXTURE_BINDING_3D:
            case GL_TEXTURE_BINDING_BUFFER:
            case GL_TEXTURE_BINDING_CUBE_MAP:
            case GL_TEXTURE_BINDING_2D_MULTISAMPLE:
            case GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY:
            case GL_TEXTURE_BINDING_CUBE_MAP_ARRAY:
            case GL_TEXTURE_BUFFER_BINDING:
            case GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT:
            case GL_TRANSFORM_FEEDBACK_BINDING:
            case GL_TRANSFORM_FEEDBACK_ACTIVE:
            case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
            case GL_TRANSFORM_FEEDBACK_PAUSED:
            case GL_TRANSFORM_FEEDBACK_BUFFER_SIZE:
            case GL_TRANSFORM_FEEDBACK_BUFFER_START:
            case GL_UNIFORM_BUFFER_BINDING:
            case GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT:
            case GL_UNIFORM_BUFFER_SIZE:
            case GL_UNIFORM_BUFFER_START:
            case GL_UNPACK_ALIGNMENT:
            case GL_UNPACK_IMAGE_HEIGHT:
            case GL_UNPACK_ROW_LENGTH:
            case GL_UNPACK_SKIP_IMAGES:
            case GL_UNPACK_SKIP_PIXELS:
            case GL_UNPACK_SKIP_ROWS:
            case GL_VERTEX_ARRAY_BINDING:
            case GL_VERTEX_BINDING_DIVISOR:
            case GL_VERTEX_BINDING_OFFSET:
            case GL_VERTEX_BINDING_STRIDE:
            case GL_TEXTURE_2D:
            case GL_TEXTURE_3D:
                count = 1;
                break;

            case GL_ALIASED_LINE_WIDTH_RANGE:
            case GL_ALIASED_POINT_SIZE_RANGE:
            case GL_DEPTH_RANGE:
            case GL_MAX_VIEWPORT_DIMS:
            case GL_MULTISAMPLE_LINE_WIDTH_RANGE:
                count = 2;
                break;

            case GL_BLEND_COLOR:
            case GL_COLOR_CLEAR_VALUE:
            case GL_COLOR_WRITEMASK:
            case GL_SCISSOR_BOX:
            case GL_VIEWPORT:
                count = 4;
                break;

            case GL_PRIMITIVE_BOUNDING_BOX:
                count = 8;
                break;

            case GL_COMPRESSED_TEXTURE_FORMATS:
                count = __orcaGLESImplLimits.numCompressedTextureFormats;
                break;

            case GL_PROGRAM_BINARY_FORMATS:
                count = __orcaGLESImplLimits.numProgramBinaryFormats;
                break;

            case GL_SHADER_BINARY_FORMATS:
                count = __orcaGLESImplLimits.numShaderBinaryFormats;
                break;

            default:
                OC_ASSERT(0, "unknown GLenum pname %i", pname);
                break;
        }
    }
    return (count);
}

u64 orca_glDrawElements_indices_length(oc_wasm* wasm, GLsizei count, GLenum type)
{
    return (orca_gl_type_size(type) * count);
}

u64 orca_glGetBooleanv_data_length(oc_wasm* wasm, GLenum pname)
{
    return (orca_glGet_data_length(pname));
}

u64 orca_glGetBufferParameteriv_params_length(oc_wasm* wasm, GLenum pname)
{
    //NOTE: all pnames return a single value in 3.1
    return (1);
}

u64 orca_glGetFloatv_data_length(oc_wasm* wasm, GLenum pname)
{
    return (orca_glGet_data_length(pname));
}

u64 orca_glGetFramebufferAttachmentParameteriv_params_length(oc_wasm* wasm, GLenum pname)
{
    //NOTE: all pnames return a single value in 3.1
    return (1);
}

u64 orca_glGetIntegerv_data_length(oc_wasm* wasm, GLenum pname)
{
    return (orca_glGet_data_length(pname));
}

u64 orca_glGetProgramiv_params_length(oc_wasm* wasm, GLenum pname)
{
    //NOTE: all pnames return a single value in 3.1
    return (1);
}

u64 orca_glGetRenderbufferParameteriv_params_length(oc_wasm* wasm, GLenum pname)
{
    //NOTE: all pnames return a single value in 3.1
    return (1);
}

u64 orca_glGetShaderiv_params_length(oc_wasm* wasm, GLenum pname)
{
    //NOTE: all pnames return a single value in 3.1
    return (1);
}

u64 orca_glTexParameter_params_length_generic(GLenum pname)
{
    u64 count = 4;
    if(pname == GL_TEXTURE_BORDER_COLOR)
    {
        count = 4;
    }
    else
    {
        count = 1;
    }
    return (count);
}

u64 orca_glGetTexParameterfv_params_length(oc_wasm* wasm, GLenum pname)
{
    return (orca_glTexParameter_params_length_generic(pname));
}

u64 orca_glGetTexParameteriv_params_length(oc_wasm* wasm, GLenum pname)
{
    return (orca_glTexParameter_params_length_generic(pname));
}

u64 orca_glReadPixels_pixels_length(oc_wasm* wasm, GLenum format, GLenum type, GLsizei width, GLsizei height)
{
    u64 count = width * height * orca_gl_type_size(type) * orca_gl_format_count(format);
    return (count);
}

u64 orca_glTexImage2D_pixels_length(oc_wasm* wasm, GLenum format, GLenum type, GLsizei width, GLsizei height)
{
    u64 count = width * height * orca_gl_type_size(type) * orca_gl_format_count(format);
    return (count);
}

u64 orca_glTexParameterfv_params_length(oc_wasm* wasm, GLenum pname)
{
    return (orca_glTexParameter_params_length_generic(pname));
}

u64 orca_glTexParameteriv_params_length(oc_wasm* wasm, GLenum pname)
{
    return (orca_glTexParameter_params_length_generic(pname));
}

u64 orca_glTexSubImage2D_pixels_length(oc_wasm* wasm, GLenum format, GLenum type, GLsizei width, GLsizei height)
{
    u64 count = width * height * orca_gl_type_size(type) * orca_gl_format_count(format);
    return (count);
}

u64 orca_glDrawRangeElements_indices_length(oc_wasm* wasm, GLsizei count, GLenum type)
{
    return (count * orca_gl_type_size(type));
}

u64 orca_glTexImage3D_pixels_length(oc_wasm* wasm, GLenum format, GLenum type, GLsizei width, GLsizei height, GLsizei depth)
{
    u64 count = width * height * depth * orca_gl_type_size(type) * orca_gl_format_count(format);
    return (count);
}

u64 orca_glTexSubImage3D_pixels_length(oc_wasm* wasm, GLenum format, GLenum type, GLsizei width, GLsizei height, GLsizei depth)
{
    u64 count = width * height * depth * orca_gl_type_size(type) * orca_gl_format_count(format);
    return (count);
}

u64 orca_glGetQueryiv_params_length(oc_wasm* wasm, GLenum pname)
{
    //NOTE: all pnames return a single value in 3.1
    return (1);
}

u64 orca_glGetQueryObjectuiv_params_length(oc_wasm* wasm, GLenum pname)
{
    //NOTE: all pnames return a single value in 3.1
    return (1);
}

u64 orca_glGetIntegeri_v_data_length(oc_wasm* wasm, GLenum target)
{
    return (orca_glGet_data_length(target));
}

u64 orca_glVertexAttribIPointer_pointer_length(oc_wasm* wasm, GLint size, GLenum type, GLsizei stride)
{
    //WARN: pointer param of glVertexAttribPointer is actually treated as an offset,
    //      so, we don't need to check if this points to valid memory ??
    return (0);
}

u64 orca_glClearBuffer_value_length_generic(GLenum buffer)
{
    u64 count = 4;
    switch(buffer)
    {
        case GL_COLOR:
            count = 4;
            break;

        case GL_DEPTH:
        case GL_STENCIL:
            count = 1;
            break;

        default:
            OC_ASSERT(0, "invalid buffer enum for glClearBuffer()");
    }
    return (count);
}

u64 orca_glClearBufferiv_value_length(oc_wasm* wasm, GLenum buffer)
{
    return (orca_glClearBuffer_value_length_generic(buffer));
}

u64 orca_glClearBufferuiv_value_length(oc_wasm* wasm, GLenum buffer)
{
    return (orca_glClearBuffer_value_length_generic(buffer));
}

u64 orca_glClearBufferfv_value_length(oc_wasm* wasm, GLenum buffer)
{
    return (orca_glClearBuffer_value_length_generic(buffer));
}

u64 orca_glGetUniformIndices_uniformIndices_length(oc_wasm* wasm, GLsizei uniformCount)
{
    return (uniformCount);
}

u64 orca_glGetActiveUniformsiv_params_length(oc_wasm* wasm, GLsizei uniformCount, GLenum pname)
{
    return (uniformCount);
}

u64 orca_glGetActiveUniformBlockiv_params_length(oc_wasm* wasm, GLuint program, GLuint uniformBlockIndex, GLenum pname)
{
    u64 count;
    if(pname == GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES)
    {
        GLint param;
        glGetActiveUniformBlockiv(program, uniformBlockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &param);
        count = param;
    }
    else
    {
        count = 1;
    }
    return (count);
}

u64 orca_glDrawElementsInstanced_indices_length(oc_wasm* wasm, GLsizei count, GLenum type)
{
    return (count * orca_gl_type_size(type));
}

u64 orca_glGetInteger64v_data_length(oc_wasm* wasm, GLenum pname)
{
    return (orca_glGet_data_length(pname));
}

u64 orca_glGetInteger64i_v_data_length(oc_wasm* wasm, GLenum target)
{
    return (orca_glGet_data_length(target));
}

u64 orca_glGetBufferParameteri64v_params_length(oc_wasm* wasm, GLenum pname)
{
    //NOTE: all pnames return a single value in 3.1
    return (1);
}

u64 orca_glSamplerParameter_param_length_generic(GLenum pname)
{
    //NOTE: same as texture parameter pnames
    return (orca_glTexParameter_params_length_generic(pname));
}

u64 orca_glSamplerParameteriv_param_length(oc_wasm* wasm, GLenum pname)
{
    return (orca_glSamplerParameter_param_length_generic(pname));
}

u64 orca_glSamplerParameterfv_param_length(oc_wasm* wasm, GLenum pname)
{
    return (orca_glSamplerParameter_param_length_generic(pname));
}

u64 orca_glGetSamplerParameteriv_params_length(oc_wasm* wasm, GLenum pname)
{
    return (orca_glSamplerParameter_param_length_generic(pname));
}

u64 orca_glGetSamplerParameterfv_params_length(oc_wasm* wasm, GLenum pname)
{
    return (orca_glSamplerParameter_param_length_generic(pname));
}

u64 orca_glGetFramebufferParameteriv_params_length(oc_wasm* wasm, GLenum pname)
{
    //NOTE: all pnames return a single value in 3.1
    return (1);
}

u64 orca_glGetProgramInterfaceiv_params_length(oc_wasm* wasm, GLenum pname)
{
    //NOTE: all pnames return a single value in 3.1
    return (1);
}

u64 orca_glGetProgramPipelineiv_params_length(oc_wasm* wasm, GLenum pname)
{
    //NOTE: all pnames return a single value in 3.1
    return (1);
}

u64 orca_glGetBooleani_v_data_length(oc_wasm* wasm, GLenum target)
{
    return (orca_glSamplerParameter_param_length_generic(target));
}

u64 orca_glGetMultisamplefv_val_length(oc_wasm* wasm, GLenum pname)
{
    return (2);
}

u64 orca_glGetTexLevelParameteriv_params_length(oc_wasm* wasm, GLenum pname)
{
    //NOTE: all pnames return a single value in 3.1
    return (1);
}

u64 orca_glGetTexLevelParameterfv_params_length(oc_wasm* wasm, GLenum pname)
{
    //NOTE: all pnames return a single value in 3.1
    return (1);
}

//------------------------------------------------------------------------
// Uniforms size checking
//------------------------------------------------------------------------

u64 orca_glGetUniform_params_length_generic(GLuint program, GLint location)
{
    //NOTE: This is super stupid but we can't get the size (or index) of a uniform directly from its location,
    //      so we have to iterate through all uniforms...
    GLint maxUniformName = 0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformName);

    int uniformCount = 0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniformCount);

    oc_arena_scope scratch = oc_scratch_begin();
    char* name = oc_arena_push(scratch.arena, maxUniformName + 1);

    u64 count = 0;
    bool found = false;

    for(int i = 0; i < uniformCount; i++)
    {
        GLsizei nameLength = 0;
        GLint uniformSize = 0;
        GLenum uniformType = 0;
        glGetActiveUniform(program, i, maxUniformName, &nameLength, &uniformSize, &uniformType, name);
        GLint uniformLocation = glGetUniformLocation(program, name);

        if(uniformLocation == location)
        {
            count = uniformSize;
            found = true;
            break;
        }
    }
    OC_ASSERT(found, "uniform location %i not found for program %i", location, program);

    oc_scratch_end(scratch);
    return (count);
}

u64 orca_glGetUniformfv_params_length(oc_wasm* wasm, GLuint program, GLint location)
{
    return (orca_glGetUniform_params_length_generic(program, location));
}

u64 orca_glGetUniformiv_params_length(oc_wasm* wasm, GLuint program, GLint location)
{
    return (orca_glGetUniform_params_length_generic(program, location));
}

u64 orca_glGetUniformuiv_params_length(oc_wasm* wasm, GLuint program, GLint location)
{
    return (orca_glGetUniform_params_length_generic(program, location));
}

//------------------------------------------------------------------------
// Null-terminated parameters checking
//------------------------------------------------------------------------

u64 orca_glGetFragDataLocation_name_length(oc_wasm* wasm, const GLchar* name)
{
    return (orca_check_cstring(wasm, name));
}

u64 orca_glGetUniformBlockIndex_uniformBlockName_length(oc_wasm* wasm, const GLchar* uniformBlockName)
{
    return (orca_check_cstring(wasm, uniformBlockName));
}

u64 orca_glGetProgramResourceIndex_name_length(oc_wasm* wasm, const GLchar* name)
{
    return (orca_check_cstring(wasm, name));
}

u64 orca_glGetProgramResourceLocation_name_length(oc_wasm* wasm, const GLchar* name)
{
    return (orca_check_cstring(wasm, name));
}

u64 orca_glBindAttribLocation_name_length(oc_wasm* wasm, const GLchar* name)
{
    return (orca_check_cstring(wasm, name));
}

u64 orca_glGetAttribLocation_name_length(oc_wasm* wasm, const GLchar* name)
{
    return (orca_check_cstring(wasm, name));
}

u64 orca_glGetUniformLocation_name_length(oc_wasm* wasm, const GLchar* name)
{
    return (orca_check_cstring(wasm, name));
}

//------------------------------------------------------------------------
// Draw indirect length checks
//------------------------------------------------------------------------

typedef struct
{
    u32 count;
    u32 primCount;
    u32 first;
    u32 reserved;
} DrawArraysIndirectCommand;

u64 orca_glDrawArraysIndirect_indirect_length(oc_wasm* wasm, const void* indirect)
{
    return (sizeof(DrawArraysIndirectCommand));
}

typedef struct
{
    u32 count;
    u32 instanceCount;
    u32 firstIndex;
    i32 baseVertex;
    u32 reservedMustBeZero;
} DrawElementsIndirectCommand;

u64 orca_glDrawElementsIndirect_indirect_length(oc_wasm* wasm, const void* indirect)
{
    return (sizeof(DrawElementsIndirectCommand));
}

//------------------------------------------------------------------------
// Fully manual bindings
//------------------------------------------------------------------------

void glShaderSource_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* _wasm)
{
    i32 shader = *(i32*)&_params[0];
    i32 count = *(i32*)&_params[1];
    i32 stringArrayOffset = *(i32*)&_params[2];
    i32 lengthArrayOffset = *(i32*)&_params[3];

    int* stringOffsetArray = (int*)((char*)_mem + stringArrayOffset);

    oc_arena_scope scratch = oc_scratch_begin();
    const char** stringArray = (const char**)oc_arena_push_array(scratch.arena, char*, count);
    for(int i = 0; i < count; i++)
    {
        stringArray[i] = (char*)_mem + stringOffsetArray[i];
    }

    int* lengthArray = lengthArrayOffset ? (int*)((char*)_mem + lengthArrayOffset) : 0;

    glShaderSource(shader, count, stringArray, lengthArray);

    oc_scratch_end(scratch);
}

void glGetVertexAttribPointerv_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* _wasm)
{
    GLuint index = *(i32*)&_params[0];
    GLenum pname = *(i32*)&_params[1];
    i32* pointer = (i32*)((char*)_mem + *(u32*)&_params[2]);
    {
        OC_ASSERT(((char*)pointer >= (char*)_mem) && (((char*)pointer - (char*)_mem) < oc_wasm_mem_size(_wasm)),
                  "parameter 'pointer' is out of bounds");
        OC_ASSERT((char*)pointer + sizeof(i32) <= ((char*)_mem + oc_wasm_mem_size(_wasm)),
                  "parameter 'pointer' overflows wasm memory");
    }
    void* rawPointer = 0;
    glGetVertexAttribPointerv(index, pname, &rawPointer);

    //NOTE: pointer is actually a _byte offset_ into a GPU buffer. So we do _not_ convert it to a wasm pointer,
    //      but we need to truncate it to u32 size...
    //WARN: can OpenGL return a byte offset > UINT_MAX ?
    *pointer = (i32)(intptr_t)rawPointer;
}

void glVertexAttribPointer_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* _wasm)
{
    GLuint index = *(u32*)&_params[0];
    GLint size = *(i32*)&_params[1];
    GLenum type = *(i32*)&_params[2];
    GLboolean normalized = (GLboolean) * (i32*)&_params[3];
    GLsizei stride = *(i32*)&_params[4];

    //NOTE: pointer is interpreted as an offset if there's a non-null buffer bound to GL_ARRAY_BUFFER,
    //      or as a pointer otherwise. Since there's no way of checking the length of client vertex arrays,
    //      we just disable those.

    GLint boundBuffer = 0;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &boundBuffer);

    if(boundBuffer != 0)
    {
        //NOTE: don't do bounds checking since pointer is really an offset in a GPU buffer
        const void* pointer = (void*)(intptr_t) * (u32*)&_params[5];

        glVertexAttribPointer(index, size, type, normalized, stride, pointer);
    }
    else
    {
        //NOTE: we crash here before letting ANGLE crash because vertex attrib pointer is not set
        OC_ASSERT("Calling glVertexAttribPointer with a GL_ARRAY_BUFFER binding of 0 is unsafe and disabled in Orca.");
    }
}

void glVertexAttribIPointer_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* _wasm)
{
    GLuint index = *(u32*)&_params[0];
    GLint size = *(i32*)&_params[1];
    GLenum type = *(i32*)&_params[2];
    GLsizei stride = *(i32*)&_params[3];

    //NOTE: pointer is interpreted as an offset if there's a non-null buffer bound to GL_ARRAY_BUFFER,
    //      or as a pointer otherwise. Since there's no way of checking the length of client vertex arrays,
    //      we just disable those.

    GLint boundBuffer = 0;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &boundBuffer);

    if(boundBuffer != 0)
    {
        //NOTE: don't do bounds checking since pointer is really an offset in a GPU buffer
        const void* pointer = (void*)(intptr_t) * (u32*)&_params[4];

        glVertexAttribIPointer(index, size, type, stride, pointer);
    }
    else
    {
        OC_ASSERT(0, "Calling glVertexAttribIPointer with a GL_ARRAY_BUFFER binding of 0 is unsafe and disabled in Orca.");
    }
}

void glGetUniformIndices_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* _wasm)
{
    GLuint program = (GLuint) * (i32*)&_params[0];
    GLsizei uniformCount = (GLsizei) * (i32*)&_params[1];
    u32* uniformNames = (u32*)((char*)_mem + *(u32*)&_params[2]);
    GLuint* uniformIndices = (GLuint*)((char*)_mem + *(u32*)&_params[3]);

    u64 memorySize = oc_wasm_mem_size(_wasm);
    //NOTE: check size of uniformNames
    {
        OC_ASSERT(((char*)uniformNames >= (char*)_mem) && (((char*)uniformNames - (char*)_mem) < memorySize),
                  "parameter 'uniformNames' is out of bounds");
        OC_ASSERT((char*)uniformNames + uniformCount * sizeof(u32) <= ((char*)_mem + memorySize),
                  "parameter 'uniformNames' overflows wasm memory");
    }
    //NOTE: check each individual uniformNames
    oc_arena_scope scratch = oc_scratch_begin();

    char** uniformNamesRaw = oc_arena_push_array(scratch.arena, char*, uniformCount);
    for(int i = 0; i < uniformCount; i++)
    {
        char* raw = ((char*)_mem + uniformNames[i]);
        OC_ASSERT(raw >= (char*)_mem && (raw - (char*)_mem) < memorySize, "uniformName[%i] is out of bounds", i);

        u64 len = orca_check_cstring(_wasm, raw);

        OC_ASSERT(raw + len <= ((char*)_mem + memorySize), "uniformName[%i] overflows wasm memory", i);

        uniformNamesRaw[i] = raw;
    }

    //NOTE: check size of uniformIndices
    {
        OC_ASSERT(((char*)uniformIndices >= (char*)_mem) && (((char*)uniformIndices - (char*)_mem) < memorySize),
                  "parameter 'uniformIndices' is out of bounds");
        OC_ASSERT((char*)uniformIndices + uniformCount * sizeof(GLuint) <= ((char*)_mem + memorySize),
                  "parameter 'uniformIndices' overflows wasm memory");
    }

    glGetUniformIndices(program, uniformCount, (const GLchar* const*)uniformNamesRaw, uniformIndices);

    oc_scratch_end(scratch);
}

typedef struct orca_gl_getstring_entry
{
    u32 offset;
    u32 len;
} orca_gl_getstring_entry;

GLenum ORCA_GL_GETSTRING_NAMES[] = {
    GL_EXTENSIONS,
    GL_VENDOR,
    GL_RENDERER,
    GL_VERSION,
    GL_SHADING_LANGUAGE_VERSION
};

enum
{
    ORCA_GL_GETSTRING_ENTRY_COUNT = sizeof(ORCA_GL_GETSTRING_NAMES) / sizeof(GLenum)
};

typedef struct orca_gl_getstring_info
{
    bool init;

    orca_gl_getstring_entry entries[ORCA_GL_GETSTRING_ENTRY_COUNT];

    u32 indexedEntryCount;
    orca_gl_getstring_entry* indexedEntries;

} orca_gl_getstring_info;

orca_gl_getstring_info __orcaGLGetStringInfo = { 0 };

void orca_gl_getstring_init(orca_gl_getstring_info* info, char* memory)
{
    u32 totalSize = 0;
    const char* strings[ORCA_GL_GETSTRING_ENTRY_COUNT] = { 0 };

    for(int i = 0; i < ORCA_GL_GETSTRING_ENTRY_COUNT; i++)
    {
        strings[i] = (const char*)glGetString(ORCA_GL_GETSTRING_NAMES[i]);
        if(strings[i])
        {
            info->entries[i].len = strlen(strings[i]) + 1;
            totalSize += info->entries[i].len;
        }
    }

    glGetIntegerv(GL_NUM_EXTENSIONS, (GLint*)&info->indexedEntryCount);
    oc_arena_scope scratch = oc_scratch_begin();
    const char** extensions = oc_arena_push(scratch.arena, info->indexedEntryCount);

    //NOTE: we will hold this until program terminates
    info->indexedEntries = oc_malloc_array(orca_gl_getstring_entry, info->indexedEntryCount);

    for(int i = 0; i < info->indexedEntryCount; i++)
    {
        extensions[i] = (const char*)glGetStringi(GL_EXTENSIONS, i);
        if(extensions[i])
        {
            info->indexedEntries[i].len = strlen(extensions[i]) + 1;
            totalSize += info->indexedEntries[i].len;
        }
    }

    u32 wasmIndex = oc_mem_grow(totalSize);

    for(int i = 0; i < ORCA_GL_GETSTRING_ENTRY_COUNT; i++)
    {
        if(strings[i])
        {
            info->entries[i].offset = wasmIndex;
            memcpy(memory + wasmIndex, strings[i], info->entries[i].len);

            wasmIndex += info->entries[i].len;
        }
    }

    for(int i = 0; i < info->indexedEntryCount; i++)
    {
        if(extensions[i])
        {
            info->indexedEntries[i].offset = wasmIndex;
            memcpy(memory + wasmIndex, extensions[i], info->indexedEntries[i].len);
            wasmIndex += info->indexedEntries[i].len;
        }
    }

    oc_scratch_end(scratch);

    info->init = true;
}

void glGetString_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* _wasm)
{
    if(!__orcaGLGetStringInfo.init)
    {
        oc_str8 memory = oc_wasm_mem_get(_wasm);
        orca_gl_getstring_init(&__orcaGLGetStringInfo, (char*)memory.ptr);
    }

    GLenum name = (GLenum) * (i32*)&_params[1];
    *(u32*)&_returns[0] = 0;

    for(int i = 0; i < ORCA_GL_GETSTRING_ENTRY_COUNT; i++)
    {
        if(name == ORCA_GL_GETSTRING_NAMES[i])
        {
            *(u32*)&_returns[0] = __orcaGLGetStringInfo.entries[i].offset;
            break;
        }
    }
    //NOTE: we still call glGetString so that it can set errors if name is incorrect
    glGetString(name);
}

void glGetStringi_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* _wasm)
{
    if(!__orcaGLGetStringInfo.init)
    {
        oc_str8 memory = oc_wasm_mem_get(_wasm);
        orca_gl_getstring_init(&__orcaGLGetStringInfo, (char*)memory.ptr);
    }

    GLenum name = (GLenum) * (i32*)&_params[1];
    GLuint index = (GLuint) * (i32*)&_params[2];

    *(u32*)&_returns[0] = 0;
    if(name == GL_EXTENSIONS && index < __orcaGLGetStringInfo.indexedEntryCount)
    {
        *(u32*)&_returns[0] = __orcaGLGetStringInfo.indexedEntries[index].offset;
    }

    //NOTE: we still call glGetString so that it can set errors if name is incorrect
    glGetStringi(name, index);
}

int manual_link_gles_api(oc_wasm* wasm)
{
#define BINDING_ERROR_HANDLING(name)                                                                   \
    if(wa_status_is_fail(status))                                                                      \
    {                                                                                                  \
        oc_log_error("Couldn't link function " #name " (%.*s)\n", oc_str8_ip(wa_status_str8(status))); \
        ret = -1;                                                                                      \
    }

    wa_status status;
    int ret = 0;

    wa_value_type int_types[10] = {
        WA_TYPE_I32,
        WA_TYPE_I32,
        WA_TYPE_I32,
        WA_TYPE_I32,
        WA_TYPE_I32,
        WA_TYPE_I32,
        WA_TYPE_I32,
        WA_TYPE_I32,
        WA_TYPE_I32,
        WA_TYPE_I32,
    };

    oc_wasm_binding binding = { 0 };
    binding.params = int_types;
    binding.returns = int_types;

    binding.importName = OC_STR8("glShaderSource");
    binding.proc = glShaderSource_stub;
    binding.countParams = 4;
    binding.countReturns = 0;
    status = oc_wasm_add_binding(wasm, &binding);
    BINDING_ERROR_HANDLING(glShaderSource)

    binding.importName = OC_STR8("glGetUniformIndices");
    binding.proc = glGetUniformIndices_stub;
    binding.countParams = 4;
    binding.countReturns = 0;
    status = oc_wasm_add_binding(wasm, &binding);
    BINDING_ERROR_HANDLING(glGetUniformIndices)

    binding.importName = OC_STR8("glGetVertexAttribPointerv");
    binding.proc = glGetVertexAttribPointerv_stub;
    binding.countParams = 4;
    binding.countReturns = 0;
    status = oc_wasm_add_binding(wasm, &binding);
    BINDING_ERROR_HANDLING(glGetVertexAttribPointerv)

    binding.importName = OC_STR8("glGetString");
    binding.proc = glGetString_stub;
    binding.countParams = 1;
    binding.countReturns = 1;
    status = oc_wasm_add_binding(wasm, &binding);
    BINDING_ERROR_HANDLING(glGetGetString)

    binding.importName = OC_STR8("glGetStringi");
    binding.proc = glGetStringi_stub;
    binding.countParams = 2;
    binding.countReturns = 1;
    status = oc_wasm_add_binding(wasm, &binding);
    BINDING_ERROR_HANDLING(glGetStringi)

    binding.importName = OC_STR8("glVertexAttribPointer");
    binding.proc = glVertexAttribPointer_stub;
    binding.countParams = 6;
    binding.countReturns = 0;
    status = oc_wasm_add_binding(wasm, &binding);
    BINDING_ERROR_HANDLING(glVertexAttribPointer)

    binding.importName = OC_STR8("glVertexAttribIPointer");
    binding.proc = glVertexAttribIPointer_stub;
    binding.countParams = 5;
    binding.countReturns = 0;
    status = oc_wasm_add_binding(wasm, &binding);
    BINDING_ERROR_HANDLING(glVertexAttribIPointer)

#undef BINDING_ERROR_HANDLING

    return (ret);
}
