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
#include "src/ast/call_expression.h"
#include "src/ast/call_statement.h"
#include "src/ast/function.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/module.h"
#include "src/ast/type/void_type.h"
#include "src/writer/msl/generator_impl.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = testing::Test;

TEST_F(MslGeneratorImplTest, EmitExpression_Call_WithoutParams) {
  ast::type::VoidType void_type;

  auto id = std::make_unique<ast::IdentifierExpression>("my_func");
  ast::CallExpression call(std::move(id), {});

  auto func = std::make_unique<ast::Function>("my_func", ast::VariableList{},
                                              &void_type);

  ast::Module m;
  m.AddFunction(std::move(func));

  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitExpression(&call)) << g.error();
  EXPECT_EQ(g.result(), "my_func()");
}

TEST_F(MslGeneratorImplTest, EmitExpression_Call_WithParams) {
  ast::type::VoidType void_type;

  auto id = std::make_unique<ast::IdentifierExpression>("my_func");
  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("param1"));
  params.push_back(std::make_unique<ast::IdentifierExpression>("param2"));
  ast::CallExpression call(std::move(id), std::move(params));

  auto func = std::make_unique<ast::Function>("my_func", ast::VariableList{},
                                              &void_type);

  ast::Module m;
  m.AddFunction(std::move(func));

  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitExpression(&call)) << g.error();
  EXPECT_EQ(g.result(), "my_func(param1, param2)");
}

TEST_F(MslGeneratorImplTest, EmitStatement_Call) {
  ast::type::VoidType void_type;

  auto id = std::make_unique<ast::IdentifierExpression>("my_func");
  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("param1"));
  params.push_back(std::make_unique<ast::IdentifierExpression>("param2"));
  ast::CallStatement call(
      std::make_unique<ast::CallExpression>(std::move(id), std::move(params)));

  auto func = std::make_unique<ast::Function>("my_func", ast::VariableList{},
                                              &void_type);

  ast::Module m;
  m.AddFunction(std::move(func));

  GeneratorImpl g(&m);
  g.increment_indent();
  ASSERT_TRUE(g.EmitStatement(&call)) << g.error();
  EXPECT_EQ(g.result(), "  my_func(param1, param2);\n");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
