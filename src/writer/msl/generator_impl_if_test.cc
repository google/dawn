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
#include "src/ast/else_statement.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/module.h"
#include "src/ast/return_statement.h"
#include "src/writer/msl/generator_impl.h"
#include "src/writer/msl/test_helper.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, Emit_If) {
  auto* cond = Expr("cond");
  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::ReturnStatement>(),
  });
  auto* i = create<ast::IfStatement>(cond, body, ast::ElseStatementList{});

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(i)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  if (cond) {
    return;
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_IfWithElseIf) {
  auto* else_cond = Expr("else_cond");
  auto* else_body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::ReturnStatement>(),
  });

  auto* cond = Expr("cond");
  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::ReturnStatement>(),
  });
  auto* i = create<ast::IfStatement>(
      cond, body,
      ast::ElseStatementList{
          create<ast::ElseStatement>(else_cond, else_body),
      });

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(i)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  if (cond) {
    return;
  } else if (else_cond) {
    return;
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_IfWithElse) {
  auto* else_body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::ReturnStatement>(),
  });

  auto* cond = Expr("cond");
  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::ReturnStatement>(),
  });
  auto* i = create<ast::IfStatement>(
      cond, body,
      ast::ElseStatementList{
          create<ast::ElseStatement>(nullptr, else_body),
      });

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(i)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  if (cond) {
    return;
  } else {
    return;
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_IfWithMultiple) {
  auto* else_cond = Expr("else_cond");

  auto* else_body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::ReturnStatement>(),
  });

  auto* else_body_2 = create<ast::BlockStatement>(ast::StatementList{
      create<ast::ReturnStatement>(),
  });

  auto* cond = Expr("cond");
  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::ReturnStatement>(),
  });
  auto* i = create<ast::IfStatement>(
      cond, body,
      ast::ElseStatementList{
          create<ast::ElseStatement>(else_cond, else_body),
          create<ast::ElseStatement>(nullptr, else_body_2),
      });

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(i)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  if (cond) {
    return;
  } else if (else_cond) {
    return;
  } else {
    return;
  }
)");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
