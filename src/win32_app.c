/************************************************************//**
*
*	@file: win32_app.c
*	@author: Martin Fouilleul
*	@date: 16/12/2022
*	@revision:
*
*****************************************************************/

#include<dwmapi.h>
#include"mp_app.c"
#include"platform_thread.h"

void mp_init_keys()
{
	memset(__mpApp.keyCodes, MP_KEY_UNKNOWN, 256*sizeof(int));

	__mpApp.keyCodes[0x00B] = MP_KEY_0;
    __mpApp.keyCodes[0x002] = MP_KEY_1;
    __mpApp.keyCodes[0x003] = MP_KEY_2;
    __mpApp.keyCodes[0x004] = MP_KEY_3;
    __mpApp.keyCodes[0x005] = MP_KEY_4;
    __mpApp.keyCodes[0x006] = MP_KEY_5;
    __mpApp.keyCodes[0x007] = MP_KEY_6;
    __mpApp.keyCodes[0x008] = MP_KEY_7;
    __mpApp.keyCodes[0x009] = MP_KEY_8;
    __mpApp.keyCodes[0x00A] = MP_KEY_9;
    __mpApp.keyCodes[0x01E] = MP_KEY_A;
    __mpApp.keyCodes[0x030] = MP_KEY_B;
    __mpApp.keyCodes[0x02E] = MP_KEY_C;
    __mpApp.keyCodes[0x020] = MP_KEY_D;
    __mpApp.keyCodes[0x012] = MP_KEY_E;
    __mpApp.keyCodes[0x021] = MP_KEY_F;
    __mpApp.keyCodes[0x022] = MP_KEY_G;
    __mpApp.keyCodes[0x023] = MP_KEY_H;
    __mpApp.keyCodes[0x017] = MP_KEY_I;
    __mpApp.keyCodes[0x024] = MP_KEY_J;
    __mpApp.keyCodes[0x025] = MP_KEY_K;
    __mpApp.keyCodes[0x026] = MP_KEY_L;
    __mpApp.keyCodes[0x032] = MP_KEY_M;
    __mpApp.keyCodes[0x031] = MP_KEY_N;
    __mpApp.keyCodes[0x018] = MP_KEY_O;
    __mpApp.keyCodes[0x019] = MP_KEY_P;
    __mpApp.keyCodes[0x010] = MP_KEY_Q;
    __mpApp.keyCodes[0x013] = MP_KEY_R;
    __mpApp.keyCodes[0x01F] = MP_KEY_S;
    __mpApp.keyCodes[0x014] = MP_KEY_T;
    __mpApp.keyCodes[0x016] = MP_KEY_U;
    __mpApp.keyCodes[0x02F] = MP_KEY_V;
    __mpApp.keyCodes[0x011] = MP_KEY_W;
    __mpApp.keyCodes[0x02D] = MP_KEY_X;
    __mpApp.keyCodes[0x015] = MP_KEY_Y;
    __mpApp.keyCodes[0x02C] = MP_KEY_Z;
    __mpApp.keyCodes[0x028] = MP_KEY_APOSTROPHE;
    __mpApp.keyCodes[0x02B] = MP_KEY_BACKSLASH;
    __mpApp.keyCodes[0x033] = MP_KEY_COMMA;
    __mpApp.keyCodes[0x00D] = MP_KEY_EQUAL;
    __mpApp.keyCodes[0x029] = MP_KEY_GRAVE_ACCENT;
    __mpApp.keyCodes[0x01A] = MP_KEY_LEFT_BRACKET;
    __mpApp.keyCodes[0x00C] = MP_KEY_MINUS;
    __mpApp.keyCodes[0x034] = MP_KEY_PERIOD;
    __mpApp.keyCodes[0x01B] = MP_KEY_RIGHT_BRACKET;
    __mpApp.keyCodes[0x027] = MP_KEY_SEMICOLON;
    __mpApp.keyCodes[0x035] = MP_KEY_SLASH;
    __mpApp.keyCodes[0x056] = MP_KEY_WORLD_2;
    __mpApp.keyCodes[0x00E] = MP_KEY_BACKSPACE;
    __mpApp.keyCodes[0x153] = MP_KEY_DELETE;
    __mpApp.keyCodes[0x14F] = MP_KEY_END;
    __mpApp.keyCodes[0x01C] = MP_KEY_ENTER;
    __mpApp.keyCodes[0x001] = MP_KEY_ESCAPE;
    __mpApp.keyCodes[0x147] = MP_KEY_HOME;
    __mpApp.keyCodes[0x152] = MP_KEY_INSERT;
    __mpApp.keyCodes[0x15D] = MP_KEY_MENU;
    __mpApp.keyCodes[0x151] = MP_KEY_PAGE_DOWN;
    __mpApp.keyCodes[0x149] = MP_KEY_PAGE_UP;
    __mpApp.keyCodes[0x045] = MP_KEY_PAUSE;
    __mpApp.keyCodes[0x146] = MP_KEY_PAUSE;
    __mpApp.keyCodes[0x039] = MP_KEY_SPACE;
    __mpApp.keyCodes[0x00F] = MP_KEY_TAB;
    __mpApp.keyCodes[0x03A] = MP_KEY_CAPS_LOCK;
    __mpApp.keyCodes[0x145] = MP_KEY_NUM_LOCK;
    __mpApp.keyCodes[0x046] = MP_KEY_SCROLL_LOCK;
    __mpApp.keyCodes[0x03B] = MP_KEY_F1;
    __mpApp.keyCodes[0x03C] = MP_KEY_F2;
    __mpApp.keyCodes[0x03D] = MP_KEY_F3;
    __mpApp.keyCodes[0x03E] = MP_KEY_F4;
    __mpApp.keyCodes[0x03F] = MP_KEY_F5;
    __mpApp.keyCodes[0x040] = MP_KEY_F6;
    __mpApp.keyCodes[0x041] = MP_KEY_F7;
    __mpApp.keyCodes[0x042] = MP_KEY_F8;
    __mpApp.keyCodes[0x043] = MP_KEY_F9;
    __mpApp.keyCodes[0x044] = MP_KEY_F10;
    __mpApp.keyCodes[0x057] = MP_KEY_F11;
    __mpApp.keyCodes[0x058] = MP_KEY_F12;
    __mpApp.keyCodes[0x064] = MP_KEY_F13;
    __mpApp.keyCodes[0x065] = MP_KEY_F14;
    __mpApp.keyCodes[0x066] = MP_KEY_F15;
    __mpApp.keyCodes[0x067] = MP_KEY_F16;
    __mpApp.keyCodes[0x068] = MP_KEY_F17;
    __mpApp.keyCodes[0x069] = MP_KEY_F18;
    __mpApp.keyCodes[0x06A] = MP_KEY_F19;
    __mpApp.keyCodes[0x06B] = MP_KEY_F20;
    __mpApp.keyCodes[0x06C] = MP_KEY_F21;
    __mpApp.keyCodes[0x06D] = MP_KEY_F22;
    __mpApp.keyCodes[0x06E] = MP_KEY_F23;
    __mpApp.keyCodes[0x076] = MP_KEY_F24;
    __mpApp.keyCodes[0x038] = MP_KEY_LEFT_ALT;
    __mpApp.keyCodes[0x01D] = MP_KEY_LEFT_CONTROL;
    __mpApp.keyCodes[0x02A] = MP_KEY_LEFT_SHIFT;
    __mpApp.keyCodes[0x15B] = MP_KEY_LEFT_SUPER;
    __mpApp.keyCodes[0x137] = MP_KEY_PRINT_SCREEN;
    __mpApp.keyCodes[0x138] = MP_KEY_RIGHT_ALT;
    __mpApp.keyCodes[0x11D] = MP_KEY_RIGHT_CONTROL;
    __mpApp.keyCodes[0x036] = MP_KEY_RIGHT_SHIFT;
    __mpApp.keyCodes[0x15C] = MP_KEY_RIGHT_SUPER;
    __mpApp.keyCodes[0x150] = MP_KEY_DOWN;
    __mpApp.keyCodes[0x14B] = MP_KEY_LEFT;
    __mpApp.keyCodes[0x14D] = MP_KEY_RIGHT;
    __mpApp.keyCodes[0x148] = MP_KEY_UP;
    __mpApp.keyCodes[0x052] = MP_KEY_KP_0;
    __mpApp.keyCodes[0x04F] = MP_KEY_KP_1;
    __mpApp.keyCodes[0x050] = MP_KEY_KP_2;
    __mpApp.keyCodes[0x051] = MP_KEY_KP_3;
    __mpApp.keyCodes[0x04B] = MP_KEY_KP_4;
    __mpApp.keyCodes[0x04C] = MP_KEY_KP_5;
    __mpApp.keyCodes[0x04D] = MP_KEY_KP_6;
    __mpApp.keyCodes[0x047] = MP_KEY_KP_7;
    __mpApp.keyCodes[0x048] = MP_KEY_KP_8;
    __mpApp.keyCodes[0x049] = MP_KEY_KP_9;
    __mpApp.keyCodes[0x04E] = MP_KEY_KP_ADD;
    __mpApp.keyCodes[0x053] = MP_KEY_KP_DECIMAL;
    __mpApp.keyCodes[0x135] = MP_KEY_KP_DIVIDE;
    __mpApp.keyCodes[0x11C] = MP_KEY_KP_ENTER;
    __mpApp.keyCodes[0x037] = MP_KEY_KP_MULTIPLY;
    __mpApp.keyCodes[0x04A] = MP_KEY_KP_SUBTRACT;

	memset(__mpApp.nativeKeys, 0, sizeof(int)*MP_KEY_COUNT);
	for(int nativeKey=0; nativeKey<256; nativeKey++)
	{
		mp_key_code mpKey = __mpApp.keyCodes[nativeKey];
		if(mpKey)
		{
			__mpApp.nativeKeys[mpKey] = nativeKey;
		}
	}
}

