/********************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
*********************************************************/
#pragma once

#include "GL/glcorearb.h"
#include "GLES3/gl32.h"

typedef struct oc_gl_api
{
    const char* name;
    PFNGLGETFLOATVPROC GetFloatv;
    PFNGLTEXBUFFERRANGEPROC TexBufferRange;
    PFNGLISBUFFERPROC IsBuffer;
    PFNGLISTEXTUREPROC IsTexture;
    PFNGLDEPTHRANGEFPROC DepthRangef;
    PFNGLENDCONDITIONALRENDERPROC EndConditionalRender;
    PFNGLBLENDFUNCIPROC BlendFunci;
    PFNGLGETPROGRAMPIPELINEIVPROC GetProgramPipelineiv;
    PFNGLWAITSYNCPROC WaitSync;
    PFNGLPROGRAMUNIFORMMATRIX2FVPROC ProgramUniformMatrix2fv;
    PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC ProgramUniformMatrix4x3dv;
    PFNGLVERTEXATTRIB1DVPROC VertexAttrib1dv;
    PFNGLSAMPLERPARAMETERIPROC SamplerParameteri;
    PFNGLGETVERTEXATTRIBIIVPROC GetVertexAttribIiv;
    PFNGLGETSAMPLERPARAMETERFVPROC GetSamplerParameterfv;
    PFNGLVERTEXATTRIB1DPROC VertexAttrib1d;
    PFNGLTEXBUFFERPROC TexBuffer;
    PFNGLINVALIDATEBUFFERDATAPROC InvalidateBufferData;
    PFNGLPROGRAMUNIFORM2IPROC ProgramUniform2i;
    PFNGLUNIFORM4DVPROC Uniform4dv;
    PFNGLUSEPROGRAMPROC UseProgram;
    PFNGLVERTEXATTRIBI3IVPROC VertexAttribI3iv;
    PFNGLDRAWELEMENTSINDIRECTPROC DrawElementsIndirect;
    PFNGLVERTEXATTRIB4UIVPROC VertexAttrib4uiv;
    PFNGLGETQUERYOBJECTIVPROC GetQueryObjectiv;
    PFNGLFRAMEBUFFERRENDERBUFFERPROC FramebufferRenderbuffer;
    PFNGLBLENDEQUATIONIPROC BlendEquationi;
    PFNGLGETACTIVESUBROUTINENAMEPROC GetActiveSubroutineName;
    PFNGLVERTEXATTRIB2SPROC VertexAttrib2s;
    PFNGLVERTEXATTRIBL1DPROC VertexAttribL1d;
    PFNGLBINDTEXTURESPROC BindTextures;
    PFNGLVERTEXATTRIB3SVPROC VertexAttrib3sv;
    PFNGLGETFLOATI_VPROC GetFloati_v;
    PFNGLBEGINTRANSFORMFEEDBACKPROC BeginTransformFeedback;
    PFNGLCLEARSTENCILPROC ClearStencil;
    PFNGLUNIFORM3IPROC Uniform3i;
    PFNGLVALIDATEPROGRAMPIPELINEPROC ValidateProgramPipeline;
    PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC ProgramUniformMatrix4x2fv;
    PFNGLVERTEXATTRIBI4UIPROC VertexAttribI4ui;
    PFNGLGETSHADERIVPROC GetShaderiv;
    PFNGLREADNPIXELSPROC ReadnPixels;
    PFNGLUNIFORMMATRIX4X2FVPROC UniformMatrix4x2fv;
    PFNGLGETSHADERPRECISIONFORMATPROC GetShaderPrecisionFormat;
    PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC ProgramUniformMatrix2x3fv;
    PFNGLTEXSUBIMAGE3DPROC TexSubImage3D;
    PFNGLGETPROGRAMRESOURCELOCATIONINDEXPROC GetProgramResourceLocationIndex;
    PFNGLBLENDFUNCPROC BlendFunc;
    PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC ProgramUniformMatrix3x4fv;
    PFNGLUNIFORM3DPROC Uniform3d;
    PFNGLVERTEXATTRIB1SVPROC VertexAttrib1sv;
    PFNGLBINDFRAGDATALOCATIONPROC BindFragDataLocation;
    PFNGLVERTEXATTRIB4BVPROC VertexAttrib4bv;
    PFNGLUNIFORM4IVPROC Uniform4iv;
    PFNGLPROGRAMUNIFORM2UIPROC ProgramUniform2ui;
    PFNGLDRAWARRAYSPROC DrawArrays;
    PFNGLPROGRAMBINARYPROC ProgramBinary;
    PFNGLVERTEXATTRIB4FPROC VertexAttrib4f;
    PFNGLVERTEXATTRIBP2UIVPROC VertexAttribP2uiv;
    PFNGLUNIFORMMATRIX3FVPROC UniformMatrix3fv;
    PFNGLUNIFORM2IPROC Uniform2i;
    PFNGLGETQUERYOBJECTUIVPROC GetQueryObjectuiv;
    PFNGLUNIFORMBLOCKBINDINGPROC UniformBlockBinding;
    PFNGLSAMPLECOVERAGEPROC SampleCoverage;
    PFNGLVERTEXATTRIB4NUSVPROC VertexAttrib4Nusv;
    PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC ProgramUniformMatrix2x4dv;
    PFNGLUNIFORM3UIVPROC Uniform3uiv;
    PFNGLVERTEXATTRIB1SPROC VertexAttrib1s;
    PFNGLGETVERTEXATTRIBPOINTERVPROC GetVertexAttribPointerv;
    PFNGLBLENDBARRIERPROC BlendBarrier;
    PFNGLDRAWRANGEELEMENTSPROC DrawRangeElements;
    PFNGLTEXSTORAGE3DPROC TexStorage3D;
    PFNGLGETINTERNALFORMATI64VPROC GetInternalformati64v;
    PFNGLGETQUERYOBJECTI64VPROC GetQueryObjecti64v;
    PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC CompressedTexSubImage1D;
    PFNGLVERTEXATTRIB3DVPROC VertexAttrib3dv;
    PFNGLVERTEXBINDINGDIVISORPROC VertexBindingDivisor;
    PFNGLUSEPROGRAMSTAGESPROC UseProgramStages;
    PFNGLVERTEXATTRIBBINDINGPROC VertexAttribBinding;
    PFNGLDEBUGMESSAGEINSERTPROC DebugMessageInsert;
    PFNGLGETTEXPARAMETERIVPROC GetTexParameteriv;
    PFNGLMULTIDRAWARRAYSINDIRECTPROC MultiDrawArraysIndirect;
    PFNGLGETTEXPARAMETERFVPROC GetTexParameterfv;
    PFNGLGETPROGRAMPIPELINEINFOLOGPROC GetProgramPipelineInfoLog;
    PFNGLENDQUERYPROC EndQuery;
    PFNGLGETPROGRAMRESOURCELOCATIONPROC GetProgramResourceLocation;
    PFNGLCOMPRESSEDTEXIMAGE2DPROC CompressedTexImage2D;
    PFNGLVERTEXATTRIBP2UIPROC VertexAttribP2ui;
    PFNGLISENABLEDIPROC IsEnabledi;
    PFNGLGETACTIVEATOMICCOUNTERBUFFERIVPROC GetActiveAtomicCounterBufferiv;
    PFNGLISPROGRAMPROC IsProgram;
    PFNGLUNIFORM1DVPROC Uniform1dv;
    PFNGLTEXPARAMETERIVPROC TexParameteriv;
    PFNGLUNIFORM2FVPROC Uniform2fv;
    PFNGLRELEASESHADERCOMPILERPROC ReleaseShaderCompiler;
    PFNGLCULLFACEPROC CullFace;
    PFNGLVERTEXATTRIBI4IPROC VertexAttribI4i;
    PFNGLGETPROGRAMRESOURCEINDEXPROC GetProgramResourceIndex;
    PFNGLSHADERBINARYPROC ShaderBinary;
    PFNGLUNIFORMMATRIX3X2DVPROC UniformMatrix3x2dv;
    PFNGLINVALIDATEFRAMEBUFFERPROC InvalidateFramebuffer;
    PFNGLATTACHSHADERPROC AttachShader;
    PFNGLFLUSHMAPPEDBUFFERRANGEPROC FlushMappedBufferRange;
    PFNGLVERTEXATTRIBP3UIVPROC VertexAttribP3uiv;
    PFNGLGETACTIVEUNIFORMNAMEPROC GetActiveUniformName;
    PFNGLMAPBUFFERPROC MapBuffer;
    PFNGLDRAWBUFFERSPROC DrawBuffers;
    PFNGLGETSYNCIVPROC GetSynciv;
    PFNGLCOPYTEXSUBIMAGE2DPROC CopyTexSubImage2D;
    PFNGLOBJECTLABELPROC ObjectLabel;
    PFNGLBUFFERSUBDATAPROC BufferSubData;
    PFNGLUNIFORM2FPROC Uniform2f;
    PFNGLDEBUGMESSAGECALLBACKPROC DebugMessageCallback;
    PFNGLVERTEXATTRIBL4DVPROC VertexAttribL4dv;
    PFNGLISPROGRAMPIPELINEPROC IsProgramPipeline;
    PFNGLRESUMETRANSFORMFEEDBACKPROC ResumeTransformFeedback;
    PFNGLVERTEXATTRIBI4IVPROC VertexAttribI4iv;
    PFNGLGETSHADERINFOLOGPROC GetShaderInfoLog;
    PFNGLGETINTEGERI_VPROC GetIntegeri_v;
    PFNGLBINDVERTEXBUFFERPROC BindVertexBuffer;
    PFNGLBLENDEQUATIONPROC BlendEquation;
    PFNGLVERTEXATTRIBL2DVPROC VertexAttribL2dv;
    PFNGLVERTEXATTRIBI1UIPROC VertexAttribI1ui;
    PFNGLVERTEXATTRIB4NSVPROC VertexAttrib4Nsv;
    PFNGLVERTEXATTRIBL4DPROC VertexAttribL4d;
    PFNGLCOPYIMAGESUBDATAPROC CopyImageSubData;
    PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC GetFramebufferAttachmentParameteriv;
    PFNGLVERTEXATTRIBL2DPROC VertexAttribL2d;
    PFNGLGETSUBROUTINEINDEXPROC GetSubroutineIndex;
    PFNGLVERTEXATTRIBI3UIVPROC VertexAttribI3uiv;
    PFNGLVERTEXATTRIB4IVPROC VertexAttrib4iv;
    PFNGLBINDVERTEXBUFFERSPROC BindVertexBuffers;
    PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC ProgramUniformMatrix2x3dv;
    PFNGLPRIMITIVEBOUNDINGBOXPROC PrimitiveBoundingBox;
    PFNGLSCISSORPROC Scissor;
    PFNGLCLIENTWAITSYNCPROC ClientWaitSync;
    PFNGLUNIFORM3UIPROC Uniform3ui;
    PFNGLVERTEXATTRIBP3UIPROC VertexAttribP3ui;
    PFNGLENABLEPROC Enable;
    PFNGLSTENCILOPSEPARATEPROC StencilOpSeparate;
    PFNGLUNIFORMMATRIX2X3DVPROC UniformMatrix2x3dv;
    PFNGLPROGRAMUNIFORMMATRIX3DVPROC ProgramUniformMatrix3dv;
    PFNGLTEXIMAGE2DMULTISAMPLEPROC TexImage2DMultisample;
    PFNGLVERTEXATTRIB4NBVPROC VertexAttrib4Nbv;
    PFNGLGETTEXIMAGEPROC GetTexImage;
    PFNGLVERTEXATTRIB4SVPROC VertexAttrib4sv;
    PFNGLPIXELSTOREIPROC PixelStorei;
    PFNGLDEPTHMASKPROC DepthMask;
    PFNGLTEXSTORAGE2DPROC TexStorage2D;
    PFNGLCLEARPROC Clear;
    PFNGLUNIFORMMATRIX3X4DVPROC UniformMatrix3x4dv;
    PFNGLDELETETRANSFORMFEEDBACKSPROC DeleteTransformFeedbacks;
    PFNGLMAPBUFFERRANGEPROC MapBufferRange;
    PFNGLMEMORYBARRIERPROC MemoryBarrier;
    PFNGLVIEWPORTINDEXEDFPROC ViewportIndexedf;
    PFNGLVERTEXATTRIB3FVPROC VertexAttrib3fv;
    PFNGLOBJECTPTRLABELPROC ObjectPtrLabel;
    PFNGLTEXSTORAGE1DPROC TexStorage1D;
    PFNGLCOMPRESSEDTEXIMAGE3DPROC CompressedTexImage3D;
    PFNGLVERTEXATTRIB1FVPROC VertexAttrib1fv;
    PFNGLVERTEXATTRIBPOINTERPROC VertexAttribPointer;
    PFNGLGETQUERYINDEXEDIVPROC GetQueryIndexediv;
    PFNGLCOMPILESHADERPROC CompileShader;
    PFNGLPROGRAMUNIFORM1IPROC ProgramUniform1i;
    PFNGLGETQUERYIVPROC GetQueryiv;
    PFNGLVERTEXATTRIBI1IVPROC VertexAttribI1iv;
    PFNGLCOPYTEXIMAGE2DPROC CopyTexImage2D;
    PFNGLGETQUERYOBJECTUI64VPROC GetQueryObjectui64v;
    PFNGLPOINTSIZEPROC PointSize;
    PFNGLDISABLEIPROC Disablei;
    PFNGLVERTEXATTRIBL1DVPROC VertexAttribL1dv;
    PFNGLCREATESHADERPROC CreateShader;
    PFNGLGETSTRINGPROC GetString;
    PFNGLVIEWPORTARRAYVPROC ViewportArrayv;
    PFNGLPROGRAMUNIFORM3DPROC ProgramUniform3d;
    PFNGLVERTEXATTRIB4NUBVPROC VertexAttrib4Nubv;
    PFNGLTEXPARAMETERIPROC TexParameteri;
    PFNGLPROGRAMUNIFORM4FVPROC ProgramUniform4fv;
    PFNGLGENERATEMIPMAPPROC GenerateMipmap;
    PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC CompressedTexSubImage3D;
    PFNGLUNIFORM3FPROC Uniform3f;
    PFNGLGETUNIFORMINDICESPROC GetUniformIndices;
    PFNGLVERTEXATTRIBLPOINTERPROC VertexAttribLPointer;
    PFNGLVERTEXATTRIBI2UIVPROC VertexAttribI2uiv;
    PFNGLQUERYCOUNTERPROC QueryCounter;
    PFNGLACTIVESHADERPROGRAMPROC ActiveShaderProgram;
    PFNGLUNIFORM1UIPROC Uniform1ui;
    PFNGLVERTEXATTRIBI1IPROC VertexAttribI1i;
    PFNGLGETTEXPARAMETERIIVPROC GetTexParameterIiv;
    PFNGLGETUNIFORMFVPROC GetUniformfv;
    PFNGLPROGRAMUNIFORM2UIVPROC ProgramUniform2uiv;
    PFNGLGETERRORPROC GetError;
    PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC GetActiveUniformBlockName;
    PFNGLTEXTUREVIEWPROC TextureView;
    PFNGLGETNUNIFORMIVPROC GetnUniformiv;
    PFNGLPROGRAMUNIFORM4DVPROC ProgramUniform4dv;
    PFNGLVIEWPORTINDEXEDFVPROC ViewportIndexedfv;
    PFNGLHINTPROC Hint;
    PFNGLGETSHADERSOURCEPROC GetShaderSource;
    PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC ProgramUniformMatrix4x3fv;
    PFNGLUNIFORM1IVPROC Uniform1iv;
    PFNGLVERTEXATTRIBI4BVPROC VertexAttribI4bv;
    PFNGLUNIFORMMATRIX4X2DVPROC UniformMatrix4x2dv;
    PFNGLBUFFERSTORAGEPROC BufferStorage;
    PFNGLISRENDERBUFFERPROC IsRenderbuffer;
    PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC GetActiveSubroutineUniformName;
    PFNGLLINKPROGRAMPROC LinkProgram;
    PFNGLGETACTIVEUNIFORMSIVPROC GetActiveUniformsiv;
    PFNGLGETDEBUGMESSAGELOGPROC GetDebugMessageLog;
    PFNGLCOPYTEXSUBIMAGE3DPROC CopyTexSubImage3D;
    PFNGLPOINTPARAMETERIPROC PointParameteri;
    PFNGLPROGRAMUNIFORM3DVPROC ProgramUniform3dv;
    PFNGLCOMPRESSEDTEXIMAGE1DPROC CompressedTexImage1D;
    PFNGLUNIFORMMATRIX3X4FVPROC UniformMatrix3x4fv;
    PFNGLGENSAMPLERSPROC GenSamplers;
    PFNGLGETCOMPRESSEDTEXIMAGEPROC GetCompressedTexImage;
    PFNGLDELETEQUERIESPROC DeleteQueries;
    PFNGLGENPROGRAMPIPELINESPROC GenProgramPipelines;
    PFNGLDISPATCHCOMPUTEINDIRECTPROC DispatchComputeIndirect;
    PFNGLVERTEXATTRIBIPOINTERPROC VertexAttribIPointer;
    PFNGLCREATEPROGRAMPROC CreateProgram;
    PFNGLCLEARTEXSUBIMAGEPROC ClearTexSubImage;
    PFNGLVERTEXATTRIB4DPROC VertexAttrib4d;
    PFNGLFRONTFACEPROC FrontFace;
    PFNGLBINDTRANSFORMFEEDBACKPROC BindTransformFeedback;
    PFNGLGETPROGRAMSTAGEIVPROC GetProgramStageiv;
    PFNGLSAMPLERPARAMETERIIVPROC SamplerParameterIiv;
    PFNGLGETINTEGER64VPROC GetInteger64v;
    PFNGLCREATESHADERPROGRAMVPROC CreateShaderProgramv;
    PFNGLBINDBUFFERSRANGEPROC BindBuffersRange;
    PFNGLUNIFORM3FVPROC Uniform3fv;
    PFNGLPROGRAMUNIFORMMATRIX4FVPROC ProgramUniformMatrix4fv;
    PFNGLBINDBUFFERSBASEPROC BindBuffersBase;
    PFNGLCLEARBUFFERFIPROC ClearBufferfi;
    PFNGLFRAMEBUFFERTEXTURE3DPROC FramebufferTexture3D;
    PFNGLDISABLEPROC Disable;
    PFNGLPROGRAMUNIFORM1IVPROC ProgramUniform1iv;
    PFNGLVERTEXATTRIBI2IVPROC VertexAttribI2iv;
    PFNGLDEPTHRANGEINDEXEDPROC DepthRangeIndexed;
    PFNGLPATCHPARAMETERIPROC PatchParameteri;
    PFNGLGETUNIFORMBLOCKINDEXPROC GetUniformBlockIndex;
    PFNGLMULTIDRAWARRAYSPROC MultiDrawArrays;
    PFNGLVERTEXATTRIBI4UBVPROC VertexAttribI4ubv;
    PFNGLBINDBUFFERPROC BindBuffer;
    PFNGLVERTEXATTRIBI3IPROC VertexAttribI3i;
    PFNGLGETDOUBLEVPROC GetDoublev;
    PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC DrawTransformFeedbackStream;
    PFNGLVERTEXATTRIBI4UIVPROC VertexAttribI4uiv;
    PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC RenderbufferStorageMultisample;
    PFNGLVERTEXATTRIBL3DVPROC VertexAttribL3dv;
    PFNGLSTENCILMASKSEPARATEPROC StencilMaskSeparate;
    PFNGLPROGRAMUNIFORM1DPROC ProgramUniform1d;
    PFNGLVIEWPORTPROC Viewport;
    PFNGLVERTEXATTRIBP1UIPROC VertexAttribP1ui;
    PFNGLVERTEXATTRIB4DVPROC VertexAttrib4dv;
    PFNGLGENQUERIESPROC GenQueries;
    PFNGLTEXPARAMETERIIVPROC TexParameterIiv;
    PFNGLPROGRAMUNIFORM2DPROC ProgramUniform2d;
    PFNGLPROGRAMUNIFORM1UIVPROC ProgramUniform1uiv;
    PFNGLVERTEXATTRIB4NUBPROC VertexAttrib4Nub;
    PFNGLISVERTEXARRAYPROC IsVertexArray;
    PFNGLPROGRAMUNIFORM3FPROC ProgramUniform3f;
    PFNGLPROGRAMUNIFORM3IVPROC ProgramUniform3iv;
    PFNGLGETPROGRAMBINARYPROC GetProgramBinary;
    PFNGLBINDRENDERBUFFERPROC BindRenderbuffer;
    PFNGLBINDFRAGDATALOCATIONINDEXEDPROC BindFragDataLocationIndexed;
    PFNGLGETSAMPLERPARAMETERIIVPROC GetSamplerParameterIiv;
    PFNGLVERTEXATTRIBDIVISORPROC VertexAttribDivisor;
    PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC ProgramUniformMatrix3x2dv;
    PFNGLFRAMEBUFFERPARAMETERIPROC FramebufferParameteri;
    PFNGLGENTRANSFORMFEEDBACKSPROC GenTransformFeedbacks;
    PFNGLDELETESYNCPROC DeleteSync;
    PFNGLPROGRAMUNIFORM1UIPROC ProgramUniform1ui;
    PFNGLTEXSUBIMAGE1DPROC TexSubImage1D;
    PFNGLCLEARDEPTHFPROC ClearDepthf;
    PFNGLREADPIXELSPROC ReadPixels;
    PFNGLVERTEXATTRIBI2IPROC VertexAttribI2i;
    PFNGLFINISHPROC Finish;
    PFNGLLINEWIDTHPROC LineWidth;
    PFNGLDELETESHADERPROC DeleteShader;
    PFNGLISSAMPLERPROC IsSampler;
    PFNGLPROGRAMUNIFORMMATRIX4DVPROC ProgramUniformMatrix4dv;
    PFNGLTRANSFORMFEEDBACKVARYINGSPROC TransformFeedbackVaryings;
    PFNGLBEGINCONDITIONALRENDERPROC BeginConditionalRender;
    PFNGLBINDSAMPLERSPROC BindSamplers;
    PFNGLDELETEPROGRAMPIPELINESPROC DeleteProgramPipelines;
    PFNGLCOLORMASKPROC ColorMask;
    PFNGLTEXPARAMETERFVPROC TexParameterfv;
    PFNGLPUSHDEBUGGROUPPROC PushDebugGroup;
    PFNGLCLEARBUFFERFVPROC ClearBufferfv;
    PFNGLISENABLEDPROC IsEnabled;
    PFNGLVERTEXATTRIB2FPROC VertexAttrib2f;
    PFNGLPROGRAMUNIFORM2FPROC ProgramUniform2f;
    PFNGLGETSAMPLERPARAMETERIUIVPROC GetSamplerParameterIuiv;
    PFNGLGETINTEGER64I_VPROC GetInteger64i_v;
    PFNGLUNIFORM2DVPROC Uniform2dv;
    PFNGLGETBUFFERSUBDATAPROC GetBufferSubData;
    PFNGLMULTIDRAWELEMENTSINDIRECTPROC MultiDrawElementsIndirect;
    PFNGLPROGRAMPARAMETERIPROC ProgramParameteri;
    PFNGLVERTEXATTRIBP4UIPROC VertexAttribP4ui;
    PFNGLSAMPLERPARAMETERFVPROC SamplerParameterfv;
    PFNGLPOINTPARAMETERFPROC PointParameterf;
    PFNGLUNIFORMMATRIX2X4FVPROC UniformMatrix2x4fv;
    PFNGLGENBUFFERSPROC GenBuffers;
    PFNGLPROGRAMUNIFORM2DVPROC ProgramUniform2dv;
    PFNGLVERTEXATTRIBFORMATPROC VertexAttribFormat;
    PFNGLTEXSUBIMAGE2DPROC TexSubImage2D;
    PFNGLVERTEXATTRIB4UBVPROC VertexAttrib4ubv;
    PFNGLGETGRAPHICSRESETSTATUSPROC GetGraphicsResetStatus;
    PFNGLGETPROGRAMINTERFACEIVPROC GetProgramInterfaceiv;
    PFNGLVERTEXATTRIBIFORMATPROC VertexAttribIFormat;
    PFNGLGETNUNIFORMFVPROC GetnUniformfv;
    PFNGLDELETEPROGRAMPROC DeleteProgram;
    PFNGLCLAMPCOLORPROC ClampColor;
    PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC DrawElementsInstancedBaseVertexBaseInstance;
    PFNGLDRAWELEMENTSPROC DrawElements;
    PFNGLDEBUGMESSAGECONTROLPROC DebugMessageControl;
    PFNGLGETRENDERBUFFERPARAMETERIVPROC GetRenderbufferParameteriv;
    PFNGLDETACHSHADERPROC DetachShader;
    PFNGLGENFRAMEBUFFERSPROC GenFramebuffers;
    PFNGLPROVOKINGVERTEXPROC ProvokingVertex;
    PFNGLSAMPLEMASKIPROC SampleMaski;
    PFNGLENDQUERYINDEXEDPROC EndQueryIndexed;
    PFNGLPROGRAMUNIFORM1FPROC ProgramUniform1f;
    PFNGLBINDFRAMEBUFFERPROC BindFramebuffer;
    PFNGLBEGINQUERYINDEXEDPROC BeginQueryIndexed;
    PFNGLUNIFORMSUBROUTINESUIVPROC UniformSubroutinesuiv;
    PFNGLGETUNIFORMIVPROC GetUniformiv;
    PFNGLFRAMEBUFFERTEXTUREPROC FramebufferTexture;
    PFNGLPOINTPARAMETERFVPROC PointParameterfv;
    PFNGLISTRANSFORMFEEDBACKPROC IsTransformFeedback;
    PFNGLCHECKFRAMEBUFFERSTATUSPROC CheckFramebufferStatus;
    PFNGLSHADERSOURCEPROC ShaderSource;
    PFNGLUNIFORMMATRIX2X4DVPROC UniformMatrix2x4dv;
    PFNGLBINDIMAGETEXTURESPROC BindImageTextures;
    PFNGLCOPYTEXIMAGE1DPROC CopyTexImage1D;
    PFNGLUNIFORMMATRIX3DVPROC UniformMatrix3dv;
    PFNGLPROGRAMUNIFORM1DVPROC ProgramUniform1dv;
    PFNGLBLITFRAMEBUFFERPROC BlitFramebuffer;
    PFNGLPOPDEBUGGROUPPROC PopDebugGroup;
    PFNGLTEXPARAMETERIUIVPROC TexParameterIuiv;
    PFNGLVERTEXATTRIB2DPROC VertexAttrib2d;
    PFNGLTEXIMAGE1DPROC TexImage1D;
    PFNGLGETOBJECTPTRLABELPROC GetObjectPtrLabel;
    PFNGLSTENCILMASKPROC StencilMask;
    PFNGLBEGINQUERYPROC BeginQuery;
    PFNGLUNIFORMMATRIX4FVPROC UniformMatrix4fv;
    PFNGLISSYNCPROC IsSync;
    PFNGLUNIFORM3DVPROC Uniform3dv;
    PFNGLPROGRAMUNIFORM2FVPROC ProgramUniform2fv;
    PFNGLVERTEXATTRIBI4SVPROC VertexAttribI4sv;
    PFNGLSCISSORARRAYVPROC ScissorArrayv;
    PFNGLVERTEXATTRIBP1UIVPROC VertexAttribP1uiv;
    PFNGLUNIFORM2UIVPROC Uniform2uiv;
    PFNGLDELETEBUFFERSPROC DeleteBuffers;
    PFNGLPROGRAMUNIFORM3UIPROC ProgramUniform3ui;
    PFNGLFRAMEBUFFERTEXTURELAYERPROC FramebufferTextureLayer;
    PFNGLENDTRANSFORMFEEDBACKPROC EndTransformFeedback;
    PFNGLBLENDFUNCSEPARATEIPROC BlendFuncSeparatei;
    PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDPROC DrawTransformFeedbackInstanced;
    PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC DrawRangeElementsBaseVertex;
    PFNGLVERTEXATTRIB1FPROC VertexAttrib1f;
    PFNGLGETUNIFORMSUBROUTINEUIVPROC GetUniformSubroutineuiv;
    PFNGLDISABLEVERTEXATTRIBARRAYPROC DisableVertexAttribArray;
    PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC ProgramUniformMatrix3x2fv;
    PFNGLVERTEXATTRIBI4USVPROC VertexAttribI4usv;
    PFNGLGETOBJECTLABELPROC GetObjectLabel;
    PFNGLBINDATTRIBLOCATIONPROC BindAttribLocation;
    PFNGLUNIFORM1FPROC Uniform1f;
    PFNGLGETUNIFORMDVPROC GetUniformdv;
    PFNGLGETUNIFORMLOCATIONPROC GetUniformLocation;
    PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC GetSubroutineUniformLocation;
    PFNGLGETTEXPARAMETERIUIVPROC GetTexParameterIuiv;
    PFNGLSAMPLERPARAMETERFPROC SamplerParameterf;
    PFNGLVERTEXATTRIBL3DPROC VertexAttribL3d;
    PFNGLTEXIMAGE3DMULTISAMPLEPROC TexImage3DMultisample;
    PFNGLTEXIMAGE3DPROC TexImage3D;
    PFNGLRENDERBUFFERSTORAGEPROC RenderbufferStorage;
    PFNGLENABLEVERTEXATTRIBARRAYPROC EnableVertexAttribArray;
    PFNGLVERTEXATTRIBP4UIVPROC VertexAttribP4uiv;
    PFNGLUNIFORM4DPROC Uniform4d;
    PFNGLVERTEXATTRIB4SPROC VertexAttrib4s;
    PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC DrawElementsInstancedBaseVertex;
    PFNGLVERTEXATTRIB3SPROC VertexAttrib3s;
    PFNGLPROGRAMUNIFORM2IVPROC ProgramUniform2iv;
    PFNGLSTENCILFUNCSEPARATEPROC StencilFuncSeparate;
    PFNGLDELETEFRAMEBUFFERSPROC DeleteFramebuffers;
    PFNGLDEPTHRANGEPROC DepthRange;
    PFNGLUNIFORMMATRIX3X2FVPROC UniformMatrix3x2fv;
    PFNGLPROGRAMUNIFORMMATRIX2DVPROC ProgramUniformMatrix2dv;
    PFNGLSHADERSTORAGEBLOCKBINDINGPROC ShaderStorageBlockBinding;
    PFNGLCLEARDEPTHPROC ClearDepth;
    PFNGLVERTEXATTRIB2DVPROC VertexAttrib2dv;
    PFNGLSAMPLERPARAMETERIUIVPROC SamplerParameterIuiv;
    PFNGLGETVERTEXATTRIBLDVPROC GetVertexAttribLdv;
    PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC ProgramUniformMatrix3x4dv;
    PFNGLDEPTHRANGEARRAYVPROC DepthRangeArrayv;
    PFNGLGETACTIVEUNIFORMPROC GetActiveUniform;
    PFNGLPATCHPARAMETERFVPROC PatchParameterfv;
    PFNGLINVALIDATETEXIMAGEPROC InvalidateTexImage;
    PFNGLVERTEXATTRIB3FPROC VertexAttrib3f;
    PFNGLPROGRAMUNIFORM4IVPROC ProgramUniform4iv;
    PFNGLPROGRAMUNIFORM4DPROC ProgramUniform4d;
    PFNGLISFRAMEBUFFERPROC IsFramebuffer;
    PFNGLPIXELSTOREFPROC PixelStoref;
    PFNGLPROGRAMUNIFORM4UIVPROC ProgramUniform4uiv;
    PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC ProgramUniformMatrix4x2dv;
    PFNGLFENCESYNCPROC FenceSync;
    PFNGLGETBUFFERPARAMETERI64VPROC GetBufferParameteri64v;
    PFNGLSTENCILOPPROC StencilOp;
    PFNGLCLEARBUFFERDATAPROC ClearBufferData;
    PFNGLGETNUNIFORMUIVPROC GetnUniformuiv;
    PFNGLGETPROGRAMRESOURCEIVPROC GetProgramResourceiv;
    PFNGLGETVERTEXATTRIBDVPROC GetVertexAttribdv;
    PFNGLGETTRANSFORMFEEDBACKVARYINGPROC GetTransformFeedbackVarying;
    PFNGLVERTEXATTRIB2FVPROC VertexAttrib2fv;
    PFNGLGETBOOLEANI_VPROC GetBooleani_v;
    PFNGLCOLORMASKIPROC ColorMaski;
    PFNGLINVALIDATEBUFFERSUBDATAPROC InvalidateBufferSubData;
    PFNGLUNIFORMMATRIX4DVPROC UniformMatrix4dv;
    PFNGLISQUERYPROC IsQuery;
    PFNGLUNIFORM4UIPROC Uniform4ui;
    PFNGLUNIFORM4IPROC Uniform4i;
    PFNGLGETSAMPLERPARAMETERIVPROC GetSamplerParameteriv;
    PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC MultiDrawElementsBaseVertex;
    PFNGLVERTEXATTRIBI1UIVPROC VertexAttribI1uiv;
    PFNGLGETINTEGERVPROC GetIntegerv;
    PFNGLUNIFORMMATRIX2X3FVPROC UniformMatrix2x3fv;
    PFNGLTEXIMAGE2DPROC TexImage2D;
    PFNGLGETATTACHEDSHADERSPROC GetAttachedShaders;
    PFNGLUNIFORM2DPROC Uniform2d;
    PFNGLMEMORYBARRIERBYREGIONPROC MemoryBarrierByRegion;
    PFNGLUNIFORMMATRIX2FVPROC UniformMatrix2fv;
    PFNGLPRIMITIVERESTARTINDEXPROC PrimitiveRestartIndex;
    PFNGLGETVERTEXATTRIBIVPROC GetVertexAttribiv;
    PFNGLGETATTRIBLOCATIONPROC GetAttribLocation;
    PFNGLTEXSTORAGE2DMULTISAMPLEPROC TexStorage2DMultisample;
    PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC CompressedTexSubImage2D;
    PFNGLGETVERTEXATTRIBFVPROC GetVertexAttribfv;
    PFNGLGETBUFFERPARAMETERIVPROC GetBufferParameteriv;
    PFNGLTEXPARAMETERFPROC TexParameterf;
    PFNGLFRAMEBUFFERTEXTURE2DPROC FramebufferTexture2D;
    PFNGLGETACTIVEATTRIBPROC GetActiveAttrib;
    PFNGLINVALIDATETEXSUBIMAGEPROC InvalidateTexSubImage;
    PFNGLDELETEVERTEXARRAYSPROC DeleteVertexArrays;
    PFNGLVERTEXATTRIBI2UIPROC VertexAttribI2ui;
    PFNGLPOINTPARAMETERIVPROC PointParameteriv;
    PFNGLGETPOINTERVPROC GetPointerv;
    PFNGLENABLEIPROC Enablei;
    PFNGLBINDBUFFERRANGEPROC BindBufferRange;
    PFNGLDRAWARRAYSINSTANCEDPROC DrawArraysInstanced;
    PFNGLDELETETEXTURESPROC DeleteTextures;
    PFNGLVERTEXATTRIB4NIVPROC VertexAttrib4Niv;
    PFNGLMULTIDRAWELEMENTSPROC MultiDrawElements;
    PFNGLGETPROGRAMIVPROC GetProgramiv;
    PFNGLDEPTHFUNCPROC DepthFunc;
    PFNGLGENTEXTURESPROC GenTextures;
    PFNGLGETINTERNALFORMATIVPROC GetInternalformativ;
    PFNGLPROGRAMUNIFORM3IPROC ProgramUniform3i;
    PFNGLSCISSORINDEXEDPROC ScissorIndexed;
    PFNGLVERTEXATTRIB2SVPROC VertexAttrib2sv;
    PFNGLTEXSTORAGE3DMULTISAMPLEPROC TexStorage3DMultisample;
    PFNGLUNIFORM2IVPROC Uniform2iv;
    PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC DrawArraysInstancedBaseInstance;
    PFNGLVERTEXATTRIBI3UIPROC VertexAttribI3ui;
    PFNGLDELETESAMPLERSPROC DeleteSamplers;
    PFNGLGENVERTEXARRAYSPROC GenVertexArrays;
    PFNGLGETFRAMEBUFFERPARAMETERIVPROC GetFramebufferParameteriv;
    PFNGLPOLYGONMODEPROC PolygonMode;
    PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC ProgramUniformMatrix2x4fv;
    PFNGLGETPROGRAMRESOURCENAMEPROC GetProgramResourceName;
    PFNGLSAMPLERPARAMETERIVPROC SamplerParameteriv;
    PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC GetActiveSubroutineUniformiv;
    PFNGLGETSTRINGIPROC GetStringi;
    PFNGLVERTEXATTRIBLFORMATPROC VertexAttribLFormat;
    PFNGLVERTEXATTRIB3DPROC VertexAttrib3d;
    PFNGLBINDVERTEXARRAYPROC BindVertexArray;
    PFNGLUNMAPBUFFERPROC UnmapBuffer;
    PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC DrawElementsInstancedBaseInstance;
    PFNGLUNIFORM4UIVPROC Uniform4uiv;
    PFNGLFRAMEBUFFERTEXTURE1DPROC FramebufferTexture1D;
    PFNGLDRAWTRANSFORMFEEDBACKSTREAMINSTANCEDPROC DrawTransformFeedbackStreamInstanced;
    PFNGLSTENCILFUNCPROC StencilFunc;
    PFNGLVALIDATEPROGRAMPROC ValidateProgram;
    PFNGLFLUSHPROC Flush;
    PFNGLPROGRAMUNIFORM3UIVPROC ProgramUniform3uiv;
    PFNGLDELETERENDERBUFFERSPROC DeleteRenderbuffers;
    PFNGLVERTEXATTRIB4FVPROC VertexAttrib4fv;
    PFNGLUNIFORMMATRIX2DVPROC UniformMatrix2dv;
    PFNGLGETFRAGDATAINDEXPROC GetFragDataIndex;
    PFNGLUNIFORM3IVPROC Uniform3iv;
    PFNGLMINSAMPLESHADINGPROC MinSampleShading;
    PFNGLGETBOOLEANVPROC GetBooleanv;
    PFNGLGETMULTISAMPLEFVPROC GetMultisamplefv;
    PFNGLGETVERTEXATTRIBIUIVPROC GetVertexAttribIuiv;
    PFNGLGETPROGRAMINFOLOGPROC GetProgramInfoLog;
    PFNGLUNIFORM4FVPROC Uniform4fv;
    PFNGLDRAWBUFFERPROC DrawBuffer;
    PFNGLUNIFORM1IPROC Uniform1i;
    PFNGLPROGRAMUNIFORM4UIPROC ProgramUniform4ui;
    PFNGLPROGRAMUNIFORMMATRIX3FVPROC ProgramUniformMatrix3fv;
    PFNGLBLENDEQUATIONSEPARATEPROC BlendEquationSeparate;
    PFNGLBINDPROGRAMPIPELINEPROC BindProgramPipeline;
    PFNGLGETDOUBLEI_VPROC GetDoublei_v;
    PFNGLBUFFERDATAPROC BufferData;
    PFNGLCLEARCOLORPROC ClearColor;
    PFNGLPROGRAMUNIFORM4IPROC ProgramUniform4i;
    PFNGLGETTEXLEVELPARAMETERIVPROC GetTexLevelParameteriv;
    PFNGLGETACTIVEUNIFORMBLOCKIVPROC GetActiveUniformBlockiv;
    PFNGLPROGRAMUNIFORM1FVPROC ProgramUniform1fv;
    PFNGLPAUSETRANSFORMFEEDBACKPROC PauseTransformFeedback;
    PFNGLGETBUFFERPOINTERVPROC GetBufferPointerv;
    PFNGLINVALIDATESUBFRAMEBUFFERPROC InvalidateSubFramebuffer;
    PFNGLSCISSORINDEXEDVPROC ScissorIndexedv;
    PFNGLUNIFORM2UIPROC Uniform2ui;
    PFNGLBINDTEXTUREPROC BindTexture;
    PFNGLDRAWELEMENTSINSTANCEDPROC DrawElementsInstanced;
    PFNGLPROGRAMUNIFORM4FPROC ProgramUniform4f;
    PFNGLBINDBUFFERBASEPROC BindBufferBase;
    PFNGLISSHADERPROC IsShader;
    PFNGLCLEARBUFFERSUBDATAPROC ClearBufferSubData;
    PFNGLVERTEXATTRIB4NUIVPROC VertexAttrib4Nuiv;
    PFNGLDRAWARRAYSINDIRECTPROC DrawArraysIndirect;
    PFNGLVERTEXATTRIB4USVPROC VertexAttrib4usv;
    PFNGLUNIFORM1DPROC Uniform1d;
    PFNGLCLEARTEXIMAGEPROC ClearTexImage;
    PFNGLUNIFORM1UIVPROC Uniform1uiv;
    PFNGLBINDSAMPLERPROC BindSampler;
    PFNGLGETTEXLEVELPARAMETERFVPROC GetTexLevelParameterfv;
    PFNGLCLEARBUFFERIVPROC ClearBufferiv;
    PFNGLLOGICOPPROC LogicOp;
    PFNGLACTIVETEXTUREPROC ActiveTexture;
    PFNGLGETFRAGDATALOCATIONPROC GetFragDataLocation;
    PFNGLBLENDCOLORPROC BlendColor;
    PFNGLUNIFORMMATRIX4X3FVPROC UniformMatrix4x3fv;
    PFNGLPROGRAMUNIFORM3FVPROC ProgramUniform3fv;
    PFNGLUNIFORM1FVPROC Uniform1fv;
    PFNGLDRAWELEMENTSBASEVERTEXPROC DrawElementsBaseVertex;
    PFNGLUNIFORM4FPROC Uniform4f;
    PFNGLBLENDEQUATIONSEPARATEIPROC BlendEquationSeparatei;
    PFNGLBLENDFUNCSEPARATEPROC BlendFuncSeparate;
    PFNGLCLEARBUFFERUIVPROC ClearBufferuiv;
    PFNGLCOPYTEXSUBIMAGE1DPROC CopyTexSubImage1D;
    PFNGLDRAWTRANSFORMFEEDBACKPROC DrawTransformFeedback;
    PFNGLREADBUFFERPROC ReadBuffer;
    PFNGLCOPYBUFFERSUBDATAPROC CopyBufferSubData;
    PFNGLGETUNIFORMUIVPROC GetUniformuiv;
    PFNGLPOLYGONOFFSETPROC PolygonOffset;
    PFNGLDISPATCHCOMPUTEPROC DispatchCompute;
    PFNGLBINDIMAGETEXTUREPROC BindImageTexture;
    PFNGLUNIFORMMATRIX4X3DVPROC UniformMatrix4x3dv;
    PFNGLGENRENDERBUFFERSPROC GenRenderbuffers;
} oc_gl_api;

