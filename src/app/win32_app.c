/************************************************************/ /**
*
*	@file: win32_app.c
*	@author: Martin Fouilleul
*	@date: 16/12/2022
*	@revision:
*
*****************************************************************/

#include "app.c"
#include "platform/platform_thread.h"
#include <dwmapi.h>

void oc_init_keys()
{
    memset(oc_appData.keyCodes, OC_KEY_UNKNOWN, 256 * sizeof(int));

    oc_appData.keyCodes[0x00B] = OC_KEY_0;
    oc_appData.keyCodes[0x002] = OC_KEY_1;
    oc_appData.keyCodes[0x003] = OC_KEY_2;
    oc_appData.keyCodes[0x004] = OC_KEY_3;
    oc_appData.keyCodes[0x005] = OC_KEY_4;
    oc_appData.keyCodes[0x006] = OC_KEY_5;
    oc_appData.keyCodes[0x007] = OC_KEY_6;
    oc_appData.keyCodes[0x008] = OC_KEY_7;
    oc_appData.keyCodes[0x009] = OC_KEY_8;
    oc_appData.keyCodes[0x00A] = OC_KEY_9;
    oc_appData.keyCodes[0x01E] = OC_KEY_A;
    oc_appData.keyCodes[0x030] = OC_KEY_B;
    oc_appData.keyCodes[0x02E] = OC_KEY_C;
    oc_appData.keyCodes[0x020] = OC_KEY_D;
    oc_appData.keyCodes[0x012] = OC_KEY_E;
    oc_appData.keyCodes[0x021] = OC_KEY_F;
    oc_appData.keyCodes[0x022] = OC_KEY_G;
    oc_appData.keyCodes[0x023] = OC_KEY_H;
    oc_appData.keyCodes[0x017] = OC_KEY_I;
    oc_appData.keyCodes[0x024] = OC_KEY_J;
    oc_appData.keyCodes[0x025] = OC_KEY_K;
    oc_appData.keyCodes[0x026] = OC_KEY_L;
    oc_appData.keyCodes[0x032] = OC_KEY_M;
    oc_appData.keyCodes[0x031] = OC_KEY_N;
    oc_appData.keyCodes[0x018] = OC_KEY_O;
    oc_appData.keyCodes[0x019] = OC_KEY_P;
    oc_appData.keyCodes[0x010] = OC_KEY_Q;
    oc_appData.keyCodes[0x013] = OC_KEY_R;
    oc_appData.keyCodes[0x01F] = OC_KEY_S;
    oc_appData.keyCodes[0x014] = OC_KEY_T;
    oc_appData.keyCodes[0x016] = OC_KEY_U;
    oc_appData.keyCodes[0x02F] = OC_KEY_V;
    oc_appData.keyCodes[0x011] = OC_KEY_W;
    oc_appData.keyCodes[0x02D] = OC_KEY_X;
    oc_appData.keyCodes[0x015] = OC_KEY_Y;
    oc_appData.keyCodes[0x02C] = OC_KEY_Z;
    oc_appData.keyCodes[0x028] = OC_KEY_APOSTROPHE;
    oc_appData.keyCodes[0x02B] = OC_KEY_BACKSLASH;
    oc_appData.keyCodes[0x033] = OC_KEY_COMMA;
    oc_appData.keyCodes[0x00D] = OC_KEY_EQUAL;
    oc_appData.keyCodes[0x029] = OC_KEY_GRAVE_ACCENT;
    oc_appData.keyCodes[0x01A] = OC_KEY_LEFT_BRACKET;
    oc_appData.keyCodes[0x00C] = OC_KEY_MINUS;
    oc_appData.keyCodes[0x034] = OC_KEY_PERIOD;
    oc_appData.keyCodes[0x01B] = OC_KEY_RIGHT_BRACKET;
    oc_appData.keyCodes[0x027] = OC_KEY_SEMICOLON;
    oc_appData.keyCodes[0x035] = OC_KEY_SLASH;
    oc_appData.keyCodes[0x056] = OC_KEY_WORLD_2;
    oc_appData.keyCodes[0x00E] = OC_KEY_BACKSPACE;
    oc_appData.keyCodes[0x153] = OC_KEY_DELETE;
    oc_appData.keyCodes[0x14F] = OC_KEY_END;
    oc_appData.keyCodes[0x01C] = OC_KEY_ENTER;
    oc_appData.keyCodes[0x001] = OC_KEY_ESCAPE;
    oc_appData.keyCodes[0x147] = OC_KEY_HOME;
    oc_appData.keyCodes[0x152] = OC_KEY_INSERT;
    oc_appData.keyCodes[0x15D] = OC_KEY_MENU;
    oc_appData.keyCodes[0x151] = OC_KEY_PAGE_DOWN;
    oc_appData.keyCodes[0x149] = OC_KEY_PAGE_UP;
    oc_appData.keyCodes[0x045] = OC_KEY_PAUSE;
    oc_appData.keyCodes[0x146] = OC_KEY_PAUSE;
    oc_appData.keyCodes[0x039] = OC_KEY_SPACE;
    oc_appData.keyCodes[0x00F] = OC_KEY_TAB;
    oc_appData.keyCodes[0x03A] = OC_KEY_CAPS_LOCK;
    oc_appData.keyCodes[0x145] = OC_KEY_NUM_LOCK;
    oc_appData.keyCodes[0x046] = OC_KEY_SCROLL_LOCK;
    oc_appData.keyCodes[0x03B] = OC_KEY_F1;
    oc_appData.keyCodes[0x03C] = OC_KEY_F2;
    oc_appData.keyCodes[0x03D] = OC_KEY_F3;
    oc_appData.keyCodes[0x03E] = OC_KEY_F4;
    oc_appData.keyCodes[0x03F] = OC_KEY_F5;
    oc_appData.keyCodes[0x040] = OC_KEY_F6;
    oc_appData.keyCodes[0x041] = OC_KEY_F7;
    oc_appData.keyCodes[0x042] = OC_KEY_F8;
    oc_appData.keyCodes[0x043] = OC_KEY_F9;
    oc_appData.keyCodes[0x044] = OC_KEY_F10;
    oc_appData.keyCodes[0x057] = OC_KEY_F11;
    oc_appData.keyCodes[0x058] = OC_KEY_F12;
    oc_appData.keyCodes[0x064] = OC_KEY_F13;
    oc_appData.keyCodes[0x065] = OC_KEY_F14;
    oc_appData.keyCodes[0x066] = OC_KEY_F15;
    oc_appData.keyCodes[0x067] = OC_KEY_F16;
    oc_appData.keyCodes[0x068] = OC_KEY_F17;
    oc_appData.keyCodes[0x069] = OC_KEY_F18;
    oc_appData.keyCodes[0x06A] = OC_KEY_F19;
    oc_appData.keyCodes[0x06B] = OC_KEY_F20;
    oc_appData.keyCodes[0x06C] = OC_KEY_F21;
    oc_appData.keyCodes[0x06D] = OC_KEY_F22;
    oc_appData.keyCodes[0x06E] = OC_KEY_F23;
    oc_appData.keyCodes[0x076] = OC_KEY_F24;
    oc_appData.keyCodes[0x038] = OC_KEY_LEFT_ALT;
    oc_appData.keyCodes[0x01D] = OC_KEY_LEFT_CONTROL;
    oc_appData.keyCodes[0x02A] = OC_KEY_LEFT_SHIFT;
    oc_appData.keyCodes[0x15B] = OC_KEY_LEFT_SUPER;
    oc_appData.keyCodes[0x137] = OC_KEY_PRINT_SCREEN;
    oc_appData.keyCodes[0x138] = OC_KEY_RIGHT_ALT;
    oc_appData.keyCodes[0x11D] = OC_KEY_RIGHT_CONTROL;
    oc_appData.keyCodes[0x036] = OC_KEY_RIGHT_SHIFT;
    oc_appData.keyCodes[0x15C] = OC_KEY_RIGHT_SUPER;
    oc_appData.keyCodes[0x150] = OC_KEY_DOWN;
    oc_appData.keyCodes[0x14B] = OC_KEY_LEFT;
    oc_appData.keyCodes[0x14D] = OC_KEY_RIGHT;
    oc_appData.keyCodes[0x148] = OC_KEY_UP;
    oc_appData.keyCodes[0x052] = OC_KEY_KP_0;
    oc_appData.keyCodes[0x04F] = OC_KEY_KP_1;
    oc_appData.keyCodes[0x050] = OC_KEY_KP_2;
    oc_appData.keyCodes[0x051] = OC_KEY_KP_3;
    oc_appData.keyCodes[0x04B] = OC_KEY_KP_4;
    oc_appData.keyCodes[0x04C] = OC_KEY_KP_5;
    oc_appData.keyCodes[0x04D] = OC_KEY_KP_6;
    oc_appData.keyCodes[0x047] = OC_KEY_KP_7;
    oc_appData.keyCodes[0x048] = OC_KEY_KP_8;
    oc_appData.keyCodes[0x049] = OC_KEY_KP_9;
    oc_appData.keyCodes[0x04E] = OC_KEY_KP_ADD;
    oc_appData.keyCodes[0x053] = OC_KEY_KP_DECIMAL;
    oc_appData.keyCodes[0x135] = OC_KEY_KP_DIVIDE;
    oc_appData.keyCodes[0x11C] = OC_KEY_KP_ENTER;
    oc_appData.keyCodes[0x037] = OC_KEY_KP_MULTIPLY;
    oc_appData.keyCodes[0x04A] = OC_KEY_KP_SUBTRACT;

    memset(oc_appData.nativeKeys, 0, sizeof(int) * OC_KEY_COUNT);
    for(int nativeKey = 0; nativeKey < 256; nativeKey++)
    {
        oc_key_code mpKey = oc_appData.keyCodes[nativeKey];
        if(mpKey)
        {
            oc_appData.nativeKeys[mpKey] = nativeKey;
        }
    }
}