void mp_init()
{
	if(!__mpApp.init)
	{
		memset(&__mpApp, 0, sizeof(__mpApp));

		mp_init_common();
		mp_init_keys();

		__mpApp.win32.savedConsoleCodePage = GetConsoleOutputCP();
        SetConsoleOutputCP(CP_UTF8);

        DWORD mode;
        GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &mode);
        mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), mode);

        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	}
}

void mp_terminate()
{
	if(__mpApp.init)
	{
		SetConsoleOutputCP(__mpApp.win32.savedConsoleCodePage);

		mp_terminate_common();
		__mpApp = (mp_app){0};
	}
}

static mp_key_code mp_convert_win32_key(int code)
{
	return(__mpApp.keyCodes[code]);
}

static mp_keymod_flags mp_get_mod_keys()
{
	mp_keymod_flags mods = 0;
	if(GetKeyState(VK_SHIFT) & 0x8000)
	{
		mods |= MP_KEYMOD_SHIFT;
	}
	if(GetKeyState(VK_CONTROL) & 0x8000)
	{
		mods |= MP_KEYMOD_CTRL;
		mods |= MP_KEYMOD_MAIN_MODIFIER;
	}
	if(GetKeyState(VK_MENU) & 0x8000)
	{
		mods |= MP_KEYMOD_ALT;
	}
	if((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000)
	{
		mods |= MP_KEYMOD_CMD;
	}
	return(mods);
}

static void process_mouse_event(mp_window_data* window, mp_key_action action, mp_key_code button)
{
	if(action == MP_KEY_PRESS)
	{
		if(!__mpApp.win32.mouseCaptureMask)
		{
			SetCapture(window->win32.hWnd);
		}
		__mpApp.win32.mouseCaptureMask |= (1<<button);
	}
	else if(action == MP_KEY_RELEASE)
	{
		__mpApp.win32.mouseCaptureMask &= ~(1<<button);
		if(!__mpApp.win32.mouseCaptureMask)
		{
			ReleaseCapture();
		}
	}

	//TODO click/double click

	mp_event event = {0};
	event.window = mp_window_handle_from_ptr(window);
	event.type = MP_EVENT_MOUSE_BUTTON;
	event.key.action = action;
	event.key.code = button;
	event.key.mods = mp_get_mod_keys();

	mp_queue_event(&event);
}

static void process_wheel_event(mp_window_data* window, f32 x, f32 y)
{
	mp_event event = {0};
	event.window = mp_window_handle_from_ptr(window);
	event.type = MP_EVENT_MOUSE_WHEEL;
	event.mouse.deltaX = x/30.0f;
	event.mouse.deltaY = -y/30.0f;
	event.mouse.mods = mp_get_mod_keys();

	mp_queue_event(&event);
}

static void win32_update_child_layers(mp_window_data* window)
{
	RECT clientRect;
	GetClientRect(window->win32.hWnd, &clientRect);
	POINT point = {0};
	ClientToScreen(window->win32.hWnd, &point);

	int clientWidth = (clientRect.right - clientRect.left);
	int clientHeight = (clientRect.bottom - clientRect.top);

	HWND insertAfter = window->win32.hWnd;

	for_list(&window->win32.layers, layer, mp_layer, listElt)
	{
		SetWindowPos(layer->hWnd,
			     	insertAfter,
			     	point.x,
			     	point.y,
			     	clientWidth,
			     	clientHeight,
		            SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER);

		insertAfter = layer->hWnd;
	}
}

LRESULT WinProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	mp_window_data* mpWindow = GetPropW(windowHandle, L"MilePost");
	//TODO: put messages in queue

	switch(message)
	{
		case WM_CLOSE:
		{
			mp_event event = {0};
			event.window = mp_window_handle_from_ptr(mpWindow);
			event.type = MP_EVENT_WINDOW_CLOSE;
			mp_queue_event(&event);
		} break;

		case WM_DPICHANGED:
		{
			u32 dpi = HIWORD(wParam);
			RECT rect = *(RECT*)lParam;

			SetWindowPos(mpWindow->win32.hWnd,
			             HWND_TOP,
			             rect.left,
			             rect.top,
			             rect.right - rect.left,
			             rect.bottom - rect.top,
			             SWP_NOACTIVATE | SWP_NOZORDER);

			//TODO: send a message

		} break;

		//TODO: enter/exit size & move
		case WM_WINDOWPOSCHANGED:
		{
			win32_update_child_layers(mpWindow);
			result = DefWindowProc(windowHandle, message, wParam, lParam);
		} break;

		case WM_SIZING:
		case WM_MOVING:
		{
			RECT* rect = (RECT*)lParam;

			mp_event event = {0};
			event.type = message == WM_SIZING ? MP_EVENT_WINDOW_RESIZE : MP_EVENT_WINDOW_MOVE;
			event.window = mp_window_handle_from_ptr(mpWindow);

			event.move.frame = mp_window_get_frame_rect(event.window);
			event.move.content = mp_window_get_content_rect(event.window);

			mp_queue_event(&event);

			win32_update_child_layers(mpWindow);
		} break;

		case WM_SETFOCUS:
		{
			mp_event event = {0};
			event.window = mp_window_handle_from_ptr(mpWindow);
			event.type = MP_EVENT_WINDOW_FOCUS;
			mp_queue_event(&event);
		} break;

		case WM_KILLFOCUS:
		{
			mp_event event = {0};
			event.window = mp_window_handle_from_ptr(mpWindow);
			event.type = MP_EVENT_WINDOW_UNFOCUS;
			mp_queue_event(&event);
		} break;

		case WM_SIZE:
		{
			bool minimized = (wParam == SIZE_MINIMIZED);
			if(minimized != mpWindow->minimized)
			{
				mpWindow->minimized = minimized;

				mp_event event = {0};
				event.window = mp_window_handle_from_ptr(mpWindow);

				if(minimized)
				{
					event.type = MP_EVENT_WINDOW_HIDE;
				}
				else if(mpWindow->minimized)
				{
					event.type = MP_EVENT_WINDOW_SHOW;
				}
				mp_queue_event(&event);
			}
		} break;

		case WM_LBUTTONDOWN:
		{
			process_mouse_event(mpWindow, MP_KEY_PRESS, MP_MOUSE_LEFT);
		} break;

		case WM_RBUTTONDOWN:
		{
			process_mouse_event(mpWindow, MP_KEY_PRESS, MP_MOUSE_RIGHT);
		} break;

		case WM_MBUTTONDOWN:
		{
			process_mouse_event(mpWindow, MP_KEY_PRESS, MP_MOUSE_MIDDLE);
		} break;

		case WM_LBUTTONUP:
		{
			process_mouse_event(mpWindow, MP_KEY_RELEASE, MP_MOUSE_LEFT);
		} break;

		case WM_RBUTTONUP:
		{
			process_mouse_event(mpWindow, MP_KEY_RELEASE, MP_MOUSE_RIGHT);
		} break;

		case WM_MBUTTONUP:
		{
			process_mouse_event(mpWindow, MP_KEY_RELEASE, MP_MOUSE_MIDDLE);
		} break;

		case WM_MOUSEMOVE:
		{
			RECT rect;
			GetClientRect(mpWindow->win32.hWnd, &rect);

			u32 dpi = GetDpiForWindow(mpWindow->win32.hWnd);
			f32 scaling = (f32)dpi/96.;

			mp_event event = {0};
			event.window = mp_window_handle_from_ptr(mpWindow);
			event.type = MP_EVENT_MOUSE_MOVE;
			event.mouse.x = LOWORD(lParam) / scaling;
			event.mouse.y = HIWORD(lParam) / scaling;

			if(__mpApp.win32.mouseTracked || __mpApp.win32.mouseCaptureMask)
			{
				event.mouse.deltaX = event.mouse.x - __mpApp.win32.lastMousePos.x;
				event.mouse.deltaY = event.mouse.y - __mpApp.win32.lastMousePos.y;
			}
			__mpApp.win32.lastMousePos = (vec2){event.mouse.x, event.mouse.y};

			if(!__mpApp.win32.mouseTracked)
			{
				__mpApp.win32.mouseTracked = true;

				TRACKMOUSEEVENT track;
				memset(&track, 0, sizeof(track));
				track.cbSize = sizeof(track);
				track.dwFlags = TME_LEAVE;
				track.hwndTrack = mpWindow->win32.hWnd;
				TrackMouseEvent(&track);

				mp_event enter = {.window = event.window,
				                  .type = MP_EVENT_MOUSE_ENTER,
				                  .mouse.x = event.mouse.x,
				                  .mouse.y = event.mouse.y};
				mp_queue_event(&enter);
			}

			mp_queue_event(&event);
		} break;

		case WM_MOUSELEAVE:
		{
			__mpApp.win32.mouseTracked = false;

			mp_event event = {0};
			event.window = mp_window_handle_from_ptr(mpWindow);
			event.type = MP_EVENT_MOUSE_LEAVE;
			mp_queue_event(&event);
		} break;

		case WM_MOUSEWHEEL:
		{
			process_wheel_event(mpWindow, 0, (float)((i16)HIWORD(wParam)));
		} break;

		case WM_MOUSEHWHEEL:
		{
			process_wheel_event(mpWindow, (float)((i16)HIWORD(wParam)), 0);
		} break;

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			mp_event event = {0};
			event.window = mp_window_handle_from_ptr(mpWindow);
			event.type = MP_EVENT_KEYBOARD_KEY;
			event.key.action = (lParam & 0x40000000) ? MP_KEY_REPEAT : MP_KEY_PRESS;
			event.key.code = mp_convert_win32_key(HIWORD(lParam) & 0x1ff);
			event.key.mods = mp_get_mod_keys();
			mp_queue_event(&event);
		} break;

		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			mp_event event = {0};
			event.window = mp_window_handle_from_ptr(mpWindow);
			event.type = MP_EVENT_KEYBOARD_KEY;
			event.key.action = MP_KEY_RELEASE;
			event.key.code = mp_convert_win32_key(HIWORD(lParam) & 0x1ff);
			event.key.mods = mp_get_mod_keys();
			mp_queue_event(&event);
		} break;

		case WM_CHAR:
		{
			if((u32)wParam >= 32)
			{
				mp_event event = {0};
				event.window = mp_window_handle_from_ptr(mpWindow);
				event.type = MP_EVENT_KEYBOARD_CHAR;
				event.character.codepoint = (utf32)wParam;
				str8 seq = utf8_encode(event.character.sequence, event.character.codepoint);
				event.character.seqLen = seq.len;
				mp_queue_event(&event);
			}
		} break;

		case WM_DROPFILES:
		{
			//TODO
		} break;

		case MP_WM_USER_DISPATCH_PROC:
		{
			mp_dispatch_proc proc = (mp_dispatch_proc)wParam;
			void* user = (void*)lParam;
			result = proc(user);
		} break;

		default:
		{
			result = DefWindowProc(windowHandle, message, wParam, lParam);
		} break;
	}

	return(result);
}

