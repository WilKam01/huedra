#pragma once

#include "core/types.hpp"

#include <type_traits>

namespace huedra {

template <typename T, u64 L>
class Vec
{
public:
    Vec() = default;

    template <u64 L2>
    explicit Vec(Vec<T, L2> vec)
    {
        u64 largest = L > L2 ? L2 : L;
        for (u64 i = 0; i < largest; ++i)
        {
            m_elements[i] = vec[i];
        }
    }

    template <typename... Ts>
        requires((sizeof...(Ts) == L || sizeof...(Ts) == 1) && (std::is_convertible_v<T, Ts> && ...))
    explicit Vec(Ts... values)
    {
        const u64 num = sizeof...(Ts);
        std::array<T, sizeof...(Ts)> elements = {static_cast<T>(values)...};
        if (num == 1)
        {
            m_elements.fill(elements[0]);
        }
        else
        {
            for (u64 i = 0; i < L; ++i)
            {
                m_elements[i] = elements[i];
            }
        }
    }

    T operator[](u64 index) const { return m_elements[index]; }
    T& operator[](u64 index) { return m_elements[index]; }

private:
    std::array<T, L> m_elements{static_cast<T>(0)};
};

} // namespace huedra