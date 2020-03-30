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
#include <vector>

#include "gtest/gtest.h"
#include "src/ast/else_statement.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/kill_statement.h"
#include "src/writer/wgsl/generator_impl.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using GeneratorImplTest = testing::Test;

TEST_F(GeneratorImplTest, Emit_If) {
  auto cond = std::make_unique<ast::IdentifierExpression>("cond");
  std::vector<std::unique_ptr<ast::Statement>> body;
  body.push_back(std::make_unique<ast::KillStatement>());

  ast::IfStatement i(std::move(cond), std::move(body));

  GeneratorImpl g;
  g.increment_indent();

  ASSERT_TRUE(g.EmitStatement(&i)) << g.error();
  EXPECT_EQ(g.result(), R"(  if (cond) {
    kill;
  }
)");
}

TEST_F(GeneratorImplTest, Emit_IfWithElseIf) {
  auto else_cond = std::make_unique<ast::IdentifierExpression>("else_cond");

  std::vector<std::unique_ptr<ast::Statement>> else_body;
  else_body.push_back(std::make_unique<ast::KillStatement>());

  std::vector<std::unique_ptr<ast::ElseStatement>> elses;
  elses.push_back(std::make_unique<ast::ElseStatement>(std::move(else_cond),
                                                       std::move(else_body)));

  auto cond = std::make_unique<ast::IdentifierExpression>("cond");
  std::vector<std::unique_ptr<ast::Statement>> body;
  body.push_back(std::make_unique<ast::KillStatement>());

  ast::IfStatement i(std::move(cond), std::move(body));
  i.set_else_statements(std::move(elses));

  GeneratorImpl g;
  g.increment_indent();

  ASSERT_TRUE(g.EmitStatement(&i)) << g.error();
  EXPECT_EQ(g.result(), R"(  if (cond) {
    kill;
  } elseif (else_cond) {
    kill;
  }
)");
}

TEST_F(GeneratorImplTest, Emit_IfWithElse) {
  std::vector<std::unique_ptr<ast::Statement>> else_body;
  else_body.push_back(std::make_unique<ast::KillStatement>());

  std::vector<std::unique_ptr<ast::ElseStatement>> elses;
  elses.push_back(std::make_unique<ast::ElseStatement>(std::move(else_body)));

  auto cond = std::make_unique<ast::IdentifierExpression>("cond");
  std::vector<std::unique_ptr<ast::Statement>> body;
  body.push_back(std::make_unique<ast::KillStatement>());

  ast::IfStatement i(std::move(cond), std::move(body));
  i.set_else_statements(std::move(elses));

  GeneratorImpl g;
  g.increment_indent();

  ASSERT_TRUE(g.EmitStatement(&i)) << g.error();
  EXPECT_EQ(g.result(), R"(  if (cond) {
    kill;
  } else {
    kill;
  }
)");
}

TEST_F(GeneratorImplTest, Emit_IfWithMultiple) {
  auto else_cond = std::make_unique<ast::IdentifierExpression>("else_cond");

  std::vector<std::unique_ptr<ast::Statement>> else_body;
  else_body.push_back(std::make_unique<ast::KillStatement>());

  std::vector<std::unique_ptr<ast::Statement>> else_body_2;
  else_body_2.push_back(std::make_unique<ast::KillStatement>());

  std::vector<std::unique_ptr<ast::ElseStatement>> elses;
  elses.push_back(std::make_unique<ast::ElseStatement>(std::move(else_cond),
                                                       std::move(else_body)));
  elses.push_back(std::make_unique<ast::ElseStatement>(std::move(else_body_2)));

  auto cond = std::make_unique<ast::IdentifierExpression>("cond");
  std::vector<std::unique_ptr<ast::Statement>> body;
  body.push_back(std::make_unique<ast::KillStatement>());

  ast::IfStatement i(std::move(cond), std::move(body));
  i.set_else_statements(std::move(elses));

  GeneratorImpl g;
  g.increment_indent();

  ASSERT_TRUE(g.EmitStatement(&i)) << g.error();
  EXPECT_EQ(g.result(), R"(  if (cond) {
    kill;
  } elseif (else_cond) {
    kill;
  } else {
    kill;
  }
)");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
