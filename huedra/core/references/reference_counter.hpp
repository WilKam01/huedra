#pragma once

#include "core/types.hpp"

namespace huedra {

class RefBase;

template <typename T>
class Ref;

class ReferenceCounter
{
    friend class RefBase;

    template <typename T>
    friend class Ref;

public:
    static void addResource(void* resource);
    static void removeResource(void* resource);

    static void reportState();

private:
    static bool addRef(void* resource, RefBase* ref);
    static void removeRef(void* resource, RefBase* ref);
};

} // namespace huedra