#include "core/global.hpp"
#include <iostream>

using namespace huedra::global;

int main()
{
    windowManager.init();
    huedra::Window* window = windowManager.createWindow("Hello Windows1!", huedra::WindowInput(600, 400));
    windowManager.createWindow(
        "Hello Windows1!", huedra::WindowInput(200, 100, window->getRect().xScreenPos, window->getRect().yScreenPos));

    while (windowManager.update())
    {
        huedra::WindowRect rect = window->getRect();
        std::cout << "Pos: (" << rect.xScreenPos << ", " << rect.yScreenPos << ") | Size: (" << rect.width << ", "
                  << rect.height << ")\n";
    }

    windowManager.cleanup();

    return 0;
}