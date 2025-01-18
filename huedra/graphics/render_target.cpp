#include "render_target.hpp"
#include "core/references/ref.hpp"

#include "core/log.hpp"

namespace huedra {

RenderTarget::RenderTarget() { ReferenceCounter::addResource(static_cast<void*>(this)); }

RenderTarget::~RenderTarget() { ReferenceCounter::removeResource(static_cast<void*>(this)); }

void RenderTarget::init(RenderTargetType type, GraphicsDataFormat format, u32 width, u32 height)
{
    m_type = type;
    m_format = format;
    m_width = width;
    m_height = height;
    m_available = true;
}

} // namespace huedra