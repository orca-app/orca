/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "app.c"
#include "platform/platform_thread.h"
#include "graphics/graphics.h"
#include <dwmapi.h>

void oc_init_keys()
{
    memset(oc_appData.scanCodes, OC_SCANCODE_UNKNOWN, 256 * sizeof(int));

    oc_appData.scanCodes[0x00B] = OC_SCANCODE_0;
    oc_appData.scanCodes[0x002] = OC_SCANCODE_1;
    oc_appData.scanCodes[0x003] = OC_SCANCODE_2;
    oc_appData.scanCodes[0x004] = OC_SCANCODE_3;
    oc_appData.scanCodes[0x005] = OC_SCANCODE_4;
    oc_appData.scanCodes[0x006] = OC_SCANCODE_5;
    oc_appData.scanCodes[0x007] = OC_SCANCODE_6;
    oc_appData.scanCodes[0x008] = OC_SCANCODE_7;
    oc_appData.scanCodes[0x009] = OC_SCANCODE_8;
    oc_appData.scanCodes[0x00A] = OC_SCANCODE_9;
    oc_appData.scanCodes[0x01E] = OC_SCANCODE_A;
    oc_appData.scanCodes[0x030] = OC_SCANCODE_B;
    oc_appData.scanCodes[0x02E] = OC_SCANCODE_C;
    oc_appData.scanCodes[0x020] = OC_SCANCODE_D;
    oc_appData.scanCodes[0x012] = OC_SCANCODE_E;
    oc_appData.scanCodes[0x021] = OC_SCANCODE_F;
    oc_appData.scanCodes[0x022] = OC_SCANCODE_G;
    oc_appData.scanCodes[0x023] = OC_SCANCODE_H;
    oc_appData.scanCodes[0x017] = OC_SCANCODE_I;
    oc_appData.scanCodes[0x024] = OC_SCANCODE_J;
    oc_appData.scanCodes[0x025] = OC_SCANCODE_K;
    oc_appData.scanCodes[0x026] = OC_SCANCODE_L;
    oc_appData.scanCodes[0x032] = OC_SCANCODE_M;
    oc_appData.scanCodes[0x031] = OC_SCANCODE_N;
    oc_appData.scanCodes[0x018] = OC_SCANCODE_O;
    oc_appData.scanCodes[0x019] = OC_SCANCODE_P;
    oc_appData.scanCodes[0x010] = OC_SCANCODE_Q;
    oc_appData.scanCodes[0x013] = OC_SCANCODE_R;
    oc_appData.scanCodes[0x01F] = OC_SCANCODE_S;
    oc_appData.scanCodes[0x014] = OC_SCANCODE_T;
    oc_appData.scanCodes[0x016] = OC_SCANCODE_U;
    oc_appData.scanCodes[0x02F] = OC_SCANCODE_V;
    oc_appData.scanCodes[0x011] = OC_SCANCODE_W;
    oc_appData.scanCodes[0x02D] = OC_SCANCODE_X;
    oc_appData.scanCodes[0x015] = OC_SCANCODE_Y;
    oc_appData.scanCodes[0x02C] = OC_SCANCODE_Z;
    oc_appData.scanCodes[0x028] = OC_SCANCODE_APOSTROPHE;
    oc_appData.scanCodes[0x02B] = OC_SCANCODE_BACKSLASH;
    oc_appData.scanCodes[0x033] = OC_SCANCODE_COMMA;
    oc_appData.scanCodes[0x00D] = OC_SCANCODE_EQUAL;
    oc_appData.scanCodes[0x029] = OC_SCANCODE_GRAVE_ACCENT;
    oc_appData.scanCodes[0x01A] = OC_SCANCODE_LEFT_BRACKET;
    oc_appData.scanCodes[0x00C] = OC_SCANCODE_MINUS;
    oc_appData.scanCodes[0x034] = OC_SCANCODE_PERIOD;
    oc_appData.scanCodes[0x01B] = OC_SCANCODE_RIGHT_BRACKET;
    oc_appData.scanCodes[0x027] = OC_SCANCODE_SEMICOLON;
    oc_appData.scanCodes[0x035] = OC_SCANCODE_SLASH;
    oc_appData.scanCodes[0x056] = OC_SCANCODE_WORLD_2;
    oc_appData.scanCodes[0x00E] = OC_SCANCODE_BACKSPACE;
    oc_appData.scanCodes[0x153] = OC_SCANCODE_DELETE;
    oc_appData.scanCodes[0x14F] = OC_SCANCODE_END;
    oc_appData.scanCodes[0x01C] = OC_SCANCODE_ENTER;
    oc_appData.scanCodes[0x001] = OC_SCANCODE_ESCAPE;
    oc_appData.scanCodes[0x147] = OC_SCANCODE_HOME;
    oc_appData.scanCodes[0x152] = OC_SCANCODE_INSERT;
    oc_appData.scanCodes[0x15D] = OC_SCANCODE_MENU;
    oc_appData.scanCodes[0x151] = OC_SCANCODE_PAGE_DOWN;
    oc_appData.scanCodes[0x149] = OC_SCANCODE_PAGE_UP;
    oc_appData.scanCodes[0x045] = OC_SCANCODE_PAUSE;
    oc_appData.scanCodes[0x146] = OC_SCANCODE_PAUSE;
    oc_appData.scanCodes[0x039] = OC_SCANCODE_SPACE;
    oc_appData.scanCodes[0x00F] = OC_SCANCODE_TAB;
    oc_appData.scanCodes[0x03A] = OC_SCANCODE_CAPS_LOCK;
    oc_appData.scanCodes[0x145] = OC_SCANCODE_NUM_LOCK;
    oc_appData.scanCodes[0x046] = OC_SCANCODE_SCROLL_LOCK;
    oc_appData.scanCodes[0x03B] = OC_SCANCODE_F1;
    oc_appData.scanCodes[0x03C] = OC_SCANCODE_F2;
    oc_appData.scanCodes[0x03D] = OC_SCANCODE_F3;
    oc_appData.scanCodes[0x03E] = OC_SCANCODE_F4;
    oc_appData.scanCodes[0x03F] = OC_SCANCODE_F5;
    oc_appData.scanCodes[0x040] = OC_SCANCODE_F6;
    oc_appData.scanCodes[0x041] = OC_SCANCODE_F7;
    oc_appData.scanCodes[0x042] = OC_SCANCODE_F8;
    oc_appData.scanCodes[0x043] = OC_SCANCODE_F9;
    oc_appData.scanCodes[0x044] = OC_SCANCODE_F10;
    oc_appData.scanCodes[0x057] = OC_SCANCODE_F11;
    oc_appData.scanCodes[0x058] = OC_SCANCODE_F12;
    oc_appData.scanCodes[0x064] = OC_SCANCODE_F13;
    oc_appData.scanCodes[0x065] = OC_SCANCODE_F14;
    oc_appData.scanCodes[0x066] = OC_SCANCODE_F15;
    oc_appData.scanCodes[0x067] = OC_SCANCODE_F16;
    oc_appData.scanCodes[0x068] = OC_SCANCODE_F17;
    oc_appData.scanCodes[0x069] = OC_SCANCODE_F18;
    oc_appData.scanCodes[0x06A] = OC_SCANCODE_F19;
    oc_appData.scanCodes[0x06B] = OC_SCANCODE_F20;
    oc_appData.scanCodes[0x06C] = OC_SCANCODE_F21;
    oc_appData.scanCodes[0x06D] = OC_SCANCODE_F22;
    oc_appData.scanCodes[0x06E] = OC_SCANCODE_F23;
    oc_appData.scanCodes[0x076] = OC_SCANCODE_F24;
    oc_appData.scanCodes[0x038] = OC_SCANCODE_LEFT_ALT;
    oc_appData.scanCodes[0x01D] = OC_SCANCODE_LEFT_CONTROL;
    oc_appData.scanCodes[0x02A] = OC_SCANCODE_LEFT_SHIFT;
    oc_appData.scanCodes[0x15B] = OC_SCANCODE_LEFT_SUPER;
    oc_appData.scanCodes[0x137] = OC_SCANCODE_PRINT_SCREEN;
    oc_appData.scanCodes[0x138] = OC_SCANCODE_RIGHT_ALT;
    oc_appData.scanCodes[0x11D] = OC_SCANCODE_RIGHT_CONTROL;
    oc_appData.scanCodes[0x036] = OC_SCANCODE_RIGHT_SHIFT;
    oc_appData.scanCodes[0x15C] = OC_SCANCODE_RIGHT_SUPER;
    oc_appData.scanCodes[0x150] = OC_SCANCODE_DOWN;
    oc_appData.scanCodes[0x14B] = OC_SCANCODE_LEFT;
    oc_appData.scanCodes[0x14D] = OC_SCANCODE_RIGHT;
    oc_appData.scanCodes[0x148] = OC_SCANCODE_UP;
    oc_appData.scanCodes[0x052] = OC_SCANCODE_KP_0;
    oc_appData.scanCodes[0x04F] = OC_SCANCODE_KP_1;
    oc_appData.scanCodes[0x050] = OC_SCANCODE_KP_2;
    oc_appData.scanCodes[0x051] = OC_SCANCODE_KP_3;
    oc_appData.scanCodes[0x04B] = OC_SCANCODE_KP_4;
    oc_appData.scanCodes[0x04C] = OC_SCANCODE_KP_5;
    oc_appData.scanCodes[0x04D] = OC_SCANCODE_KP_6;
    oc_appData.scanCodes[0x047] = OC_SCANCODE_KP_7;
    oc_appData.scanCodes[0x048] = OC_SCANCODE_KP_8;
    oc_appData.scanCodes[0x049] = OC_SCANCODE_KP_9;
    oc_appData.scanCodes[0x04E] = OC_SCANCODE_KP_ADD;
    oc_appData.scanCodes[0x053] = OC_SCANCODE_KP_DECIMAL;
    oc_appData.scanCodes[0x135] = OC_SCANCODE_KP_DIVIDE;
    oc_appData.scanCodes[0x11C] = OC_SCANCODE_KP_ENTER;
    oc_appData.scanCodes[0x037] = OC_SCANCODE_KP_MULTIPLY;
    oc_appData.scanCodes[0x04A] = OC_SCANCODE_KP_SUBTRACT;
}

