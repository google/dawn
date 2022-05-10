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
float MakeFloat(uint32_t sign, uint32_t biased_exponent, uint32_t mantissa) {
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

// Makes an IEEE 754 binary64 floating point number with
// - 0 sign if sign is 0, 1 otherwise
// - 'exponent_bits' is placed in the exponent space.
//   So, the exponent bias must already be included.
double MakeDouble(uint64_t sign, uint64_t biased_exponent, uint64_t mantissa) {
    const uint64_t sign_bit = sign ? 0x8000000000000000u : 0u;
    // The binary64 exponent is 11 bits, just below the sign.
    const uint64_t exponent_bits = (biased_exponent & 0x7FFull) << 52;
    // The mantissa is the bottom 52 bits.
    const uint64_t mantissa_bits = (mantissa & 0xFFFFFFFFFFFFFull);

    uint64_t bits = sign_bit | exponent_bits | mantissa_bits;
    double result = 0.0;
    static_assert(sizeof(result) == sizeof(bits),
                  "expected double and uint64_t to be the same size");
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
        ASSERT_TRUE(c->Is<ast::IntLiteralExpression>());
        EXPECT_EQ(c->As<ast::IntLiteralExpression>()->value, 234);
        EXPECT_EQ(c->As<ast::IntLiteralExpression>()->suffix,
                  ast::IntLiteralExpression::Suffix::kNone);
        EXPECT_EQ(c->source.range, (Source::Range{{1u, 1u}, {1u, 4u}}));
    }
    {
        auto p = parser("234i");
        auto c = p->const_literal();
        EXPECT_TRUE(c.matched);
        EXPECT_FALSE(c.errored);
        EXPECT_FALSE(p->has_error()) << p->error();
        ASSERT_NE(c.value, nullptr);
        ASSERT_TRUE(c->Is<ast::IntLiteralExpression>());
        EXPECT_EQ(c->As<ast::IntLiteralExpression>()->value, 234);
        EXPECT_EQ(c->As<ast::IntLiteralExpression>()->suffix,
                  ast::IntLiteralExpression::Suffix::kI);
        EXPECT_EQ(c->source.range, (Source::Range{{1u, 1u}, {1u, 5u}}));
    }
    {
        auto p = parser("-234");
        auto c = p->const_literal();
        EXPECT_TRUE(c.matched);
        EXPECT_FALSE(c.errored);
        EXPECT_FALSE(p->has_error()) << p->error();
        ASSERT_NE(c.value, nullptr);
        ASSERT_TRUE(c->Is<ast::IntLiteralExpression>());
        EXPECT_EQ(c->As<ast::IntLiteralExpression>()->value, -234);
        EXPECT_EQ(c->As<ast::IntLiteralExpression>()->suffix,
                  ast::IntLiteralExpression::Suffix::kNone);
        EXPECT_EQ(c->source.range, (Source::Range{{1u, 1u}, {1u, 5u}}));
    }
    {
        auto p = parser("-234i");
        auto c = p->const_literal();
        EXPECT_TRUE(c.matched);
        EXPECT_FALSE(c.errored);
        EXPECT_FALSE(p->has_error()) << p->error();
        ASSERT_NE(c.value, nullptr);
        ASSERT_TRUE(c->Is<ast::IntLiteralExpression>());
        EXPECT_EQ(c->As<ast::IntLiteralExpression>()->value, -234);
        EXPECT_EQ(c->As<ast::IntLiteralExpression>()->suffix,
                  ast::IntLiteralExpression::Suffix::kI);
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
    ASSERT_TRUE(c->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(c->As<ast::IntLiteralExpression>()->value, 234);
    EXPECT_EQ(c->As<ast::IntLiteralExpression>()->suffix, ast::IntLiteralExpression::Suffix::kU);
    EXPECT_EQ(c->source.range, (Source::Range{{1u, 1u}, {1u, 5u}}));
}

TEST_F(ParserImplTest, ConstLiteral_Uint_Negative) {
    auto p = parser("-234u");
    auto c = p->const_literal();
    EXPECT_FALSE(c.matched);
    EXPECT_TRUE(c.errored);
    EXPECT_EQ(p->error(), "1:1: unsigned literal cannot be negative");
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
    EXPECT_DOUBLE_EQ(c->As<ast::FloatLiteralExpression>()->value, 234e12);
    EXPECT_EQ(c->As<ast::FloatLiteralExpression>()->suffix,
              ast::FloatLiteralExpression::Suffix::kNone);
    EXPECT_EQ(c->source.range, (Source::Range{{1u, 1u}, {1u, 8u}}));
}

TEST_F(ParserImplTest, ConstLiteral_FloatF) {
    auto p = parser("234.e12f");
    auto c = p->const_literal();
    EXPECT_TRUE(c.matched);
    EXPECT_FALSE(c.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(c.value, nullptr);
    ASSERT_TRUE(c->Is<ast::FloatLiteralExpression>());
    EXPECT_DOUBLE_EQ(c->As<ast::FloatLiteralExpression>()->value, 234e12);
    EXPECT_EQ(c->As<ast::FloatLiteralExpression>()->suffix,
              ast::FloatLiteralExpression::Suffix::kF);
    EXPECT_EQ(c->source.range, (Source::Range{{1u, 1u}, {1u, 9u}}));
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
    EXPECT_EQ(p->error(), "1:1: magnitude too small to be represented as f32");
    ASSERT_EQ(c.value, nullptr);
}

TEST_F(ParserImplTest, ConstLiteral_InvalidFloat_TooLargeNegative) {
    auto p = parser("-1.2e+256");
    auto c = p->const_literal();
    EXPECT_FALSE(c.matched);
    EXPECT_TRUE(c.errored);
    EXPECT_EQ(p->error(), "1:1: value too small for f32");
    ASSERT_EQ(c.value, nullptr);
}

TEST_F(ParserImplTest, ConstLiteral_InvalidFloat_TooLargePositive) {
    auto p = parser("1.2e+256");
    auto c = p->const_literal();
    EXPECT_FALSE(c.matched);
    EXPECT_TRUE(c.errored);
    EXPECT_EQ(p->error(), "1:1: value too large for f32");
    ASSERT_EQ(c.value, nullptr);
}

struct FloatLiteralTestCase {
    std::string input;
    double expected;
    bool operator==(const FloatLiteralTestCase& other) const {
        return (input == other.input) && std::equal_to<double>()(expected, other.expected);
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
    EXPECT_DOUBLE_EQ(c->As<ast::FloatLiteralExpression>()->value, params.expected);
    if (params.input.back() == 'f') {
        EXPECT_EQ(c->As<ast::FloatLiteralExpression>()->suffix,
                  ast::FloatLiteralExpression::Suffix::kF);
    } else {
        EXPECT_EQ(c->As<ast::FloatLiteralExpression>()->suffix,
                  ast::FloatLiteralExpression::Suffix::kNone);
    }
}
using FloatLiteralTestCaseList = std::vector<FloatLiteralTestCase>;

FloatLiteralTestCaseList DecimalFloatCases() {
    return FloatLiteralTestCaseList{
        {"0.0", 0.0},                        // Zero
        {"1.0", 1.0},                        // One
        {"-1.0", -1.0},                      // MinusOne
        {"1000000000.0", 1e9},               // Billion
        {"-0.0", std::copysign(0.0, -5.0)},  // NegativeZero
        {"0.0", MakeDouble(0, 0, 0)},        // Zero
        {"-0.0", MakeDouble(1, 0, 0)},       // NegativeZero
        {"1.0", MakeDouble(0, 1023, 0)},     // One
        {"-1.0", MakeDouble(1, 1023, 0)},    // NegativeOne
    };
}

INSTANTIATE_TEST_SUITE_P(ParserImplFloatLiteralTest_Float,
                         ParserImplFloatLiteralTest,
                         testing::ValuesIn(DecimalFloatCases()));

const double NegInf = MakeDouble(1, 0x7FF, 0);
const double PosInf = MakeDouble(0, 0x7FF, 0);
FloatLiteralTestCaseList HexFloatCases() {
    return FloatLiteralTestCaseList{
        // Regular numbers
        {"0x0p+0", 0.0},
        {"0x1p+0", 1.0},
        {"0x1p+1", 2.0},
        {"0x1.8p+1", 3.0},
        {"0x1.99999ap-4", 0.10000000149011612},
        {"0x1p-1", 0.5},
        {"0x1p-2", 0.25},
        {"0x1.8p-1", 0.75},
        {"-0x0p+0", -0.0},
        {"-0x1p+0", -1.0},
        {"-0x1p-1", -0.5},
        {"-0x1p-2", -0.25},
        {"-0x1.8p-1", -0.75},

        // Large numbers
        {"0x1p+9", 512.0},
        {"0x1p+10", 1024.0},
        {"0x1.02p+10", 1024.0 + 8.0},
        {"-0x1p+9", -512.0},
        {"-0x1p+10", -1024.0},
        {"-0x1.02p+10", -1024.0 - 8.0},

        // Small numbers
        {"0x1p-9", 1.0 / 512.0},
        {"0x1p-10", 1.0 / 1024.0},
        {"0x1.02p-3", 1.0 / 1024.0 + 1.0 / 8.0},
        {"-0x1p-9", 1.0 / -512.0},
        {"-0x1p-10", 1.0 / -1024.0},
        {"-0x1.02p-3", 1.0 / -1024.0 - 1.0 / 8.0},

        // Near lowest non-denorm
        {"0x1p-124", std::ldexp(1.0 * 8.0, -127)},
        {"0x1p-125", std::ldexp(1.0 * 4.0, -127)},
        {"-0x1p-124", -std::ldexp(1.0 * 8.0, -127)},
        {"-0x1p-125", -std::ldexp(1.0 * 4.0, -127)},

        // Lowest non-denorm
        {"0x1p-126", std::ldexp(1.0 * 2.0, -127)},
        {"-0x1p-126", -std::ldexp(1.0 * 2.0, -127)},

        // Denormalized values
        {"0x1p-127", std::ldexp(1.0, -127)},
        {"0x1p-128", std::ldexp(1.0 / 2.0, -127)},
        {"0x1p-129", std::ldexp(1.0 / 4.0, -127)},
        {"0x1p-130", std::ldexp(1.0 / 8.0, -127)},
        {"-0x1p-127", -std::ldexp(1.0, -127)},
        {"-0x1p-128", -std::ldexp(1.0 / 2.0, -127)},
        {"-0x1p-129", -std::ldexp(1.0 / 4.0, -127)},
        {"-0x1p-130", -std::ldexp(1.0 / 8.0, -127)},

        {"0x1.8p-127", std::ldexp(1.0, -127) + (std::ldexp(1.0, -127) / 2.0)},
        {"0x1.8p-128", std::ldexp(1.0, -127) / 2.0 + (std::ldexp(1.0, -127) / 4.0)},

        // F32 extremities
        {"0x1p-149", static_cast<double>(MakeFloat(0, 0, 1))},                 // +SmallestDenormal
        {"0x1p-148", static_cast<double>(MakeFloat(0, 0, 2))},                 // +BiggerDenormal
        {"0x1.fffffcp-127", static_cast<double>(MakeFloat(0, 0, 0x7fffff))},   // +LargestDenormal
        {"-0x1p-149", static_cast<double>(MakeFloat(1, 0, 1))},                // -SmallestDenormal
        {"-0x1p-148", static_cast<double>(MakeFloat(1, 0, 2))},                // -BiggerDenormal
        {"-0x1.fffffcp-127", static_cast<double>(MakeFloat(1, 0, 0x7fffff))},  // -LargestDenormal

        {"0x1.2bfaf8p-127", static_cast<double>(MakeFloat(0, 0, 0xcafebe))},   // +Subnormal
        {"-0x1.2bfaf8p-127", static_cast<double>(MakeFloat(1, 0, 0xcafebe))},  // -Subnormal
        {"0x1.55554p-130", static_cast<double>(MakeFloat(0, 0, 0xaaaaa))},     // +Subnormal
        {"-0x1.55554p-130", static_cast<double>(MakeFloat(1, 0, 0xaaaaa))},    // -Subnormal

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
        {"0x1p-500", 0.0},  // Exponent underflows
        {"-0x1p-500", -0.0},
        {"0x0.00000000001p-126", 0.0},  // Fraction causes underflow
        {"-0x0.0000000001p-127", -0.0},
        {"0x0.01p-142", 0.0},
        {"-0x0.01p-142", -0.0},    // Fraction causes additional underflow
        {"0x1.0p-2147483520", 0},  // -(INT_MAX - 127) (smallest valid exponent)

        // Zero with non-zero exponent -> Zero
        {"0x0p+0", 0.0},
        {"0x0p+1", 0.0},
        {"0x0p-1", 0.0},
        {"0x0p+9999999999", 0.0},
        {"0x0p-9999999999", 0.0},
        // Same, but with very large positive exponents that would cause overflow
        // if the mantissa were non-zero.
        {"0x0p+4000000000", 0.0},    // 4 billion:
        {"0x0p+40000000000", 0.0},   // 40 billion
        {"-0x0p+40000000000", 0.0},  // As above 2, but negative mantissa
        {"-0x0p+400000000000", 0.0},
        {"0x0.00p+4000000000", 0.0},  // As above 4, but with fractional part
        {"0x0.00p+40000000000", 0.0},
        {"-0x0.00p+40000000000", 0.0},
        {"-0x0.00p+400000000000", 0.0},
        {"0x0p-4000000000", 0.0},  // As above 8, but with negative exponents
        {"0x0p-40000000000", 0.0},
        {"-0x0p-40000000000", 0.0},
        {"-0x0p-400000000000", 0.0},
        {"0x0.00p-4000000000", 0.0},
        {"0x0.00p-40000000000", 0.0},
        {"-0x0.00p-40000000000", 0.0},
        {"-0x0.00p-400000000000", 0.0},

        // Test parsing
        {"0x0p0", 0.0},
        {"0x0p-0", 0.0},
        {"0x0p+000", 0.0},
        {"0x00000000000000p+000000000000000", 0.0},
        {"0x00000000000000p-000000000000000", 0.0},
        {"0x00000000000001p+000000000000000", 1.0},
        {"0x00000000000001p-000000000000000", 1.0},
        {"0x0000000000000000000001.99999ap-000000000000000004", 0.10000000149011612},
        {"0x2p+0", 2.0},
        {"0xFFp+0", 255.0},
        {"0x0.8p+0", 0.5},
        {"0x0.4p+0", 0.25},
        {"0x0.4p+1", 2 * 0.25},
        {"0x0.4p+2", 4 * 0.25},
        {"0x123Ep+1", 9340.0},
        {"-0x123Ep+1", -9340.0},
        {"0x1a2b3cP12", 7.024656384e+09},
        {"-0x1a2b3cP12", -7.024656384e+09},

        // Examples without a binary exponent part.
        {"0x1.", 1.0},
        {"0x.8", 0.5},
        {"0x1.8", 1.5},
        {"-0x1.", -1.0},
        {"-0x.8", -0.5},
        {"-0x1.8", -1.5},

        // Examples with a binary exponent and a 'f' suffix.
        {"0x1.p0f", 1.0},
        {"0x.8p2f", 2.0},
        {"0x1.8p-1f", 0.75},
        {"0x2p-2f", 0.5},  // No binary point
        {"-0x1.p0f", -1.0},
        {"-0x.8p2f", -2.0},
        {"-0x1.8p-1f", -0.75},
        {"-0x2p-2f", -0.5},  // No binary point
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
    EXPECT_DOUBLE_EQ(c->As<ast::FloatLiteralExpression>()->value,
                     std::numeric_limits<float>::max());
    EXPECT_EQ(c->As<ast::FloatLiteralExpression>()->suffix,
              ast::FloatLiteralExpression::Suffix::kNone);
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
    EXPECT_DOUBLE_EQ(c->As<ast::FloatLiteralExpression>()->value,
                     std::numeric_limits<float>::lowest());
    EXPECT_EQ(c->As<ast::FloatLiteralExpression>()->suffix,
              ast::FloatLiteralExpression::Suffix::kNone);
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
