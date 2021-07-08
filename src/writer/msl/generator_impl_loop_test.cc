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
  auto* multi_stmt = Call("atomicCompareExchangeWeak", AddressOf("a"), 1, 2);
  auto* f = For(Decl(Var("b", nullptr, multi_stmt)), nullptr, nullptr,
                Block(Return()));
  WrapInFunction(f);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(f)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  {
    int prev_value = 1;
    bool matched = atomic_compare_exchange_weak_explicit(&(a), &prev_value, 2, memory_order_relaxed, memory_order_relaxed);
    int2 b = int2(prev_value, matched);
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

TEST_F(MslGeneratorImplTest, Emit_ForLoopWithMultiStmtCond) {
  // var<workgroup> a : atomic<i32>;
  // for(; atomicCompareExchangeWeak(&a, 1, 2).x == 0; ) {
  //   return;
  // }

  Global("a", ty.atomic<i32>(), ast::StorageClass::kWorkgroup);
  auto* multi_stmt = create<ast::BinaryExpression>(
      ast::BinaryOp::kEqual,
      MemberAccessor(Call("atomicCompareExchangeWeak", AddressOf("a"), 1, 2),
                     "x"),
      Expr(0));
  auto* f = For(nullptr, multi_stmt, nullptr, Block(Return()));
  WrapInFunction(f);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(f)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  while (true) {
    int prev_value = 1;
    bool matched = atomic_compare_exchange_weak_explicit(&(a), &prev_value, 2, memory_order_relaxed, memory_order_relaxed);
    if (!((int2(prev_value, matched).x == 0))) { break; }
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
  // for(; ; ignore(atomicCompareExchangeWeak(&a, 1, 2))) {
  //   return;
  // }

  Global("a", ty.atomic<i32>(), ast::StorageClass::kWorkgroup);
  auto* multi_stmt =
      Ignore(Call("atomicCompareExchangeWeak", AddressOf("a"), 1, 2));
  auto* f = For(nullptr, nullptr, multi_stmt, Block(Return()));
  WrapInFunction(f);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(f)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  while (true) {
    return;
    int prev_value = 1;
    bool matched = atomic_compare_exchange_weak_explicit(&(a), &prev_value, 2, memory_order_relaxed, memory_order_relaxed);
    (void) int2(prev_value, matched);
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
  // for(var b = atomicCompareExchangeWeak(&a, 1, 2);
  //     atomicCompareExchangeWeak(&a, 1, 2).x == 0;
  //     ignore(atomicCompareExchangeWeak(&a, 1, 2))) {
  //   return;
  // }
  Global("a", ty.atomic<i32>(), ast::StorageClass::kWorkgroup);
  auto* multi_stmt_a = Call("atomicCompareExchangeWeak", AddressOf("a"), 1, 2);
  auto* multi_stmt_b = create<ast::BinaryExpression>(
      ast::BinaryOp::kEqual,
      MemberAccessor(Call("atomicCompareExchangeWeak", AddressOf("a"), 1, 2),
                     "x"),
      Expr(0));
  auto* multi_stmt_c =
      Ignore(Call("atomicCompareExchangeWeak", AddressOf("a"), 1, 2));
  auto* f = For(Decl(Var("b", nullptr, multi_stmt_a)), multi_stmt_b,
                multi_stmt_c, Block(Return()));
  WrapInFunction(f);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(f)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  {
    int prev_value = 1;
    bool matched = atomic_compare_exchange_weak_explicit(&(a), &prev_value, 2, memory_order_relaxed, memory_order_relaxed);
    int2 b = int2(prev_value, matched);
    while (true) {
      int prev_value_1 = 1;
      bool matched_1 = atomic_compare_exchange_weak_explicit(&(a), &prev_value_1, 2, memory_order_relaxed, memory_order_relaxed);
      if (!((int2(prev_value_1, matched_1).x == 0))) { break; }
      return;
      int prev_value_2 = 1;
      bool matched_2 = atomic_compare_exchange_weak_explicit(&(a), &prev_value_2, 2, memory_order_relaxed, memory_order_relaxed);
      (void) int2(prev_value_2, matched_2);
    }
  }
)");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
