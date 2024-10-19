#include "core/file/utils.hpp"
#include "core/global.hpp"
#include "core/log.hpp"
#include "core/serialization/json.hpp"
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
    Ref<Window> window1 = Global::windowManager.addWindow("Hello", huedra::WindowInput(300, 300, 100, 100), window);

    // Draw data
    std::vector<MeshData> meshes = loadGltf("assets/mesh/untitled.gltf");

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

    Ref<ResourceSet> resourseSet(nullptr);

    PipelineBuilder builder;
    builder.init(PipelineType::GRAPHICS)
        .addShader(ShaderStage::VERTEX, "assets/shaders/shader.vert")
        .addShader(ShaderStage::FRAGMENT, "assets/shaders/shader.frag")
        .addVertexInputStream({sizeof(vec3), VertexInputRate::VERTEX, {{GraphicsDataFormat::RGB_32_FLOAT, 0}}})
        .addVertexInputStream({sizeof(vec2), VertexInputRate::VERTEX, {{GraphicsDataFormat::RG_32_FLOAT, 0}}})
        .addVertexInputStream({sizeof(vec3), VertexInputRate::VERTEX, {{GraphicsDataFormat::RGB_32_FLOAT, 0}}})
        .addPushConstantRange(HU_SHADER_STAGE_VERTEX, sizeof(modelMatrix))
        .addResourceSet()
        .addResourceBinding(HU_SHADER_STAGE_VERTEX, ResourceType::UNIFORM_BUFFER)
        .addResourceBinding(HU_SHADER_STAGE_FRAGMENT, ResourceType::TEXTURE);

    RenderPassSettings settings;
    settings.clearColor = {{0.2f, 0.2f, 0.2f}};

    RenderGraphBuilder graph;
    graph.init().addGraphicsPass(
        "Pass", builder, window->getRenderTarget(),
        [&meshes, positionsBuffer, uvsBuffer, normalsBuffer, indexBuffer, &resourseSet,
         &modelMatrix](RenderContext& renderContext) {
            renderContext.bindVertexBuffers({positionsBuffer, uvsBuffer, normalsBuffer});
            renderContext.bindIndexBuffer(indexBuffer);
            renderContext.bindResourceSet(resourseSet);
            renderContext.pushConstants(HU_SHADER_STAGE_VERTEX, sizeof(modelMatrix), &modelMatrix);
            renderContext.drawIndexed(static_cast<u32>(meshes[0].indices.size()), 1, 0, 0);
        },
        {}, settings);

    graph.addGraphicsPass("Pass1", builder, window1->getRenderTarget(),
                          [&meshes, positionsBuffer, uvsBuffer, normalsBuffer, indexBuffer, &resourseSet,
                           &modelMatrix](RenderContext& renderContext) {
                              renderContext.bindVertexBuffers({positionsBuffer, uvsBuffer, normalsBuffer});
                              renderContext.bindIndexBuffer(indexBuffer);
                              renderContext.bindResourceSet(resourseSet);
                              renderContext.pushConstants(HU_SHADER_STAGE_VERTEX, sizeof(modelMatrix), &modelMatrix);
                              renderContext.drawIndexed(static_cast<u32>(meshes[0].indices.size()), 1, 0, 0);
                          });

    Global::graphicsManager.setRenderGraph(graph);

    resourseSet = Global::graphicsManager.createResourceSet("Pass", 0);
    resourseSet->assignBuffer(viewProjBuffer, 0);
    resourseSet->assignTexture(texture, 1);

    vec3 eye(0.0f, 0.0f, 5.0f);
    vec3 rot(0.0f);
    while (Global::windowManager.update())
    {
        Global::timer.update();

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

        rect = window->getRect();
        matrix4 viewProj =
            perspective(radians(90.0f), static_cast<float>(rect.screenWidth) / static_cast<float>(rect.screenHeight),
                        vec2(0.1f, 100.0f)) *
            lookTo(eye, -forward, up);

        viewProjBuffer->write(sizeof(viewProj), &viewProj);

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