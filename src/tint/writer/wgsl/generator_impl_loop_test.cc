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

#include "src/tint/writer/wgsl/test_helper.h"

namespace tint::writer::wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, Emit_Loop) {
    auto* body = Block(create<ast::DiscardStatement>());
    auto* continuing = Block();
    auto* l = Loop(body, continuing);

    WrapInFunction(l);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitStatement(l)) << gen.error();
    EXPECT_EQ(gen.result(), R"(  loop {
    discard;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_LoopWithContinuing) {
    Func("a_statement", {}, ty.void_(), {});

    auto* body = Block(create<ast::DiscardStatement>());
    auto* continuing = Block(CallStmt(Call("a_statement")));
    auto* l = Loop(body, continuing);

    WrapInFunction(l);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitStatement(l)) << gen.error();
    EXPECT_EQ(gen.result(), R"(  loop {
    discard;

    continuing {
      a_statement();
    }
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_ForLoopWithMultiStmtInit) {
    // var<workgroup> a : atomic<i32>;
    // for({ignore(1); ignore(2);}; ; ) {
    //   return;
    // }
    Global("a", ty.atomic<i32>(), ast::StorageClass::kWorkgroup);
    auto* multi_stmt = Block(Ignore(1), Ignore(2));
    auto* f = For(multi_stmt, nullptr, nullptr, Block(Return()));
    WrapInFunction(f);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitStatement(f)) << gen.error();
    EXPECT_EQ(gen.result(), R"(  for({
    _ = 1;
    _ = 2;
  }; ; ) {
    return;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_ForLoopWithSimpleCond) {
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

TEST_F(WgslGeneratorImplTest, Emit_ForLoopWithSimpleCont) {
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

TEST_F(WgslGeneratorImplTest, Emit_ForLoopWithMultiStmtCont) {
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
    EXPECT_EQ(gen.result(), R"(  for(; ; {
    _ = 1;
    _ = 2;
  }) {
    return;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_ForLoopWithSimpleInitCondCont) {
    // for(var i : i32; true; i = i + 1) {
    //   return;
    // }

    auto* f = For(Decl(Var("i", ty.i32())), true, Assign("i", Add("i", 1)), Block(Return()));
    WrapInFunction(f);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitStatement(f)) << gen.error();
    EXPECT_EQ(gen.result(), R"(  for(var i : i32; true; i = (i + 1)) {
    return;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_ForLoopWithMultiStmtInitCondCont) {
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
    EXPECT_EQ(gen.result(), R"(  for({
    _ = 1;
    _ = 2;
  }; true; {
    _ = 3;
    _ = 4;
  }) {
    return;
  }
)");
}

}  // namespace
}  // namespace tint::writer::wgsl
