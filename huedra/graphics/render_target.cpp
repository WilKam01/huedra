#include "render_target.hpp"
#include "huedra/core/references/ref.hpp"

#include "huedra/core/log.hpp"

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