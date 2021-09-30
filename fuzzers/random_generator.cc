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

#include "fuzzers/random_generator.h"

#include <algorithm>
#include <cassert>
#include <vector>

#include "src/utils/hash.h"

namespace tint {
namespace fuzzers {

namespace {

/// Generate integer from uniform distribution
/// @tparam I - integer type
/// @param engine - random number engine to use
/// @param lower - Lower bound of integer generated
/// @param upper - Upper bound of integer generated
/// @returns i, where lower <= i < upper
template <typename I>
I RandomUInt(std::mt19937* engine, I lower, I upper) {
  assert(lower < upper && "|lower| must be stictly less than |upper|");

  return std::uniform_int_distribution<I>(lower, upper - 1)(*engine);
}

/// Calculate the hash for the contents of a c-style data buffer
/// This is intentionally not implemented as a generic override of HashCombine
/// in "src/utils/hash.h", because it conflicts with the vardiac override for
/// the case where a pointer and an integer are being hashed.
/// @param data - pointer to buffer to be hashed
/// @param size - number of elements in buffer
/// @returns hash of the data in the buffer
size_t HashBuffer(const uint8_t* data, const size_t size) {
  size_t hash = 102931;
  utils::HashCombine(&hash, size);
  for (size_t i = 0; i < size; i++) {
    utils::HashCombine(&hash, data[i]);
  }
  return hash;
}

}  // namespace

RandomGenerator::RandomGenerator(uint64_t seed) : engine_(seed) {}

RandomGenerator::RandomGenerator(const uint8_t* data, size_t size)
    : engine_(RandomGenerator::CalculateSeed(data, size)) {}

uint32_t RandomGenerator::GetUInt32(uint32_t lower, uint32_t upper) {
  return RandomUInt(&engine_, lower, upper);
}

uint32_t RandomGenerator::GetUInt32(uint32_t bound) {
  assert(bound > 0 && "|bound| must be greater than 0");
  return RandomUInt(&engine_, 0u, bound);
}

uint64_t RandomGenerator::GetUInt64(uint64_t lower, uint64_t upper) {
  return RandomUInt(&engine_, lower, upper);
}

uint64_t RandomGenerator::GetUInt64(uint64_t bound) {
  assert(bound > 0 && "|bound| must be greater than 0");
  return RandomUInt(&engine_, static_cast<uint64_t>(0), bound);
}

uint8_t RandomGenerator::GetByte() {
  return std::independent_bits_engine<std::mt19937, 8, uint8_t>(engine_)();
}

uint32_t RandomGenerator::Get4Bytes() {
  return std::independent_bits_engine<std::mt19937, 32, uint32_t>(engine_)();
}

void RandomGenerator::GetNBytes(uint8_t* dest, size_t n) {
  assert(dest && "|dest| must not be nullptr");
  std::generate(
      dest, dest + n,
      std::independent_bits_engine<std::mt19937, 8, uint8_t>(engine_));
}

bool RandomGenerator::GetBool() {
  return RandomUInt(&engine_, 0u, 2u);
}

bool RandomGenerator::GetWeightedBool(uint32_t percentage) {
  static const uint32_t kMaxPercentage = 100;
  assert(percentage <= kMaxPercentage &&
         "|percentage| needs to be within [0, 100]");
  return RandomUInt(&engine_, 0u, kMaxPercentage) < percentage;
}

uint64_t RandomGenerator::CalculateSeed(const uint8_t* data, size_t size) {
  assert(data != nullptr && "|data| must be !nullptr");

  // Number of bytes we want to skip at the start of data for the hash.
  // Fewer bytes may be skipped when `size` is small.
  // Has lower precedence than kHashDesiredMinBytes.
  static const int64_t kHashDesiredLeadingSkipBytes = 5;
  // Minimum number of bytes we want to use in the hash.
  // Used for short buffers.
  static const int64_t kHashDesiredMinBytes = 4;
  // Maximum number of bytes we want to use in the hash.
  static const int64_t kHashDesiredMaxBytes = 32;
  int64_t size_i64 = static_cast<int64_t>(size);
  int64_t hash_begin_i64 =
      std::min(kHashDesiredLeadingSkipBytes,
               std::max<int64_t>(size_i64 - kHashDesiredMinBytes, 0));
  int64_t hash_end_i64 =
      std::min(hash_begin_i64 + kHashDesiredMaxBytes, size_i64);
  size_t hash_begin = static_cast<size_t>(hash_begin_i64);
  size_t hash_size = static_cast<size_t>(hash_end_i64) - hash_begin;
  return HashBuffer(data + hash_begin, hash_size);
}

}  // namespace fuzzers
}  // namespace tint
