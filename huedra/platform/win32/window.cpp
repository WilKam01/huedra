#include "window.hpp"
#include "core/global.hpp"
#include "core/log.hpp"

namespace huedra {

LRESULT CALLBACK Win32Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    Win32Window* self = nullptr;
    if (uMsg == WM_NCCREATE)
    {
        LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        self = static_cast<Win32Window*>(lpcs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else
    {
        self = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (self)
    {
        WindowRect rect = self->getRect();
        RECT winRect;
        switch (uMsg)
        {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
            rect.screenWidth = LOWORD(lParam);
            rect.screenHeight = HIWORD(lParam);

            if (GetWindowRect(self->m_handle, &winRect))
            {
                rect.width = winRect.right - winRect.left;
                rect.height = winRect.bottom - winRect.top;
            }
            break;
        case WM_MOVE:
            rect.xScreenPos = static_cast<i32>(static_cast<i16>(LOWORD(lParam)));
            rect.yScreenPos = static_cast<i32>(static_cast<i16>(HIWORD(lParam)));

            if (GetWindowRect(self->m_handle, &winRect))
            {
                rect.xPos = winRect.left;
                rect.yPos = winRect.top;
            }
            break;
        case WM_MOVING:
            winRect = *reinterpret_cast<RECT*>(lParam);
            rect.xScreenPos = winRect.left;
            rect.yScreenPos = winRect.top;

            if (GetWindowRect(self->m_handle, &winRect))
            {
                rect.xPos = winRect.left;
                rect.yPos = winRect.top;
            }
            break;
        case WM_KEYDOWN:
            Global::input.setKey(convertKey(wParam), true);
            break;
        case WM_KEYUP:
            Global::input.setKey(convertKey(wParam), false);
            break;
        case WM_SYSKEYDOWN:
            Global::input.setKey(Keys::ALT, true);
            return 0;
        case WM_SYSKEYUP:
            Global::input.setKey(Keys::ALT, false);
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
            EndPaint(hwnd, &ps);
            break;
        }
        }
        self->updateRect(rect);
        rect = self->getRect();
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool Win32Window::init(const std::string& title, const WindowInput& input, HINSTANCE instance)
{
    Window::init(title, {});

    const char CLASS_NAME[] = "Window Class";

    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = input.width;
    rect.bottom = input.height;

    AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, 0, 0);

    // clang-format off
    m_handle = CreateWindowEx(
        0,
        CLASS_NAME,
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        input.xPos.value_or(CW_USEDEFAULT),
        input.yPos.value_or(CW_USEDEFAULT),
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL,
        NULL,
        instance,
        this
    );
    // clang-format on

    if (m_handle == NULL)
    {
        log(LogLevel::WARNING, "Failed to create win32 window!");
        return false;
    }

    ShowWindow(m_handle, 1);

    return true;
}

void Win32Window::cleanup()
{
    Window::cleanup();
    if (IsWindow(m_handle))
    {
        DestroyWindow(m_handle);
    }
}

// TODO: Run on another thread since resizing and moving window will halt execution in DispatchMessage
bool Win32Window::update()
{
    Global::input.setKeyToggle(KeyToggles::CAPS_LOCK, GetKeyState(VK_CAPITAL) & 1);
    Global::input.setKeyToggle(KeyToggles::NUM_LOCK, GetKeyState(VK_NUMLOCK) & 1);
    Global::input.setKeyToggle(KeyToggles::SCR_LOCK, GetKeyState(VK_SCROLL) & 1);

    MSG msg{};
    while (PeekMessage(&msg, m_handle, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return IsWindow(m_handle);
}

void Win32Window::setTitle(const std::string& title)
{
    if (SetWindowTextA(m_handle, title.c_str()))
    {
        updateTitle(title);
    }
}

void Win32Window::setResolution(u32 width, u32 height)
{
    SetWindowPos(m_handle, NULL, 0, 0, width, height, SWP_NOMOVE);
}

void Win32Window::setPos(i32 x, i32 y) { SetWindowPos(m_handle, NULL, x, y, 0, 0, SWP_NOSIZE); }

Keys Win32Window::convertKey(i64 code)
{
    char letter = static_cast<char>(code);
    if (letter >= 'A' && letter <= 'Z')
    {
        return static_cast<Keys>(static_cast<u32>(Keys::A) + static_cast<u32>(letter - 'A'));
    }

    Keys key = Keys::NONE;
    switch (code)
    {
    case VK_LEFT:
        key = Keys::ARR_LEFT;
        break;
    case VK_RIGHT:
        key = Keys::ARR_RIGHT;
        break;
    case VK_UP:
        key = Keys::ARR_UP;
        break;
    case VK_DOWN:
        key = Keys::ARR_DOWN;
        break;
    case VK_ESCAPE:
        key = Keys::ESCAPE;
        break;
    case VK_SHIFT:
        key = Keys::SHIFT;
        break;
    case VK_CONTROL:
        key = Keys::CTRL;
        break;
    case VK_MENU:
        key = Keys::ALT;
        break;
    case VK_TAB:
        key = Keys::TAB;
        break;
    case VK_BACK:
        key = Keys::BACKSPACE;
        break;
    case VK_RETURN:
        key = Keys::ENTER;
        break;
    case VK_SPACE:
        key = Keys::SPACE;
        break;
    case VK_CAPITAL:
        key = Keys::CAPS_LOCK;
        break;
    case VK_NUMLOCK:
        key = Keys::NUM_LOCK;
        break;
    case VK_SCROLL:
        key = Keys::SCR_LOCK;
        break;
    case VK_F1:
        key = Keys::F1;
        break;
    case VK_F2:
        key = Keys::F2;
        break;
    case VK_F3:
        key = Keys::F3;
        break;
    case VK_F4:
        key = Keys::F4;
        break;
    case VK_F5:
        key = Keys::F5;
        break;
    case VK_F6:
        key = Keys::F6;
        break;
    case VK_F7:
        key = Keys::F7;
        break;
    case VK_F8:
        key = Keys::F8;
        break;
    case VK_F9:
        key = Keys::F9;
        break;
    case VK_F10:
        key = Keys::F10;
        break;
    case VK_F11:
        key = Keys::F11;
        break;
    case VK_F12:
        key = Keys::F12;
        break;
    case VK_OEM_COMMA:
        key = Keys::COMMA;
        break;
    case VK_OEM_PERIOD:
        key = Keys::DOT;
        break;
    case VK_OEM_MINUS:
        key = Keys::MINUS;
        break;
    case VK_OEM_PLUS:
        key = Keys::PLUS;
        break;
    case VK_INSERT:
        key = Keys::INSERT;
        break;
    case VK_DELETE:
        key = Keys::DEL;
        break;
    case VK_HOME:
        key = Keys::HOME;
        break;
    case VK_END:
        key = Keys::END;
        break;
    case VK_PRIOR:
        key = Keys::PAGE_UP;
        break;
    case VK_NEXT:
        key = Keys::PAGE_DOWN;
        break;
    case VK_DIVIDE:
        key = Keys::NUMPAD_DIVIDE;
        break;
    case VK_MULTIPLY:
        key = Keys::NUMPAD_MULT;
        break;
    case VK_SUBTRACT:
        key = Keys::NUMPAD_MINUS;
        break;
    case VK_ADD:
        key = Keys::NUMPAD_PLUS;
        break;
    case VK_SEPARATOR:
        key = Keys::NUMPAD_DEL;
        break;
    case VK_NUMPAD0:
        key = Keys::NUMPAD_0;
        break;
    case VK_NUMPAD1:
        key = Keys::NUMPAD_1;
        break;
    case VK_NUMPAD2:
        key = Keys::NUMPAD_2;
        break;
    case VK_NUMPAD3:
        key = Keys::NUMPAD_3;
        break;
    case VK_NUMPAD4:
        key = Keys::NUMPAD_4;
        break;
    case VK_NUMPAD5:
        key = Keys::NUMPAD_5;
        break;
    case VK_NUMPAD6:
        key = Keys::NUMPAD_6;
        break;
    case VK_NUMPAD7:
        key = Keys::NUMPAD_7;
        break;
    case VK_NUMPAD8:
        key = Keys::NUMPAD_8;
        break;
    case VK_NUMPAD9:
        key = Keys::NUMPAD_9;
        break;
    }

    if (key == Keys::NONE && letter >= '0' && letter <= '9')
    {
        return static_cast<Keys>(static_cast<u32>(Keys::_0) + static_cast<u32>(letter - '0'));
    }

    return key;
}

} // namespace huedra