#include "core/file/utils.hpp"
#include "core/global.hpp"
#include "core/log.hpp"
#include "core/serialization/json.hpp"
#include "core/string/utils.hpp"
#include "core/types.hpp"
#include "graphics/pipeline_builder.hpp"
#include "graphics/pipeline_data.hpp"
#include "graphics/render_context.hpp"
#include "graphics/render_pass_builder.hpp"
#include "graphics/render_target.hpp"
#include "math/conversions.hpp"
#include "math/matrix.hpp"
#include "math/matrix_projection.hpp"
#include "math/matrix_transform.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "resources/font/data.hpp"
#include "resources/font/loader.hpp"
#include "resources/mesh/loader.hpp"
#include "resources/texture/loader.hpp"
#include "scene/components/transform.hpp"
#include <cctype>

using namespace huedra;

int main()
{
    global::timer.init();
    global::windowManager.init();
    global::graphicsManager.init();
    global::resourceManager.init();

    FontData font = loadTtf("assets/fonts/OpenSans-Regular.ttf");

    Ref<Window> window = global::windowManager.addWindow("Main", WindowInput(1280, 720));

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

    ShaderModule& shaderModule = global::resourceManager.loadShaderModule("assets/shaders/deffered.slang");
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
        renderContext.bindBuffer(viewProjBuffer, "cameraMatrix");
        renderContext.bindTexture(texture, "resources.texture");
        renderContext.bindSampler(SAMPLER_LINEAR, "resources.sampler");

        global::sceneManager.query<Transform>([&](Transform& transform) {
            transform.rotation += vec3(math::radians(5), math::radians(10), 0.0f) * global::timer.dt();
            matrix4 mat = transform.applyMatrix();
            renderContext.setParameter(&mat, sizeof(matrix4), "modelMatrix");
            renderContext.drawIndexed(static_cast<u32>(meshes[0].indices.size()), 1, 0, 0);
        });
    };

    // Deffered compute pass resources
    struct
    {
        vec4 position;
        vec4 color;
    } lightData{.position = vec4(0.0f, 10.0f, 5.0f, 0.0f), .color = vec4(0.5f, 0.1f, 0.5f, 5.0f)};

    Ref<Buffer> lightBuffer = global::graphicsManager.createBuffer(BufferType::STATIC, HU_BUFFER_USAGE_CONSTANT_BUFFER,
                                                                   sizeof(lightData), &lightData);

    std::array<Ref<RenderTarget>, 3> gBuffers;
    for (auto& gBuffer : gBuffers)
    {
        gBuffer = global::graphicsManager.createRenderTarget(
            RenderTargetType::COLOR_AND_DEPTH, window->getRenderTarget()->getFormat(),
            window->getRenderTarget()->getWidth(), window->getRenderTarget()->getHeight());
    }

    PipelineBuilder computeBuilder;
    computeBuilder.init(PipelineType::COMPUTE).addShader(shaderModule, "computeMain");

    RenderCommands computeCommands = [&lightBuffer, &gBuffers, &window](RenderContext& renderContext) {
        renderContext.bindBuffer(lightBuffer, "lightBuffer");
        renderContext.bindTexture(gBuffers[0]->getColorTexture(), "gBuffers.position");
        renderContext.bindTexture(gBuffers[1]->getColorTexture(), "gBuffers.normal");
        renderContext.bindTexture(gBuffers[2]->getColorTexture(), "gBuffers.albedo");
        renderContext.bindTexture(window->getRenderTarget()->getColorTexture(), "outputTexture");
        renderContext.dispatch(window->getRenderTarget()->getWidth(), window->getRenderTarget()->getHeight(), 1);
    };

    ShaderModule& fontShaderModule = global::resourceManager.loadShaderModule("assets/shaders/font.slang");
    PipelineBuilder fontPipeline;
    fontPipeline.init(PipelineType::GRAPHICS)
        .addShader(fontShaderModule, "vertMain")
        .addShader(fontShaderModule, "fragMain")
        .addVertexInputStream({.size = sizeof(vec2),
                               .inputRate = huedra::VertexInputRate::VERTEX,
                               .attributes = {{.format = GraphicsDataFormat::RG_32_FLOAT, .offset = 0}}})
        .setPrimitive(PrimitiveType::LINE, PrimitiveLayout::LINE_LIST);

    struct alignas(16)
    {
        vec2 position;
        vec2 size;
        matrix4 projection;
    } fontInfo{.position = vec2(100.0f, static_cast<float>(window->getScreenSize().y) / 2.0f),
               .size = vec2((64.0f * 72.0f) / (72.0f * static_cast<float>(font.unitsPerEm))),
               .projection = math::ortho(vec2(0, static_cast<float>(window->getScreenSize().x)),
                                         vec2(0, static_cast<float>(window->getScreenSize().y)), vec2(0.0f, 1.0f))};
    Ref<Buffer> fontInfoBuffer = global::graphicsManager.createBuffer(
        BufferType::DYNAMIC, HU_BUFFER_USAGE_CONSTANT_BUFFER, sizeof(fontInfo), &fontInfo);

    // Only supporting ASCII for now
    std::array<Ref<Buffer>, 127> glyphVertexBuffers;
    std::array<Ref<Buffer>, 127> glyphIndexBuffers;
    std::array<u32, 127> glyphIndexCounts{{}};
    for (u32 i = 0; i < glyphVertexBuffers.size(); ++i)
    {
        Glyph& glyph = font.glyphs[font.characterMappings[i]];
        std::vector<vec2> points(glyph.points.size());
        for (u32 j = 0; j < points.size(); ++j)
        {
            points[j] = static_cast<vec2>(glyph.points[j].position);
        }
        glyphVertexBuffers[i] = global::graphicsManager.createBuffer(BufferType::STATIC, HU_BUFFER_USAGE_VERTEX_BUFFER,
                                                                     sizeof(vec2) * points.size(), points.data());
        std::vector<u32> indices(points.size() * 2, 0);
        u32 curIndicesIndex{0};
        u16 startOfCountour{0};
        for (auto& endIndex : glyph.contourEndPointIndices)
        {
            for (u16 j = startOfCountour; j <= endIndex; ++j)
            {
                indices[curIndicesIndex++] = j;
                indices[curIndicesIndex++] = j + 1;
            }
            indices[curIndicesIndex - 1] = startOfCountour; // Correct it to go back to start of contour
            startOfCountour = endIndex + 1;
        }
        glyphIndexBuffers[i] = global::graphicsManager.createBuffer(BufferType::STATIC, HU_BUFFER_USAGE_INDEX_BUFFER,
                                                                    sizeof(u32) * indices.size(), indices.data());
        glyphIndexCounts[i] = indices.size();
    }

    std::array<vec2, 2> cursorVertexValues{vec2(0.0f, 0.0f), vec2(0.0f, static_cast<float>(font.unitsPerEm))};
    Ref<Buffer> cursorVertexBuffer =
        global::graphicsManager.createBuffer(BufferType::STATIC, HU_BUFFER_USAGE_VERTEX_BUFFER,
                                             sizeof(vec2) * cursorVertexValues.size(), cursorVertexValues.data());

    std::array<u32, 2> cursorIndexValues{0, 1};
    Ref<Buffer> cursorIndexBuffer =
        global::graphicsManager.createBuffer(BufferType::STATIC, HU_BUFFER_USAGE_INDEX_BUFFER,
                                             sizeof(u32) * cursorIndexValues.size(), cursorIndexValues.data());

    std::string renderText{"hello world!"};
    bool renderCursor{true};
    u32 cursorIndex{static_cast<u32>(renderText.length())};
    Timer cursorBlinkTimer;
    cursorBlinkTimer.init();

    RenderCommands fontRenderCommands = [&fontInfoBuffer, &fontInfo, &glyphVertexBuffers, &glyphIndexBuffers,
                                         &glyphIndexCounts, &font, &renderText, &cursorVertexBuffer, &cursorIndexBuffer,
                                         &cursorIndexValues, &renderCursor,
                                         &cursorIndex](RenderContext& renderContext) {
        renderContext.bindBuffer(fontInfoBuffer, "info");
        float cursor{0.0f};
        float renderCursorOffset{0.0f};
        for (u32 i = 0; i < renderText.length(); ++i)
        {
            u32 index = static_cast<u32>(static_cast<unsigned char>(renderText[i]));
            u32 glyphIndex = font.characterMappings[index];

            // Render cursor
            if (i == cursorIndex && renderCursor)
            {
                renderCursorOffset = cursor + static_cast<float>(font.glyphs[glyphIndex].leftSideBearing);
            }

            if (renderText[i] != ' ')
            {
                // Render character
                float actualOffset = cursor * fontInfo.size.x;
                renderContext.bindVertexBuffers({glyphVertexBuffers[index]});
                renderContext.bindIndexBuffer(glyphIndexBuffers[index]);
                renderContext.setParameter(&actualOffset, sizeof(float), "offset");
                renderContext.drawIndexed(glyphIndexCounts[index], 1, 0, 0);
                cursor += static_cast<float>(font.glyphs[glyphIndex].advanceWidth);
            }
            else
            {
                cursor += 0.33f * static_cast<float>(font.unitsPerEm);
            }
        }

        // Render cursor
        if (renderCursor)
        {
            if (cursorIndex == renderText.length())
            {
                renderCursorOffset = cursor;
            }
            float actualOffset = (renderCursorOffset - 75.0f) * fontInfo.size.x;
            renderContext.bindVertexBuffers({cursorVertexBuffer});
            renderContext.bindIndexBuffer(cursorIndexBuffer);
            renderContext.setParameter(&actualOffset, sizeof(float), "offset");
            renderContext.drawIndexed(cursorIndexValues.size(), 1, 0, 0);
        }
    };

    vec3 eye(0.0f, 0.0f, 12.0f);
    vec3 rot(0.0f);
    uvec2 windowRenderTargetSize{window->getRenderTarget()->getSize()};
    while (global::windowManager.update())
    {
        global::timer.update();
        global::graphicsManager.update();

        cursorBlinkTimer.update();
        if (cursorBlinkTimer.passedInterval(static_cast<u64>(0.5f * static_cast<float>(Timer::SECONDS_TO_NANO))))
        {
            renderCursor = !renderCursor;
        }

        if (global::input.isKeyPressed(Keys::BACKSPACE) && cursorIndex != 0)
        {
            renderText.erase(--cursorIndex, 1);
        }
        else if (global::input.isKeyPressed(Keys::ARR_LEFT) && cursorIndex > 0)
        {
            --cursorIndex;
            renderCursor = true;
        }
        else if (global::input.isKeyPressed(Keys::ARR_RIGHT) && cursorIndex < renderText.length())
        {
            ++cursorIndex;
            renderCursor = true;
        }
        else if (static_cast<bool>(std::isprint(global::input.getCharacter())))
        {
            renderText.insert(cursorIndex++, 1, global::input.getCharacter());
        }

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
            // Window resized, need to recreate gBuffers
            if (windowRenderTargetSize != window->getRenderTarget()->getSize())
            {
                for (auto& gBuffer : gBuffers)
                {
                    global::graphicsManager.removeRenderTarget(gBuffer);
                    gBuffer = global::graphicsManager.createRenderTarget(
                        RenderTargetType::COLOR_AND_DEPTH, window->getRenderTarget()->getFormat(),
                        window->getRenderTarget()->getWidth(), window->getRenderTarget()->getHeight());
                }
                windowRenderTargetSize = window->getRenderTarget()->getSize();

                fontInfo.position.y = static_cast<float>(window->getScreenSize().y) / 2.0f;
                fontInfo.projection =
                    math::ortho(vec2(0, static_cast<float>(window->getScreenSize().x)),
                                vec2(0, static_cast<float>(window->getScreenSize().y)), vec2(0.0f, 1.0f));
            }

            RenderPassBuilder renderPass;
            renderPass.init(RenderPassType::GRAPHICS, builder)
                .addResource(ResourceAccessType::READ, viewProjBuffer, ShaderStage::VERTEX)
                .addResource(ResourceAccessType::READ, texture, ShaderStage::FRAGMENT)
                .setCommands(commands);
            for (auto& gBuffer : gBuffers)
            {
                renderPass.addRenderTarget(gBuffer);
            }
            renderGraph.addPass("GBuffer Pass", renderPass);

            renderPass.init(RenderPassType::COMPUTE, computeBuilder)
                .addResource(ResourceAccessType::READ, lightBuffer, ShaderStage::COMPUTE)
                .addResource(ResourceAccessType::WRITE, window->getRenderTarget()->getColorTexture(),
                             ShaderStage::COMPUTE)
                .setCommands(computeCommands);

            for (auto& gBuffer : gBuffers)
            {
                renderPass.addResource(ResourceAccessType::READ, gBuffer->getColorTexture(), ShaderStage::COMPUTE);
            }
            renderGraph.addPass("Deffered pass", renderPass);

            renderGraph.addPass("Fonts", RenderPassBuilder()
                                             .init(RenderPassType::GRAPHICS, fontPipeline)
                                             .addRenderTarget(window->getRenderTarget())
                                             .setClearRenderTargets(false)
                                             .setRenderTargetUse(RenderTargetType::COLOR)
                                             .setCommands(fontRenderCommands));
        }

        fontInfoBuffer->write(&fontInfo, sizeof(fontInfo));

        rect = window->getRect();
        viewProj = math::perspective(math::radians(90.0f),
                                     static_cast<float>(rect.screenWidth) / static_cast<float>(rect.screenHeight),
                                     vec2(0.1f, 100.0f)) *
                   math::lookTo(eye, -forward, up);
        viewProjBuffer->write(&viewProj, sizeof(viewProj));

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

            log(LogLevel::D_INFO, "Elapsed: {:.5f}, Delta: {:.5f}, FPS: {}", global::timer.elapsedSeconds(),
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