void oc_win32_update_keyboard_layout()
{
    memcpy(oc_appData.keyMap, oc_defaultKeyMap, sizeof(oc_key_code) * OC_SCANCODE_COUNT);

    for(int winCode = 0; winCode < OC_SCANCODE_COUNT; winCode++)
    {
        oc_scan_code scanCode = oc_appData.scanCodes[winCode];

        if(scanCode < 256)
        {
            int vk = MapVirtualKey(winCode, MAPVK_VSC_TO_VK);
            if(vk)
            {
                int ch = MapVirtualKey(vk, MAPVK_VK_TO_CHAR);
                ch &= 0x7fff;
                if(ch)
                {
                    if(ch >= 'A' && ch <= 'Z')
                    {
                        oc_appData.keyMap[scanCode] = 'a' + (ch - 'A');
                    }
                    else
                    {
                        oc_appData.keyMap[scanCode] = ch;
                    }
                }
            }
        }
    }

    //NOTE fix digit row for azerty keyboards
    bool azerty = true;
    for(int scanCode = OC_SCANCODE_0; scanCode <= OC_SCANCODE_9; scanCode++)
    {
        if(oc_appData.keyMap[scanCode] >= OC_KEY_0 && oc_appData.keyMap[scanCode] <= OC_KEY_9)
        {
            azerty = false;
            break;
        }
    }
    if(azerty)
    {
        for(int scanCode = OC_SCANCODE_0; scanCode <= OC_SCANCODE_9; scanCode++)
        {
            oc_appData.keyMap[scanCode] = OC_KEY_0 + (scanCode - OC_SCANCODE_0);
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
        oc_win32_update_keyboard_layout();

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

        oc_appData.win32.mainThreadID = GetCurrentThreadId();

        oc_vsync_init();
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
    return (oc_appData.scanCodes[code]);
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

    if(action == OC_KEY_PRESS)
    {
        u32 clickTime = GetMessageTime();
        if(clickTime - oc_appData.win32.lastClickTime[button] > GetDoubleClickTime())
        {
            oc_appData.win32.clickCount[button] = 0;
        }
        for(int i = 0; i < OC_MOUSE_BUTTON_COUNT; i++)
        {
            if(i != button)
            {
                oc_appData.win32.clickCount[i] = 0;
            }
        }
        oc_appData.win32.lastClickTime[button] = clickTime;
        oc_appData.win32.clickCount[button]++;
    }
    else
    {
        u32 clickTime = GetMessageTime();
        if(clickTime - oc_appData.win32.lastClickTime[button] > GetDoubleClickTime())
        {
            oc_appData.win32.clickCount[button] = 0;
        }
    }

    oc_event event = { 0 };
    event.window = oc_window_handle_from_ptr(window);
    event.type = OC_EVENT_MOUSE_BUTTON;
    event.key.action = action;
    event.key.button = button;
    event.key.mods = oc_get_mod_keys();
    event.key.clickCount = oc_appData.win32.clickCount[button];

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

    oc_list_for(window->win32.layers, layer, oc_layer, listElt)
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

    oc_list_for(window->win32.layers, layer, oc_layer, listElt)
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
        case WM_ACTIVATE:
        {
            for(int i = 0; i < OC_MOUSE_BUTTON_COUNT; i++)
            {
                oc_appData.win32.clickCount[i] = 0;
            }
        }
        break;

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
            if(abs(event.mouse.x - oc_appData.win32.lastMousePos.x) > GetSystemMetrics(SM_CXDOUBLECLK) / 2
               || abs(event.mouse.y - oc_appData.win32.lastMousePos.y) > GetSystemMetrics(SM_CYDOUBLECLK) / 2)
            {
                for(int i = 0; i < OC_MOUSE_BUTTON_COUNT; i++)
                {
                    oc_appData.win32.clickCount[i] = 0;
                }
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

        case WM_INPUTLANGCHANGE:
        {
            oc_win32_update_keyboard_layout();
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
            event.key.scanCode = oc_convert_win32_key(HIWORD(lParam) & 0x1ff);
            event.key.keyCode = oc_scancode_to_keycode(event.key.scanCode);
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
            event.key.scanCode = oc_convert_win32_key(HIWORD(lParam) & 0x1ff);
            event.key.keyCode = oc_scancode_to_keycode(event.key.scanCode);
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
    PostThreadMessage(oc_appData.win32.mainThreadID, OC_WM_USER_WAKEUP, 0, 0);
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
                oc_list_for(filters.list, elt, oc_str8_elt, listElt)
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

    oc_win32_path_normalize_slash_in_place(res);

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
                oc_list_for(filters.list, elt, oc_str8_elt, listElt)
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

    oc_win32_path_normalize_slash_in_place(res);

    return (res);
}

#include "platform/platform_io_internal.h"

oc_str16 win32_path_from_handle_null_terminated(oc_arena* arena, HANDLE handle); // defined in win32_io.c

oc_file_dialog_result oc_file_dialog_for_table(oc_arena* arena, oc_file_dialog_desc* desc, oc_file_table* table)
{
    oc_arena_scope scratch = oc_scratch_begin_next(arena);
    oc_file_dialog_result result = { 0 };

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if(SUCCEEDED(hr))
    {
        IFileDialog* dialog = 0;

        if(desc->kind == OC_FILE_DIALOG_OPEN)
        {
            hr = CoCreateInstance(&CLSID_FileOpenDialog, NULL, CLSCTX_ALL, &IID_IFileOpenDialog, (void**)&dialog);
            if(SUCCEEDED(hr))
            {
                FILEOPENDIALOGOPTIONS opt;
                dialog->lpVtbl->GetOptions(dialog, &opt);

                //NOTE: OC_FILE_DIALOG_FILES is always implied, since IFileDialog offers no way to pick _only_ folders
                if(desc->flags & OC_FILE_DIALOG_DIRECTORIES)
                {
                    opt |= FOS_PICKFOLDERS;
                }
                else
                {
                    opt &= ~FOS_PICKFOLDERS;
                }

                if(desc->flags & OC_FILE_DIALOG_MULTIPLE)
                {
                    opt |= FOS_ALLOWMULTISELECT;
                }
                else
                {
                    opt &= ~FOS_ALLOWMULTISELECT;
                }

                dialog->lpVtbl->SetOptions(dialog, opt);
            }
        }
        else
        {
            hr = CoCreateInstance(&CLSID_FileSaveDialog, NULL, CLSCTX_ALL, &IID_IFileSaveDialog, (void**)&dialog);

            //NOTE: OC_FILE_DIALOG_CREATE_DIRECTORIES is implied, IFileSaveDialog offers no way of disabling it...
        }

        if(SUCCEEDED(hr))
        {
            //NOTE set title
            if(desc->title.len)
            {
                oc_str16 titleWide = oc_win32_utf8_to_wide(scratch.arena, desc->title);
                dialog->lpVtbl->SetTitle(dialog, (LPCWSTR)titleWide.ptr);
            }

            //NOTE set ok button
            if(desc->okLabel.len)
            {
                oc_str16 okLabelWide = oc_win32_utf8_to_wide(scratch.arena, desc->okLabel);
                dialog->lpVtbl->SetOkButtonLabel(dialog, (LPCWSTR)okLabelWide.ptr);
            }

            //NOTE: set starting path
            oc_str8 startPath = { 0 };
            {
                oc_str8_list list = { 0 };
                if(!oc_file_is_nil(desc->startAt))
                {
                    oc_file_slot* slot = oc_file_slot_from_handle(table, desc->startAt);
                    if(slot)
                    {
                        oc_str16 pathWide = win32_path_from_handle_null_terminated(scratch.arena, slot->fd);
                        oc_str8 path = oc_win32_wide_to_utf8(scratch.arena, pathWide);
                        //NOTE: remove potential \\?\ prefix which doesn't work with SHCreateItemFromParsingName()
                        if(!oc_str8_cmp(oc_str8_slice(path, 0, 4), OC_STR8("\\\\?\\")))
                        {
                            path = oc_str8_slice(path, 4, path.len);
                        }
                        oc_str8_list_push(scratch.arena, &list, path);
                    }
                }
                if(desc->startPath.len)
                {
                    oc_str8_list_push(scratch.arena, &list, desc->startPath);
                }
                startPath = oc_path_join(scratch.arena, list);
            }

            if(startPath.len)
            {
                oc_str16 pathWide = oc_win32_utf8_to_wide(scratch.arena, startPath);

                IShellItem* item = 0;
                hr = SHCreateItemFromParsingName((LPCWSTR)pathWide.ptr, NULL, &IID_IShellItem, (void**)&item);
                if(SUCCEEDED(hr))
                {
                    hr = dialog->lpVtbl->SetFolder(dialog, item);
                    item->lpVtbl->Release(item);
                }
            }

            //NOTE: set filters
            if(desc->filters.eltCount)
            {
                COMDLG_FILTERSPEC* filterSpecs = oc_arena_push_array(scratch.arena, COMDLG_FILTERSPEC, desc->filters.eltCount);

                int i = 0;
                oc_list_for(desc->filters.list, elt, oc_str8_elt, listElt)
                {
                    oc_str8_list list = { 0 };
                    oc_str8_list_push(scratch.arena, &list, OC_STR8("*."));
                    oc_str8_list_push(scratch.arena, &list, elt->string);
                    oc_str8 filter = oc_str8_list_join(scratch.arena, list);

                    int filterWideSize = 1 + MultiByteToWideChar(CP_UTF8, 0, filter.ptr, filter.len, NULL, 0);
                    filterSpecs[i].pszSpec = oc_arena_push_array(scratch.arena, wchar_t, filterWideSize);
                    MultiByteToWideChar(CP_UTF8, 0, filter.ptr, filter.len, (LPWSTR)filterSpecs[i].pszSpec, filterWideSize);
                    ((LPWSTR)(filterSpecs[i].pszSpec))[filterWideSize - 1] = 0;

                    filterSpecs[i].pszName = filterSpecs[i].pszSpec;
                    i++;
                }

                hr = dialog->lpVtbl->SetFileTypes(dialog, i, filterSpecs);
            }

            hr = dialog->lpVtbl->Show(dialog, NULL);

            if(SUCCEEDED(hr))
            {
                if(desc->kind == OC_FILE_DIALOG_OPEN && (desc->flags & OC_FILE_DIALOG_MULTIPLE))
                {
                    IShellItemArray* array = 0;
                    hr = ((IFileOpenDialog*)dialog)->lpVtbl->GetResults((IFileOpenDialog*)dialog, &array);
                    if(SUCCEEDED(hr))
                    {
                        int count = 0;
                        array->lpVtbl->GetCount(array, &count);
                        for(int itemIndex = 0; itemIndex < count; itemIndex++)
                        {
                            IShellItem* item = 0;
                            hr = array->lpVtbl->GetItemAt(array, itemIndex, &item);
                            if(SUCCEEDED(hr))
                            {
                                PWSTR pathWCStr = 0;
                                hr = item->lpVtbl->GetDisplayName(item, SIGDN_FILESYSPATH, &pathWCStr);
                                if(SUCCEEDED(hr))
                                {
                                    oc_str16 pathWide = oc_str16_from_buffer(lstrlenW(pathWCStr), pathWCStr);
                                    oc_str8 path = oc_win32_wide_to_utf8(arena, pathWide);

                                    //NOTE: convert Windows backslashes to forward slashes
                                    oc_win32_path_normalize_slash_in_place(path);

                                    oc_str8_list_push(arena, &result.selection, path);

                                    if(itemIndex == 0)
                                    {
                                        result.path = path;
                                    }

                                    CoTaskMemFree(pathWCStr);
                                }
                                item->lpVtbl->Release(item);
                            }
                        }
                        result.button = OC_FILE_DIALOG_OK;
                    }
                }
                else
                {
                    IShellItem* item;
                    hr = dialog->lpVtbl->GetResult(dialog, &item);
                    if(SUCCEEDED(hr))
                    {
                        PWSTR pathWCStr;
                        hr = item->lpVtbl->GetDisplayName(item, SIGDN_FILESYSPATH, &pathWCStr);

                        if(SUCCEEDED(hr))
                        {
                            oc_str16 pathWide = oc_str16_from_buffer(lstrlenW(pathWCStr), pathWCStr);
                            oc_str8 path = oc_win32_wide_to_utf8(arena, pathWide);

                            oc_win32_path_normalize_slash_in_place(path);

                            result.path = path;
                            oc_str8_list_push(arena, &result.selection, result.path);

                            CoTaskMemFree(pathWCStr);
                        }
                        item->lpVtbl->Release(item);
                        result.button = OC_FILE_DIALOG_OK;
                    }
                }
            }
        }
        dialog->lpVtbl->Release(dialog);
    }
    CoUninitialize();

    oc_scratch_end(scratch);
    return (result);
}

#include <commctrl.h>

int oc_alert_popup(oc_str8 title,
                   oc_str8 message,
                   oc_str8_list options)
{
    oc_arena_scope scratch = oc_scratch_begin();
    TASKDIALOG_BUTTON* buttons = oc_arena_push_array(scratch.arena, TASKDIALOG_BUTTON, options.eltCount);

    int i = 0;
    oc_list_for(options.list, elt, oc_str8_elt, listElt)
    {
        int textWideSize = MultiByteToWideChar(CP_UTF8, 0, elt->string.ptr, elt->string.len, NULL, 0);
        wchar_t* textWide = oc_arena_push_array(scratch.arena, wchar_t, textWideSize + 1);
        MultiByteToWideChar(CP_UTF8, 0, elt->string.ptr, elt->string.len, textWide, textWideSize);
        textWide[textWideSize] = '\0';

        buttons[i].nButtonID = i + 100;
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
            button -= 100;
        }
    }

    oc_scratch_end(scratch);
    return (button);
}
