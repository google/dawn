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
#include "src/ast/float_literal.h"
#include "src/ast/relational_expression.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type_constructor_expression.h"
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

  Builder b;
  EXPECT_EQ(b.GenerateConstructorExpression(&c, true), 2);
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

  Builder b;
  EXPECT_EQ(b.GenerateConstructorExpression(&t, true), 5);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 3
%5 = OpConstantComposite %1 %3 %3 %4
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

  Builder b;
  EXPECT_EQ(b.GenerateConstructorExpression(&t, true), 5);
  EXPECT_EQ(b.GenerateConstructorExpression(&t, true), 5);
  ASSERT_FALSE(b.has_error()) << b.error();
}

TEST_F(BuilderTest, Constructor_NonConst_Type_Fails) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 2);
  auto rel = std::make_unique<ast::RelationalExpression>(
      ast::Relation::kAdd,
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::FloatLiteral>(&f32, 3.0f)),
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::move(rel));

  ast::TypeConstructorExpression t(&vec, std::move(vals));

  Builder b;
  EXPECT_EQ(b.GenerateConstructorExpression(&t, true), 0);
  EXPECT_TRUE(b.has_error());
  EXPECT_EQ(b.error(), R"(constructor must be a constant expression)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
