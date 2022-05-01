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
