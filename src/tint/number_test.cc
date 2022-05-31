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
#include <tuple>
#include <vector>

#include "src/tint/program_builder.h"
#include "src/tint/utils/compiler_macros.h"

#include "gtest/gtest.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint {
namespace {

constexpr int64_t kHighestI32 = static_cast<int64_t>(std::numeric_limits<int32_t>::max());
constexpr int64_t kHighestU32 = static_cast<int64_t>(std::numeric_limits<uint32_t>::max());
constexpr int64_t kLowestI32 = static_cast<int64_t>(std::numeric_limits<int32_t>::min());
constexpr int64_t kLowestU32 = static_cast<int64_t>(std::numeric_limits<uint32_t>::min());

// Highest float32 value.
constexpr double kHighestF32 = 0x1.fffffep+127;

// Next ULP up from kHighestF32 for a float64.
constexpr double kHighestF32NextULP = 0x1.fffffe0000001p+127;

// Smallest positive normal float32 value.
constexpr double kSmallestF32 = 0x1p-126;

// Highest subnormal value for a float32.
constexpr double kHighestF32Subnormal = 0x0.fffffep-126;

// Highest float16 value.
constexpr double kHighestF16 = 0x1.ffcp+15;

// Next ULP up from kHighestF16 for a float64.
constexpr double kHighestF16NextULP = 0x1.ffc0000000001p+15;

// Smallest positive normal float16 value.
constexpr double kSmallestF16 = 0x1p-14;

// Highest subnormal value for a float32.
constexpr double kHighestF16Subnormal = 0x0.ffcp-14;

constexpr double kLowestF32 = -kHighestF32;
constexpr double kLowestF32NextULP = -kHighestF32NextULP;
constexpr double kLowestF16 = -kHighestF16;
constexpr double kLowestF16NextULP = -kHighestF16NextULP;

// MSVC (only in release builds) can grumble about some of the inlined numerical overflow /
// underflow that's done in this file. We like to think we know what we're doing, so silence the
// warning.
TINT_BEGIN_DISABLE_WARNING(CONSTANT_OVERFLOW);

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

TEST(NumberTest, CheckedConvertSubnormals) {
    EXPECT_EQ(CheckedConvert<f32>(AFloat(kHighestF32Subnormal)), f32(kHighestF32Subnormal));
    EXPECT_EQ(CheckedConvert<f16>(AFloat(kHighestF16Subnormal)), f16(kHighestF16Subnormal));
    EXPECT_EQ(CheckedConvert<f32>(AFloat(-kHighestF32Subnormal)), f32(-kHighestF32Subnormal));
    EXPECT_EQ(CheckedConvert<f16>(AFloat(-kHighestF16Subnormal)), f16(-kHighestF16Subnormal));
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

using BinaryCheckedCase = std::tuple<std::optional<AInt>, AInt, AInt>;

#undef OVERFLOW  // corecrt_math.h :(
#define OVERFLOW \
    {}

using CheckedAddTest = testing::TestWithParam<BinaryCheckedCase>;
TEST_P(CheckedAddTest, Test) {
    auto expect = std::get<0>(GetParam());
    auto a = std::get<1>(GetParam());
    auto b = std::get<2>(GetParam());
    EXPECT_EQ(CheckedAdd(a, b), expect) << std::hex << "0x" << a << " * 0x" << b;
    EXPECT_EQ(CheckedAdd(b, a), expect) << std::hex << "0x" << a << " * 0x" << b;
}
INSTANTIATE_TEST_SUITE_P(
    CheckedAddTest,
    CheckedAddTest,
    testing::ValuesIn(std::vector<BinaryCheckedCase>{
        {AInt(0), AInt(0), AInt(0)},
        {AInt(1), AInt(1), AInt(0)},
        {AInt(2), AInt(1), AInt(1)},
        {AInt(0), AInt(-1), AInt(1)},
        {AInt(3), AInt(2), AInt(1)},
        {AInt(-1), AInt(-2), AInt(1)},
        {AInt(0x300), AInt(0x100), AInt(0x200)},
        {AInt(0x100), AInt(-0x100), AInt(0x200)},
        {AInt(AInt::kHighest), AInt(1), AInt(AInt::kHighest - 1)},
        {AInt(AInt::kLowest), AInt(-1), AInt(AInt::kLowest + 1)},
        {AInt(AInt::kHighest), AInt(0x7fffffff00000000ll), AInt(0x00000000ffffffffll)},
        {AInt(AInt::kHighest), AInt(AInt::kHighest), AInt(0)},
        {AInt(AInt::kLowest), AInt(AInt::kLowest), AInt(0)},
        {OVERFLOW, AInt(1), AInt(AInt::kHighest)},
        {OVERFLOW, AInt(-1), AInt(AInt::kLowest)},
        {OVERFLOW, AInt(2), AInt(AInt::kHighest)},
        {OVERFLOW, AInt(-2), AInt(AInt::kLowest)},
        {OVERFLOW, AInt(10000), AInt(AInt::kHighest)},
        {OVERFLOW, AInt(-10000), AInt(AInt::kLowest)},
        {OVERFLOW, AInt(AInt::kHighest), AInt(AInt::kHighest)},
        {OVERFLOW, AInt(AInt::kLowest), AInt(AInt::kLowest)},
        ////////////////////////////////////////////////////////////////////////
    }));

using CheckedMulTest = testing::TestWithParam<BinaryCheckedCase>;
TEST_P(CheckedMulTest, Test) {
    auto expect = std::get<0>(GetParam());
    auto a = std::get<1>(GetParam());
    auto b = std::get<2>(GetParam());
    EXPECT_EQ(CheckedMul(a, b), expect) << std::hex << "0x" << a << " * 0x" << b;
    EXPECT_EQ(CheckedMul(b, a), expect) << std::hex << "0x" << a << " * 0x" << b;
}
INSTANTIATE_TEST_SUITE_P(
    CheckedMulTest,
    CheckedMulTest,
    testing::ValuesIn(std::vector<BinaryCheckedCase>{
        {AInt(0), AInt(0), AInt(0)},
        {AInt(0), AInt(1), AInt(0)},
        {AInt(1), AInt(1), AInt(1)},
        {AInt(-1), AInt(-1), AInt(1)},
        {AInt(2), AInt(2), AInt(1)},
        {AInt(-2), AInt(-2), AInt(1)},
        {AInt(0x20000), AInt(0x100), AInt(0x200)},
        {AInt(-0x20000), AInt(-0x100), AInt(0x200)},
        {AInt(0x4000000000000000ll), AInt(0x80000000ll), AInt(0x80000000ll)},
        {AInt(0x4000000000000000ll), AInt(-0x80000000ll), AInt(-0x80000000ll)},
        {AInt(0x1000000000000000ll), AInt(0x40000000ll), AInt(0x40000000ll)},
        {AInt(-0x1000000000000000ll), AInt(-0x40000000ll), AInt(0x40000000ll)},
        {AInt(0x100000000000000ll), AInt(0x1000000), AInt(0x100000000ll)},
        {AInt(0x2000000000000000ll), AInt(0x1000000000000000ll), AInt(2)},
        {AInt(-0x2000000000000000ll), AInt(0x1000000000000000ll), AInt(-2)},
        {AInt(-0x2000000000000000ll), AInt(-0x1000000000000000ll), AInt(2)},
        {AInt(-0x2000000000000000ll), AInt(0x1000000000000000ll), AInt(-2)},
        {AInt(0x4000000000000000ll), AInt(0x1000000000000000ll), AInt(4)},
        {AInt(-0x4000000000000000ll), AInt(0x1000000000000000ll), AInt(-4)},
        {AInt(-0x4000000000000000ll), AInt(-0x1000000000000000ll), AInt(4)},
        {AInt(-0x4000000000000000ll), AInt(0x1000000000000000ll), AInt(-4)},
        {AInt(-0x8000000000000000ll), AInt(0x1000000000000000ll), AInt(-8)},
        {AInt(-0x8000000000000000ll), AInt(-0x1000000000000000ll), AInt(8)},
        {AInt(0), AInt(AInt::kHighest), AInt(0)},
        {AInt(0), AInt(AInt::kLowest), AInt(0)},
        {OVERFLOW, AInt(0x1000000000000000ll), AInt(8)},
        {OVERFLOW, AInt(-0x1000000000000000ll), AInt(-8)},
        {OVERFLOW, AInt(0x800000000000000ll), AInt(0x10)},
        {OVERFLOW, AInt(0x80000000ll), AInt(0x100000000ll)},
        {OVERFLOW, AInt(AInt::kHighest), AInt(AInt::kHighest)},
        {OVERFLOW, AInt(AInt::kHighest), AInt(AInt::kLowest)},
        ////////////////////////////////////////////////////////////////////////
    }));

using TernaryCheckedCase = std::tuple<std::optional<AInt>, AInt, AInt, AInt>;

using CheckedMaddTest = testing::TestWithParam<TernaryCheckedCase>;
TEST_P(CheckedMaddTest, Test) {
    auto expect = std::get<0>(GetParam());
    auto a = std::get<1>(GetParam());
    auto b = std::get<2>(GetParam());
    auto c = std::get<3>(GetParam());
    EXPECT_EQ(CheckedMadd(a, b, c), expect)
        << std::hex << "0x" << a << " * 0x" << b << " + 0x" << c;
    EXPECT_EQ(CheckedMadd(b, a, c), expect)
        << std::hex << "0x" << a << " * 0x" << b << " + 0x" << c;
}
INSTANTIATE_TEST_SUITE_P(
    CheckedMaddTest,
    CheckedMaddTest,
    testing::ValuesIn(std::vector<TernaryCheckedCase>{
        {AInt(0), AInt(0), AInt(0), AInt(0)},
        {AInt(0), AInt(1), AInt(0), AInt(0)},
        {AInt(1), AInt(1), AInt(1), AInt(0)},
        {AInt(2), AInt(1), AInt(1), AInt(1)},
        {AInt(0), AInt(1), AInt(-1), AInt(1)},
        {AInt(-1), AInt(1), AInt(-2), AInt(1)},
        {AInt(-1), AInt(-1), AInt(1), AInt(0)},
        {AInt(2), AInt(2), AInt(1), AInt(0)},
        {AInt(-2), AInt(-2), AInt(1), AInt(0)},
        {AInt(0), AInt(AInt::kHighest), AInt(0), AInt(0)},
        {AInt(0), AInt(AInt::kLowest), AInt(0), AInt(0)},
        {AInt(3), AInt(1), AInt(2), AInt(1)},
        {AInt(0x300), AInt(1), AInt(0x100), AInt(0x200)},
        {AInt(0x100), AInt(1), AInt(-0x100), AInt(0x200)},
        {AInt(0x20000), AInt(0x100), AInt(0x200), AInt(0)},
        {AInt(-0x20000), AInt(-0x100), AInt(0x200), AInt(0)},
        {AInt(0x4000000000000000ll), AInt(0x80000000ll), AInt(0x80000000ll), AInt(0)},
        {AInt(0x4000000000000000ll), AInt(-0x80000000ll), AInt(-0x80000000ll), AInt(0)},
        {AInt(0x1000000000000000ll), AInt(0x40000000ll), AInt(0x40000000ll), AInt(0)},
        {AInt(-0x1000000000000000ll), AInt(-0x40000000ll), AInt(0x40000000ll), AInt(0)},
        {AInt(0x100000000000000ll), AInt(0x1000000), AInt(0x100000000ll), AInt(0)},
        {AInt(0x2000000000000000ll), AInt(0x1000000000000000ll), AInt(2), AInt(0)},
        {AInt(-0x2000000000000000ll), AInt(0x1000000000000000ll), AInt(-2), AInt(0)},
        {AInt(-0x2000000000000000ll), AInt(-0x1000000000000000ll), AInt(2), AInt(0)},
        {AInt(-0x2000000000000000ll), AInt(0x1000000000000000ll), AInt(-2), AInt(0)},
        {AInt(0x4000000000000000ll), AInt(0x1000000000000000ll), AInt(4), AInt(0)},
        {AInt(-0x4000000000000000ll), AInt(0x1000000000000000ll), AInt(-4), AInt(0)},
        {AInt(-0x4000000000000000ll), AInt(-0x1000000000000000ll), AInt(4), AInt(0)},
        {AInt(-0x4000000000000000ll), AInt(0x1000000000000000ll), AInt(-4), AInt(0)},
        {AInt(-0x8000000000000000ll), AInt(0x1000000000000000ll), AInt(-8), AInt(0)},
        {AInt(-0x8000000000000000ll), AInt(-0x1000000000000000ll), AInt(8), AInt(0)},
        {AInt(AInt::kHighest), AInt(1), AInt(1), AInt(AInt::kHighest - 1)},
        {AInt(AInt::kLowest), AInt(1), AInt(-1), AInt(AInt::kLowest + 1)},
        {AInt(AInt::kHighest), AInt(1), AInt(0x7fffffff00000000ll), AInt(0x00000000ffffffffll)},
        {AInt(AInt::kHighest), AInt(1), AInt(AInt::kHighest), AInt(0)},
        {AInt(AInt::kLowest), AInt(1), AInt(AInt::kLowest), AInt(0)},
        {OVERFLOW, AInt(0x1000000000000000ll), AInt(8), AInt(0)},
        {OVERFLOW, AInt(-0x1000000000000000ll), AInt(-8), AInt(0)},
        {OVERFLOW, AInt(0x800000000000000ll), AInt(0x10), AInt(0)},
        {OVERFLOW, AInt(0x80000000ll), AInt(0x100000000ll), AInt(0)},
        {OVERFLOW, AInt(AInt::kHighest), AInt(AInt::kHighest), AInt(0)},
        {OVERFLOW, AInt(AInt::kHighest), AInt(AInt::kLowest), AInt(0)},
        {OVERFLOW, AInt(1), AInt(1), AInt(AInt::kHighest)},
        {OVERFLOW, AInt(1), AInt(-1), AInt(AInt::kLowest)},
        {OVERFLOW, AInt(1), AInt(2), AInt(AInt::kHighest)},
        {OVERFLOW, AInt(1), AInt(-2), AInt(AInt::kLowest)},
        {OVERFLOW, AInt(1), AInt(10000), AInt(AInt::kHighest)},
        {OVERFLOW, AInt(1), AInt(-10000), AInt(AInt::kLowest)},
        {OVERFLOW, AInt(1), AInt(AInt::kHighest), AInt(AInt::kHighest)},
        {OVERFLOW, AInt(1), AInt(AInt::kLowest), AInt(AInt::kLowest)},
        {OVERFLOW, AInt(1), AInt(AInt::kHighest), AInt(1)},
        {OVERFLOW, AInt(1), AInt(AInt::kLowest), AInt(-1)},
    }));

TINT_END_DISABLE_WARNING(CONSTANT_OVERFLOW);

}  // namespace
}  // namespace tint
