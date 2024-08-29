#include "resource_set.hpp"
#include "core/references/reference_counter.hpp"

namespace huedra {

ResourceSet::ResourceSet() { ReferenceCounter::addResource(static_cast<void*>(this)); }

ResourceSet::~ResourceSet() { ReferenceCounter::removeResource(static_cast<void*>(this)); }

void ResourceSet::init(u32 setIndex) { m_setIndex = setIndex; }

} // namespace huedra
