#pragma once

#include "core/references/reference_counter.hpp"

namespace huedra {

class RefBase
{
    friend class ReferenceCounter;

public:
    RefBase() = default;
    virtual ~RefBase() = default;

    RefBase(const RefBase& rhs) = default;
    RefBase& operator=(const RefBase& rhs) = default;
    RefBase(RefBase&& rhs) = default;
    RefBase& operator=(RefBase&& rhs) = default;

    virtual bool valid() = 0;

private:
    virtual void setInvalid() = 0;
};

template <typename T>
class Ref : public RefBase
{
public:
    Ref() = default;
    explicit Ref(T* ptr);
    ~Ref() override;

    Ref(const Ref<T>& ref);
    Ref(Ref<T>& ref);
    Ref(const Ref<T>&& ref);
    Ref(Ref<T>&& ref);
    Ref<T>& operator=(const Ref<T>& rhs);
    Ref<T>& operator=(Ref<T>&& rhs);

    T* get();
    T* operator->();
    bool valid() override;

private:
    void setInvalid() override;

    void init(T* ptr, bool valid = true);
    void cleanup();

    T* m_ptr{nullptr};
    bool m_valid{false};
};

template <typename T>
inline Ref<T>::Ref(T* ptr)
{
    init(ptr);
}

template <typename T>
inline Ref<T>::~Ref()
{
    cleanup();
}

template <typename T>
inline Ref<T>::Ref(const Ref<T>& ref)
{
    init(ref.m_ptr, ref.m_valid);
}

template <typename T>
inline Ref<T>::Ref(Ref<T>& ref)
{
    init(ref.m_ptr, ref.m_valid);
}

template <typename T>
inline Ref<T>::Ref(const Ref<T>&& ref)
{
    if (this != &ref)
    {
        bool valid = ref.m_valid;
        cleanup();
        init(ref.m_ptr, valid);
    }
}

template <typename T>
inline Ref<T>::Ref(Ref<T>&& ref)
{
    if (this != &ref)
    {
        bool valid = ref.m_valid;
        cleanup();
        init(ref.m_ptr, valid);
    }
}

template <typename T>
inline Ref<T>& Ref<T>::operator=(const Ref<T>& rhs)
{
    if (this != &rhs)
    {
        init(rhs.m_ptr, rhs.m_valid);
    }
    return *this;
}

template <typename T>
inline Ref<T>& Ref<T>::operator=(Ref<T>&& rhs)
{
    if (this != &rhs)
    {
        bool valid = rhs.m_valid;
        cleanup();
        init(rhs.m_ptr, valid);
    }
    return *this;
}

template <typename T>
inline T* Ref<T>::get()
{
    return m_ptr;
}

template <typename T>
inline T* Ref<T>::operator->()
{
    return m_ptr;
}

template <typename T>
inline bool Ref<T>::valid()
{
    return m_valid && m_ptr;
}

template <typename T>
inline void Ref<T>::setInvalid()
{
    m_valid = false;
}

template <typename T>
inline void Ref<T>::init(T* ptr, bool valid)
{
    if (ptr && valid)
    {
        m_ptr = ptr;
        m_valid = ReferenceCounter::addRef(static_cast<void*>(m_ptr), this);
    }
}

template <typename T>
inline void Ref<T>::cleanup()
{
    if (m_ptr)
    {
        ReferenceCounter::removeRef(static_cast<void*>(m_ptr), this);
    }
}

} // namespace huedra