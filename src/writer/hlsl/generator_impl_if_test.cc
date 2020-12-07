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
  auto* cond = create<ast::IdentifierExpression>("cond");
  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::ReturnStatement>());

  ast::IfStatement i(Source{}, cond, body, ast::ElseStatementList{});
  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(out, &i)) << gen.error();
  EXPECT_EQ(result(), R"(  if (cond) {
    return;
  }
)");
}

TEST_F(HlslGeneratorImplTest_If, Emit_IfWithElseIf) {
  auto* else_cond = create<ast::IdentifierExpression>("else_cond");
  auto* else_body = create<ast::BlockStatement>();
  else_body->append(create<ast::ReturnStatement>());

  auto* cond = create<ast::IdentifierExpression>("cond");
  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::ReturnStatement>());

  ast::IfStatement i(Source{}, cond, body,
                     {create<ast::ElseStatement>(else_cond, else_body)});

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(out, &i)) << gen.error();
  EXPECT_EQ(result(), R"(  if (cond) {
    return;
  } else {
    if (else_cond) {
      return;
    }
  }
)");
}

TEST_F(HlslGeneratorImplTest_If, Emit_IfWithElse) {
  auto* else_body = create<ast::BlockStatement>();
  else_body->append(create<ast::ReturnStatement>());

  auto* cond = create<ast::IdentifierExpression>("cond");
  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::ReturnStatement>());

  ast::IfStatement i(Source{}, cond, body,
                     {create<ast::ElseStatement>(else_body)});

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(out, &i)) << gen.error();
  EXPECT_EQ(result(), R"(  if (cond) {
    return;
  } else {
    return;
  }
)");
}

TEST_F(HlslGeneratorImplTest_If, Emit_IfWithMultiple) {
  auto* else_cond = create<ast::IdentifierExpression>("else_cond");

  auto* else_body = create<ast::BlockStatement>();
  else_body->append(create<ast::ReturnStatement>());

  auto* else_body_2 = create<ast::BlockStatement>();
  else_body_2->append(create<ast::ReturnStatement>());

  auto* cond = create<ast::IdentifierExpression>("cond");
  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::ReturnStatement>());

  ast::IfStatement i(Source{}, cond, body,
                     {
                         create<ast::ElseStatement>(else_cond, else_body),
                         create<ast::ElseStatement>(else_body_2),
                     });

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(out, &i)) << gen.error();
  EXPECT_EQ(result(), R"(  if (cond) {
    return;
  } else {
    if (else_cond) {
      return;
    } else {
      return;
    }
  }
)");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