//--------------------------------------------------------------------
// app management
//--------------------------------------------------------------------

bool mp_should_quit()
{
	return(__mpApp.shouldQuit);
}

void mp_cancel_quit()
{
	__mpApp.shouldQuit = false;
}

void mp_request_quit()
{
	__mpApp.shouldQuit = true;
}

void mp_pump_events(f64 timeout)
{
	MSG message;

	if(timeout < 0)
	{
		WaitMessage();
	}
	else if(timeout > 0)
	{
		MsgWaitForMultipleObjects(0, NULL, FALSE, (DWORD) (timeout * 1e3), QS_ALLEVENTS);
	}

	while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
}

i32 mp_dispatch_on_main_thread_sync(mp_window main_window, mp_dispatch_proc proc, void* user)
{
	mp_window_data* window_data = mp_window_ptr_from_handle(main_window);
	DEBUG_ASSERT(window_data != NULL);

	LRESULT result = SendMessage(window_data->win32.hWnd, MP_WM_USER_DISPATCH_PROC, (WPARAM)proc, (LPARAM)user);
	return result;
}

//--------------------------------------------------------------------
// window management
//--------------------------------------------------------------------

//WARN: the following header pulls in objbase.h (even with WIN32_LEAN_AND_MEAN), which
//      #defines interface to struct... so make sure to #undef interface since it's a
//      name we want to be able to use throughout the codebase
#include<ShellScalingApi.h>
#undef interface

