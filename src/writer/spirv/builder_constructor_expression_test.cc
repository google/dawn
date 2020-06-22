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

#include <string>

#include "gtest/gtest.h"
#include "spirv/unified1/spirv.h"
#include "spirv/unified1/spirv.hpp11"
#include "src/ast/binary_expression.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/type/f32_type.h"
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

TEST_F(BuilderTest, Constructor_Const) {
  ast::type::F32Type f32;
  auto fl = std::make_unique<ast::FloatLiteral>(&f32, 42.2f);
  ast::ScalarConstructorExpression c(std::move(fl));

  ast::Module mod;
  Builder b(&mod);
  EXPECT_EQ(b.GenerateConstructorExpression(&c, true), 2u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 42.2000008
)");
}

TEST_F(BuilderTest, Constructor_Type) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::TypeConstructorExpression t(&vec, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  EXPECT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  EXPECT_EQ(b.GenerateConstructorExpression(&t, true), 5u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 3
%5 = OpConstantComposite %1 %3 %3 %4
)");
}

TEST_F(BuilderTest, Constructor_Type_ZeroInit) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 2);

  ast::ExpressionList vals;
  ast::TypeConstructorExpression t(&vec, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  EXPECT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});

  EXPECT_EQ(b.GenerateConstructorExpression(&t, false), 3u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 2
%3 = OpConstantNull %1
)");
}

TEST_F(BuilderTest, Constructor_Type_NonConstructorParam) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 2);

  auto var = std::make_unique<ast::Variable>(
      "ident", ast::StorageClass::kFunction, &f32);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::IdentifierExpression>("ident"));

  ast::TypeConstructorExpression t(&vec, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());
  EXPECT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(var.get())) << b.error();

  EXPECT_EQ(b.GenerateConstructorExpression(&t, false), 8u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Function %3
%4 = OpConstantNull %3
%5 = OpTypeVector %3 2
%6 = OpConstant %3 1
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %4
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%8 = OpCompositeConstruct %5 %6 %7
)");
}

TEST_F(BuilderTest, Constructor_Type_NonConstVector) {
  ast::type::F32Type f32;
  ast::type::VectorType vec2(&f32, 2);
  ast::type::VectorType vec4(&f32, 4);

  auto var = std::make_unique<ast::Variable>(
      "ident", ast::StorageClass::kFunction, &vec2);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::IdentifierExpression>("ident"));

  ast::TypeConstructorExpression t(&vec4, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());
  EXPECT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(var.get())) << b.error();

  EXPECT_EQ(b.GenerateConstructorExpression(&t, false), 11u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 2
%2 = OpTypePointer Function %3
%5 = OpConstantNull %3
%6 = OpTypeVector %4 4
%7 = OpConstant %4 1
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %5
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%8 = OpLoad %3 %1
%9 = OpCompositeExtract %4 %8 0
%10 = OpCompositeExtract %4 %8 1
%11 = OpCompositeConstruct %6 %7 %7 %9 %10
)");
}

TEST_F(BuilderTest, Constructor_Type_Dedups) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::TypeConstructorExpression t(&vec, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  EXPECT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  EXPECT_EQ(b.GenerateConstructorExpression(&t, true), 5u);
  EXPECT_EQ(b.GenerateConstructorExpression(&t, true), 5u);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest, Constructor_NonConst_Type_Fails) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 2);
  auto rel = std::make_unique<ast::BinaryExpression>(
      ast::BinaryOp::kAdd,
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::FloatLiteral>(&f32, 3.0f)),
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::move(rel));

  ast::TypeConstructorExpression t(&vec, std::move(vals));

  ast::Module mod;
  Builder b(&mod);
  EXPECT_EQ(b.GenerateConstructorExpression(&t, true), 0u);
  EXPECT_TRUE(b.has_error());
  EXPECT_EQ(b.error(), R"(constructor must be a constant expression)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
