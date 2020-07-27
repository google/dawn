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

#include "gtest/gtest.h"
#include "src/ast/discard_statement.h"
#include "src/ast/else_statement.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/writer/wgsl/generator_impl.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using WgslGeneratorImplTest = testing::Test;

TEST_F(WgslGeneratorImplTest, Emit_If) {
  auto cond = std::make_unique<ast::IdentifierExpression>("cond");
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::DiscardStatement>());

  ast::IfStatement i(std::move(cond), std::move(body));

  GeneratorImpl g;
  g.increment_indent();

  ASSERT_TRUE(g.EmitStatement(&i)) << g.error();
  EXPECT_EQ(g.result(), R"(  if (cond) {
    discard;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_IfWithElseIf) {
  auto else_cond = std::make_unique<ast::IdentifierExpression>("else_cond");
  auto else_body = std::make_unique<ast::BlockStatement>();
  else_body->append(std::make_unique<ast::DiscardStatement>());

  ast::ElseStatementList elses;
  elses.push_back(std::make_unique<ast::ElseStatement>(std::move(else_cond),
                                                       std::move(else_body)));

  auto cond = std::make_unique<ast::IdentifierExpression>("cond");
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::DiscardStatement>());

  ast::IfStatement i(std::move(cond), std::move(body));
  i.set_else_statements(std::move(elses));

  GeneratorImpl g;
  g.increment_indent();

  ASSERT_TRUE(g.EmitStatement(&i)) << g.error();
  EXPECT_EQ(g.result(), R"(  if (cond) {
    discard;
  } elseif (else_cond) {
    discard;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_IfWithElse) {
  auto else_body = std::make_unique<ast::BlockStatement>();
  else_body->append(std::make_unique<ast::DiscardStatement>());

  ast::ElseStatementList elses;
  elses.push_back(std::make_unique<ast::ElseStatement>(std::move(else_body)));

  auto cond = std::make_unique<ast::IdentifierExpression>("cond");
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::DiscardStatement>());

  ast::IfStatement i(std::move(cond), std::move(body));
  i.set_else_statements(std::move(elses));

  GeneratorImpl g;
  g.increment_indent();

  ASSERT_TRUE(g.EmitStatement(&i)) << g.error();
  EXPECT_EQ(g.result(), R"(  if (cond) {
    discard;
  } else {
    discard;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_IfWithMultiple) {
  auto else_cond = std::make_unique<ast::IdentifierExpression>("else_cond");

  auto else_body = std::make_unique<ast::BlockStatement>();
  else_body->append(std::make_unique<ast::DiscardStatement>());

  auto else_body_2 = std::make_unique<ast::BlockStatement>();
  else_body_2->append(std::make_unique<ast::DiscardStatement>());

  ast::ElseStatementList elses;
  elses.push_back(std::make_unique<ast::ElseStatement>(std::move(else_cond),
                                                       std::move(else_body)));
  elses.push_back(std::make_unique<ast::ElseStatement>(std::move(else_body_2)));

  auto cond = std::make_unique<ast::IdentifierExpression>("cond");
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::DiscardStatement>());

  ast::IfStatement i(std::move(cond), std::move(body));
  i.set_else_statements(std::move(elses));

  GeneratorImpl g;
  g.increment_indent();

  ASSERT_TRUE(g.EmitStatement(&i)) << g.error();
  EXPECT_EQ(g.result(), R"(  if (cond) {
    discard;
  } elseif (else_cond) {
    discard;
  } else {
    discard;
  }
)");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
