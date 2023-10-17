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

#ifndef SRC_TINT_FUZZERS_RANDOM_GENERATOR_H_
#define SRC_TINT_FUZZERS_RANDOM_GENERATOR_H_

#include <memory>
#include <random>
#include <vector>

#include "src/tint/fuzzers/random_generator_engine.h"

namespace tint::fuzzers {

/// Pseudo random generator utility class for fuzzing
class RandomGenerator {
  public:
    /// @brief Initializes using provided engine
    /// @param engine - engine implementation to use
    explicit RandomGenerator(std::unique_ptr<RandomGeneratorEngine> engine);

    /// @brief Creates a MersenneTwisterEngine and initializes using that
    /// @param seed - seed value to use for engine
    explicit RandomGenerator(uint64_t seed);

    /// Destructor
    ~RandomGenerator() = default;

    /// Move Constructor
    RandomGenerator(RandomGenerator&&) = default;

    /// Get uint32_t value from uniform distribution.
    /// @param lower - lower bound of integer generated
    /// @param upper - upper bound of integer generated
    /// @returns i, where lower <= i < upper
    uint32_t GetUInt32(uint32_t lower, uint32_t upper);

    /// Get uint32_t value from uniform distribution.
    /// @param bound - Upper bound of integer generated
    /// @returns i, where 0 <= i < bound
    uint32_t GetUInt32(uint32_t bound);

    /// Get uint32_t value from uniform distribution.
    /// @param lower - lower bound of integer generated
    /// @param upper - upper bound of integer generated
    /// @returns i, where lower <= i < upper
    uint64_t GetUInt64(uint64_t lower, uint64_t upper);

    /// Get uint64_t value from uniform distribution.
    /// @param bound - Upper bound of integer generated
    /// @returns i, where 0 <= i < bound
    uint64_t GetUInt64(uint64_t bound);

    /// Get 1 byte of pseudo-random data
    /// Should be more efficient then calling GetNBytes(1);
    /// @returns 1-byte of random data
    uint8_t GetByte();

    /// Get 4 bytes of pseudo-random data
    /// Should be more efficient then calling GetNBytes(4);
    /// @returns 4-bytes of random data
    uint32_t Get4Bytes();

    /// Get N bytes of pseudo-random data
    /// @param dest - memory location to store data
    /// @param n - number of bytes of data to get
    void GetNBytes(uint8_t* dest, size_t n);

    /// Get random bool with even odds
    /// @returns true 50% of the time and false %50 of time.
    bool GetBool();

    /// Get random bool with weighted odds
    /// @param percentage - likelihood of true being returned
    /// @returns true |percentage|% of the time, and false (100 - |percentage|)%
    /// of the time.
    bool GetWeightedBool(uint32_t percentage);

    /// Returns a randomly-chosen element from vector v.
    /// @param v - the vector from which the random element will be selected.
    /// @return a random element of vector v.
    template <typename T>
    inline T GetRandomElement(const std::vector<T>& v) {
        return v[GetUInt64(0, v.size())];
    }

    /// Calculate a seed value based on a blob of data.
    /// Currently hashes bytes near the front of the buffer, after skipping N
    /// bytes.
    /// @param data - pointer to data to base calculation off of, must be !nullptr
    /// @param size - number of elements in |data|, must be > 0
    /// @returns calculated seed value
    static uint64_t CalculateSeed(const uint8_t* data, size_t size);

  private:
    // Disallow copy & assign
    RandomGenerator(const RandomGenerator&) = delete;
    RandomGenerator& operator=(const RandomGenerator&) = delete;

    std::unique_ptr<RandomGeneratorEngine> engine_;
};  // class RandomGenerator

}  // namespace tint::fuzzers

#endif  // SRC_TINT_FUZZERS_RANDOM_GENERATOR_H_
