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

#ifndef FUZZERS_RANDOM_GENERATOR_H_
#define FUZZERS_RANDOM_GENERATOR_H_

#include <random>
#include <vector>

namespace tint {
namespace fuzzers {

/// Pseudo random generator utility class for fuzzing
class RandomGenerator {
 public:
  /// @brief Initializes the internal engine
  /// @param seed - seed value passed to engine
  explicit RandomGenerator(uint64_t seed);

  /// @brief Wrapper that invokes CalculateSeed for caller
  /// @param data - data fuzzer to calculate seed from
  /// @param size - size of data buffer
  explicit RandomGenerator(const uint8_t* data, size_t size);

  ~RandomGenerator() {}

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
  /// @param n - number of bytes of data to generate
  void GetNBytes(uint8_t* dest, size_t n);

  /// Get random bool with even odds
  /// @returns true 50% of the time and false %50 of time.
  bool GetBool();

  /// Get random bool with weighted odds
  /// @param percentage - likelihood of true being returned
  /// @returns true |percentage|% of the time, and false (100 - |percentage|)%
  /// of the time.
  bool GetWeightedBool(uint32_t percentage);

  /// Calculate a seed value based on a blob of data.
  /// Currently hashes bytes near the front of the buffer, after skipping N
  /// bytes.
  /// @param data - pointer to data to base calculation off of, must be !nullptr
  /// @param size - number of elements in |data|, must be > 0
  static uint64_t CalculateSeed(const uint8_t* data, size_t size);

  /// Returns a randomly-chosen element from vector v.
  /// @param v - the vector from which the random element will be selected.
  /// @return a random element of vector v.
  template <typename T>
  inline T GetRandomElement(const std::vector<T>& v) {
    return v[GetUInt64(0, v.size() - 1)];
  }

 private:
  std::mt19937 engine_;

};  // class RandomGenerator

}  // namespace fuzzers
}  // namespace tint

#endif  // FUZZERS_RANDOM_GENERATOR_H_
