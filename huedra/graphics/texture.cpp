#include "texture.hpp"
#include "core/references/reference_counter.hpp"

namespace huedra {

Texture::Texture() { ReferenceCounter::addResource(static_cast<void*>(this)); }

Texture::~Texture() { ReferenceCounter::removeResource(static_cast<void*>(this)); }

void Texture::init(u32 width, u32 height, GraphicsDataFormat format, TextureType type)
{
    m_width = width;
    m_height = height;
    m_format = format;
    m_type = type;
}

} // namespace huedra