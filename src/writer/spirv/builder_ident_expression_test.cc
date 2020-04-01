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
#include "src/ast/float_literal.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/type/vector_type.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/variable.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = testing::Test;

TEST_F(BuilderTest, IdentifierExpression_GlobalConst) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  std::vector<std::unique_ptr<ast::Expression>> vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  auto init =
      std::make_unique<ast::TypeConstructorExpression>(&vec, std::move(vals));

  ast::Variable v("var", ast::StorageClass::kOutput, &f32);
  v.set_constructor(std::move(init));
  v.set_is_const(true);

  Builder b;
  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 3
%5 = OpConstantComposite %1 %3 %3 %4
)");

  ast::IdentifierExpression expr("var");
  EXPECT_EQ(b.GenerateIdentifierExpression(&expr), 5);
}

TEST_F(BuilderTest, IdentifierExpression_GlobalVar) {
  ast::type::F32Type f32;
  ast::Variable v("var", ast::StorageClass::kOutput, &f32);

  Builder b;
  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "var"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Output %3
%1 = OpVariable %2 Output
)");

  ast::IdentifierExpression expr("var");
  EXPECT_EQ(b.GenerateIdentifierExpression(&expr), 1);
}

TEST_F(BuilderTest, IdentifierExpression_FunctionConst) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  std::vector<std::unique_ptr<ast::Expression>> vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  auto init =
      std::make_unique<ast::TypeConstructorExpression>(&vec, std::move(vals));

  ast::Variable v("var", ast::StorageClass::kOutput, &f32);
  v.set_constructor(std::move(init));
  v.set_is_const(true);

  Builder b;
  EXPECT_TRUE(b.GenerateFunctionVariable(&v)) << b.error();
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 3
%5 = OpConstantComposite %1 %3 %3 %4
)");

  ast::IdentifierExpression expr("var");
  EXPECT_EQ(b.GenerateIdentifierExpression(&expr), 5);
}

TEST_F(BuilderTest, IdentifierExpression_FunctionVar) {
  ast::type::F32Type f32;
  ast::Variable v("var", ast::StorageClass::kNone, &f32);

  Builder b;
  b.push_function(Function{});
  EXPECT_TRUE(b.GenerateFunctionVariable(&v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "var"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Function %3
)");

  const auto& func = b.functions()[0];
  EXPECT_EQ(DumpInstructions(func.variables()), R"(%1 = OpVariable %2 Function
)");

  ast::IdentifierExpression expr("var");
  EXPECT_EQ(b.GenerateIdentifierExpression(&expr), 1);
}

TEST_F(BuilderTest, DISABLED_IdentifierExpression_MultiName) {}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint

