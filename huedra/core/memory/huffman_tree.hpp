#pragma once

#include "core/types.hpp"

namespace huedra {

class HuffmanTree
{
public:
    HuffmanTree() = default;
    ~HuffmanTree() = default;

    HuffmanTree(const HuffmanTree& rhs) = default;
    HuffmanTree& operator=(const HuffmanTree& rhs) = default;
    HuffmanTree(HuffmanTree&& rhs) = default;
    HuffmanTree& operator=(HuffmanTree&& rhs) = default;

    void init(const std::vector<u32>& codeLengths, u32 range);

    u32 decodeSymbol(const u8* bytes, u64& bits) const;

private:
    std::vector<i32> m_elements;
};

} // namespace huedra