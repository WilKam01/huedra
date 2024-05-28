#include "platform/win32/window.hpp"

#include <iostream>

int main()
{
    huedra::Win32Window window;
    window.init("Hello Windows!", huedra::WindowInput(600, 400), GetModuleHandle(NULL));

    while (window.update())
    {
        huedra::WindowRect rect = window.getRect();
        std::cout << " | Width: " << rect.width << " | Height: " << rect.height << " | xPos: " << rect.xPos
                  << " | yPos: " << rect.yPos << " | Screen Width: " << rect.screenWidth
                  << " | Screen Height: " << rect.screenHeight << " | xScrenPos: " << rect.xScreenPos
                  << " | yScreenPos: " << rect.yScreenPos << " |\n";
    }

    return 0;
}