ORCA_API oc_gl_api* oc_gl_get_api(void);

#define glGetFloatv oc_gl_get_api()->GetFloatv
#define glTexBufferRange oc_gl_get_api()->TexBufferRange
#define glIsBuffer oc_gl_get_api()->IsBuffer
#define glIsTexture oc_gl_get_api()->IsTexture
#define glDepthRangef oc_gl_get_api()->DepthRangef
#define glEndConditionalRender oc_gl_get_api()->EndConditionalRender
#define glBlendFunci oc_gl_get_api()->BlendFunci
#define glGetProgramPipelineiv oc_gl_get_api()->GetProgramPipelineiv
#define glWaitSync oc_gl_get_api()->WaitSync
#define glProgramUniformMatrix2fv oc_gl_get_api()->ProgramUniformMatrix2fv
#define glProgramUniformMatrix4x3dv oc_gl_get_api()->ProgramUniformMatrix4x3dv
#define glVertexAttrib1dv oc_gl_get_api()->VertexAttrib1dv
#define glSamplerParameteri oc_gl_get_api()->SamplerParameteri
#define glGetVertexAttribIiv oc_gl_get_api()->GetVertexAttribIiv
#define glGetSamplerParameterfv oc_gl_get_api()->GetSamplerParameterfv
#define glVertexAttrib1d oc_gl_get_api()->VertexAttrib1d
#define glTexBuffer oc_gl_get_api()->TexBuffer
#define glInvalidateBufferData oc_gl_get_api()->InvalidateBufferData
#define glProgramUniform2i oc_gl_get_api()->ProgramUniform2i
#define glUniform4dv oc_gl_get_api()->Uniform4dv
#define glUseProgram oc_gl_get_api()->UseProgram
#define glVertexAttribI3iv oc_gl_get_api()->VertexAttribI3iv
#define glDrawElementsIndirect oc_gl_get_api()->DrawElementsIndirect
#define glVertexAttrib4uiv oc_gl_get_api()->VertexAttrib4uiv
#define glGetQueryObjectiv oc_gl_get_api()->GetQueryObjectiv
#define glFramebufferRenderbuffer oc_gl_get_api()->FramebufferRenderbuffer
#define glBlendEquationi oc_gl_get_api()->BlendEquationi
#define glGetActiveSubroutineName oc_gl_get_api()->GetActiveSubroutineName
#define glVertexAttrib2s oc_gl_get_api()->VertexAttrib2s
#define glVertexAttribL1d oc_gl_get_api()->VertexAttribL1d
#define glBindTextures oc_gl_get_api()->BindTextures
#define glVertexAttrib3sv oc_gl_get_api()->VertexAttrib3sv
#define glGetFloati_v oc_gl_get_api()->GetFloati_v
#define glBeginTransformFeedback oc_gl_get_api()->BeginTransformFeedback
#define glClearStencil oc_gl_get_api()->ClearStencil
#define glUniform3i oc_gl_get_api()->Uniform3i
#define glValidateProgramPipeline oc_gl_get_api()->ValidateProgramPipeline
#define glProgramUniformMatrix4x2fv oc_gl_get_api()->ProgramUniformMatrix4x2fv
#define glVertexAttribI4ui oc_gl_get_api()->VertexAttribI4ui
#define glGetShaderiv oc_gl_get_api()->GetShaderiv
#define glReadnPixels oc_gl_get_api()->ReadnPixels
#define glUniformMatrix4x2fv oc_gl_get_api()->UniformMatrix4x2fv
#define glGetShaderPrecisionFormat oc_gl_get_api()->GetShaderPrecisionFormat
#define glProgramUniformMatrix2x3fv oc_gl_get_api()->ProgramUniformMatrix2x3fv
#define glTexSubImage3D oc_gl_get_api()->TexSubImage3D
#define glGetProgramResourceLocationIndex oc_gl_get_api()->GetProgramResourceLocationIndex
#define glBlendFunc oc_gl_get_api()->BlendFunc
#define glProgramUniformMatrix3x4fv oc_gl_get_api()->ProgramUniformMatrix3x4fv
#define glUniform3d oc_gl_get_api()->Uniform3d
#define glVertexAttrib1sv oc_gl_get_api()->VertexAttrib1sv
#define glBindFragDataLocation oc_gl_get_api()->BindFragDataLocation
#define glVertexAttrib4bv oc_gl_get_api()->VertexAttrib4bv
#define glUniform4iv oc_gl_get_api()->Uniform4iv
#define glProgramUniform2ui oc_gl_get_api()->ProgramUniform2ui
#define glDrawArrays oc_gl_get_api()->DrawArrays
#define glProgramBinary oc_gl_get_api()->ProgramBinary
#define glVertexAttrib4f oc_gl_get_api()->VertexAttrib4f
#define glVertexAttribP2uiv oc_gl_get_api()->VertexAttribP2uiv
#define glUniformMatrix3fv oc_gl_get_api()->UniformMatrix3fv
#define glUniform2i oc_gl_get_api()->Uniform2i
#define glGetQueryObjectuiv oc_gl_get_api()->GetQueryObjectuiv
#define glUniformBlockBinding oc_gl_get_api()->UniformBlockBinding
#define glSampleCoverage oc_gl_get_api()->SampleCoverage
#define glVertexAttrib4Nusv oc_gl_get_api()->VertexAttrib4Nusv
#define glProgramUniformMatrix2x4dv oc_gl_get_api()->ProgramUniformMatrix2x4dv
#define glUniform3uiv oc_gl_get_api()->Uniform3uiv
#define glVertexAttrib1s oc_gl_get_api()->VertexAttrib1s
#define glGetVertexAttribPointerv oc_gl_get_api()->GetVertexAttribPointerv
#define glBlendBarrier oc_gl_get_api()->BlendBarrier
#define glDrawRangeElements oc_gl_get_api()->DrawRangeElements
#define glTexStorage3D oc_gl_get_api()->TexStorage3D
#define glGetInternalformati64v oc_gl_get_api()->GetInternalformati64v
#define glGetQueryObjecti64v oc_gl_get_api()->GetQueryObjecti64v
#define glCompressedTexSubImage1D oc_gl_get_api()->CompressedTexSubImage1D
#define glVertexAttrib3dv oc_gl_get_api()->VertexAttrib3dv
#define glVertexBindingDivisor oc_gl_get_api()->VertexBindingDivisor
#define glUseProgramStages oc_gl_get_api()->UseProgramStages
#define glVertexAttribBinding oc_gl_get_api()->VertexAttribBinding
#define glDebugMessageInsert oc_gl_get_api()->DebugMessageInsert
#define glGetTexParameteriv oc_gl_get_api()->GetTexParameteriv
#define glMultiDrawArraysIndirect oc_gl_get_api()->MultiDrawArraysIndirect
#define glGetTexParameterfv oc_gl_get_api()->GetTexParameterfv
#define glGetProgramPipelineInfoLog oc_gl_get_api()->GetProgramPipelineInfoLog
#define glEndQuery oc_gl_get_api()->EndQuery
#define glGetProgramResourceLocation oc_gl_get_api()->GetProgramResourceLocation
#define glCompressedTexImage2D oc_gl_get_api()->CompressedTexImage2D
#define glVertexAttribP2ui oc_gl_get_api()->VertexAttribP2ui
#define glIsEnabledi oc_gl_get_api()->IsEnabledi
#define glGetActiveAtomicCounterBufferiv oc_gl_get_api()->GetActiveAtomicCounterBufferiv
#define glIsProgram oc_gl_get_api()->IsProgram
#define glUniform1dv oc_gl_get_api()->Uniform1dv
#define glTexParameteriv oc_gl_get_api()->TexParameteriv
#define glUniform2fv oc_gl_get_api()->Uniform2fv
#define glReleaseShaderCompiler oc_gl_get_api()->ReleaseShaderCompiler
#define glCullFace oc_gl_get_api()->CullFace
#define glVertexAttribI4i oc_gl_get_api()->VertexAttribI4i
#define glGetProgramResourceIndex oc_gl_get_api()->GetProgramResourceIndex
#define glShaderBinary oc_gl_get_api()->ShaderBinary
#define glUniformMatrix3x2dv oc_gl_get_api()->UniformMatrix3x2dv
#define glInvalidateFramebuffer oc_gl_get_api()->InvalidateFramebuffer
#define glAttachShader oc_gl_get_api()->AttachShader
#define glFlushMappedBufferRange oc_gl_get_api()->FlushMappedBufferRange
#define glVertexAttribP3uiv oc_gl_get_api()->VertexAttribP3uiv
#define glGetActiveUniformName oc_gl_get_api()->GetActiveUniformName
#define glMapBuffer oc_gl_get_api()->MapBuffer
#define glDrawBuffers oc_gl_get_api()->DrawBuffers
#define glGetSynciv oc_gl_get_api()->GetSynciv
#define glCopyTexSubImage2D oc_gl_get_api()->CopyTexSubImage2D
#define glObjectLabel oc_gl_get_api()->ObjectLabel
#define glBufferSubData oc_gl_get_api()->BufferSubData
#define glUniform2f oc_gl_get_api()->Uniform2f
#define glDebugMessageCallback oc_gl_get_api()->DebugMessageCallback
#define glVertexAttribL4dv oc_gl_get_api()->VertexAttribL4dv
#define glIsProgramPipeline oc_gl_get_api()->IsProgramPipeline
#define glResumeTransformFeedback oc_gl_get_api()->ResumeTransformFeedback
#define glVertexAttribI4iv oc_gl_get_api()->VertexAttribI4iv
#define glGetShaderInfoLog oc_gl_get_api()->GetShaderInfoLog
#define glGetIntegeri_v oc_gl_get_api()->GetIntegeri_v
#define glBindVertexBuffer oc_gl_get_api()->BindVertexBuffer
#define glBlendEquation oc_gl_get_api()->BlendEquation
#define glVertexAttribL2dv oc_gl_get_api()->VertexAttribL2dv
#define glVertexAttribI1ui oc_gl_get_api()->VertexAttribI1ui
#define glVertexAttrib4Nsv oc_gl_get_api()->VertexAttrib4Nsv
#define glVertexAttribL4d oc_gl_get_api()->VertexAttribL4d
#define glCopyImageSubData oc_gl_get_api()->CopyImageSubData
#define glGetFramebufferAttachmentParameteriv oc_gl_get_api()->GetFramebufferAttachmentParameteriv
#define glVertexAttribL2d oc_gl_get_api()->VertexAttribL2d
#define glGetSubroutineIndex oc_gl_get_api()->GetSubroutineIndex
#define glVertexAttribI3uiv oc_gl_get_api()->VertexAttribI3uiv
#define glVertexAttrib4iv oc_gl_get_api()->VertexAttrib4iv
#define glBindVertexBuffers oc_gl_get_api()->BindVertexBuffers
#define glProgramUniformMatrix2x3dv oc_gl_get_api()->ProgramUniformMatrix2x3dv
#define glPrimitiveBoundingBox oc_gl_get_api()->PrimitiveBoundingBox
#define glScissor oc_gl_get_api()->Scissor
#define glClientWaitSync oc_gl_get_api()->ClientWaitSync
#define glUniform3ui oc_gl_get_api()->Uniform3ui
#define glVertexAttribP3ui oc_gl_get_api()->VertexAttribP3ui
#define glEnable oc_gl_get_api()->Enable
#define glStencilOpSeparate oc_gl_get_api()->StencilOpSeparate
#define glUniformMatrix2x3dv oc_gl_get_api()->UniformMatrix2x3dv
#define glProgramUniformMatrix3dv oc_gl_get_api()->ProgramUniformMatrix3dv
#define glTexImage2DMultisample oc_gl_get_api()->TexImage2DMultisample
#define glVertexAttrib4Nbv oc_gl_get_api()->VertexAttrib4Nbv
#define glGetTexImage oc_gl_get_api()->GetTexImage
#define glVertexAttrib4sv oc_gl_get_api()->VertexAttrib4sv
#define glPixelStorei oc_gl_get_api()->PixelStorei
#define glDepthMask oc_gl_get_api()->DepthMask
#define glTexStorage2D oc_gl_get_api()->TexStorage2D
#define glClear oc_gl_get_api()->Clear
#define glUniformMatrix3x4dv oc_gl_get_api()->UniformMatrix3x4dv
#define glDeleteTransformFeedbacks oc_gl_get_api()->DeleteTransformFeedbacks
#define glMapBufferRange oc_gl_get_api()->MapBufferRange
#define glMemoryBarrier oc_gl_get_api()->MemoryBarrier
#define glViewportIndexedf oc_gl_get_api()->ViewportIndexedf
#define glVertexAttrib3fv oc_gl_get_api()->VertexAttrib3fv
#define glObjectPtrLabel oc_gl_get_api()->ObjectPtrLabel
#define glTexStorage1D oc_gl_get_api()->TexStorage1D
#define glCompressedTexImage3D oc_gl_get_api()->CompressedTexImage3D
#define glVertexAttrib1fv oc_gl_get_api()->VertexAttrib1fv
#define glVertexAttribPointer oc_gl_get_api()->VertexAttribPointer
#define glGetQueryIndexediv oc_gl_get_api()->GetQueryIndexediv
#define glCompileShader oc_gl_get_api()->CompileShader
#define glProgramUniform1i oc_gl_get_api()->ProgramUniform1i
#define glGetQueryiv oc_gl_get_api()->GetQueryiv
#define glVertexAttribI1iv oc_gl_get_api()->VertexAttribI1iv
#define glCopyTexImage2D oc_gl_get_api()->CopyTexImage2D
#define glGetQueryObjectui64v oc_gl_get_api()->GetQueryObjectui64v
#define glPointSize oc_gl_get_api()->PointSize
#define glDisablei oc_gl_get_api()->Disablei
#define glVertexAttribL1dv oc_gl_get_api()->VertexAttribL1dv
#define glCreateShader oc_gl_get_api()->CreateShader
#define glGetString oc_gl_get_api()->GetString
#define glViewportArrayv oc_gl_get_api()->ViewportArrayv
#define glProgramUniform3d oc_gl_get_api()->ProgramUniform3d
#define glVertexAttrib4Nubv oc_gl_get_api()->VertexAttrib4Nubv
#define glTexParameteri oc_gl_get_api()->TexParameteri
#define glProgramUniform4fv oc_gl_get_api()->ProgramUniform4fv
#define glGenerateMipmap oc_gl_get_api()->GenerateMipmap
#define glCompressedTexSubImage3D oc_gl_get_api()->CompressedTexSubImage3D
#define glUniform3f oc_gl_get_api()->Uniform3f
#define glGetUniformIndices oc_gl_get_api()->GetUniformIndices
#define glVertexAttribLPointer oc_gl_get_api()->VertexAttribLPointer
#define glVertexAttribI2uiv oc_gl_get_api()->VertexAttribI2uiv
#define glQueryCounter oc_gl_get_api()->QueryCounter
#define glActiveShaderProgram oc_gl_get_api()->ActiveShaderProgram
#define glUniform1ui oc_gl_get_api()->Uniform1ui
#define glVertexAttribI1i oc_gl_get_api()->VertexAttribI1i
#define glGetTexParameterIiv oc_gl_get_api()->GetTexParameterIiv
#define glGetUniformfv oc_gl_get_api()->GetUniformfv
#define glProgramUniform2uiv oc_gl_get_api()->ProgramUniform2uiv
#define glGetError oc_gl_get_api()->GetError
#define glGetActiveUniformBlockName oc_gl_get_api()->GetActiveUniformBlockName
#define glTextureView oc_gl_get_api()->TextureView
#define glGetnUniformiv oc_gl_get_api()->GetnUniformiv
#define glProgramUniform4dv oc_gl_get_api()->ProgramUniform4dv
#define glViewportIndexedfv oc_gl_get_api()->ViewportIndexedfv
#define glHint oc_gl_get_api()->Hint
#define glGetShaderSource oc_gl_get_api()->GetShaderSource
#define glProgramUniformMatrix4x3fv oc_gl_get_api()->ProgramUniformMatrix4x3fv
#define glUniform1iv oc_gl_get_api()->Uniform1iv
#define glVertexAttribI4bv oc_gl_get_api()->VertexAttribI4bv
#define glUniformMatrix4x2dv oc_gl_get_api()->UniformMatrix4x2dv
#define glBufferStorage oc_gl_get_api()->BufferStorage
#define glIsRenderbuffer oc_gl_get_api()->IsRenderbuffer
#define glGetActiveSubroutineUniformName oc_gl_get_api()->GetActiveSubroutineUniformName
#define glLinkProgram oc_gl_get_api()->LinkProgram
#define glGetActiveUniformsiv oc_gl_get_api()->GetActiveUniformsiv
#define glGetDebugMessageLog oc_gl_get_api()->GetDebugMessageLog
#define glCopyTexSubImage3D oc_gl_get_api()->CopyTexSubImage3D
#define glPointParameteri oc_gl_get_api()->PointParameteri
#define glProgramUniform3dv oc_gl_get_api()->ProgramUniform3dv
#define glCompressedTexImage1D oc_gl_get_api()->CompressedTexImage1D
#define glUniformMatrix3x4fv oc_gl_get_api()->UniformMatrix3x4fv
#define glGenSamplers oc_gl_get_api()->GenSamplers
#define glGetCompressedTexImage oc_gl_get_api()->GetCompressedTexImage
#define glDeleteQueries oc_gl_get_api()->DeleteQueries
#define glGenProgramPipelines oc_gl_get_api()->GenProgramPipelines
#define glDispatchComputeIndirect oc_gl_get_api()->DispatchComputeIndirect
#define glVertexAttribIPointer oc_gl_get_api()->VertexAttribIPointer
#define glCreateProgram oc_gl_get_api()->CreateProgram
#define glClearTexSubImage oc_gl_get_api()->ClearTexSubImage
#define glVertexAttrib4d oc_gl_get_api()->VertexAttrib4d
#define glFrontFace oc_gl_get_api()->FrontFace
#define glBindTransformFeedback oc_gl_get_api()->BindTransformFeedback
#define glGetProgramStageiv oc_gl_get_api()->GetProgramStageiv
#define glSamplerParameterIiv oc_gl_get_api()->SamplerParameterIiv
#define glGetInteger64v oc_gl_get_api()->GetInteger64v
#define glCreateShaderProgramv oc_gl_get_api()->CreateShaderProgramv
#define glBindBuffersRange oc_gl_get_api()->BindBuffersRange
#define glUniform3fv oc_gl_get_api()->Uniform3fv
#define glProgramUniformMatrix4fv oc_gl_get_api()->ProgramUniformMatrix4fv
#define glBindBuffersBase oc_gl_get_api()->BindBuffersBase
#define glClearBufferfi oc_gl_get_api()->ClearBufferfi
#define glFramebufferTexture3D oc_gl_get_api()->FramebufferTexture3D
#define glDisable oc_gl_get_api()->Disable
#define glProgramUniform1iv oc_gl_get_api()->ProgramUniform1iv
#define glVertexAttribI2iv oc_gl_get_api()->VertexAttribI2iv
#define glDepthRangeIndexed oc_gl_get_api()->DepthRangeIndexed
#define glPatchParameteri oc_gl_get_api()->PatchParameteri
#define glGetUniformBlockIndex oc_gl_get_api()->GetUniformBlockIndex
#define glMultiDrawArrays oc_gl_get_api()->MultiDrawArrays
#define glVertexAttribI4ubv oc_gl_get_api()->VertexAttribI4ubv
#define glBindBuffer oc_gl_get_api()->BindBuffer
#define glVertexAttribI3i oc_gl_get_api()->VertexAttribI3i
#define glGetDoublev oc_gl_get_api()->GetDoublev
#define glDrawTransformFeedbackStream oc_gl_get_api()->DrawTransformFeedbackStream
#define glVertexAttribI4uiv oc_gl_get_api()->VertexAttribI4uiv
#define glRenderbufferStorageMultisample oc_gl_get_api()->RenderbufferStorageMultisample
#define glVertexAttribL3dv oc_gl_get_api()->VertexAttribL3dv
#define glStencilMaskSeparate oc_gl_get_api()->StencilMaskSeparate
#define glProgramUniform1d oc_gl_get_api()->ProgramUniform1d
#define glViewport oc_gl_get_api()->Viewport
#define glVertexAttribP1ui oc_gl_get_api()->VertexAttribP1ui
#define glVertexAttrib4dv oc_gl_get_api()->VertexAttrib4dv
#define glGenQueries oc_gl_get_api()->GenQueries
#define glTexParameterIiv oc_gl_get_api()->TexParameterIiv
#define glProgramUniform2d oc_gl_get_api()->ProgramUniform2d
#define glProgramUniform1uiv oc_gl_get_api()->ProgramUniform1uiv
#define glVertexAttrib4Nub oc_gl_get_api()->VertexAttrib4Nub
#define glIsVertexArray oc_gl_get_api()->IsVertexArray
#define glProgramUniform3f oc_gl_get_api()->ProgramUniform3f
#define glProgramUniform3iv oc_gl_get_api()->ProgramUniform3iv
#define glGetProgramBinary oc_gl_get_api()->GetProgramBinary
#define glBindRenderbuffer oc_gl_get_api()->BindRenderbuffer
#define glBindFragDataLocationIndexed oc_gl_get_api()->BindFragDataLocationIndexed
#define glGetSamplerParameterIiv oc_gl_get_api()->GetSamplerParameterIiv
#define glVertexAttribDivisor oc_gl_get_api()->VertexAttribDivisor
#define glProgramUniformMatrix3x2dv oc_gl_get_api()->ProgramUniformMatrix3x2dv
#define glFramebufferParameteri oc_gl_get_api()->FramebufferParameteri
#define glGenTransformFeedbacks oc_gl_get_api()->GenTransformFeedbacks
#define glDeleteSync oc_gl_get_api()->DeleteSync
#define glProgramUniform1ui oc_gl_get_api()->ProgramUniform1ui
#define glTexSubImage1D oc_gl_get_api()->TexSubImage1D
#define glClearDepthf oc_gl_get_api()->ClearDepthf
#define glReadPixels oc_gl_get_api()->ReadPixels
#define glVertexAttribI2i oc_gl_get_api()->VertexAttribI2i
#define glFinish oc_gl_get_api()->Finish
#define glLineWidth oc_gl_get_api()->LineWidth
#define glDeleteShader oc_gl_get_api()->DeleteShader
#define glIsSampler oc_gl_get_api()->IsSampler
#define glProgramUniformMatrix4dv oc_gl_get_api()->ProgramUniformMatrix4dv
#define glTransformFeedbackVaryings oc_gl_get_api()->TransformFeedbackVaryings
#define glBeginConditionalRender oc_gl_get_api()->BeginConditionalRender
#define glBindSamplers oc_gl_get_api()->BindSamplers
#define glDeleteProgramPipelines oc_gl_get_api()->DeleteProgramPipelines
#define glColorMask oc_gl_get_api()->ColorMask
#define glTexParameterfv oc_gl_get_api()->TexParameterfv
#define glPushDebugGroup oc_gl_get_api()->PushDebugGroup
#define glClearBufferfv oc_gl_get_api()->ClearBufferfv
#define glIsEnabled oc_gl_get_api()->IsEnabled
#define glVertexAttrib2f oc_gl_get_api()->VertexAttrib2f
#define glProgramUniform2f oc_gl_get_api()->ProgramUniform2f
#define glGetSamplerParameterIuiv oc_gl_get_api()->GetSamplerParameterIuiv
#define glGetInteger64i_v oc_gl_get_api()->GetInteger64i_v
#define glUniform2dv oc_gl_get_api()->Uniform2dv
#define glGetBufferSubData oc_gl_get_api()->GetBufferSubData
#define glMultiDrawElementsIndirect oc_gl_get_api()->MultiDrawElementsIndirect
#define glProgramParameteri oc_gl_get_api()->ProgramParameteri
#define glVertexAttribP4ui oc_gl_get_api()->VertexAttribP4ui
#define glSamplerParameterfv oc_gl_get_api()->SamplerParameterfv
#define glPointParameterf oc_gl_get_api()->PointParameterf
#define glUniformMatrix2x4fv oc_gl_get_api()->UniformMatrix2x4fv
#define glGenBuffers oc_gl_get_api()->GenBuffers
#define glProgramUniform2dv oc_gl_get_api()->ProgramUniform2dv
#define glVertexAttribFormat oc_gl_get_api()->VertexAttribFormat
#define glTexSubImage2D oc_gl_get_api()->TexSubImage2D
#define glVertexAttrib4ubv oc_gl_get_api()->VertexAttrib4ubv
#define glGetGraphicsResetStatus oc_gl_get_api()->GetGraphicsResetStatus
#define glGetProgramInterfaceiv oc_gl_get_api()->GetProgramInterfaceiv
#define glVertexAttribIFormat oc_gl_get_api()->VertexAttribIFormat
#define glGetnUniformfv oc_gl_get_api()->GetnUniformfv
#define glDeleteProgram oc_gl_get_api()->DeleteProgram
#define glClampColor oc_gl_get_api()->ClampColor
#define glDrawElementsInstancedBaseVertexBaseInstance oc_gl_get_api()->DrawElementsInstancedBaseVertexBaseInstance
#define glDrawElements oc_gl_get_api()->DrawElements
#define glDebugMessageControl oc_gl_get_api()->DebugMessageControl
#define glGetRenderbufferParameteriv oc_gl_get_api()->GetRenderbufferParameteriv
#define glDetachShader oc_gl_get_api()->DetachShader
#define glGenFramebuffers oc_gl_get_api()->GenFramebuffers
#define glProvokingVertex oc_gl_get_api()->ProvokingVertex
#define glSampleMaski oc_gl_get_api()->SampleMaski
#define glEndQueryIndexed oc_gl_get_api()->EndQueryIndexed
#define glProgramUniform1f oc_gl_get_api()->ProgramUniform1f
#define glBindFramebuffer oc_gl_get_api()->BindFramebuffer
#define glBeginQueryIndexed oc_gl_get_api()->BeginQueryIndexed
#define glUniformSubroutinesuiv oc_gl_get_api()->UniformSubroutinesuiv
#define glGetUniformiv oc_gl_get_api()->GetUniformiv
#define glFramebufferTexture oc_gl_get_api()->FramebufferTexture
#define glPointParameterfv oc_gl_get_api()->PointParameterfv
#define glIsTransformFeedback oc_gl_get_api()->IsTransformFeedback
#define glCheckFramebufferStatus oc_gl_get_api()->CheckFramebufferStatus
#define glShaderSource oc_gl_get_api()->ShaderSource
#define glUniformMatrix2x4dv oc_gl_get_api()->UniformMatrix2x4dv
#define glBindImageTextures oc_gl_get_api()->BindImageTextures
#define glCopyTexImage1D oc_gl_get_api()->CopyTexImage1D
#define glUniformMatrix3dv oc_gl_get_api()->UniformMatrix3dv
#define glProgramUniform1dv oc_gl_get_api()->ProgramUniform1dv
#define glBlitFramebuffer oc_gl_get_api()->BlitFramebuffer
#define glPopDebugGroup oc_gl_get_api()->PopDebugGroup
#define glTexParameterIuiv oc_gl_get_api()->TexParameterIuiv
#define glVertexAttrib2d oc_gl_get_api()->VertexAttrib2d
#define glTexImage1D oc_gl_get_api()->TexImage1D
#define glGetObjectPtrLabel oc_gl_get_api()->GetObjectPtrLabel
#define glStencilMask oc_gl_get_api()->StencilMask
#define glBeginQuery oc_gl_get_api()->BeginQuery
#define glUniformMatrix4fv oc_gl_get_api()->UniformMatrix4fv
#define glIsSync oc_gl_get_api()->IsSync
#define glUniform3dv oc_gl_get_api()->Uniform3dv
#define glProgramUniform2fv oc_gl_get_api()->ProgramUniform2fv
#define glVertexAttribI4sv oc_gl_get_api()->VertexAttribI4sv
#define glScissorArrayv oc_gl_get_api()->ScissorArrayv
#define glVertexAttribP1uiv oc_gl_get_api()->VertexAttribP1uiv
#define glUniform2uiv oc_gl_get_api()->Uniform2uiv
#define glDeleteBuffers oc_gl_get_api()->DeleteBuffers
#define glProgramUniform3ui oc_gl_get_api()->ProgramUniform3ui
#define glFramebufferTextureLayer oc_gl_get_api()->FramebufferTextureLayer
#define glEndTransformFeedback oc_gl_get_api()->EndTransformFeedback
#define glBlendFuncSeparatei oc_gl_get_api()->BlendFuncSeparatei
#define glDrawTransformFeedbackInstanced oc_gl_get_api()->DrawTransformFeedbackInstanced
#define glDrawRangeElementsBaseVertex oc_gl_get_api()->DrawRangeElementsBaseVertex
#define glVertexAttrib1f oc_gl_get_api()->VertexAttrib1f
#define glGetUniformSubroutineuiv oc_gl_get_api()->GetUniformSubroutineuiv
#define glDisableVertexAttribArray oc_gl_get_api()->DisableVertexAttribArray
#define glProgramUniformMatrix3x2fv oc_gl_get_api()->ProgramUniformMatrix3x2fv
#define glVertexAttribI4usv oc_gl_get_api()->VertexAttribI4usv
#define glGetObjectLabel oc_gl_get_api()->GetObjectLabel
#define glBindAttribLocation oc_gl_get_api()->BindAttribLocation
#define glUniform1f oc_gl_get_api()->Uniform1f
#define glGetUniformdv oc_gl_get_api()->GetUniformdv
#define glGetUniformLocation oc_gl_get_api()->GetUniformLocation
#define glGetSubroutineUniformLocation oc_gl_get_api()->GetSubroutineUniformLocation
#define glGetTexParameterIuiv oc_gl_get_api()->GetTexParameterIuiv
#define glSamplerParameterf oc_gl_get_api()->SamplerParameterf
#define glVertexAttribL3d oc_gl_get_api()->VertexAttribL3d
#define glTexImage3DMultisample oc_gl_get_api()->TexImage3DMultisample
#define glTexImage3D oc_gl_get_api()->TexImage3D
#define glRenderbufferStorage oc_gl_get_api()->RenderbufferStorage
#define glEnableVertexAttribArray oc_gl_get_api()->EnableVertexAttribArray
#define glVertexAttribP4uiv oc_gl_get_api()->VertexAttribP4uiv
#define glUniform4d oc_gl_get_api()->Uniform4d
#define glVertexAttrib4s oc_gl_get_api()->VertexAttrib4s
#define glDrawElementsInstancedBaseVertex oc_gl_get_api()->DrawElementsInstancedBaseVertex
#define glVertexAttrib3s oc_gl_get_api()->VertexAttrib3s
#define glProgramUniform2iv oc_gl_get_api()->ProgramUniform2iv
#define glStencilFuncSeparate oc_gl_get_api()->StencilFuncSeparate
#define glDeleteFramebuffers oc_gl_get_api()->DeleteFramebuffers
#define glDepthRange oc_gl_get_api()->DepthRange
#define glUniformMatrix3x2fv oc_gl_get_api()->UniformMatrix3x2fv
#define glProgramUniformMatrix2dv oc_gl_get_api()->ProgramUniformMatrix2dv
#define glShaderStorageBlockBinding oc_gl_get_api()->ShaderStorageBlockBinding
#define glClearDepth oc_gl_get_api()->ClearDepth
#define glVertexAttrib2dv oc_gl_get_api()->VertexAttrib2dv
#define glSamplerParameterIuiv oc_gl_get_api()->SamplerParameterIuiv
#define glGetVertexAttribLdv oc_gl_get_api()->GetVertexAttribLdv
#define glProgramUniformMatrix3x4dv oc_gl_get_api()->ProgramUniformMatrix3x4dv
#define glDepthRangeArrayv oc_gl_get_api()->DepthRangeArrayv
#define glGetActiveUniform oc_gl_get_api()->GetActiveUniform
#define glPatchParameterfv oc_gl_get_api()->PatchParameterfv
#define glInvalidateTexImage oc_gl_get_api()->InvalidateTexImage
#define glVertexAttrib3f oc_gl_get_api()->VertexAttrib3f
#define glProgramUniform4iv oc_gl_get_api()->ProgramUniform4iv
#define glProgramUniform4d oc_gl_get_api()->ProgramUniform4d
#define glIsFramebuffer oc_gl_get_api()->IsFramebuffer
#define glPixelStoref oc_gl_get_api()->PixelStoref
#define glProgramUniform4uiv oc_gl_get_api()->ProgramUniform4uiv
#define glProgramUniformMatrix4x2dv oc_gl_get_api()->ProgramUniformMatrix4x2dv
#define glFenceSync oc_gl_get_api()->FenceSync
#define glGetBufferParameteri64v oc_gl_get_api()->GetBufferParameteri64v
#define glStencilOp oc_gl_get_api()->StencilOp
#define glClearBufferData oc_gl_get_api()->ClearBufferData
#define glGetnUniformuiv oc_gl_get_api()->GetnUniformuiv
#define glGetProgramResourceiv oc_gl_get_api()->GetProgramResourceiv
#define glGetVertexAttribdv oc_gl_get_api()->GetVertexAttribdv
#define glGetTransformFeedbackVarying oc_gl_get_api()->GetTransformFeedbackVarying
#define glVertexAttrib2fv oc_gl_get_api()->VertexAttrib2fv
#define glGetBooleani_v oc_gl_get_api()->GetBooleani_v
#define glColorMaski oc_gl_get_api()->ColorMaski
#define glInvalidateBufferSubData oc_gl_get_api()->InvalidateBufferSubData
#define glUniformMatrix4dv oc_gl_get_api()->UniformMatrix4dv
#define glIsQuery oc_gl_get_api()->IsQuery
#define glUniform4ui oc_gl_get_api()->Uniform4ui
#define glUniform4i oc_gl_get_api()->Uniform4i
#define glGetSamplerParameteriv oc_gl_get_api()->GetSamplerParameteriv
#define glMultiDrawElementsBaseVertex oc_gl_get_api()->MultiDrawElementsBaseVertex
#define glVertexAttribI1uiv oc_gl_get_api()->VertexAttribI1uiv
#define glGetIntegerv oc_gl_get_api()->GetIntegerv
#define glUniformMatrix2x3fv oc_gl_get_api()->UniformMatrix2x3fv
#define glTexImage2D oc_gl_get_api()->TexImage2D
#define glGetAttachedShaders oc_gl_get_api()->GetAttachedShaders
#define glUniform2d oc_gl_get_api()->Uniform2d
#define glMemoryBarrierByRegion oc_gl_get_api()->MemoryBarrierByRegion
#define glUniformMatrix2fv oc_gl_get_api()->UniformMatrix2fv
#define glPrimitiveRestartIndex oc_gl_get_api()->PrimitiveRestartIndex
#define glGetVertexAttribiv oc_gl_get_api()->GetVertexAttribiv
#define glGetAttribLocation oc_gl_get_api()->GetAttribLocation
#define glTexStorage2DMultisample oc_gl_get_api()->TexStorage2DMultisample
#define glCompressedTexSubImage2D oc_gl_get_api()->CompressedTexSubImage2D
#define glGetVertexAttribfv oc_gl_get_api()->GetVertexAttribfv
#define glGetBufferParameteriv oc_gl_get_api()->GetBufferParameteriv
#define glTexParameterf oc_gl_get_api()->TexParameterf
#define glFramebufferTexture2D oc_gl_get_api()->FramebufferTexture2D
#define glGetActiveAttrib oc_gl_get_api()->GetActiveAttrib
#define glInvalidateTexSubImage oc_gl_get_api()->InvalidateTexSubImage
#define glDeleteVertexArrays oc_gl_get_api()->DeleteVertexArrays
#define glVertexAttribI2ui oc_gl_get_api()->VertexAttribI2ui
#define glPointParameteriv oc_gl_get_api()->PointParameteriv
#define glGetPointerv oc_gl_get_api()->GetPointerv
#define glEnablei oc_gl_get_api()->Enablei
#define glBindBufferRange oc_gl_get_api()->BindBufferRange
#define glDrawArraysInstanced oc_gl_get_api()->DrawArraysInstanced
#define glDeleteTextures oc_gl_get_api()->DeleteTextures
#define glVertexAttrib4Niv oc_gl_get_api()->VertexAttrib4Niv
#define glMultiDrawElements oc_gl_get_api()->MultiDrawElements
#define glGetProgramiv oc_gl_get_api()->GetProgramiv
#define glDepthFunc oc_gl_get_api()->DepthFunc
#define glGenTextures oc_gl_get_api()->GenTextures
#define glGetInternalformativ oc_gl_get_api()->GetInternalformativ
#define glProgramUniform3i oc_gl_get_api()->ProgramUniform3i
#define glScissorIndexed oc_gl_get_api()->ScissorIndexed
#define glVertexAttrib2sv oc_gl_get_api()->VertexAttrib2sv
#define glTexStorage3DMultisample oc_gl_get_api()->TexStorage3DMultisample
#define glUniform2iv oc_gl_get_api()->Uniform2iv
#define glDrawArraysInstancedBaseInstance oc_gl_get_api()->DrawArraysInstancedBaseInstance
#define glVertexAttribI3ui oc_gl_get_api()->VertexAttribI3ui
#define glDeleteSamplers oc_gl_get_api()->DeleteSamplers
#define glGenVertexArrays oc_gl_get_api()->GenVertexArrays
#define glGetFramebufferParameteriv oc_gl_get_api()->GetFramebufferParameteriv
#define glPolygonMode oc_gl_get_api()->PolygonMode
#define glProgramUniformMatrix2x4fv oc_gl_get_api()->ProgramUniformMatrix2x4fv
#define glGetProgramResourceName oc_gl_get_api()->GetProgramResourceName
#define glSamplerParameteriv oc_gl_get_api()->SamplerParameteriv
#define glGetActiveSubroutineUniformiv oc_gl_get_api()->GetActiveSubroutineUniformiv
#define glGetStringi oc_gl_get_api()->GetStringi
#define glVertexAttribLFormat oc_gl_get_api()->VertexAttribLFormat
#define glVertexAttrib3d oc_gl_get_api()->VertexAttrib3d
#define glBindVertexArray oc_gl_get_api()->BindVertexArray
#define glUnmapBuffer oc_gl_get_api()->UnmapBuffer
#define glDrawElementsInstancedBaseInstance oc_gl_get_api()->DrawElementsInstancedBaseInstance
#define glUniform4uiv oc_gl_get_api()->Uniform4uiv
#define glFramebufferTexture1D oc_gl_get_api()->FramebufferTexture1D
#define glDrawTransformFeedbackStreamInstanced oc_gl_get_api()->DrawTransformFeedbackStreamInstanced
#define glStencilFunc oc_gl_get_api()->StencilFunc
#define glValidateProgram oc_gl_get_api()->ValidateProgram
#define glFlush oc_gl_get_api()->Flush
#define glProgramUniform3uiv oc_gl_get_api()->ProgramUniform3uiv
#define glDeleteRenderbuffers oc_gl_get_api()->DeleteRenderbuffers
#define glVertexAttrib4fv oc_gl_get_api()->VertexAttrib4fv
#define glUniformMatrix2dv oc_gl_get_api()->UniformMatrix2dv
#define glGetFragDataIndex oc_gl_get_api()->GetFragDataIndex
#define glUniform3iv oc_gl_get_api()->Uniform3iv
#define glMinSampleShading oc_gl_get_api()->MinSampleShading
#define glGetBooleanv oc_gl_get_api()->GetBooleanv
#define glGetMultisamplefv oc_gl_get_api()->GetMultisamplefv
#define glGetVertexAttribIuiv oc_gl_get_api()->GetVertexAttribIuiv
#define glGetProgramInfoLog oc_gl_get_api()->GetProgramInfoLog
#define glUniform4fv oc_gl_get_api()->Uniform4fv
#define glDrawBuffer oc_gl_get_api()->DrawBuffer
#define glUniform1i oc_gl_get_api()->Uniform1i
#define glProgramUniform4ui oc_gl_get_api()->ProgramUniform4ui
#define glProgramUniformMatrix3fv oc_gl_get_api()->ProgramUniformMatrix3fv
#define glBlendEquationSeparate oc_gl_get_api()->BlendEquationSeparate
#define glBindProgramPipeline oc_gl_get_api()->BindProgramPipeline
#define glGetDoublei_v oc_gl_get_api()->GetDoublei_v
#define glBufferData oc_gl_get_api()->BufferData
#define glClearColor oc_gl_get_api()->ClearColor
#define glProgramUniform4i oc_gl_get_api()->ProgramUniform4i
#define glGetTexLevelParameteriv oc_gl_get_api()->GetTexLevelParameteriv
#define glGetActiveUniformBlockiv oc_gl_get_api()->GetActiveUniformBlockiv
#define glProgramUniform1fv oc_gl_get_api()->ProgramUniform1fv
#define glPauseTransformFeedback oc_gl_get_api()->PauseTransformFeedback
#define glGetBufferPointerv oc_gl_get_api()->GetBufferPointerv
#define glInvalidateSubFramebuffer oc_gl_get_api()->InvalidateSubFramebuffer
#define glScissorIndexedv oc_gl_get_api()->ScissorIndexedv
#define glUniform2ui oc_gl_get_api()->Uniform2ui
#define glBindTexture oc_gl_get_api()->BindTexture
#define glDrawElementsInstanced oc_gl_get_api()->DrawElementsInstanced
#define glProgramUniform4f oc_gl_get_api()->ProgramUniform4f
#define glBindBufferBase oc_gl_get_api()->BindBufferBase
#define glIsShader oc_gl_get_api()->IsShader
#define glClearBufferSubData oc_gl_get_api()->ClearBufferSubData
#define glVertexAttrib4Nuiv oc_gl_get_api()->VertexAttrib4Nuiv
#define glDrawArraysIndirect oc_gl_get_api()->DrawArraysIndirect
#define glVertexAttrib4usv oc_gl_get_api()->VertexAttrib4usv
#define glUniform1d oc_gl_get_api()->Uniform1d
#define glClearTexImage oc_gl_get_api()->ClearTexImage
#define glUniform1uiv oc_gl_get_api()->Uniform1uiv
#define glBindSampler oc_gl_get_api()->BindSampler
#define glGetTexLevelParameterfv oc_gl_get_api()->GetTexLevelParameterfv
#define glClearBufferiv oc_gl_get_api()->ClearBufferiv
#define glLogicOp oc_gl_get_api()->LogicOp
#define glActiveTexture oc_gl_get_api()->ActiveTexture
#define glGetFragDataLocation oc_gl_get_api()->GetFragDataLocation
#define glBlendColor oc_gl_get_api()->BlendColor
#define glUniformMatrix4x3fv oc_gl_get_api()->UniformMatrix4x3fv
#define glProgramUniform3fv oc_gl_get_api()->ProgramUniform3fv
#define glUniform1fv oc_gl_get_api()->Uniform1fv
#define glDrawElementsBaseVertex oc_gl_get_api()->DrawElementsBaseVertex
#define glUniform4f oc_gl_get_api()->Uniform4f
#define glBlendEquationSeparatei oc_gl_get_api()->BlendEquationSeparatei
#define glBlendFuncSeparate oc_gl_get_api()->BlendFuncSeparate
#define glClearBufferuiv oc_gl_get_api()->ClearBufferuiv
#define glCopyTexSubImage1D oc_gl_get_api()->CopyTexSubImage1D
#define glDrawTransformFeedback oc_gl_get_api()->DrawTransformFeedback
#define glReadBuffer oc_gl_get_api()->ReadBuffer
#define glCopyBufferSubData oc_gl_get_api()->CopyBufferSubData
#define glGetUniformuiv oc_gl_get_api()->GetUniformuiv
#define glPolygonOffset oc_gl_get_api()->PolygonOffset
#define glDispatchCompute oc_gl_get_api()->DispatchCompute
#define glBindImageTexture oc_gl_get_api()->BindImageTexture
#define glUniformMatrix4x3dv oc_gl_get_api()->UniformMatrix4x3dv
#define glGenRenderbuffers oc_gl_get_api()->GenRenderbuffers
