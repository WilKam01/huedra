#pragma once

#include "core/types.hpp"
#include "graphics/pipeline_data.hpp"

namespace huedra {

enum class TextureType
{
    COLOR,
    DEPTH,
};

class Texture
{
public:
    Texture();
    ~Texture();

    void init(u32 width, u32 height, GraphicsDataFormat format, TextureType type);
    virtual void cleanup() = 0;

    u32 getWidth() const { return m_width; }
    u32 getHeight() const { return m_height; }
    GraphicsDataFormat getFormat() const { return m_format; }
    TextureType getType() const { return m_type; }

private:
    u32 m_width{0};
    u32 m_height{0};
    GraphicsDataFormat m_format{GraphicsDataFormat::UNDEFINED};
    TextureType m_type{TextureType::COLOR};
};

} // namespace huedra