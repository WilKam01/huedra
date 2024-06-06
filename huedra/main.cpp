#include "core/global.hpp"
#include <iostream>

using namespace huedra::global;

int main()
{
    windowManager.init();
    huedra::Window* window = windowManager.createWindow("Hello Windows!", huedra::WindowInput(1278, 1360, -7, 0));

    graphicsManager.init();

    while (windowManager.update())
    {
        huedra::WindowRect rect = window->getRect();
        std::cout << "Pos: (" << rect.xScreenPos << ", " << rect.yScreenPos << ") | Size: (" << rect.screenWidth << ", "
                  << rect.screenHeight << ")\n";
    }

    graphicsManager.cleanup();
    windowManager.cleanup();
}