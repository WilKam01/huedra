#include "core/file/utils.hpp"
#include "core/global.hpp"
#include "core/input/keys.hpp"
#include "core/input/mouse.hpp"
#include "core/log.hpp"
#include "core/serialization/json.hpp"
#include "core/string/utils.hpp"
#include "core/types.hpp"
#include "graphics/pipeline_builder.hpp"
#include "graphics/pipeline_data.hpp"
#include "graphics/render_context.hpp"
#include "graphics/render_graph_builder.hpp"
#include "graphics/render_pass_builder.hpp"
#include "graphics/shader_module.hpp"
#include "math/conversions.hpp"
#include "math/matrix_projection.hpp"
#include "math/matrix_transform.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "resources/mesh/loader.hpp"
#include "resources/texture/loader.hpp"
#include "scene/components/transform.hpp"
#include "window/window.hpp"

using namespace huedra;

int main()
{
    global::timer.init();
    global::windowManager.init();
    global::graphicsManager.init();
    global::resourceManager.init();

    Ref<Window> window = global::windowManager.addWindow("Main", WindowInput(1280, 720));
    Ref<Window> window1 = global::windowManager.addWindow("Second", WindowInput(480, 480, 1400, -250), window);

    ShaderModule& module = global::resourceManager.loadShaderModule("assets/shaders/triangle.slang");

    std::array<vec2, 3> positions{vec2(0.0f, 0.5f), vec2(-0.5f, -0.5f), vec2(0.5f, -0.5f)};
    Ref<Buffer> positionsBuffer = global::graphicsManager.createBuffer(
        BufferType::STATIC, HU_BUFFER_USAGE_VERTEX_BUFFER, positions.size() * sizeof(vec2), positions.data());

    std::array<vec3, 3> colors{vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f)};
    Ref<Buffer> colorsBuffer = global::graphicsManager.createBuffer(BufferType::STATIC, HU_BUFFER_USAGE_VERTEX_BUFFER,
                                                                    colors.size() * sizeof(vec3), colors.data());

    PipelineBuilder pipelineBuilder;
    pipelineBuilder.init(PipelineType::GRAPHICS)
        .addShader(module, "vertMain")
        .addShader(module, "fragMain")
        .addVertexInputStream({.size = sizeof(vec2),
                               .inputRate = VertexInputRate::VERTEX,
                               .attributes{{.format = GraphicsDataFormat::RG_32_FLOAT, .offset = 0}}})
        .addVertexInputStream({.size = sizeof(vec3),
                               .inputRate = VertexInputRate::VERTEX,
                               .attributes{{.format = GraphicsDataFormat::RGB_32_FLOAT, .offset = 0}}});

    Timer renderTimer;
    renderTimer.init();
    while (global::windowManager.update())
    {
        global::timer.update();
        global::graphicsManager.update();

        if (global::input.isKeyActive(KeyToggles::CAPS_LOCK))
        {
            log(LogLevel::D_INFO, "CAPS ACTIVE");
        }

        if (global::input.isMouseButtonPressed(MouseButton::EXTRA2))
        {
            log(LogLevel::D_INFO, "Extra 2 click is pressed");
        }
        if (global::input.isMouseButtonDoubleClicked(MouseButton::EXTRA2))
        {
            log(LogLevel::D_INFO, "Extra 2 is double pressed");
        }

        if (global::input.isKeyPressed(Keys::ESCAPE))
        {
            global::input.toggleMouseHidden();
        }

        static CursorType cursor{CursorType::DEFAULT};
        static MouseMode mode{MouseMode::DISABLED};
        if (global::input.isKeyPressed(Keys::ENTER))
        {
            mode = static_cast<MouseMode>(((static_cast<u32>(mode)) % 3) + 1);
            global::input.setMouseMode(mode);
            cursor = static_cast<CursorType>((static_cast<u32>(cursor) + 1) % 15);
            global::input.setCursor(cursor);
        }

        RenderGraphBuilder graph;
        if (window.valid() && window->getRenderTarget()->isAvailable())
        {
            graph.addPass("Pass", RenderPassBuilder()
                                      .init(RenderPassType::GRAPHICS, pipelineBuilder)
                                      .addRenderTarget(window->getRenderTarget(), vec3(0.1f))
                                      .setCommands([&](RenderContext& context) {
                                          context.bindVertexBuffers({positionsBuffer, colorsBuffer});
                                          context.draw(3, 1, 0, 0);
                                      }));
        }

        renderTimer.update();
        if (renderTimer.passedInterval((1.0 / 240.0) * Timer::SECONDS_TO_NANO))
        {
            global::graphicsManager.render(graph);
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

    global::resourceManager.cleanup();
    global::graphicsManager.cleanup();
    global::windowManager.cleanup();

#ifdef DEBUG
    ReferenceCounter::reportState();
#endif
}