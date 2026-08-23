#pragma once

#include "core/types.hpp"

namespace huedra::constants {

// Time
const u64 SECONDS_TO_NANO = 1'000'000'000;
const u64 MILLISECONDS_TO_NANO = 1'000'000;
const u64 MICROSECONDS_TO_NANO = 1'000;

// Hash
const u64 FNV_PRIME = 0x00000100000001b3;
const u64 FNV_OFFSET = 0xcbf29ce484222325;

}; // namespace huedra