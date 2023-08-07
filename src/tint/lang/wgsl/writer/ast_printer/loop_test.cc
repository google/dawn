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

#include "src/tint/lang/wgsl/writer/ast_printer/helper_test.h"

#include "gmock/gmock.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::wgsl::writer {
namespace {

using WgslASTPrinterTest = TestHelper;

TEST_F(WgslASTPrinterTest, Emit_Loop) {
    auto* body = Block(Break());
    auto* continuing = Block();
    auto* l = Loop(body, continuing);

    Func("F", tint::Empty, ty.void_(), Vector{l}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    gen.EmitStatement(l);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  loop {
    break;
  }
)");
}

TEST_F(WgslASTPrinterTest, Emit_LoopWithContinuing) {
    Func("a_statement", {}, ty.void_(), {});

    auto* body = Block(Break());
    auto* continuing = Block(CallStmt(Call("a_statement")));
    auto* l = Loop(body, continuing);

    Func("F", tint::Empty, ty.void_(), Vector{l}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    gen.EmitStatement(l);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  loop {
    break;

    continuing {
      a_statement();
    }
  }
)");
}

TEST_F(WgslASTPrinterTest, Emit_LoopWithContinuing_BreakIf) {
    Func("a_statement", {}, ty.void_(), {});

    auto* body = Block(Discard());
    auto* continuing = Block(CallStmt(Call("a_statement")), BreakIf(true));
    auto* l = Loop(body, continuing);

    Func("F", tint::Empty, ty.void_(), Vector{l}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    gen.EmitStatement(l);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  loop {
    discard;

    continuing {
      a_statement();
      break if true;
    }
  }
)");
}

TEST_F(WgslASTPrinterTest, Emit_ForLoopWithMultiStmtInit) {
    // var<workgroup> a : atomic<i32>;
    // for({ignore(1i); ignore(2i);}; ; ) {
    //   return;
    // }
    GlobalVar("a", ty.atomic<i32>(), core::AddressSpace::kWorkgroup);
    auto* multi_stmt = Block(Ignore(1_i), Ignore(2_i));
    auto* f = For(multi_stmt, nullptr, nullptr, Block(Return()));
    WrapInFunction(f);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    gen.EmitStatement(f);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  for({
    _ = 1i;
    _ = 2i;
  }; ; ) {
    return;
  }
)");
}

TEST_F(WgslASTPrinterTest, Emit_ForLoopWithSimpleCond) {
    // for(; true; ) {
    //   return;
    // }

    auto* f = For(nullptr, true, nullptr, Block(Return()));
    WrapInFunction(f);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    gen.EmitStatement(f);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  for(; true; ) {
    return;
  }
)");
}

TEST_F(WgslASTPrinterTest, Emit_ForLoopWithSimpleCont) {
    // for(; ; i = i + 1i) {
    //   return;
    // }

    auto* v = Decl(Var("i", ty.i32()));
    auto* f = For(nullptr, nullptr, Assign("i", Add("i", 1_i)), Block(Return()));
    WrapInFunction(v, f);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    gen.EmitStatement(f);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  for(; ; i = (i + 1i)) {
    return;
  }
)");
}

TEST_F(WgslASTPrinterTest, Emit_ForLoopWithMultiStmtCont) {
    // var<workgroup> a : atomic<i32>;
    // for(; ; { ignore(1i); ignore(2i); }) {
    //   return;
    // }

    GlobalVar("a", ty.atomic<i32>(), core::AddressSpace::kWorkgroup);
    auto* multi_stmt = Block(Ignore(1_i), Ignore(2_i));
    auto* f = For(nullptr, nullptr, multi_stmt, Block(Return()));
    WrapInFunction(f);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    gen.EmitStatement(f);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  for(; ; {
    _ = 1i;
    _ = 2i;
  }) {
    return;
  }
)");
}

TEST_F(WgslASTPrinterTest, Emit_ForLoopWithSimpleInitCondCont) {
    // for(var i : i32; true; i = i + 1i) {
    //   return;
    // }

    auto* f = For(Decl(Var("i", ty.i32())), true, Assign("i", Add("i", 1_i)), Block(Return()));
    WrapInFunction(f);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    gen.EmitStatement(f);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  for(var i : i32; true; i = (i + 1i)) {
    return;
  }
)");
}

TEST_F(WgslASTPrinterTest, Emit_ForLoopWithMultiStmtInitCondCont) {
    // var<workgroup> a : atomic<i32>;
    // for({ ignore(1i); ignore(2i); }; true; { ignore(3i); ignore(4i); }) {
    //   return;
    // }
    GlobalVar("a", ty.atomic<i32>(), core::AddressSpace::kWorkgroup);
    auto* multi_stmt_a = Block(Ignore(1_i), Ignore(2_i));
    auto* multi_stmt_b = Block(Ignore(3_i), Ignore(4_i));
    auto* f = For(multi_stmt_a, Expr(true), multi_stmt_b, Block(Return()));
    WrapInFunction(f);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    gen.EmitStatement(f);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  for({
    _ = 1i;
    _ = 2i;
  }; true; {
    _ = 3i;
    _ = 4i;
  }) {
    return;
  }
)");
}

TEST_F(WgslASTPrinterTest, Emit_While) {
    // while(true) {
    //   return;
    // }

    auto* f = While(Expr(true), Block(Return()));
    WrapInFunction(f);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    gen.EmitStatement(f);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  while(true) {
    return;
  }
)");
}

TEST_F(WgslASTPrinterTest, Emit_While_WithContinue) {
    // while(true) {
    //   continue;
    // }

    auto* f = While(Expr(true), Block(Continue()));
    WrapInFunction(f);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    gen.EmitStatement(f);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  while(true) {
    continue;
  }
)");
}

TEST_F(WgslASTPrinterTest, Emit_WhileMultiCond) {
    // while(true && false) {
    //   return;
    // }

    auto* multi_stmt =
        create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, Expr(true), Expr(false));
    auto* f = While(multi_stmt, Block(Return()));
    WrapInFunction(f);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    gen.EmitStatement(f);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  while((true && false)) {
    return;
  }
)");
}

}  // namespace
}  // namespace tint::wgsl::writer
