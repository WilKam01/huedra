#include "window_manager.hpp"
#include "core/global.hpp"
#include "core/log.hpp"

#ifdef WIN32
#include "platform/win32/window.hpp"
#endif

namespace huedra {

void WindowManager::init()
{
#ifdef WIN32
    const char CLASS_NAME[] = "Window Class";

    WNDCLASS wc = {};
    wc.lpfnWndProc = Win32Window::WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);
#endif
}

bool WindowManager::update()
{
    for (size_t i = 0; i < m_windows.size();)
    {
        Window* window = m_windows[i];
        if (window->shouldClose() || !window->update())
        {
            m_windows.erase(m_windows.begin() + i);
            Global::graphicsManager.removeSwapchain(i);

            window->cleanup();
            delete window;
        }
        else
        {
            ++i;
        }
    }

    return !m_windows.empty();
}

void WindowManager::cleanup()
{
    for (auto& window : m_windows)
    {
        window->cleanup();
        delete window;
    }
}

Ref<Window> WindowManager::addWindow(const std::string& title, const WindowInput& input, Ref<Window> parent)
{
    Window* window = createWindow(title, input);

    if (window)
    {
        m_windows.push_back(window);
        Global::graphicsManager.createSwapchain(window);
        if (parent.valid())
        {
            window->setParent(parent);
        }
    }
    else
    {
        log(LogLevel::WARNING, "Failed to create window!");
    }

    return Ref<Window>(window);
}

Window* WindowManager::createWindow(const std::string& title, const WindowInput& input)
{
    Window* ret = nullptr;
    bool success = false;
#ifdef WIN32
    Win32Window* window = new Win32Window();
    success = window->init(title, input, GetModuleHandle(NULL));
#endif
    if (success)
    {
        ret = window;
    }
    else
    {
        delete window;
    }

    return ret;
}

} // namespace huedra
