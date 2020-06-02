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
#include "src/ast/identifier_expression.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/variable.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = testing::Test;

TEST_F(BuilderTest, IdentifierExpression_GlobalConst) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  auto init =
      std::make_unique<ast::TypeConstructorExpression>(&vec, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  EXPECT_TRUE(td.DetermineResultType(init.get())) << td.error();

  ast::Variable v("var", ast::StorageClass::kOutput, &f32);
  v.set_constructor(std::move(init));
  v.set_is_const(true);

  td.RegisterVariableForTesting(&v);

  Builder b(&mod);
  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 3
%5 = OpConstantComposite %1 %3 %3 %4
)");

  ast::IdentifierExpression expr("var");
  ASSERT_TRUE(td.DetermineResultType(&expr));

  EXPECT_EQ(b.GenerateIdentifierExpression(&expr), 5u);
}

TEST_F(BuilderTest, IdentifierExpression_GlobalVar) {
  ast::type::F32Type f32;
  ast::Variable v("var", ast::StorageClass::kOutput, &f32);

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(&v);

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "var"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Output %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Output %4
)");

  ast::IdentifierExpression expr("var");
  ASSERT_TRUE(td.DetermineResultType(&expr));
  EXPECT_EQ(b.GenerateIdentifierExpression(&expr), 1u);
}

TEST_F(BuilderTest, IdentifierExpression_FunctionConst) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  auto init =
      std::make_unique<ast::TypeConstructorExpression>(&vec, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  EXPECT_TRUE(td.DetermineResultType(init.get())) << td.error();

  ast::Variable v("var", ast::StorageClass::kOutput, &f32);
  v.set_constructor(std::move(init));
  v.set_is_const(true);
  td.RegisterVariableForTesting(&v);

  Builder b(&mod);
  EXPECT_TRUE(b.GenerateFunctionVariable(&v)) << b.error();
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 3
%5 = OpConstantComposite %1 %3 %3 %4
)");

  ast::IdentifierExpression expr("var");
  ASSERT_TRUE(td.DetermineResultType(&expr));
  EXPECT_EQ(b.GenerateIdentifierExpression(&expr), 5u);
}

TEST_F(BuilderTest, IdentifierExpression_FunctionVar) {
  ast::type::F32Type f32;
  ast::Variable v("var", ast::StorageClass::kNone, &f32);

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(&v);

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_TRUE(b.GenerateFunctionVariable(&v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "var"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Function %3
%4 = OpConstantNull %3
)");

  const auto& func = b.functions()[0];
  EXPECT_EQ(DumpInstructions(func.variables()),
            R"(%1 = OpVariable %2 Function %4
)");

  ast::IdentifierExpression expr("var");
  ASSERT_TRUE(td.DetermineResultType(&expr));
  EXPECT_EQ(b.GenerateIdentifierExpression(&expr), 1u);
}

TEST_F(BuilderTest, IdentifierExpression_Load) {
  ast::type::I32Type i32;

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ast::Variable var("var", ast::StorageClass::kPrivate, &i32);

  td.RegisterVariableForTesting(&var);

  auto lhs = std::make_unique<ast::IdentifierExpression>("var");
  auto rhs = std::make_unique<ast::IdentifierExpression>("var");

  ast::BinaryExpression expr(ast::BinaryOp::kAdd, std::move(lhs),
                             std::move(rhs));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(&var)) << b.error();

  EXPECT_EQ(b.GenerateBinaryExpression(&expr), 7u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%5 = OpLoad %3 %1
%6 = OpLoad %3 %1
%7 = OpIAdd %3 %5 %6
)");
}

TEST_F(BuilderTest, IdentifierExpression_NoLoadConst) {
  ast::type::I32Type i32;

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ast::Variable var("var", ast::StorageClass::kNone, &i32);
  var.set_constructor(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2)));
  var.set_is_const(true);

  td.RegisterVariableForTesting(&var);

  auto lhs = std::make_unique<ast::IdentifierExpression>("var");
  auto rhs = std::make_unique<ast::IdentifierExpression>("var");

  ast::BinaryExpression expr(ast::BinaryOp::kAdd, std::move(lhs),
                             std::move(rhs));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(&var)) << b.error();

  EXPECT_EQ(b.GenerateBinaryExpression(&expr), 3u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeInt 32 1
%2 = OpConstant %1 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%3 = OpIAdd %1 %2 %2
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