void oc_init()
{
    if(!oc_appData.init)
    {
        memset(&oc_appData, 0, sizeof(oc_appData));

        oc_clock_init();

        oc_init_common();
        oc_init_keys();

        oc_appData.win32.savedConsoleCodePage = GetConsoleOutputCP();
        SetConsoleOutputCP(CP_UTF8);

        DWORD mode;
        GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &mode);
        mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), mode);

        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

        u32 wheelScrollLines = 3;
        SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &wheelScrollLines, 0);
        oc_appData.win32.wheelScrollLines = wheelScrollLines;
    }
}

void oc_terminate()
{
    if(oc_appData.init)
    {
        SetConsoleOutputCP(oc_appData.win32.savedConsoleCodePage);

        oc_terminate_common();
        oc_appData = (oc_app){ 0 };
    }
}

static oc_key_code oc_convert_win32_key(int code)
{
    return (oc_appData.keyCodes[code]);
}

static oc_keymod_flags oc_get_mod_keys()
{
    oc_keymod_flags mods = 0;
    if(GetKeyState(VK_SHIFT) & 0x8000)
    {
        mods |= OC_KEYMOD_SHIFT;
    }
    if(GetKeyState(VK_CONTROL) & 0x8000)
    {
        mods |= OC_KEYMOD_CTRL;
        mods |= OC_KEYMOD_MAIN_MODIFIER;
    }
    if(GetKeyState(VK_MENU) & 0x8000)
    {
        mods |= OC_KEYMOD_ALT;
    }
    if((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000)
    {
        mods |= OC_KEYMOD_CMD;
    }
    return (mods);
}

static void oc_win32_process_mouse_event(oc_window_data* window, oc_key_action action, oc_key_code button)
{
    if(action == OC_KEY_PRESS)
    {
        if(!oc_appData.win32.mouseCaptureMask)
        {
            SetCapture(window->win32.hWnd);
        }
        oc_appData.win32.mouseCaptureMask |= (1 << button);
    }
    else if(action == OC_KEY_RELEASE)
    {
        oc_appData.win32.mouseCaptureMask &= ~(1 << button);
        if(!oc_appData.win32.mouseCaptureMask)
        {
            ReleaseCapture();
        }
    }

    //TODO click/double click

    oc_event event = { 0 };
    event.window = oc_window_handle_from_ptr(window);
    event.type = OC_EVENT_MOUSE_BUTTON;
    event.key.action = action;
    event.key.code = button;
    event.key.mods = oc_get_mod_keys();

    oc_queue_event(&event);
}

static void oc_win32_process_wheel_event(oc_window_data* window, f32 x, f32 y)
{
    oc_event event = { 0 };
    event.window = oc_window_handle_from_ptr(window);
    event.type = OC_EVENT_MOUSE_WHEEL;
    // Borrowed from https://source.chromium.org/chromium/chromium/src/+/3e1a26c44c024d97dc9a4c09bbc6a2365398ca2c:ui/events/blink/web_input_event_builders_win.cc;l=318-330
    f32 scrollMultiplier = oc_appData.win32.wheelScrollLines * 100.0 / 3.0;
    event.mouse.deltaX = x / WHEEL_DELTA * scrollMultiplier;
    event.mouse.deltaY = -y / WHEEL_DELTA * scrollMultiplier;
    event.mouse.mods = oc_get_mod_keys();

    oc_queue_event(&event);
}

static void oc_win32_update_child_layers(oc_window_data* window)
{
    RECT clientRect;
    GetClientRect(window->win32.hWnd, &clientRect);
    POINT point = { 0 };
    ClientToScreen(window->win32.hWnd, &point);

    int clientWidth = (clientRect.right - clientRect.left);
    int clientHeight = (clientRect.bottom - clientRect.top);

    oc_list_for(&window->win32.layers, layer, oc_layer, listElt)
    {
        SetWindowPos(layer->hWnd,
                     0,
                     point.x,
                     point.y,
                     clientWidth,
                     clientHeight,
                     SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
    }
}

static void oc_win32_update_child_layers_zorder(oc_window_data* window)
{
    HWND insertAfter = window->win32.hWnd;

    oc_list_for(&window->win32.layers, layer, oc_layer, listElt)
    {
        SetWindowPos(layer->hWnd,
                     insertAfter,
                     0, 0, 0, 0,
                     SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW | SWP_NOOWNERZORDER);

        insertAfter = layer->hWnd;
    }
    SetWindowPos(window->win32.hWnd,
                 insertAfter,
                 0, 0, 0, 0,
                 SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);
}

LRESULT oc_win32_win_proc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    oc_window_data* mpWindow = GetPropW(windowHandle, L"MilePost");
    //TODO: put messages in queue

    bool handled = true;

    switch(message)
    {
        case WM_CLOSE:
        {
            oc_event event = { 0 };
            event.window = oc_window_handle_from_ptr(mpWindow);
            event.type = OC_EVENT_WINDOW_CLOSE;
            oc_queue_event(&event);
        }
        break;

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
        }
        break;

        //TODO: enter/exit size & move
        case WM_WINDOWPOSCHANGED:
        {
            oc_win32_update_child_layers(mpWindow);
            result = DefWindowProc(windowHandle, message, wParam, lParam);
        }
        break;

        case WM_SIZING:
        case WM_MOVING:
        {
            RECT* rect = (RECT*)lParam;

            oc_event event = { 0 };
            event.type = message == WM_SIZING ? OC_EVENT_WINDOW_RESIZE : OC_EVENT_WINDOW_MOVE;
            event.window = oc_window_handle_from_ptr(mpWindow);

            event.move.frame = oc_window_get_frame_rect(event.window);
            event.move.content = oc_window_get_content_rect(event.window);

            oc_queue_event(&event);

            oc_win32_update_child_layers(mpWindow);
        }
        break;

        case WM_SETFOCUS:
        {
            oc_event event = { 0 };
            event.window = oc_window_handle_from_ptr(mpWindow);
            event.type = OC_EVENT_WINDOW_FOCUS;
            oc_queue_event(&event);
        }
        break;

        case WM_KILLFOCUS:
        {
            oc_event event = { 0 };
            event.window = oc_window_handle_from_ptr(mpWindow);
            event.type = OC_EVENT_WINDOW_UNFOCUS;
            oc_queue_event(&event);
        }
        break;

        case WM_SIZE:
        {
            bool minimized = (wParam == SIZE_MINIMIZED);
            if(minimized != mpWindow->minimized)
            {
                mpWindow->minimized = minimized;

                oc_event event = { 0 };
                event.window = oc_window_handle_from_ptr(mpWindow);

                if(minimized)
                {
                    event.type = OC_EVENT_WINDOW_HIDE;
                }
                else if(mpWindow->minimized)
                {
                    event.type = OC_EVENT_WINDOW_SHOW;
                }
                oc_queue_event(&event);
            }
        }
        break;

        case WM_LBUTTONDOWN:
        {
            oc_win32_process_mouse_event(mpWindow, OC_KEY_PRESS, OC_MOUSE_LEFT);
        }
        break;

        case WM_RBUTTONDOWN:
        {
            oc_win32_process_mouse_event(mpWindow, OC_KEY_PRESS, OC_MOUSE_RIGHT);
        }
        break;

        case WM_MBUTTONDOWN:
        {
            oc_win32_process_mouse_event(mpWindow, OC_KEY_PRESS, OC_MOUSE_MIDDLE);
        }
        break;

        case WM_LBUTTONUP:
        {
            oc_win32_process_mouse_event(mpWindow, OC_KEY_RELEASE, OC_MOUSE_LEFT);
        }
        break;

        case WM_RBUTTONUP:
        {
            oc_win32_process_mouse_event(mpWindow, OC_KEY_RELEASE, OC_MOUSE_RIGHT);
        }
        break;

        case WM_MBUTTONUP:
        {
            oc_win32_process_mouse_event(mpWindow, OC_KEY_RELEASE, OC_MOUSE_MIDDLE);
        }
        break;

        case WM_MOUSEMOVE:
        {
            RECT rect;
            GetClientRect(mpWindow->win32.hWnd, &rect);

            u32 dpi = GetDpiForWindow(mpWindow->win32.hWnd);
            f32 scaling = (f32)dpi / 96.;

            oc_event event = { 0 };
            event.window = oc_window_handle_from_ptr(mpWindow);
            event.type = OC_EVENT_MOUSE_MOVE;
            event.mouse.x = LOWORD(lParam) / scaling;
            event.mouse.y = HIWORD(lParam) / scaling;

            if(oc_appData.win32.mouseTracked || oc_appData.win32.mouseCaptureMask)
            {
                event.mouse.deltaX = event.mouse.x - oc_appData.win32.lastMousePos.x;
                event.mouse.deltaY = event.mouse.y - oc_appData.win32.lastMousePos.y;
            }
            oc_appData.win32.lastMousePos = (oc_vec2){ event.mouse.x, event.mouse.y };

            if(!oc_appData.win32.mouseTracked)
            {
                oc_appData.win32.mouseTracked = true;

                TRACKMOUSEEVENT track;
                memset(&track, 0, sizeof(track));
                track.cbSize = sizeof(track);
                track.dwFlags = TME_LEAVE;
                track.hwndTrack = mpWindow->win32.hWnd;
                TrackMouseEvent(&track);

                oc_event enter = { .window = event.window,
                                   .type = OC_EVENT_MOUSE_ENTER,
                                   .mouse.x = event.mouse.x,
                                   .mouse.y = event.mouse.y };
                oc_queue_event(&enter);
            }

            oc_queue_event(&event);
        }
        break;

        case WM_MOUSELEAVE:
        {
            oc_appData.win32.mouseTracked = false;

            oc_event event = { 0 };
            event.window = oc_window_handle_from_ptr(mpWindow);
            event.type = OC_EVENT_MOUSE_LEAVE;
            oc_queue_event(&event);
        }
        break;

        case WM_MOUSEWHEEL:
        {
            oc_win32_process_wheel_event(mpWindow, 0, (float)((i16)HIWORD(wParam)));
        }
        break;

        case WM_MOUSEHWHEEL:
        {
            oc_win32_process_wheel_event(mpWindow, (float)((i16)HIWORD(wParam)), 0);
        }
        break;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            // Need to pass these through to the normal event handler to handle system shortcuts like Alt+F4.
            // Same for WM_SYSKEYUP
            if(message == WM_SYSKEYDOWN)
            {
                handled = false;
            }

            oc_event event = { 0 };
            event.window = oc_window_handle_from_ptr(mpWindow);
            event.type = OC_EVENT_KEYBOARD_KEY;
            event.key.action = (lParam & 0x40000000) ? OC_KEY_REPEAT : OC_KEY_PRESS;
            event.key.code = oc_convert_win32_key(HIWORD(lParam) & 0x1ff);
            event.key.mods = oc_get_mod_keys();
            oc_queue_event(&event);
        }
        break;

        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            if(message == WM_SYSKEYUP)
            {
                handled = false;
            }

            oc_event event = { 0 };
            event.window = oc_window_handle_from_ptr(mpWindow);
            event.type = OC_EVENT_KEYBOARD_KEY;
            event.key.action = OC_KEY_RELEASE;
            event.key.code = oc_convert_win32_key(HIWORD(lParam) & 0x1ff);
            event.key.mods = oc_get_mod_keys();
            oc_queue_event(&event);
        }
        break;

        case WM_CHAR:
        {
            if((u32)wParam >= 0x20 && (u32)wParam <= 0x7e)
            {
                oc_event event = { 0 };
                event.window = oc_window_handle_from_ptr(mpWindow);
                event.type = OC_EVENT_KEYBOARD_CHAR;
                event.character.codepoint = (oc_utf32)wParam;
                oc_str8 seq = oc_utf8_encode(event.character.sequence, event.character.codepoint);
                event.character.seqLen = seq.len;
                oc_queue_event(&event);
            }
        }
        break;

        case WM_SETTINGCHANGE:
        {
            if((u32)wParam == SPI_SETWHEELSCROLLLINES)
            {
                u32 wheelScrollLines;
                if(SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &wheelScrollLines, 0) != 0)
                {
                    oc_appData.win32.wheelScrollLines = wheelScrollLines;
                }
            }
        }
        break;

        case WM_DROPFILES:
        {
            //TODO
        }
        break;

        case OC_WM_USER_DISPATCH_PROC:
        {
            oc_dispatch_proc proc = (oc_dispatch_proc)wParam;
            void* user = (void*)lParam;
            result = proc(user);
        }
        break;

        default:
        {
            handled = false;
        }
        break;
    }

    if(handled == false)
    {
        result = DefWindowProc(windowHandle, message, wParam, lParam);
    }

    return (result);
}

