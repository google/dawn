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
  Func("a_statement", {}, ty.void_(), {});

  auto* body = Block(create<ast::DiscardStatement>());
  auto* continuing = Block(CallStmt(Call("a_statement")));
  auto* l = Loop(body, continuing);
  WrapInFunction(l);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(l)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  while (true) {
    discard_fragment();
    {
      a_statement();
    }
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_LoopNestedWithContinuing) {
  Func("a_statement", {}, ty.void_(), {});

  Global("lhs", ty.f32(), ast::StorageClass::kPrivate);
  Global("rhs", ty.f32(), ast::StorageClass::kPrivate);

  auto* body = Block(create<ast::DiscardStatement>());
  auto* continuing = Block(CallStmt(Call("a_statement")));
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
        a_statement();
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

  Func("a_statement", {}, ty.void_(), {});

  auto* f =
      For(nullptr, nullptr, nullptr, Block(CallStmt(Call("a_statement"))));
  WrapInFunction(f);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(f)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  for(; ; ) {
    a_statement();
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_ForLoopWithSimpleInit) {
  // for(var i : i32; ; ) {
  //   return;
  // }

  Func("a_statement", {}, ty.void_(), {});

  auto* f = For(Decl(Var("i", ty.i32())), nullptr, nullptr,
                Block(CallStmt(Call("a_statement"))));
  WrapInFunction(f);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(f)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  for(int i = 0; ; ) {
    a_statement();
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_ForLoopWithMultiStmtInit) {
  // fn f(i : i32) {}
  //
  // var<workgroup> a : atomic<i32>;
  // for({f(1); f(2);}; ; ) {
  //   return;
  // }

  Func("f", {Param("i", ty.i32())}, ty.void_(), {});
  auto f = [&](auto&& expr) { return CallStmt(Call("f", expr)); };

  Func("a_statement", {}, ty.void_(), {});

  Global("a", ty.atomic<i32>(), ast::StorageClass::kWorkgroup);
  auto* multi_stmt = Block(f(1), f(2));
  auto* loop =
      For(multi_stmt, nullptr, nullptr, Block(CallStmt(Call("a_statement"))));
  WrapInFunction(loop);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(loop)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  {
    {
      f(1);
      f(2);
    }
    for(; ; ) {
      a_statement();
    }
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_ForLoopWithSimpleCond) {
  // for(; true; ) {
  //   return;
  // }

  Func("a_statement", {}, ty.void_(), {});

  auto* f = For(nullptr, true, nullptr, Block(CallStmt(Call("a_statement"))));
  WrapInFunction(f);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(f)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  for(; true; ) {
    a_statement();
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_ForLoopWithSimpleCont) {
  // for(; ; i = i + 1) {
  //   return;
  // }

  Func("a_statement", {}, ty.void_(), {});

  auto* v = Decl(Var("i", ty.i32()));
  auto* f = For(nullptr, nullptr, Assign("i", Add("i", 1)),
                Block(CallStmt(Call("a_statement"))));
  WrapInFunction(v, f);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(f)) << gen.error();
  EXPECT_EQ(
      gen.result(),
      R"(  for(; ; i = as_type<int>((as_type<uint>(i) + as_type<uint>(1)))) {
    a_statement();
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_ForLoopWithMultiStmtCont) {
  // fn f(i : i32) {}
  //
  // var<workgroup> a : atomic<i32>;
  // for(; ; { f(1); f(2); }) {
  //   return;
  // }

  Func("f", {Param("i", ty.i32())}, ty.void_(), {});
  auto f = [&](auto&& expr) { return CallStmt(Call("f", expr)); };

  Func("a_statement", {}, ty.void_(), {});

  Global("a", ty.atomic<i32>(), ast::StorageClass::kWorkgroup);
  auto* multi_stmt = Block(f(1), f(2));
  auto* loop =
      For(nullptr, nullptr, multi_stmt, Block(CallStmt(Call("a_statement"))));
  WrapInFunction(loop);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(loop)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  while (true) {
    a_statement();
    {
      f(1);
      f(2);
    }
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_ForLoopWithSimpleInitCondCont) {
  // for(var i : i32; true; i = i + 1) {
  //   return;
  // }

  Func("a_statement", {}, ty.void_(), {});

  auto* f = For(Decl(Var("i", ty.i32())), true, Assign("i", Add("i", 1)),
                Block(CallStmt(Call("a_statement"))));
  WrapInFunction(f);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(f)) << gen.error();
  EXPECT_EQ(
      gen.result(),
      R"(  for(int i = 0; true; i = as_type<int>((as_type<uint>(i) + as_type<uint>(1)))) {
    a_statement();
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_ForLoopWithMultiStmtInitCondCont) {
  // fn f(i : i32) {}
  //
  // var<workgroup> a : atomic<i32>;
  // for({ f(1); f(2); }; true; { f(3); f(4); }) {
  //   return;
  // }

  Func("f", {Param("i", ty.i32())}, ty.void_(), {});
  auto f = [&](auto&& expr) { return CallStmt(Call("f", expr)); };

  Func("a_statement", {}, ty.void_(), {});

  Global("a", ty.atomic<i32>(), ast::StorageClass::kWorkgroup);
  auto* multi_stmt_a = Block(f(1), f(2));
  auto* multi_stmt_b = Block(f(3), f(4));
  auto* loop = For(multi_stmt_a, Expr(true), multi_stmt_b,
                   Block(CallStmt(Call("a_statement"))));
  WrapInFunction(loop);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(loop)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  {
    {
      f(1);
      f(2);
    }
    while (true) {
      if (!(true)) { break; }
      a_statement();
      {
        f(3);
        f(4);
      }
    }
  }
)");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
