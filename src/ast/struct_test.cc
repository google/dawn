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

#include "src/ast/struct.h"
#include "gtest/gtest-spi.h"
#include "src/ast/alias.h"
#include "src/ast/array.h"
#include "src/ast/bool.h"
#include "src/ast/f32.h"
#include "src/ast/i32.h"
#include "src/ast/matrix.h"
#include "src/ast/pointer.h"
#include "src/ast/sampler.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/test_helper.h"
#include "src/ast/texture.h"
#include "src/ast/u32.h"
#include "src/ast/vector.h"

namespace tint {
namespace ast {
namespace {

using AstStructTest = TestHelper;

TEST_F(AstStructTest, Creation) {
  auto name = Sym("s");
  auto* s = create<Struct>(name, StructMemberList{Member("a", ty.i32())},
                           DecorationList{});
  EXPECT_EQ(s->name(), name);
  EXPECT_EQ(s->members().size(), 1u);
  EXPECT_TRUE(s->decorations().empty());
  EXPECT_EQ(s->source().range.begin.line, 0u);
  EXPECT_EQ(s->source().range.begin.column, 0u);
  EXPECT_EQ(s->source().range.end.line, 0u);
  EXPECT_EQ(s->source().range.end.column, 0u);
}

TEST_F(AstStructTest, Creation_WithDecorations) {
  auto name = Sym("s");
  DecorationList decos;
  decos.push_back(create<StructBlockDecoration>());

  auto* s =
      create<Struct>(name, StructMemberList{Member("a", ty.i32())}, decos);
  EXPECT_EQ(s->name(), name);
  EXPECT_EQ(s->members().size(), 1u);
  ASSERT_EQ(s->decorations().size(), 1u);
  EXPECT_TRUE(s->decorations()[0]->Is<StructBlockDecoration>());
  EXPECT_EQ(s->source().range.begin.line, 0u);
  EXPECT_EQ(s->source().range.begin.column, 0u);
  EXPECT_EQ(s->source().range.end.line, 0u);
  EXPECT_EQ(s->source().range.end.column, 0u);
}

TEST_F(AstStructTest, CreationWithSourceAndDecorations) {
  auto name = Sym("s");
  auto* s = create<Struct>(
      Source{Source::Range{Source::Location{27, 4}, Source::Location{27, 8}}},
      name, StructMemberList{Member("a", ty.i32())},
      DecorationList{create<StructBlockDecoration>()});
  EXPECT_EQ(s->name(), name);
  EXPECT_EQ(s->members().size(), 1u);
  ASSERT_EQ(s->decorations().size(), 1u);
  EXPECT_TRUE(s->decorations()[0]->Is<StructBlockDecoration>());
  EXPECT_EQ(s->source().range.begin.line, 27u);
  EXPECT_EQ(s->source().range.begin.column, 4u);
  EXPECT_EQ(s->source().range.end.line, 27u);
  EXPECT_EQ(s->source().range.end.column, 8u);
}

TEST_F(AstStructTest, Assert_Null_StructMember) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b;
        b.create<Struct>(b.Sym("S"),
                         StructMemberList{b.Member("a", b.ty.i32()), nullptr},
                         DecorationList{});
      },
      "internal compiler error");
}

TEST_F(AstStructTest, Assert_Null_Decoration) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b;
        b.create<Struct>(b.Sym("S"),
                         StructMemberList{b.Member("a", b.ty.i32())},
                         DecorationList{nullptr});
      },
      "internal compiler error");
}

TEST_F(AstStructTest, Assert_DifferentProgramID_StructMember) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b1;
        ProgramBuilder b2;
        b1.create<Struct>(b1.Sym("S"),
                          StructMemberList{b2.Member("a", b2.ty.i32())},
                          DecorationList{});
      },
      "internal compiler error");
}

TEST_F(AstStructTest, Assert_DifferentProgramID_Decoration) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b1;
        ProgramBuilder b2;
        b1.create<Struct>(b1.Sym("S"),
                          StructMemberList{b1.Member("a", b1.ty.i32())},
                          DecorationList{b2.create<StructBlockDecoration>()});
      },
      "internal compiler error");
}

TEST_F(AstStructTest, ToStr) {
  auto* s = create<Struct>(Sym("S"), StructMemberList{Member("a", ty.i32())},
                           DecorationList{create<StructBlockDecoration>()});

  EXPECT_EQ(str(s), R"(Struct S {
  [[block]]
  StructMember{a: __i32}
}
)");
}

TEST_F(AstStructTest, TypeName) {
  auto name = Sym("my_struct");
  auto* s =
      create<ast::Struct>(name, ast::StructMemberList{}, ast::DecorationList{});
  EXPECT_EQ(s->type_name(), "__struct_$1");
}

TEST_F(AstStructTest, FriendlyName) {
  auto name = Sym("my_struct");
  auto* s =
      create<ast::Struct>(name, ast::StructMemberList{}, ast::DecorationList{});
  EXPECT_EQ(s->FriendlyName(Symbols()), "my_struct");
}

}  // namespace
}  // namespace ast
}  // namespace tint
