#pragma once

#include "core/references/ref.hpp"
#include "core/types.hpp"
#include "graphics/pipeline_data.hpp"
#include "graphics/texture.hpp"

namespace huedra {

enum class RenderTargetType
{
    COLOR,
    DEPTH,
    COLOR_AND_DEPTH
};

class RenderTarget
{
public:
    RenderTarget();
    virtual ~RenderTarget();

    virtual Ref<Texture> getColorTexture() = 0;
    virtual Ref<Texture> getDepthTexture() = 0;

    bool isAvailable() const { return m_available; }
    RenderTargetType getType() { return m_type; }
    GraphicsDataFormat getFormat() { return m_format; }
    u32 getWidth() { return m_width; }
    u32 getHeight() { return m_height; }
    bool usesColor() { return m_type == RenderTargetType::COLOR || m_type == RenderTargetType::COLOR_AND_DEPTH; }
    bool usesDepth() { return m_type == RenderTargetType::DEPTH || m_type == RenderTargetType::COLOR_AND_DEPTH; }

protected:
    void init(RenderTargetType type, GraphicsDataFormat format, u32 width, u32 height);

    bool m_available{true};

private:
    RenderTargetType m_type;
    GraphicsDataFormat m_format{GraphicsDataFormat::UNDEFINED};
    u32 m_width{0};
    u32 m_height{0};
};

} // namespace huedra