
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

#include "src/tint/lang/msl/writer/printer/helper_test.h"
#include "src/tint/utils/text/string_stream.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::msl::writer {
namespace {

struct BinaryData {
    const char* result;
    enum ir::Binary::Kind op;
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

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
void foo() {
  uint const left = 1u;
  uint const right = 2u;
  uint const val = )" + params.result + R"(;
}
)");
}
INSTANTIATE_TEST_SUITE_P(
    MslPrinterTest,
    MslPrinterBinaryTest,
    testing::Values(BinaryData{"(left + right)", ir::Binary::Kind::kAdd},
                    BinaryData{"(left - right)", ir::Binary::Kind::kSubtract},
                    BinaryData{"(left * right)", ir::Binary::Kind::kMultiply},
                    BinaryData{"(left / right)", ir::Binary::Kind::kDivide},
                    BinaryData{"(left % right)", ir::Binary::Kind::kModulo},
                    BinaryData{"(left & right)", ir::Binary::Kind::kAnd},
                    BinaryData{"(left | right)", ir::Binary::Kind::kOr},
                    BinaryData{"(left ^ right)", ir::Binary::Kind::kXor},
                    BinaryData{"(left << right)", ir::Binary::Kind::kShiftLeft},
                    BinaryData{"(left >> right)", ir::Binary::Kind::kShiftRight}));

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

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
void foo() {
  uint const left = 1u;
  uint const right = 2u;
  bool const val = )" + params.result + R"(;
}
)");
}
INSTANTIATE_TEST_SUITE_P(
    MslPrinterTest,
    MslPrinterBinaryBoolTest,
    testing::Values(BinaryData{"(left == right)", ir::Binary::Kind::kEqual},
                    BinaryData{"(left != right)", ir::Binary::Kind::kNotEqual},
                    BinaryData{"(left < right)", ir::Binary::Kind::kLessThan},
                    BinaryData{"(left > right)", ir::Binary::Kind::kGreaterThan},
                    BinaryData{"(left <= right)", ir::Binary::Kind::kLessThanEqual},
                    BinaryData{"(left >= right)", ir::Binary::Kind::kGreaterThanEqual}));

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

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
void foo() {
  int const left = 1i;
  int const right = 3i;
  int const val = )" + params.result + R"(;
      }
)");
}

constexpr BinaryData signed_overflow_defined_behaviour_cases[] = {
    {"as_type<int>((as_type<uint>(left) + as_type<uint>(right)))", ir::Binary::Kind::kAdd},
    {"as_type<int>((as_type<uint>(left) - as_type<uint>(right)))", ir::Binary::Kind::kSubtract},
    {"as_type<int>((as_type<uint>(left) * as_type<uint>(right)))", ir::Binary::Kind::kMultiply}};
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

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
void foo() {
  int const left = 1i;
  uint const right = 2u;
  int const val = )" + params.result + R"(;
      }
)");
}

constexpr BinaryData shift_signed_overflow_defined_behaviour_cases[] = {
    {"as_type<int>((as_type<uint>(left) << right))", ir::Binary::Kind::kShiftLeft},
    {"(left >> right)", ir::Binary::Kind::kShiftRight}};
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
        auto* left = b.Var("left", ty.ptr<builtin::AddressSpace::kFunction, i32>());
        auto* right = b.Var("right", ty.ptr<builtin::AddressSpace::kFunction, i32>());

        auto* expr1 = b.Binary(params.op, ty.i32(), left, right);
        auto* expr2 = b.Binary(params.op, ty.i32(), expr1, right);

        b.Let("val", expr2);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
