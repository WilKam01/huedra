#include "reference_counter.hpp"
#include "core/log.hpp"
#include "core/references/ref.hpp"

namespace huedra {

namespace {
std::map<void*, std::vector<RefBase*>>& getRefs()
{
    static std::map<void*, std::vector<RefBase*>> references;
    return references;
}
} // namespace

void ReferenceCounter::addResource(void* resource)
{
    if (!getRefs().contains(resource))
    {
        std::vector<RefBase*> vec; // Empty vec
        getRefs().insert(std::make_pair(resource, vec));
    }
}

void ReferenceCounter::removeResource(void* resource)
{
    if (getRefs().contains(resource))
    {
        for (auto& ref : getRefs()[resource])
        {
            ref->setInvalid();
        }
        getRefs().erase(resource);
    }
}

void ReferenceCounter::reportState()
{
    if (!getRefs().empty())
    {
        log(LogLevel::WARNING, "ReferenceCounter: found {} resources not removed", getRefs().size());
        for (auto& [ptr, refs] : getRefs())
        {
            log(LogLevel::WARNING, "Address: 0x{:x} | getRefs() alive: {}", ptr, refs.size());
        }
    }
}

bool ReferenceCounter::addRef(void* resource, RefBase* ref)
{
    if (!getRefs().contains(resource))
    {
        ref->setInvalid();
#ifdef DEBUG
        log(LogLevel::WARNING, "ReferenceCounter: addref() failed to find resource");
#endif
        return false;
    }

    getRefs()[resource].push_back(ref);
    return true;
}

void ReferenceCounter::removeRef(void* resource, RefBase* ref)
{
    if (!ref->valid())
    {
        return;
    }

    if (getRefs().contains(resource))
    {
        std::vector<RefBase*>& refs = getRefs()[resource];
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