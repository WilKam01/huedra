#include "core/global.hpp"

using namespace huedra;

int main()
{
    Global::windowManager.init();
    Global::graphicsManager.init();

    Ref<Window> window = Global::windowManager.addWindow("Hello Windows!", huedra::WindowInput(1278, 1360, -7, 0));
    Ref<Window> window1 = Global::windowManager.addWindow("Hello", huedra::WindowInput(300, 300, 100, 100), window);

    PipelineBuilder builder;
    builder.init(PipelineType::GRAPHICS)
        .addShader(ShaderStage::VERTEX, "shaders/shader.vert")
        .addShader(ShaderStage::FRAGMENT, "shaders/shader.frag");

    Ref<Pipeline> pipeline = Global::graphicsManager.createPipeline(builder);
    RenderPass pass;
    pass.initGraphics("Pass", pipeline, window.get()->getRenderTarget());

    builder.init(PipelineType::GRAPHICS)
        .addShader(ShaderStage::VERTEX, "shaders/shader1.vert")
        .addShader(ShaderStage::FRAGMENT, "shaders/shader.frag")
        .addPushConstantRange(SHADER_STAGE_VERTEX, 64)
        .addPushConstantRange(SHADER_STAGE_FRAGMENT, 128)
        .addResourceSet()
        .addResourceBinding(SHADER_STAGE_VERTEX, ResourceType::UNIFORM_BUFFER)
        .addResourceBinding(SHADER_STAGE_FRAGMENT, ResourceType::TEXTURE)
        .addResourceBinding(SHADER_STAGE_VERTEX | SHADER_STAGE_FRAGMENT, ResourceType::UNIFORM_BUFFER)
        .addResourceSet()
        .addResourceBinding(SHADER_STAGE_GRAPHICS_ALL, ResourceType::UNIFORM_BUFFER);

    pipeline = Global::graphicsManager.createPipeline(builder);
    RenderPass pass1;
    pass1.initGraphics("Pass1", pipeline, window1.get()->getRenderTarget());

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