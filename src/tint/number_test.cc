// Copyright 2022 The Tint Authors.
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

#include <cmath>

#include "src/tint/program_builder.h"

#include "gtest/gtest.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint {
namespace {

constexpr int64_t kHighestI32 = static_cast<int64_t>(std::numeric_limits<int32_t>::max());
constexpr int64_t kHighestU32 = static_cast<int64_t>(std::numeric_limits<uint32_t>::max());
constexpr int64_t kLowestI32 = static_cast<int64_t>(std::numeric_limits<int32_t>::min());
constexpr int64_t kLowestU32 = static_cast<int64_t>(std::numeric_limits<uint32_t>::min());

// Highest float32 value. Calculated as:
// (2^127)×(1+(0x7fffff÷0x800000))
constexpr double kHighestF32 = 340282346638528859811704183484516925440.0;

// Next ULP up from kHighestF32 for a float64. Calculated as:
// (2^127)×(1+(0xfffffe0000001÷0x10000000000000))
constexpr double kHighestF32NextULP = 340282346638528897590636046441678635008.0;

// Smallest positive normal float32 value. Calculated as:
// 2^-126
constexpr double kSmallestF32 = 1.1754943508222875e-38;

// Next ULP down from kSmallestF32 for a float64. Calculated as:
// (2^-127)×(1+(0xfffffffffffff÷0x10000000000000))
constexpr double kSmallestF32PrevULP = 1.1754943508222874e-38;

// Highest float16 value. Calculated as:
// (2^15)×(1+(0x3ff÷0x400))
constexpr double kHighestF16 = 65504.0;

// Next ULP up from kHighestF16 for a float64. Calculated as:
// (2^15)×(1+(0xffc0000000001÷0x10000000000000))
constexpr double kHighestF16NextULP = 65504.00000000001;

// Smallest positive normal float16 value. Calculated as:
// 2^-14
constexpr double kSmallestF16 = 0.00006103515625;

// Next ULP down from kSmallestF16 for a float64. Calculated as:
// (2^-15)×(1+(0xfffffffffffff÷0x10000000000000))
constexpr double kSmallestF16PrevULP = 0.00006103515624999999;

constexpr double kLowestF32 = -kHighestF32;
constexpr double kLowestF32NextULP = -kHighestF32NextULP;
constexpr double kLowestF16 = -kHighestF16;
constexpr double kLowestF16NextULP = -kHighestF16NextULP;

TEST(NumberTest, CheckedConvertIdentity) {
    EXPECT_EQ(CheckedConvert<AInt>(0_a), 0_a);
    EXPECT_EQ(CheckedConvert<AFloat>(0_a), 0.0_a);
    EXPECT_EQ(CheckedConvert<i32>(0_i), 0_i);
    EXPECT_EQ(CheckedConvert<u32>(0_u), 0_u);
    EXPECT_EQ(CheckedConvert<f32>(0_f), 0_f);
    EXPECT_EQ(CheckedConvert<f16>(0_h), 0_h);

    EXPECT_EQ(CheckedConvert<AInt>(1_a), 1_a);
    EXPECT_EQ(CheckedConvert<AFloat>(1_a), 1.0_a);
    EXPECT_EQ(CheckedConvert<i32>(1_i), 1_i);
    EXPECT_EQ(CheckedConvert<u32>(1_u), 1_u);
    EXPECT_EQ(CheckedConvert<f32>(1_f), 1_f);
    EXPECT_EQ(CheckedConvert<f16>(1_h), 1_h);
}

TEST(NumberTest, CheckedConvertLargestValue) {
    EXPECT_EQ(CheckedConvert<i32>(AInt(kHighestI32)), i32(kHighestI32));
    EXPECT_EQ(CheckedConvert<u32>(AInt(kHighestU32)), u32(kHighestU32));
    EXPECT_EQ(CheckedConvert<f32>(AFloat(kHighestF32)), f32(kHighestF32));
    EXPECT_EQ(CheckedConvert<f16>(AFloat(kHighestF16)), f16(kHighestF16));
}

TEST(NumberTest, CheckedConvertLowestValue) {
    EXPECT_EQ(CheckedConvert<i32>(AInt(kLowestI32)), i32(kLowestI32));
    EXPECT_EQ(CheckedConvert<u32>(AInt(kLowestU32)), u32(kLowestU32));
    EXPECT_EQ(CheckedConvert<f32>(AFloat(kLowestF32)), f32(kLowestF32));
    EXPECT_EQ(CheckedConvert<f16>(AFloat(kLowestF16)), f16(kLowestF16));
}

TEST(NumberTest, CheckedConvertSmallestValue) {
    EXPECT_EQ(CheckedConvert<i32>(AInt(0)), i32(0));
    EXPECT_EQ(CheckedConvert<u32>(AInt(0)), u32(0));
    EXPECT_EQ(CheckedConvert<f32>(AFloat(kSmallestF32)), f32(kSmallestF32));
    EXPECT_EQ(CheckedConvert<f16>(AFloat(kSmallestF16)), f16(kSmallestF16));
}

TEST(NumberTest, CheckedConvertExceedsPositiveLimit) {
    EXPECT_EQ(CheckedConvert<i32>(AInt(kHighestI32 + 1)), ConversionFailure::kExceedsPositiveLimit);
    EXPECT_EQ(CheckedConvert<u32>(AInt(kHighestU32 + 1)), ConversionFailure::kExceedsPositiveLimit);
    EXPECT_EQ(CheckedConvert<f32>(AFloat(kHighestF32NextULP)),
              ConversionFailure::kExceedsPositiveLimit);
    EXPECT_EQ(CheckedConvert<f16>(AFloat(kHighestF16NextULP)),
              ConversionFailure::kExceedsPositiveLimit);
}

TEST(NumberTest, CheckedConvertExceedsNegativeLimit) {
    EXPECT_EQ(CheckedConvert<i32>(AInt(kLowestI32 - 1)), ConversionFailure::kExceedsNegativeLimit);
    EXPECT_EQ(CheckedConvert<u32>(AInt(kLowestU32 - 1)), ConversionFailure::kExceedsNegativeLimit);
    EXPECT_EQ(CheckedConvert<f32>(AFloat(kLowestF32NextULP)),
              ConversionFailure::kExceedsNegativeLimit);
    EXPECT_EQ(CheckedConvert<f16>(AFloat(kLowestF16NextULP)),
              ConversionFailure::kExceedsNegativeLimit);
}

TEST(NumberTest, CheckedConvertTooSmall) {
    EXPECT_EQ(CheckedConvert<f32>(AFloat(kSmallestF32PrevULP)), ConversionFailure::kTooSmall);
    EXPECT_EQ(CheckedConvert<f16>(AFloat(kSmallestF16PrevULP)), ConversionFailure::kTooSmall);
}

TEST(NumberTest, QuantizeF16) {
    constexpr float nan = std::numeric_limits<float>::quiet_NaN();
    constexpr float inf = std::numeric_limits<float>::infinity();

    EXPECT_EQ(f16(0.0), 0.0f);
    EXPECT_EQ(f16(1.0), 1.0f);
    EXPECT_EQ(f16(0.00006106496), 0.000061035156f);
    EXPECT_EQ(f16(1.0004883), 1.0f);
    EXPECT_EQ(f16(-8196), -8192.f);
    EXPECT_EQ(f16(65504.003), inf);
    EXPECT_EQ(f16(-65504.003), -inf);
    EXPECT_EQ(f16(inf), inf);
    EXPECT_EQ(f16(-inf), -inf);
    EXPECT_TRUE(std::isnan(f16(nan)));
}

}  // namespace
}  // namespace tint
