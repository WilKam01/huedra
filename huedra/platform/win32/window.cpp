#include "window.hpp"
#include "core/global.hpp"
#include "core/log.hpp"

namespace huedra {

// Win32 is notorious for converting actual bytes to other types
// Therefore, it's permissable to use reinterpret_cast and other pointer arithmetic
// NOLINTBEGIN(performance-no-int-to-ptr, performance-no-int-to-ptr)
LRESULT CALLBACK WindowWin32::windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    WindowWin32* self = nullptr;
    if (uMsg == WM_NCCREATE)
    {
        auto* lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        self = static_cast<WindowWin32*>(lpcs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else
    {
        self = reinterpret_cast<WindowWin32*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (self != nullptr)
    {
        i32 xPos{0};
        i32 yPos{0};
        i32 screenXPos{0};
        i32 screenYPos{0};
        u32 width{0};
        u32 height{0};
        u32 screenWidth{0};
        u32 screenHeight{0};
        MouseButton button = MouseButton::NONE;

        RECT winRect{};
        POINT point{};

        switch (uMsg)
        {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
            screenWidth = LOWORD(lParam);
            screenHeight = HIWORD(lParam);

            if (GetWindowRect(self->m_handle, &winRect) != 0)
            {
                width = winRect.right - winRect.left;
                height = winRect.bottom - winRect.top;
            }
            self->updateResolution(width, height, screenWidth, screenHeight);
            break;
        case WM_MOVE:
            screenXPos = static_cast<i32>(static_cast<i16>(LOWORD(lParam)));
            screenYPos = static_cast<i32>(static_cast<i16>(HIWORD(lParam)));

            if (GetWindowRect(self->m_handle, &winRect) != 0)
            {
                xPos = winRect.left;
                yPos = winRect.top;
            }
            self->updatePosition(xPos, yPos, screenXPos, screenYPos);
            break;
        case WM_MOVING:
            winRect = *reinterpret_cast<RECT*>(lParam);
            screenXPos = winRect.left;
            screenYPos = winRect.top;

            if (GetWindowRect(self->m_handle, &winRect) != 0)
            {
                xPos = winRect.left;
                yPos = winRect.top;
            }
            self->updatePosition(xPos, yPos, screenXPos, screenYPos);
            break;
        case WM_INPUT: {
            RAWINPUT raw;
            UINT size = sizeof(RAWINPUT);
            GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER));

            if (raw.header.dwType == RIM_TYPEMOUSE)
            {
                global::input.setMouseDelta(ivec2(raw.data.mouse.lLastX, raw.data.mouse.lLastY));
            }
            break;
        }
        case WM_MOUSEMOVE:
            GetCursorPos(&point);
            global::input.setMousePosition(ivec2(point.x, point.y));
            break;
        case WM_MOUSEWHEEL:
            global::input.setMouseScrollVertical(GET_WHEEL_DELTA_WPARAM(wParam));
            break;
        case WM_MOUSEHWHEEL:
            global::input.setMouseScrollHorizontal(GET_WHEEL_DELTA_WPARAM(wParam));
            break;
        case WM_ACTIVATE:
            if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE)
            {
                global::windowManager.setFocusedWindow(self);
            }
            else if (wParam == WA_INACTIVE && global::windowManager.getFocusedWindow().get() == self)
            {
                global::windowManager.setFocusedWindow(nullptr);
            }
            break;
        case WM_SETCURSOR:
            global::windowManager.setCursor(global::input.getCursor());
            break;
        case WM_KEYDOWN:
            global::input.setKey(convertKey(wParam), true);
            break;
        case WM_KEYUP:
            global::input.setKey(convertKey(wParam), false);
            break;
        case WM_SYSKEYDOWN:
            global::input.setKey(Keys::ALT, true);
            return 0;
        case WM_SYSKEYUP:
            global::input.setKey(Keys::ALT, false);
            return 0;
        case WM_LBUTTONDOWN:
            global::input.setMouseButton(MouseButton::LEFT, true);
            break;
        case WM_LBUTTONUP:
            global::input.setMouseButton(MouseButton::LEFT, false);
            break;
        case WM_LBUTTONDBLCLK:
            global::input.setMouseButtonDoubleClick(MouseButton::LEFT);
            break;
        case WM_RBUTTONDOWN:
            global::input.setMouseButton(MouseButton::RIGHT, true);
            break;
        case WM_RBUTTONUP:
            global::input.setMouseButton(MouseButton::RIGHT, false);
            break;
        case WM_RBUTTONDBLCLK:
            global::input.setMouseButtonDoubleClick(MouseButton::RIGHT);
            break;
        case WM_MBUTTONDOWN:
            global::input.setMouseButton(MouseButton::MIDDLE, true);
            break;
        case WM_MBUTTONUP:
            global::input.setMouseButton(MouseButton::MIDDLE, false);
            break;
        case WM_MBUTTONDBLCLK:
            global::input.setMouseButtonDoubleClick(MouseButton::MIDDLE);
            break;
        case WM_XBUTTONDOWN:
            button = GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? MouseButton::EXTRA1 : MouseButton::EXTRA2;
            global::input.setMouseButton(button, true);
            break;
        case WM_XBUTTONUP:
            button = GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? MouseButton::EXTRA1 : MouseButton::EXTRA2;
            global::input.setMouseButton(button, false);
            break;
        case WM_XBUTTONDBLCLK:
            button = GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? MouseButton::EXTRA1 : MouseButton::EXTRA2;
            global::input.setMouseButtonDoubleClick(button);
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            FillRect(hdc, &ps.rcPaint, reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1));
            EndPaint(hwnd, &ps);
            break;
        }
        default:
            break;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
// NOLINTEND(performance-no-int-to-ptr, performance-no-int-to-ptr)

bool WindowWin32::init(const std::string& title, const WindowInput& input, HINSTANCE instance)
{
    RECT winRect;
    winRect.left = 0;
    winRect.top = 0;
    winRect.right = static_cast<i32>(input.width);
    winRect.bottom = static_cast<i32>(input.height);

    AdjustWindowRectEx(&winRect, WS_OVERLAPPEDWINDOW, 0, 0);

    // clang-format off
    m_handle = CreateWindowEx(
        0,
        "Window Class",
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        input.xPos.value_or(CW_USEDEFAULT),
        input.yPos.value_or(CW_USEDEFAULT),
        winRect.right - winRect.left,
        winRect.bottom - winRect.top,
        nullptr,
        nullptr,
        instance,
        this
    );
    // clang-format on

    if (m_handle == nullptr)
    {
        log(LogLevel::WARNING, "Failed to create win32 window!");
        return false;
    }

    ShowWindow(m_handle, 1);

    // Init base window class with position and resolution
    WindowRect rect{};
    if (GetWindowRect(m_handle, &winRect) != 0)
    {
        rect.xPos = winRect.left;
        rect.yPos = winRect.top;
        rect.width = winRect.right - winRect.left;
        rect.height = winRect.bottom - winRect.top;
    }
    if (GetClientRect(m_handle, &winRect) != 0)
    {
        rect.screenXPos = winRect.left;
        rect.screenYPos = winRect.top;
        rect.screenWidth = winRect.right - winRect.left;
        rect.screenHeight = winRect.bottom - winRect.top;
    }
    Window::init(title, rect);

    return true;
}

void WindowWin32::cleanup()
{
    Window::cleanup();
    if (IsWindow(m_handle) != 0)
    {
        DestroyWindow(m_handle);
    }
}

// TODO: Run on another thread since resizing and moving window will halt execution in DispatchMessage
bool WindowWin32::update()
{
    global::input.setKeyToggle(KeyToggles::CAPS_LOCK, (GetKeyState(VK_CAPITAL) & 1) != 0);
    global::input.setKeyToggle(KeyToggles::NUM_LOCK, (GetKeyState(VK_NUMLOCK) & 1) != 0);
    global::input.setKeyToggle(KeyToggles::SCR_LOCK, (GetKeyState(VK_SCROLL) & 1) != 0);

    MSG msg{};
    while (PeekMessage(&msg, m_handle, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return IsWindow(m_handle) != 0;
}

void WindowWin32::setTitle(const std::string& title)
{
    if (SetWindowTextA(m_handle, title.c_str()) != 0)
    {
        updateTitle(title);
    }
}

void WindowWin32::setResolution(u32 width, u32 height)
{
    SetWindowPos(m_handle, nullptr, 0, 0, static_cast<i32>(width), static_cast<i32>(height), SWP_NOMOVE);
}

void WindowWin32::setPosition(i32 x, i32 y) { SetWindowPos(m_handle, nullptr, x, y, 0, 0, SWP_NOSIZE); }

Keys WindowWin32::convertKey(u32 code)
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
    default:
        break;
    }

    if (key == Keys::NONE && letter >= '0' && letter <= '9')
    {
        return static_cast<Keys>(static_cast<u32>(Keys::NUM_0) + static_cast<u32>(letter - '0'));
    }

    return key;
}

} // namespace huedra