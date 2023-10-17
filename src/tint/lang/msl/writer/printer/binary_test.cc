
// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/msl/writer/printer/helper_test.h"
#include "src/tint/utils/text/string_stream.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::msl::writer {
namespace {

struct BinaryData {
    const char* result;
    core::ir::BinaryOp op;
};
inline std::ostream& operator<<(std::ostream& out, BinaryData data) {
    StringStream str;
    str << data.op;
    out << str.str();
    return out;
}

using MslPrinterBinaryTest = MslPrinterTestWithParam<BinaryData>;
TEST_P(MslPrinterBinaryTest, Emit) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* l = b.Let("left", b.Constant(1_u));
        auto* r = b.Let("right", b.Constant(2_u));
        auto* bin = b.Binary(params.op, ty.u32(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_;
    EXPECT_EQ(output_, MetalHeader() + R"(
void foo() {
  uint const left = 1u;
  uint const right = 2u;
  uint const val = )" + params.result +
                           R"(;
}
)");
}
INSTANTIATE_TEST_SUITE_P(
    MslPrinterTest,
    MslPrinterBinaryTest,
    testing::Values(BinaryData{"(left + right)", core::ir::BinaryOp::kAdd},
                    BinaryData{"(left - right)", core::ir::BinaryOp::kSubtract},
                    BinaryData{"(left * right)", core::ir::BinaryOp::kMultiply},
                    BinaryData{"(left / right)", core::ir::BinaryOp::kDivide},
                    BinaryData{"(left % right)", core::ir::BinaryOp::kModulo},
                    BinaryData{"(left & right)", core::ir::BinaryOp::kAnd},
                    BinaryData{"(left | right)", core::ir::BinaryOp::kOr},
                    BinaryData{"(left ^ right)", core::ir::BinaryOp::kXor},
                    BinaryData{"(left << right)", core::ir::BinaryOp::kShiftLeft},
                    BinaryData{"(left >> right)", core::ir::BinaryOp::kShiftRight}));

using MslPrinterBinaryBoolTest = MslPrinterTestWithParam<BinaryData>;
TEST_P(MslPrinterBinaryBoolTest, Emit) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* l = b.Let("left", b.Constant(1_u));
        auto* r = b.Let("right", b.Constant(2_u));
        auto* bin = b.Binary(params.op, ty.bool_(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_;
    EXPECT_EQ(output_, MetalHeader() + R"(
void foo() {
  uint const left = 1u;
  uint const right = 2u;
  bool const val = )" + params.result +
                           R"(;
}
)");
}
INSTANTIATE_TEST_SUITE_P(
    MslPrinterTest,
    MslPrinterBinaryBoolTest,
    testing::Values(BinaryData{"(left == right)", core::ir::BinaryOp::kEqual},
                    BinaryData{"(left != right)", core::ir::BinaryOp::kNotEqual},
                    BinaryData{"(left < right)", core::ir::BinaryOp::kLessThan},
                    BinaryData{"(left > right)", core::ir::BinaryOp::kGreaterThan},
                    BinaryData{"(left <= right)", core::ir::BinaryOp::kLessThanEqual},
                    BinaryData{"(left >= right)", core::ir::BinaryOp::kGreaterThanEqual}));

// TODO(dsinclair): Needs transform
// TODO(dsinclair): Requires `bitcast` support
using MslPrinterBinaryTest_SignedOverflowDefinedBehaviour = MslPrinterTestWithParam<BinaryData>;
TEST_P(MslPrinterBinaryTest_SignedOverflowDefinedBehaviour, DISABLED_Emit) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* l = b.Let("a", b.Constant(1_i));
        auto* r = b.Let("b", b.Constant(3_i));

