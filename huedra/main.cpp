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
    std::array<vec3, 8> positions = {{{-1.0f, -1.0f, 1.0f},
                                      {1.0f, -1.0f, 1.0f},
                                      {1.0f, 1.0f, 1.0f},
                                      {-1.0f, 1.0f, 1.0f},
                                      {-1.0f, -1.0f, -1.0f},
                                      {1.0f, -1.0f, -1.0f},
                                      {1.0f, 1.0f, -1.0f},
                                      {-1.0f, 1.0f, -1.0f}}};
    std::array<vec3, 8> colors = {{{0.5f, 0.0f, 0.0f},
                                   {0.0f, 0.5f, 0.0f},
                                   {0.0f, 0.0f, 0.5f},
                                   {0.5f, 0.5f, 0.5f},
                                   {0.5f, 0.0f, 0.0f},
                                   {0.0f, 0.5f, 0.0f},
                                   {0.0f, 0.0f, 0.5f},
                                   {0.5f, 0.5f, 0.5f}}};
    std::array<u32, 36> indices = {0, 1, 2, 2, 3, 0, 4, 6, 5, 6, 4, 7, 4, 5, 1, 1, 0, 4,
                                   3, 2, 6, 6, 7, 3, 1, 5, 6, 6, 2, 1, 4, 0, 3, 3, 7, 4};

    Ref<Buffer> vertexPositionsBuffer = Global::graphicsManager.createBuffer(
        BufferType::STATIC, HU_BUFFER_USAGE_VERTEX_BUFFER, sizeof(vec3) * 8, positions.data());
    Ref<Buffer> vertexColorsBuffer = Global::graphicsManager.createBuffer(
        BufferType::STATIC, HU_BUFFER_USAGE_VERTEX_BUFFER, sizeof(vec3) * 8, colors.data());
    Ref<Buffer> indexBuffer = Global::graphicsManager.createBuffer(BufferType::STATIC, HU_BUFFER_USAGE_INDEX_BUFFER,
                                                                   sizeof(u32) * 36, indices.data());

    // Shader Resources
    WindowRect rect = window.get()->getRect();
    matrix4 viewProj =
        perspective(radians(45.0f), static_cast<float>(rect.screenWidth) / static_cast<float>(rect.screenHeight),
                    vec2(0.1f, 100.0f)) *
        lookAt(vec3(0.0f, 0.0f, -5.0f), vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));

    matrix4 modelMatrix(1.0f);

    Ref<Buffer> viewProjBuffer = Global::graphicsManager.createBuffer(
        BufferType::DYNAMIC, HU_BUFFER_USAGE_UNIFORM_BUFFER, sizeof(viewProj), &viewProj);

    Ref<ResourceSet> resourseSet(nullptr);

    PipelineBuilder builder;
    builder.init(PipelineType::GRAPHICS)
        .addShader(ShaderStage::VERTEX, "shaders/shader.vert")
        .addShader(ShaderStage::FRAGMENT, "shaders/shader.frag")
        .addVertexInputStream({sizeof(vec3), VertexInputRate::VERTEX, {{GraphicsDataFormat::RGB_32_FLOAT, 0}}})
        .addVertexInputStream({sizeof(vec3), VertexInputRate::VERTEX, {{GraphicsDataFormat::RGB_32_FLOAT, 0}}})
        .addPushConstantRange(HU_SHADER_STAGE_VERTEX, sizeof(modelMatrix))
        .addResourceSet()
        .addResourceBinding(HU_SHADER_STAGE_VERTEX, ResourceType::UNIFORM_BUFFER);

    RenderPassSettings settings;
    settings.clearColor = {{0.2f, 0.2f, 0.2f}};

    RenderGraphBuilder graph;
    graph.init().addGraphicsPass(
        "Pass", builder, window.get()->getRenderTarget(),
        [vertexPositionsBuffer, vertexColorsBuffer, indexBuffer, &resourseSet,
         &modelMatrix](RenderContext& renderContext) {
            renderContext.bindVertexBuffers({vertexPositionsBuffer, vertexColorsBuffer});
            renderContext.bindIndexBuffer(indexBuffer);
            renderContext.bindResourceSet(resourseSet);
            renderContext.pushConstants(HU_SHADER_STAGE_VERTEX, sizeof(modelMatrix), &modelMatrix);
            renderContext.drawIndexed(36, 1, 0, 0);
        },
        {}, settings);

    graph.addGraphicsPass("Pass1", builder, window1.get()->getRenderTarget(),
                          [vertexPositionsBuffer, vertexColorsBuffer, indexBuffer](RenderContext& renderContext) {
                              renderContext.bindVertexBuffers({vertexPositionsBuffer, vertexColorsBuffer});
                              renderContext.bindIndexBuffer(indexBuffer);
                              renderContext.drawIndexed(36, 1, 0, 0);
                          });

    Global::graphicsManager.setRenderGraph(graph);

    resourseSet = Global::graphicsManager.createResourceSet("Pass", 0);
    resourseSet.get()->assignBuffer(viewProjBuffer, 0);

    while (Global::windowManager.update())
    {
        Global::timer.update();

        static float scaler = 10.0f;
        scaler += 5.0f * Global::timer.dt();
        if (scaler > 100.0f)
        {
            scaler = 10.0f;
        }

        modelMatrix = scale(matrix4(1.0f), vec3(scaler / 100.0f));

        matrix4 mat = rotateY(matrix4(1.0f), Global::timer.secondsElapsed());
        vec4 eye = mat * vec4(0.0f, 0.0f, -3.0f, 0.0f);

        rect = window.get()->getRect();
        matrix4 viewProj =
            perspective(radians(90.0f), static_cast<float>(rect.screenWidth) / static_cast<float>(rect.screenHeight),
                        vec2(0.1f, 100.0f)) *
            lookAt(vec3(eye.x, eye.y, eye.z), vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));

        viewProjBuffer.get()->write(sizeof(viewProj), &viewProj);

        Global::graphicsManager.render();

        if (Global::input.isKeyPressed(Keys::A))
        {
            log(LogLevel::INFO, "Pressed A");
        }
        else if (Global::input.isKeyReleased(Keys::A))
        {
            log(LogLevel::INFO, "Released A");
        }

        if (Global::input.isKeyToggled(KeyToggles::CAPS_LOCK))
        {
            log(LogLevel::INFO, "Caps is on");
        }

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
        Global::input.update();
    }

    Global::graphicsManager.cleanup();
    Global::windowManager.cleanup();

#ifdef DEBUG
    ReferenceCounter::reportState();
#endif
}