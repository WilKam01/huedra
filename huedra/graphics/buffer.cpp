#include "buffer.hpp"
#include "core/references/reference_counter.hpp"

namespace huedra {

Buffer::Buffer() { ReferenceCounter::addResource(static_cast<void*>(this)); }

Buffer::~Buffer() { ReferenceCounter::removeResource(static_cast<void*>(this)); }

void Buffer::init(BufferType type, BufferUsageFlags usage, u64 size)
{
    m_type = type;
    m_usage = usage;
    m_size = size;
}

} // namespace huedra