        auto* bin = b.Binary(params.op, ty.i32(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_;
    EXPECT_EQ(output_, MetalHeader() + R"(
void foo() {
  int const left = 1i;
  int const right = 3i;
  int const val = )" + params.result +
                           R"(;
      }
)");
}

constexpr BinaryData signed_overflow_defined_behaviour_cases[] = {
    {"as_type<int>((as_type<uint>(left) + as_type<uint>(right)))", core::ir::BinaryOp::kAdd},
    {"as_type<int>((as_type<uint>(left) - as_type<uint>(right)))", core::ir::BinaryOp::kSubtract},
    {"as_type<int>((as_type<uint>(left) * as_type<uint>(right)))", core::ir::BinaryOp::kMultiply}};
INSTANTIATE_TEST_SUITE_P(MslPrinterTest,
                         MslPrinterBinaryTest_SignedOverflowDefinedBehaviour,
                         testing::ValuesIn(signed_overflow_defined_behaviour_cases));

// TODO(dsinclair): Needs transform
// TODO(dsinclair): Requires `bitcast` support
using MslPrinterBinaryTest_ShiftSignedOverflowDefinedBehaviour =
    MslPrinterTestWithParam<BinaryData>;
TEST_P(MslPrinterBinaryTest_ShiftSignedOverflowDefinedBehaviour, DISABLED_Emit) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* l = b.Let("a", b.Constant(1_i));
        auto* r = b.Let("b", b.Constant(2_u));
        auto* bin = b.Binary(params.op, ty.i32(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_;
    EXPECT_EQ(output_, MetalHeader() + R"(
void foo() {
  int const left = 1i;
  uint const right = 2u;
  int const val = )" + params.result +
                           R"(;
      }
)");
}

constexpr BinaryData shift_signed_overflow_defined_behaviour_cases[] = {
    {"as_type<int>((as_type<uint>(left) << right))", core::ir::BinaryOp::kShiftLeft},
    {"(left >> right)", core::ir::BinaryOp::kShiftRight}};
INSTANTIATE_TEST_SUITE_P(MslPrinterTest,
                         MslPrinterBinaryTest_ShiftSignedOverflowDefinedBehaviour,
                         testing::ValuesIn(shift_signed_overflow_defined_behaviour_cases));

// TODO(dsinclair): Needs transform
// TODO(dsinclair): Requires `bitcast`
using MslPrinterBinaryTest_SignedOverflowDefinedBehaviour_Chained =
    MslPrinterTestWithParam<BinaryData>;
TEST_P(MslPrinterBinaryTest_SignedOverflowDefinedBehaviour_Chained, DISABLED_Emit) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* left = b.Var("left", ty.ptr<core::AddressSpace::kFunction, i32>());
        auto* right = b.Var("right", ty.ptr<core::AddressSpace::kFunction, i32>());

        auto* expr1 = b.Binary(params.op, ty.i32(), left, right);
        auto* expr2 = b.Binary(params.op, ty.i32(), expr1, right);

        b.Let("val", expr2);
    });

    ASSERT_TRUE(Generate()) << err_ << output_;
    EXPECT_EQ(output_, MetalHeader() + R"(
void foo() {
  int left;
  int right;
  int const val = )" + params.result +
                           R"(;
)");
}
constexpr BinaryData signed_overflow_defined_behaviour_chained_cases[] = {
    {R"(as_type<int>((as_type<uint>(as_type<int>((as_type<uint>(left) + as_type<uint>(right)))) +
    as_type<uint>(right))))",
     core::ir::BinaryOp::kAdd},
    {R"(as_type<int>((as_type<uint>(as_type<int>((as_type<uint>(left) - as_type<uint>(right)))) -
    as_type<uint>(right))))",
     core::ir::BinaryOp::kSubtract},
    {R"(as_type<int>((as_type<uint>(as_type<int>((as_type<uint>(left) * as_type<uint>(right)))) *
    as_type<uint>(right))))",
     core::ir::BinaryOp::kMultiply}};
INSTANTIATE_TEST_SUITE_P(MslPrinterTest,
                         MslPrinterBinaryTest_SignedOverflowDefinedBehaviour_Chained,
                         testing::ValuesIn(signed_overflow_defined_behaviour_chained_cases));

// TODO(dsinclair): Needs transform
// TODO(dsinclair): Requires `bitcast`
using MslPrinterBinaryTest_ShiftSignedOverflowDefinedBehaviour_Chained =
    MslPrinterTestWithParam<BinaryData>;
TEST_P(MslPrinterBinaryTest_ShiftSignedOverflowDefinedBehaviour_Chained, DISABLED_Emit) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* left = b.Var("left", ty.ptr<core::AddressSpace::kFunction, i32>());
        auto* right = b.Var("right", ty.ptr<core::AddressSpace::kFunction, u32>());

        auto* expr1 = b.Binary(params.op, ty.i32(), left, right);
        auto* expr2 = b.Binary(params.op, ty.i32(), expr1, right);

        b.Let("val", expr2);
    });

    ASSERT_TRUE(Generate()) << err_ << output_;
    EXPECT_EQ(output_, MetalHeader() + R"(
void foo() {
  int left;
  uint right;
  int const val = )" + params.result +
                           R"(;
)");
}
constexpr BinaryData shift_signed_overflow_defined_behaviour_chained_cases[] = {
    {R"(as_type<int>((as_type<uint>(as_type<int>((as_type<uint>(left) << right))) << right)))",
     core::ir::BinaryOp::kShiftLeft},
    {R"(((left >> right) >> right))", core::ir::BinaryOp::kShiftRight},
};
INSTANTIATE_TEST_SUITE_P(MslPrinterTest,
                         MslPrinterBinaryTest_ShiftSignedOverflowDefinedBehaviour_Chained,
                         testing::ValuesIn(shift_signed_overflow_defined_behaviour_chained_cases));