mp_window mp_window_create(mp_rect rect, const char* title, mp_window_style style)
{
	WNDCLASS windowClass = {.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
	                       .lpfnWndProc = WinProc,
	                       .hInstance = GetModuleHandleW(NULL),
	                       .lpszClassName = "ApplicationWindowClass",
	                       .hCursor = LoadCursor(0, IDC_ARROW)};

	if(!RegisterClass(&windowClass))
	{
		//TODO: error
		goto quit;
	}

	u32 dpiX, dpiY;
	HMONITOR monitor = MonitorFromPoint((POINT){rect.x, rect.y}, MONITOR_DEFAULTTOPRIMARY);
	GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);

	f32 scaleX = (f32)dpiX/96.;
	f32 scaleY = (f32)dpiY/96.;

	RECT frame = {
		rect.x * scaleX,
		rect.y * scaleY,
		(rect.x + rect.w)*scaleX,
		(rect.y + rect.h)*scaleY
	};

	DWORD winStyle = WS_OVERLAPPEDWINDOW;
	AdjustWindowRect(&frame, winStyle, FALSE);

	HWND windowHandle = CreateWindow("ApplicationWindowClass", "Test Window",
	                                 winStyle,
	                                 frame.left, frame.top,
	                                 frame.right-frame.left,
	                                 frame.bottom-frame.top,
	                                 0, 0, windowClass.hInstance, 0);

	if(!windowHandle)
	{
		//TODO: error
		goto quit;
	}

	UpdateWindow(windowHandle);

	//TODO: return wrapped window
	quit:;
	mp_window_data* window = mp_window_alloc();
	window->win32.hWnd = windowHandle;
	window->win32.layers = (list_info){0};

	SetPropW(windowHandle, L"MilePost", window);

	return(mp_window_handle_from_ptr(window));
}

void mp_window_destroy(mp_window window)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		DestroyWindow(windowData->win32.hWnd);
		//TODO: check when to unregister class

		mp_window_recycle_ptr(windowData);
	}
}

void* mp_window_native_pointer(mp_window window)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		return(windowData->win32.hWnd);
	}
	else
	{
		return(0);
	}
}

bool mp_window_should_close(mp_window window)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		return(windowData->shouldClose);
	}
	else
	{
		return(false);
	}
}

void mp_window_request_close(mp_window window)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		windowData->shouldClose = true;
		PostMessage(windowData->win32.hWnd, WM_CLOSE, 0, 0);
	}
}

void mp_window_cancel_close(mp_window window)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		windowData->shouldClose = false;
	}
}


bool mp_window_is_hidden(mp_window window)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		return(IsWindowVisible(windowData->win32.hWnd));
	}
	else
	{
		return(false);
	}
}

void mp_window_hide(mp_window window)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		ShowWindow(windowData->win32.hWnd, SW_HIDE);
	}
}

