#include "scene_manager.hpp"

namespace huedra {

void SceneManager::init() {}

void SceneManager::cleanup()
{
    m_freeEntities.clear();
    m_componentSets.clear();
    m_versions.clear();
}

Entity SceneManager::addEntity()
{
    if (!m_freeEntities.empty())
    {
        Entity entity = m_freeEntities.front();
        m_freeEntities.pop_front();
        ++m_versions[getId(entity)];
        return entity + (1u << 24);
    }
    m_versions.push_back(0);
    return m_versions.size() - 1;
}

void SceneManager::removeEntity(Entity entity)
{
    if (isEntityValid(entity))
    {
        m_freeEntities.push_back(entity);
    }
}

bool SceneManager::isEntityValid(Entity entity) const
{
    u32 id = getId(entity);
    if (m_versions.size() <= id)
    {
        return false;
    }
    return m_versions[id] == getVersion(entity);
}

} // namespace huedra