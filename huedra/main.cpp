#include "core/global.hpp"

using namespace huedra;

int main()
{
    Global::windowManager.init();
    Global::graphicsManager.init();

    Ref<Window> window = Global::windowManager.addWindow("Hello Windows!", huedra::WindowInput(1278, 1360, -7, 0));
    Ref<Window> window1 = Global::windowManager.addWindow("Hello", huedra::WindowInput(300, 300, 100, 100), window);

    RenderPass pass;
    pass.initGraphics("Pass", window.get()->getRenderTarget());

    RenderPass pass1;
    pass1.initGraphics("Pass1", window1.get()->getRenderTarget());

    Global::graphicsManager.addRenderPass(pass);
    Global::graphicsManager.addRenderPass(pass1);

    while (Global::windowManager.update())
    {
        Global::graphicsManager.render();
    }

    Global::graphicsManager.cleanup();
    Global::windowManager.cleanup();

#ifdef DEBUG
    ReferenceCounter::reportState();
#endif
}