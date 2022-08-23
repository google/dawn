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

#include "src/tint/fuzzers/random_generator.h"

#include <algorithm>
#include <cassert>
#include <utility>

#include "src/tint/fuzzers/mersenne_twister_engine.h"
#include "src/tint/fuzzers/random_generator_engine.h"
#include "src/tint/utils/hash.h"

namespace tint::fuzzers {

namespace {

/// Calculate the hash for the contents of a c-style data buffer
/// This is intentionally not implemented as a generic override of HashCombine
/// in "src/tint/utils/hash.h", because it conflicts with the vardiac override
/// for the case where a pointer and an integer are being hashed.
/// @param data - pointer to buffer to be hashed
/// @param size - number of elements in buffer
/// @returns hash of the data in the buffer
size_t HashBuffer(const uint8_t* data, const size_t size) {
    size_t hash = utils::Hash(size);
    for (size_t i = 0; i < size; i++) {
        hash = utils::HashCombine(hash, data[i]);
    }
    return hash;
}

}  // namespace

RandomGenerator::RandomGenerator(std::unique_ptr<RandomGeneratorEngine> engine)
    : engine_(std::move(engine)) {}

RandomGenerator::RandomGenerator(uint64_t seed)
    : RandomGenerator(std::make_unique<MersenneTwisterEngine>(seed)) {}

uint32_t RandomGenerator::GetUInt32(uint32_t lower, uint32_t upper) {
    assert(lower < upper && "|lower| must be strictly less than |upper|");
    return engine_->RandomUInt32(lower, upper);
}

uint32_t RandomGenerator::GetUInt32(uint32_t bound) {
    assert(bound > 0 && "|bound| must be greater than 0");
    return engine_->RandomUInt32(0u, bound);
}

uint64_t RandomGenerator::GetUInt64(uint64_t lower, uint64_t upper) {
    assert(lower < upper && "|lower| must be strictly less than |upper|");
    return engine_->RandomUInt64(lower, upper);
}

uint64_t RandomGenerator::GetUInt64(uint64_t bound) {
    assert(bound > 0 && "|bound| must be greater than 0");
    return engine_->RandomUInt64(static_cast<uint64_t>(0), bound);
}

uint8_t RandomGenerator::GetByte() {
    uint8_t result;
    engine_->RandomNBytes(&result, 1);
    return result;
}

uint32_t RandomGenerator::Get4Bytes() {
    uint32_t result;
    engine_->RandomNBytes(reinterpret_cast<uint8_t*>(&result), 4);
    return result;
}

void RandomGenerator::GetNBytes(uint8_t* dest, size_t n) {
    assert(dest && "|dest| must not be nullptr");
    engine_->RandomNBytes(dest, n);
}

bool RandomGenerator::GetBool() {
    return engine_->RandomUInt32(0u, 2u);
}

bool RandomGenerator::GetWeightedBool(uint32_t percentage) {
    static const uint32_t kMaxPercentage = 100;
    assert(percentage <= kMaxPercentage && "|percentage| needs to be within [0, 100]");
    return engine_->RandomUInt32(0u, kMaxPercentage) < percentage;
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
    auto size_i64 = static_cast<int64_t>(size);
    auto hash_begin_i64 = std::min(kHashDesiredLeadingSkipBytes,
                                   std::max<int64_t>(size_i64 - kHashDesiredMinBytes, 0));
    auto hash_end_i64 = std::min(hash_begin_i64 + kHashDesiredMaxBytes, size_i64);
    auto hash_begin = static_cast<size_t>(hash_begin_i64);
    auto hash_size = static_cast<size_t>(hash_end_i64) - hash_begin;
    return HashBuffer(data + hash_begin, hash_size);
}
}  // namespace tint::fuzzers
