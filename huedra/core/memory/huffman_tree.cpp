#include "huffman_tree.hpp"
#include "core/memory/utils.hpp"

namespace huedra {

void HuffmanTree::init(const std::vector<u32>& codeLengths, u32 range)
{
    m_elements.push_back(-1); // Add root of tree

    u32 maxBits = 0;
    for (auto& len : codeLengths)
    {
        maxBits = std::max(len, maxBits);
    }

    std::vector<u32> codeLengthCounts(maxBits + 1, 0);
    for (u64 i = 1; i <= maxBits; ++i)
    {
        codeLengthCounts[i] = std::count_if(codeLengths.begin(), codeLengths.end(), [i](u32 x) { return x == i; });
    }

    std::vector<u32> nextCode(maxBits + 1);
    for (u64 i = 2; i <= maxBits; ++i)
    {
        nextCode[i] = (nextCode[i - 1] + codeLengthCounts[i - 1]) << 1;
    }

    for (u64 i = 0; i < codeLengths.size(); ++i)
    {
        u32 bitLen = codeLengths[i];
        if (bitLen == 0)
        {
            continue;
        }

        // Insert code
        u64 index = 0;
        for (i64 j = bitLen - 1; j >= 0; --j)
        {
            index = 2 * index + 1; // Assume left child
            if (static_cast<bool>(nextCode[bitLen] & (1u << j)))
            {
                ++index; // Sets to right child if bit is 1
            }

            // Outside tree "bounds"
            if (index >= m_elements.size())
            {
                // Increase the height by one level of nodes
                u64 newSize = 2 * m_elements.size() + 1;
                m_elements.resize(newSize, -1);
            }
        }
        m_elements[index] = i;
        ++nextCode[bitLen];
    }
}

u32 HuffmanTree::decodeSymbol(const u8* bytes, u64& bits) const
{
    u32 index = 0;
    while (m_elements[index] == -1)
    {
        index = index * 2 + 1;                   // Assume left child
        index += readBits(&bytes[0], bits++, 1); // Sets to right child if 1
    }
    return m_elements[index];
}

} // namespace huedra