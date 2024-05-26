#include "window.hpp"

namespace huedra {

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
        EndPaint(hwnd, &ps);
    }
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool Win32Window::init(const std::string& title, Vector2i rect, HINSTANCE instance)
{
    Window::init(title, rect);

    const char CLASS_NAME[] = "Window Class";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = instance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // clang-format off
    m_window = CreateWindowEx(
        0,
        CLASS_NAME,
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rect.x,
        rect.y,
        NULL,
        NULL,
        instance,
        NULL
    );
    // clang-format on

    if (m_window == NULL)
    {
        // Log Error
        return false;
    }

    ShowWindow(m_window, 1);

    return true;
}

void Win32Window::cleanup() { Window::cleanup(); }

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