void foo() {
  int left;
  int right;
  int const val = )" + params.result + R"(;
)");
}
constexpr BinaryData signed_overflow_defined_behaviour_chained_cases[] = {
    {R"(as_type<int>((as_type<uint>(as_type<int>((as_type<uint>(left) + as_type<uint>(right)))) +
    as_type<uint>(right))))",
     ir::Binary::Kind::kAdd},
    {R"(as_type<int>((as_type<uint>(as_type<int>((as_type<uint>(left) - as_type<uint>(right)))) -
    as_type<uint>(right))))",
     ir::Binary::Kind::kSubtract},
    {R"(as_type<int>((as_type<uint>(as_type<int>((as_type<uint>(left) * as_type<uint>(right)))) *
    as_type<uint>(right))))",
     ir::Binary::Kind::kMultiply}};
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
        auto* left = b.Var("left", ty.ptr<builtin::AddressSpace::kFunction, i32>());
        auto* right = b.Var("right", ty.ptr<builtin::AddressSpace::kFunction, u32>());

        auto* expr1 = b.Binary(params.op, ty.i32(), left, right);
        auto* expr2 = b.Binary(params.op, ty.i32(), expr1, right);

        b.Let("val", expr2);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
void foo() {
  int left;
  uint right;
  int const val = )" + params.result + R"(;
)");
}
constexpr BinaryData shift_signed_overflow_defined_behaviour_chained_cases[] = {
    {R"(as_type<int>((as_type<uint>(as_type<int>((as_type<uint>(left) << right))) << right)))",
     ir::Binary::Kind::kShiftLeft},
    {R"(((left >> right) >> right))", ir::Binary::Kind::kShiftRight},
};
INSTANTIATE_TEST_SUITE_P(MslPrinterTest,
                         MslPrinterBinaryTest_ShiftSignedOverflowDefinedBehaviour_Chained,
                         testing::ValuesIn(shift_signed_overflow_defined_behaviour_chained_cases));

// TODO(dsinclair): Needs transform
TEST_F(MslPrinterTest, DISABLED_BinaryModF32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* left = b.Var("left", ty.ptr<builtin::AddressSpace::kFunction, f32>());
        auto* right = b.Var("right", ty.ptr<builtin::AddressSpace::kFunction, f32>());

        auto* expr1 = b.Binary(ir::Binary::Kind::kModulo, ty.f32(), left, right);

        b.Let("val", expr1);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
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
        auto* left = b.Var("left", ty.ptr<builtin::AddressSpace::kFunction, f16>());
        auto* right = b.Var("right", ty.ptr<builtin::AddressSpace::kFunction, f16>());

        auto* expr1 = b.Binary(ir::Binary::Kind::kModulo, ty.f16(), left, right);

        b.Let("val", expr1);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
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
        auto* left = b.Var("left", ty.ptr(builtin::AddressSpace::kFunction, ty.vec3<f32>()));
        auto* right = b.Var("right", ty.ptr(builtin::AddressSpace::kFunction, ty.vec3<f32>()));

        auto* expr1 = b.Binary(ir::Binary::Kind::kModulo, ty.vec3<f32>(), left, right);

        b.Let("val", expr1);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
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
        auto* left = b.Var("left", ty.ptr(builtin::AddressSpace::kFunction, ty.vec3<f16>()));
        auto* right = b.Var("right", ty.ptr(builtin::AddressSpace::kFunction, ty.vec3<f16>()));

        auto* expr1 = b.Binary(ir::Binary::Kind::kModulo, ty.vec3<f16>(), left, right);

        b.Let("val", expr1);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
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
        auto* left = b.Var("left", ty.ptr(builtin::AddressSpace::kFunction, ty.bool_()));
        auto* right = b.Var("right", ty.ptr(builtin::AddressSpace::kFunction, ty.bool_()));

        auto* expr1 = b.Binary(ir::Binary::Kind::kAdd, ty.bool_(), left, right);

        b.Let("val", expr1);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
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
        auto* left = b.Var("left", ty.ptr(builtin::AddressSpace::kFunction, ty.bool_()));
        auto* right = b.Var("right", ty.ptr(builtin::AddressSpace::kFunction, ty.bool_()));

        auto* expr1 = b.Binary(ir::Binary::Kind::kOr, ty.bool_(), left, right);

        b.Let("val", expr1);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
void foo() {
  float left;
  float right;
  float const val = bool(left | right);
)");
}

}  // namespace
}  // namespace tint::msl::writer
