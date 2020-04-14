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

#include <memory>

#include "gtest/gtest.h"
#include "src/ast/binary_expression.h"
#include "src/ast/float_literal.h"
#include "src/ast/int_literal.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = testing::Test;

struct BinaryData {
  ast::BinaryOp op;
  std::string name;
};
inline std::ostream& operator<<(std::ostream& out, BinaryData data) {
  out << data.op;
  return out;
}

using BinaryArithIntegerTest = testing::TestWithParam<BinaryData>;
TEST_P(BinaryArithIntegerTest, Scalar) {
  auto param = GetParam();

  ast::type::I32Type i32;

  auto lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 3));
  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 4));

  ast::BinaryExpression expr(param.op, std::move(lhs), std::move(rhs));

  Context ctx;
  TypeDeterminer td(&ctx);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b;
  b.push_function(Function{});

  ASSERT_EQ(b.GenerateBinaryExpression(&expr), 4) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeInt 32 1
%2 = OpConstant %1 3
%3 = OpConstant %1 4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            "%4 = " + param.name + " %1 %2 %3\n");
}

TEST_P(BinaryArithIntegerTest, Vector) {
  auto param = GetParam();

  ast::type::I32Type i32;
  ast::type::VectorType vec3(&i32, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 1)));
  auto lhs =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 1)));
  auto rhs =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  Context ctx;
  TypeDeterminer td(&ctx);

  ast::BinaryExpression expr(param.op, std::move(lhs), std::move(rhs));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b;
  b.push_function(Function{});

  ASSERT_EQ(b.GenerateBinaryExpression(&expr), 5) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstantComposite %1 %3 %3 %3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            "%5 = " + param.name + " %1 %4 %4\n");
}
INSTANTIATE_TEST_SUITE_P(BuilderTest,
                         BinaryArithIntegerTest,
                         testing::Values(BinaryData{ast::BinaryOp::kAdd,
                                                    "OpIAdd"}));

using BinaryArithFloatTest = testing::TestWithParam<BinaryData>;
TEST_P(BinaryArithFloatTest, Scalar) {
  auto param = GetParam();

  ast::type::F32Type f32;

  auto lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.2f));
  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 4.5f));

  ast::BinaryExpression expr(param.op, std::move(lhs), std::move(rhs));

  Context ctx;
  TypeDeterminer td(&ctx);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b;
  b.push_function(Function{});

  ASSERT_EQ(b.GenerateBinaryExpression(&expr), 4) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 3.20000005
%3 = OpConstant %1 4.5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            "%4 = " + param.name + " %1 %2 %3\n");
}

TEST_P(BinaryArithFloatTest, Vector) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  auto lhs =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  auto rhs =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  Context ctx;
  TypeDeterminer td(&ctx);

  ast::BinaryExpression expr(param.op, std::move(lhs), std::move(rhs));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b;
  b.push_function(Function{});

  ASSERT_EQ(b.GenerateBinaryExpression(&expr), 5) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstantComposite %1 %3 %3 %3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            "%5 = " + param.name + " %1 %4 %4\n");
}
INSTANTIATE_TEST_SUITE_P(BuilderTest,
                         BinaryArithFloatTest,
                         testing::Values(BinaryData{ast::BinaryOp::kAdd,
                                                    "OpFAdd"}));

using BinaryCompareIntegerTest = testing::TestWithParam<BinaryData>;
TEST_P(BinaryCompareIntegerTest, Scalar) {
  auto param = GetParam();

  ast::type::I32Type i32;

  auto lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 3));
  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 4));

  ast::BinaryExpression expr(param.op, std::move(lhs), std::move(rhs));

  Context ctx;
  TypeDeterminer td(&ctx);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b;
  b.push_function(Function{});

  ASSERT_EQ(b.GenerateBinaryExpression(&expr), 4) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeInt 32 1
%2 = OpConstant %1 3
%3 = OpConstant %1 4
%5 = OpTypeBool
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            "%4 = " + param.name + " %5 %2 %3\n");
}

TEST_P(BinaryCompareIntegerTest, Vector) {
  auto param = GetParam();

  ast::type::I32Type i32;
  ast::type::VectorType vec3(&i32, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 1)));
  auto lhs =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::IntLiteral>(&i32, 1)));
  auto rhs =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  Context ctx;
  TypeDeterminer td(&ctx);

  ast::BinaryExpression expr(param.op, std::move(lhs), std::move(rhs));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b;
  b.push_function(Function{});

  ASSERT_EQ(b.GenerateBinaryExpression(&expr), 5) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstantComposite %1 %3 %3 %3
%7 = OpTypeBool
%6 = OpTypeVector %7 3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            "%5 = " + param.name + " %6 %4 %4\n");
}
INSTANTIATE_TEST_SUITE_P(
    BuilderTest,
    BinaryCompareIntegerTest,
    testing::Values(BinaryData{ast::BinaryOp::kEqual, "OpIEqual"},
                    BinaryData{ast::BinaryOp::kNotEqual, "OpINotEqual"}));

using BinaryCompareFloatTest = testing::TestWithParam<BinaryData>;
TEST_P(BinaryCompareFloatTest, Scalar) {
  auto param = GetParam();

  ast::type::F32Type f32;

  auto lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.2f));
  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 4.5f));

  ast::BinaryExpression expr(param.op, std::move(lhs), std::move(rhs));

  Context ctx;
  TypeDeterminer td(&ctx);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b;
  b.push_function(Function{});

  ASSERT_EQ(b.GenerateBinaryExpression(&expr), 4) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 3.20000005
%3 = OpConstant %1 4.5
%5 = OpTypeBool
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            "%4 = " + param.name + " %5 %2 %3\n");
}

TEST_P(BinaryCompareFloatTest, Vector) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  auto lhs =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  auto rhs =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  Context ctx;
  TypeDeterminer td(&ctx);

  ast::BinaryExpression expr(param.op, std::move(lhs), std::move(rhs));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b;
  b.push_function(Function{});

  ASSERT_EQ(b.GenerateBinaryExpression(&expr), 5) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstantComposite %1 %3 %3 %3
%7 = OpTypeBool
%6 = OpTypeVector %7 3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            "%5 = " + param.name + " %6 %4 %4\n");
}
INSTANTIATE_TEST_SUITE_P(
    BuilderTest,
    BinaryCompareFloatTest,
    testing::Values(BinaryData{ast::BinaryOp::kEqual, "OpFOrdEqual"},
                    BinaryData{ast::BinaryOp::kNotEqual, "OpFOrdNotEqual"}));

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
