#pragma once

#include "core/references/ref.hpp"
#include "core/types.hpp"
#include "graphics/pipeline_data.hpp"
#include "graphics/texture.hpp"
#include "math/vec2.hpp"
#include <array>
#include <string_view>

namespace huedra {

enum class RenderTargetType
{
    COLOR,
    DEPTH,
    COLOR_AND_DEPTH
};
// TODO: Create this automatically? Macro?
constexpr std::array<std::string_view, 3> RenderTargetTypeNames{"Color", "Depth", "Color and Depth"};

class RenderTarget
{
public:
    RenderTarget();
    virtual ~RenderTarget();

    RenderTarget(const RenderTarget& rhs) = default;
    RenderTarget& operator=(const RenderTarget& rhs) = default;
    RenderTarget(RenderTarget&& rhs) = default;
    RenderTarget& operator=(RenderTarget&& rhs) = default;

    virtual Ref<Texture> getColorTexture() = 0;
    virtual Ref<Texture> getDepthTexture() = 0;

    bool isAvailable() const { return m_available; }
    RenderTargetType getType() const { return m_type; }
    GraphicsDataFormat getFormat() const { return m_format; }
    u32 getWidth() const { return m_width; }
    u32 getHeight() const { return m_height; }
    uvec2 getSize() const { return {m_width, m_height}; }
    bool usesColor() const { return m_type == RenderTargetType::COLOR || m_type == RenderTargetType::COLOR_AND_DEPTH; }
    bool usesDepth() const { return m_type == RenderTargetType::DEPTH || m_type == RenderTargetType::COLOR_AND_DEPTH; }

protected:
    void init(RenderTargetType type, GraphicsDataFormat format, u32 width, u32 height);
    void setAvailability(bool available) { m_available = available; }

private:
    RenderTargetType m_type{RenderTargetType::COLOR};
    GraphicsDataFormat m_format{GraphicsDataFormat::UNDEFINED};
    bool m_available{true};
    u32 m_width{0};
    u32 m_height{0};
};

} // namespace huedra