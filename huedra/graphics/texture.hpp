#pragma once

#include "core/types.hpp"
#include "graphics/context.hpp"
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

    void init(u32 width, u32 height, GraphicsDataFormat format, TextureType type, GraphicalContext* context);

    void setId(u64 id);

    u32 getWidth() const { return m_width; }
    u32 getHeight() const { return m_height; }
    GraphicsDataFormat getFormat() const { return m_format; }
    TextureType getType() const { return m_type; }
    u64 getId() { return m_id; }

private:
    u32 m_width{0};
    u32 m_height{0};
    GraphicsDataFormat m_format{GraphicsDataFormat::UNDEFINED};
    TextureType m_type{TextureType::COLOR};
    GraphicalContext* p_context{nullptr};
    u64 m_id{0};
};

} // namespace huedra
