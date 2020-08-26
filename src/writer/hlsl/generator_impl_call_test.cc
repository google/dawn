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

#include "src/ast/call_expression.h"
#include "src/ast/call_statement.h"
#include "src/ast/function.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/module.h"
#include "src/ast/type/void_type.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_Call = TestHelper;

TEST_F(HlslGeneratorImplTest_Call, EmitExpression_Call_WithoutParams) {
  ast::type::VoidType void_type;

  auto id = std::make_unique<ast::IdentifierExpression>("my_func");
  ast::CallExpression call(std::move(id), {});

  auto func = std::make_unique<ast::Function>("my_func", ast::VariableList{},
                                              &void_type);
  mod()->AddFunction(std::move(func));

  ASSERT_TRUE(gen().EmitExpression(out(), &call)) << gen().error();
  EXPECT_EQ(result(), "my_func()");
}

TEST_F(HlslGeneratorImplTest_Call, EmitExpression_Call_WithParams) {
  ast::type::VoidType void_type;

  auto id = std::make_unique<ast::IdentifierExpression>("my_func");
  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("param1"));
  params.push_back(std::make_unique<ast::IdentifierExpression>("param2"));
  ast::CallExpression call(std::move(id), std::move(params));

  auto func = std::make_unique<ast::Function>("my_func", ast::VariableList{},
                                              &void_type);
  mod()->AddFunction(std::move(func));

  ASSERT_TRUE(gen().EmitExpression(out(), &call)) << gen().error();
  EXPECT_EQ(result(), "my_func(param1, param2)");
}

TEST_F(HlslGeneratorImplTest_Call, EmitStatement_Call) {
  ast::type::VoidType void_type;

  auto id = std::make_unique<ast::IdentifierExpression>("my_func");
  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("param1"));
  params.push_back(std::make_unique<ast::IdentifierExpression>("param2"));
  ast::CallStatement call(
      std::make_unique<ast::CallExpression>(std::move(id), std::move(params)));

  auto func = std::make_unique<ast::Function>("my_func", ast::VariableList{},
                                              &void_type);
  mod()->AddFunction(std::move(func));
  gen().increment_indent();
  ASSERT_TRUE(gen().EmitStatement(out(), &call)) << gen().error();
  EXPECT_EQ(result(), "  my_func(param1, param2);\n");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