// TODO(dsinclair): Needs transform
TEST_F(MslPrinterTest, DISABLED_BinaryModF32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* left = b.Var("left", ty.ptr<core::AddressSpace::kFunction, f32>());
        auto* right = b.Var("right", ty.ptr<core::AddressSpace::kFunction, f32>());

        auto* expr1 = b.Binary(core::ir::BinaryOp::kModulo, ty.f32(), left, right);

        b.Let("val", expr1);
    });

    ASSERT_TRUE(Generate()) << err_ << output_;
    EXPECT_EQ(output_, MetalHeader() + R"(
void foo() {
  float left;
  float right;
  float const val = fmod(left, right);
)");
}

// TODO(dsinclair): Needs transform
TEST_F(MslPrinterTest, DISABLED_BinaryModF16) {
    // Enable f16?

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* left = b.Var("left", ty.ptr<core::AddressSpace::kFunction, f16>());
        auto* right = b.Var("right", ty.ptr<core::AddressSpace::kFunction, f16>());

        auto* expr1 = b.Binary(core::ir::BinaryOp::kModulo, ty.f16(), left, right);

        b.Let("val", expr1);
    });

    ASSERT_TRUE(Generate()) << err_ << output_;
    EXPECT_EQ(output_, MetalHeader() + R"(
void foo() {
  half left;
  half right;
  half const val = fmod(left, right);
)");
}

// TODO(dsinclair): Needs transform
TEST_F(MslPrinterTest, DISABLED_BinaryModVec3F32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* left = b.Var("left", ty.ptr(core::AddressSpace::kFunction, ty.vec3<f32>()));
        auto* right = b.Var("right", ty.ptr(core::AddressSpace::kFunction, ty.vec3<f32>()));

        auto* expr1 = b.Binary(core::ir::BinaryOp::kModulo, ty.vec3<f32>(), left, right);

        b.Let("val", expr1);
    });

    ASSERT_TRUE(Generate()) << err_ << output_;
    EXPECT_EQ(output_, MetalHeader() + R"(
void foo() {
  float3 left;
  float3 right;
  float3 const val = fmod(left, right);
)");
}

// TODO(dsinclair): Needs transform
TEST_F(MslPrinterTest, DISABLED_BinaryModVec3F16) {
    // Enable f16?

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* left = b.Var("left", ty.ptr(core::AddressSpace::kFunction, ty.vec3<f16>()));
        auto* right = b.Var("right", ty.ptr(core::AddressSpace::kFunction, ty.vec3<f16>()));

        auto* expr1 = b.Binary(core::ir::BinaryOp::kModulo, ty.vec3<f16>(), left, right);

        b.Let("val", expr1);
    });

    ASSERT_TRUE(Generate()) << err_ << output_;
    EXPECT_EQ(output_, MetalHeader() + R"(
void foo() {
  half3 left;
  half3 right;
  half3 const val = fmod(left, right);
)");
}

// TODO(dsinclair): Needs transform
TEST_F(MslPrinterTest, DISABLED_BinaryBoolAnd) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* left = b.Var("left", ty.ptr(core::AddressSpace::kFunction, ty.bool_()));
        auto* right = b.Var("right", ty.ptr(core::AddressSpace::kFunction, ty.bool_()));

        auto* expr1 = b.Binary(core::ir::BinaryOp::kAdd, ty.bool_(), left, right);

        b.Let("val", expr1);
    });

    ASSERT_TRUE(Generate()) << err_ << output_;
    EXPECT_EQ(output_, MetalHeader() + R"(
void foo() {
  float left;
  float right;
  float const val = bool(left & right);
)");
}

// TODO(dsinclair): Needs transform
TEST_F(MslPrinterTest, DISABLED_BinaryBoolOr) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* left = b.Var("left", ty.ptr(core::AddressSpace::kFunction, ty.bool_()));
        auto* right = b.Var("right", ty.ptr(core::AddressSpace::kFunction, ty.bool_()));

        auto* expr1 = b.Binary(core::ir::BinaryOp::kOr, ty.bool_(), left, right);

        b.Let("val", expr1);
    });

    ASSERT_TRUE(Generate()) << err_ << output_;
    EXPECT_EQ(output_, MetalHeader() + R"(
void foo() {
  float left;
  float right;
  float const val = bool(left | right);
)");
}

}  // namespace
}  // namespace tint::msl::writer
