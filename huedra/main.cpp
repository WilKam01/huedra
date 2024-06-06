#include "core/global.hpp"

using namespace huedra::global;

int main()
{
    windowManager.init();
    huedra::Window* window = windowManager.createWindow("Hello Windows!", huedra::WindowInput(1278, 1360, -7, 0));

    graphicsManager.init();

    while (windowManager.update())
    {
    }

    graphicsManager.cleanup();
    windowManager.cleanup();
}