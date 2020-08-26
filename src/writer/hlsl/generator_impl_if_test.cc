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

#include "src/ast/else_statement.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/module.h"
#include "src/ast/return_statement.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_If = TestHelper;

TEST_F(HlslGeneratorImplTest_If, Emit_If) {
  auto cond = std::make_unique<ast::IdentifierExpression>("cond");
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>());

  ast::IfStatement i(std::move(cond), std::move(body));
  gen().increment_indent();

  ASSERT_TRUE(gen().EmitStatement(out(), &i)) << gen().error();
  EXPECT_EQ(result(), R"(  if (cond) {
    return;
  }
)");
}

TEST_F(HlslGeneratorImplTest_If, Emit_IfWithElseIf) {
  auto else_cond = std::make_unique<ast::IdentifierExpression>("else_cond");
  auto else_body = std::make_unique<ast::BlockStatement>();
  else_body->append(std::make_unique<ast::ReturnStatement>());

  ast::ElseStatementList elses;
  elses.push_back(std::make_unique<ast::ElseStatement>(std::move(else_cond),
                                                       std::move(else_body)));

  auto cond = std::make_unique<ast::IdentifierExpression>("cond");
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>());

  ast::IfStatement i(std::move(cond), std::move(body));
  i.set_else_statements(std::move(elses));

  gen().increment_indent();

  ASSERT_TRUE(gen().EmitStatement(out(), &i)) << gen().error();
  EXPECT_EQ(result(), R"(  if (cond) {
    return;
  } else if (else_cond) {
    return;
  }
)");
}

TEST_F(HlslGeneratorImplTest_If, Emit_IfWithElse) {
  auto else_body = std::make_unique<ast::BlockStatement>();
  else_body->append(std::make_unique<ast::ReturnStatement>());

  ast::ElseStatementList elses;
  elses.push_back(std::make_unique<ast::ElseStatement>(std::move(else_body)));

  auto cond = std::make_unique<ast::IdentifierExpression>("cond");
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>());

  ast::IfStatement i(std::move(cond), std::move(body));
  i.set_else_statements(std::move(elses));

  gen().increment_indent();

  ASSERT_TRUE(gen().EmitStatement(out(), &i)) << gen().error();
  EXPECT_EQ(result(), R"(  if (cond) {
    return;
  } else {
    return;
  }
)");
}

TEST_F(HlslGeneratorImplTest_If, Emit_IfWithMultiple) {
  auto else_cond = std::make_unique<ast::IdentifierExpression>("else_cond");

  auto else_body = std::make_unique<ast::BlockStatement>();
  else_body->append(std::make_unique<ast::ReturnStatement>());

  auto else_body_2 = std::make_unique<ast::BlockStatement>();
  else_body_2->append(std::make_unique<ast::ReturnStatement>());

  ast::ElseStatementList elses;
  elses.push_back(std::make_unique<ast::ElseStatement>(std::move(else_cond),
                                                       std::move(else_body)));
  elses.push_back(std::make_unique<ast::ElseStatement>(std::move(else_body_2)));

  auto cond = std::make_unique<ast::IdentifierExpression>("cond");
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>());

  ast::IfStatement i(std::move(cond), std::move(body));
  i.set_else_statements(std::move(elses));

  gen().increment_indent();

  ASSERT_TRUE(gen().EmitStatement(out(), &i)) << gen().error();
  EXPECT_EQ(result(), R"(  if (cond) {
    return;
  } else if (else_cond) {
    return;
  } else {
    return;
  }
)");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
