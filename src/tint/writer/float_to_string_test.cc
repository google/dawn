// Copyright 2020 The Tint Authors.
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

#include "src/tint/writer/float_to_string.h"

#include <cmath>
#include <cstring>
#include <limits>

#include "gtest/gtest.h"

namespace tint::writer {
namespace {

// Makes an IEEE 754 binary32 floating point number with
// - 0 sign if sign is 0, 1 otherwise
// - 'exponent_bits' is placed in the exponent space.
//   So, the exponent bias must already be included.
float MakeFloat(int sign, int biased_exponent, int mantissa) {
    const uint32_t sign_bit = sign ? 0x80000000u : 0u;
    // The binary32 exponent is 8 bits, just below the sign.
    const uint32_t exponent_bits = (biased_exponent & 0xffu) << 23;
    // The mantissa is the bottom 23 bits.
    const uint32_t mantissa_bits = (mantissa & 0x7fffffu);

    uint32_t bits = sign_bit | exponent_bits | mantissa_bits;
    float result = 0.0f;
    static_assert(sizeof(result) == sizeof(bits),
                  "expected float and uint32_t to be the same size");
    std::memcpy(&result, &bits, sizeof(bits));
    return result;
}

TEST(FloatToStringTest, Zero) {
    EXPECT_EQ(FloatToString(0.0f), "0.0");
}

TEST(FloatToStringTest, One) {
    EXPECT_EQ(FloatToString(1.0f), "1.0");
}

TEST(FloatToStringTest, MinusOne) {
    EXPECT_EQ(FloatToString(-1.0f), "-1.0");
}

TEST(FloatToStringTest, Billion) {
    EXPECT_EQ(FloatToString(1e9f), "1000000000.0");
}

TEST(FloatToStringTest, Small) {
    EXPECT_NE(FloatToString(std::numeric_limits<float>::epsilon()), "0.0");
}

TEST(FloatToStringTest, Highest) {
    const auto highest = std::numeric_limits<float>::max();
    const auto expected_highest = 340282346638528859811704183484516925440.0f;
    if (highest < expected_highest || highest > expected_highest) {
        GTEST_SKIP() << "std::numeric_limits<float>::max() is not as expected for "
                        "this target";
    }
    EXPECT_EQ(FloatToString(std::numeric_limits<float>::max()),
              "340282346638528859811704183484516925440.0");
}

TEST(FloatToStringTest, Lowest) {
    // Some compilers complain if you test floating point numbers for equality.
    // So say it via two inequalities.
    const auto lowest = std::numeric_limits<float>::lowest();
    const auto expected_lowest = -340282346638528859811704183484516925440.0f;
    if (lowest < expected_lowest || lowest > expected_lowest) {
        GTEST_SKIP() << "std::numeric_limits<float>::lowest() is not as expected for "
                        "this target";
    }
    EXPECT_EQ(FloatToString(std::numeric_limits<float>::lowest()),
              "-340282346638528859811704183484516925440.0");
}

TEST(FloatToStringTest, Precision) {
    EXPECT_EQ(FloatToString(1e-8f), "0.00000001");
    EXPECT_EQ(FloatToString(1e-9f), "0.000000001");
    EXPECT_EQ(FloatToString(1e-10f), "1.00000001e-10");
    EXPECT_EQ(FloatToString(1e-20f), "9.99999968e-21");
}

// FloatToBitPreservingString
//
// First replicate the tests for FloatToString

TEST(FloatToBitPreservingStringTest, Zero) {
    EXPECT_EQ(FloatToBitPreservingString(0.0f), "0.0");
}

TEST(FloatToBitPreservingStringTest, One) {
    EXPECT_EQ(FloatToBitPreservingString(1.0f), "1.0");
}

TEST(FloatToBitPreservingStringTest, MinusOne) {
    EXPECT_EQ(FloatToBitPreservingString(-1.0f), "-1.0");
}

TEST(FloatToBitPreservingStringTest, Billion) {
    EXPECT_EQ(FloatToBitPreservingString(1e9f), "1000000000.0");
}

TEST(FloatToBitPreservingStringTest, Small) {
    EXPECT_NE(FloatToBitPreservingString(std::numeric_limits<float>::epsilon()), "0.0");
}

TEST(FloatToBitPreservingStringTest, Highest) {
    const auto highest = std::numeric_limits<float>::max();
    const auto expected_highest = 340282346638528859811704183484516925440.0f;
    if (highest < expected_highest || highest > expected_highest) {
        GTEST_SKIP() << "std::numeric_limits<float>::max() is not as expected for "
                        "this target";
    }
    EXPECT_EQ(FloatToBitPreservingString(std::numeric_limits<float>::max()),
              "340282346638528859811704183484516925440.0");
}

TEST(FloatToBitPreservingStringTest, Lowest) {
    // Some compilers complain if you test floating point numbers for equality.
    // So say it via two inequalities.
    const auto lowest = std::numeric_limits<float>::lowest();
    const auto expected_lowest = -340282346638528859811704183484516925440.0f;
    if (lowest < expected_lowest || lowest > expected_lowest) {
        GTEST_SKIP() << "std::numeric_limits<float>::lowest() is not as expected for "
                        "this target";
    }
    EXPECT_EQ(FloatToBitPreservingString(std::numeric_limits<float>::lowest()),
              "-340282346638528859811704183484516925440.0");
}

// Special cases for bit-preserving output.

TEST(FloatToBitPreservingStringTest, NegativeZero) {
    EXPECT_EQ(FloatToBitPreservingString(std::copysign(0.0f, -5.0f)), "-0.0");
}

TEST(FloatToBitPreservingStringTest, ZeroAsBits) {
    EXPECT_EQ(FloatToBitPreservingString(MakeFloat(0, 0, 0)), "0.0");
    EXPECT_EQ(FloatToBitPreservingString(MakeFloat(1, 0, 0)), "-0.0");
}

TEST(FloatToBitPreservingStringTest, OneBits) {
    EXPECT_EQ(FloatToBitPreservingString(MakeFloat(0, 127, 0)), "1.0");
    EXPECT_EQ(FloatToBitPreservingString(MakeFloat(1, 127, 0)), "-1.0");
}

TEST(FloatToBitPreservingStringTest, SmallestDenormal) {
    EXPECT_EQ(FloatToBitPreservingString(MakeFloat(0, 0, 1)), "0x1p-149");
    EXPECT_EQ(FloatToBitPreservingString(MakeFloat(1, 0, 1)), "-0x1p-149");
}

TEST(FloatToBitPreservingStringTest, BiggerDenormal) {
    EXPECT_EQ(FloatToBitPreservingString(MakeFloat(0, 0, 2)), "0x1p-148");
    EXPECT_EQ(FloatToBitPreservingString(MakeFloat(1, 0, 2)), "-0x1p-148");
}

TEST(FloatToBitPreservingStringTest, LargestDenormal) {
    EXPECT_EQ(FloatToBitPreservingString(MakeFloat(0, 0, 0x7fffff)), "0x1.fffffcp-127");
}

TEST(FloatToBitPreservingStringTest, Subnormal_cafebe) {
    EXPECT_EQ(FloatToBitPreservingString(MakeFloat(0, 0, 0xcafebe)), "0x1.2bfaf8p-127");
    EXPECT_EQ(FloatToBitPreservingString(MakeFloat(1, 0, 0xcafebe)), "-0x1.2bfaf8p-127");
}

TEST(FloatToBitPreservingStringTest, Subnormal_aaaaa) {
    EXPECT_EQ(FloatToBitPreservingString(MakeFloat(0, 0, 0xaaaaa)), "0x1.55554p-130");
    EXPECT_EQ(FloatToBitPreservingString(MakeFloat(1, 0, 0xaaaaa)), "-0x1.55554p-130");
}

TEST(FloatToBitPreservingStringTest, Infinity) {
    EXPECT_EQ(FloatToBitPreservingString(MakeFloat(0, 255, 0)), "0x1p+128");
    EXPECT_EQ(FloatToBitPreservingString(MakeFloat(1, 255, 0)), "-0x1p+128");
}

// TODO(dneto): It's unclear how Infinity and NaN should be handled.
// https://github.com/gpuweb/gpuweb/issues/1769
// Windows x86-64 sets the high mantissa bit on NaNs.
// Disable NaN tests for now.

TEST(FloatToBitPreservingStringTest, DISABLED_NaN_MsbOnly) {
    EXPECT_EQ(FloatToBitPreservingString(MakeFloat(0, 255, 0x400000)), "0x1.8p+128");
    EXPECT_EQ(FloatToBitPreservingString(MakeFloat(1, 255, 0x400000)), "-0x1.8p+128");
}

TEST(FloatToBitPreservingStringTest, DISABLED_NaN_LsbOnly) {
    EXPECT_EQ(FloatToBitPreservingString(MakeFloat(0, 255, 0x1)), "0x1.000002p+128");
    EXPECT_EQ(FloatToBitPreservingString(MakeFloat(1, 255, 0x1)), "-0x1.000002p+128");
}

TEST(FloatToBitPreservingStringTest, DISABLED_NaN_NonMsb) {
    EXPECT_EQ(FloatToBitPreservingString(MakeFloat(0, 255, 0x20101f)), "0x1.40203ep+128");
    EXPECT_EQ(FloatToBitPreservingString(MakeFloat(1, 255, 0x20101f)), "-0x1.40203ep+128");
}

}  // namespace
}  // namespace tint::writer
