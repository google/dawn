
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
    bool bool_result;
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

    const type::Type* result_type = ty.u32();
    if (params.bool_result) {
        result_type = ty.bool_();
    }

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&]() {
        auto* l = b.Let("left", b.Constant(1_u));
        auto* r = b.Let("right", b.Constant(2_u));
        auto* bin = b.Binary(params.op, result_type, l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    std::string result_type_name = (params.bool_result ? "bool" : "uint");

    std::string result = MetalHeader() + R"(
void foo() {
  uint const left = 1u;
  uint const right = 2u;
  )";
    result += result_type_name + " const val = " + params.result;
    result += R"(;
}
)";

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), result);
}
INSTANTIATE_TEST_SUITE_P(
    MslPrinterTest,
    MslPrinterBinaryTest,
    testing::Values(BinaryData{"(left + right)", ir::Binary::Kind::kAdd, false},
                    BinaryData{"(left - right)", ir::Binary::Kind::kSubtract, false},
                    BinaryData{"(left * right)", ir::Binary::Kind::kMultiply, false},
                    BinaryData{"(left / right)", ir::Binary::Kind::kDivide, false},
                    BinaryData{"(left % right)", ir::Binary::Kind::kModulo, false},
                    BinaryData{"(left & right)", ir::Binary::Kind::kAnd, false},
                    BinaryData{"(left | right)", ir::Binary::Kind::kOr, false},
                    BinaryData{"(left ^ right)", ir::Binary::Kind::kXor, false},
                    BinaryData{"(left == right)", ir::Binary::Kind::kEqual, true},
                    BinaryData{"(left != right)", ir::Binary::Kind::kNotEqual, true},
                    BinaryData{"(left < right)", ir::Binary::Kind::kLessThan, true},
                    BinaryData{"(left > right)", ir::Binary::Kind::kGreaterThan, true},
                    BinaryData{"(left <= right)", ir::Binary::Kind::kLessThanEqual, true},
                    BinaryData{"(left >= right)", ir::Binary::Kind::kGreaterThanEqual, true},
                    BinaryData{"(left << right)", ir::Binary::Kind::kShiftLeft, false},
                    BinaryData{"(left >> right)", ir::Binary::Kind::kShiftRight, false}));