//--------------------------------------------------------------------
// app management
//--------------------------------------------------------------------

bool oc_should_quit()
{
    return (oc_appData.shouldQuit);
}

void oc_cancel_quit()
{
    oc_appData.shouldQuit = false;
}

void oc_request_quit()
{
    oc_appData.shouldQuit = true;
}

void oc_pump_events(f64 timeout)
{
    MSG message;

    if(timeout < 0)
    {
        WaitMessage();
    }
    else if(timeout > 0)
    {
        MsgWaitForMultipleObjects(0, NULL, FALSE, (DWORD)(timeout * 1e3), QS_ALLEVENTS);
    }

    while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

i32 oc_dispatch_on_main_thread_sync(oc_window main_window, oc_dispatch_proc proc, void* user)
{
    oc_window_data* window_data = oc_window_ptr_from_handle(main_window);
    OC_DEBUG_ASSERT(window_data != NULL);

    LRESULT result = SendMessage(window_data->win32.hWnd, OC_WM_USER_DISPATCH_PROC, (WPARAM)proc, (LPARAM)user);
    return result;
}

//--------------------------------------------------------------------
// window management
//--------------------------------------------------------------------

//WARN: the following header pulls in objbase.h (even with WIN32_LEAN_AND_MEAN), which
//      #defines interface to struct... so make sure to #undef interface since it's a
//      name we want to be able to use throughout the codebase
#include <ShellScalingApi.h>
#undef interface

oc_window oc_window_create(oc_rect rect, oc_str8 title, oc_window_style style)
{
    WNDCLASS windowClass = { .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
                             .lpfnWndProc = oc_win32_win_proc,
                             .hInstance = GetModuleHandleW(NULL),
                             .lpszClassName = "ApplicationWindowClass",
                             .hCursor = LoadCursor(0, IDC_ARROW) };

    if(!RegisterClass(&windowClass))
    {
        //TODO: error
        goto quit;
    }

    u32 dpiX, dpiY;
    HMONITOR monitor = MonitorFromPoint((POINT){ rect.x, rect.y }, MONITOR_DEFAULTTOPRIMARY);
    GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);

    f32 scaleX = (f32)dpiX / 96.;
    f32 scaleY = (f32)dpiY / 96.;

    RECT frame = {
        rect.x * scaleX,
        rect.y * scaleY,
        (rect.x + rect.w) * scaleX,
        (rect.y + rect.h) * scaleY
    };

    DWORD winStyle = WS_OVERLAPPEDWINDOW;
    AdjustWindowRect(&frame, winStyle, FALSE);

    oc_arena_scope scratch = oc_scratch_begin();
    const char* titleCString = oc_str8_to_cstring(scratch.arena, title);

    HWND windowHandle = CreateWindow("ApplicationWindowClass", titleCString,
                                     winStyle,
                                     frame.left, frame.top,
                                     frame.right - frame.left,
                                     frame.bottom - frame.top,
                                     0, 0, windowClass.hInstance, 0);

    oc_scratch_end(scratch);

    if(!windowHandle)
    {
        //TODO: error
        goto quit;
    }

    UpdateWindow(windowHandle);

//TODO: return wrapped window
quit:;
    oc_window_data* window = oc_window_alloc();
    window->win32.hWnd = windowHandle;
    window->win32.layers = (oc_list){ 0 };

    SetPropW(windowHandle, L"MilePost", window);

    return (oc_window_handle_from_ptr(window));
}

