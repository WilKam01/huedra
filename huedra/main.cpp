#include "core/global.hpp"
#include "core/log.hpp"
#include "math/conversions.hpp"
#include "math/matrix_projection.hpp"
#include "math/matrix_transform.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"

using namespace huedra;

int main()
{
    Global::timer.init();
    Global::windowManager.init();
    Global::graphicsManager.init();

    Ref<Window> window =
        Global::windowManager.addWindow("Hello Windows!", huedra::WindowInput(1278, 1360, -7, 0, false));
    Ref<Window> window1 = Global::windowManager.addWindow("Hello", huedra::WindowInput(300, 300, 100, 100), window);

    // Draw data
    std::array<vec2, 4> positions = {{{-1.0f, -1.0f}, {1.0f, -1.0f}, {1.0f, 1.0f}, {-1.0f, 1.0f}}};
    std::array<vec3, 4> colors = {{{0.5f, 0.0f, 0.0f}, {0.0f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.5f}, {0.5f, 0.5f, 0.5f}}};
    std::array<u32, 6> indices = {0, 1, 2, 2, 3, 0};

    Ref<Buffer> vertexPositionsBuffer = Global::graphicsManager.createBuffer(
        BufferType::STATIC, HU_BUFFER_USAGE_VERTEX_BUFFER, sizeof(vec2) * 4, positions.data());
    Ref<Buffer> vertexColorsBuffer = Global::graphicsManager.createBuffer(
        BufferType::STATIC, HU_BUFFER_USAGE_VERTEX_BUFFER, sizeof(vec3) * 4, colors.data());
    Ref<Buffer> indexBuffer = Global::graphicsManager.createBuffer(BufferType::STATIC, HU_BUFFER_USAGE_INDEX_BUFFER,
                                                                   sizeof(u32) * 6, indices.data());

    // Shader Resources
    WindowRect rect = window.get()->getRect();
    matrix4 viewProj =
        perspective(radians(90.0f), static_cast<float>(rect.screenWidth) / static_cast<float>(rect.screenHeight),
                    vec2(0.1f, 10.0f)) *
        lookAt(vec3(0.0f, 0.0f, -25.0f), vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));

    Ref<Buffer> viewProjBuffer = Global::graphicsManager.createBuffer(
        BufferType::STATIC, HU_BUFFER_USAGE_UNIFORM_BUFFER, sizeof(viewProj), &viewProj);

    Ref<ResourceSet> resourseSet(nullptr);

    PipelineBuilder builder;
    builder.init(PipelineType::GRAPHICS)
        .addShader(ShaderStage::VERTEX, "shaders/shader.vert")
        .addShader(ShaderStage::FRAGMENT, "shaders/shader.frag")
        .addVertexInputStream({sizeof(vec2), VertexInputRate::VERTEX, {{GraphicsDataFormat::RG_32_FLOAT, 0}}})
        .addVertexInputStream({sizeof(vec3), VertexInputRate::VERTEX, {{GraphicsDataFormat::RGB_32_FLOAT, 0}}})
        .addResourceSet()
        .addResourceBinding(HU_SHADER_STAGE_VERTEX, ResourceType::UNIFORM_BUFFER);

    RenderPassSettings settings;
    settings.clearColor = {{0.2f, 0.2f, 0.2f}};

    RenderGraphBuilder graph;
    graph.init().addGraphicsPass(
        "Pass", builder, window.get()->getRenderTarget(),
        [vertexPositionsBuffer, vertexColorsBuffer, indexBuffer, &resourseSet](RenderContext& renderContext) {
            renderContext.bindVertexBuffers({vertexPositionsBuffer, vertexColorsBuffer});
            renderContext.bindIndexBuffer(indexBuffer);
            renderContext.bindResourceSet(resourseSet);
            renderContext.drawIndexed(6, 1, 0, 0);
        },
        {}, settings);

    graph.addGraphicsPass("Pass1", builder, window1.get()->getRenderTarget(),
                          [vertexPositionsBuffer, vertexColorsBuffer, indexBuffer](RenderContext& renderContext) {
                              renderContext.bindVertexBuffers({vertexPositionsBuffer, vertexColorsBuffer});
                              renderContext.bindIndexBuffer(indexBuffer);
                              renderContext.drawIndexed(3, 1, 0, 0);
                          });

    Global::graphicsManager.setRenderGraph(graph);

    resourseSet = Global::graphicsManager.createResourceSet("Pass", 0);

    while (Global::windowManager.update())
    {
        Global::timer.update();

        static u32 count = 0;
        if (count < GraphicsManager::MAX_FRAMES_IN_FLIGHT)
        {
            resourseSet.get()->assignBuffer(viewProjBuffer, 0);
            count++;
        }
        Global::graphicsManager.render();

        static u32 i = 0;
        static std::array<u32, 500> avgFps;

        avgFps[i++] = static_cast<u32>(1.0f / Global::timer.dt());
        if (i >= 500)
        {
            u32 sum = 0;
            for (auto& fps : avgFps)
            {
                sum += fps;
            }

            log(LogLevel::INFO, "Elapsed: %f, Delta: %f, FPS: %u", Global::timer.secondsElapsed(), Global::timer.dt(),
                sum / 500);
            i = 0;
        }
    }

    Global::graphicsManager.cleanup();
    Global::windowManager.cleanup();

#ifdef DEBUG
    ReferenceCounter::reportState();
#endif
}