// using MslPrinterBinaryTest_SignedOverflowDefinedBehaviour = MslPrinterTestWithParam<BinaryData>;
// TEST_P(MslPrinterBinaryTest_SignedOverflowDefinedBehaviour, Emit) {
//     auto params = GetParam();
//
//     auto* func = b.Function("foo", ty.void_());
//     b.Append(func->Block(), [&]() {
//         auto* l = b.Let("a", b.Constant(1_i));
//         auto* r = b.Let("b", (params.op == ir::Binary::Kind::kShiftLeft ||
//                               params.op == ir::Binary::Kind::kShiftRight)
//                                  ? b.Constant(2_u)
//                                  : b.Constant(3_i));
//
//         auto* bin = b.Binary(params.op, ty.i32(), l, r);
//         b.Let("val", bin);
//         b.Return(func);
//     });
//
//     ASSERT_TRUE(IRIsValid()) << Error();
//     ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
//     EXPECT_THAT(generator_.Result(), testing::HasSubstr(params.result));
// }
//
// constexpr BinaryData signed_overflow_defined_behaviour_cases[] = {
//     {"as_type<int>((as_type<uint>(a) << b))", ir::Binary::Kind::kShiftLeft},
//     {"(a >> b)", ir::Binary::Kind::kShiftRight},
//     {"as_type<int>((as_type<uint>(a) + as_type<uint>(b)))", ir::Binary::Kind::kAdd},
//     {"as_type<int>((as_type<uint>(a) - as_type<uint>(b)))", ir::Binary::Kind::kSubtract},
//     {"as_type<int>((as_type<uint>(a) * as_type<uint>(b)))", ir::Binary::Kind::kMultiply}};
// INSTANTIATE_TEST_SUITE_P(MslPrinterTest,
//                          MslPrinterBinaryTest_SignedOverflowDefinedBehaviour,
//                          testing::ValuesIn(signed_overflow_defined_behaviour_cases));
//
//  using MslBinaryTest_SignedOverflowDefinedBehaviour_Chained = TestParamHelper<BinaryData>;
//  TEST_P(MslBinaryTest_SignedOverflowDefinedBehaviour_Chained, Emit) {
//      auto params = GetParam();
//
//      auto a_type = ty.i32();
//      auto b_type =
//          (params.op == ast::BinaryOp::kShiftLeft || params.op == ast::BinaryOp::kShiftRight)
//              ? ty.u32()
//              : ty.i32();
//
//      auto* a = Var("a", a_type);
//      auto* b = Var("b", b_type);
//
//      auto* expr1 = create<ast::BinaryExpression>(params.op, Expr(a), Expr(b));
//      auto* expr2 = create<ast::BinaryExpression>(params.op, expr1, Expr(b));
//      WrapInFunction(a, b, expr2);
//
//      Printer& gen = Build();
//
//      StringStream out;
//      ASSERT_TRUE(gen.EmitExpression(out, expr2)) << gen.Diagnostics();
//      EXPECT_EQ(out.str(), params.result);
//  }
//  using Op = ast::BinaryOp;
//  constexpr BinaryData signed_overflow_defined_behaviour_chained_cases[] = {
//      {R"(as_type<int>((as_type<uint>(as_type<int>((as_type<uint>(a) << b))) << b)))",
//       Op::kShiftLeft},
//      {R"(((a >> b) >> b))", Op::kShiftRight},
//      {R"(as_type<int>((as_type<uint>(as_type<int>((as_type<uint>(a) + as_type<uint>(b)))) +
//      as_type<uint>(b))))",
//       Op::kAdd},
//      {R"(as_type<int>((as_type<uint>(as_type<int>((as_type<uint>(a) - as_type<uint>(b)))) -
//      as_type<uint>(b))))",
//       Op::kSubtract},
//      {R"(as_type<int>((as_type<uint>(as_type<int>((as_type<uint>(a) * as_type<uint>(b)))) *
//      as_type<uint>(b))))",
//       Op::kMultiply}};
//  INSTANTIATE_TEST_SUITE_P(MslPrinterTest,
//                           MslBinaryTest_SignedOverflowDefinedBehaviour_Chained,
//                           testing::ValuesIn(signed_overflow_defined_behaviour_chained_cases));
//
//  TEST_F(MslBinaryTest, ModF32) {
//      auto* left = Var("left", ty.f32());
//      auto* right = Var("right", ty.f32());
//      auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kModulo, Expr(left), Expr(right));
//      WrapInFunction(left, right, expr);
//
//      Printer& gen = Build();
//
//      StringStream out;
//      ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
//      EXPECT_EQ(out.str(), "fmod(left, right)");
//  }
//
//  TEST_F(MslBinaryTest, ModF16) {
//      Enable(builtin::Extension::kF16);
//
//      auto* left = Var("left", ty.f16());
//      auto* right = Var("right", ty.f16());
//      auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kModulo, Expr(left), Expr(right));
//      WrapInFunction(left, right, expr);
//
//      Printer& gen = Build();
//
//      StringStream out;
//      ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
//      EXPECT_EQ(out.str(), "fmod(left, right)");
//  }
//
//  TEST_F(MslBinaryTest, ModVec3F32) {
//      auto* left = Var("left", ty.vec3<f32>());
//      auto* right = Var("right", ty.vec3<f32>());
//      auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kModulo, Expr(left), Expr(right));
//      WrapInFunction(left, right, expr);
//
//      Printer& gen = Build();
//
//      StringStream out;
//      ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
//      EXPECT_EQ(out.str(), "fmod(left, right)");
//  }
//
//  TEST_F(MslBinaryTest, ModVec3F16) {
//      Enable(builtin::Extension::kF16);
//
//      auto* left = Var("left", ty.vec3<f16>());
//      auto* right = Var("right", ty.vec3<f16>());
//      auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kModulo, Expr(left), Expr(right));
//      WrapInFunction(left, right, expr);
//
//      Printer& gen = Build();
//
//      StringStream out;
//      ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
//      EXPECT_EQ(out.str(), "fmod(left, right)");
//  }
//
//  TEST_F(MslBinaryTest, BoolAnd) {
//      auto* left = Var("left", Expr(true));
//      auto* right = Var("right", Expr(false));
//      auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kAnd, Expr(left), Expr(right));
//      WrapInFunction(left, right, expr);
//
//      Printer& gen = Build();
//
//      StringStream out;
//      ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
//      EXPECT_EQ(out.str(), "bool(left & right)");
//  }
//
//  TEST_F(MslBinaryTest, BoolOr) {
//      auto* left = Var("left", Expr(true));
//      auto* right = Var("right", Expr(false));
//      auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kOr, Expr(left), Expr(right));
//      WrapInFunction(left, right, expr);
//
//      Printer& gen = Build();
//
//      StringStream out;
//      ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
//      EXPECT_EQ(out.str(), "bool(left | right)");
//  }

}  // namespace
}  // namespace tint::msl::writer
