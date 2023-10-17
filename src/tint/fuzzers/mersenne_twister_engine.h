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

#ifndef SRC_TINT_FUZZERS_MERSENNE_TWISTER_ENGINE_H_
#define SRC_TINT_FUZZERS_MERSENNE_TWISTER_ENGINE_H_

#include <random>

#include "src/tint/fuzzers/random_generator_engine.h"

namespace tint::fuzzers {

/// Standard MT based random number generation
class MersenneTwisterEngine : public RandomGeneratorEngine {
  public:
    /// @brief Initializes using provided seed
    /// @param seed - seed value to use
    explicit MersenneTwisterEngine(uint64_t seed);
    ~MersenneTwisterEngine() override = default;

    /// Generate random uint32_t value from uniform distribution.
    /// @param lower - lower bound of integer generated
    /// @param upper - upper bound of integer generated
    /// @returns i, where lower <= i < upper
    uint32_t RandomUInt32(uint32_t lower, uint32_t upper) override;

    /// Get random uint64_t value from uniform distribution.
    /// @param lower - lower bound of integer generated
    /// @param upper - upper bound of integer generated
    /// @returns i, where lower <= i < upper
    uint64_t RandomUInt64(uint64_t lower, uint64_t upper) override;

    /// Get N bytes of pseudo-random data
    /// @param dest - memory location to store data
    /// @param n - number of bytes of data to generate
    void RandomNBytes(uint8_t* dest, size_t n) override;

  private:
    // Disallow copy & assign
    MersenneTwisterEngine(const MersenneTwisterEngine&) = delete;
    MersenneTwisterEngine& operator=(const MersenneTwisterEngine&) = delete;

    std::mt19937_64 engine_;
};  // class MersenneTwisterEngine

}  // namespace tint::fuzzers

#endif  // SRC_TINT_FUZZERS_MERSENNE_TWISTER_ENGINE_H_
