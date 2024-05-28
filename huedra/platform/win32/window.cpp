#include "window.hpp"
#include <iostream>

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
        case WM_MOVING:
            winRect = *reinterpret_cast<RECT*>(lParam);
            rect.xScreenPos = winRect.left;
            rect.yScreenPos = winRect.top;
            // std::cout << "Moving\n";

            if (GetWindowRect(self->m_handle, &winRect))
            {
                rect.xPos = winRect.left;
                rect.yPos = winRect.top;
            }
            break;
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
        // std::cout << "Rect: (" << rect.xScreenPos << ", " << rect.yScreenPos << ")\n";
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool Win32Window::init(const std::string& title, WindowInput input, HINSTANCE instance)
{
    Window::init(title, {});

    const char CLASS_NAME[] = "Window Class";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = instance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

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
        // Log Error
        return false;
    }

    ShowWindow(m_handle, 1);

    return true;
}

void Win32Window::cleanup() { Window::cleanup(); }

// TODO: Run on another thread since resizing and moving window will halt execution in DispatchMessage
bool Win32Window::update()
{
    MSG msg{};
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (msg.message == WM_QUIT)
        {
            return false;
        }
    }

    return true;
}

} // namespace huedra