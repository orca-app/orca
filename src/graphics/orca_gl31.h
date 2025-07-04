#ifndef __gles2_orca_gl31_h_
#define __gles2_orca_gl31_h_ 1

#ifdef __cplusplus
extern "C" {
#endif

/* Generated C header for:
 * API: gles2
 * Profile: common
 * Versions considered: 2\.[0-9]|3\.[01]
 * Versions emitted: .*
 * Default extensions included: None
 * Additional extensions included: _nomatch_^
 * Extensions removed: _nomatch_^
 */

#ifndef GL_ES_VERSION_2_0
#define GL_ES_VERSION_2_0 1
#include <KHR/khrplatform.h>
typedef khronos_int8_t GLbyte;
typedef khronos_float_t GLclampf;
typedef khronos_int32_t GLfixed;
typedef khronos_int16_t GLshort;
typedef khronos_uint16_t GLushort;
typedef void GLvoid;
typedef struct __GLsync *GLsync;
typedef khronos_int64_t GLint64;
typedef khronos_uint64_t GLuint64;
typedef unsigned int GLenum;
typedef unsigned int GLuint;
typedef char GLchar;
typedef khronos_float_t GLfloat;
typedef khronos_ssize_t GLsizeiptr;
typedef khronos_intptr_t GLintptr;
typedef unsigned int GLbitfield;
typedef int GLint;
typedef unsigned char GLboolean;
typedef int GLsizei;
typedef khronos_uint8_t GLubyte;
#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_STENCIL_BUFFER_BIT             0x00000400
#define GL_COLOR_BUFFER_BIT               0x00004000
#define GL_FALSE                          0
#define GL_TRUE                           1
#define GL_POINTS                         0x0000
#define GL_LINES                          0x0001
#define GL_LINE_LOOP                      0x0002
#define GL_LINE_STRIP                     0x0003
#define GL_TRIANGLES                      0x0004
#define GL_TRIANGLE_STRIP                 0x0005
#define GL_TRIANGLE_FAN                   0x0006
#define GL_ZERO                           0
#define GL_ONE                            1
#define GL_SRC_COLOR                      0x0300
#define GL_ONE_MINUS_SRC_COLOR            0x0301
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GL_DST_ALPHA                      0x0304
#define GL_ONE_MINUS_DST_ALPHA            0x0305
#define GL_DST_COLOR                      0x0306
#define GL_ONE_MINUS_DST_COLOR            0x0307
#define GL_SRC_ALPHA_SATURATE             0x0308
#define GL_FUNC_ADD                       0x8006
#define GL_BLEND_EQUATION                 0x8009
#define GL_BLEND_EQUATION_RGB             0x8009
#define GL_BLEND_EQUATION_ALPHA           0x883D
#define GL_FUNC_SUBTRACT                  0x800A
#define GL_FUNC_REVERSE_SUBTRACT          0x800B
#define GL_BLEND_DST_RGB                  0x80C8
#define GL_BLEND_SRC_RGB                  0x80C9
#define GL_BLEND_DST_ALPHA                0x80CA
#define GL_BLEND_SRC_ALPHA                0x80CB
#define GL_CONSTANT_COLOR                 0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR       0x8002
#define GL_CONSTANT_ALPHA                 0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA       0x8004
#define GL_BLEND_COLOR                    0x8005
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_ARRAY_BUFFER_BINDING           0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING   0x8895
#define GL_STREAM_DRAW                    0x88E0
#define GL_STATIC_DRAW                    0x88E4
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_BUFFER_SIZE                    0x8764
#define GL_BUFFER_USAGE                   0x8765
#define GL_CURRENT_VERTEX_ATTRIB          0x8626
#define GL_FRONT                          0x0404
#define GL_BACK                           0x0405
#define GL_FRONT_AND_BACK                 0x0408
#define GL_TEXTURE_2D                     0x0DE1
#define GL_CULL_FACE                      0x0B44
#define GL_BLEND                          0x0BE2
#define GL_DITHER                         0x0BD0
#define GL_STENCIL_TEST                   0x0B90
#define GL_DEPTH_TEST                     0x0B71
#define GL_SCISSOR_TEST                   0x0C11
#define GL_POLYGON_OFFSET_FILL            0x8037
#define GL_SAMPLE_ALPHA_TO_COVERAGE       0x809E
#define GL_SAMPLE_COVERAGE                0x80A0
#define GL_NO_ERROR                       0
#define GL_INVALID_ENUM                   0x0500
#define GL_INVALID_VALUE                  0x0501
#define GL_INVALID_OPERATION              0x0502
#define GL_OUT_OF_MEMORY                  0x0505
#define GL_CW                             0x0900
#define GL_CCW                            0x0901
#define GL_LINE_WIDTH                     0x0B21
#define GL_ALIASED_POINT_SIZE_RANGE       0x846D
#define GL_ALIASED_LINE_WIDTH_RANGE       0x846E
#define GL_CULL_FACE_MODE                 0x0B45
#define GL_FRONT_FACE                     0x0B46
#define GL_DEPTH_RANGE                    0x0B70
#define GL_DEPTH_WRITEMASK                0x0B72
#define GL_DEPTH_CLEAR_VALUE              0x0B73
#define GL_DEPTH_FUNC                     0x0B74
#define GL_STENCIL_CLEAR_VALUE            0x0B91
#define GL_STENCIL_FUNC                   0x0B92
#define GL_STENCIL_FAIL                   0x0B94
#define GL_STENCIL_PASS_DEPTH_FAIL        0x0B95
#define GL_STENCIL_PASS_DEPTH_PASS        0x0B96
#define GL_STENCIL_REF                    0x0B97
#define GL_STENCIL_VALUE_MASK             0x0B93
#define GL_STENCIL_WRITEMASK              0x0B98
#define GL_STENCIL_BACK_FUNC              0x8800
#define GL_STENCIL_BACK_FAIL              0x8801
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL   0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS   0x8803
#define GL_STENCIL_BACK_REF               0x8CA3
#define GL_STENCIL_BACK_VALUE_MASK        0x8CA4
#define GL_STENCIL_BACK_WRITEMASK         0x8CA5
#define GL_VIEWPORT                       0x0BA2
#define GL_SCISSOR_BOX                    0x0C10
#define GL_COLOR_CLEAR_VALUE              0x0C22
#define GL_COLOR_WRITEMASK                0x0C23
#define GL_UNPACK_ALIGNMENT               0x0CF5
#define GL_PACK_ALIGNMENT                 0x0D05
#define GL_MAX_TEXTURE_SIZE               0x0D33
#define GL_MAX_VIEWPORT_DIMS              0x0D3A
#define GL_SUBPIXEL_BITS                  0x0D50
#define GL_RED_BITS                       0x0D52
#define GL_GREEN_BITS                     0x0D53
#define GL_BLUE_BITS                      0x0D54
#define GL_ALPHA_BITS                     0x0D55
#define GL_DEPTH_BITS                     0x0D56
#define GL_STENCIL_BITS                   0x0D57
#define GL_POLYGON_OFFSET_UNITS           0x2A00
#define GL_POLYGON_OFFSET_FACTOR          0x8038
#define GL_TEXTURE_BINDING_2D             0x8069
#define GL_SAMPLE_BUFFERS                 0x80A8
#define GL_SAMPLES                        0x80A9
#define GL_SAMPLE_COVERAGE_VALUE          0x80AA
#define GL_SAMPLE_COVERAGE_INVERT         0x80AB
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS 0x86A2
#define GL_COMPRESSED_TEXTURE_FORMATS     0x86A3
#define GL_DONT_CARE                      0x1100
#define GL_FASTEST                        0x1101
#define GL_NICEST                         0x1102
#define GL_GENERATE_MIPMAP_HINT           0x8192
#define GL_BYTE                           0x1400
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_SHORT                          0x1402
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_INT                            0x1404
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406
#define GL_FIXED                          0x140C
#define GL_DEPTH_COMPONENT                0x1902
#define GL_ALPHA                          0x1906
#define GL_RGB                            0x1907
#define GL_RGBA                           0x1908
#define GL_LUMINANCE                      0x1909
#define GL_LUMINANCE_ALPHA                0x190A
#define GL_UNSIGNED_SHORT_4_4_4_4         0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1         0x8034
#define GL_UNSIGNED_SHORT_5_6_5           0x8363
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_MAX_VERTEX_ATTRIBS             0x8869
#define GL_MAX_VERTEX_UNIFORM_VECTORS     0x8DFB
#define GL_MAX_VARYING_VECTORS            0x8DFC
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS 0x8B4C
#define GL_MAX_TEXTURE_IMAGE_UNITS        0x8872
#define GL_MAX_FRAGMENT_UNIFORM_VECTORS   0x8DFD
#define GL_SHADER_TYPE                    0x8B4F
#define GL_DELETE_STATUS                  0x8B80
#define GL_LINK_STATUS                    0x8B82
#define GL_VALIDATE_STATUS                0x8B83
#define GL_ATTACHED_SHADERS               0x8B85
#define GL_ACTIVE_UNIFORMS                0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH      0x8B87
#define GL_ACTIVE_ATTRIBUTES              0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH    0x8B8A
#define GL_SHADING_LANGUAGE_VERSION       0x8B8C
#define GL_CURRENT_PROGRAM                0x8B8D
#define GL_NEVER                          0x0200
#define GL_LESS                           0x0201
#define GL_EQUAL                          0x0202
#define GL_LEQUAL                         0x0203
#define GL_GREATER                        0x0204
#define GL_NOTEQUAL                       0x0205
#define GL_GEQUAL                         0x0206
#define GL_ALWAYS                         0x0207
#define GL_KEEP                           0x1E00
#define GL_REPLACE                        0x1E01
#define GL_INCR                           0x1E02
#define GL_DECR                           0x1E03
#define GL_INVERT                         0x150A
#define GL_INCR_WRAP                      0x8507
#define GL_DECR_WRAP                      0x8508
#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_EXTENSIONS                     0x1F03
#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601
#define GL_NEAREST_MIPMAP_NEAREST         0x2700
#define GL_LINEAR_MIPMAP_NEAREST          0x2701
#define GL_NEAREST_MIPMAP_LINEAR          0x2702
#define GL_LINEAR_MIPMAP_LINEAR           0x2703
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_WRAP_S                 0x2802
#define GL_TEXTURE_WRAP_T                 0x2803
#define GL_TEXTURE                        0x1702
#define GL_TEXTURE_CUBE_MAP               0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP       0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X    0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X    0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y    0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y    0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z    0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z    0x851A
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE      0x851C
#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#define GL_TEXTURE2                       0x84C2
#define GL_TEXTURE3                       0x84C3
#define GL_TEXTURE4                       0x84C4
#define GL_TEXTURE5                       0x84C5
#define GL_TEXTURE6                       0x84C6
#define GL_TEXTURE7                       0x84C7
#define GL_TEXTURE8                       0x84C8
#define GL_TEXTURE9                       0x84C9
#define GL_TEXTURE10                      0x84CA
#define GL_TEXTURE11                      0x84CB
#define GL_TEXTURE12                      0x84CC
#define GL_TEXTURE13                      0x84CD
#define GL_TEXTURE14                      0x84CE
#define GL_TEXTURE15                      0x84CF
#define GL_TEXTURE16                      0x84D0
#define GL_TEXTURE17                      0x84D1
#define GL_TEXTURE18                      0x84D2
#define GL_TEXTURE19                      0x84D3
#define GL_TEXTURE20                      0x84D4
#define GL_TEXTURE21                      0x84D5
#define GL_TEXTURE22                      0x84D6
#define GL_TEXTURE23                      0x84D7
#define GL_TEXTURE24                      0x84D8
#define GL_TEXTURE25                      0x84D9
#define GL_TEXTURE26                      0x84DA
#define GL_TEXTURE27                      0x84DB
#define GL_TEXTURE28                      0x84DC
#define GL_TEXTURE29                      0x84DD
#define GL_TEXTURE30                      0x84DE
#define GL_TEXTURE31                      0x84DF
#define GL_ACTIVE_TEXTURE                 0x84E0
#define GL_REPEAT                         0x2901
#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_MIRRORED_REPEAT                0x8370
#define GL_FLOAT_VEC2                     0x8B50
#define GL_FLOAT_VEC3                     0x8B51
#define GL_FLOAT_VEC4                     0x8B52
#define GL_INT_VEC2                       0x8B53
#define GL_INT_VEC3                       0x8B54
#define GL_INT_VEC4                       0x8B55
#define GL_BOOL                           0x8B56
#define GL_BOOL_VEC2                      0x8B57
#define GL_BOOL_VEC3                      0x8B58
#define GL_BOOL_VEC4                      0x8B59
#define GL_FLOAT_MAT2                     0x8B5A
#define GL_FLOAT_MAT3                     0x8B5B
#define GL_FLOAT_MAT4                     0x8B5C
#define GL_SAMPLER_2D                     0x8B5E
#define GL_SAMPLER_CUBE                   0x8B60
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED    0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE       0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE     0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE       0x8625
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED 0x886A
#define GL_VERTEX_ATTRIB_ARRAY_POINTER    0x8645
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING 0x889F
#define GL_IMPLEMENTATION_COLOR_READ_TYPE 0x8B9A
#define GL_IMPLEMENTATION_COLOR_READ_FORMAT 0x8B9B
#define GL_COMPILE_STATUS                 0x8B81
#define GL_INFO_LOG_LENGTH                0x8B84
#define GL_SHADER_SOURCE_LENGTH           0x8B88
#define GL_SHADER_COMPILER                0x8DFA
#define GL_SHADER_BINARY_FORMATS          0x8DF8
#define GL_NUM_SHADER_BINARY_FORMATS      0x8DF9
#define GL_LOW_FLOAT                      0x8DF0
#define GL_MEDIUM_FLOAT                   0x8DF1
#define GL_HIGH_FLOAT                     0x8DF2
#define GL_LOW_INT                        0x8DF3
#define GL_MEDIUM_INT                     0x8DF4
#define GL_HIGH_INT                       0x8DF5
#define GL_FRAMEBUFFER                    0x8D40
#define GL_RENDERBUFFER                   0x8D41
#define GL_RGBA4                          0x8056
#define GL_RGB5_A1                        0x8057
#define GL_RGB565                         0x8D62
#define GL_DEPTH_COMPONENT16              0x81A5
#define GL_STENCIL_INDEX8                 0x8D48
#define GL_RENDERBUFFER_WIDTH             0x8D42
#define GL_RENDERBUFFER_HEIGHT            0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT   0x8D44
#define GL_RENDERBUFFER_RED_SIZE          0x8D50
#define GL_RENDERBUFFER_GREEN_SIZE        0x8D51
#define GL_RENDERBUFFER_BLUE_SIZE         0x8D52
#define GL_RENDERBUFFER_ALPHA_SIZE        0x8D53
#define GL_RENDERBUFFER_DEPTH_SIZE        0x8D54
#define GL_RENDERBUFFER_STENCIL_SIZE      0x8D55
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE 0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME 0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL 0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE 0x8CD3
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_DEPTH_ATTACHMENT               0x8D00
#define GL_STENCIL_ATTACHMENT             0x8D20
#define GL_NONE                           0
#define GL_FRAMEBUFFER_COMPLETE           0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT 0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS 0x8CD9
#define GL_FRAMEBUFFER_UNSUPPORTED        0x8CDD
#define GL_FRAMEBUFFER_BINDING            0x8CA6
#define GL_RENDERBUFFER_BINDING           0x8CA7
#define GL_MAX_RENDERBUFFER_SIZE          0x84E8
#define GL_INVALID_FRAMEBUFFER_OPERATION  0x0506
typedef void (PFNGLACTIVETEXTUREPROC) (GLenum texture);
typedef void (PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
typedef void (PFNGLBINDATTRIBLOCATIONPROC) (GLuint program, GLuint index, const GLchar *name);
typedef void (PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void (PFNGLBINDFRAMEBUFFERPROC) (GLenum target, GLuint framebuffer);
typedef void (PFNGLBINDRENDERBUFFERPROC) (GLenum target, GLuint renderbuffer);
typedef void (PFNGLBINDTEXTUREPROC) (GLenum target, GLuint texture);
typedef void (PFNGLBLENDCOLORPROC) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (PFNGLBLENDEQUATIONPROC) (GLenum mode);
typedef void (PFNGLBLENDEQUATIONSEPARATEPROC) (GLenum modeRGB, GLenum modeAlpha);
typedef void (PFNGLBLENDFUNCPROC) (GLenum sfactor, GLenum dfactor);
typedef void (PFNGLBLENDFUNCSEPARATEPROC) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
typedef void (PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void (PFNGLBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
typedef GLenum (PFNGLCHECKFRAMEBUFFERSTATUSPROC) (GLenum target);
typedef void (PFNGLCLEARPROC) (GLbitfield mask);
typedef void (PFNGLCLEARCOLORPROC) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (PFNGLCLEARDEPTHFPROC) (GLfloat d);
typedef void (PFNGLCLEARSTENCILPROC) (GLint s);
typedef void (PFNGLCOLORMASKPROC) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
typedef void (PFNGLCOMPILESHADERPROC) (GLuint shader);
typedef void (PFNGLCOMPRESSEDTEXIMAGE2DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
typedef void (PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
typedef void (PFNGLCOPYTEXIMAGE2DPROC) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
typedef void (PFNGLCOPYTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef GLuint (PFNGLCREATEPROGRAMPROC) (void);
typedef GLuint (PFNGLCREATESHADERPROC) (GLenum type);
typedef void (PFNGLCULLFACEPROC) (GLenum mode);
typedef void (PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint *buffers);
typedef void (PFNGLDELETEFRAMEBUFFERSPROC) (GLsizei n, const GLuint *framebuffers);
typedef void (PFNGLDELETEPROGRAMPROC) (GLuint program);
typedef void (PFNGLDELETERENDERBUFFERSPROC) (GLsizei n, const GLuint *renderbuffers);
typedef void (PFNGLDELETESHADERPROC) (GLuint shader);
typedef void (PFNGLDELETETEXTURESPROC) (GLsizei n, const GLuint *textures);
typedef void (PFNGLDEPTHFUNCPROC) (GLenum func);
typedef void (PFNGLDEPTHMASKPROC) (GLboolean flag);
typedef void (PFNGLDEPTHRANGEFPROC) (GLfloat n, GLfloat f);
typedef void (PFNGLDETACHSHADERPROC) (GLuint program, GLuint shader);
typedef void (PFNGLDISABLEPROC) (GLenum cap);
typedef void (PFNGLDISABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void (PFNGLDRAWARRAYSPROC) (GLenum mode, GLint first, GLsizei count);
typedef void (PFNGLDRAWELEMENTSPROC) (GLenum mode, GLsizei count, GLenum type, const void *indices);
typedef void (PFNGLENABLEPROC) (GLenum cap);
typedef void (PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void (PFNGLFINISHPROC) (void);
typedef void (PFNGLFLUSHPROC) (void);
typedef void (PFNGLFRAMEBUFFERRENDERBUFFERPROC) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void (PFNGLFRAMEBUFFERTEXTURE2DPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (PFNGLFRONTFACEPROC) (GLenum mode);
typedef void (PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
typedef void (PFNGLGENERATEMIPMAPPROC) (GLenum target);
typedef void (PFNGLGENFRAMEBUFFERSPROC) (GLsizei n, GLuint *framebuffers);
typedef void (PFNGLGENRENDERBUFFERSPROC) (GLsizei n, GLuint *renderbuffers);
typedef void (PFNGLGENTEXTURESPROC) (GLsizei n, GLuint *textures);
typedef void (PFNGLGETACTIVEATTRIBPROC) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
typedef void (PFNGLGETACTIVEUNIFORMPROC) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
typedef void (PFNGLGETATTACHEDSHADERSPROC) (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
typedef GLint (PFNGLGETATTRIBLOCATIONPROC) (GLuint program, const GLchar *name);
typedef void (PFNGLGETBOOLEANVPROC) (GLenum pname, GLboolean *data);
typedef void (PFNGLGETBUFFERPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef GLenum (PFNGLGETERRORPROC) (void);
typedef void (PFNGLGETFLOATVPROC) (GLenum pname, GLfloat *data);
typedef void (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC) (GLenum target, GLenum attachment, GLenum pname, GLint *params);
typedef void (PFNGLGETINTEGERVPROC) (GLenum pname, GLint *data);
typedef void (PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint *params);
typedef void (PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (PFNGLGETRENDERBUFFERPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint *params);
typedef void (PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (PFNGLGETSHADERPRECISIONFORMATPROC) (GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
typedef void (PFNGLGETSHADERSOURCEPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
typedef const GLubyte *(PFNGLGETSTRINGPROC) (GLenum name);
typedef void (PFNGLGETTEXPARAMETERFVPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (PFNGLGETTEXPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (PFNGLGETUNIFORMFVPROC) (GLuint program, GLint location, GLfloat *params);
typedef void (PFNGLGETUNIFORMIVPROC) (GLuint program, GLint location, GLint *params);
typedef GLint (PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar *name);
typedef void (PFNGLGETVERTEXATTRIBFVPROC) (GLuint index, GLenum pname, GLfloat *params);
typedef void (PFNGLGETVERTEXATTRIBIVPROC) (GLuint index, GLenum pname, GLint *params);
typedef void (PFNGLGETVERTEXATTRIBPOINTERVPROC) (GLuint index, GLenum pname, void **pointer);
typedef void (PFNGLHINTPROC) (GLenum target, GLenum mode);
typedef GLboolean (PFNGLISBUFFERPROC) (GLuint buffer);
typedef GLboolean (PFNGLISENABLEDPROC) (GLenum cap);
typedef GLboolean (PFNGLISFRAMEBUFFERPROC) (GLuint framebuffer);
typedef GLboolean (PFNGLISPROGRAMPROC) (GLuint program);
typedef GLboolean (PFNGLISRENDERBUFFERPROC) (GLuint renderbuffer);
typedef GLboolean (PFNGLISSHADERPROC) (GLuint shader);
typedef GLboolean (PFNGLISTEXTUREPROC) (GLuint texture);
typedef void (PFNGLLINEWIDTHPROC) (GLfloat width);
typedef void (PFNGLLINKPROGRAMPROC) (GLuint program);
typedef void (PFNGLPIXELSTOREIPROC) (GLenum pname, GLint param);
typedef void (PFNGLPOLYGONOFFSETPROC) (GLfloat factor, GLfloat units);
typedef void (PFNGLREADPIXELSPROC) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
typedef void (PFNGLRELEASESHADERCOMPILERPROC) (void);
typedef void (PFNGLRENDERBUFFERSTORAGEPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (PFNGLSAMPLECOVERAGEPROC) (GLfloat value, GLboolean invert);
typedef void (PFNGLSCISSORPROC) (GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (PFNGLSHADERBINARYPROC) (GLsizei count, const GLuint *shaders, GLenum binaryFormat, const void *binary, GLsizei length);
typedef void (PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
typedef void (PFNGLSTENCILFUNCPROC) (GLenum func, GLint ref, GLuint mask);
typedef void (PFNGLSTENCILFUNCSEPARATEPROC) (GLenum face, GLenum func, GLint ref, GLuint mask);
typedef void (PFNGLSTENCILMASKPROC) (GLuint mask);
typedef void (PFNGLSTENCILMASKSEPARATEPROC) (GLenum face, GLuint mask);
typedef void (PFNGLSTENCILOPPROC) (GLenum fail, GLenum zfail, GLenum zpass);
typedef void (PFNGLSTENCILOPSEPARATEPROC) (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
typedef void (PFNGLTEXIMAGE2DPROC) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
typedef void (PFNGLTEXPARAMETERFPROC) (GLenum target, GLenum pname, GLfloat param);
typedef void (PFNGLTEXPARAMETERFVPROC) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (PFNGLTEXPARAMETERIPROC) (GLenum target, GLenum pname, GLint param);
typedef void (PFNGLTEXPARAMETERIVPROC) (GLenum target, GLenum pname, const GLint *params);
typedef void (PFNGLTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
typedef void (PFNGLUNIFORM1FPROC) (GLint location, GLfloat v0);
typedef void (PFNGLUNIFORM1FVPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (PFNGLUNIFORM1IPROC) (GLint location, GLint v0);
typedef void (PFNGLUNIFORM1IVPROC) (GLint location, GLsizei count, const GLint *value);
typedef void (PFNGLUNIFORM2FPROC) (GLint location, GLfloat v0, GLfloat v1);
typedef void (PFNGLUNIFORM2FVPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (PFNGLUNIFORM2IPROC) (GLint location, GLint v0, GLint v1);
typedef void (PFNGLUNIFORM2IVPROC) (GLint location, GLsizei count, const GLint *value);
typedef void (PFNGLUNIFORM3FPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (PFNGLUNIFORM3FVPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (PFNGLUNIFORM3IPROC) (GLint location, GLint v0, GLint v1, GLint v2);
typedef void (PFNGLUNIFORM3IVPROC) (GLint location, GLsizei count, const GLint *value);
typedef void (PFNGLUNIFORM4FPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (PFNGLUNIFORM4FVPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (PFNGLUNIFORM4IPROC) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void (PFNGLUNIFORM4IVPROC) (GLint location, GLsizei count, const GLint *value);
typedef void (PFNGLUNIFORMMATRIX2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (PFNGLUNIFORMMATRIX3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (PFNGLUNIFORMMATRIX4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (PFNGLUSEPROGRAMPROC) (GLuint program);
typedef void (PFNGLVALIDATEPROGRAMPROC) (GLuint program);
typedef void (PFNGLVERTEXATTRIB1FPROC) (GLuint index, GLfloat x);
typedef void (PFNGLVERTEXATTRIB1FVPROC) (GLuint index, const GLfloat *v);
typedef void (PFNGLVERTEXATTRIB2FPROC) (GLuint index, GLfloat x, GLfloat y);
typedef void (PFNGLVERTEXATTRIB2FVPROC) (GLuint index, const GLfloat *v);
typedef void (PFNGLVERTEXATTRIB3FPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef void (PFNGLVERTEXATTRIB3FVPROC) (GLuint index, const GLfloat *v);
typedef void (PFNGLVERTEXATTRIB4FPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (PFNGLVERTEXATTRIB4FVPROC) (GLuint index, const GLfloat *v);
typedef void (PFNGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void (PFNGLVIEWPORTPROC) (GLint x, GLint y, GLsizei width, GLsizei height);
void ORCA_IMPORT(glActiveTexture) (GLenum texture);
void ORCA_IMPORT(glAttachShader) (GLuint program, GLuint shader);
void ORCA_IMPORT(glBindAttribLocation) (GLuint program, GLuint index, const GLchar *name);
void ORCA_IMPORT(glBindBuffer) (GLenum target, GLuint buffer);
void ORCA_IMPORT(glBindFramebuffer) (GLenum target, GLuint framebuffer);
void ORCA_IMPORT(glBindRenderbuffer) (GLenum target, GLuint renderbuffer);
void ORCA_IMPORT(glBindTexture) (GLenum target, GLuint texture);
void ORCA_IMPORT(glBlendColor) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void ORCA_IMPORT(glBlendEquation) (GLenum mode);
void ORCA_IMPORT(glBlendEquationSeparate) (GLenum modeRGB, GLenum modeAlpha);
void ORCA_IMPORT(glBlendFunc) (GLenum sfactor, GLenum dfactor);
void ORCA_IMPORT(glBlendFuncSeparate) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
void ORCA_IMPORT(glBufferData) (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
void ORCA_IMPORT(glBufferSubData) (GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
GLenum ORCA_IMPORT(glCheckFramebufferStatus) (GLenum target);
void ORCA_IMPORT(glClear) (GLbitfield mask);
void ORCA_IMPORT(glClearColor) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void ORCA_IMPORT(glClearDepthf) (GLfloat d);
void ORCA_IMPORT(glClearStencil) (GLint s);
void ORCA_IMPORT(glColorMask) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
void ORCA_IMPORT(glCompileShader) (GLuint shader);
void ORCA_IMPORT(glCompressedTexImage2D) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
void ORCA_IMPORT(glCompressedTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
void ORCA_IMPORT(glCopyTexImage2D) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
void ORCA_IMPORT(glCopyTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GLuint ORCA_IMPORT(glCreateProgram) (void);
GLuint ORCA_IMPORT(glCreateShader) (GLenum type);
void ORCA_IMPORT(glCullFace) (GLenum mode);
void ORCA_IMPORT(glDeleteBuffers) (GLsizei n, const GLuint *buffers);
void ORCA_IMPORT(glDeleteFramebuffers) (GLsizei n, const GLuint *framebuffers);
void ORCA_IMPORT(glDeleteProgram) (GLuint program);
void ORCA_IMPORT(glDeleteRenderbuffers) (GLsizei n, const GLuint *renderbuffers);
void ORCA_IMPORT(glDeleteShader) (GLuint shader);
void ORCA_IMPORT(glDeleteTextures) (GLsizei n, const GLuint *textures);
void ORCA_IMPORT(glDepthFunc) (GLenum func);
void ORCA_IMPORT(glDepthMask) (GLboolean flag);
void ORCA_IMPORT(glDepthRangef) (GLfloat n, GLfloat f);
void ORCA_IMPORT(glDetachShader) (GLuint program, GLuint shader);
void ORCA_IMPORT(glDisable) (GLenum cap);
void ORCA_IMPORT(glDisableVertexAttribArray) (GLuint index);
void ORCA_IMPORT(glDrawArrays) (GLenum mode, GLint first, GLsizei count);
void ORCA_IMPORT(glDrawElements) (GLenum mode, GLsizei count, GLenum type, const void *indices);
void ORCA_IMPORT(glEnable) (GLenum cap);
void ORCA_IMPORT(glEnableVertexAttribArray) (GLuint index);
void ORCA_IMPORT(glFinish) (void);
void ORCA_IMPORT(glFlush) (void);
void ORCA_IMPORT(glFramebufferRenderbuffer) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
void ORCA_IMPORT(glFramebufferTexture2D) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
void ORCA_IMPORT(glFrontFace) (GLenum mode);
void ORCA_IMPORT(glGenBuffers) (GLsizei n, GLuint *buffers);
void ORCA_IMPORT(glGenerateMipmap) (GLenum target);
void ORCA_IMPORT(glGenFramebuffers) (GLsizei n, GLuint *framebuffers);
void ORCA_IMPORT(glGenRenderbuffers) (GLsizei n, GLuint *renderbuffers);
void ORCA_IMPORT(glGenTextures) (GLsizei n, GLuint *textures);
void ORCA_IMPORT(glGetActiveAttrib) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
void ORCA_IMPORT(glGetActiveUniform) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
void ORCA_IMPORT(glGetAttachedShaders) (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
GLint ORCA_IMPORT(glGetAttribLocation) (GLuint program, const GLchar *name);
void ORCA_IMPORT(glGetBooleanv) (GLenum pname, GLboolean *data);
void ORCA_IMPORT(glGetBufferParameteriv) (GLenum target, GLenum pname, GLint *params);
GLenum ORCA_IMPORT(glGetError) (void);
void ORCA_IMPORT(glGetFloatv) (GLenum pname, GLfloat *data);
void ORCA_IMPORT(glGetFramebufferAttachmentParameteriv) (GLenum target, GLenum attachment, GLenum pname, GLint *params);
void ORCA_IMPORT(glGetIntegerv) (GLenum pname, GLint *data);
void ORCA_IMPORT(glGetProgramiv) (GLuint program, GLenum pname, GLint *params);
void ORCA_IMPORT(glGetProgramInfoLog) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
void ORCA_IMPORT(glGetRenderbufferParameteriv) (GLenum target, GLenum pname, GLint *params);
void ORCA_IMPORT(glGetShaderiv) (GLuint shader, GLenum pname, GLint *params);
void ORCA_IMPORT(glGetShaderInfoLog) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
void ORCA_IMPORT(glGetShaderPrecisionFormat) (GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
void ORCA_IMPORT(glGetShaderSource) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
const GLubyte *ORCA_IMPORT(glGetString) (GLenum name);
void ORCA_IMPORT(glGetTexParameterfv) (GLenum target, GLenum pname, GLfloat *params);
void ORCA_IMPORT(glGetTexParameteriv) (GLenum target, GLenum pname, GLint *params);
void ORCA_IMPORT(glGetUniformfv) (GLuint program, GLint location, GLfloat *params);
void ORCA_IMPORT(glGetUniformiv) (GLuint program, GLint location, GLint *params);
GLint ORCA_IMPORT(glGetUniformLocation) (GLuint program, const GLchar *name);
void ORCA_IMPORT(glGetVertexAttribfv) (GLuint index, GLenum pname, GLfloat *params);
void ORCA_IMPORT(glGetVertexAttribiv) (GLuint index, GLenum pname, GLint *params);
void ORCA_IMPORT(glGetVertexAttribPointerv) (GLuint index, GLenum pname, void **pointer);
void ORCA_IMPORT(glHint) (GLenum target, GLenum mode);
GLboolean ORCA_IMPORT(glIsBuffer) (GLuint buffer);
GLboolean ORCA_IMPORT(glIsEnabled) (GLenum cap);
GLboolean ORCA_IMPORT(glIsFramebuffer) (GLuint framebuffer);
GLboolean ORCA_IMPORT(glIsProgram) (GLuint program);
GLboolean ORCA_IMPORT(glIsRenderbuffer) (GLuint renderbuffer);
GLboolean ORCA_IMPORT(glIsShader) (GLuint shader);
GLboolean ORCA_IMPORT(glIsTexture) (GLuint texture);
void ORCA_IMPORT(glLineWidth) (GLfloat width);
void ORCA_IMPORT(glLinkProgram) (GLuint program);
void ORCA_IMPORT(glPixelStorei) (GLenum pname, GLint param);
void ORCA_IMPORT(glPolygonOffset) (GLfloat factor, GLfloat units);
void ORCA_IMPORT(glReadPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
void ORCA_IMPORT(glReleaseShaderCompiler) (void);
void ORCA_IMPORT(glRenderbufferStorage) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
void ORCA_IMPORT(glSampleCoverage) (GLfloat value, GLboolean invert);
void ORCA_IMPORT(glScissor) (GLint x, GLint y, GLsizei width, GLsizei height);
void ORCA_IMPORT(glShaderBinary) (GLsizei count, const GLuint *shaders, GLenum binaryFormat, const void *binary, GLsizei length);
void ORCA_IMPORT(glShaderSource) (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
void ORCA_IMPORT(glStencilFunc) (GLenum func, GLint ref, GLuint mask);
void ORCA_IMPORT(glStencilFuncSeparate) (GLenum face, GLenum func, GLint ref, GLuint mask);
void ORCA_IMPORT(glStencilMask) (GLuint mask);
void ORCA_IMPORT(glStencilMaskSeparate) (GLenum face, GLuint mask);
void ORCA_IMPORT(glStencilOp) (GLenum fail, GLenum zfail, GLenum zpass);
void ORCA_IMPORT(glStencilOpSeparate) (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
void ORCA_IMPORT(glTexImage2D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
void ORCA_IMPORT(glTexParameterf) (GLenum target, GLenum pname, GLfloat param);
void ORCA_IMPORT(glTexParameterfv) (GLenum target, GLenum pname, const GLfloat *params);
void ORCA_IMPORT(glTexParameteri) (GLenum target, GLenum pname, GLint param);
void ORCA_IMPORT(glTexParameteriv) (GLenum target, GLenum pname, const GLint *params);
void ORCA_IMPORT(glTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
void ORCA_IMPORT(glUniform1f) (GLint location, GLfloat v0);
void ORCA_IMPORT(glUniform1fv) (GLint location, GLsizei count, const GLfloat *value);
void ORCA_IMPORT(glUniform1i) (GLint location, GLint v0);
void ORCA_IMPORT(glUniform1iv) (GLint location, GLsizei count, const GLint *value);
void ORCA_IMPORT(glUniform2f) (GLint location, GLfloat v0, GLfloat v1);
void ORCA_IMPORT(glUniform2fv) (GLint location, GLsizei count, const GLfloat *value);
void ORCA_IMPORT(glUniform2i) (GLint location, GLint v0, GLint v1);
void ORCA_IMPORT(glUniform2iv) (GLint location, GLsizei count, const GLint *value);
void ORCA_IMPORT(glUniform3f) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
void ORCA_IMPORT(glUniform3fv) (GLint location, GLsizei count, const GLfloat *value);
void ORCA_IMPORT(glUniform3i) (GLint location, GLint v0, GLint v1, GLint v2);
void ORCA_IMPORT(glUniform3iv) (GLint location, GLsizei count, const GLint *value);
void ORCA_IMPORT(glUniform4f) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
void ORCA_IMPORT(glUniform4fv) (GLint location, GLsizei count, const GLfloat *value);
void ORCA_IMPORT(glUniform4i) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
void ORCA_IMPORT(glUniform4iv) (GLint location, GLsizei count, const GLint *value);
void ORCA_IMPORT(glUniformMatrix2fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void ORCA_IMPORT(glUniformMatrix3fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void ORCA_IMPORT(glUniformMatrix4fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void ORCA_IMPORT(glUseProgram) (GLuint program);
void ORCA_IMPORT(glValidateProgram) (GLuint program);
void ORCA_IMPORT(glVertexAttrib1f) (GLuint index, GLfloat x);
void ORCA_IMPORT(glVertexAttrib1fv) (GLuint index, const GLfloat *v);
void ORCA_IMPORT(glVertexAttrib2f) (GLuint index, GLfloat x, GLfloat y);
void ORCA_IMPORT(glVertexAttrib2fv) (GLuint index, const GLfloat *v);
void ORCA_IMPORT(glVertexAttrib3f) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
void ORCA_IMPORT(glVertexAttrib3fv) (GLuint index, const GLfloat *v);
void ORCA_IMPORT(glVertexAttrib4f) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void ORCA_IMPORT(glVertexAttrib4fv) (GLuint index, const GLfloat *v);
void ORCA_IMPORT(glVertexAttribPointer) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
void ORCA_IMPORT(glViewport) (GLint x, GLint y, GLsizei width, GLsizei height);
#endif /* GL_ES_VERSION_2_0 */

#ifndef GL_ES_VERSION_3_0
#define GL_ES_VERSION_3_0 1
typedef khronos_uint16_t GLhalf;
#define GL_READ_BUFFER                    0x0C02
#define GL_UNPACK_ROW_LENGTH              0x0CF2
#define GL_UNPACK_SKIP_ROWS               0x0CF3
#define GL_UNPACK_SKIP_PIXELS             0x0CF4
#define GL_PACK_ROW_LENGTH                0x0D02
#define GL_PACK_SKIP_ROWS                 0x0D03
#define GL_PACK_SKIP_PIXELS               0x0D04
#define GL_COLOR                          0x1800
#define GL_DEPTH                          0x1801
#define GL_STENCIL                        0x1802
#define GL_RED                            0x1903
#define GL_RGB8                           0x8051
#define GL_RGBA8                          0x8058
#define GL_RGB10_A2                       0x8059
#define GL_TEXTURE_BINDING_3D             0x806A
#define GL_UNPACK_SKIP_IMAGES             0x806D
#define GL_UNPACK_IMAGE_HEIGHT            0x806E
#define GL_TEXTURE_3D                     0x806F
#define GL_TEXTURE_WRAP_R                 0x8072
#define GL_MAX_3D_TEXTURE_SIZE            0x8073
#define GL_UNSIGNED_INT_2_10_10_10_REV    0x8368
#define GL_MAX_ELEMENTS_VERTICES          0x80E8
#define GL_MAX_ELEMENTS_INDICES           0x80E9
#define GL_TEXTURE_MIN_LOD                0x813A
#define GL_TEXTURE_MAX_LOD                0x813B
#define GL_TEXTURE_BASE_LEVEL             0x813C
#define GL_TEXTURE_MAX_LEVEL              0x813D
#define GL_MIN                            0x8007
#define GL_MAX                            0x8008
#define GL_DEPTH_COMPONENT24              0x81A6
#define GL_MAX_TEXTURE_LOD_BIAS           0x84FD
#define GL_TEXTURE_COMPARE_MODE           0x884C
#define GL_TEXTURE_COMPARE_FUNC           0x884D
#define GL_CURRENT_QUERY                  0x8865
#define GL_QUERY_RESULT                   0x8866
#define GL_QUERY_RESULT_AVAILABLE         0x8867
#define GL_BUFFER_MAPPED                  0x88BC
#define GL_BUFFER_MAP_POINTER             0x88BD
#define GL_STREAM_READ                    0x88E1
#define GL_STREAM_COPY                    0x88E2
#define GL_STATIC_READ                    0x88E5
#define GL_STATIC_COPY                    0x88E6
#define GL_DYNAMIC_READ                   0x88E9
#define GL_DYNAMIC_COPY                   0x88EA
#define GL_MAX_DRAW_BUFFERS               0x8824
#define GL_DRAW_BUFFER0                   0x8825
#define GL_DRAW_BUFFER1                   0x8826
#define GL_DRAW_BUFFER2                   0x8827
#define GL_DRAW_BUFFER3                   0x8828
#define GL_DRAW_BUFFER4                   0x8829
#define GL_DRAW_BUFFER5                   0x882A
#define GL_DRAW_BUFFER6                   0x882B
#define GL_DRAW_BUFFER7                   0x882C
#define GL_DRAW_BUFFER8                   0x882D
#define GL_DRAW_BUFFER9                   0x882E
#define GL_DRAW_BUFFER10                  0x882F
#define GL_DRAW_BUFFER11                  0x8830
#define GL_DRAW_BUFFER12                  0x8831
#define GL_DRAW_BUFFER13                  0x8832
#define GL_DRAW_BUFFER14                  0x8833
#define GL_DRAW_BUFFER15                  0x8834
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS 0x8B49
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS  0x8B4A
#define GL_SAMPLER_3D                     0x8B5F
#define GL_SAMPLER_2D_SHADOW              0x8B62
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT 0x8B8B
#define GL_PIXEL_PACK_BUFFER              0x88EB
#define GL_PIXEL_UNPACK_BUFFER            0x88EC
#define GL_PIXEL_PACK_BUFFER_BINDING      0x88ED
#define GL_PIXEL_UNPACK_BUFFER_BINDING    0x88EF
#define GL_FLOAT_MAT2x3                   0x8B65
#define GL_FLOAT_MAT2x4                   0x8B66
#define GL_FLOAT_MAT3x2                   0x8B67
#define GL_FLOAT_MAT3x4                   0x8B68
#define GL_FLOAT_MAT4x2                   0x8B69
#define GL_FLOAT_MAT4x3                   0x8B6A
#define GL_SRGB                           0x8C40
#define GL_SRGB8                          0x8C41
#define GL_SRGB8_ALPHA8                   0x8C43
#define GL_COMPARE_REF_TO_TEXTURE         0x884E
#define GL_MAJOR_VERSION                  0x821B
#define GL_MINOR_VERSION                  0x821C
#define GL_NUM_EXTENSIONS                 0x821D
#define GL_RGBA32F                        0x8814
#define GL_RGB32F                         0x8815
#define GL_RGBA16F                        0x881A
#define GL_RGB16F                         0x881B
#define GL_VERTEX_ATTRIB_ARRAY_INTEGER    0x88FD
#define GL_MAX_ARRAY_TEXTURE_LAYERS       0x88FF
#define GL_MIN_PROGRAM_TEXEL_OFFSET       0x8904
#define GL_MAX_PROGRAM_TEXEL_OFFSET       0x8905
#define GL_MAX_VARYING_COMPONENTS         0x8B4B
#define GL_TEXTURE_2D_ARRAY               0x8C1A
#define GL_TEXTURE_BINDING_2D_ARRAY       0x8C1D
#define GL_R11F_G11F_B10F                 0x8C3A
#define GL_UNSIGNED_INT_10F_11F_11F_REV   0x8C3B
#define GL_RGB9_E5                        0x8C3D
#define GL_UNSIGNED_INT_5_9_9_9_REV       0x8C3E
#define GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH 0x8C76
#define GL_TRANSFORM_FEEDBACK_BUFFER_MODE 0x8C7F
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS 0x8C80
#define GL_TRANSFORM_FEEDBACK_VARYINGS    0x8C83
#define GL_TRANSFORM_FEEDBACK_BUFFER_START 0x8C84
#define GL_TRANSFORM_FEEDBACK_BUFFER_SIZE 0x8C85
#define GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN 0x8C88
#define GL_RASTERIZER_DISCARD             0x8C89
#define GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS 0x8C8A
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS 0x8C8B
#define GL_INTERLEAVED_ATTRIBS            0x8C8C
#define GL_SEPARATE_ATTRIBS               0x8C8D
#define GL_TRANSFORM_FEEDBACK_BUFFER      0x8C8E
#define GL_TRANSFORM_FEEDBACK_BUFFER_BINDING 0x8C8F
#define GL_RGBA32UI                       0x8D70
#define GL_RGB32UI                        0x8D71
#define GL_RGBA16UI                       0x8D76
#define GL_RGB16UI                        0x8D77
#define GL_RGBA8UI                        0x8D7C
#define GL_RGB8UI                         0x8D7D
#define GL_RGBA32I                        0x8D82
#define GL_RGB32I                         0x8D83
#define GL_RGBA16I                        0x8D88
#define GL_RGB16I                         0x8D89
#define GL_RGBA8I                         0x8D8E
#define GL_RGB8I                          0x8D8F
#define GL_RED_INTEGER                    0x8D94
#define GL_RGB_INTEGER                    0x8D98
#define GL_RGBA_INTEGER                   0x8D99
#define GL_SAMPLER_2D_ARRAY               0x8DC1
#define GL_SAMPLER_2D_ARRAY_SHADOW        0x8DC4
#define GL_SAMPLER_CUBE_SHADOW            0x8DC5
#define GL_UNSIGNED_INT_VEC2              0x8DC6
#define GL_UNSIGNED_INT_VEC3              0x8DC7
#define GL_UNSIGNED_INT_VEC4              0x8DC8
#define GL_INT_SAMPLER_2D                 0x8DCA
#define GL_INT_SAMPLER_3D                 0x8DCB
#define GL_INT_SAMPLER_CUBE               0x8DCC
#define GL_INT_SAMPLER_2D_ARRAY           0x8DCF
#define GL_UNSIGNED_INT_SAMPLER_2D        0x8DD2
#define GL_UNSIGNED_INT_SAMPLER_3D        0x8DD3
#define GL_UNSIGNED_INT_SAMPLER_CUBE      0x8DD4
#define GL_UNSIGNED_INT_SAMPLER_2D_ARRAY  0x8DD7
#define GL_BUFFER_ACCESS_FLAGS            0x911F
#define GL_BUFFER_MAP_LENGTH              0x9120
#define GL_BUFFER_MAP_OFFSET              0x9121
#define GL_DEPTH_COMPONENT32F             0x8CAC
#define GL_DEPTH32F_STENCIL8              0x8CAD
#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV 0x8DAD
#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING 0x8210
#define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE 0x8211
#define GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE 0x8212
#define GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE 0x8213
#define GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE 0x8214
#define GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE 0x8215
#define GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE 0x8216
#define GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE 0x8217
#define GL_FRAMEBUFFER_DEFAULT            0x8218
#define GL_FRAMEBUFFER_UNDEFINED          0x8219
#define GL_DEPTH_STENCIL_ATTACHMENT       0x821A
#define GL_DEPTH_STENCIL                  0x84F9
#define GL_UNSIGNED_INT_24_8              0x84FA
#define GL_DEPTH24_STENCIL8               0x88F0
#define GL_UNSIGNED_NORMALIZED            0x8C17
#define GL_DRAW_FRAMEBUFFER_BINDING       0x8CA6
#define GL_READ_FRAMEBUFFER               0x8CA8
#define GL_DRAW_FRAMEBUFFER               0x8CA9
#define GL_READ_FRAMEBUFFER_BINDING       0x8CAA
#define GL_RENDERBUFFER_SAMPLES           0x8CAB
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER 0x8CD4
#define GL_MAX_COLOR_ATTACHMENTS          0x8CDF
#define GL_COLOR_ATTACHMENT1              0x8CE1
#define GL_COLOR_ATTACHMENT2              0x8CE2
#define GL_COLOR_ATTACHMENT3              0x8CE3
#define GL_COLOR_ATTACHMENT4              0x8CE4
#define GL_COLOR_ATTACHMENT5              0x8CE5
#define GL_COLOR_ATTACHMENT6              0x8CE6
#define GL_COLOR_ATTACHMENT7              0x8CE7
#define GL_COLOR_ATTACHMENT8              0x8CE8
#define GL_COLOR_ATTACHMENT9              0x8CE9
#define GL_COLOR_ATTACHMENT10             0x8CEA
#define GL_COLOR_ATTACHMENT11             0x8CEB
#define GL_COLOR_ATTACHMENT12             0x8CEC
#define GL_COLOR_ATTACHMENT13             0x8CED
#define GL_COLOR_ATTACHMENT14             0x8CEE
#define GL_COLOR_ATTACHMENT15             0x8CEF
#define GL_COLOR_ATTACHMENT16             0x8CF0
#define GL_COLOR_ATTACHMENT17             0x8CF1
#define GL_COLOR_ATTACHMENT18             0x8CF2
#define GL_COLOR_ATTACHMENT19             0x8CF3
#define GL_COLOR_ATTACHMENT20             0x8CF4
#define GL_COLOR_ATTACHMENT21             0x8CF5
#define GL_COLOR_ATTACHMENT22             0x8CF6
#define GL_COLOR_ATTACHMENT23             0x8CF7
#define GL_COLOR_ATTACHMENT24             0x8CF8
#define GL_COLOR_ATTACHMENT25             0x8CF9
#define GL_COLOR_ATTACHMENT26             0x8CFA
#define GL_COLOR_ATTACHMENT27             0x8CFB
#define GL_COLOR_ATTACHMENT28             0x8CFC
#define GL_COLOR_ATTACHMENT29             0x8CFD
#define GL_COLOR_ATTACHMENT30             0x8CFE
#define GL_COLOR_ATTACHMENT31             0x8CFF
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE 0x8D56
#define GL_MAX_SAMPLES                    0x8D57
#define GL_HALF_FLOAT                     0x140B
#define GL_MAP_READ_BIT                   0x0001
#define GL_MAP_WRITE_BIT                  0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT       0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT      0x0008
#define GL_MAP_FLUSH_EXPLICIT_BIT         0x0010
#define GL_MAP_UNSYNCHRONIZED_BIT         0x0020
#define GL_RG                             0x8227
#define GL_RG_INTEGER                     0x8228
#define GL_R8                             0x8229
#define GL_RG8                            0x822B
#define GL_R16F                           0x822D
#define GL_R32F                           0x822E
#define GL_RG16F                          0x822F
#define GL_RG32F                          0x8230
#define GL_R8I                            0x8231
#define GL_R8UI                           0x8232
#define GL_R16I                           0x8233
#define GL_R16UI                          0x8234
#define GL_R32I                           0x8235
#define GL_R32UI                          0x8236
#define GL_RG8I                           0x8237
#define GL_RG8UI                          0x8238
#define GL_RG16I                          0x8239
#define GL_RG16UI                         0x823A
#define GL_RG32I                          0x823B
#define GL_RG32UI                         0x823C
#define GL_VERTEX_ARRAY_BINDING           0x85B5
#define GL_R8_SNORM                       0x8F94
#define GL_RG8_SNORM                      0x8F95
#define GL_RGB8_SNORM                     0x8F96
#define GL_RGBA8_SNORM                    0x8F97
#define GL_SIGNED_NORMALIZED              0x8F9C
#define GL_PRIMITIVE_RESTART_FIXED_INDEX  0x8D69
#define GL_COPY_READ_BUFFER               0x8F36
#define GL_COPY_WRITE_BUFFER              0x8F37
#define GL_COPY_READ_BUFFER_BINDING       0x8F36
#define GL_COPY_WRITE_BUFFER_BINDING      0x8F37
#define GL_UNIFORM_BUFFER                 0x8A11
#define GL_UNIFORM_BUFFER_BINDING         0x8A28
#define GL_UNIFORM_BUFFER_START           0x8A29
#define GL_UNIFORM_BUFFER_SIZE            0x8A2A
#define GL_MAX_VERTEX_UNIFORM_BLOCKS      0x8A2B
#define GL_MAX_FRAGMENT_UNIFORM_BLOCKS    0x8A2D
#define GL_MAX_COMBINED_UNIFORM_BLOCKS    0x8A2E
#define GL_MAX_UNIFORM_BUFFER_BINDINGS    0x8A2F
#define GL_MAX_UNIFORM_BLOCK_SIZE         0x8A30
#define GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS 0x8A31
#define GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS 0x8A33
#define GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT 0x8A34
#define GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH 0x8A35
#define GL_ACTIVE_UNIFORM_BLOCKS          0x8A36
#define GL_UNIFORM_TYPE                   0x8A37
#define GL_UNIFORM_SIZE                   0x8A38
#define GL_UNIFORM_NAME_LENGTH            0x8A39
#define GL_UNIFORM_BLOCK_INDEX            0x8A3A
#define GL_UNIFORM_OFFSET                 0x8A3B
#define GL_UNIFORM_ARRAY_STRIDE           0x8A3C
#define GL_UNIFORM_MATRIX_STRIDE          0x8A3D
#define GL_UNIFORM_IS_ROW_MAJOR           0x8A3E
#define GL_UNIFORM_BLOCK_BINDING          0x8A3F
#define GL_UNIFORM_BLOCK_DATA_SIZE        0x8A40
#define GL_UNIFORM_BLOCK_NAME_LENGTH      0x8A41
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS  0x8A42
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES 0x8A43
#define GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER 0x8A44
#define GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER 0x8A46
#define GL_INVALID_INDEX                  0xFFFFFFFFu
#define GL_MAX_VERTEX_OUTPUT_COMPONENTS   0x9122
#define GL_MAX_FRAGMENT_INPUT_COMPONENTS  0x9125
#define GL_MAX_SERVER_WAIT_TIMEOUT        0x9111
#define GL_OBJECT_TYPE                    0x9112
#define GL_SYNC_CONDITION                 0x9113
#define GL_SYNC_STATUS                    0x9114
#define GL_SYNC_FLAGS                     0x9115
#define GL_SYNC_FENCE                     0x9116
#define GL_SYNC_GPU_COMMANDS_COMPLETE     0x9117
#define GL_UNSIGNALED                     0x9118
#define GL_SIGNALED                       0x9119
#define GL_ALREADY_SIGNALED               0x911A
#define GL_TIMEOUT_EXPIRED                0x911B
#define GL_CONDITION_SATISFIED            0x911C
#define GL_WAIT_FAILED                    0x911D
#define GL_SYNC_FLUSH_COMMANDS_BIT        0x00000001
#define GL_TIMEOUT_IGNORED                0xFFFFFFFFFFFFFFFFull
#define GL_VERTEX_ATTRIB_ARRAY_DIVISOR    0x88FE
#define GL_ANY_SAMPLES_PASSED             0x8C2F
#define GL_ANY_SAMPLES_PASSED_CONSERVATIVE 0x8D6A
#define GL_SAMPLER_BINDING                0x8919
#define GL_RGB10_A2UI                     0x906F
#define GL_TEXTURE_SWIZZLE_R              0x8E42
#define GL_TEXTURE_SWIZZLE_G              0x8E43
#define GL_TEXTURE_SWIZZLE_B              0x8E44
#define GL_TEXTURE_SWIZZLE_A              0x8E45
#define GL_GREEN                          0x1904
#define GL_BLUE                           0x1905
#define GL_INT_2_10_10_10_REV             0x8D9F
#define GL_TRANSFORM_FEEDBACK             0x8E22
#define GL_TRANSFORM_FEEDBACK_PAUSED      0x8E23
#define GL_TRANSFORM_FEEDBACK_ACTIVE      0x8E24
#define GL_TRANSFORM_FEEDBACK_BINDING     0x8E25
#define GL_PROGRAM_BINARY_RETRIEVABLE_HINT 0x8257
#define GL_PROGRAM_BINARY_LENGTH          0x8741
#define GL_NUM_PROGRAM_BINARY_FORMATS     0x87FE
#define GL_PROGRAM_BINARY_FORMATS         0x87FF
#define GL_COMPRESSED_R11_EAC             0x9270
#define GL_COMPRESSED_SIGNED_R11_EAC      0x9271
#define GL_COMPRESSED_RG11_EAC            0x9272
#define GL_COMPRESSED_SIGNED_RG11_EAC     0x9273
#define GL_COMPRESSED_RGB8_ETC2           0x9274
#define GL_COMPRESSED_SRGB8_ETC2          0x9275
#define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9276
#define GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9277
#define GL_COMPRESSED_RGBA8_ETC2_EAC      0x9278
#define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC 0x9279
#define GL_TEXTURE_IMMUTABLE_FORMAT       0x912F
#define GL_MAX_ELEMENT_INDEX              0x8D6B
#define GL_NUM_SAMPLE_COUNTS              0x9380
#define GL_TEXTURE_IMMUTABLE_LEVELS       0x82DF
typedef void (PFNGLREADBUFFERPROC) (GLenum src);
typedef void (PFNGLDRAWRANGEELEMENTSPROC) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices);
typedef void (PFNGLTEXIMAGE3DPROC) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
typedef void (PFNGLTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
typedef void (PFNGLCOPYTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (PFNGLCOMPRESSEDTEXIMAGE3DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data);
typedef void (PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
typedef void (PFNGLGENQUERIESPROC) (GLsizei n, GLuint *ids);
typedef void (PFNGLDELETEQUERIESPROC) (GLsizei n, const GLuint *ids);
typedef GLboolean (PFNGLISQUERYPROC) (GLuint id);
typedef void (PFNGLBEGINQUERYPROC) (GLenum target, GLuint id);
typedef void (PFNGLENDQUERYPROC) (GLenum target);
typedef void (PFNGLGETQUERYIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (PFNGLGETQUERYOBJECTUIVPROC) (GLuint id, GLenum pname, GLuint *params);
typedef void (PFNGLDRAWBUFFERSPROC) (GLsizei n, const GLenum *bufs);
typedef void (PFNGLUNIFORMMATRIX2X3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (PFNGLUNIFORMMATRIX3X2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (PFNGLUNIFORMMATRIX2X4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (PFNGLUNIFORMMATRIX4X2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (PFNGLUNIFORMMATRIX3X4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (PFNGLUNIFORMMATRIX4X3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (PFNGLBLITFRAMEBUFFERPROC) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
typedef void (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (PFNGLFRAMEBUFFERTEXTURELAYERPROC) (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
typedef void (PFNGLBINDVERTEXARRAYPROC) (GLuint array);
typedef void (PFNGLDELETEVERTEXARRAYSPROC) (GLsizei n, const GLuint *arrays);
typedef void (PFNGLGENVERTEXARRAYSPROC) (GLsizei n, GLuint *arrays);
typedef GLboolean (PFNGLISVERTEXARRAYPROC) (GLuint array);
typedef void (PFNGLGETINTEGERI_VPROC) (GLenum target, GLuint index, GLint *data);
typedef void (PFNGLBEGINTRANSFORMFEEDBACKPROC) (GLenum primitiveMode);
typedef void (PFNGLENDTRANSFORMFEEDBACKPROC) (void);
typedef void (PFNGLBINDBUFFERRANGEPROC) (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
typedef void (PFNGLBINDBUFFERBASEPROC) (GLenum target, GLuint index, GLuint buffer);
typedef void (PFNGLTRANSFORMFEEDBACKVARYINGSPROC) (GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode);
typedef void (PFNGLGETTRANSFORMFEEDBACKVARYINGPROC) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name);
typedef void (PFNGLVERTEXATTRIBIPOINTERPROC) (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
typedef void (PFNGLGETVERTEXATTRIBIIVPROC) (GLuint index, GLenum pname, GLint *params);
typedef void (PFNGLGETVERTEXATTRIBIUIVPROC) (GLuint index, GLenum pname, GLuint *params);
typedef void (PFNGLVERTEXATTRIBI4IPROC) (GLuint index, GLint x, GLint y, GLint z, GLint w);
typedef void (PFNGLVERTEXATTRIBI4UIPROC) (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
typedef void (PFNGLVERTEXATTRIBI4IVPROC) (GLuint index, const GLint *v);
typedef void (PFNGLVERTEXATTRIBI4UIVPROC) (GLuint index, const GLuint *v);
typedef void (PFNGLGETUNIFORMUIVPROC) (GLuint program, GLint location, GLuint *params);
typedef GLint (PFNGLGETFRAGDATALOCATIONPROC) (GLuint program, const GLchar *name);
typedef void (PFNGLUNIFORM1UIPROC) (GLint location, GLuint v0);
typedef void (PFNGLUNIFORM2UIPROC) (GLint location, GLuint v0, GLuint v1);
typedef void (PFNGLUNIFORM3UIPROC) (GLint location, GLuint v0, GLuint v1, GLuint v2);
typedef void (PFNGLUNIFORM4UIPROC) (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
typedef void (PFNGLUNIFORM1UIVPROC) (GLint location, GLsizei count, const GLuint *value);
typedef void (PFNGLUNIFORM2UIVPROC) (GLint location, GLsizei count, const GLuint *value);
typedef void (PFNGLUNIFORM3UIVPROC) (GLint location, GLsizei count, const GLuint *value);
typedef void (PFNGLUNIFORM4UIVPROC) (GLint location, GLsizei count, const GLuint *value);
typedef void (PFNGLCLEARBUFFERIVPROC) (GLenum buffer, GLint drawbuffer, const GLint *value);
typedef void (PFNGLCLEARBUFFERUIVPROC) (GLenum buffer, GLint drawbuffer, const GLuint *value);
typedef void (PFNGLCLEARBUFFERFVPROC) (GLenum buffer, GLint drawbuffer, const GLfloat *value);
typedef void (PFNGLCLEARBUFFERFIPROC) (GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
typedef const GLubyte *(PFNGLGETSTRINGIPROC) (GLenum name, GLuint index);
typedef void (PFNGLCOPYBUFFERSUBDATAPROC) (GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
typedef void (PFNGLGETUNIFORMINDICESPROC) (GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices);
typedef void (PFNGLGETACTIVEUNIFORMSIVPROC) (GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params);
typedef GLuint (PFNGLGETUNIFORMBLOCKINDEXPROC) (GLuint program, const GLchar *uniformBlockName);
typedef void (PFNGLGETACTIVEUNIFORMBLOCKIVPROC) (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
typedef void (PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC) (GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName);
typedef void (PFNGLUNIFORMBLOCKBINDINGPROC) (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
typedef void (PFNGLDRAWARRAYSINSTANCEDPROC) (GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
typedef void (PFNGLDRAWELEMENTSINSTANCEDPROC) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount);
typedef GLsync (PFNGLFENCESYNCPROC) (GLenum condition, GLbitfield flags);
typedef GLboolean (PFNGLISSYNCPROC) (GLsync sync);
typedef void (PFNGLDELETESYNCPROC) (GLsync sync);
typedef GLenum (PFNGLCLIENTWAITSYNCPROC) (GLsync sync, GLbitfield flags, GLuint64 timeout);
typedef void (PFNGLWAITSYNCPROC) (GLsync sync, GLbitfield flags, GLuint64 timeout);
typedef void (PFNGLGETINTEGER64VPROC) (GLenum pname, GLint64 *data);
typedef void (PFNGLGETSYNCIVPROC) (GLsync sync, GLenum pname, GLsizei count, GLsizei *length, GLint *values);
typedef void (PFNGLGETINTEGER64I_VPROC) (GLenum target, GLuint index, GLint64 *data);
typedef void (PFNGLGETBUFFERPARAMETERI64VPROC) (GLenum target, GLenum pname, GLint64 *params);
typedef void (PFNGLGENSAMPLERSPROC) (GLsizei count, GLuint *samplers);
typedef void (PFNGLDELETESAMPLERSPROC) (GLsizei count, const GLuint *samplers);
typedef GLboolean (PFNGLISSAMPLERPROC) (GLuint sampler);
typedef void (PFNGLBINDSAMPLERPROC) (GLuint unit, GLuint sampler);
typedef void (PFNGLSAMPLERPARAMETERIPROC) (GLuint sampler, GLenum pname, GLint param);
typedef void (PFNGLSAMPLERPARAMETERIVPROC) (GLuint sampler, GLenum pname, const GLint *param);
typedef void (PFNGLSAMPLERPARAMETERFPROC) (GLuint sampler, GLenum pname, GLfloat param);
typedef void (PFNGLSAMPLERPARAMETERFVPROC) (GLuint sampler, GLenum pname, const GLfloat *param);
typedef void (PFNGLGETSAMPLERPARAMETERIVPROC) (GLuint sampler, GLenum pname, GLint *params);
typedef void (PFNGLGETSAMPLERPARAMETERFVPROC) (GLuint sampler, GLenum pname, GLfloat *params);
typedef void (PFNGLVERTEXATTRIBDIVISORPROC) (GLuint index, GLuint divisor);
typedef void (PFNGLBINDTRANSFORMFEEDBACKPROC) (GLenum target, GLuint id);
typedef void (PFNGLDELETETRANSFORMFEEDBACKSPROC) (GLsizei n, const GLuint *ids);
typedef void (PFNGLGENTRANSFORMFEEDBACKSPROC) (GLsizei n, GLuint *ids);
typedef GLboolean (PFNGLISTRANSFORMFEEDBACKPROC) (GLuint id);
typedef void (PFNGLPAUSETRANSFORMFEEDBACKPROC) (void);
typedef void (PFNGLRESUMETRANSFORMFEEDBACKPROC) (void);
typedef void (PFNGLGETPROGRAMBINARYPROC) (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
typedef void (PFNGLPROGRAMBINARYPROC) (GLuint program, GLenum binaryFormat, const void *binary, GLsizei length);
typedef void (PFNGLPROGRAMPARAMETERIPROC) (GLuint program, GLenum pname, GLint value);
typedef void (PFNGLINVALIDATEFRAMEBUFFERPROC) (GLenum target, GLsizei numAttachments, const GLenum *attachments);
typedef void (PFNGLINVALIDATESUBFRAMEBUFFERPROC) (GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (PFNGLTEXSTORAGE2DPROC) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (PFNGLTEXSTORAGE3DPROC) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
typedef void (PFNGLGETINTERNALFORMATIVPROC) (GLenum target, GLenum internalformat, GLenum pname, GLsizei count, GLint *params);
void ORCA_IMPORT(glReadBuffer) (GLenum src);
void ORCA_IMPORT(glDrawRangeElements) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices);
void ORCA_IMPORT(glTexImage3D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
void ORCA_IMPORT(glTexSubImage3D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
void ORCA_IMPORT(glCopyTexSubImage3D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
void ORCA_IMPORT(glCompressedTexImage3D) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data);
void ORCA_IMPORT(glCompressedTexSubImage3D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
void ORCA_IMPORT(glGenQueries) (GLsizei n, GLuint *ids);
void ORCA_IMPORT(glDeleteQueries) (GLsizei n, const GLuint *ids);
GLboolean ORCA_IMPORT(glIsQuery) (GLuint id);
void ORCA_IMPORT(glBeginQuery) (GLenum target, GLuint id);
void ORCA_IMPORT(glEndQuery) (GLenum target);
void ORCA_IMPORT(glGetQueryiv) (GLenum target, GLenum pname, GLint *params);
void ORCA_IMPORT(glGetQueryObjectuiv) (GLuint id, GLenum pname, GLuint *params);
void ORCA_IMPORT(glDrawBuffers) (GLsizei n, const GLenum *bufs);
void ORCA_IMPORT(glUniformMatrix2x3fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void ORCA_IMPORT(glUniformMatrix3x2fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void ORCA_IMPORT(glUniformMatrix2x4fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void ORCA_IMPORT(glUniformMatrix4x2fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void ORCA_IMPORT(glUniformMatrix3x4fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void ORCA_IMPORT(glUniformMatrix4x3fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void ORCA_IMPORT(glBlitFramebuffer) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
void ORCA_IMPORT(glRenderbufferStorageMultisample) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
void ORCA_IMPORT(glFramebufferTextureLayer) (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
void ORCA_IMPORT(glBindVertexArray) (GLuint array);
void ORCA_IMPORT(glDeleteVertexArrays) (GLsizei n, const GLuint *arrays);
void ORCA_IMPORT(glGenVertexArrays) (GLsizei n, GLuint *arrays);
GLboolean ORCA_IMPORT(glIsVertexArray) (GLuint array);
void ORCA_IMPORT(glGetIntegeri_v) (GLenum target, GLuint index, GLint *data);
void ORCA_IMPORT(glBeginTransformFeedback) (GLenum primitiveMode);
void ORCA_IMPORT(glEndTransformFeedback) (void);
void ORCA_IMPORT(glBindBufferRange) (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
void ORCA_IMPORT(glBindBufferBase) (GLenum target, GLuint index, GLuint buffer);
void ORCA_IMPORT(glTransformFeedbackVaryings) (GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode);
void ORCA_IMPORT(glGetTransformFeedbackVarying) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name);
void ORCA_IMPORT(glVertexAttribIPointer) (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
void ORCA_IMPORT(glGetVertexAttribIiv) (GLuint index, GLenum pname, GLint *params);
void ORCA_IMPORT(glGetVertexAttribIuiv) (GLuint index, GLenum pname, GLuint *params);
void ORCA_IMPORT(glVertexAttribI4i) (GLuint index, GLint x, GLint y, GLint z, GLint w);
void ORCA_IMPORT(glVertexAttribI4ui) (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
void ORCA_IMPORT(glVertexAttribI4iv) (GLuint index, const GLint *v);
void ORCA_IMPORT(glVertexAttribI4uiv) (GLuint index, const GLuint *v);
void ORCA_IMPORT(glGetUniformuiv) (GLuint program, GLint location, GLuint *params);
GLint ORCA_IMPORT(glGetFragDataLocation) (GLuint program, const GLchar *name);
void ORCA_IMPORT(glUniform1ui) (GLint location, GLuint v0);
void ORCA_IMPORT(glUniform2ui) (GLint location, GLuint v0, GLuint v1);
void ORCA_IMPORT(glUniform3ui) (GLint location, GLuint v0, GLuint v1, GLuint v2);
void ORCA_IMPORT(glUniform4ui) (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
void ORCA_IMPORT(glUniform1uiv) (GLint location, GLsizei count, const GLuint *value);
void ORCA_IMPORT(glUniform2uiv) (GLint location, GLsizei count, const GLuint *value);
void ORCA_IMPORT(glUniform3uiv) (GLint location, GLsizei count, const GLuint *value);
void ORCA_IMPORT(glUniform4uiv) (GLint location, GLsizei count, const GLuint *value);
void ORCA_IMPORT(glClearBufferiv) (GLenum buffer, GLint drawbuffer, const GLint *value);
void ORCA_IMPORT(glClearBufferuiv) (GLenum buffer, GLint drawbuffer, const GLuint *value);
void ORCA_IMPORT(glClearBufferfv) (GLenum buffer, GLint drawbuffer, const GLfloat *value);
void ORCA_IMPORT(glClearBufferfi) (GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
const GLubyte *ORCA_IMPORT(glGetStringi) (GLenum name, GLuint index);
void ORCA_IMPORT(glCopyBufferSubData) (GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
void ORCA_IMPORT(glGetUniformIndices) (GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices);
void ORCA_IMPORT(glGetActiveUniformsiv) (GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params);
GLuint ORCA_IMPORT(glGetUniformBlockIndex) (GLuint program, const GLchar *uniformBlockName);
void ORCA_IMPORT(glGetActiveUniformBlockiv) (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
void ORCA_IMPORT(glGetActiveUniformBlockName) (GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName);
void ORCA_IMPORT(glUniformBlockBinding) (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
void ORCA_IMPORT(glDrawArraysInstanced) (GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
void ORCA_IMPORT(glDrawElementsInstanced) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount);
GLsync ORCA_IMPORT(glFenceSync) (GLenum condition, GLbitfield flags);
GLboolean ORCA_IMPORT(glIsSync) (GLsync sync);
void ORCA_IMPORT(glDeleteSync) (GLsync sync);
GLenum ORCA_IMPORT(glClientWaitSync) (GLsync sync, GLbitfield flags, GLuint64 timeout);
void ORCA_IMPORT(glWaitSync) (GLsync sync, GLbitfield flags, GLuint64 timeout);
void ORCA_IMPORT(glGetInteger64v) (GLenum pname, GLint64 *data);
void ORCA_IMPORT(glGetSynciv) (GLsync sync, GLenum pname, GLsizei count, GLsizei *length, GLint *values);
void ORCA_IMPORT(glGetInteger64i_v) (GLenum target, GLuint index, GLint64 *data);
void ORCA_IMPORT(glGetBufferParameteri64v) (GLenum target, GLenum pname, GLint64 *params);
void ORCA_IMPORT(glGenSamplers) (GLsizei count, GLuint *samplers);
void ORCA_IMPORT(glDeleteSamplers) (GLsizei count, const GLuint *samplers);
GLboolean ORCA_IMPORT(glIsSampler) (GLuint sampler);
void ORCA_IMPORT(glBindSampler) (GLuint unit, GLuint sampler);
void ORCA_IMPORT(glSamplerParameteri) (GLuint sampler, GLenum pname, GLint param);
void ORCA_IMPORT(glSamplerParameteriv) (GLuint sampler, GLenum pname, const GLint *param);
void ORCA_IMPORT(glSamplerParameterf) (GLuint sampler, GLenum pname, GLfloat param);
void ORCA_IMPORT(glSamplerParameterfv) (GLuint sampler, GLenum pname, const GLfloat *param);
void ORCA_IMPORT(glGetSamplerParameteriv) (GLuint sampler, GLenum pname, GLint *params);
void ORCA_IMPORT(glGetSamplerParameterfv) (GLuint sampler, GLenum pname, GLfloat *params);
void ORCA_IMPORT(glVertexAttribDivisor) (GLuint index, GLuint divisor);
void ORCA_IMPORT(glBindTransformFeedback) (GLenum target, GLuint id);
void ORCA_IMPORT(glDeleteTransformFeedbacks) (GLsizei n, const GLuint *ids);
void ORCA_IMPORT(glGenTransformFeedbacks) (GLsizei n, GLuint *ids);
GLboolean ORCA_IMPORT(glIsTransformFeedback) (GLuint id);
void ORCA_IMPORT(glPauseTransformFeedback) (void);
void ORCA_IMPORT(glResumeTransformFeedback) (void);
void ORCA_IMPORT(glGetProgramBinary) (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
void ORCA_IMPORT(glProgramBinary) (GLuint program, GLenum binaryFormat, const void *binary, GLsizei length);
void ORCA_IMPORT(glProgramParameteri) (GLuint program, GLenum pname, GLint value);
void ORCA_IMPORT(glInvalidateFramebuffer) (GLenum target, GLsizei numAttachments, const GLenum *attachments);
void ORCA_IMPORT(glInvalidateSubFramebuffer) (GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height);
void ORCA_IMPORT(glTexStorage2D) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
void ORCA_IMPORT(glTexStorage3D) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
void ORCA_IMPORT(glGetInternalformativ) (GLenum target, GLenum internalformat, GLenum pname, GLsizei count, GLint *params);
#endif /* GL_ES_VERSION_3_0 */

#ifndef GL_ES_VERSION_3_1
#define GL_ES_VERSION_3_1 1
#define GL_COMPUTE_SHADER                 0x91B9
#define GL_MAX_COMPUTE_UNIFORM_BLOCKS     0x91BB
#define GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS 0x91BC
#define GL_MAX_COMPUTE_IMAGE_UNIFORMS     0x91BD
#define GL_MAX_COMPUTE_SHARED_MEMORY_SIZE 0x8262
#define GL_MAX_COMPUTE_UNIFORM_COMPONENTS 0x8263
#define GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS 0x8264
#define GL_MAX_COMPUTE_ATOMIC_COUNTERS    0x8265
#define GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS 0x8266
#define GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS 0x90EB
#define GL_MAX_COMPUTE_WORK_GROUP_COUNT   0x91BE
#define GL_MAX_COMPUTE_WORK_GROUP_SIZE    0x91BF
#define GL_COMPUTE_WORK_GROUP_SIZE        0x8267
#define GL_DISPATCH_INDIRECT_BUFFER       0x90EE
#define GL_DISPATCH_INDIRECT_BUFFER_BINDING 0x90EF
#define GL_COMPUTE_SHADER_BIT             0x00000020
#define GL_DRAW_INDIRECT_BUFFER           0x8F3F
#define GL_DRAW_INDIRECT_BUFFER_BINDING   0x8F43
#define GL_MAX_UNIFORM_LOCATIONS          0x826E
#define GL_FRAMEBUFFER_DEFAULT_WIDTH      0x9310
#define GL_FRAMEBUFFER_DEFAULT_HEIGHT     0x9311
#define GL_FRAMEBUFFER_DEFAULT_SAMPLES    0x9313
#define GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS 0x9314
#define GL_MAX_FRAMEBUFFER_WIDTH          0x9315
#define GL_MAX_FRAMEBUFFER_HEIGHT         0x9316
#define GL_MAX_FRAMEBUFFER_SAMPLES        0x9318
#define GL_UNIFORM                        0x92E1
#define GL_UNIFORM_BLOCK                  0x92E2
#define GL_PROGRAM_INPUT                  0x92E3
#define GL_PROGRAM_OUTPUT                 0x92E4
#define GL_BUFFER_VARIABLE                0x92E5
#define GL_SHADER_STORAGE_BLOCK           0x92E6
#define GL_ATOMIC_COUNTER_BUFFER          0x92C0
#define GL_TRANSFORM_FEEDBACK_VARYING     0x92F4
#define GL_ACTIVE_RESOURCES               0x92F5
#define GL_MAX_NAME_LENGTH                0x92F6
#define GL_MAX_NUM_ACTIVE_VARIABLES       0x92F7
#define GL_NAME_LENGTH                    0x92F9
#define GL_TYPE                           0x92FA
#define GL_ARRAY_SIZE                     0x92FB
#define GL_OFFSET                         0x92FC
#define GL_BLOCK_INDEX                    0x92FD
#define GL_ARRAY_STRIDE                   0x92FE
#define GL_MATRIX_STRIDE                  0x92FF
#define GL_IS_ROW_MAJOR                   0x9300
#define GL_ATOMIC_COUNTER_BUFFER_INDEX    0x9301
#define GL_BUFFER_BINDING                 0x9302
#define GL_BUFFER_DATA_SIZE               0x9303
#define GL_NUM_ACTIVE_VARIABLES           0x9304
#define GL_ACTIVE_VARIABLES               0x9305
#define GL_REFERENCED_BY_VERTEX_SHADER    0x9306
#define GL_REFERENCED_BY_FRAGMENT_SHADER  0x930A
#define GL_REFERENCED_BY_COMPUTE_SHADER   0x930B
#define GL_TOP_LEVEL_ARRAY_SIZE           0x930C
#define GL_TOP_LEVEL_ARRAY_STRIDE         0x930D
#define GL_LOCATION                       0x930E
#define GL_VERTEX_SHADER_BIT              0x00000001
#define GL_FRAGMENT_SHADER_BIT            0x00000002
#define GL_ALL_SHADER_BITS                0xFFFFFFFF
#define GL_PROGRAM_SEPARABLE              0x8258
#define GL_ACTIVE_PROGRAM                 0x8259
#define GL_PROGRAM_PIPELINE_BINDING       0x825A
#define GL_ATOMIC_COUNTER_BUFFER_BINDING  0x92C1
#define GL_ATOMIC_COUNTER_BUFFER_START    0x92C2
#define GL_ATOMIC_COUNTER_BUFFER_SIZE     0x92C3
#define GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS 0x92CC
#define GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS 0x92D0
#define GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS 0x92D1
#define GL_MAX_VERTEX_ATOMIC_COUNTERS     0x92D2
#define GL_MAX_FRAGMENT_ATOMIC_COUNTERS   0x92D6
#define GL_MAX_COMBINED_ATOMIC_COUNTERS   0x92D7
#define GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE 0x92D8
#define GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS 0x92DC
#define GL_ACTIVE_ATOMIC_COUNTER_BUFFERS  0x92D9
#define GL_UNSIGNED_INT_ATOMIC_COUNTER    0x92DB
#define GL_MAX_IMAGE_UNITS                0x8F38
#define GL_MAX_VERTEX_IMAGE_UNIFORMS      0x90CA
#define GL_MAX_FRAGMENT_IMAGE_UNIFORMS    0x90CE
#define GL_MAX_COMBINED_IMAGE_UNIFORMS    0x90CF
#define GL_IMAGE_BINDING_NAME             0x8F3A
#define GL_IMAGE_BINDING_LEVEL            0x8F3B
#define GL_IMAGE_BINDING_LAYERED          0x8F3C
#define GL_IMAGE_BINDING_LAYER            0x8F3D
#define GL_IMAGE_BINDING_ACCESS           0x8F3E
#define GL_IMAGE_BINDING_FORMAT           0x906E
#define GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT 0x00000001
#define GL_ELEMENT_ARRAY_BARRIER_BIT      0x00000002
#define GL_UNIFORM_BARRIER_BIT            0x00000004
#define GL_TEXTURE_FETCH_BARRIER_BIT      0x00000008
#define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT 0x00000020
#define GL_COMMAND_BARRIER_BIT            0x00000040
#define GL_PIXEL_BUFFER_BARRIER_BIT       0x00000080
#define GL_TEXTURE_UPDATE_BARRIER_BIT     0x00000100
#define GL_BUFFER_UPDATE_BARRIER_BIT      0x00000200
#define GL_FRAMEBUFFER_BARRIER_BIT        0x00000400
#define GL_TRANSFORM_FEEDBACK_BARRIER_BIT 0x00000800
#define GL_ATOMIC_COUNTER_BARRIER_BIT     0x00001000
#define GL_ALL_BARRIER_BITS               0xFFFFFFFF
#define GL_IMAGE_2D                       0x904D
#define GL_IMAGE_3D                       0x904E
#define GL_IMAGE_CUBE                     0x9050
#define GL_IMAGE_2D_ARRAY                 0x9053
#define GL_INT_IMAGE_2D                   0x9058
#define GL_INT_IMAGE_3D                   0x9059
#define GL_INT_IMAGE_CUBE                 0x905B
#define GL_INT_IMAGE_2D_ARRAY             0x905E
#define GL_UNSIGNED_INT_IMAGE_2D          0x9063
#define GL_UNSIGNED_INT_IMAGE_3D          0x9064
#define GL_UNSIGNED_INT_IMAGE_CUBE        0x9066
#define GL_UNSIGNED_INT_IMAGE_2D_ARRAY    0x9069
#define GL_IMAGE_FORMAT_COMPATIBILITY_TYPE 0x90C7
#define GL_IMAGE_FORMAT_COMPATIBILITY_BY_SIZE 0x90C8
#define GL_IMAGE_FORMAT_COMPATIBILITY_BY_CLASS 0x90C9
#define GL_READ_ONLY                      0x88B8
#define GL_WRITE_ONLY                     0x88B9
#define GL_READ_WRITE                     0x88BA
#define GL_SHADER_STORAGE_BUFFER          0x90D2
#define GL_SHADER_STORAGE_BUFFER_BINDING  0x90D3
#define GL_SHADER_STORAGE_BUFFER_START    0x90D4
#define GL_SHADER_STORAGE_BUFFER_SIZE     0x90D5
#define GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS 0x90D6
#define GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS 0x90DA
#define GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS 0x90DB
#define GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS 0x90DC
#define GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS 0x90DD
#define GL_MAX_SHADER_STORAGE_BLOCK_SIZE  0x90DE
#define GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT 0x90DF
#define GL_SHADER_STORAGE_BARRIER_BIT     0x00002000
#define GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES 0x8F39
#define GL_DEPTH_STENCIL_TEXTURE_MODE     0x90EA
#define GL_STENCIL_INDEX                  0x1901
#define GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET 0x8E5E
#define GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET 0x8E5F
#define GL_SAMPLE_POSITION                0x8E50
#define GL_SAMPLE_MASK                    0x8E51
#define GL_SAMPLE_MASK_VALUE              0x8E52
#define GL_TEXTURE_2D_MULTISAMPLE         0x9100
#define GL_MAX_SAMPLE_MASK_WORDS          0x8E59
#define GL_MAX_COLOR_TEXTURE_SAMPLES      0x910E
#define GL_MAX_DEPTH_TEXTURE_SAMPLES      0x910F
#define GL_MAX_INTEGER_SAMPLES            0x9110
#define GL_TEXTURE_BINDING_2D_MULTISAMPLE 0x9104
#define GL_TEXTURE_SAMPLES                0x9106
#define GL_TEXTURE_FIXED_SAMPLE_LOCATIONS 0x9107
#define GL_TEXTURE_WIDTH                  0x1000
#define GL_TEXTURE_HEIGHT                 0x1001
#define GL_TEXTURE_DEPTH                  0x8071
#define GL_TEXTURE_INTERNAL_FORMAT        0x1003
#define GL_TEXTURE_RED_SIZE               0x805C
#define GL_TEXTURE_GREEN_SIZE             0x805D
#define GL_TEXTURE_BLUE_SIZE              0x805E
#define GL_TEXTURE_ALPHA_SIZE             0x805F
#define GL_TEXTURE_DEPTH_SIZE             0x884A
#define GL_TEXTURE_STENCIL_SIZE           0x88F1
#define GL_TEXTURE_SHARED_SIZE            0x8C3F
#define GL_TEXTURE_RED_TYPE               0x8C10
#define GL_TEXTURE_GREEN_TYPE             0x8C11
#define GL_TEXTURE_BLUE_TYPE              0x8C12
#define GL_TEXTURE_ALPHA_TYPE             0x8C13
#define GL_TEXTURE_DEPTH_TYPE             0x8C16
#define GL_TEXTURE_COMPRESSED             0x86A1
#define GL_SAMPLER_2D_MULTISAMPLE         0x9108
#define GL_INT_SAMPLER_2D_MULTISAMPLE     0x9109
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE 0x910A
#define GL_VERTEX_ATTRIB_BINDING          0x82D4
#define GL_VERTEX_ATTRIB_RELATIVE_OFFSET  0x82D5
#define GL_VERTEX_BINDING_DIVISOR         0x82D6
#define GL_VERTEX_BINDING_OFFSET          0x82D7
#define GL_VERTEX_BINDING_STRIDE          0x82D8
#define GL_VERTEX_BINDING_BUFFER          0x8F4F
#define GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET 0x82D9
#define GL_MAX_VERTEX_ATTRIB_BINDINGS     0x82DA
#define GL_MAX_VERTEX_ATTRIB_STRIDE       0x82E5
typedef void (PFNGLDISPATCHCOMPUTEPROC) (GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
typedef void (PFNGLDISPATCHCOMPUTEINDIRECTPROC) (GLintptr indirect);
typedef void (PFNGLDRAWARRAYSINDIRECTPROC) (GLenum mode, const void *indirect);
typedef void (PFNGLDRAWELEMENTSINDIRECTPROC) (GLenum mode, GLenum type, const void *indirect);
typedef void (PFNGLFRAMEBUFFERPARAMETERIPROC) (GLenum target, GLenum pname, GLint param);
typedef void (PFNGLGETFRAMEBUFFERPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (PFNGLGETPROGRAMINTERFACEIVPROC) (GLuint program, GLenum programInterface, GLenum pname, GLint *params);
typedef GLuint (PFNGLGETPROGRAMRESOURCEINDEXPROC) (GLuint program, GLenum programInterface, const GLchar *name);
typedef void (PFNGLGETPROGRAMRESOURCENAMEPROC) (GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);
typedef void (PFNGLGETPROGRAMRESOURCEIVPROC) (GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei count, GLsizei *length, GLint *params);
typedef GLint (PFNGLGETPROGRAMRESOURCELOCATIONPROC) (GLuint program, GLenum programInterface, const GLchar *name);
typedef void (PFNGLUSEPROGRAMSTAGESPROC) (GLuint pipeline, GLbitfield stages, GLuint program);
typedef void (PFNGLACTIVESHADERPROGRAMPROC) (GLuint pipeline, GLuint program);
typedef GLuint (PFNGLCREATESHADERPROGRAMVPROC) (GLenum type, GLsizei count, const GLchar *const*strings);
typedef void (PFNGLBINDPROGRAMPIPELINEPROC) (GLuint pipeline);
typedef void (PFNGLDELETEPROGRAMPIPELINESPROC) (GLsizei n, const GLuint *pipelines);
typedef void (PFNGLGENPROGRAMPIPELINESPROC) (GLsizei n, GLuint *pipelines);
typedef GLboolean (PFNGLISPROGRAMPIPELINEPROC) (GLuint pipeline);
typedef void (PFNGLGETPROGRAMPIPELINEIVPROC) (GLuint pipeline, GLenum pname, GLint *params);
typedef void (PFNGLPROGRAMUNIFORM1IPROC) (GLuint program, GLint location, GLint v0);
typedef void (PFNGLPROGRAMUNIFORM2IPROC) (GLuint program, GLint location, GLint v0, GLint v1);
typedef void (PFNGLPROGRAMUNIFORM3IPROC) (GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
typedef void (PFNGLPROGRAMUNIFORM4IPROC) (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void (PFNGLPROGRAMUNIFORM1UIPROC) (GLuint program, GLint location, GLuint v0);
typedef void (PFNGLPROGRAMUNIFORM2UIPROC) (GLuint program, GLint location, GLuint v0, GLuint v1);
typedef void (PFNGLPROGRAMUNIFORM3UIPROC) (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
typedef void (PFNGLPROGRAMUNIFORM4UIPROC) (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
typedef void (PFNGLPROGRAMUNIFORM1FPROC) (GLuint program, GLint location, GLfloat v0);
typedef void (PFNGLPROGRAMUNIFORM2FPROC) (GLuint program, GLint location, GLfloat v0, GLfloat v1);
typedef void (PFNGLPROGRAMUNIFORM3FPROC) (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (PFNGLPROGRAMUNIFORM4FPROC) (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (PFNGLPROGRAMUNIFORM1IVPROC) (GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void (PFNGLPROGRAMUNIFORM2IVPROC) (GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void (PFNGLPROGRAMUNIFORM3IVPROC) (GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void (PFNGLPROGRAMUNIFORM4IVPROC) (GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void (PFNGLPROGRAMUNIFORM1UIVPROC) (GLuint program, GLint location, GLsizei count, const GLuint *value);
typedef void (PFNGLPROGRAMUNIFORM2UIVPROC) (GLuint program, GLint location, GLsizei count, const GLuint *value);
typedef void (PFNGLPROGRAMUNIFORM3UIVPROC) (GLuint program, GLint location, GLsizei count, const GLuint *value);
typedef void (PFNGLPROGRAMUNIFORM4UIVPROC) (GLuint program, GLint location, GLsizei count, const GLuint *value);
typedef void (PFNGLPROGRAMUNIFORM1FVPROC) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void (PFNGLPROGRAMUNIFORM2FVPROC) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void (PFNGLPROGRAMUNIFORM3FVPROC) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void (PFNGLPROGRAMUNIFORM4FVPROC) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void (PFNGLPROGRAMUNIFORMMATRIX2FVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (PFNGLPROGRAMUNIFORMMATRIX3FVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (PFNGLPROGRAMUNIFORMMATRIX4FVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (PFNGLVALIDATEPROGRAMPIPELINEPROC) (GLuint pipeline);
typedef void (PFNGLGETPROGRAMPIPELINEINFOLOGPROC) (GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (PFNGLBINDIMAGETEXTUREPROC) (GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
typedef void (PFNGLGETBOOLEANI_VPROC) (GLenum target, GLuint index, GLboolean *data);
typedef void (PFNGLMEMORYBARRIERPROC) (GLbitfield barriers);
typedef void (PFNGLMEMORYBARRIERBYREGIONPROC) (GLbitfield barriers);
typedef void (PFNGLTEXSTORAGE2DMULTISAMPLEPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
typedef void (PFNGLGETMULTISAMPLEFVPROC) (GLenum pname, GLuint index, GLfloat *val);
typedef void (PFNGLSAMPLEMASKIPROC) (GLuint maskNumber, GLbitfield mask);
typedef void (PFNGLGETTEXLEVELPARAMETERIVPROC) (GLenum target, GLint level, GLenum pname, GLint *params);
typedef void (PFNGLGETTEXLEVELPARAMETERFVPROC) (GLenum target, GLint level, GLenum pname, GLfloat *params);
typedef void (PFNGLBINDVERTEXBUFFERPROC) (GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
typedef void (PFNGLVERTEXATTRIBFORMATPROC) (GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
typedef void (PFNGLVERTEXATTRIBIFORMATPROC) (GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
typedef void (PFNGLVERTEXATTRIBBINDINGPROC) (GLuint attribindex, GLuint bindingindex);
typedef void (PFNGLVERTEXBINDINGDIVISORPROC) (GLuint bindingindex, GLuint divisor);
void ORCA_IMPORT(glDispatchCompute) (GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
void ORCA_IMPORT(glDispatchComputeIndirect) (GLintptr indirect);
void ORCA_IMPORT(glDrawArraysIndirect) (GLenum mode, const void *indirect);
void ORCA_IMPORT(glDrawElementsIndirect) (GLenum mode, GLenum type, const void *indirect);
void ORCA_IMPORT(glFramebufferParameteri) (GLenum target, GLenum pname, GLint param);
void ORCA_IMPORT(glGetFramebufferParameteriv) (GLenum target, GLenum pname, GLint *params);
void ORCA_IMPORT(glGetProgramInterfaceiv) (GLuint program, GLenum programInterface, GLenum pname, GLint *params);
GLuint ORCA_IMPORT(glGetProgramResourceIndex) (GLuint program, GLenum programInterface, const GLchar *name);
void ORCA_IMPORT(glGetProgramResourceName) (GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);
void ORCA_IMPORT(glGetProgramResourceiv) (GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei count, GLsizei *length, GLint *params);
GLint ORCA_IMPORT(glGetProgramResourceLocation) (GLuint program, GLenum programInterface, const GLchar *name);
void ORCA_IMPORT(glUseProgramStages) (GLuint pipeline, GLbitfield stages, GLuint program);
void ORCA_IMPORT(glActiveShaderProgram) (GLuint pipeline, GLuint program);
GLuint ORCA_IMPORT(glCreateShaderProgramv) (GLenum type, GLsizei count, const GLchar *const*strings);
void ORCA_IMPORT(glBindProgramPipeline) (GLuint pipeline);
void ORCA_IMPORT(glDeleteProgramPipelines) (GLsizei n, const GLuint *pipelines);
void ORCA_IMPORT(glGenProgramPipelines) (GLsizei n, GLuint *pipelines);
GLboolean ORCA_IMPORT(glIsProgramPipeline) (GLuint pipeline);
void ORCA_IMPORT(glGetProgramPipelineiv) (GLuint pipeline, GLenum pname, GLint *params);
void ORCA_IMPORT(glProgramUniform1i) (GLuint program, GLint location, GLint v0);
void ORCA_IMPORT(glProgramUniform2i) (GLuint program, GLint location, GLint v0, GLint v1);
void ORCA_IMPORT(glProgramUniform3i) (GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
void ORCA_IMPORT(glProgramUniform4i) (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
void ORCA_IMPORT(glProgramUniform1ui) (GLuint program, GLint location, GLuint v0);
void ORCA_IMPORT(glProgramUniform2ui) (GLuint program, GLint location, GLuint v0, GLuint v1);
void ORCA_IMPORT(glProgramUniform3ui) (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
void ORCA_IMPORT(glProgramUniform4ui) (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
void ORCA_IMPORT(glProgramUniform1f) (GLuint program, GLint location, GLfloat v0);
void ORCA_IMPORT(glProgramUniform2f) (GLuint program, GLint location, GLfloat v0, GLfloat v1);
void ORCA_IMPORT(glProgramUniform3f) (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
void ORCA_IMPORT(glProgramUniform4f) (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
void ORCA_IMPORT(glProgramUniform1iv) (GLuint program, GLint location, GLsizei count, const GLint *value);
void ORCA_IMPORT(glProgramUniform2iv) (GLuint program, GLint location, GLsizei count, const GLint *value);
void ORCA_IMPORT(glProgramUniform3iv) (GLuint program, GLint location, GLsizei count, const GLint *value);
void ORCA_IMPORT(glProgramUniform4iv) (GLuint program, GLint location, GLsizei count, const GLint *value);
void ORCA_IMPORT(glProgramUniform1uiv) (GLuint program, GLint location, GLsizei count, const GLuint *value);
void ORCA_IMPORT(glProgramUniform2uiv) (GLuint program, GLint location, GLsizei count, const GLuint *value);
void ORCA_IMPORT(glProgramUniform3uiv) (GLuint program, GLint location, GLsizei count, const GLuint *value);
void ORCA_IMPORT(glProgramUniform4uiv) (GLuint program, GLint location, GLsizei count, const GLuint *value);
void ORCA_IMPORT(glProgramUniform1fv) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
void ORCA_IMPORT(glProgramUniform2fv) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
void ORCA_IMPORT(glProgramUniform3fv) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
void ORCA_IMPORT(glProgramUniform4fv) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
void ORCA_IMPORT(glProgramUniformMatrix2fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void ORCA_IMPORT(glProgramUniformMatrix3fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void ORCA_IMPORT(glProgramUniformMatrix4fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void ORCA_IMPORT(glProgramUniformMatrix2x3fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void ORCA_IMPORT(glProgramUniformMatrix3x2fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void ORCA_IMPORT(glProgramUniformMatrix2x4fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void ORCA_IMPORT(glProgramUniformMatrix4x2fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void ORCA_IMPORT(glProgramUniformMatrix3x4fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void ORCA_IMPORT(glProgramUniformMatrix4x3fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void ORCA_IMPORT(glValidateProgramPipeline) (GLuint pipeline);
void ORCA_IMPORT(glGetProgramPipelineInfoLog) (GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
void ORCA_IMPORT(glBindImageTexture) (GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
void ORCA_IMPORT(glGetBooleani_v) (GLenum target, GLuint index, GLboolean *data);
void ORCA_IMPORT(glMemoryBarrier) (GLbitfield barriers);
void ORCA_IMPORT(glMemoryBarrierByRegion) (GLbitfield barriers);
void ORCA_IMPORT(glTexStorage2DMultisample) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
void ORCA_IMPORT(glGetMultisamplefv) (GLenum pname, GLuint index, GLfloat *val);
void ORCA_IMPORT(glSampleMaski) (GLuint maskNumber, GLbitfield mask);
void ORCA_IMPORT(glGetTexLevelParameteriv) (GLenum target, GLint level, GLenum pname, GLint *params);
void ORCA_IMPORT(glGetTexLevelParameterfv) (GLenum target, GLint level, GLenum pname, GLfloat *params);
void ORCA_IMPORT(glBindVertexBuffer) (GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
void ORCA_IMPORT(glVertexAttribFormat) (GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
void ORCA_IMPORT(glVertexAttribIFormat) (GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
void ORCA_IMPORT(glVertexAttribBinding) (GLuint attribindex, GLuint bindingindex);
void ORCA_IMPORT(glVertexBindingDivisor) (GLuint bindingindex, GLuint divisor);
#endif /* GL_ES_VERSION_3_1 */

#ifdef __cplusplus
}
#endif

#endif
