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

#include "src/ast/variable_decl_statement.h"
#include "src/writer/msl/test_helper.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, Emit_Loop) {
  auto* body = Block(create<ast::DiscardStatement>());
  auto* continuing = Block();
  auto* l = Loop(body, continuing);
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
  auto* body = Block(create<ast::DiscardStatement>());
  auto* continuing = Block(Return());
  auto* l = Loop(body, continuing);
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
  Global("lhs", ty.f32(), ast::StorageClass::kInput);
  Global("rhs", ty.f32(), ast::StorageClass::kInput);

  auto* body = Block(create<ast::DiscardStatement>());
  auto* continuing = Block(Return());
  auto* inner = Loop(body, continuing);

  body = Block(inner);

  continuing = Block(Assign("lhs", "rhs"));

  auto* outer = Loop(body, continuing);
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

  Global("rhs", ty.f32(), ast::StorageClass::kInput);

  auto* var = Var("lhs", ty.f32(), ast::StorageClass::kNone, Expr(2.4f));

  auto* body = Block(Decl(var), Decl(Var("other", ty.f32())));

  auto* continuing = Block(Assign("lhs", "rhs"));

  auto* outer = Loop(body, continuing);
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
