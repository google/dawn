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
  EXPECT_EQ(gen.result(), R"(  while (true) {
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
  EXPECT_EQ(gen.result(), R"(  while (true) {
    discard_fragment();
    {
      return;
    }
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_LoopNestedWithContinuing) {
  Global("lhs", ty.f32(), ast::StorageClass::kPrivate);
  Global("rhs", ty.f32(), ast::StorageClass::kPrivate);

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
  EXPECT_EQ(gen.result(), R"(  while (true) {
    while (true) {
      discard_fragment();
      {
        return;
      }
    }
    {
      lhs = rhs;
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

  Global("rhs", ty.f32(), ast::StorageClass::kPrivate);

  auto* var = Var("lhs", ty.f32(), ast::StorageClass::kNone, Expr(2.4f));

  auto* body = Block(Decl(var), Decl(Var("other", ty.f32())));

  auto* continuing = Block(Assign("lhs", "rhs"));

  auto* outer = Loop(body, continuing);
  WrapInFunction(outer);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(outer)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  while (true) {
    float lhs = 2.400000095f;
    float other = 0.0f;
    {
      lhs = rhs;
    }
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_ForLoop) {
  // for(; ; ) {
  //   return;
  // }

  auto* f = For(nullptr, nullptr, nullptr, Block(Return()));
  WrapInFunction(f);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(f)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  for(; ; ) {
    return;
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_ForLoopWithSimpleInit) {
  // for(var i : i32; ; ) {
  //   return;
  // }

  auto* f = For(Decl(Var("i", ty.i32())), nullptr, nullptr, Block(Return()));
  WrapInFunction(f);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(f)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  for(int i = 0; ; ) {
    return;
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_ForLoopWithMultiStmtInit) {
  // var<workgroup> a : atomic<i32>;
  // for(var b = atomicCompareExchangeWeak(&a, 1, 2); ; ) {
  //   return;
  // }
  Global("a", ty.atomic<i32>(), ast::StorageClass::kWorkgroup);
  auto* multi_stmt = Block(Ignore(1), Ignore(2));
  auto* f = For(multi_stmt, nullptr, nullptr, Block(Return()));
  WrapInFunction(f);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(f)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  {
    {
      (void) 1;
      (void) 2;
    }
    for(; ; ) {
      return;
    }
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_ForLoopWithSimpleCond) {
  // for(; true; ) {
  //   return;
  // }

  auto* f = For(nullptr, true, nullptr, Block(Return()));
  WrapInFunction(f);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(f)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  for(; true; ) {
    return;
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_ForLoopWithSimpleCont) {
  // for(; ; i = i + 1) {
  //   return;
  // }

  auto* v = Decl(Var("i", ty.i32()));
  auto* f = For(nullptr, nullptr, Assign("i", Add("i", 1)), Block(Return()));
  WrapInFunction(v, f);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(f)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  for(; ; i = (i + 1)) {
    return;
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_ForLoopWithMultiStmtCont) {
  // var<workgroup> a : atomic<i32>;
  // for(; ; { ignore(1); ignore(2); }) {
  //   return;
  // }

  Global("a", ty.atomic<i32>(), ast::StorageClass::kWorkgroup);
  auto* multi_stmt = Block(Ignore(1), Ignore(2));
  auto* f = For(nullptr, nullptr, multi_stmt, Block(Return()));
  WrapInFunction(f);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(f)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  while (true) {
    return;
    {
      (void) 1;
      (void) 2;
    }
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_ForLoopWithSimpleInitCondCont) {
  // for(var i : i32; true; i = i + 1) {
  //   return;
  // }

  auto* f = For(Decl(Var("i", ty.i32())), true, Assign("i", Add("i", 1)),
                Block(Return()));
  WrapInFunction(f);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(f)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  for(int i = 0; true; i = (i + 1)) {
    return;
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_ForLoopWithMultiStmtInitCondCont) {
  // var<workgroup> a : atomic<i32>;
  // for({ ignore(1); ignore(2); }; true; { ignore(3); ignore(4); }) {
  //   return;
  // }
  Global("a", ty.atomic<i32>(), ast::StorageClass::kWorkgroup);
  auto* multi_stmt_a = Block(Ignore(1), Ignore(2));
  auto* multi_stmt_b = Block(Ignore(3), Ignore(4));
  auto* f = For(multi_stmt_a, Expr(true), multi_stmt_b, Block(Return()));
  WrapInFunction(f);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(f)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  {
    {
      (void) 1;
      (void) 2;
    }
    while (true) {
      if (!(true)) { break; }
      return;
      {
        (void) 3;
        (void) 4;
      }
    }
  }
)");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
