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
            transform.position = vec3(-(numEnities / 2.0f) + static_cast<float>(x) + 0.5f,
                                      -(numEnities / 2.0f) + static_cast<float>(y) + 0.5f, 0.0f) *
                                 3.0f;
            transform.rotation =
                vec3(math::radians(15) * static_cast<float>(x), math::radians(15) * static_cast<float>(y), 0.0f);
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
        BufferType::DYNAMIC, HU_BUFFER_USAGE_CONSTANT_BUFFER, sizeof(viewProj), &viewProj);

    TextureData& tex = global::resourceManager.loadTextureData("assets/textures/test.png", TexelChannelFormat::RGBA);
    Ref<Texture> texture = global::graphicsManager.createTexture(tex);

    ShaderModule& shaderModule = global::resourceManager.loadShaderModule("assets/shaders/shader.slang");
    PipelineBuilder builder;
    builder.init(PipelineType::GRAPHICS)
        .addShader(shaderModule, "vertMain")
        .addShader(shaderModule, "fragMain")
        .addVertexInputStream({.size = sizeof(vec3),
                               .inputRate = VertexInputRate::VERTEX,
                               .attributes{{.format = GraphicsDataFormat::RGB_32_FLOAT, .offset = 0}}})
        .addVertexInputStream({.size = sizeof(vec2),
                               .inputRate = VertexInputRate::VERTEX,
                               .attributes{{.format = GraphicsDataFormat::RG_32_FLOAT, .offset = 0}}})
        .addVertexInputStream({.size = sizeof(vec3),
                               .inputRate = VertexInputRate::VERTEX,
                               .attributes{{.format = GraphicsDataFormat::RGB_32_FLOAT, .offset = 0}}})
        .setPrimitive(PrimitiveType::TRIANGLE, PrimitiveLayout::TRIANGLE_LIST);

    RenderCommands commands = [&meshes, positionsBuffer, uvsBuffer, normalsBuffer, indexBuffer, viewProjBuffer, texture,
                               numEnities](RenderContext& renderContext) {
        renderContext.bindVertexBuffers({positionsBuffer, uvsBuffer, normalsBuffer});
        renderContext.bindIndexBuffer(indexBuffer);
        renderContext.bindBuffer(viewProjBuffer, "cameraBuffer");
        renderContext.bindTexture(texture, "resources.texture");
        renderContext.bindSampler(SAMPLER_LINEAR, "resources.sampler");

        global::sceneManager.query<Transform>([&](Transform& transform) {
            transform.rotation += vec3(math::radians(5), math::radians(10), 0.0f) * global::timer.dt();
            matrix4 mat = transform.applyMatrix();
            renderContext.setParameter(&mat, sizeof(matrix4), "model");
            renderContext.drawIndexed(static_cast<u32>(meshes[0].indices.size()), 1, 0, 0);
        });
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
            rot -= vec3(static_cast<float>(mouseDt.y), static_cast<float>(mouseDt.x), 0.0f) * 1.0f * global::timer.dt();
        }
        else
        {
            rot += vec3(static_cast<float>(global::input.isKeyDown(Keys::I)) -
                            static_cast<float>(global::input.isKeyDown(Keys::K)),
                        static_cast<float>(global::input.isKeyDown(Keys::J)) -
                            static_cast<float>(global::input.isKeyDown(Keys::L)),
                        static_cast<float>(global::input.isKeyDown(Keys::O)) -
                            static_cast<float>(global::input.isKeyDown(Keys::U))) *
                   1.0f * global::timer.dt();
        }

        matrix3 rMat = math::rotateZ(matrix3(1.0f), rot.z) * math::rotateY(matrix3(1.0f), rot.y) *
                       math::rotateX(matrix3(1.0f), rot.x);

        vec3 right = vec3(rMat(0, 0), rMat(1, 0), rMat(2, 0));
        vec3 up = vec3(rMat(0, 1), rMat(1, 1), rMat(2, 1));
        vec3 forward = vec3(rMat(0, 2), rMat(1, 2), rMat(2, 2));

        float eyeSpeed = 5.0f + (10.0f * static_cast<float>(global::input.isKeyDown(Keys::SHIFT)));
        eye += ((static_cast<float>(global::input.isKeyDown(Keys::D)) -
                 static_cast<float>(global::input.isKeyDown(Keys::A))) *
                    right +
                (static_cast<float>(global::input.isKeyDown(Keys::Q)) -
                 static_cast<float>(global::input.isKeyDown(Keys::E))) *
                    up +
                (static_cast<float>(global::input.isKeyDown(Keys::S)) -
                 static_cast<float>(global::input.isKeyDown(Keys::W))) *
                    forward) *
               eyeSpeed * global::timer.dt();

        RenderGraphBuilder renderGraph;
        if (window.valid() && window->getRenderTarget()->isAvailable())
        {
            RenderPassBuilder renderPass =
                RenderPassBuilder()
                    .init(RenderPassType::GRAPHICS, builder)
                    .addResource(ResourceAccessType::READ, viewProjBuffer, ShaderStage::VERTEX)
                    .addResource(ResourceAccessType::READ, texture, ShaderStage::FRAGMENT)
                    .addRenderTarget(window->getRenderTarget())
                    .setCommands(commands);
            renderGraph.addPass("Render Pass", renderPass);
        }

        viewProj = math::perspective(math::radians(90.0f),
                                     static_cast<float>(rect.screenWidth) / static_cast<float>(rect.screenHeight),
                                     vec2(0.1f, 100.0f)) *
                   math::lookTo(eye, -forward, up);

        viewProjBuffer->write(sizeof(viewProj), &viewProj);

        global::graphicsManager.render(renderGraph);

        if (global::input.isKeyActive(KeyToggles::CAPS_LOCK))
        {
            log(LogLevel::D_INFO, "Caps is on");
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

            log(LogLevel::D_INFO, "Elapsed: {:.5f}, Delta: {:.5f}, FPS: {}", global::timer.secondsElapsed(),
                global::timer.dt(), sum / 500);
            i = 0;

            if (window.valid())
            {
                window->setTitle(std::format("Main FPS: {}", sum / 500));
            }
        }
        global::input.update();
    }

    global::graphicsManager.removeTexture(texture);
    global::graphicsManager.removeBuffer(viewProjBuffer);

    global::resourceManager.cleanup();
    global::graphicsManager.cleanup();
    global::windowManager.cleanup();

#ifdef DEBUG
    ReferenceCounter::reportState();
#endif
}