#include "buffer.hpp"
#include "core/global.hpp"
#include "core/references/reference_counter.hpp"

namespace huedra {

Buffer::Buffer() { ReferenceCounter::addResource(static_cast<void*>(this)); }

Buffer::~Buffer() { ReferenceCounter::removeResource(static_cast<void*>(this)); }

void Buffer::init(BufferType type, BufferUsageFlags usage, u64 size, GraphicalContext* context)
{
    m_type = type;
    m_usage = usage;
    m_size = size;
    p_context = context;
    m_id = 0;
}

void Buffer::read(u64 size, void* data) { p_context->readBuffer(m_id, size, data); }

void Buffer::write(u64 size, void* data) { p_context->writeToBuffer(m_id, size, data); }

void Buffer::setId(u64 id) { m_id = id; }

} // namespace huedra