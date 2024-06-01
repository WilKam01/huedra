#include "core/global.hpp"
#include <iostream>

using namespace huedra::global;

int main()
{
    windowManager.init();
    huedra::Window* window = windowManager.createWindow("Hello Windows!", huedra::WindowInput(600, 400));
    huedra::Window* window2 = windowManager.createWindow(
        "Hello Windows1!", huedra::WindowInput(200, 100, window->getRect().xScreenPos, window->getRect().yScreenPos));
    huedra::Window* window3 = windowManager.createWindow("Hello Windows2!", huedra::WindowInput(100, 100));
    window2->setParent(window);
    window3->setParent(window2);

    huedra::WindowRect rect = window->getRect();
    window->setPos(rect.xPos + 200, rect.yPos + 200);
    window2->setResolution(200, 400);
    window3->setTitle("Window3");

    while (windowManager.update())
    {
        rect = window->getRect();
        std::cout << "Pos: (" << rect.xScreenPos << ", " << rect.yScreenPos << ") | Size: (" << rect.screenWidth << ", "
                  << rect.screenHeight << ")\n";
    }

    windowManager.cleanup();

    return 0;
}