void mp_window_show(mp_window window)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		ShowWindow(windowData->win32.hWnd, SW_NORMAL);
	}
}

bool mp_window_is_minimized(mp_window window)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		return(windowData->minimized);
	}
	else
	{
		return(false);
	}
}

void mp_window_minimize(mp_window window)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		ShowWindow(windowData->win32.hWnd, SW_MINIMIZE);
	}
}

void mp_window_maximize(mp_window window)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		ShowWindow(windowData->win32.hWnd, SW_MAXIMIZE);
	}
}

void mp_window_restore(mp_window window)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		ShowWindow(windowData->win32.hWnd, SW_RESTORE);
	}
}

bool mp_window_has_focus(mp_window window)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		return(GetActiveWindow() == windowData->win32.hWnd);
	}
	else
	{
		return(false);
	}
}

void mp_window_focus(mp_window window)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		SetFocus(windowData->win32.hWnd);
	}
}

void mp_window_unfocus(mp_window window)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		SetFocus(0);
	}
}

void mp_window_send_to_back(mp_window window)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		SetWindowPos(windowData->win32.hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
}

void mp_window_bring_to_front(mp_window window)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		if(!IsWindowVisible(windowData->win32.hWnd))
		{
			ShowWindow(windowData->win32.hWnd, SW_NORMAL);
		}
		SetWindowPos(windowData->win32.hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
}

mp_rect mp_window_get_frame_rect(mp_window window)
{
	mp_rect rect = {0};
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		//NOTE: GetWindowRect() includes the drop shadow, which we don't want, so we call
		//      DwmGetWindowAttribute() instead.
		//      Note that contrary to what the GetWindowRect() docs suggests when mentionning
		//      this, DwmGetWindowAttributes() _does_ seem to adjust for DPI.
		u32 dpi = GetDpiForWindow(windowData->win32.hWnd);
		f32 scale = (float)dpi/96.;

		RECT frame;
		HRESULT res = DwmGetWindowAttribute(windowData->win32.hWnd,
		                                    DWMWA_EXTENDED_FRAME_BOUNDS,
		                                    &frame,
		                                    sizeof(RECT));
		if(res == S_OK)
		{
			rect = (mp_rect){
				frame.left / scale,
				frame.top / scale,
				(frame.right - frame.left)/scale,
				(frame.bottom - frame.top)/scale};
		}
	}
	return(rect);
}

mp_rect win32_get_drop_shadow_offsets(HWND hWnd)
{
	RECT frameIncludingShadow;
	RECT frameExcludingShadow;

	GetWindowRect(hWnd, &frameIncludingShadow);
	DwmGetWindowAttribute(hWnd,
	                      DWMWA_EXTENDED_FRAME_BOUNDS,
	                      &frameExcludingShadow,
	                      sizeof(RECT));

	mp_rect extents = {
		.x = frameIncludingShadow.left - frameExcludingShadow.left,
		.y = frameIncludingShadow.top - frameExcludingShadow.top,
		.w = frameIncludingShadow.right - frameExcludingShadow.right,
		.h = frameIncludingShadow.bottom- frameExcludingShadow.bottom
	};

	return(extents);
}

void mp_window_set_frame_rect(mp_window window, mp_rect rect)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		u32 dpi = GetDpiForWindow(windowData->win32.hWnd);
		f32 scale = (float)dpi/96.;

		//NOTE compute the size of the drop shadow to add it in setwindowpos
		mp_rect shadowOffsets = win32_get_drop_shadow_offsets(windowData->win32.hWnd);

		RECT frame = {
			rect.x * scale + shadowOffsets.x,
			rect.y * scale + shadowOffsets.y,
			(rect.x + rect.w)*scale + shadowOffsets.w,
			(rect.y + rect.h)*scale + shadowOffsets.h
		};

		SetWindowPos(windowData->win32.hWnd,
		             HWND_TOP,
		             frame.left,
		             frame.top,
		             frame.right - frame.left,
		             frame.bottom - frame.top,
		             SWP_NOZORDER|SWP_NOACTIVATE);
	}
}

mp_rect mp_window_get_content_rect(mp_window window)
{
	mp_rect rect = {0};
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		RECT client;
		if(GetClientRect(windowData->win32.hWnd, &client))
		{
			u32 dpi = GetDpiForWindow(windowData->win32.hWnd);
			f32 scale = (float)dpi/96.;

			POINT origin = {0, 0};
			ClientToScreen(windowData->win32.hWnd, &origin);

			rect = (mp_rect){
				origin.x/scale,
				origin.y/scale,
				(client.right - client.left)/scale,
				(client.bottom - client.top)/scale};
		}
	}
	return(rect);
}

void mp_window_set_content_rect(mp_window window, mp_rect rect)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		u32 dpi = GetDpiForWindow(windowData->win32.hWnd);
		f32 scale = (float)dpi/96.;

		RECT frame = {
			rect.x * scale,
			rect.y * scale,
			(rect.x + rect.w)*scale,
			(rect.y + rect.h)*scale};

		DWORD style = GetWindowLong(windowData->win32.hWnd, GWL_STYLE);
		BOOL menu = (GetMenu(windowData->win32.hWnd) != NULL);
		AdjustWindowRect(&frame, style, menu);

		SetWindowPos(windowData->win32.hWnd,
		             HWND_TOP,
		             frame.left,
		             frame.top,
		             frame.right - frame.left,
		             frame.bottom - frame.top,
		             SWP_NOZORDER|SWP_NOACTIVATE);
	}
}

void mp_window_center(mp_window window)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{

		mp_rect frame = mp_window_get_frame_rect(window);

		HMONITOR monitor = MonitorFromWindow(windowData->win32.hWnd, MONITOR_DEFAULTTOPRIMARY);
		if(monitor)
		{
			MONITORINFO monitorInfo = {.cbSize = sizeof(MONITORINFO)};
			GetMonitorInfoW(monitor, &monitorInfo);

			int dpiX, dpiY;
			GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
			f32 scaleX = dpiX/96.;
			f32 scaleY = dpiY/96.;

			f32 monX = monitorInfo.rcWork.left/scaleX;
			f32 monY = monitorInfo.rcWork.top/scaleY;
			f32 monW = (monitorInfo.rcWork.right - monitorInfo.rcWork.left)/scaleX;
			f32 monH = (monitorInfo.rcWork.bottom - monitorInfo.rcWork.top)/scaleY;

			frame.x = monX + 0.5*(monW - frame.w);
			frame.y = monY + 0.5*(monH - frame.h);

			mp_window_set_frame_rect(window, frame);
		}
	}
}


