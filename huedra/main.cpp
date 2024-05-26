#include "platform/win32/window.hpp"

int main()
{
    huedra::Win32Window window;
    window.init("Hello Windows!", huedra::Vector2i(600, 400), GetModuleHandle(NULL));

    while (window.update())
    {
    }

    return 0;
}