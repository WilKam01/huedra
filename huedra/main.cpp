#include "core/file/utils.hpp"
#include "core/global.hpp"
#include "core/input/keys.hpp"
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
#include "window/window.hpp"
#include <string>

using namespace huedra;

int main()
{
    global::timer.init();
    global::windowManager.init();
    global::graphicsManager.init();
    global::resourceManager.init();

    Ref<Window> window = global::windowManager.addWindow("Main", WindowInput(1280, 720, 100, 100));

    while (global::windowManager.update())
    {
        global::timer.update();
        global::graphicsManager.update();

        if (global::input.isKeyActive(KeyToggles::CAPS_LOCK))
        {
            log(LogLevel::D_INFO, "CAPS ACTIVE");
        }

        if (global::input.isKeyDown(Keys::A))
        {
            log(LogLevel::D_INFO, "A is down");
        }
        if (global::input.isKeyDown(Keys::S))
        {
            log(LogLevel::D_INFO, "S is down");
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

            if (window.valid())
            {
                window->setTitle("Main (FPS:" + std::to_string(sum / 500) + ")");
            }

            // log(LogLevel::D_INFO, "Elapsed: {:.5f}, Delta: {:.5f}, FPS: {}", global::timer.secondsElapsed(),
            // global::timer.dt(), sum / 500);
            i = 0;
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