void oc_window_destroy(oc_window window)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        DestroyWindow(windowData->win32.hWnd);
        //TODO: check when to unregister class

        oc_window_recycle_ptr(windowData);
    }
}

void* oc_window_native_pointer(oc_window window)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        return (windowData->win32.hWnd);
    }
    else
    {
        return (0);
    }
}

bool oc_window_should_close(oc_window window)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        return (windowData->shouldClose);
    }
    else
    {
        return (false);
    }
}

void oc_window_request_close(oc_window window)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        windowData->shouldClose = true;
        PostMessage(windowData->win32.hWnd, WM_CLOSE, 0, 0);
    }
}

void oc_window_cancel_close(oc_window window)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        windowData->shouldClose = false;
    }
}

bool oc_window_is_hidden(oc_window window)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        return (IsWindowVisible(windowData->win32.hWnd));
    }
    else
    {
        return (false);
    }
}

void oc_window_hide(oc_window window)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        ShowWindow(windowData->win32.hWnd, SW_HIDE);
    }
}

void oc_window_show(oc_window window)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        ShowWindow(windowData->win32.hWnd, SW_NORMAL);
    }
}

void oc_window_set_title(oc_window window, oc_str8 title)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        oc_arena_scope scratch = oc_scratch_begin();
        const char* titleCString = oc_str8_to_cstring(scratch.arena, title);

        SetWindowText(windowData->win32.hWnd, titleCString);

        oc_scratch_end(scratch);
    }
}

