/************************************************************//**
*
*	@file: win32_gl_surface.c
*	@author: Martin Fouilleul
*	@date: 01/08/2022
*	@revision:
*
*****************************************************************/

#define WIN32_GL_LOADER_IMPL
#include"win32_gl_loader.h"

#include"graphics_internal.h"
#include"win32_app.h"

#define MG_IMPLEMENTS_BACKEND_GL

typedef struct mg_gl_surface
{
	mg_surface_info interface;

	HDC hDC;
	HGLRC glContext;

} mg_gl_surface;

void mg_gl_surface_destroy(mg_surface_info* interface)
{
	mg_gl_surface* surface = (mg_gl_surface*)interface;
	//TODO
}

void mg_gl_surface_prepare(mg_surface_info* interface)
{
	mg_gl_surface* surface = (mg_gl_surface*)interface;

	wglMakeCurrent(surface->hDC, surface->glContext);
}

void mg_gl_surface_present(mg_surface_info* interface)
{
	mg_gl_surface* surface = (mg_gl_surface*)interface;

	SwapBuffers(surface->hDC);
}

PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
PFNWGLMAKECONTEXTCURRENTARBPROC wglMakeContextCurrentARB;
PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;

PFNGLCREATESHADERPROC glCreateShader;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;

mg_surface mg_gl_surface_create_for_window(mp_window window)
{
	mg_surface surfaceHandle = mg_surface_nil();

	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		//NOTE: create a dummy window
		WNDCLASS windowClass = {.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
		                        .lpfnWndProc = WinProc,
		                        .hInstance = GetModuleHandleW(NULL),
		                        .lpszClassName = "HelperWindowClass",
		                        .hCursor = LoadCursor(0, IDC_ARROW)};

		if(!RegisterClass(&windowClass))
		{
			//TODO: error
			goto quit;
		}

		HWND helperWindowHandle = CreateWindow("HelperWindowClass", "Test Window",
	                                 	WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
	                                 	800, 600,
	                                 	0, 0, windowClass.hInstance, 0);

		if(!helperWindowHandle)
		{
			//TODO: error
			goto quit;
		}

		//NOTE(martin): create a dummy OpenGL context, to be able to load extensions
		HDC dummyDC = GetDC(helperWindowHandle);

		PIXELFORMATDESCRIPTOR pixelFormatDesc =
		{
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    // Flags
			PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
			32,                   // Colordepth of the framebuffer.
			0, 0, 0, 0, 0, 0,
			0,
			0,
			0,
			0, 0, 0, 0,
			24,                   // Number of bits for the depthbuffer
			8,                    // Number of bits for the stencilbuffer
			0,                    // Number of Aux buffers in the framebuffer.
			PFD_MAIN_PLANE,
			0,
			0, 0, 0
		};

		int pixelFormat = ChoosePixelFormat(dummyDC, &pixelFormatDesc);
		SetPixelFormat(dummyDC, pixelFormat, &pixelFormatDesc);

		HGLRC dummyGLContext = wglCreateContext(dummyDC);
		wglMakeCurrent(dummyDC, dummyGLContext);

		//NOTE(martin): now load extension functions
		wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
		wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
		wglMakeContextCurrentARB = (PFNWGLMAKECONTEXTCURRENTARBPROC)wglGetProcAddress("wglMakeContextCurrentARB");
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

		glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
		glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
		glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
		glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
		glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
		glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
		glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
		glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
		glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
		glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
		glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
		glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress("glGenVertexArrays");
		glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress("glBindVertexArray");
		glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
		glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
		glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
		glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");
		glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
		glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");

		//NOTE(martin): now create the true pixel format and gl context
		int pixelFormatAttrs[] = {
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_COLOR_BITS_ARB, 32,
			WGL_DEPTH_BITS_ARB, 24,
			WGL_STENCIL_BITS_ARB, 8,
			0};

		HDC hDC = GetDC(windowData->win32.hWnd);
		u32 numFormats = 0;

		wglChoosePixelFormatARB(hDC, pixelFormatAttrs, 0, 1, &pixelFormat, &numFormats);

		if(!pixelFormat)
		{
			//TODO: error
		}
		SetPixelFormat(hDC, pixelFormat, &pixelFormatDesc);

		int contextAttrs[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
			WGL_CONTEXT_MINOR_VERSION_ARB, 3,
			0};

		HGLRC glContext = wglCreateContextAttribsARB(hDC, dummyGLContext, contextAttrs);

		if(!glContext)
		{
			//TODO error
			int error = GetLastError();
			printf("error: %i\n", error);
		}

		//NOTE: destroy dummy context and dummy window
		wglMakeCurrent(hDC, 0);
		wglDeleteContext(dummyGLContext);
		DestroyWindow(helperWindowHandle);

		//NOTE: make gl context current
		wglMakeCurrent(hDC, glContext);
		wglSwapIntervalEXT(1);

		//TODO save important info in surface_data and return a handle
		mg_gl_surface* surface = malloc_type(mg_gl_surface);
		surface->interface.backend = MG_BACKEND_GL;
		surface->interface.destroy = mg_gl_surface_destroy;
		surface->interface.prepare = mg_gl_surface_prepare;
		surface->interface.present = mg_gl_surface_present;

		surface->hDC = hDC;
		surface->glContext = glContext;

		surfaceHandle = mg_surface_alloc_handle((mg_surface_info*)surface);
	}

	quit:;
	return(surfaceHandle);
}
