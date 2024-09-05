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

    std::array<u8, 4> textureData = {{0, 192, 128, 255}};
    Ref<Texture> texture = Global::graphicsManager.createTexture(1, 1, GraphicsDataFormat::RGBA_8_UNORM, sizeof(u8) * 4,
                                                                 textureData.data());

    Ref<ResourceSet> resourseSet(nullptr);

    PipelineBuilder builder;
    builder.init(PipelineType::GRAPHICS)
        .addShader(ShaderStage::VERTEX, "shaders/shader.vert")
        .addShader(ShaderStage::FRAGMENT, "shaders/shader.frag")
        .addVertexInputStream({sizeof(vec3), VertexInputRate::VERTEX, {{GraphicsDataFormat::RGB_32_FLOAT, 0}}})
        .addVertexInputStream({sizeof(vec3), VertexInputRate::VERTEX, {{GraphicsDataFormat::RGB_32_FLOAT, 0}}})
        .addPushConstantRange(HU_SHADER_STAGE_VERTEX, sizeof(modelMatrix))
        .addResourceSet()
        .addResourceBinding(HU_SHADER_STAGE_VERTEX, ResourceType::UNIFORM_BUFFER)
        .addResourceBinding(HU_SHADER_STAGE_FRAGMENT, ResourceType::TEXTURE);

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
            renderContext.drawIndexed(36, 2, 0, 0);
        },
        {}, settings);

    graph.addGraphicsPass("Pass1", builder, window1.get()->getRenderTarget(),
                          [vertexPositionsBuffer, vertexColorsBuffer, indexBuffer](RenderContext& renderContext) {
                              renderContext.bindVertexBuffers({vertexPositionsBuffer, vertexColorsBuffer});
                              renderContext.bindIndexBuffer(indexBuffer);
                              renderContext.drawIndexed(36, 2, 0, 0);
                          });

    Global::graphicsManager.setRenderGraph(graph);

    resourseSet = Global::graphicsManager.createResourceSet("Pass", 0);
    resourseSet.get()->assignBuffer(viewProjBuffer, 0);
    resourseSet.get()->assignTexture(texture, 1);

    vec3 eye(0.0f, 0.0f, -5.0f);
    vec3 rot(0.0f);
    while (Global::windowManager.update())
    {
        Global::timer.update();

        modelMatrix = rotateY(matrix4(1.0f), Global::timer.secondsElapsed());

        rot += vec3(Global::input.isKeyDown(Keys::K) - Global::input.isKeyDown(Keys::I),
                    Global::input.isKeyDown(Keys::J) - Global::input.isKeyDown(Keys::L),
                    Global::input.isKeyDown(Keys::U) - Global::input.isKeyDown(Keys::O)) *
               1.0f * Global::timer.dt();

        matrix3 rMat = rotateZ(matrix3(1.0f), rot.z) * rotateY(matrix3(1.0f), rot.y) * rotateX(matrix3(1.0f), rot.x);

        vec3 right = vec3(rMat(0, 0), rMat(1, 0), rMat(2, 0));
        vec3 up = vec3(rMat(0, 1), rMat(1, 1), rMat(2, 1));
        vec3 forward = vec3(rMat(0, 2), rMat(1, 2), rMat(2, 2));

        eye += (static_cast<float>(Global::input.isKeyDown(Keys::A) - Global::input.isKeyDown(Keys::D)) * right +
                static_cast<float>(Global::input.isKeyDown(Keys::Q) - Global::input.isKeyDown(Keys::E)) * up +
                static_cast<float>(Global::input.isKeyDown(Keys::W) - Global::input.isKeyDown(Keys::S)) * forward) *
               5.0f * Global::timer.dt();

        rect = window.get()->getRect();
        matrix4 viewProj =
            perspective(radians(90.0f), static_cast<float>(rect.screenWidth) / static_cast<float>(rect.screenHeight),
                        vec2(0.1f, 100.0f)) *
            lookTo(eye, forward, up);

        viewProjBuffer.get()->write(sizeof(viewProj), &viewProj);

        Global::graphicsManager.render();

        if (Global::input.isKeyActive(KeyToggles::CAPS_LOCK))
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