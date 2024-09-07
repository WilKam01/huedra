#include "resource_set.hpp"
#include "core/references/reference_counter.hpp"

namespace huedra {

ResourceSet::ResourceSet() { ReferenceCounter::addResource(static_cast<void*>(this)); }

ResourceSet::~ResourceSet() { ReferenceCounter::removeResource(static_cast<void*>(this)); }

void ResourceSet::init(u32 setIndex, const std::vector<ResourceBinding>& bindings)
{
    m_setIndex = setIndex;
    m_bindings = bindings;
}

bool ResourceSet::isCompatible(const PipelineBuilder& builder) const
{
    if (m_setIndex >= builder.getResources().size())
    {
        return false;
    }

    std::vector<ResourceBinding> bindings = builder.getResources()[m_setIndex];
    if (m_bindings.size() != bindings.size())
    {
        return false;
    }

    for (u64 i = 0; i < m_bindings.size(); ++i)
    {
        if (m_bindings[i].resource != bindings[i].resource || m_bindings[i].shaderStage != bindings[i].shaderStage)
        {
            return false;
        }
    }

    return true;
}

} // namespace huedra
