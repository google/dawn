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

#include "src/tint/fuzzers/random_generator.h"

#include <memory>

#include "gtest/gtest.h"

#include "src/tint/fuzzers/mersenne_twister_engine.h"

namespace tint::fuzzers {
namespace {

/// Implementation of RandomGeneratorEngine that just returns a stream of
/// monotonically increasing numbers.
class MonotonicEngine : public RandomGeneratorEngine {
  public:
    uint32_t RandomUInt32(uint32_t, uint32_t) override { return next_++; }

    uint64_t RandomUInt64(uint64_t, uint64_t) override { return next_++; }

    void RandomNBytes(uint8_t*, size_t) override {
        assert(false && "MonotonicDelegate does not implement RandomNBytes");
    }

  private:
    uint32_t next_ = 0;
};

class RandomGeneratorTest : public testing::Test {
  public:
    void SetUp() override { rng_ = std::make_unique<RandomGenerator>(0); }

    void TearDown() override {}

  protected:
    std::unique_ptr<RandomGenerator> rng_;
};

#ifndef NDEBUG
TEST_F(RandomGeneratorTest, GetUInt32ReversedBoundsCrashes) {
    EXPECT_DEATH_IF_SUPPORTED(rng_->GetUInt32(10, 5), ".*");
}

TEST_F(RandomGeneratorTest, GetUInt32EmptyBoundsCrashes) {
    EXPECT_DEATH_IF_SUPPORTED(rng_->GetUInt32(5, 5), ".*");
}

TEST_F(RandomGeneratorTest, GetUInt32ZeroBoundCrashes) {
    EXPECT_DEATH_IF_SUPPORTED(rng_->GetUInt32(0u), ".*");
}
#endif  // NDEBUG

TEST_F(RandomGeneratorTest, GetUInt32SingularReturnsOneValue) {
    {
        uint32_t result = rng_->GetUInt32(5u, 6u);
        ASSERT_EQ(5u, result);
    }
    {
        uint32_t result = rng_->GetUInt32(1u);
        ASSERT_EQ(0u, result);
    }
}

TEST_F(RandomGeneratorTest, GetUInt32StaysInBounds) {
    {
        uint32_t result = rng_->GetUInt32(5u, 10u);
        ASSERT_LE(5u, result);
        ASSERT_GT(10u, result);
    }
    {
        uint32_t result = rng_->GetUInt32(10u);
        ASSERT_LE(0u, result);
        ASSERT_GT(10u, result);
    }
}

#ifndef NDEBUG
TEST_F(RandomGeneratorTest, GetUInt64ReversedBoundsCrashes) {
    EXPECT_DEATH_IF_SUPPORTED(rng_->GetUInt64(10, 5), ".*");
}

TEST_F(RandomGeneratorTest, GetUInt64EmptyBoundsCrashes) {
    EXPECT_DEATH_IF_SUPPORTED(rng_->GetUInt64(5, 5), ".*");
}

TEST_F(RandomGeneratorTest, GetUInt64ZeroBoundCrashes) {
    EXPECT_DEATH_IF_SUPPORTED(rng_->GetUInt64(0u), ".*");
}
#endif  // NDEBUG

TEST_F(RandomGeneratorTest, GetUInt64SingularReturnsOneValue) {
    {
        uint64_t result = rng_->GetUInt64(5u, 6u);
        ASSERT_EQ(5u, result);
    }
    {
        uint64_t result = rng_->GetUInt64(1u);
        ASSERT_EQ(0u, result);
    }
}

TEST_F(RandomGeneratorTest, GetUInt64StaysInBounds) {
    {
        uint64_t result = rng_->GetUInt64(5u, 10u);
        ASSERT_LE(5u, result);
        ASSERT_GT(10u, result);
    }
    {
        uint64_t result = rng_->GetUInt64(10u);
        ASSERT_LE(0u, result);
        ASSERT_GT(10u, result);
    }
}

TEST_F(RandomGeneratorTest, GetByte) {
    rng_->GetByte();
}

#ifndef NDEBUG
TEST_F(RandomGeneratorTest, GetNBytesNullDataBufferCrashes) {
    EXPECT_DEATH_IF_SUPPORTED(rng_->GetNBytes(nullptr, 5), ".*");
}
#endif  // NDEBUG

TEST_F(RandomGeneratorTest, GetNBytes) {
    std::vector<uint8_t> data;
    for (uint32_t i = 25; i < 1000u; i = i + 25) {
        data.resize(i);
        rng_->GetNBytes(data.data(), data.size());
    }
}

TEST_F(RandomGeneratorTest, GetBool) {
    rng_->GetBool();
}

TEST_F(RandomGeneratorTest, GetWeightedBoolZeroAlwaysFalse) {
    ASSERT_FALSE(rng_->GetWeightedBool(0));
}

TEST_F(RandomGeneratorTest, GetWeightedBoolHundredAlwaysTrue) {
    ASSERT_TRUE(rng_->GetWeightedBool(100));
}

#ifndef NDEBUG
TEST_F(RandomGeneratorTest, GetWeightedBoolAboveHundredCrashes) {
    EXPECT_DEATH_IF_SUPPORTED(rng_->GetWeightedBool(101), ".*");
    EXPECT_DEATH_IF_SUPPORTED(rng_->GetWeightedBool(500), ".*");
}
#endif  // NDEBUG

TEST_F(RandomGeneratorTest, GetWeightedBool) {
    for (uint32_t i = 0; i <= 100; i++) {
        rng_ = std::make_unique<RandomGenerator>(std::make_unique<MonotonicEngine>());
        for (uint32_t j = 0; j <= 100; j++) {
            if (j < i) {
                ASSERT_TRUE(rng_->GetWeightedBool(i));
            } else {
                ASSERT_FALSE(rng_->GetWeightedBool(i));
            }
        }
    }
}

#ifndef NDEBUG
TEST_F(RandomGeneratorTest, GetRandomElementEmptyVectorCrashes) {
    std::vector<uint8_t> v;
    EXPECT_DEATH_IF_SUPPORTED(rng_->GetRandomElement(v), ".*");
}
#endif  // NDEBUG

TEST_F(RandomGeneratorTest, GetRandomElement) {
    std::vector<uint32_t> v;
    for (uint32_t i = 25; i < 100u; i = i + 25) {
        rng_ = std::make_unique<RandomGenerator>(std::make_unique<MonotonicEngine>());
        v.resize(i);
        std::iota(v.begin(), v.end(), 0);
        for (uint32_t j = 0; j < i; j++) {
            EXPECT_EQ(j, rng_->GetRandomElement(v));
        }
    }
}

}  // namespace
}  // namespace tint::fuzzers
