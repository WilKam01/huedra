
#include "huedra/core/global.hpp"
#include "huedra/core/log.hpp"

namespace log = huedra::log;
using huedra::Ref;

int main()
{
    huedra::global::init();

    Ref<huedra::Window> window = huedra::global::windowManager.addWindow("HDR example", huedra::WindowInput{1280, 720});

    while (huedra::global::windowManager.update())
    {
        huedra::global::timer.update();
        huedra::global::graphicsManager.update();

        huedra::RenderPassBuilder pass;
        pass.init(huedra::RenderPassType::GRAPHICS)
            .addRenderTarget(window->getRenderTarget(), huedra::vec3(0.1f))
            .setClearRenderTargets(true);

        huedra::RenderGraphBuilder renderGraph;
        renderGraph.init().addPass("clear", pass);

        huedra::global::graphicsManager.render(renderGraph);
    }

    huedra::global::cleanup();
    return 0;
}