//--------------------------------------------------------------------------------
// clipboard functions
//--------------------------------------------------------------------------------

MP_API void mp_clipboard_clear(void)
{
	if(OpenClipboard(NULL))
	{
		EmptyClipboard();
		CloseClipboard();
	}
}

MP_API void mp_clipboard_set_string(str8 string)
{
	if(OpenClipboard(NULL))
	{
		EmptyClipboard();

		int wideCount = MultiByteToWideChar(CP_UTF8, 0, string.ptr, string.len, 0, 0);
		HANDLE handle = GlobalAlloc(GMEM_MOVEABLE, (wideCount+1)*sizeof(wchar_t));
		if(handle)
		{
			char* memory = GlobalLock(handle);
			if(memory)
			{
				MultiByteToWideChar(CP_UTF8, 0, string.ptr, string.len, (wchar_t*)memory, wideCount);
				((wchar_t*)memory)[wideCount] = '\0';

				GlobalUnlock(handle);
				SetClipboardData(CF_UNICODETEXT, handle);
			}
		}
		CloseClipboard();
	}
}

MP_API str8 mp_clipboard_get_string(mem_arena* arena)
{
	str8 string = {0};

	if(OpenClipboard(NULL))
	{
		HANDLE handle = GetClipboardData(CF_UNICODETEXT);
		if(handle)
		{
			char* memory = GlobalLock(handle);
			if(memory)
			{
				u64 size = WideCharToMultiByte(CP_UTF8, 0, (wchar_t*)memory, -1, 0, 0, 0, 0);
				if(size)
				{
					string.ptr = mem_arena_alloc(arena, size);
					string.len = size - 1;
					WideCharToMultiByte(CP_UTF8, 0, (wchar_t*)memory, -1, string.ptr, size, 0, 0);
					GlobalUnlock(handle);
				}
			}
		}
		CloseClipboard();
	}
	return(string);
}

MP_API str8 mp_clipboard_copy_string(str8 backing)
{
	//TODO
	return((str8){0});
}


//--------------------------------------------------------------------------------
// win32 surfaces
//--------------------------------------------------------------------------------

#include"graphics_surface.h"

vec2 mg_win32_surface_contents_scaling(mg_surface_data* surface)
{
	u32 dpi = GetDpiForWindow(surface->layer.hWnd);
	vec2 contentsScaling = (vec2){(float)dpi/96., (float)dpi/96.};
	return(contentsScaling);
}

vec2 mg_win32_surface_get_size(mg_surface_data* surface)
{
	vec2 size = {0};
	RECT rect;
	if(GetClientRect(surface->layer.hWnd, &rect))
	{
		u32 dpi = GetDpiForWindow(surface->layer.hWnd);
		f32 scale = (float)dpi/96.;
		size = (vec2){(rect.right - rect.left)/scale, (rect.bottom - rect.top)/scale};
	}
	return(size);
}

bool mg_win32_surface_get_hidden(mg_surface_data* surface)
{
	bool hidden = !IsWindowVisible(surface->layer.hWnd);
	return(hidden);
}

void mg_win32_surface_set_hidden(mg_surface_data* surface, bool hidden)
{
	ShowWindow(surface->layer.hWnd, hidden ? SW_HIDE : SW_NORMAL);
}

void* mg_win32_surface_native_layer(mg_surface_data* surface)
{
	return((void*)surface->layer.hWnd);
}

mg_surface_id mg_win32_surface_remote_id(mg_surface_data* surface)
{
	return((mg_surface_id)surface->layer.hWnd);
}

void mg_win32_surface_host_connect(mg_surface_data* surface, mg_surface_id remoteID)
{
	HWND dstWnd = surface->layer.hWnd;
	HWND srcWnd = (HWND)remoteID;

	RECT dstRect;
	GetClientRect(dstWnd, &dstRect);

	SetParent(srcWnd, dstWnd);
	ShowWindow(srcWnd, SW_NORMAL);

	SetWindowPos(srcWnd,
			     HWND_TOP,
			     0,
			     0,
			     dstRect.right - dstRect.left,
			     dstRect.bottom - dstRect.top,
			     SWP_NOACTIVATE | SWP_NOZORDER);
}

void mg_surface_cleanup(mg_surface_data* surface)
{
	list_remove(&surface->layer.parent->win32.layers, &surface->layer.listElt);
	DestroyWindow(surface->layer.hWnd);
}

LRESULT LayerWinProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	if(message == WM_NCHITTEST)
	{
		return(HTTRANSPARENT);
	}
	else
	{
		return(DefWindowProc(windowHandle, message, wParam, lParam));
	}
}

void mg_surface_init_for_window(mg_surface_data* surface, mp_window_data* window)
{
	surface->contentsScaling = mg_win32_surface_contents_scaling;
	surface->getSize = mg_win32_surface_get_size;
	surface->getHidden = mg_win32_surface_get_hidden;
	surface->setHidden = mg_win32_surface_set_hidden;
	surface->nativeLayer = mg_win32_surface_native_layer;

	//NOTE(martin): create a child window for the surface
	WNDCLASS layerWindowClass = {.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
		                         .lpfnWndProc = LayerWinProc,
		                         .hInstance = GetModuleHandleW(NULL),
		                         .lpszClassName = "layer_window_class",
		                         .hCursor = LoadCursor(0, IDC_ARROW)};

	RegisterClass(&layerWindowClass);

	RECT parentRect;
	GetClientRect(window->win32.hWnd, &parentRect);
	POINT point = {0};
	ClientToScreen(window->win32.hWnd, &point);

	int clientWidth = parentRect.right - parentRect.left;
	int clientHeight = parentRect.bottom - parentRect.top;

	surface->layer.hWnd = CreateWindow("layer_window_class", "layer",
	                                     WS_POPUP | WS_VISIBLE,
	                                     point.x, point.y, clientWidth, clientHeight,
	                                     window->win32.hWnd,
	                                     0,
	                                     layerWindowClass.hInstance,
	                                     0);

	HRGN region = CreateRectRgn(0, 0, -1, -1);

	DWM_BLURBEHIND bb = {0};
	bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
	bb.hRgnBlur = region;
	bb.fEnable = TRUE;

	HRESULT res = DwmEnableBlurBehindWindow(surface->layer.hWnd, &bb);

	DeleteObject(region);
	if(res != S_OK)
	{
		log_error("couldn't enable blur behind\n");
	}

	surface->layer.parent = window;
	list_append(&window->win32.layers, &surface->layer.listElt);
}

