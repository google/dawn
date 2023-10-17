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

#ifndef SRC_TINT_FUZZERS_RANDOM_GENERATOR_ENGINE_H_
#define SRC_TINT_FUZZERS_RANDOM_GENERATOR_ENGINE_H_

#include <memory>
#include <random>
#include <vector>

namespace tint::fuzzers {

/// Wrapper interface around STL random number engine
class RandomGeneratorEngine {
  public:
    /// Constructor
    RandomGeneratorEngine();

    /// Destructor
    virtual ~RandomGeneratorEngine();

    /// Move Constructor
    RandomGeneratorEngine(RandomGeneratorEngine&&);

    /// Generates a random uint32_t value from uniform distribution.
    /// @param lower - lower bound of integer generated
    /// @param upper - upper bound of integer generated
    /// @returns i, where lower <= i < upper
    virtual uint32_t RandomUInt32(uint32_t lower, uint32_t upper) = 0;

    /// Generates a random uint64_t value from uniform distribution.
    /// @param lower - lower bound of integer generated
    /// @param upper - upper bound of integer generated
    /// @returns i, where lower <= i < upper
    virtual uint64_t RandomUInt64(uint64_t lower, uint64_t upper) = 0;

    /// Generates N bytes of pseudo-random data
    /// @param dest - memory location to store data
    /// @param n - number of bytes of data to generate
    virtual void RandomNBytes(uint8_t* dest, size_t n) = 0;

  private:
    // Disallow copy & assign
    RandomGeneratorEngine(const RandomGeneratorEngine&) = delete;
    RandomGeneratorEngine& operator=(const RandomGeneratorEngine&) = delete;
};  // class RandomGeneratorEngine

}  // namespace tint::fuzzers

#endif  // SRC_TINT_FUZZERS_RANDOM_GENERATOR_ENGINE_H_
