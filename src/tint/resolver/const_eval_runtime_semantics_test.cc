// Copyright 2023 The Tint Authors.
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

#include "src/tint/resolver/const_eval_test.h"

#include "src/tint/constant/scalar.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

class ResolverConstEvalRuntimeSemanticsTest : public ResolverConstEvalTest {
  protected:
    /// Default constructor.
    ResolverConstEvalRuntimeSemanticsTest()
        : const_eval(ConstEval(*this, /* use_runtime_semantics */ true)) {}

    /// The ConstEval object used during testing (has runtime semantics enabled).
    ConstEval const_eval;

    /// @returns the contents of the diagnostics list as a string
    std::string error() {
        diag::Formatter::Style style{};
        style.print_newline_at_end = false;
        diag::Formatter formatter{style};
        return formatter.format(Diagnostics());
    }

    /// Helper to make a scalar constant::Value from a value.
    template <typename T>
    const constant::Value* Scalar(T value) {
        if constexpr (IsAbstract<T>) {
            if constexpr (IsFloatingPoint<T>) {
                return create<constant::Scalar<AFloat>>(create<type::AbstractFloat>(), value);
            } else if constexpr (IsIntegral<T>) {
                return create<constant::Scalar<AInt>>(create<type::AbstractInt>(), value);
            }
        } else if constexpr (IsFloatingPoint<T>) {
            return create<constant::Scalar<f32>>(create<type::F32>(), value);
        } else if constexpr (IsSignedIntegral<T>) {
            return create<constant::Scalar<i32>>(create<type::I32>(), value);
        } else if constexpr (IsUnsignedIntegral<T>) {
            return create<constant::Scalar<u32>>(create<type::U32>(), value);
        }
    }
};

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Add_AInt_Overflow) {
    auto* a = Scalar(AInt::Highest());
    auto* b = Scalar(AInt(1));
    auto result = const_eval.OpPlus(a->Type(), utils::Vector{a, b}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<AInt>(), 0);
    EXPECT_EQ(error(),
              R"(warning: '9223372036854775807 + 1' cannot be represented as 'abstract-int')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Add_AFloat_Overflow) {
    auto* a = Scalar(AFloat::Highest());
    auto* b = Scalar(AFloat::Highest());
    auto result = const_eval.OpPlus(a->Type(), utils::Vector{a, b}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<AFloat>(), 0.f);
    EXPECT_EQ(
        error(),
        R"(warning: '179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.0 + 179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.0' cannot be represented as 'abstract-float')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Add_F32_Overflow) {
    auto* a = Scalar(f32::Highest());
    auto* b = Scalar(f32::Highest());
    auto result = const_eval.OpPlus(a->Type(), utils::Vector{a, b}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(
        error(),
        R"(warning: '340282346638528859811704183484516925440.0 + 340282346638528859811704183484516925440.0' cannot be represented as 'f32')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Sub_AInt_Overflow) {
    auto* a = Scalar(AInt::Lowest());
    auto* b = Scalar(AInt(1));
    auto result = const_eval.OpMinus(a->Type(), utils::Vector{a, b}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<AInt>(), 0);
    EXPECT_EQ(error(),
              R"(warning: '-9223372036854775808 - 1' cannot be represented as 'abstract-int')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Sub_AFloat_Overflow) {
    auto* a = Scalar(AFloat::Lowest());
    auto* b = Scalar(AFloat::Highest());
    auto result = const_eval.OpMinus(a->Type(), utils::Vector{a, b}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<AFloat>(), 0.f);
    EXPECT_EQ(
        error(),
        R"(warning: '-179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.0 - 179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.0' cannot be represented as 'abstract-float')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Sub_F32_Overflow) {
    auto* a = Scalar(f32::Lowest());
    auto* b = Scalar(f32::Highest());
    auto result = const_eval.OpMinus(a->Type(), utils::Vector{a, b}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(
        error(),
        R"(warning: '-340282346638528859811704183484516925440.0 - 340282346638528859811704183484516925440.0' cannot be represented as 'f32')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Mul_AInt_Overflow) {
    auto* a = Scalar(AInt::Highest());
    auto* b = Scalar(AInt(2));
    auto result = const_eval.OpMultiply(a->Type(), utils::Vector{a, b}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<AInt>(), 0);
    EXPECT_EQ(error(),
              R"(warning: '9223372036854775807 * 2' cannot be represented as 'abstract-int')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Mul_AFloat_Overflow) {
    auto* a = Scalar(AFloat::Highest());
    auto* b = Scalar(AFloat::Highest());
    auto result = const_eval.OpMultiply(a->Type(), utils::Vector{a, b}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<AFloat>(), 0.f);
    EXPECT_EQ(
        error(),
        R"(warning: '179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.0 * 179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.0' cannot be represented as 'abstract-float')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Mul_F32_Overflow) {
    auto* a = Scalar(f32::Highest());
    auto* b = Scalar(f32::Highest());
    auto result = const_eval.OpMultiply(a->Type(), utils::Vector{a, b}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(
        error(),
        R"(warning: '340282346638528859811704183484516925440.0 * 340282346638528859811704183484516925440.0' cannot be represented as 'f32')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Div_AInt_ZeroDenominator) {
    auto* a = Scalar(AInt(42));
    auto* b = Scalar(AInt(0));
    auto result = const_eval.OpDivide(a->Type(), utils::Vector{a, b}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<AInt>(), 42);
    EXPECT_EQ(error(), R"(warning: '42 / 0' cannot be represented as 'abstract-int')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Div_I32_ZeroDenominator) {
    auto* a = Scalar(i32(42));
    auto* b = Scalar(i32(0));
    auto result = const_eval.OpDivide(a->Type(), utils::Vector{a, b}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<i32>(), 42);
    EXPECT_EQ(error(), R"(warning: '42 / 0' cannot be represented as 'i32')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Div_U32_ZeroDenominator) {
    auto* a = Scalar(u32(42));
    auto* b = Scalar(u32(0));
    auto result = const_eval.OpDivide(a->Type(), utils::Vector{a, b}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<u32>(), 42);
    EXPECT_EQ(error(), R"(warning: '42 / 0' cannot be represented as 'u32')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Div_AFloat_ZeroDenominator) {
    auto* a = Scalar(AFloat(42));
    auto* b = Scalar(AFloat(0));
    auto result = const_eval.OpDivide(a->Type(), utils::Vector{a, b}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<AFloat>(), 42.f);
    EXPECT_EQ(error(), R"(warning: '42.0 / 0.0' cannot be represented as 'abstract-float')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Div_F32_ZeroDenominator) {
    auto* a = Scalar(f32(42));
    auto* b = Scalar(f32(0));
    auto result = const_eval.OpDivide(a->Type(), utils::Vector{a, b}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 42.f);
    EXPECT_EQ(error(), R"(warning: '42.0 / 0.0' cannot be represented as 'f32')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Div_I32_MostNegativeByMinInt) {
    auto* a = Scalar(i32::Lowest());
    auto* b = Scalar(i32(-1));
    auto result = const_eval.OpDivide(a->Type(), utils::Vector{a, b}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<i32>(), i32::Lowest());
    EXPECT_EQ(error(), R"(warning: '-2147483648 / -1' cannot be represented as 'i32')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Mod_AInt_ZeroDenominator) {
    auto* a = Scalar(AInt(42));
    auto* b = Scalar(AInt(0));
    auto result = const_eval.OpModulo(a->Type(), utils::Vector{a, b}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<AInt>(), 0);
    EXPECT_EQ(error(), R"(warning: '42 % 0' cannot be represented as 'abstract-int')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Mod_I32_ZeroDenominator) {
    auto* a = Scalar(i32(42));
    auto* b = Scalar(i32(0));
    auto result = const_eval.OpModulo(a->Type(), utils::Vector{a, b}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<i32>(), 0);
    EXPECT_EQ(error(), R"(warning: '42 % 0' cannot be represented as 'i32')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Mod_U32_ZeroDenominator) {
    auto* a = Scalar(u32(42));
    auto* b = Scalar(u32(0));
    auto result = const_eval.OpModulo(a->Type(), utils::Vector{a, b}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<u32>(), 0);
    EXPECT_EQ(error(), R"(warning: '42 % 0' cannot be represented as 'u32')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Mod_AFloat_ZeroDenominator) {
    auto* a = Scalar(AFloat(42));
    auto* b = Scalar(AFloat(0));
    auto result = const_eval.OpModulo(a->Type(), utils::Vector{a, b}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<AFloat>(), 0.f);
    EXPECT_EQ(error(), R"(warning: '42.0 % 0.0' cannot be represented as 'abstract-float')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Mod_F32_ZeroDenominator) {
    auto* a = Scalar(f32(42));
    auto* b = Scalar(f32(0));
    auto result = const_eval.OpModulo(a->Type(), utils::Vector{a, b}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(), R"(warning: '42.0 % 0.0' cannot be represented as 'f32')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Mod_I32_MostNegativeByMinInt) {
    auto* a = Scalar(i32::Lowest());
    auto* b = Scalar(i32(-1));
    auto result = const_eval.OpModulo(a->Type(), utils::Vector{a, b}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<i32>(), 0);
    EXPECT_EQ(error(), R"(warning: '-2147483648 % -1' cannot be represented as 'i32')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, ShiftLeft_AInt_SignChange) {
    auto* a = Scalar(AInt(0x0FFFFFFFFFFFFFFFll));
    auto* b = Scalar(u32(9));
    auto result = const_eval.OpShiftLeft(a->Type(), utils::Vector{a, b}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<AInt>(), static_cast<AInt>(0x0FFFFFFFFFFFFFFFull << 9));
    EXPECT_EQ(error(), R"(warning: shift left operation results in sign change)");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, ShiftLeft_I32_SignChange) {
    auto* a = Scalar(i32(0x0FFFFFFF));
    auto* b = Scalar(u32(9));
    auto result = const_eval.OpShiftLeft(a->Type(), utils::Vector{a, b}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<i32>(), static_cast<i32>(0x0FFFFFFFu << 9));
    EXPECT_EQ(error(), R"(warning: shift left operation results in sign change)");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, ShiftLeft_I32_MoreThanBitWidth) {
    auto* a = Scalar(i32(0x1));
    auto* b = Scalar(u32(33));
    auto result = const_eval.OpShiftLeft(a->Type(), utils::Vector{a, b}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<i32>(), 2);
    EXPECT_EQ(
        error(),
        R"(warning: shift left value must be less than the bit width of the lhs, which is 32)");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, ShiftLeft_U32_MoreThanBitWidth) {
    auto* a = Scalar(u32(0x1));
    auto* b = Scalar(u32(33));
    auto result = const_eval.OpShiftLeft(a->Type(), utils::Vector{a, b}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<u32>(), 2);
    EXPECT_EQ(
        error(),
        R"(warning: shift left value must be less than the bit width of the lhs, which is 32)");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, ShiftRight_I32_MoreThanBitWidth) {
    auto* a = Scalar(i32(0x2));
    auto* b = Scalar(u32(33));
    auto result = const_eval.OpShiftRight(a->Type(), utils::Vector{a, b}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<i32>(), 1);
    EXPECT_EQ(
        error(),
        R"(warning: shift right value must be less than the bit width of the lhs, which is 32)");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, ShiftRight_U32_MoreThanBitWidth) {
    auto* a = Scalar(u32(0x2));
    auto* b = Scalar(u32(33));
    auto result = const_eval.OpShiftRight(a->Type(), utils::Vector{a, b}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<u32>(), 1);
    EXPECT_EQ(
        error(),
        R"(warning: shift right value must be less than the bit width of the lhs, which is 32)");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Acos_F32_OutOfRange) {
    auto* a = Scalar(f32(2));
    auto result = const_eval.acos(a->Type(), utils::Vector{a}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(),
              R"(warning: acos must be called with a value in the range [-1 .. 1] (inclusive))");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Acosh_F32_OutOfRange) {
    auto* a = Scalar(f32(-1));
    auto result = const_eval.acosh(a->Type(), utils::Vector{a}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(), R"(warning: acosh must be called with a value >= 1.0)");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Asin_F32_OutOfRange) {
    auto* a = Scalar(f32(2));
    auto result = const_eval.asin(a->Type(), utils::Vector{a}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(),
              R"(warning: asin must be called with a value in the range [-1 .. 1] (inclusive))");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Atanh_F32_OutOfRange) {
    auto* a = Scalar(f32(2));
    auto result = const_eval.atanh(a->Type(), utils::Vector{a}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(),
              R"(warning: atanh must be called with a value in the range (-1 .. 1) (exclusive))");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Exp_F32_Overflow) {
    auto* a = Scalar(f32(1000));
    auto result = const_eval.exp(a->Type(), utils::Vector{a}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(), R"(warning: e^1000.0 cannot be represented as 'f32')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Exp2_F32_Overflow) {
    auto* a = Scalar(f32(1000));
    auto result = const_eval.exp2(a->Type(), utils::Vector{a}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(), R"(warning: 2^1000.0 cannot be represented as 'f32')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, ExtractBits_I32_TooManyBits) {
    auto* a = Scalar(i32(0x12345678));
    auto* offset = Scalar(u32(24));
    auto* count = Scalar(u32(16));
    auto result = const_eval.extractBits(a->Type(), utils::Vector{a, offset, count}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<i32>(), 0x12);
    EXPECT_EQ(error(),
              R"(warning: 'offset + 'count' must be less than or equal to the bit width of 'e')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, ExtractBits_U32_TooManyBits) {
    auto* a = Scalar(u32(0x12345678));
    auto* offset = Scalar(u32(24));
    auto* count = Scalar(u32(16));
    auto result = const_eval.extractBits(a->Type(), utils::Vector{a, offset, count}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<u32>(), 0x12);
    EXPECT_EQ(error(),
              R"(warning: 'offset + 'count' must be less than or equal to the bit width of 'e')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, InsertBits_I32_TooManyBits) {
    auto* a = Scalar(i32(0x99345678));
    auto* b = Scalar(i32(0x12));
    auto* offset = Scalar(u32(24));
    auto* count = Scalar(u32(16));
    auto result = const_eval.insertBits(a->Type(), utils::Vector{a, b, offset, count}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<i32>(), 0x12345678);
    EXPECT_EQ(error(),
              R"(warning: 'offset + 'count' must be less than or equal to the bit width of 'e')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, InsertBits_U32_TooManyBits) {
    auto* a = Scalar(u32(0x99345678));
    auto* b = Scalar(u32(0x12));
    auto* offset = Scalar(u32(24));
    auto* count = Scalar(u32(16));
    auto result = const_eval.insertBits(a->Type(), utils::Vector{a, b, offset, count}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<u32>(), 0x12345678);
    EXPECT_EQ(error(),
              R"(warning: 'offset + 'count' must be less than or equal to the bit width of 'e')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, InverseSqrt_F32_OutOfRange) {
    auto* a = Scalar(f32(-1));
    auto result = const_eval.inverseSqrt(a->Type(), utils::Vector{a}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(), R"(warning: inverseSqrt must be called with a value > 0)");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, LDExpr_F32_OutOfRange) {
    auto* a = Scalar(f32(42.f));
    auto* b = Scalar(f32(200));
    auto result = const_eval.ldexp(a->Type(), utils::Vector{a, b}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(), R"(warning: e2 must be less than or equal to 128)");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Log_F32_OutOfRange) {
    auto* a = Scalar(f32(-1));
    auto result = const_eval.log(a->Type(), utils::Vector{a}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(), R"(warning: log must be called with a value > 0)");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Log2_F32_OutOfRange) {
    auto* a = Scalar(f32(-1));
    auto result = const_eval.log2(a->Type(), utils::Vector{a}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(), R"(warning: log2 must be called with a value > 0)");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Normalize_ZeroLength) {
    auto* zero = Scalar(f32(0));
    auto* vec =
        const_eval.VecSplat(create<type::Vector>(create<type::F32>(), 4u), utils::Vector{zero}, {})
            .Get();
    auto result = const_eval.normalize(vec->Type(), utils::Vector{vec}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->Index(0)->ValueAs<f32>(), 0.f);
    EXPECT_EQ(result.Get()->Index(1)->ValueAs<f32>(), 0.f);
    EXPECT_EQ(result.Get()->Index(2)->ValueAs<f32>(), 0.f);
    EXPECT_EQ(result.Get()->Index(3)->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(), R"(warning: zero length vector can not be normalized)");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Pack2x16Float_OutOfRange) {
    auto* a = Scalar(f32(75250.f));
    auto* b = Scalar(f32(42.1f));
    auto* vec =
        const_eval.VecInitS(create<type::Vector>(create<type::F32>(), 2u), utils::Vector{a, b}, {})
            .Get();
    auto result = const_eval.pack2x16float(create<type::U32>(), utils::Vector{vec}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<u32>(), 0x51430000);
    EXPECT_EQ(error(), R"(warning: value 75250.0 cannot be represented as 'f16')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Pow_F32_Overflow) {
    auto* a = Scalar(f32(2));
    auto* b = Scalar(f32(1000));
    auto result = const_eval.pow(a->Type(), utils::Vector{a, b}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(), R"(warning: '2.0 ^ 1000.0' cannot be represented as 'f32')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Unpack2x16Float_OutOfRange) {
    auto* a = Scalar(u32(0x51437C00));
    auto result = const_eval.unpack2x16float(create<type::U32>(), utils::Vector{a}, {});
    ASSERT_TRUE(result);
    EXPECT_FLOAT_EQ(result.Get()->Index(0)->ValueAs<f32>(), 0.f);
    EXPECT_FLOAT_EQ(result.Get()->Index(1)->ValueAs<f32>(), 42.09375f);
    EXPECT_EQ(error(), R"(warning: value inf cannot be represented as 'f32')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, QuantizeToF16_OutOfRange) {
    auto* a = Scalar(f32(75250.f));
    auto result = const_eval.quantizeToF16(create<type::U32>(), utils::Vector{a}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<u32>(), 0);
    EXPECT_EQ(error(), R"(warning: value 75250.0 cannot be represented as 'f16')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Sqrt_F32_OutOfRange) {
    auto* a = Scalar(f32(-1));
    auto result = const_eval.sqrt(a->Type(), utils::Vector{a}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(), R"(warning: sqrt must be called with a value >= 0)");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Bitcast_Infinity) {
    auto* a = Scalar(u32(0x7F800000));
    auto result = const_eval.Bitcast(create<type::F32>(), a, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(), R"(warning: value inf cannot be represented as 'f32')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Bitcast_NaN) {
    auto* a = Scalar(u32(0x7FC00000));
    auto result = const_eval.Bitcast(create<type::F32>(), a, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(), R"(warning: value nan cannot be represented as 'f32')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Convert_F32_TooHigh) {
    auto* a = Scalar(AFloat::Highest());
    auto result = const_eval.Convert(create<type::F32>(), a, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), f32::kHighestValue);
    EXPECT_EQ(
        error(),
        R"(warning: value 179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.0 cannot be represented as 'f32')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Convert_F32_TooLow) {
    auto* a = Scalar(AFloat::Lowest());
    auto result = const_eval.Convert(create<type::F32>(), a, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), f32::kLowestValue);
    EXPECT_EQ(
        error(),
        R"(warning: value -179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.0 cannot be represented as 'f32')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Convert_F16_TooHigh) {
    auto* a = Scalar(f32(1000000.0));
    auto result = const_eval.Convert(create<type::F16>(), a, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), f16::kHighestValue);
    EXPECT_EQ(error(), R"(warning: value 1000000.0 cannot be represented as 'f16')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Convert_F16_TooLow) {
    auto* a = Scalar(f32(-1000000.0));
    auto result = const_eval.Convert(create<type::F16>(), a, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), f16::kLowestValue);
    EXPECT_EQ(error(), R"(warning: value -1000000.0 cannot be represented as 'f16')");
}

TEST_F(ResolverConstEvalRuntimeSemanticsTest, Vec_Overflow_SingleComponent) {
    // Test that overflow for an element-wise vector operation only affects a single component.
    auto* vec4f = create<type::Vector>(create<type::F32>(), 4u);
    auto* a = const_eval
                  .VecInitS(vec4f,
                            utils::Vector{
                                Scalar(f32(1)),
                                Scalar(f32(4)),
                                Scalar(f32(-1)),
                                Scalar(f32(65536)),
                            },
                            {})
                  .Get();
    auto result = const_eval.sqrt(a->Type(), utils::Vector{a}, {});
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Get()->Index(0)->ValueAs<f32>(), 1);
    EXPECT_EQ(result.Get()->Index(1)->ValueAs<f32>(), 2);
    EXPECT_EQ(result.Get()->Index(2)->ValueAs<f32>(), 0);
    EXPECT_EQ(result.Get()->Index(3)->ValueAs<f32>(), 256);
    EXPECT_EQ(error(), R"(warning: sqrt must be called with a value >= 0)");
}

}  // namespace
}  // namespace tint::resolver
