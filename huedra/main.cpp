#include "core/file/utils.hpp"
#include "core/global.hpp"
#include "core/log.hpp"
#include "core/serialization/json.hpp"
#include "graphics/render_pass_builder.hpp"
#include "math/conversions.hpp"
#include "math/matrix_projection.hpp"
#include "math/matrix_transform.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "resources/mesh/loader.hpp"
#include "resources/texture/loader.hpp"

using namespace huedra;

int main()
{
    Global::timer.init();
    Global::windowManager.init();
    Global::graphicsManager.init();

    JsonObject json = parseJson(readBytes("assets/test.json"));

    json["arr"][5] = 5;
    json["strings"][4] = "test";
    json["arrays"][3] = JsonArray();
    json["arrays"][3][0] = 4;
    json["arrays"][3][1] = 5;
    json["arrays"][3][2] = 6;
    json["objects"][3] = JsonObject();
    json["objects"][3]["test1"] = 2;

    writeBytes("assets/result.json", serializeJson(json));

    Ref<Window> window = Global::windowManager.addWindow("Hello Windows!", huedra::WindowInput(1278, 1360, -7, 0));

    // Draw data
    std::vector<MeshData> meshes = loadGlb("assets/mesh/untitled.glb");

    if (meshes.empty())
    {
        log(LogLevel::ERR, "meshes array is empty");
    }

    if (meshes[0].uvs.empty())
    {
        log(LogLevel::ERR, "Imported mesh: %s has no uv coordinates", meshes[0].name.c_str());
    }
    if (meshes[0].normals.empty())
    {
        log(LogLevel::ERR, "Imported mesh: %s has no normals", meshes[0].name.c_str());
    }

    Ref<Buffer> positionsBuffer =
        Global::graphicsManager.createBuffer(BufferType::STATIC, HU_BUFFER_USAGE_VERTEX_BUFFER,
                                             sizeof(vec3) * meshes[0].positions.size(), meshes[0].positions.data());

    Ref<Buffer> uvsBuffer = Global::graphicsManager.createBuffer(
        BufferType::STATIC, HU_BUFFER_USAGE_VERTEX_BUFFER, sizeof(vec2) * meshes[0].uvs.size(), meshes[0].uvs.data());

    Ref<Buffer> normalsBuffer =
        Global::graphicsManager.createBuffer(BufferType::STATIC, HU_BUFFER_USAGE_VERTEX_BUFFER,
                                             sizeof(vec3) * meshes[0].normals.size(), meshes[0].normals.data());
    Ref<Buffer> indexBuffer =
        Global::graphicsManager.createBuffer(BufferType::STATIC, HU_BUFFER_USAGE_INDEX_BUFFER,
                                             sizeof(u32) * meshes[0].indices.size(), meshes[0].indices.data());

    // Shader Resources
    WindowRect rect = window->getRect();
    matrix4 viewProj =
        perspective(radians(45.0f), static_cast<float>(rect.screenWidth) / static_cast<float>(rect.screenHeight),
                    vec2(0.1f, 100.0f)) *
        lookAt(vec3(0.0f, 0.0f, -5.0f), vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));

    matrix4 modelMatrix(1.0f);

    Ref<Buffer> viewProjBuffer = Global::graphicsManager.createBuffer(
        BufferType::DYNAMIC, HU_BUFFER_USAGE_UNIFORM_BUFFER, sizeof(viewProj), &viewProj);

    TextureData png = loadPng("assets/textures/test.png", TexelChannelFormat::RGBA);

    std::vector<u8> texData = {{0x11, 0xf1, 0x11, 0xf1, 0x11, 0xf1, 0xff, 0xff}};
    TextureData textureData{1, 1, GraphicsDataFormat::RGBA_16_UNORM, sizeof(u8) * 8, texData};
    Ref<Texture> texture = Global::graphicsManager.createTexture(png);

    const u32 gBufferCount = 3;
    uvec2 gBufferDimensions(rect.screenWidth, rect.screenHeight);
    std::array<Ref<RenderTarget>, gBufferCount> gBuffers{};
    for (auto& gBuffer : gBuffers)
    {
        gBuffer = Global::graphicsManager.createRenderTarget(RenderTargetType::COLOR_AND_DEPTH,
                                                             GraphicsDataFormat::RGBA_8_UNORM, gBufferDimensions.x,
                                                             gBufferDimensions.y);
    }

    PipelineBuilder builder;
    builder.init(PipelineType::GRAPHICS)
        .addShader(ShaderStage::VERTEX, "assets/shaders/shader.vert")
        .addShader(ShaderStage::FRAGMENT, "assets/shaders/shader.frag")
        .addVertexInputStream({sizeof(vec3), VertexInputRate::VERTEX, {{GraphicsDataFormat::RGB_32_FLOAT, 0}}})
        .addVertexInputStream({sizeof(vec2), VertexInputRate::VERTEX, {{GraphicsDataFormat::RG_32_FLOAT, 0}}})
        .addVertexInputStream({sizeof(vec3), VertexInputRate::VERTEX, {{GraphicsDataFormat::RGB_32_FLOAT, 0}}})
        .addPushConstantRange(HU_SHADER_STAGE_VERTEX, sizeof(matrix4))
        .addResourceSet()
        .addResourceBinding(HU_SHADER_STAGE_VERTEX, ResourceType::UNIFORM_BUFFER)
        .addResourceBinding(HU_SHADER_STAGE_FRAGMENT, ResourceType::UNFIFORM_TEXTURE);

    RenderCommands commands = [&meshes, positionsBuffer, uvsBuffer, normalsBuffer, indexBuffer, viewProjBuffer, texture,
                               &modelMatrix](RenderContext& renderContext) {
        renderContext.bindVertexBuffers({positionsBuffer, uvsBuffer, normalsBuffer});
        renderContext.bindIndexBuffer(indexBuffer);
        renderContext.bindBuffer(viewProjBuffer, 0, 0);
        renderContext.bindTexture(texture, 0, 1, SAMPLER_LINEAR);
        renderContext.pushConstants(HU_SHADER_STAGE_VERTEX, sizeof(matrix4), &modelMatrix);
        renderContext.drawIndexed(static_cast<u32>(meshes[0].indices.size()), 1, 0, 0);
    };

    // Compute pipeline and info
    PipelineBuilder computeBuilder;
    computeBuilder.init(PipelineType::COMPUTE)
        .addShader(ShaderStage::COMPUTE, "assets/shaders/shader.comp")
        .addResourceSet();

    for (auto& gBuffer : gBuffers)
    {
        computeBuilder.addResourceBinding(HU_SHADER_STAGE_COMPUTE, ResourceType::UNFIFORM_TEXTURE);
    }

    computeBuilder.addResourceBinding(HU_SHADER_STAGE_COMPUTE, ResourceType::UNIFORM_BUFFER)
        .addResourceBinding(HU_SHADER_STAGE_COMPUTE, ResourceType::STORAGE_TEXTURE);

    struct LightData
    {
        vec4 pos;
        vec4 color;
    } lightData{vec4(0.0f, 10.0f, 5.0f, 1.0f), vec4(0.5f, 0.1f, 0.5f, 3.0f)};

    Ref<Buffer> computeBuffer = Global::graphicsManager.createBuffer(
        BufferType::DYNAMIC, HU_BUFFER_USAGE_UNIFORM_BUFFER, sizeof(LightData), &lightData);

    RenderCommands computeCommands = [&gBuffers, &computeBuffer, &window](RenderContext& renderContext) {
        for (u32 i = 0; i < gBufferCount; ++i)
        {
            renderContext.bindTexture(gBuffers[i]->getColorTexture(), 0, i, SAMPLER_NEAR);
        }
        renderContext.bindBuffer(computeBuffer, 0, gBufferCount);
        renderContext.bindTexture(window->getRenderTarget()->getColorTexture(), 0, gBufferCount + 1, SAMPLER_NEAR);
        WindowRect rect = window->getRect();
        renderContext.dispatch((rect.width + 31) / 32, (rect.height + 31) / 32, 1);
    };

    vec3 eye(0.0f, 0.0f, 5.0f);
    vec3 rot(0.0f);
    while (Global::windowManager.update())
    {
        Global::timer.update();
        Global::graphicsManager.update();

        modelMatrix = matrix4(1.0f);

        rot += vec3(Global::input.isKeyDown(Keys::K) - Global::input.isKeyDown(Keys::I),
                    Global::input.isKeyDown(Keys::J) - Global::input.isKeyDown(Keys::L),
                    Global::input.isKeyDown(Keys::O) - Global::input.isKeyDown(Keys::U)) *
               1.0f * Global::timer.dt();

        matrix3 rMat = rotateZ(matrix3(1.0f), rot.z) * rotateY(matrix3(1.0f), rot.y) * rotateX(matrix3(1.0f), rot.x);

        vec3 right = vec3(rMat(0, 0), rMat(1, 0), rMat(2, 0));
        vec3 up = vec3(rMat(0, 1), rMat(1, 1), rMat(2, 1));
        vec3 forward = vec3(rMat(0, 2), rMat(1, 2), rMat(2, 2));

        eye += (static_cast<float>(Global::input.isKeyDown(Keys::D) - Global::input.isKeyDown(Keys::A)) * right +
                static_cast<float>(Global::input.isKeyDown(Keys::Q) - Global::input.isKeyDown(Keys::E)) * up +
                static_cast<float>(Global::input.isKeyDown(Keys::S) - Global::input.isKeyDown(Keys::W)) * forward) *
               5.0f * Global::timer.dt();

        static Keys selectedWindow{Keys::_0};
        if (Global::input.isKeyPressed(Keys::_0))
        {
            selectedWindow = Keys::_0;
        }
        else if (Global::input.isKeyPressed(Keys::_1))
        {
            selectedWindow = Keys::_1;
        }
        else if (Global::input.isKeyPressed(Keys::_2))
        {
            selectedWindow = Keys::_2;
        }

        RenderGraphBuilder renderGraph;
        if (window.valid() && window->getRenderTarget()->isAvailable() && selectedWindow != Keys::_2)
        {
            WindowRect newRect = window->getRect();
            if (newRect.screenWidth != rect.screenWidth || newRect.screenHeight != rect.screenHeight)
            {
                rect = newRect;
                for (auto& gBuffer : gBuffers)
                {
                    Global::graphicsManager.removeRenderTarget(gBuffer);
                    gBuffer = Global::graphicsManager.createRenderTarget(RenderTargetType::COLOR_AND_DEPTH,
                                                                         GraphicsDataFormat::RGBA_8_UNORM,
                                                                         rect.screenWidth, rect.screenHeight);
                }
            }

            RenderPassBuilder renderPass =
                RenderPassBuilder()
                    .init(RenderPassType::GRAPHICS, builder)
                    .addResource(ResourceAccessType::READ, viewProjBuffer, HU_SHADER_STAGE_VERTEX)
                    .addResource(ResourceAccessType::READ, texture, HU_SHADER_STAGE_FRAGMENT)
                    .setCommands(commands);
            for (auto& gBuffer : gBuffers)
            {
                renderPass.addRenderTarget(gBuffer);
            }
            renderGraph.addPass("GBuffer Pass", renderPass);

            renderPass.init(RenderPassType::COMPUTE, computeBuilder);
            for (auto& gBuffer : gBuffers)
            {
                renderPass.addResource(ResourceAccessType::READ, gBuffer->getColorTexture(), HU_SHADER_STAGE_COMPUTE);
            }
            renderPass.addResource(ResourceAccessType::WRITE, window->getRenderTarget()->getColorTexture(),
                                   HU_SHADER_STAGE_COMPUTE);
            renderPass.setCommands(computeCommands);
            renderGraph.addPass("Ligthning Pass", renderPass);
        }

        viewProj =
            perspective(radians(90.0f), static_cast<float>(rect.screenWidth) / static_cast<float>(rect.screenHeight),
                        vec2(0.1f, 100.0f)) *
            lookTo(eye, -forward, up);

        viewProjBuffer->write(sizeof(viewProj), &viewProj);

        Global::graphicsManager.render(renderGraph);

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

    Global::graphicsManager.removeBuffer(computeBuffer);
    Global::graphicsManager.removeTexture(texture);
    Global::graphicsManager.removeBuffer(viewProjBuffer);

    Global::graphicsManager.cleanup();
    Global::windowManager.cleanup();

#ifdef DEBUG
    ReferenceCounter::reportState();
#endif
}