bool oc_window_is_minimized(oc_window window)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        return (windowData->minimized);
    }
    else
    {
        return (false);
    }
}

void oc_window_minimize(oc_window window)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        ShowWindow(windowData->win32.hWnd, SW_MINIMIZE);
    }
}

void oc_window_maximize(oc_window window)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        ShowWindow(windowData->win32.hWnd, SW_MAXIMIZE);
    }
}

void oc_window_restore(oc_window window)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        ShowWindow(windowData->win32.hWnd, SW_RESTORE);
    }
}

bool oc_window_has_focus(oc_window window)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        return (GetActiveWindow() == windowData->win32.hWnd);
    }
    else
    {
        return (false);
    }
}

void oc_window_focus(oc_window window)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        SetFocus(windowData->win32.hWnd);
    }
}

void oc_window_unfocus(oc_window window)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        SetFocus(0);
    }
}

void oc_window_send_to_back(oc_window window)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        SetWindowPos(windowData->win32.hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
}

void oc_window_bring_to_front(oc_window window)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        if(!IsWindowVisible(windowData->win32.hWnd))
        {
            ShowWindow(windowData->win32.hWnd, SW_NORMAL);
        }
        SetWindowPos(windowData->win32.hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
}

oc_rect oc_window_get_frame_rect(oc_window window)
{
    oc_rect rect = { 0 };
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        //NOTE: GetWindowRect() includes the drop shadow, which we don't want, so we call
        //      DwmGetWindowAttribute() instead.
        //      Note that contrary to what the GetWindowRect() docs suggests when mentionning
        //      this, DwmGetWindowAttributes() _does_ seem to adjust for DPI.
        u32 dpi = GetDpiForWindow(windowData->win32.hWnd);
        f32 scale = (float)dpi / 96.;

        RECT frame;
        HRESULT res = DwmGetWindowAttribute(windowData->win32.hWnd,
                                            DWMWA_EXTENDED_FRAME_BOUNDS,
                                            &frame,
                                            sizeof(RECT));
        if(res == S_OK)
        {
            rect = (oc_rect){
                frame.left / scale,
                frame.top / scale,
                (frame.right - frame.left) / scale,
                (frame.bottom - frame.top) / scale
            };
        }
    }
    return (rect);
}

static oc_rect oc_win32_get_drop_shadow_offsets(HWND hWnd)
{
    RECT frameIncludingShadow;
    RECT frameExcludingShadow;

    GetWindowRect(hWnd, &frameIncludingShadow);
    DwmGetWindowAttribute(hWnd,
                          DWMWA_EXTENDED_FRAME_BOUNDS,
                          &frameExcludingShadow,
                          sizeof(RECT));

    oc_rect extents = {
        .x = frameIncludingShadow.left - frameExcludingShadow.left,
        .y = frameIncludingShadow.top - frameExcludingShadow.top,
        .w = frameIncludingShadow.right - frameExcludingShadow.right,
        .h = frameIncludingShadow.bottom - frameExcludingShadow.bottom
    };

    return (extents);
}

