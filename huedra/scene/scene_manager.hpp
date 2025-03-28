#pragma once

#include "core/log.hpp"
#include "scene/types.hpp"

#include <functional>
#include <typeindex>

namespace huedra {

class SceneManager
{
public:
    SceneManager() = default;
    ~SceneManager() = default;

    SceneManager(const SceneManager& rhs) = default;
    SceneManager& operator=(const SceneManager& rhs) = default;
    SceneManager(SceneManager&& rhs) = default;
    SceneManager& operator=(SceneManager&& rhs) = default;

    void init();
    void cleanup();

    Entity addEntity();
    void removeEntity(Entity entity);
    bool isEntityValid(Entity entity) const;

    u32 getEntityCount() const { return static_cast<u32>(m_versions.size()); }

    template <typename... Args>
    bool hasComponents(Entity entity) const;

    template <typename T>
    void setComponent(Entity entity, const T& component);

    template <typename... Args>
    void setComponents(Entity entity, const Args&... args);

    template <typename T>
    void removeComponent(Entity entity);

    template <typename... Args>
    void removeComponents(Entity entity);

    template <typename T>
    T& getComponent(Entity entity) const;

    template <typename... Args>
    std::tuple<Args&...> getComponents(Entity entity) const;

    template <typename T>
    void query(std::function<void(T&)> func) const;

    // func should be structured as std::function<void(Args&...)>
    template <typename... Args>
    void query(auto&& func) const;

private:
    constexpr static u32 getId(Entity entity) { return entity & 0x00ffffff; }
    constexpr static u8 getVersion(Entity entity) { return static_cast<u8>(entity >> 24); }

    std::deque<Entity> m_freeEntities;
    struct SparseSet
    {
        std::vector<Entity> dense;
        ComponentListBase* componentList;
        std::vector<u32> sparse; // Indices to dense vector using Entity id
    };
    std::unordered_map<std::type_index, SparseSet> m_componentSets;
    std::vector<u8> m_versions; // Tracks versions of Entities
};

template <typename... Args>
inline bool SceneManager::hasComponents(Entity entity) const
{
    if (!(m_componentSets.contains(typeid(Args)) && ...) || !isEntityValid(entity))
    {
        return false;
    }
    bool hasAll = true;
    auto func = [&](const SparseSet& set) {
        u32 id = getId(entity);
        if (set.sparse.size() <= id || set.dense[set.sparse[id]] != entity)
        {
            hasAll = false;
        }
    };
    ((func(m_componentSets.at(typeid(Args)))), ...);
    return hasAll;
}

template <typename T>
inline void SceneManager::setComponent(Entity entity, const T& component)
{
    if (!m_componentSets.contains(typeid(T)) || !isEntityValid(entity))
    {
        m_componentSets.insert(std::pair<std::type_index, SparseSet>(typeid(T), {}));
        m_componentSets[typeid(T)].componentList = new ComponentList<T>();
    }
    SparseSet& set = m_componentSets[typeid(T)];
    set.dense.push_back(entity);

    auto* componentList = static_cast<ComponentList<T>*>(set.componentList);
    componentList->components.push_back(component);

    u32 id = getId(entity);
    if (set.sparse.size() <= id)
    {
        set.sparse.resize(id + 1, 0);
    }
    set.sparse[id] = set.dense.size() - 1;
}

template <typename... Args>
inline void SceneManager::setComponents(Entity entity, const Args&... args)
{
    ((setComponent(entity, args)), ...);
}

template <typename T>
inline void SceneManager::removeComponent(Entity entity)
{
    if (!hasComponents<T>(entity))
    {
        log(LogLevel::WARNING, "removeComponent(): entity {} does not have a {} component", entity, typeid(T).name());
        return;
    }

    SparseSet& set = m_componentSets[typeid(T)];
    u32 denseIndex = set.sparse[getId(entity)];

    auto* componentList = static_cast<ComponentList<T>*>(set.componentList);
    set.dense[denseIndex] = set.dense.back();
    componentList->components[denseIndex] = componentList->components.back();
    set.sparse[getId(set.dense[denseIndex])] = denseIndex;

    set.dense.pop_back();
    componentList->components.pop_back();
}

template <typename... Args>
inline void SceneManager::removeComponents(Entity entity)
{
    ((removeComponent<Args>(entity)), ...);
}

template <typename T>
inline T& SceneManager::getComponent(Entity entity) const
{
    if (!hasComponents<T>(entity))
    {
        log(LogLevel::ERR, "getComponent(): entity {} does not have a {} component", entity, typeid(T).name());
    }

    const SparseSet& set = m_componentSets.at(typeid(T));
    u32 id = getId(entity);
    return static_cast<ComponentList<T>*>(set.componentList)->components[set.sparse[id]];
}

template <typename... Args>
inline std::tuple<Args&...> SceneManager::getComponents(Entity entity) const
{
    return std::tuple<Args&...>(getComponent<Args>(entity)...);
}

template <typename T>
inline void SceneManager::query(std::function<void(T&)> func) const
{
    if (m_componentSets.contains(typeid(T)))
    {
        ComponentList<T>* list = static_cast<ComponentList<T>*>(m_componentSets.at(typeid(T)).componentList);
        for (auto& comp : list)
        {
            func(comp);
        }
    }
}

// func should be structured as std::function<void(Args&...)>
template <typename... Args>
inline void SceneManager::query(auto&& func) const
{
    if ((!m_componentSets.contains(typeid(Args)) || ...))
    {
        return;
    }

    u32 min = ~0u;
    const std::array<const SparseSet*, sizeof...(Args)> sets = {&m_componentSets.at(typeid(Args))...};
    const std::vector<Entity>* entities = nullptr;
    for (auto& set : sets)
    {
        if (set->dense.size() < min)
        {
            min = set->dense.size();
            entities = &set->dense;
        }
    }

    for (const auto& entity : *entities)
    {
        if (hasComponents<Args...>(entity))
        {
            func(getComponent<Args>(entity)...);
        }
    }
}

} // namespace huedra
