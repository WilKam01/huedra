#include <bit>
#include <cstdint>
#include <iostream>

#include "types.hpp"

int main()
{
    huedra::u64 num = 64;
    std::cout << "Hello World! " << std::byteswap(num) << "\n";
}