void oc_window_set_frame_rect(oc_window window, oc_rect rect)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        u32 dpi = GetDpiForWindow(windowData->win32.hWnd);
        f32 scale = (float)dpi / 96.;

        //NOTE compute the size of the drop shadow to add it in setwindowpos
        oc_rect shadowOffsets = oc_win32_get_drop_shadow_offsets(windowData->win32.hWnd);

        RECT frame = {
            rect.x * scale + shadowOffsets.x,
            rect.y * scale + shadowOffsets.y,
            (rect.x + rect.w) * scale + shadowOffsets.w,
            (rect.y + rect.h) * scale + shadowOffsets.h
        };

        SetWindowPos(windowData->win32.hWnd,
                     HWND_TOP,
                     frame.left,
                     frame.top,
                     frame.right - frame.left,
                     frame.bottom - frame.top,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

oc_rect oc_window_get_content_rect(oc_window window)
{
    oc_rect rect = { 0 };
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        RECT client;
        if(GetClientRect(windowData->win32.hWnd, &client))
        {
            u32 dpi = GetDpiForWindow(windowData->win32.hWnd);
            f32 scale = (float)dpi / 96.;

            POINT origin = { 0, 0 };
            ClientToScreen(windowData->win32.hWnd, &origin);

            rect = (oc_rect){
                origin.x / scale,
                origin.y / scale,
                (client.right - client.left) / scale,
                (client.bottom - client.top) / scale
            };
        }
    }
    return (rect);
}

void oc_window_set_content_rect(oc_window window, oc_rect rect)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        u32 dpi = GetDpiForWindow(windowData->win32.hWnd);
        f32 scale = (float)dpi / 96.;

        RECT frame = {
            rect.x * scale,
            rect.y * scale,
            (rect.x + rect.w) * scale,
            (rect.y + rect.h) * scale
        };

        DWORD style = GetWindowLong(windowData->win32.hWnd, GWL_STYLE);
        BOOL menu = (GetMenu(windowData->win32.hWnd) != NULL);
        AdjustWindowRect(&frame, style, menu);

        SetWindowPos(windowData->win32.hWnd,
                     HWND_TOP,
                     frame.left,
                     frame.top,
                     frame.right - frame.left,
                     frame.bottom - frame.top,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

void oc_window_center(oc_window window)
{
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {

        oc_rect frame = oc_window_get_frame_rect(window);

        HMONITOR monitor = MonitorFromWindow(windowData->win32.hWnd, MONITOR_DEFAULTTOPRIMARY);
        if(monitor)
        {
            MONITORINFO monitorInfo = { .cbSize = sizeof(MONITORINFO) };
            GetMonitorInfoW(monitor, &monitorInfo);

            int dpiX, dpiY;
            GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
            f32 scaleX = dpiX / 96.;
            f32 scaleY = dpiY / 96.;

            f32 monX = monitorInfo.rcWork.left / scaleX;
            f32 monY = monitorInfo.rcWork.top / scaleY;
            f32 monW = (monitorInfo.rcWork.right - monitorInfo.rcWork.left) / scaleX;
            f32 monH = (monitorInfo.rcWork.bottom - monitorInfo.rcWork.top) / scaleY;

            frame.x = monX + 0.5 * (monW - frame.w);
            frame.y = monY + 0.5 * (monH - frame.h);

            oc_window_set_frame_rect(window, frame);
        }
    }
}

//--------------------------------------------------------------------------------
// clipboard functions
//--------------------------------------------------------------------------------

void oc_clipboard_clear(void)
{
    if(OpenClipboard(NULL))
    {
        EmptyClipboard();
        CloseClipboard();
    }
}

void oc_clipboard_set_string(oc_str8 string)
{
    if(OpenClipboard(NULL))
    {
        EmptyClipboard();

        int wideCount = MultiByteToWideChar(CP_UTF8, 0, string.ptr, string.len, 0, 0);
        HANDLE handle = GlobalAlloc(GMEM_MOVEABLE, (wideCount + 1) * sizeof(wchar_t));
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

oc_str8 oc_clipboard_get_string(oc_arena* arena)
{
    oc_str8 string = { 0 };

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
                    string.ptr = oc_arena_push(arena, size);
                    string.len = size - 1;
                    WideCharToMultiByte(CP_UTF8, 0, (wchar_t*)memory, -1, string.ptr, size, 0, 0);
                    GlobalUnlock(handle);
                }
            }
        }
        CloseClipboard();
    }
    return (string);
}

oc_str8 oc_clipboard_copy_string(oc_str8 backing)
{
    //TODO
    return ((oc_str8){ 0 });
}

//--------------------------------------------------------------------------------
// win32 surfaces
//--------------------------------------------------------------------------------

#include "graphics/graphics_surface.h"

oc_vec2 oc_win32_surface_contents_scaling(oc_surface_data* surface)
{
    u32 dpi = GetDpiForWindow(surface->layer.hWnd);
    oc_vec2 contentsScaling = (oc_vec2){ (float)dpi / 96., (float)dpi / 96. };
    return (contentsScaling);
}

oc_vec2 oc_win32_surface_get_size(oc_surface_data* surface)
{
    oc_vec2 size = { 0 };
    RECT rect;
    if(GetClientRect(surface->layer.hWnd, &rect))
    {
        u32 dpi = GetDpiForWindow(surface->layer.hWnd);
        f32 scale = (float)dpi / 96.;
        size = (oc_vec2){ (rect.right - rect.left) / scale, (rect.bottom - rect.top) / scale };
    }
    return (size);
}

bool oc_win32_surface_get_hidden(oc_surface_data* surface)
{
    bool hidden = !IsWindowVisible(surface->layer.hWnd);
    return (hidden);
}

void oc_win32_surface_set_hidden(oc_surface_data* surface, bool hidden)
{
    ShowWindow(surface->layer.hWnd, hidden ? SW_HIDE : SW_NORMAL);
}

void oc_win32_surface_bring_to_front(oc_surface_data* surface)
{
	oc_list_remove(&surface->layer.parent->win32.layers, &surface->layer.listElt);
	oc_list_push(&surface->layer.parent->win32.layers, &surface->layer.listElt);
	oc_win32_update_child_layers_zorder(surface->layer.parent);
}

void oc_win32_surface_send_to_back(oc_surface_data* surface)
{
	oc_list_remove(&surface->layer.parent->win32.layers, &surface->layer.listElt);
	oc_list_push_back(&surface->layer.parent->win32.layers, &surface->layer.listElt);
	oc_win32_update_child_layers_zorder(surface->layer.parent);
}

void* oc_win32_surface_native_layer(oc_surface_data* surface)
{
    return ((void*)surface->layer.hWnd);
}

