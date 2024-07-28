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
        .addShader(ShaderStage::FRAGMENT, "shaders/shader.frag")
        .addVertexInputStream({sizeof(float) * 2, VertexInputRate::VERTEX, {{GraphicsDataFormat::RG_32_FLOAT, 0}}})
        .addVertexInputStream({sizeof(float) * 3, VertexInputRate::VERTEX, {{GraphicsDataFormat::RGB_32_FLOAT, 0}}});

    Ref<Pipeline> pipeline = Global::graphicsManager.createPipeline(builder);
    RenderPass pass;
    pass.initGraphics("Pass", pipeline, window.get()->getRenderTarget());

    builder.init(PipelineType::GRAPHICS)
        .addShader(ShaderStage::VERTEX, "shaders/shader.vert")
        .addShader(ShaderStage::FRAGMENT, "shaders/shader1.frag")
        .addVertexInputStream({sizeof(float) * 2, VertexInputRate::VERTEX, {{GraphicsDataFormat::RG_32_FLOAT, 0}}})
        .addVertexInputStream({sizeof(float) * 3, VertexInputRate::VERTEX, {{GraphicsDataFormat::RGB_32_FLOAT, 0}}})
        .addPushConstantRange(HU_SHADER_STAGE_VERTEX, 64)
        .addPushConstantRange(HU_SHADER_STAGE_FRAGMENT, 128)
        .addResourceSet()
        .addResourceBinding(HU_SHADER_STAGE_VERTEX, ResourceType::UNIFORM_BUFFER)
        .addResourceBinding(HU_SHADER_STAGE_FRAGMENT, ResourceType::TEXTURE)
        .addResourceBinding(HU_SHADER_STAGE_VERTEX | HU_SHADER_STAGE_FRAGMENT, ResourceType::UNIFORM_BUFFER)
        .addResourceSet()
        .addResourceBinding(HU_SHADER_STAGE_GRAPHICS_ALL, ResourceType::UNIFORM_BUFFER);

    pipeline = Global::graphicsManager.createPipeline(builder);
    RenderPass pass1;
    pass1.initGraphics("Pass1", pipeline, window1.get()->getRenderTarget());

    Global::graphicsManager.addRenderPass(pass);
    Global::graphicsManager.addRenderPass(pass1);

    std::array<u32, 4> nums{1, 2, 3, 4};
    Ref<Buffer> buffer = Global::graphicsManager.createBuffer(BufferType::DYNAMIC, HU_BUFFER_USAGE_UNIFORM_BUFFER,
                                                              sizeof(u32) * 4, nums.data());

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