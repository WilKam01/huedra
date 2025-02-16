#include "core/file/utils.hpp"
#include "core/global.hpp"
#include "core/log.hpp"
#include "core/serialization/json.hpp"
#include "core/string/utils.hpp"
#include "graphics/render_pass_builder.hpp"
#include "math/conversions.hpp"
#include "math/matrix_projection.hpp"
#include "math/matrix_transform.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "resources/mesh/loader.hpp"
#include "resources/texture/loader.hpp"
#include "scene/components/transform.hpp"

using namespace huedra;

int main()
{
    global::timer.init();
    global::windowManager.init();
    global::graphicsManager.init();
    global::resourceManager.init();

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

    Ref<Window> window = global::windowManager.addWindow("Main", WindowInput(1278, 1360, -7, 0));

    // Draw data
    std::vector<MeshData>& meshes = global::resourceManager.loadMeshData("assets/mesh/untitled.glb");

    if (meshes.empty())
    {
        log(LogLevel::ERR, "meshes array is empty");
    }
    if (meshes[0].uvs.empty())
    {
        log(LogLevel::ERR, "Imported mesh: {} has no uv coordinates", meshes[0].name.c_str());
    }
    if (meshes[0].normals.empty())
    {
        log(LogLevel::ERR, "Imported mesh: {} has no normals", meshes[0].name.c_str());
    }

    Ref<Buffer> positionsBuffer =
        global::graphicsManager.createBuffer(BufferType::STATIC, HU_BUFFER_USAGE_VERTEX_BUFFER,
                                             sizeof(vec3) * meshes[0].positions.size(), meshes[0].positions.data());

    Ref<Buffer> uvsBuffer = global::graphicsManager.createBuffer(
        BufferType::STATIC, HU_BUFFER_USAGE_VERTEX_BUFFER, sizeof(vec2) * meshes[0].uvs.size(), meshes[0].uvs.data());

    Ref<Buffer> normalsBuffer =
        global::graphicsManager.createBuffer(BufferType::STATIC, HU_BUFFER_USAGE_VERTEX_BUFFER,
                                             sizeof(vec3) * meshes[0].normals.size(), meshes[0].normals.data());
    Ref<Buffer> indexBuffer =
        global::graphicsManager.createBuffer(BufferType::STATIC, HU_BUFFER_USAGE_INDEX_BUFFER,
                                             sizeof(u32) * meshes[0].indices.size(), meshes[0].indices.data());

    // Scene Entities
    const u32 numEnities = 7;
    for (u32 x = 0; x < numEnities; ++x)
    {
        for (u32 y = 0; y < numEnities; ++y)
        {
            Entity e = global::sceneManager.addEntity();
            Transform transform;
            transform.position = vec3(-(numEnities / 2.0f) + x + 0.5f, -(numEnities / 2.0f) + y + 0.5f, 0.0f) * 3.0f;
            transform.rotation = vec3(math::radians(15) * x, math::radians(15) * y, 0.0f);
            transform.scale = vec3(1.0f);
            global::sceneManager.setComponent(e, transform);
        }
    }

    // Shader Resources
    WindowRect rect = window->getRect();
    matrix4 viewProj = math::perspective(math::radians(45),
                                         static_cast<float>(rect.screenWidth) / static_cast<float>(rect.screenHeight),
                                         vec2(0.1f, 100.0f)) *
                       math::lookAt(vec3(0.0f, 0.0f, -5.0f), vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));

    Ref<Buffer> viewProjBuffer = global::graphicsManager.createBuffer(
        BufferType::DYNAMIC, HU_BUFFER_USAGE_UNIFORM_BUFFER, sizeof(viewProj), &viewProj);

    TextureData& tex = global::resourceManager.loadTextureData("assets/textures/1x1.png", TexelChannelFormat::RGBA);
    Ref<Texture> texture = global::graphicsManager.createTexture(tex);

    const u32 gBufferCount = 3;
    uvec2 gBufferDimensions(rect.screenWidth, rect.screenHeight);
    std::array<Ref<RenderTarget>, gBufferCount> gBuffers{};
    for (auto& gBuffer : gBuffers)
    {
        gBuffer = global::graphicsManager.createRenderTarget(RenderTargetType::COLOR_AND_DEPTH,
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
                               numEnities](RenderContext& renderContext) {
        renderContext.bindVertexBuffers({positionsBuffer, uvsBuffer, normalsBuffer});
        renderContext.bindIndexBuffer(indexBuffer);
        renderContext.bindBuffer(viewProjBuffer, 0, 0);
        renderContext.bindTexture(texture, 0, 1, SAMPLER_LINEAR);

        global::sceneManager.query<Transform>([&](Transform& transform) {
            transform.rotation += vec3(math::radians(5), math::radians(10), 0.0f) * global::timer.dt();
            matrix4 mat = transform.applyMatrix();
            renderContext.pushConstants(HU_SHADER_STAGE_VERTEX, sizeof(matrix4), &mat);
            renderContext.drawIndexed(static_cast<u32>(meshes[0].indices.size()), 1, 0, 0);
        });
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
    } lightData{vec4(0.0f, 10.0f, 5.0f, 1.0f), vec4(0.2f, 0.1f, 0.5f, 2.0f)};

    Ref<Buffer> computeBuffer = global::graphicsManager.createBuffer(
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

    vec3 eye(0.0f, 0.0f, 12.0f);
    vec3 rot(0.0f);
    while (global::windowManager.update())
    {
        global::timer.update();
        global::graphicsManager.update();

        static bool lock = false;
        if (global::input.isKeyPressed(Keys::ESCAPE))
        {
            lock = !lock;
            if (lock)
            {
                global::input.setMouseMode(MouseMode::LOCKED);
            }
            else
            {
                global::input.setMouseMode(MouseMode::NORMAL);
            }
            global::input.setMouseHidden(lock);
        }

        if (global::input.isKeyPressed(Keys::ENTER))
        {
            global::input.setCursor(static_cast<CursorType>((static_cast<u32>(global::input.getCursor()) + 1) % 13));
        }

        if (lock)
        {
            ivec2 mouseDt = global::input.getMouseDelta();
            rot -= vec3(mouseDt.y, mouseDt.x, 0.0f) * 1.0f * global::timer.dt();
        }
        else
        {
            rot += vec3(global::input.isKeyDown(Keys::K) - global::input.isKeyDown(Keys::I),
                        global::input.isKeyDown(Keys::J) - global::input.isKeyDown(Keys::L),
                        global::input.isKeyDown(Keys::O) - global::input.isKeyDown(Keys::U)) *
                   1.0f * global::timer.dt();
        }

        matrix3 rMat = math::rotateZ(matrix3(1.0f), rot.z) * math::rotateY(matrix3(1.0f), rot.y) *
                       math::rotateX(matrix3(1.0f), rot.x);

        vec3 right = vec3(rMat(0, 0), rMat(1, 0), rMat(2, 0));
        vec3 up = vec3(rMat(0, 1), rMat(1, 1), rMat(2, 1));
        vec3 forward = vec3(rMat(0, 2), rMat(1, 2), rMat(2, 2));

        float eyeSpeed = 5.0f + 10.0f * global::input.isKeyDown(Keys::SHIFT);
        eye += (static_cast<float>(global::input.isKeyDown(Keys::D) - global::input.isKeyDown(Keys::A)) * right +
                static_cast<float>(global::input.isKeyDown(Keys::Q) - global::input.isKeyDown(Keys::E)) * up +
                static_cast<float>(global::input.isKeyDown(Keys::S) - global::input.isKeyDown(Keys::W)) * forward) *
               eyeSpeed * global::timer.dt();

        RenderGraphBuilder renderGraph;
        if (window.valid() && window->getRenderTarget()->isAvailable())
        {
            WindowRect newRect = window->getRect();
            if (newRect.screenWidth != rect.screenWidth || newRect.screenHeight != rect.screenHeight)
            {
                rect = newRect;
                for (auto& gBuffer : gBuffers)
                {
                    global::graphicsManager.removeRenderTarget(gBuffer);
                    gBuffer = global::graphicsManager.createRenderTarget(RenderTargetType::COLOR_AND_DEPTH,
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

        viewProj = math::perspective(math::radians(90.0f),
                                     static_cast<float>(rect.screenWidth) / static_cast<float>(rect.screenHeight),
                                     vec2(0.1f, 100.0f)) *
                   math::lookTo(eye, -forward, up);

        viewProjBuffer->write(sizeof(viewProj), &viewProj);

        global::graphicsManager.render(renderGraph);

        if (global::input.isKeyActive(KeyToggles::CAPS_LOCK))
        {
            log(LogLevel::INFO, "Caps is on");
        }

        static u32 i = 0;
        static std::array<u32, 500> avgFps;

        avgFps[i++] = static_cast<u32>(1.0f / global::timer.dt());
        if (i >= 500)
        {
            u32 sum = 0;
            for (auto& fps : avgFps)
            {
                sum += fps;
            }

            log(LogLevel::INFO, "Elapsed: {:.5f}, Delta: {:.5f}, FPS: {}", global::timer.secondsElapsed(),
                global::timer.dt(), sum / 500);
            i = 0;
        }
        global::input.update();
    }

    global::graphicsManager.removeBuffer(computeBuffer);
    global::graphicsManager.removeTexture(texture);
    global::graphicsManager.removeBuffer(viewProjBuffer);

    global::resourceManager.cleanup();
    global::graphicsManager.cleanup();
    global::windowManager.cleanup();

#ifdef DEBUG
    ReferenceCounter::reportState();
#endif
}