// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/tint/fuzzers/mersenne_twister_engine.h"

#include <algorithm>
#include <cassert>

#include "src/tint/utils/hash.h"

namespace tint::fuzzers {

namespace {

/// Generate integer from uniform distribution
/// @tparam I - integer type
/// @param engine - random number engine to use
/// @param lower - Lower bound of integer generated
/// @param upper - Upper bound of integer generated
/// @returns i, where lower <= i < upper
template <typename I>
I RandomInteger(std::mt19937_64* engine, I lower, I upper) {
    assert(lower < upper && "|lower| must be strictly less than |upper|");
    return std::uniform_int_distribution<I>(lower, upper - 1)(*engine);
}

}  // namespace

MersenneTwisterEngine::MersenneTwisterEngine(uint64_t seed) : engine_(seed) {}

uint32_t MersenneTwisterEngine::RandomUInt32(uint32_t lower, uint32_t upper) {
    return RandomInteger(&engine_, lower, upper);
}

uint64_t MersenneTwisterEngine::RandomUInt64(uint64_t lower, uint64_t upper) {
    return RandomInteger(&engine_, lower, upper);
}

void MersenneTwisterEngine::RandomNBytes(uint8_t* dest, size_t n) {
    assert(dest && "|dest| must not be nullptr");
    std::generate(dest, dest + n,
                  std::independent_bits_engine<std::mt19937_64, 8, uint8_t>(engine_));
}

}  // namespace tint::fuzzers
