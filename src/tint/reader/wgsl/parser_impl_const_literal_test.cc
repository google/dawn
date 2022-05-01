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

#include "src/tint/reader/wgsl/parser_impl_test_helper.h"

#include <cmath>
#include <cstring>

#include "gmock/gmock.h"

namespace tint::reader::wgsl {
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

TEST_F(ParserImplTest, ConstLiteral_Int) {
    {
        auto p = parser("234");
        auto c = p->const_literal();
        EXPECT_TRUE(c.matched);
        EXPECT_FALSE(c.errored);
        EXPECT_FALSE(p->has_error()) << p->error();
        ASSERT_NE(c.value, nullptr);
        ASSERT_TRUE(c->Is<ast::SintLiteralExpression>());
        EXPECT_EQ(c->As<ast::SintLiteralExpression>()->value, 234);
        EXPECT_EQ(c->source.range, (Source::Range{{1u, 1u}, {1u, 4u}}));
    }
    {
        auto p = parser("234i");
        auto c = p->const_literal();
        EXPECT_TRUE(c.matched);
        EXPECT_FALSE(c.errored);
        EXPECT_FALSE(p->has_error()) << p->error();
        ASSERT_NE(c.value, nullptr);
        ASSERT_TRUE(c->Is<ast::SintLiteralExpression>());
        EXPECT_EQ(c->As<ast::SintLiteralExpression>()->value, 234);
        EXPECT_EQ(c->source.range, (Source::Range{{1u, 1u}, {1u, 5u}}));
    }
    {
        auto p = parser("-234");
        auto c = p->const_literal();
        EXPECT_TRUE(c.matched);
        EXPECT_FALSE(c.errored);
        EXPECT_FALSE(p->has_error()) << p->error();
        ASSERT_NE(c.value, nullptr);
        ASSERT_TRUE(c->Is<ast::SintLiteralExpression>());
        EXPECT_EQ(c->As<ast::SintLiteralExpression>()->value, -234);
        EXPECT_EQ(c->source.range, (Source::Range{{1u, 1u}, {1u, 5u}}));
    }
    {
        auto p = parser("-234i");
        auto c = p->const_literal();
        EXPECT_TRUE(c.matched);
        EXPECT_FALSE(c.errored);
        EXPECT_FALSE(p->has_error()) << p->error();
        ASSERT_NE(c.value, nullptr);
        ASSERT_TRUE(c->Is<ast::SintLiteralExpression>());
        EXPECT_EQ(c->As<ast::SintLiteralExpression>()->value, -234);
        EXPECT_EQ(c->source.range, (Source::Range{{1u, 1u}, {1u, 6u}}));
    }
}

TEST_F(ParserImplTest, ConstLiteral_Uint) {
    auto p = parser("234u");
    auto c = p->const_literal();
    EXPECT_TRUE(c.matched);
    EXPECT_FALSE(c.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(c.value, nullptr);
    ASSERT_TRUE(c->Is<ast::UintLiteralExpression>());
    EXPECT_EQ(c->As<ast::UintLiteralExpression>()->value, 234u);
    EXPECT_EQ(c->source.range, (Source::Range{{1u, 1u}, {1u, 5u}}));
}

TEST_F(ParserImplTest, ConstLiteral_Uint_Negative) {
    auto p = parser("-234u");
    auto c = p->const_literal();
    EXPECT_FALSE(c.matched);
    EXPECT_TRUE(c.errored);
    EXPECT_EQ(p->error(), "1:1: u32 (-234) must not be negative");
    ASSERT_EQ(c.value, nullptr);
}

TEST_F(ParserImplTest, ConstLiteral_Float) {
    auto p = parser("234.e12");
    auto c = p->const_literal();
    EXPECT_TRUE(c.matched);
    EXPECT_FALSE(c.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(c.value, nullptr);
    ASSERT_TRUE(c->Is<ast::FloatLiteralExpression>());
    EXPECT_FLOAT_EQ(c->As<ast::FloatLiteralExpression>()->value, 234e12f);
    EXPECT_EQ(c->source.range, (Source::Range{{1u, 1u}, {1u, 8u}}));
}

TEST_F(ParserImplTest, ConstLiteral_InvalidFloat_IncompleteExponent) {
    auto p = parser("1.0e+");
    auto c = p->const_literal();
    EXPECT_FALSE(c.matched);
    EXPECT_TRUE(c.errored);
    EXPECT_EQ(p->error(), "1:1: incomplete exponent for floating point literal: 1.0e+");
    ASSERT_EQ(c.value, nullptr);
}

TEST_F(ParserImplTest, ConstLiteral_InvalidFloat_TooSmallMagnitude) {
    auto p = parser("1e-256");
    auto c = p->const_literal();
    EXPECT_FALSE(c.matched);
    EXPECT_TRUE(c.errored);
    EXPECT_EQ(p->error(), "1:1: f32 (1e-256) magnitude too small, not representable");
    ASSERT_EQ(c.value, nullptr);
}

TEST_F(ParserImplTest, ConstLiteral_InvalidFloat_TooLargeNegative) {
    auto p = parser("-1.2e+256");
    auto c = p->const_literal();
    EXPECT_FALSE(c.matched);
    EXPECT_TRUE(c.errored);
    EXPECT_EQ(p->error(), "1:1: f32 (-1.2e+256) too large (negative)");
    ASSERT_EQ(c.value, nullptr);
}

TEST_F(ParserImplTest, ConstLiteral_InvalidFloat_TooLargePositive) {
    auto p = parser("1.2e+256");
    auto c = p->const_literal();
    EXPECT_FALSE(c.matched);
    EXPECT_TRUE(c.errored);
    EXPECT_EQ(p->error(), "1:1: f32 (1.2e+256) too large (positive)");
    ASSERT_EQ(c.value, nullptr);
}

// Returns true if the given non-Nan float numbers are equal.
bool FloatEqual(float a, float b) {
    // Avoid Clang complaining about equality test on float.
    // -Wfloat-equal.
    return (a <= b) && (a >= b);
}

struct FloatLiteralTestCase {
    std::string input;
    float expected;
    bool operator==(const FloatLiteralTestCase& other) const {
        return (input == other.input) && FloatEqual(expected, other.expected);
    }
};

inline std::ostream& operator<<(std::ostream& out, FloatLiteralTestCase data) {
    out << data.input;
    return out;
}

class ParserImplFloatLiteralTest : public ParserImplTestWithParam<FloatLiteralTestCase> {};
TEST_P(ParserImplFloatLiteralTest, Parse) {
    auto params = GetParam();
    SCOPED_TRACE(params.input);
    auto p = parser(params.input);
    auto c = p->const_literal();
    EXPECT_TRUE(c.matched);
    EXPECT_FALSE(c.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(c.value, nullptr);
    ASSERT_TRUE(c->Is<ast::FloatLiteralExpression>());
    EXPECT_FLOAT_EQ(c->As<ast::FloatLiteralExpression>()->value, params.expected);
}

using FloatLiteralTestCaseList = std::vector<FloatLiteralTestCase>;

FloatLiteralTestCaseList DecimalFloatCases() {
    return FloatLiteralTestCaseList{
        {"0.0", 0.0f},                         // Zero
        {"1.0", 1.0f},                         // One
        {"-1.0", -1.0f},                       // MinusOne
        {"1000000000.0", 1e9f},                // Billion
        {"-0.0", std::copysign(0.0f, -5.0f)},  // NegativeZero
        {"0.0", MakeFloat(0, 0, 0)},           // Zero
        {"-0.0", MakeFloat(1, 0, 0)},          // NegativeZero
        {"1.0", MakeFloat(0, 127, 0)},         // One
        {"-1.0", MakeFloat(1, 127, 0)},        // NegativeOne
    };
}

INSTANTIATE_TEST_SUITE_P(ParserImplFloatLiteralTest_Float,
                         ParserImplFloatLiteralTest,
                         testing::ValuesIn(DecimalFloatCases()));

const float NegInf = MakeFloat(1, 255, 0);
const float PosInf = MakeFloat(0, 255, 0);
FloatLiteralTestCaseList HexFloatCases() {
    return FloatLiteralTestCaseList{
        // Regular numbers
        {"0x0p+0", 0.f},
        {"0x1p+0", 1.f},
        {"0x1p+1", 2.f},
        {"0x1.8p+1", 3.f},
        {"0x1.99999ap-4", 0.1f},
        {"0x1p-1", 0.5f},
        {"0x1p-2", 0.25f},
        {"0x1.8p-1", 0.75f},
        {"-0x0p+0", -0.f},
        {"-0x1p+0", -1.f},
        {"-0x1p-1", -0.5f},
        {"-0x1p-2", -0.25f},
        {"-0x1.8p-1", -0.75f},

        // Large numbers
        {"0x1p+9", 512.f},
        {"0x1p+10", 1024.f},
        {"0x1.02p+10", 1024.f + 8.f},
        {"-0x1p+9", -512.f},
        {"-0x1p+10", -1024.f},
        {"-0x1.02p+10", -1024.f - 8.f},

        // Small numbers
        {"0x1p-9", 1.0f / 512.f},
        {"0x1p-10", 1.0f / 1024.f},
        {"0x1.02p-3", 1.0f / 1024.f + 1.0f / 8.f},
        {"-0x1p-9", 1.0f / -512.f},
        {"-0x1p-10", 1.0f / -1024.f},
        {"-0x1.02p-3", 1.0f / -1024.f - 1.0f / 8.f},

        // Near lowest non-denorm
        {"0x1p-124", std::ldexp(1.f * 8.f, -127)},
        {"0x1p-125", std::ldexp(1.f * 4.f, -127)},
        {"-0x1p-124", -std::ldexp(1.f * 8.f, -127)},
        {"-0x1p-125", -std::ldexp(1.f * 4.f, -127)},

        // Lowest non-denorm
        {"0x1p-126", std::ldexp(1.f * 2.f, -127)},
        {"-0x1p-126", -std::ldexp(1.f * 2.f, -127)},

        // Denormalized values
        {"0x1p-127", std::ldexp(1.f, -127)},
        {"0x1p-128", std::ldexp(1.f / 2.f, -127)},
        {"0x1p-129", std::ldexp(1.f / 4.f, -127)},
        {"0x1p-130", std::ldexp(1.f / 8.f, -127)},
        {"-0x1p-127", -std::ldexp(1.f, -127)},
        {"-0x1p-128", -std::ldexp(1.f / 2.f, -127)},
        {"-0x1p-129", -std::ldexp(1.f / 4.f, -127)},
        {"-0x1p-130", -std::ldexp(1.f / 8.f, -127)},

        {"0x1.8p-127", std::ldexp(1.f, -127) + (std::ldexp(1.f, -127) / 2.f)},
        {"0x1.8p-128", std::ldexp(1.f, -127) / 2.f + (std::ldexp(1.f, -127) / 4.f)},

        {"0x1p-149", MakeFloat(0, 0, 1)},                 // +SmallestDenormal
        {"0x1p-148", MakeFloat(0, 0, 2)},                 // +BiggerDenormal
        {"0x1.fffffcp-127", MakeFloat(0, 0, 0x7fffff)},   // +LargestDenormal
        {"-0x1p-149", MakeFloat(1, 0, 1)},                // -SmallestDenormal
        {"-0x1p-148", MakeFloat(1, 0, 2)},                // -BiggerDenormal
        {"-0x1.fffffcp-127", MakeFloat(1, 0, 0x7fffff)},  // -LargestDenormal

        {"0x1.2bfaf8p-127", MakeFloat(0, 0, 0xcafebe)},   // +Subnormal
        {"-0x1.2bfaf8p-127", MakeFloat(1, 0, 0xcafebe)},  // -Subnormal
        {"0x1.55554p-130", MakeFloat(0, 0, 0xaaaaa)},     // +Subnormal
        {"-0x1.55554p-130", MakeFloat(1, 0, 0xaaaaa)},    // -Subnormal

        // Nan -> Infinity
        {"0x1.8p+128", PosInf},
        {"0x1.0002p+128", PosInf},
        {"0x1.0018p+128", PosInf},
        {"0x1.01ep+128", PosInf},
        {"0x1.fffffep+128", PosInf},
        {"-0x1.8p+128", NegInf},
        {"-0x1.0002p+128", NegInf},
        {"-0x1.0018p+128", NegInf},
        {"-0x1.01ep+128", NegInf},
        {"-0x1.fffffep+128", NegInf},

        // Infinity
        {"0x1p+128", PosInf},
        {"-0x1p+128", NegInf},
        {"0x32p+127", PosInf},
        {"0x32p+500", PosInf},
        {"-0x32p+127", NegInf},
        {"-0x32p+500", NegInf},

        // Overflow -> Infinity
        {"0x1p+129", PosInf},
        {"0x1.1p+128", PosInf},
        {"-0x1p+129", NegInf},
        {"-0x1.1p+128", NegInf},
        {"0x1.0p2147483520", PosInf},  // INT_MAX - 127 (largest valid exponent)

        // Underflow -> Zero
        {"0x1p-500", 0.f},  // Exponent underflows
        {"-0x1p-500", -0.f},
        {"0x0.00000000001p-126", 0.f},  // Fraction causes underflow
        {"-0x0.0000000001p-127", -0.f},
        {"0x0.01p-142", 0.f},
        {"-0x0.01p-142", -0.f},    // Fraction causes additional underflow
        {"0x1.0p-2147483520", 0},  // -(INT_MAX - 127) (smallest valid exponent)

        // Zero with non-zero exponent -> Zero
        {"0x0p+0", 0.f},
        {"0x0p+1", 0.f},
        {"0x0p-1", 0.f},
        {"0x0p+9999999999", 0.f},
        {"0x0p-9999999999", 0.f},
        // Same, but with very large positive exponents that would cause overflow
        // if the mantissa were non-zero.
        {"0x0p+4000000000", 0.f},    // 4 billion:
        {"0x0p+40000000000", 0.f},   // 40 billion
        {"-0x0p+40000000000", 0.f},  // As above 2, but negative mantissa
        {"-0x0p+400000000000", 0.f},
        {"0x0.00p+4000000000", 0.f},  // As above 4, but with fractional part
        {"0x0.00p+40000000000", 0.f},
        {"-0x0.00p+40000000000", 0.f},
        {"-0x0.00p+400000000000", 0.f},
        {"0x0p-4000000000", 0.f},  // As above 8, but with negative exponents
        {"0x0p-40000000000", 0.f},
        {"-0x0p-40000000000", 0.f},
        {"-0x0p-400000000000", 0.f},
        {"0x0.00p-4000000000", 0.f},
        {"0x0.00p-40000000000", 0.f},
        {"-0x0.00p-40000000000", 0.f},
        {"-0x0.00p-400000000000", 0.f},

        // Test parsing
        {"0x0p0", 0.f},
        {"0x0p-0", 0.f},
        {"0x0p+000", 0.f},
        {"0x00000000000000p+000000000000000", 0.f},
        {"0x00000000000000p-000000000000000", 0.f},
        {"0x00000000000001p+000000000000000", 1.f},
        {"0x00000000000001p-000000000000000", 1.f},
        {"0x0000000000000000000001.99999ap-000000000000000004", 0.1f},
        {"0x2p+0", 2.f},
        {"0xFFp+0", 255.f},
        {"0x0.8p+0", 0.5f},
        {"0x0.4p+0", 0.25f},
        {"0x0.4p+1", 2 * 0.25f},
        {"0x0.4p+2", 4 * 0.25f},
        {"0x123Ep+1", 9340.f},
        {"-0x123Ep+1", -9340.f},
        {"0x1a2b3cP12", 7.024656e+09f},
        {"-0x1a2b3cP12", -7.024656e+09f},

        // Examples without a binary exponent part.
        {"0x1.", 1.0f},
        {"0x.8", 0.5f},
        {"0x1.8", 1.5f},
        {"-0x1.", -1.0f},
        {"-0x.8", -0.5f},
        {"-0x1.8", -1.5f},

        // Examples with a binary exponent and a 'f' suffix.
        {"0x1.p0f", 1.0f},
        {"0x.8p2f", 2.0f},
        {"0x1.8p-1f", 0.75f},
        {"0x2p-2f", 0.5f},  // No binary point
        {"-0x1.p0f", -1.0f},
        {"-0x.8p2f", -2.0f},
        {"-0x1.8p-1f", -0.75f},
        {"-0x2p-2f", -0.5f},  // No binary point
    };
}
INSTANTIATE_TEST_SUITE_P(ParserImplFloatLiteralTest_HexFloat,
                         ParserImplFloatLiteralTest,
                         testing::ValuesIn(HexFloatCases()));

// Now test all the same hex float cases, but with 0X instead of 0x
template <typename ARR>
std::vector<FloatLiteralTestCase> UpperCase0X(const ARR& cases) {
    std::vector<FloatLiteralTestCase> result;
    result.reserve(cases.size());
    for (const auto& c : cases) {
        result.emplace_back(c);
        auto& input = result.back().input;
        const auto where = input.find("0x");
        if (where != std::string::npos) {
            input[where + 1] = 'X';
        }
    }
    return result;
}

using UpperCase0XTest = ::testing::Test;
TEST_F(UpperCase0XTest, Samples) {
    const auto cases = FloatLiteralTestCaseList{
        {"absent", 0.0}, {"0x", 1.0},      {"0X", 2.0},      {"-0x", 3.0},
        {"-0X", 4.0},    {"  0x1p1", 5.0}, {"  -0x1p", 6.0}, {" examine ", 7.0}};
    const auto expected = FloatLiteralTestCaseList{
        {"absent", 0.0}, {"0X", 1.0},      {"0X", 2.0},      {"-0X", 3.0},
        {"-0X", 4.0},    {"  0X1p1", 5.0}, {"  -0X1p", 6.0}, {" examine ", 7.0}};

    auto result = UpperCase0X(cases);
    EXPECT_THAT(result, ::testing::ElementsAreArray(expected));
}

INSTANTIATE_TEST_SUITE_P(ParserImplFloatLiteralTest_HexFloat_UpperCase0X,
                         ParserImplFloatLiteralTest,
                         testing::ValuesIn(UpperCase0X(HexFloatCases())));

struct InvalidLiteralTestCase {
    const char* input;
    const char* error_msg;
};
class ParserImplInvalidLiteralTest : public ParserImplTestWithParam<InvalidLiteralTestCase> {};
TEST_P(ParserImplInvalidLiteralTest, Parse) {
    auto params = GetParam();
    SCOPED_TRACE(params.input);
    auto p = parser(params.input);
    auto c = p->const_literal();
    EXPECT_FALSE(c.matched);
    EXPECT_TRUE(c.errored);
    EXPECT_EQ(p->error(), params.error_msg);
    ASSERT_EQ(c.value, nullptr);
}

InvalidLiteralTestCase invalid_hexfloat_mantissa_too_large_cases[] = {
    {"0x1.ffffffff8p0", "1:1: mantissa is too large for hex float"},
    {"0x1f.fffffff8p0", "1:1: mantissa is too large for hex float"},
    {"0x1ff.ffffff8p0", "1:1: mantissa is too large for hex float"},
    {"0x1fff.fffff8p0", "1:1: mantissa is too large for hex float"},
    {"0x1ffff.ffff8p0", "1:1: mantissa is too large for hex float"},
    {"0x1fffff.fff8p0", "1:1: mantissa is too large for hex float"},
    {"0x1ffffff.ff8p0", "1:1: mantissa is too large for hex float"},
    {"0x1fffffff.f8p0", "1:1: mantissa is too large for hex float"},
    {"0x1ffffffff.8p0", "1:1: mantissa is too large for hex float"},
    {"0x1ffffffff8.p0", "1:1: mantissa is too large for hex float"},
};
INSTANTIATE_TEST_SUITE_P(ParserImplInvalidLiteralTest_HexFloatMantissaTooLarge,
                         ParserImplInvalidLiteralTest,
                         testing::ValuesIn(invalid_hexfloat_mantissa_too_large_cases));

InvalidLiteralTestCase invalid_hexfloat_exponent_too_large_cases[] = {
    {"0x1p+2147483521", "1:1: exponent is too large for hex float"},
    {"0x1p-2147483521", "1:1: exponent is too large for hex float"},
    {"0x1p+4294967296", "1:1: exponent is too large for hex float"},
    {"0x1p-4294967296", "1:1: exponent is too large for hex float"},
};
INSTANTIATE_TEST_SUITE_P(ParserImplInvalidLiteralTest_HexFloatExponentTooLarge,
                         ParserImplInvalidLiteralTest,
                         testing::ValuesIn(invalid_hexfloat_exponent_too_large_cases));

InvalidLiteralTestCase invalid_hexfloat_exponent_missing_cases[] = {
    // Lower case p
    {"0x0p", "1:1: expected an exponent value for hex float"},
    {"0x0p+", "1:1: expected an exponent value for hex float"},
    {"0x0p-", "1:1: expected an exponent value for hex float"},
    {"0x1.0p", "1:1: expected an exponent value for hex float"},
    {"0x0.1p", "1:1: expected an exponent value for hex float"},
    // Upper case p
    {"0x0P", "1:1: expected an exponent value for hex float"},
    {"0x0P+", "1:1: expected an exponent value for hex float"},
    {"0x0P-", "1:1: expected an exponent value for hex float"},
    {"0x1.0P", "1:1: expected an exponent value for hex float"},
    {"0x0.1P", "1:1: expected an exponent value for hex float"},
};
INSTANTIATE_TEST_SUITE_P(ParserImplInvalidLiteralTest_HexFloatExponentMissing,
                         ParserImplInvalidLiteralTest,
                         testing::ValuesIn(invalid_hexfloat_exponent_missing_cases));

TEST_F(ParserImplTest, ConstLiteral_FloatHighest) {
    const auto highest = std::numeric_limits<float>::max();
    const auto expected_highest = 340282346638528859811704183484516925440.0f;
    if (highest < expected_highest || highest > expected_highest) {
        GTEST_SKIP() << "std::numeric_limits<float>::max() is not as expected for "
                        "this target";
    }
    auto p = parser("340282346638528859811704183484516925440.0");
    auto c = p->const_literal();
    EXPECT_TRUE(c.matched);
    EXPECT_FALSE(c.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(c.value, nullptr);
    ASSERT_TRUE(c->Is<ast::FloatLiteralExpression>());
    EXPECT_FLOAT_EQ(c->As<ast::FloatLiteralExpression>()->value, std::numeric_limits<float>::max());
    EXPECT_EQ(c->source.range, (Source::Range{{1u, 1u}, {1u, 42u}}));
}

TEST_F(ParserImplTest, ConstLiteral_FloatLowest) {
    // Some compilers complain if you test floating point numbers for equality.
    // So say it via two inequalities.
    const auto lowest = std::numeric_limits<float>::lowest();
    const auto expected_lowest = -340282346638528859811704183484516925440.0f;
    if (lowest < expected_lowest || lowest > expected_lowest) {
        GTEST_SKIP() << "std::numeric_limits<float>::lowest() is not as expected for "
                        "this target";
    }

    auto p = parser("-340282346638528859811704183484516925440.0");
    auto c = p->const_literal();
    EXPECT_TRUE(c.matched);
    EXPECT_FALSE(c.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(c.value, nullptr);
    ASSERT_TRUE(c->Is<ast::FloatLiteralExpression>());
    EXPECT_FLOAT_EQ(c->As<ast::FloatLiteralExpression>()->value,
                    std::numeric_limits<float>::lowest());
    EXPECT_EQ(c->source.range, (Source::Range{{1u, 1u}, {1u, 43u}}));
}

TEST_F(ParserImplTest, ConstLiteral_True) {
    auto p = parser("true");
    auto c = p->const_literal();
    EXPECT_TRUE(c.matched);
    EXPECT_FALSE(c.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(c.value, nullptr);
    ASSERT_TRUE(c->Is<ast::BoolLiteralExpression>());
    EXPECT_TRUE(c->As<ast::BoolLiteralExpression>()->value);
    EXPECT_EQ(c->source.range, (Source::Range{{1u, 1u}, {1u, 5u}}));
}

TEST_F(ParserImplTest, ConstLiteral_False) {
    auto p = parser("false");
    auto c = p->const_literal();
    EXPECT_TRUE(c.matched);
    EXPECT_FALSE(c.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(c.value, nullptr);
    ASSERT_TRUE(c->Is<ast::BoolLiteralExpression>());
    EXPECT_FALSE(c->As<ast::BoolLiteralExpression>()->value);
    EXPECT_EQ(c->source.range, (Source::Range{{1u, 1u}, {1u, 6u}}));
}

TEST_F(ParserImplTest, ConstLiteral_NoMatch) {
    auto p = parser("another-token");
    auto c = p->const_literal();
    EXPECT_FALSE(c.matched);
    EXPECT_FALSE(c.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_EQ(c.value, nullptr);
}

}  // namespace
}  // namespace tint::reader::wgsl
