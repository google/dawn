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

#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/writer/glsl/test_helper.h"

#include "gmock/gmock.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::glsl {
namespace {

using GlslGeneratorImplTest_Loop = TestHelper;

TEST_F(GlslGeneratorImplTest_Loop, Emit_Loop) {
    auto* body = Block(Break());
    auto* continuing = Block();
    auto* l = Loop(body, continuing);

    Func("F", utils::Empty, ty.void_(), utils::Vector{l},
         utils::Vector{Stage(ast::PipelineStage::kFragment)});

    GeneratorImpl& gen = Build();

    gen.increment_indent();
    gen.EmitStatement(l);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(  while (true) {
    break;
  }
)");
}

TEST_F(GlslGeneratorImplTest_Loop, Emit_LoopWithContinuing) {
    Func("a_statement", {}, ty.void_(), {});

    auto* body = Block(Break());
    auto* continuing = Block(CallStmt(Call("a_statement")));
    auto* l = Loop(body, continuing);

    Func("F", utils::Empty, ty.void_(), utils::Vector{l},
         utils::Vector{Stage(ast::PipelineStage::kFragment)});

    GeneratorImpl& gen = Build();

    gen.increment_indent();
    gen.EmitStatement(l);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(  while (true) {
    break;
    {
      a_statement();
    }
  }
)");
}

TEST_F(GlslGeneratorImplTest_Loop, Emit_LoopWithContinuing_BreakIf) {
    Func("a_statement", {}, ty.void_(), {});

    auto* body = Block(Break());
    auto* continuing = Block(CallStmt(Call("a_statement")), BreakIf(true));
    auto* l = Loop(body, continuing);

    Func("F", utils::Empty, ty.void_(), utils::Vector{l},
         utils::Vector{Stage(ast::PipelineStage::kFragment)});

    GeneratorImpl& gen = Build();

    gen.increment_indent();
    gen.EmitStatement(l);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(  while (true) {
    break;
    {
      a_statement();
      if (true) { break; }
    }
  }
)");
}

TEST_F(GlslGeneratorImplTest_Loop, Emit_LoopNestedWithContinuing) {
    Func("a_statement", {}, ty.void_(), {});

    GlobalVar("lhs", ty.f32(), builtin::AddressSpace::kPrivate);
    GlobalVar("rhs", ty.f32(), builtin::AddressSpace::kPrivate);

    auto* body = Block(Break());
    auto* continuing = Block(CallStmt(Call("a_statement")));
    auto* inner = Loop(body, continuing);

    body = Block(inner);

    auto* lhs = Expr("lhs");
    auto* rhs = Expr("rhs");

    continuing = Block(Assign(lhs, rhs), BreakIf(true));

    auto* outer = Loop(body, continuing);

    Func("F", utils::Empty, ty.void_(), utils::Vector{outer},
         utils::Vector{Stage(ast::PipelineStage::kFragment)});

    GeneratorImpl& gen = Build();

    gen.increment_indent();
    gen.EmitStatement(outer);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(  while (true) {
    while (true) {
      break;
      {
        a_statement();
      }
    }
    {
      lhs = rhs;
      if (true) { break; }
    }
  }
)");
}

TEST_F(GlslGeneratorImplTest_Loop, Emit_LoopWithVarUsedInContinuing) {
    // loop {
    //   var lhs : f32 = 2.5;
    //   var other : f32;
    //   break;
    //   continuing {
    //     lhs = rhs
    //   }
    // }

    GlobalVar("rhs", ty.f32(), builtin::AddressSpace::kPrivate);

    auto* body = Block(Decl(Var("lhs", ty.f32(), Expr(2.5_f))),  //
                       Decl(Var("other", ty.f32())),             //
                       Break());
    auto* continuing = Block(Assign("lhs", "rhs"));
    auto* outer = Loop(body, continuing);
    WrapInFunction(outer);

    GeneratorImpl& gen = Build();

    gen.increment_indent();
    gen.EmitStatement(outer);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(  while (true) {
    float lhs = 2.5f;
    float other = 0.0f;
    break;
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

    auto* f = For(nullptr, nullptr, nullptr,  //
                  Block(Return()));
    WrapInFunction(f);

    GeneratorImpl& gen = Build();

    gen.increment_indent();
    gen.EmitStatement(f);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(  {
    for(; ; ) {
      return;
    }
  }
)");
}

TEST_F(GlslGeneratorImplTest_Loop, Emit_ForLoopWithSimpleInit) {
    // for(var i : i32; ; ) {
    //   return;
    // }

    auto* f = For(Decl(Var("i", ty.i32())), nullptr, nullptr,  //
                  Block(Return()));
    WrapInFunction(f);

    GeneratorImpl& gen = Build();

    gen.increment_indent();
    gen.EmitStatement(f);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(  {
    for(int i = 0; ; ) {
      return;
    }
  }
)");
}

TEST_F(GlslGeneratorImplTest_Loop, Emit_ForLoopWithMultiStmtInit) {
    // let t = true;
    // for(var b = t && false; ; ) {
    //   return;
    // }

    auto* t = Let("t", Expr(true));
    auto* multi_stmt = LogicalAnd(t, false);
    auto* f = For(Decl(Var("b", multi_stmt)), nullptr, nullptr, Block(Return()));
    WrapInFunction(t, f);

    GeneratorImpl& gen = Build();

    gen.increment_indent();
    gen.EmitStatement(f);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(  {
    bool tint_tmp = t;
    if (tint_tmp) {
      tint_tmp = false;
    }
    bool b = (tint_tmp);
    for(; ; ) {
      return;
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
    gen.EmitStatement(f);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(  {
    for(; true; ) {
      a_statement();
    }
  }
)");
}

TEST_F(GlslGeneratorImplTest_Loop, Emit_ForLoopWithMultiStmtCond) {
    // let t = true;
    // for(; t && false; ) {
    //   return;
    // }

    Func("a_statement", {}, ty.void_(), {});
    auto* t = Let("t", Expr(true));
    auto* multi_stmt = LogicalAnd(t, false);
    auto* f = For(nullptr, multi_stmt, nullptr, Block(CallStmt(Call("a_statement"))));
    WrapInFunction(t, f);

    GeneratorImpl& gen = Build();

    gen.increment_indent();
    gen.EmitStatement(f);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(  {
    while (true) {
      bool tint_tmp = t;
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
    // for(; ; i = i + 1i) {
    //   return;
    // }

    auto* v = Decl(Var("i", ty.i32()));
    auto* f = For(nullptr, nullptr, Assign("i", Add("i", 1_i)),  //
                  Block(Return()));
    WrapInFunction(v, f);

    GeneratorImpl& gen = Build();

    gen.increment_indent();
    gen.EmitStatement(f);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(  {
    for(; ; i = (i + 1)) {
      return;
    }
  }
)");
}

TEST_F(GlslGeneratorImplTest_Loop, Emit_ForLoopWithMultiStmtCont) {
    // let t = true;
    // for(; ; i = t && false) {
    //   return;
    // }

    auto* t = Let("t", Expr(true));
    auto* multi_stmt = LogicalAnd(t, false);
    auto* v = Decl(Var("i", ty.bool_()));
    auto* f = For(nullptr, nullptr, Assign("i", multi_stmt),  //
                  Block(Return()));
    WrapInFunction(t, v, f);

    GeneratorImpl& gen = Build();

    gen.increment_indent();
    gen.EmitStatement(f);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(  {
    while (true) {
      return;
      bool tint_tmp = t;
      if (tint_tmp) {
        tint_tmp = false;
      }
      i = (tint_tmp);
    }
  }
)");
}

TEST_F(GlslGeneratorImplTest_Loop, Emit_ForLoopWithSimpleInitCondCont) {
    // for(var i : i32; true; i = ii + 1) {
    //   return;
    // }

    auto* f = For(Decl(Var("i", ty.i32())), true, Assign("i", Add("i", 1_i)), Block(Return()));
    WrapInFunction(f);

    GeneratorImpl& gen = Build();

    gen.increment_indent();
    gen.EmitStatement(f);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(  {
    for(int i = 0; true; i = (i + 1)) {
      return;
    }
  }
)");
}

TEST_F(GlslGeneratorImplTest_Loop, Emit_ForLoopWithMultiStmtInitCondCont) {
    // let t = true;
    // for(var i = t && false; t && false; i = t && false) {
    //   return;
    // }

    auto* t = Let("t", Expr(true));
    auto* multi_stmt_a = LogicalAnd(t, false);
    auto* multi_stmt_b = LogicalAnd(t, false);
    auto* multi_stmt_c = LogicalAnd(t, false);

    auto* f = For(Decl(Var("i", multi_stmt_a)), multi_stmt_b, Assign("i", multi_stmt_c),  //
                  Block(Return()));
    WrapInFunction(t, f);

    GeneratorImpl& gen = Build();

    gen.increment_indent();
    gen.EmitStatement(f);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(  {
    bool tint_tmp = t;
    if (tint_tmp) {
      tint_tmp = false;
    }
    bool i = (tint_tmp);
    while (true) {
      bool tint_tmp_1 = t;
      if (tint_tmp_1) {
        tint_tmp_1 = false;
      }
      if (!((tint_tmp_1))) { break; }
      return;
      bool tint_tmp_2 = t;
      if (tint_tmp_2) {
        tint_tmp_2 = false;
      }
      i = (tint_tmp_2);
    }
  }
)");
}

TEST_F(GlslGeneratorImplTest_Loop, Emit_While) {
    // while(true) {
    //   return;
    // }

    auto* f = While(Expr(true), Block(Return()));
    WrapInFunction(f);

    GeneratorImpl& gen = Build();

    gen.increment_indent();
    gen.EmitStatement(f);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(  while(true) {
    return;
  }
)");
}

TEST_F(GlslGeneratorImplTest_Loop, Emit_While_WithContinue) {
    // while(true) {
    //   continue;
    // }

    auto* f = While(Expr(true), Block(Continue()));
    WrapInFunction(f);

    GeneratorImpl& gen = Build();

    gen.increment_indent();
    gen.EmitStatement(f);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(  while(true) {
    continue;
  }
)");
}

TEST_F(GlslGeneratorImplTest_Loop, Emit_WhileWithMultiStmtCond) {
    // let t = true;
    // while(t && false) {
    //   return;
    // }

    Func("a_statement", {}, ty.void_(), {});

    auto* t = Let("t", Expr(true));
    auto* multi_stmt = LogicalAnd(t, false);
    auto* f = While(multi_stmt, Block(CallStmt(Call("a_statement"))));
    WrapInFunction(t, f);

    GeneratorImpl& gen = Build();

    gen.increment_indent();
    gen.EmitStatement(f);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(  while (true) {
    bool tint_tmp = t;
    if (tint_tmp) {
      tint_tmp = false;
    }
    if (!((tint_tmp))) { break; }
    a_statement();
  }
)");
}

}  // namespace
}  // namespace tint::writer::glsl
