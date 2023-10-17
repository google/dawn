// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/fuzzers/mersenne_twister_engine.h"

#include <algorithm>
#include <cassert>

#include "src/tint/utils/math/hash.h"

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
