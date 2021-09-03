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

}  // namespace

RandomGenerator::RandomGenerator(uint32_t seed) : engine_(seed) {}

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

std::vector<uint8_t> RandomGenerator::GetNBytes(size_t n) {
  std::vector<uint8_t> result(n);
  std::generate(
      std::begin(result), std::end(result),
      std::independent_bits_engine<std::mt19937, 8, uint8_t>(engine_));
  return result;
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

}  // namespace fuzzers
}  // namespace tint
