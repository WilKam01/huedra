#include "core/global.hpp"

using namespace huedra;

int main()
{
    Global::windowManager.init();
    Global::graphicsManager.init();

    huedra::Window* window = Global::windowManager.addWindow("Hello Windows!", huedra::WindowInput(1278, 1360, -7, 0));

    while (Global::windowManager.update())
    {
    }

    Global::graphicsManager.cleanup();
    Global::windowManager.cleanup();
}