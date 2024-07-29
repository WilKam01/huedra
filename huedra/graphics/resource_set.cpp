#include "resource_set.hpp"
#include "core/log.hpp"

namespace huedra {

void ResourceSet::init(Pipeline* pipeline, u32 setIndex)
{
    p_pipeline = pipeline;
    m_setIndex = setIndex;

    if (setIndex >= pipeline->getBuilder().getResources().size())
    {
        log(LogLevel::ERR, "Could not create resource set, requested setIndex does not exist in pipeline");
    }
}

} // namespace huedra
