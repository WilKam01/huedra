
#include "huedra/core/global.hpp"
#include "huedra/core/log.hpp"

namespace log = huedra::log;
using huedra::Ref;

int main()
{
    huedra::global::init();

    Ref<huedra::Window> window = huedra::global::windowManager.addWindow("HDR example", huedra::WindowInput{1280, 720});

    huedra::PipelineBuilder pipeline;
    pipeline.init(huedra::PipelineType::GRAPHICS)
        .addShader(huedra::global::resourceManager.loadShaderModule("assets/shaders/shader.slang"), "vertMain")
        .addShader(huedra::global::resourceManager.loadShaderModule("assets/shaders/shader.slang"), "fragMain");

    huedra::HdrTextureData hdrData = huedra::global::resourceManager.loadHdrTextureData(
        "assets/textures/citrus_orchard_road_puresky_1k.hdr", huedra::TexelChannelFormat::RGBA);
    Ref<huedra::Texture> hdrTexture = huedra::global::graphicsManager.createTexture(hdrData);

    while (huedra::global::windowManager.update())
    {
        huedra::global::timer.update();
        huedra::global::graphicsManager.update();

        huedra::RenderPassBuilder pass;
        pass.init(huedra::RenderPassType::GRAPHICS)
            .addRenderTarget(window->getRenderTarget(), huedra::vec3(0.1f))
            .setClearRenderTargets(true)
            .addResource(huedra::ResourceAccessType::READ, hdrTexture, huedra::ShaderStage::FRAGMENT)
            .setPipeline(pipeline)
            .setCommands([&hdrTexture](huedra::RenderContext& context) {
                context.bindTexture(hdrTexture, "hdr");
                context.bindSampler(huedra::SAMPLER_LINEAR, "sampler");
                context.draw(4, 1, 0, 0);
            });

        huedra::RenderGraphBuilder renderGraph;
        renderGraph.init().addPass("hdr", pass);

        huedra::global::graphicsManager.render(renderGraph);
    }

    huedra::global::cleanup();
    return 0;
}