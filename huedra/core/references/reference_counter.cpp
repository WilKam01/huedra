#include "reference_counter.hpp"
#include "core/log.hpp"
#include "core/references/ref.hpp"

namespace huedra {

std::map<void*, std::vector<RefBase*>> ReferenceCounter::m_references = {};

void ReferenceCounter::addResource(void* resource)
{
    if (!m_references.count(resource))
    {
        std::vector<RefBase*> vec; // Empty vec
        m_references.insert(std::make_pair(resource, vec));
    }
}

void ReferenceCounter::removeResource(void* resource)
{
    if (m_references.count(resource))
    {
        for (auto& ref : m_references[resource])
        {
            ref->setInvalid();
        }
        m_references.erase(resource);
    }
}

void ReferenceCounter::reportState()
{
    if (!m_references.empty())
    {
        log(LogLevel::WARNING, "ReferenceCounter: found %d resources not removed", m_references.size());
        for (auto& ref : m_references)
        {
            log(LogLevel::WARNING, "Address: %p | References alive: %d", ref.first, ref.second.size());
        }
    }
}

bool ReferenceCounter::addRef(void* resource, RefBase* ref)
{
    if (!m_references.count(resource))
    {
        ref->setInvalid();
#ifdef DEBUG
        log(LogLevel::WARNING, "ReferenceCounter: addref() failed to find resource");
#endif
        return false;
    }

    m_references[resource].push_back(ref);
    return true;
}

void ReferenceCounter::removeRef(void* resource, RefBase* ref)
{
    if (!ref->valid())
    {
        return;
    }

    if (m_references.count(resource))
    {
        std::vector<RefBase*>& refs = m_references[resource];
        for (auto it = refs.begin(); it != refs.end(); ++it)
        {
            if ((*it) == ref)
            {
                it = refs.erase(it);
                break;
            }
        }
    }
}

} // namespace huedra