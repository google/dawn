// Copyright 2021 The Tint Authors.
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
#include "src/writer/glsl/test_helper.h"

namespace tint {
namespace writer {
namespace glsl {
namespace {

using GlslGeneratorImplTest_Loop = TestHelper;

TEST_F(GlslGeneratorImplTest_Loop, Emit_Loop) {
  auto* body = Block(create<ast::DiscardStatement>());
  auto* continuing = Block();
  auto* l = Loop(body, continuing);

  WrapInFunction(l);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(l)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  while (true) {
    discard;
  }
)");
}

TEST_F(GlslGeneratorImplTest_Loop, Emit_LoopWithContinuing) {
  Func("a_statement", {}, ty.void_(), {});

  auto* body = Block(create<ast::DiscardStatement>());
  auto* continuing = Block(CallStmt(Call("a_statement")));
  auto* l = Loop(body, continuing);

  WrapInFunction(l);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(l)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  while (true) {
    discard;
    {
      a_statement();
    }
  }
)");
}

TEST_F(GlslGeneratorImplTest_Loop, Emit_LoopNestedWithContinuing) {
  Func("a_statement", {}, ty.void_(), {});

  Global("lhs", ty.f32(), ast::StorageClass::kPrivate);
  Global("rhs", ty.f32(), ast::StorageClass::kPrivate);

  auto* body = Block(create<ast::DiscardStatement>());
  auto* continuing = Block(CallStmt(Call("a_statement")));
  auto* inner = Loop(body, continuing);

  body = Block(inner);

  auto* lhs = Expr("lhs");
  auto* rhs = Expr("rhs");

  continuing = Block(Assign(lhs, rhs));

  auto* outer = Loop(body, continuing);
  WrapInFunction(outer);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(outer)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  while (true) {
    while (true) {
      discard;
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

TEST_F(GlslGeneratorImplTest_Loop, Emit_LoopWithVarUsedInContinuing) {
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

  auto* lhs = Expr("lhs");
  auto* rhs = Expr("rhs");

  auto* continuing = Block(Assign(lhs, rhs));
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

TEST_F(GlslGeneratorImplTest_Loop, Emit_ForLoop) {
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
  EXPECT_EQ(gen.result(), R"(  {
    for(; ; ) {
      a_statement();
    }
  }
)");
}

TEST_F(GlslGeneratorImplTest_Loop, Emit_ForLoopWithSimpleInit) {
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
  EXPECT_EQ(gen.result(), R"(  {
    for(int i = 0; ; ) {
      a_statement();
    }
  }
)");
}

TEST_F(GlslGeneratorImplTest_Loop, Emit_ForLoopWithMultiStmtInit) {
  // for(var b = true && false; ; ) {
  //   return;
  // }
  Func("a_statement", {}, ty.void_(), {});

  auto* multi_stmt = create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd,
                                                   Expr(true), Expr(false));
  auto* f = For(Decl(Var("b", nullptr, multi_stmt)), nullptr, nullptr,
                Block(CallStmt(Call("a_statement"))));
  WrapInFunction(f);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(f)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  {
    bool tint_tmp = true;
    if (tint_tmp) {
      tint_tmp = false;
    }
    bool b = (tint_tmp);
    for(; ; ) {
      a_statement();
    }
  }
)");
}

TEST_F(GlslGeneratorImplTest_Loop, Emit_ForLoopWithSimpleCond) {
  // for(; true; ) {
  //   return;
  // }

  Func("a_statement", {}, ty.void_(), {});

  auto* f = For(nullptr, true, nullptr, Block(CallStmt(Call("a_statement"))));
  WrapInFunction(f);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(f)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  {
    for(; true; ) {
      a_statement();
    }
  }
)");
}

TEST_F(GlslGeneratorImplTest_Loop, Emit_ForLoopWithMultiStmtCond) {
  // for(; true && false; ) {
  //   return;
  // }

  Func("a_statement", {}, ty.void_(), {});

  auto* multi_stmt = create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd,
                                                   Expr(true), Expr(false));
  auto* f =
      For(nullptr, multi_stmt, nullptr, Block(CallStmt(Call("a_statement"))));
  WrapInFunction(f);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(f)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  {
    while (true) {
      bool tint_tmp = true;
      if (tint_tmp) {
        tint_tmp = false;
      }
      if (!((tint_tmp))) { break; }
      a_statement();
    }
  }
)");
}

TEST_F(GlslGeneratorImplTest_Loop, Emit_ForLoopWithSimpleCont) {
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
  EXPECT_EQ(gen.result(), R"(  {
    for(; ; i = (i + 1)) {
      a_statement();
    }
  }
)");
}

TEST_F(GlslGeneratorImplTest_Loop, Emit_ForLoopWithMultiStmtCont) {
  // for(; ; i = true && false) {
  //   return;
  // }

  Func("a_statement", {}, ty.void_(), {});

  auto* multi_stmt = create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd,
                                                   Expr(true), Expr(false));
  auto* v = Decl(Var("i", ty.bool_()));
  auto* f = For(nullptr, nullptr, Assign("i", multi_stmt),
                Block(CallStmt(Call("a_statement"))));
  WrapInFunction(v, f);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(f)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  {
    while (true) {
      a_statement();
      bool tint_tmp = true;
      if (tint_tmp) {
        tint_tmp = false;
      }
      i = (tint_tmp);
    }
  }
)");
}

TEST_F(GlslGeneratorImplTest_Loop, Emit_ForLoopWithSimpleInitCondCont) {
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
  EXPECT_EQ(gen.result(), R"(  {
    for(int i = 0; true; i = (i + 1)) {
      a_statement();
    }
  }
)");
}

TEST_F(GlslGeneratorImplTest_Loop, Emit_ForLoopWithMultiStmtInitCondCont) {
  // for(var i = true && false; true && false; i = true && false) {
  //   return;
  // }
  Func("a_statement", {}, ty.void_(), {});

  auto* multi_stmt_a = create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd,
                                                     Expr(true), Expr(false));
  auto* multi_stmt_b = create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd,
                                                     Expr(true), Expr(false));
  auto* multi_stmt_c = create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd,
                                                     Expr(true), Expr(false));

  auto* f =
      For(Decl(Var("i", nullptr, multi_stmt_a)), multi_stmt_b,
          Assign("i", multi_stmt_c), Block(CallStmt(Call("a_statement"))));
  WrapInFunction(f);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(f)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  {
    bool tint_tmp = true;
    if (tint_tmp) {
      tint_tmp = false;
    }
    bool i = (tint_tmp);
    while (true) {
      bool tint_tmp_1 = true;
      if (tint_tmp_1) {
        tint_tmp_1 = false;
      }
      if (!((tint_tmp_1))) { break; }
      a_statement();
      bool tint_tmp_2 = true;
      if (tint_tmp_2) {
        tint_tmp_2 = false;
      }
      i = (tint_tmp_2);
    }
  }
)");
}

}  // namespace
}  // namespace glsl
}  // namespace writer
}  // namespace tint