void mg_surface_init_remote(mg_surface_data* surface, u32 width, u32 height)
{
	surface->contentsScaling = mg_win32_surface_contents_scaling;
	surface->getSize = mg_win32_surface_get_size;
	surface->getHidden = mg_win32_surface_get_hidden;
	surface->setHidden = mg_win32_surface_set_hidden;
	surface->nativeLayer = mg_win32_surface_native_layer;
	surface->remoteID = mg_win32_surface_remote_id;

	WNDCLASS layerWindowClass = {.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
		                         .lpfnWndProc = DefWindowProc,
		                         .hInstance = GetModuleHandleW(NULL),
		                         .lpszClassName = "server_layer_window_class",
		                         .hCursor = LoadCursor(0, IDC_ARROW)};

	RegisterClass(&layerWindowClass);

	//NOTE(martin): create a temporary parent window. This seems like a necessary hack, because if layer window is created as
	//              a normal window first, and then parented to the client window, it breaks resizing the parent
	//              window for some reason...
	HWND tmpParent = CreateWindow("server_layer_window_class", "layerParent",
	                              WS_OVERLAPPED,
	                              0, 0, width, height,
	                              0,
	                              0,
	                              layerWindowClass.hInstance,
	                              0);

	//NOTE: create the layer window
	surface->layer.hWnd = CreateWindowEx(WS_EX_NOACTIVATE,
	                             "server_layer_window_class", "layer",
	                             WS_CHILD,
	                             0, 0, width, height,
	                             tmpParent,
	                             0,
	                             layerWindowClass.hInstance,
	                             0);

	//NOTE: unparent it and destroy tmp parent
	SetParent(surface->layer.hWnd, 0);
	DestroyWindow(tmpParent);
}

mg_surface_data* mg_win32_surface_create_host(mp_window window)
{
	mg_surface_data* surface = 0;
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		surface = malloc_type(mg_surface_data);
		if(surface)
		{
			memset(surface, 0, sizeof(mg_surface_data));
			mg_surface_init_for_window(surface, windowData);

			surface->api = MG_HOST;
			surface->hostConnect = mg_win32_surface_host_connect;
		}
	}
	return(surface);
}

//--------------------------------------------------------------------
// native open/save/alert windows
//--------------------------------------------------------------------

//TODO: GetOpenFileName() doesn't seem to support selecting folders, and
//      requires filters which pair a "descriptive" name with an extension

#define interface struct
#include<shobjidl.h>
#include<shtypes.h>
#undef interface


MP_API str8 mp_open_dialog(mem_arena* arena,
                           const char* title,
                           const char* defaultPath,
                           int filterCount,
                           const char** filters,
                           bool directory)
{
	str8 res = {0};
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if(SUCCEEDED(hr))
	{
		IFileOpenDialog* dialog = 0;
		hr = CoCreateInstance(&CLSID_FileOpenDialog, NULL, CLSCTX_ALL, &IID_IFileOpenDialog, (void**)&dialog);
		if(SUCCEEDED(hr))
		{
			if(directory)
			{
				FILEOPENDIALOGOPTIONS opt;
				dialog->lpVtbl->GetOptions(dialog, &opt);
				dialog->lpVtbl->SetOptions(dialog, opt | FOS_PICKFOLDERS);
			}

			if(filterCount && filters)
			{
				mem_arena_scope tmp = mem_arena_scope_begin(arena);
				COMDLG_FILTERSPEC* filterSpecs = mem_arena_alloc_array(arena, COMDLG_FILTERSPEC, filterCount);
				for(int i=0; i<filterCount; i++)
				{
					str8_list list = {0};
					str8_list_push(arena, &list, STR8("*."));
					str8_list_push(arena, &list, STR8(filters[i]));
					str8 filter = str8_list_join(arena, list);

					int filterWideSize = 1 + MultiByteToWideChar(CP_UTF8, 0, filter.ptr, filter.len, NULL, 0);
					filterSpecs[i].pszSpec = mem_arena_alloc_array(arena, wchar_t, filterWideSize);
					MultiByteToWideChar(CP_UTF8, 0, filter.ptr, filter.len, (LPWSTR)filterSpecs[i].pszSpec, filterWideSize);
					((LPWSTR)(filterSpecs[i].pszSpec))[filterWideSize-1] = 0;

					filterSpecs[i].pszName = filterSpecs[i].pszSpec;
				}

				hr = dialog->lpVtbl->SetFileTypes(dialog, filterCount, filterSpecs);

				mem_arena_scope_end(tmp);
			}

			if(defaultPath)
			{
				mem_arena_scope tmp = mem_arena_scope_begin(arena);
				int pathWideSize = MultiByteToWideChar(CP_UTF8, 0, defaultPath, -1, NULL, 0);
				LPWSTR pathWide = mem_arena_alloc_array(arena, wchar_t, pathWideSize);
				MultiByteToWideChar(CP_UTF8, 0, defaultPath, -1, pathWide, pathWideSize);

				IShellItem* item = 0;
				hr = SHCreateItemFromParsingName(pathWide, NULL, &IID_IShellItem, (void**)&item);
				if(SUCCEEDED(hr))
				{
					hr = dialog->lpVtbl->SetFolder(dialog, item);
					item->lpVtbl->Release(item);
				}
				mem_arena_scope_end(tmp);
			}

			hr = dialog->lpVtbl->Show(dialog, NULL);
			if(SUCCEEDED(hr))
			{
				IShellItem* item;
				hr = dialog->lpVtbl->GetResult(dialog, &item);
				if(SUCCEEDED(hr))
				{
					PWSTR filePath;
					hr = item->lpVtbl->GetDisplayName(item, SIGDN_FILESYSPATH, &filePath);

					if(SUCCEEDED(hr))
					{
						int utf8Size = WideCharToMultiByte(CP_UTF8, 0, filePath, -1, NULL, 0, NULL, NULL);
						if(utf8Size > 0)
						{
							res.ptr = mem_arena_alloc(arena, utf8Size);
							res.len = utf8Size-1;
							WideCharToMultiByte(CP_UTF8, 0, filePath, -1, res.ptr, utf8Size, NULL, NULL);
						}
						CoTaskMemFree(filePath);
					}
					item->lpVtbl->Release(item);
				}
			}
		}
	}
	CoUninitialize();
	return(res);
}

