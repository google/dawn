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
