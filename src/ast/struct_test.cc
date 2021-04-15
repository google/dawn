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

#include "gtest/gtest-spi.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/test_helper.h"

namespace tint {
namespace ast {
namespace {

using StructTest = TestHelper;

TEST_F(StructTest, Creation) {
  auto* s =
      create<Struct>(StructMemberList{Member("a", ty.i32())}, DecorationList{});
  EXPECT_EQ(s->members().size(), 1u);
  EXPECT_TRUE(s->decorations().empty());
  EXPECT_EQ(s->source().range.begin.line, 0u);
  EXPECT_EQ(s->source().range.begin.column, 0u);
  EXPECT_EQ(s->source().range.end.line, 0u);
  EXPECT_EQ(s->source().range.end.column, 0u);
}

TEST_F(StructTest, Creation_WithDecorations) {
  DecorationList decos;
  decos.push_back(create<StructBlockDecoration>());

  auto* s = create<Struct>(StructMemberList{Member("a", ty.i32())}, decos);
  EXPECT_EQ(s->members().size(), 1u);
  ASSERT_EQ(s->decorations().size(), 1u);
  EXPECT_TRUE(s->decorations()[0]->Is<StructBlockDecoration>());
  EXPECT_EQ(s->source().range.begin.line, 0u);
  EXPECT_EQ(s->source().range.begin.column, 0u);
  EXPECT_EQ(s->source().range.end.line, 0u);
  EXPECT_EQ(s->source().range.end.column, 0u);
}

TEST_F(StructTest, CreationWithSourceAndDecorations) {
  DecorationList decos;
  decos.push_back(create<StructBlockDecoration>());

  auto* s = create<Struct>(
      Source{Source::Range{Source::Location{27, 4}, Source::Location{27, 8}}},
      StructMemberList{Member("a", ty.i32())}, decos);
  EXPECT_EQ(s->members().size(), 1u);
  ASSERT_EQ(s->decorations().size(), 1u);
  EXPECT_TRUE(s->decorations()[0]->Is<StructBlockDecoration>());
  EXPECT_EQ(s->source().range.begin.line, 27u);
  EXPECT_EQ(s->source().range.begin.column, 4u);
  EXPECT_EQ(s->source().range.end.line, 27u);
  EXPECT_EQ(s->source().range.end.column, 8u);
}

TEST_F(StructTest, Assert_Null_StructMember) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b;
        b.create<Struct>(StructMemberList{b.Member("a", b.ty.i32()), nullptr},
                         DecorationList{});
      },
      "internal compiler error");
}

TEST_F(StructTest, Assert_Null_Decoration) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b;
        b.create<Struct>(StructMemberList{b.Member("a", b.ty.i32())},
                         DecorationList{nullptr});
      },
      "internal compiler error");
}

TEST_F(StructTest, Assert_DifferentProgramID_StructMember) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b1;
        ProgramBuilder b2;
        b1.create<Struct>(StructMemberList{b2.Member("a", b2.ty.i32())},
                          DecorationList{});
      },
      "internal compiler error");
}

TEST_F(StructTest, Assert_DifferentProgramID_Decoration) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b1;
        ProgramBuilder b2;
        b1.create<Struct>(StructMemberList{b1.Member("a", b1.ty.i32())},
                          DecorationList{b2.create<StructBlockDecoration>()});
      },
      "internal compiler error");
}

TEST_F(StructTest, ToStr) {
  DecorationList decos;
  decos.push_back(create<StructBlockDecoration>());
  auto* s = create<Struct>(StructMemberList{Member("a", ty.i32())}, decos);

  EXPECT_EQ(str(s), R"(Struct{
  [[block]]
  StructMember{a: __i32}
}
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