oc_surface_id oc_win32_surface_remote_id(oc_surface_data* surface)
{
    return ((oc_surface_id)surface->layer.hWnd);
}

void oc_win32_surface_host_connect(oc_surface_data* surface, oc_surface_id remoteID)
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

void oc_surface_cleanup(oc_surface_data* surface)
{
    oc_list_remove(&surface->layer.parent->win32.layers, &surface->layer.listElt);
    DestroyWindow(surface->layer.hWnd);
}

LRESULT LayerWinProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    if(message == WM_NCHITTEST)
    {
        return (HTTRANSPARENT);
    }
    else
    {
        return (DefWindowProc(windowHandle, message, wParam, lParam));
    }
}

void oc_surface_init_for_window(oc_surface_data* surface, oc_window_data* window)
{
    surface->contentsScaling = oc_win32_surface_contents_scaling;
    surface->getSize = oc_win32_surface_get_size;
    surface->getHidden = oc_win32_surface_get_hidden;
    surface->setHidden = oc_win32_surface_set_hidden;
    surface->nativeLayer = oc_win32_surface_native_layer;
    surface->bringToFront = oc_win32_surface_bring_to_front;
    surface->sendToBack = oc_win32_surface_send_to_back;

    //NOTE(martin): create a child window for the surface
    WNDCLASS layerWindowClass = { .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
                                  .lpfnWndProc = LayerWinProc,
                                  .hInstance = GetModuleHandleW(NULL),
                                  .lpszClassName = "layer_window_class",
                                  .hCursor = LoadCursor(0, IDC_ARROW) };

    RegisterClass(&layerWindowClass);

    RECT parentRect;
    GetClientRect(window->win32.hWnd, &parentRect);
    POINT point = { 0 };
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

    DWM_BLURBEHIND bb = { 0 };
    bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
    bb.hRgnBlur = region;
    bb.fEnable = TRUE;

    HRESULT res = DwmEnableBlurBehindWindow(surface->layer.hWnd, &bb);

    DeleteObject(region);
    if(res != S_OK)
    {
        oc_log_error("couldn't enable blur behind\n");
    }

    //NOTE(reuben): Creating the child window takes focus away from the main window, but we want to keep
    //the focus on the main window
    SetFocus(window->win32.hWnd);

    surface->layer.parent = window;
    oc_list_append(&window->win32.layers, &surface->layer.listElt);
}

void oc_surface_init_remote(oc_surface_data* surface, u32 width, u32 height)
{
    surface->contentsScaling = oc_win32_surface_contents_scaling;
    surface->getSize = oc_win32_surface_get_size;
    surface->getHidden = oc_win32_surface_get_hidden;
    surface->setHidden = oc_win32_surface_set_hidden;
    surface->nativeLayer = oc_win32_surface_native_layer;
    surface->remoteID = oc_win32_surface_remote_id;

    WNDCLASS layerWindowClass = { .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
                                  .lpfnWndProc = DefWindowProc,
                                  .hInstance = GetModuleHandleW(NULL),
                                  .lpszClassName = "server_layer_window_class",
                                  .hCursor = LoadCursor(0, IDC_ARROW) };

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

oc_surface_data* oc_win32_surface_create_host(oc_window window)
{
    oc_surface_data* surface = 0;
    oc_window_data* windowData = oc_window_ptr_from_handle(window);
    if(windowData)
    {
        surface = oc_malloc_type(oc_surface_data);
        if(surface)
        {
            memset(surface, 0, sizeof(oc_surface_data));
            oc_surface_init_for_window(surface, windowData);

            surface->api = OC_HOST;
            surface->hostConnect = oc_win32_surface_host_connect;
        }
    }
    return (surface);
}

//--------------------------------------------------------------------
// native open/save/alert windows
//--------------------------------------------------------------------

//TODO: GetOpenFileName() doesn't seem to support selecting folders, and
//      requires filters which pair a "descriptive" name with an extension

#define interface struct
#include <shobjidl.h>
#include <shtypes.h>
#undef interface

oc_str8 oc_open_dialog(oc_arena* arena,
                       oc_str8 title,
                       oc_str8 defaultPath,
                       oc_str8_list filters,
                       bool directory)
{
    oc_str8 res = { 0 };
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

            if(filters.eltCount)
            {
                oc_arena_scope tmp = oc_arena_scope_begin(arena);
                COMDLG_FILTERSPEC* filterSpecs = oc_arena_push_array(arena, COMDLG_FILTERSPEC, filters.eltCount);

                int i = 0;
                oc_list_for(&filters.list, elt, oc_str8_elt, listElt)
                {
                    oc_str8_list list = { 0 };
                    oc_str8_list_push(arena, &list, OC_STR8("*."));
                    oc_str8_list_push(arena, &list, elt->string);
                    oc_str8 filter = oc_str8_list_join(arena, list);

                    int filterWideSize = 1 + MultiByteToWideChar(CP_UTF8, 0, filter.ptr, filter.len, NULL, 0);
                    filterSpecs[i].pszSpec = oc_arena_push_array(arena, wchar_t, filterWideSize);
                    MultiByteToWideChar(CP_UTF8, 0, filter.ptr, filter.len, (LPWSTR)filterSpecs[i].pszSpec, filterWideSize);
                    ((LPWSTR)(filterSpecs[i].pszSpec))[filterWideSize - 1] = 0;

                    filterSpecs[i].pszName = filterSpecs[i].pszSpec;
                    i++;
                }

                hr = dialog->lpVtbl->SetFileTypes(dialog, i, filterSpecs);

                oc_arena_scope_end(tmp);
            }

            if(defaultPath.len)
            {
                oc_arena_scope tmp = oc_arena_scope_begin(arena);
                int pathWideSize = MultiByteToWideChar(CP_UTF8, 0, defaultPath.ptr, defaultPath.len, NULL, 0);
                LPWSTR pathWide = oc_arena_push_array(arena, wchar_t, pathWideSize + 1);
                MultiByteToWideChar(CP_UTF8, 0, defaultPath.ptr, defaultPath.len, pathWide, pathWideSize);
                pathWide[pathWideSize] = '\0';

                IShellItem* item = 0;
                hr = SHCreateItemFromParsingName(pathWide, NULL, &IID_IShellItem, (void**)&item);
                if(SUCCEEDED(hr))
                {
                    hr = dialog->lpVtbl->SetFolder(dialog, item);
                    item->lpVtbl->Release(item);
                }
                oc_arena_scope_end(tmp);
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
                        int oc_utf8Size = WideCharToMultiByte(CP_UTF8, 0, filePath, -1, NULL, 0, NULL, NULL);
                        if(oc_utf8Size > 0)
                        {
                            res.ptr = oc_arena_push(arena, oc_utf8Size);
                            res.len = oc_utf8Size - 1;
                            WideCharToMultiByte(CP_UTF8, 0, filePath, -1, res.ptr, oc_utf8Size, NULL, NULL);
                        }
                        CoTaskMemFree(filePath);
                    }
                    item->lpVtbl->Release(item);
                }
            }
        }
    }
    CoUninitialize();
    return (res);
}

