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
    inline Vec(Vec<T, L2> vec)
    {
        u64 largest = L > L2 ? L2 : L;
        for (u64 i = 0; i < largest; ++i)
        {
            m_elements[i] = vec[i];
        }
    }

    template <typename... Ts, typename = std::enable_if_t<(sizeof...(Ts) == L || sizeof...(Ts) == 1) &&
                                                          std::conjunction_v<std::is_convertible<T, Ts>...>>>
    inline Vec(Ts... values)
    {
        const u64 num = sizeof...(Ts);
        T elements[] = {static_cast<T>(values)...};
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

    inline T operator[](u64 index) const { return m_elements[index]; }

private:
    std::array<T, L> m_elements{};
};

} // namespace huedra