MP_API str8 mp_save_dialog(mem_arena* arena,
                           const char* title,
                           const char* defaultPath,
                           int filterCount,
                           const char** filters)
{
	str8 res = {0};
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if(SUCCEEDED(hr))
	{
		IFileOpenDialog* dialog = 0;
		hr = CoCreateInstance(&CLSID_FileSaveDialog, NULL, CLSCTX_ALL, &IID_IFileSaveDialog, (void**)&dialog);
		if(SUCCEEDED(hr))
		{
			if(filterCount && filters)
			{
				mem_arena_scope tmp = mem_arena_scope_begin(arena);
				COMDLG_FILTERSPEC* filterSpecs = mem_arena_alloc_array(arena, COMDLG_FILTERSPEC, filterCount);
				for(int i=0; i<filterCount; i++)
				{
					str8_list list = {0};
					str8_list_push(arena, &list, STR8("*."));
					str8_list_push(arena, &list, STR8(filters[i]));
					str8 filter = str8_list_join(arena, list);

					int filterWideSize = 1 + MultiByteToWideChar(CP_UTF8, 0, filter.ptr, filter.len, NULL, 0);
					filterSpecs[i].pszSpec = mem_arena_alloc_array(arena, wchar_t, filterWideSize);
					MultiByteToWideChar(CP_UTF8, 0, filter.ptr, filter.len, (LPWSTR)filterSpecs[i].pszSpec, filterWideSize);
					((LPWSTR)(filterSpecs[i].pszSpec))[filterWideSize-1] = 0;

					filterSpecs[i].pszName = filterSpecs[i].pszSpec;
				}

				hr = dialog->lpVtbl->SetFileTypes(dialog, filterCount, filterSpecs);

				mem_arena_scope_end(tmp);
			}

			if(defaultPath)
			{
				mem_arena_scope tmp = mem_arena_scope_begin(arena);
				int pathWideSize = MultiByteToWideChar(CP_UTF8, 0, defaultPath, -1, NULL, 0);
				LPWSTR pathWide = mem_arena_alloc_array(arena, wchar_t, pathWideSize);
				MultiByteToWideChar(CP_UTF8, 0, defaultPath, -1, pathWide, pathWideSize);

				IShellItem* item = 0;
				hr = SHCreateItemFromParsingName(pathWide, NULL, &IID_IShellItem, (void**)&item);
				if(SUCCEEDED(hr))
				{
					hr = dialog->lpVtbl->SetFolder(dialog, item);
					item->lpVtbl->Release(item);
				}
				mem_arena_scope_end(tmp);
			}

			hr = dialog->lpVtbl->Show(dialog, NULL);
			if(SUCCEEDED(hr))
			{
				IShellItem* item;
				hr = dialog->lpVtbl->GetResult(dialog, &item);
				if(SUCCEEDED(hr))
				{
					PWSTR filePath;
					hr = item->lpVtbl->GetDisplayName(item, SIGDN_FILESYSPATH, &filePath);

					if(SUCCEEDED(hr))
					{
						int utf8Size = WideCharToMultiByte(CP_UTF8, 0, filePath, -1, NULL, 0, NULL, NULL);
						if(utf8Size > 0)
						{
							res.ptr = mem_arena_alloc(arena, utf8Size);
							res.len = utf8Size-1;
							WideCharToMultiByte(CP_UTF8, 0, filePath, -1, res.ptr, utf8Size, NULL, NULL);
						}
						CoTaskMemFree(filePath);
					}
					item->lpVtbl->Release(item);
				}
			}
		}
	}
	CoUninitialize();
	return(res);
}

#include<commctrl.h>

MP_API int mp_alert_popup(const char* title,
                          const char* message,
                          u32 count,
                          const char** options)
{
	mem_arena* scratch = mem_scratch();
	mem_arena_scope tmp = mem_arena_scope_begin(scratch);
	TASKDIALOG_BUTTON* buttons = mem_arena_alloc_array(scratch, TASKDIALOG_BUTTON, count);

	for(int i=0; i<count; i++)
	{
		int textWideSize = MultiByteToWideChar(CP_UTF8, 0, options[i], -1, NULL, 0);
		wchar_t* textWide = mem_arena_alloc_array(scratch, wchar_t, textWideSize);
		MultiByteToWideChar(CP_UTF8, 0, options[i], -1, textWide, textWideSize);

		buttons[i].nButtonID = i+1;
		buttons[i].pszButtonText = textWide;
	}

	int titleWideSize = MultiByteToWideChar(CP_UTF8, 0, title, -1, NULL, 0);
	wchar_t* titleWide = mem_arena_alloc_array(scratch, wchar_t, titleWideSize);
	MultiByteToWideChar(CP_UTF8, 0, title, -1, titleWide, titleWideSize);

	int messageWideSize = MultiByteToWideChar(CP_UTF8, 0, message, -1, NULL, 0);
	wchar_t* messageWide = mem_arena_alloc_array(scratch, wchar_t, messageWideSize);
	MultiByteToWideChar(CP_UTF8, 0, message, -1, messageWide, messageWideSize);

	TASKDIALOGCONFIG config =
	{
		.cbSize = sizeof(TASKDIALOGCONFIG),
		.hwndParent = NULL,
		.hInstance = NULL,
		.dwFlags = 0,
		.dwCommonButtons = 0,
		.pszWindowTitle = titleWide,
		.hMainIcon = 0,
		.pszMainIcon = TD_WARNING_ICON,
		.pszMainInstruction = messageWide,
		.pszContent = NULL,
		.cButtons = count,
		.pButtons = buttons,
		.nDefaultButton = 0,
	};

	int button = -1;
	HRESULT hRes = TaskDialogIndirect(&config, &button, NULL, NULL);
	if(hRes == S_OK)
	{
		if(button == IDCANCEL)
		{
			button = -1;
		}
		else
		{
			button--;
		}
	}

	mem_arena_scope_end(tmp);
	return(button);
}