oc_str8 oc_save_dialog(oc_arena* arena,
                       oc_str8 title,
                       oc_str8 defaultPath,
                       oc_str8_list filters)
{
    oc_str8 res = { 0 };
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if(SUCCEEDED(hr))
    {
        IFileOpenDialog* dialog = 0;
        hr = CoCreateInstance(&CLSID_FileSaveDialog, NULL, CLSCTX_ALL, &IID_IFileSaveDialog, (void**)&dialog);
        if(SUCCEEDED(hr))
        {
            if(filters.eltCount)
            {
                oc_arena_scope tmp = oc_arena_scope_begin(arena);
                COMDLG_FILTERSPEC* filterSpecs = oc_arena_push_array(arena, COMDLG_FILTERSPEC, filters.eltCount);

                int i = 0;
                oc_list_for(&filters.list, elt, oc_str8_elt, listElt)
                {
                    oc_str8_list list = { 0 };
                    oc_str8_list_push(arena, &list, OC_STR8("*."));
                    oc_str8_list_push(arena, &list, elt->string);
                    oc_str8 filter = oc_str8_list_join(arena, list);

                    int filterWideSize = 1 + MultiByteToWideChar(CP_UTF8, 0, filter.ptr, filter.len, NULL, 0);
                    filterSpecs[i].pszSpec = oc_arena_push_array(arena, wchar_t, filterWideSize);
                    MultiByteToWideChar(CP_UTF8, 0, filter.ptr, filter.len, (LPWSTR)filterSpecs[i].pszSpec, filterWideSize);
                    ((LPWSTR)(filterSpecs[i].pszSpec))[filterWideSize - 1] = 0;

                    filterSpecs[i].pszName = filterSpecs[i].pszSpec;
                    i++;
                }

                hr = dialog->lpVtbl->SetFileTypes(dialog, i, filterSpecs);

                oc_arena_scope_end(tmp);
            }

            if(defaultPath.len)
            {
                oc_arena_scope tmp = oc_arena_scope_begin(arena);
                int pathWideSize = MultiByteToWideChar(CP_UTF8, 0, defaultPath.ptr, defaultPath.len, NULL, 0);
                LPWSTR pathWide = oc_arena_push_array(arena, wchar_t, pathWideSize + 1);
                MultiByteToWideChar(CP_UTF8, 0, defaultPath.ptr, defaultPath.len, pathWide, pathWideSize);
                pathWide[pathWideSize] = '\0';

                IShellItem* item = 0;
                hr = SHCreateItemFromParsingName(pathWide, NULL, &IID_IShellItem, (void**)&item);
                if(SUCCEEDED(hr))
                {
                    hr = dialog->lpVtbl->SetFolder(dialog, item);
                    item->lpVtbl->Release(item);
                }
                oc_arena_scope_end(tmp);
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
                        int oc_utf8Size = WideCharToMultiByte(CP_UTF8, 0, filePath, -1, NULL, 0, NULL, NULL);
                        if(oc_utf8Size > 0)
                        {
                            res.ptr = oc_arena_push(arena, oc_utf8Size);
                            res.len = oc_utf8Size - 1;
                            WideCharToMultiByte(CP_UTF8, 0, filePath, -1, res.ptr, oc_utf8Size, NULL, NULL);
                        }
                        CoTaskMemFree(filePath);
                    }
                    item->lpVtbl->Release(item);
                }
            }
        }
    }
    CoUninitialize();
    return (res);
}

#include <commctrl.h>

int oc_alert_popup(oc_str8 title,
                   oc_str8 message,
                   oc_str8_list options)
{
    oc_arena_scope scratch = oc_scratch_begin();
    TASKDIALOG_BUTTON* buttons = oc_arena_push_array(scratch.arena, TASKDIALOG_BUTTON, options.eltCount);

    int i = 0;
    oc_list_for(&options.list, elt, oc_str8_elt, listElt)
    {
        int textWideSize = MultiByteToWideChar(CP_UTF8, 0, elt->string.ptr, elt->string.len, NULL, 0);
        wchar_t* textWide = oc_arena_push_array(scratch.arena, wchar_t, textWideSize + 1);
        MultiByteToWideChar(CP_UTF8, 0, elt->string.ptr, elt->string.len, textWide, textWideSize);
        textWide[textWideSize] = '\0';

        buttons[i].nButtonID = i + 1;
        buttons[i].pszButtonText = textWide;

        i++;
    }

    int titleWideSize = MultiByteToWideChar(CP_UTF8, 0, title.ptr, title.len, NULL, 0);
    wchar_t* titleWide = oc_arena_push_array(scratch.arena, wchar_t, titleWideSize + 1);
    MultiByteToWideChar(CP_UTF8, 0, title.ptr, title.len, titleWide, titleWideSize);
    titleWide[titleWideSize] = '\0';

    int messageWideSize = MultiByteToWideChar(CP_UTF8, 0, message.ptr, message.len, NULL, 0);
    wchar_t* messageWide = oc_arena_push_array(scratch.arena, wchar_t, messageWideSize + 1);
    MultiByteToWideChar(CP_UTF8, 0, message.ptr, message.len, messageWide, messageWideSize);
    messageWide[messageWideSize] = '\0';

    TASKDIALOGCONFIG config = {
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
        .cButtons = options.eltCount,
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

    oc_scratch_end(scratch);
    return (button);
}
