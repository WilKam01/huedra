#pragma once

#include "core/types.hpp"

namespace huedra {

// Combined version and id (upper byte = version, lower 3 bytes = id)
using Entity = u32;

struct ComponentListBase
{};

template <typename T>
struct ComponentList : public ComponentListBase
{
    std::vector<T> components;
};

} // namespace huedra
