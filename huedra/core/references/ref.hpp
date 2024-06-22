#pragma once

#include "core/references/reference_counter.hpp"

namespace huedra {

class RefBase
{
    friend class ReferenceCounter;

public:
    RefBase() = default;
    virtual ~RefBase() = default;

    virtual bool valid() = 0;

private:
    virtual void setInvalid() = 0;
};

template <typename T>
class Ref : public RefBase
{
public:
    Ref(T* ptr);
    ~Ref();

    T* get();
    bool valid() override;

private:
    void setInvalid() override;

    T* m_ptr{nullptr};
    bool m_valid{false};
};

template <typename T>
inline Ref<T>::Ref(T* ptr)
{
    if (ptr)
    {
        m_ptr = ptr;
        m_valid = true;
        ReferenceCounter::addRef(static_cast<void*>(m_ptr), this);
    }
}

template <typename T>
inline Ref<T>::~Ref()
{
    if (m_ptr)
    {
        ReferenceCounter::removeRef(static_cast<void*>(m_ptr), this);
    }
}

template <typename T>
inline T* Ref<T>::get()
{
    return m_ptr;
}

template <typename T>
inline bool Ref<T>::valid()
{
    return m_valid;
}

template <typename T>
inline void Ref<T>::setInvalid()
{
    m_valid = false;
}

} // namespace huedra