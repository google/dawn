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
#include "src/ast/assignment_statement.h"
#include "src/ast/discard_statement.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/loop_statement.h"
#include "src/ast/return_statement.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/program.h"
#include "src/type/f32_type.h"
#include "src/writer/msl/generator_impl.h"
#include "src/writer/msl/test_helper.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, Emit_Loop) {
  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::DiscardStatement>(),
  });
  auto* continuing = create<ast::BlockStatement>(ast::StatementList{});
  auto* l = create<ast::LoopStatement>(body, continuing);
  WrapInFunction(l);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(l)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  for(;;) {
    discard_fragment();
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_LoopWithContinuing) {
  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::DiscardStatement>(),
  });
  auto* continuing = create<ast::BlockStatement>(ast::StatementList{
      create<ast::ReturnStatement>(),
  });
  auto* l = create<ast::LoopStatement>(body, continuing);
  WrapInFunction(l);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(l)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  {
    bool tint_msl_is_first_1 = true;
    for(;;) {
      if (!tint_msl_is_first_1) {
        return;
      }
      tint_msl_is_first_1 = false;

      discard_fragment();
    }
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_LoopNestedWithContinuing) {
  Global("lhs", ty.f32(), ast::StorageClass::kNone);
  Global("rhs", ty.f32(), ast::StorageClass::kNone);

  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::DiscardStatement>(),
  });
  auto* continuing = create<ast::BlockStatement>(ast::StatementList{
      create<ast::ReturnStatement>(),
  });
  auto* inner = create<ast::LoopStatement>(body, continuing);

  body = create<ast::BlockStatement>(ast::StatementList{
      inner,
  });

  continuing = create<ast::BlockStatement>(ast::StatementList{
      create<ast::AssignmentStatement>(Expr("lhs"), Expr("rhs")),
  });

  auto* outer = create<ast::LoopStatement>(body, continuing);
  WrapInFunction(outer);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(outer)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  {
    bool tint_msl_is_first_1 = true;
    for(;;) {
      if (!tint_msl_is_first_1) {
        lhs = rhs;
      }
      tint_msl_is_first_1 = false;

      {
        bool tint_msl_is_first_2 = true;
        for(;;) {
          if (!tint_msl_is_first_2) {
            return;
          }
          tint_msl_is_first_2 = false;

          discard_fragment();
        }
      }
    }
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_LoopWithVarUsedInContinuing) {
  // loop {
  //   var lhs : f32 = 2.4;
  //   var other : f32;
  //   continuing {
  //     lhs = rhs
  //   }
  // }
  //
  // ->
  // {
  //   float lhs;
  //   float other;
  //   for (;;) {
  //     if (continuing) {
  //       lhs = rhs;
  //     }
  //     lhs = 2.4f;
  //     other = 0.0f;
  //   }
  // }

  Global("rhs", ty.f32(), ast::StorageClass::kNone);

  auto* var = Var("lhs", ty.f32(), ast::StorageClass::kFunction, Expr(2.4f));

  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::VariableDeclStatement>(var),
      create<ast::VariableDeclStatement>(
          Var("other", ty.f32(), ast::StorageClass::kFunction))});

  auto* continuing = create<ast::BlockStatement>(ast::StatementList{
      create<ast::AssignmentStatement>(Expr("lhs"), Expr("rhs")),
  });

  auto* outer = create<ast::LoopStatement>(body, continuing);
  WrapInFunction(outer);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(outer)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  {
    bool tint_msl_is_first_1 = true;
    float lhs;
    float other;
    for(;;) {
      if (!tint_msl_is_first_1) {
        lhs = rhs;
      }
      tint_msl_is_first_1 = false;

      lhs = 2.400000095f;
      other = 0.0f;
    }
  }
)");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
