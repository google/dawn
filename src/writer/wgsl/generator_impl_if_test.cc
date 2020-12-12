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
#include "src/writer/wgsl/test_helper.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, Emit_If) {
  auto* cond = create<ast::IdentifierExpression>(
      Source{}, mod.RegisterSymbol("cond"), "cond");
  auto* body = create<ast::BlockStatement>(Source{});
  body->append(create<ast::DiscardStatement>(Source{}));

  ast::IfStatement i(Source{}, cond, body, ast::ElseStatementList{});

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(&i)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  if (cond) {
    discard;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_IfWithElseIf) {
  auto* else_cond = create<ast::IdentifierExpression>(
      Source{}, mod.RegisterSymbol("else_cond"), "else_cond");
  auto* else_body = create<ast::BlockStatement>(Source{});
  else_body->append(create<ast::DiscardStatement>(Source{}));

  auto* cond = create<ast::IdentifierExpression>(
      Source{}, mod.RegisterSymbol("cond"), "cond");
  auto* body = create<ast::BlockStatement>(Source{});
  body->append(create<ast::DiscardStatement>(Source{}));

  ast::IfStatement i(
      Source{}, cond, body,
      {create<ast::ElseStatement>(Source{}, else_cond, else_body)});

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(&i)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  if (cond) {
    discard;
  } elseif (else_cond) {
    discard;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_IfWithElse) {
  auto* else_body = create<ast::BlockStatement>(Source{});
  else_body->append(create<ast::DiscardStatement>(Source{}));

  auto* cond = create<ast::IdentifierExpression>(
      Source{}, mod.RegisterSymbol("cond"), "cond");
  auto* body = create<ast::BlockStatement>(Source{});
  body->append(create<ast::DiscardStatement>(Source{}));

  ast::IfStatement i(
      Source{}, cond, body,
      {create<ast::ElseStatement>(Source{}, nullptr, else_body)});

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(&i)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  if (cond) {
    discard;
  } else {
    discard;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_IfWithMultiple) {
  auto* else_cond = create<ast::IdentifierExpression>(
      Source{}, mod.RegisterSymbol("else_cond"), "else_cond");

  auto* else_body = create<ast::BlockStatement>(Source{});
  else_body->append(create<ast::DiscardStatement>(Source{}));

  auto* else_body_2 = create<ast::BlockStatement>(Source{});
  else_body_2->append(create<ast::DiscardStatement>(Source{}));

  auto* cond = create<ast::IdentifierExpression>(
      Source{}, mod.RegisterSymbol("cond"), "cond");
  auto* body = create<ast::BlockStatement>(Source{});
  body->append(create<ast::DiscardStatement>(Source{}));

  ast::IfStatement i(
      Source{}, cond, body,
      {
          create<ast::ElseStatement>(Source{}, else_cond, else_body),
          create<ast::ElseStatement>(Source{}, nullptr, else_body_2),
      });

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(&i)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  if (cond) {
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
