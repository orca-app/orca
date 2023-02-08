/************************************************************//**
*
*	@file: win32_app.c
*	@author: Martin Fouilleul
*	@date: 16/12/2022
*	@revision:
*
*****************************************************************/

#include"mp_app.c"

#define LOG_SUBSYSTEM "Application"

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

static mp_key_mods mp_get_mod_keys()
{
	mp_key_mods mods = 0;
	if(GetKeyState(VK_SHIFT) & 0x8000)
	{
		mods |= MP_KEYMOD_SHIFT;
	}
	if(GetKeyState(VK_CONTROL) & 0x8000)
	{
		mods |= MP_KEYMOD_CTRL;
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

	mp_update_key_state(&__mpApp.inputState.mouse.buttons[button], action == MP_KEY_PRESS);
	//TODO click/double click

	mp_event event = {0};
	event.window = mp_window_handle_from_ptr(window);
	event.type = MP_EVENT_MOUSE_BUTTON;
	event.key.action = MP_KEY_PRESS;
	event.key.code = MP_MOUSE_LEFT;
	event.key.mods = mp_get_mod_keys();

	mp_queue_event(&event);
}

static void process_wheel_event(mp_window_data* window, f32 x, f32 y)
{
	mp_event event = {0};
	event.window = mp_window_handle_from_ptr(window);
	event.type = MP_EVENT_MOUSE_WHEEL;
	event.move.deltaX = -x/30.0f;
	event.move.deltaY = y/30.0f;
	event.move.mods = mp_get_mod_keys();

	mp_queue_event(&event);
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

		//TODO: enter/exit size & move

		case WM_SIZING:
		{
			RECT* rect = (RECT*)lParam;

			mp_event event = {0};
			event.window = mp_window_handle_from_ptr(mpWindow);
			event.type = MP_EVENT_WINDOW_RESIZE;
			event.frame.rect = (mp_rect){rect->bottom, rect->left, rect->top - rect->bottom, rect->right - rect->left};
			mp_queue_event(&event);
		} break;

		case WM_MOVING:
		{
			RECT* rect = (RECT*)lParam;

			mp_event event = {0};
			event.window = mp_window_handle_from_ptr(mpWindow);
			event.type = MP_EVENT_WINDOW_MOVE;
			event.frame.rect = (mp_rect){rect->bottom, rect->left, rect->top - rect->bottom, rect->right - rect->left};
			mp_queue_event(&event);
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

			mp_event event = {0};
			event.window = mp_window_handle_from_ptr(mpWindow);
			event.type = MP_EVENT_MOUSE_MOVE;
			event.move.x = LOWORD(lParam);
			event.move.y = rect.bottom - HIWORD(lParam);

			if(__mpApp.inputState.mouse.posValid)
			{
				event.move.deltaX = event.move.x - __mpApp.inputState.mouse.pos.x;
				event.move.deltaY = event.move.y - __mpApp.inputState.mouse.pos.y;
			}
			else
			{
				__mpApp.inputState.mouse.posValid = true;
			}

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
				                  .move.x = event.move.x,
				                  .move.y = event.move.y};
				mp_queue_event(&enter);
			}
			__mpApp.inputState.mouse.pos.x = event.move.x;
			__mpApp.inputState.mouse.pos.y = event.move.y;

			mp_queue_event(&event);
		} break;

		case WM_MOUSELEAVE:
		{
			__mpApp.win32.mouseTracked = false;

			if(!__mpApp.win32.mouseCaptureMask)
			{
				__mpApp.inputState.mouse.posValid = false;
			}

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
			mp_event event = {0};
			event.window = mp_window_handle_from_ptr(mpWindow);
			event.type = MP_EVENT_KEYBOARD_CHAR;
			event.character.codepoint = (utf32)wParam;
			str8 seq = utf8_encode(event.character.sequence, event.character.codepoint);
			event.character.seqLen = seq.len;
			mp_queue_event(&event);
		} break;

		case WM_DROPFILES:
		{
			//TODO
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
	while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
	__mpApp.inputState.frameCounter++;
}

//--------------------------------------------------------------------
// window management
//--------------------------------------------------------------------

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

	/*
	//NOTE: get primary monitor dimensions
	const POINT ptZero = { 0, 0 };
	HMONITOR monitor = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO monitorInfo = {.cbSize = sizeof(MONITORINFO)};
	GetMonitorInfo(monitor, &monitorInfo);
	RECT adjustRect = {rect.x, monitorInfo.rcMonitor.bottom - rect.y - rect.h, rect.w, rect.h};
	AdjustWindowRect(&adjustRect, WS_OVERLAPPEDWINDOW, false);
	*/

	HWND windowHandle = CreateWindow("ApplicationWindowClass", "Test Window",
	                                 WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
	                                 rect.w, rect.h,
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

mp_rect mp_window_get_content_rect(mp_window window)
{
	mp_rect rect = {0};
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		RECT winRect;
		if(GetClientRect(windowData->win32.hWnd, &winRect))
		{
			rect = (mp_rect){0, 0, winRect.right - winRect.left, winRect.bottom - winRect.top};
		}
	}
	return(rect);
}

/////////////////////////////////////////// WIP ///////////////////////////////////////////////
//TODO: this is thrown here for a quick test. We should:
//			- check for errors
//          - use utf8 version of API
str8 mp_app_get_executable_path(mem_arena* arena)
{
	char* buffer = mem_arena_alloc_array(arena, char, MAX_PATH+1);
	int size = GetModuleFileNameA(NULL, buffer, MAX_PATH+1);
	//TODO: check for errors...

	return(str8_from_buffer(size, buffer));
}

str8 mp_app_get_resource_path(mem_arena* arena, const char* name)
{
	str8_list list = {0};
	mem_arena* scratch = mem_scratch();

	str8 executablePath = mp_app_get_executable_path(scratch);
	char* executablePathCString = str8_to_cstring(scratch, executablePath);

	char* driveBuffer = mem_arena_alloc_array(scratch, char, MAX_PATH);
	char* dirBuffer = mem_arena_alloc_array(scratch, char, MAX_PATH);

	_splitpath_s(executablePathCString, driveBuffer, MAX_PATH, dirBuffer, MAX_PATH, 0, 0, 0, 0);

	str8 drive = str8_from_cstring(driveBuffer);
	str8 dirPath = str8_from_cstring(dirBuffer);

	str8_list_push(scratch, &list, drive);
	str8_list_push(scratch, &list, dirPath);
	str8_list_push(scratch, &list, str8_lit("\\"));
	str8_list_push(scratch, &list, str8_push_cstring(scratch, name));
	str8 path = str8_list_join(scratch, list);
	char* pathCString = str8_to_cstring(scratch, path);

	char* buffer = mem_arena_alloc_array(arena, char, path.len+1);
	char* filePart = 0;
	int size = GetFullPathName(pathCString, MAX_PATH, buffer, &filePart);

	str8 result = str8_from_buffer(size, buffer);
	return(result);
}
//////////////////////////////////////////////////////////////////////////////////////////////////

#undef LOG_SUBSYSTEM
