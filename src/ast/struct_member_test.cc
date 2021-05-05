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
#include "src/ast/test_helper.h"

namespace tint {
namespace ast {
namespace {

using StructMemberTest = TestHelper;

TEST_F(StructMemberTest, Creation) {
  auto* st = Member("a", ty.i32(), {MemberSize(4)});
  EXPECT_EQ(st->symbol(), Symbol(1, ID()));
  EXPECT_TRUE(st->type()->Is<ast::I32>());
  EXPECT_EQ(st->decorations().size(), 1u);
  EXPECT_TRUE(st->decorations()[0]->Is<StructMemberSizeDecoration>());
  EXPECT_EQ(st->source().range.begin.line, 0u);
  EXPECT_EQ(st->source().range.begin.column, 0u);
  EXPECT_EQ(st->source().range.end.line, 0u);
  EXPECT_EQ(st->source().range.end.column, 0u);
}

TEST_F(StructMemberTest, CreationWithSource) {
  auto* st = Member(
      Source{Source::Range{Source::Location{27, 4}, Source::Location{27, 8}}},
      "a", ty.i32());
  EXPECT_EQ(st->symbol(), Symbol(1, ID()));
  EXPECT_TRUE(st->type()->Is<ast::I32>());
  EXPECT_EQ(st->decorations().size(), 0u);
  EXPECT_EQ(st->source().range.begin.line, 27u);
  EXPECT_EQ(st->source().range.begin.column, 4u);
  EXPECT_EQ(st->source().range.end.line, 27u);
  EXPECT_EQ(st->source().range.end.column, 8u);
}

TEST_F(StructMemberTest, Assert_Empty_Symbol) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b;
        b.Member("", b.ty.i32());
      },
      "internal compiler error");
}

TEST_F(StructMemberTest, Assert_Null_Type) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b;
        b.Member("a", nullptr);
      },
      "internal compiler error");
}

TEST_F(StructMemberTest, Assert_Null_Decoration) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b;
        b.Member("a", b.ty.i32(), {b.MemberSize(4), nullptr});
      },
      "internal compiler error");
}

TEST_F(StructMemberTest, Assert_DifferentProgramID_Symbol) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b1;
        ProgramBuilder b2;
        b1.Member(b2.Sym("a"), b1.ty.i32(), {b1.MemberSize(4)});
      },
      "internal compiler error");
}

TEST_F(StructMemberTest, Assert_DifferentProgramID_Decoration) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b1;
        ProgramBuilder b2;
        b1.Member("a", b1.ty.i32(), {b2.MemberSize(4)});
      },
      "internal compiler error");
}

TEST_F(StructMemberTest, ToStr) {
  auto* st = Member("a", ty.i32(), {MemberSize(4)});
  EXPECT_EQ(str(st), "StructMember{[[ size 4 ]] a: __i32}\n");
}

TEST_F(StructMemberTest, ToStrNoDecorations) {
  auto* st = Member("a", ty.i32());
  EXPECT_EQ(str(st), "StructMember{a: __i32}\n");
}

}  // namespace
}  // namespace ast
}  // namespace tint
