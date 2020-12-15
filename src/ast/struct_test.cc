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

#include <memory>
#include <sstream>
#include <utility>

#include "src/ast/struct_block_decoration.h"
#include "src/ast/struct_member.h"
#include "src/ast/test_helper.h"
#include "src/ast/type/i32_type.h"

namespace tint {
namespace ast {
namespace {

using StructTest = TestHelper;

TEST_F(StructTest, Creation) {
  StructMemberList members;
  members.push_back(create<StructMember>(mod->RegisterSymbol("a"), "a", ty.i32,
                                         StructMemberDecorationList{}));

  auto* s = create<Struct>(members, ast::StructDecorationList{});
  EXPECT_EQ(s->members().size(), 1u);
  EXPECT_TRUE(s->decorations().empty());
  EXPECT_EQ(s->source().range.begin.line, 0u);
  EXPECT_EQ(s->source().range.begin.column, 0u);
  EXPECT_EQ(s->source().range.end.line, 0u);
  EXPECT_EQ(s->source().range.end.column, 0u);
}  // namespace

TEST_F(StructTest, Creation_WithDecorations) {
  StructMemberList members;
  members.push_back(create<StructMember>(mod->RegisterSymbol("a"), "a", ty.i32,
                                         StructMemberDecorationList{}));

  StructDecorationList decos;
  decos.push_back(create<StructBlockDecoration>());

  auto* s = create<Struct>(members, decos);
  EXPECT_EQ(s->members().size(), 1u);
  ASSERT_EQ(s->decorations().size(), 1u);
  EXPECT_TRUE(s->decorations()[0]->Is<StructBlockDecoration>());
  EXPECT_EQ(s->source().range.begin.line, 0u);
  EXPECT_EQ(s->source().range.begin.column, 0u);
  EXPECT_EQ(s->source().range.end.line, 0u);
  EXPECT_EQ(s->source().range.end.column, 0u);
}

TEST_F(StructTest, CreationWithSourceAndDecorations) {
  StructMemberList members;
  members.emplace_back(create<StructMember>(
      mod->RegisterSymbol("a"), "a", ty.i32, StructMemberDecorationList{}));

  StructDecorationList decos;
  decos.push_back(create<StructBlockDecoration>());

  auto* s = create<Struct>(
      Source{Source::Range{Source::Location{27, 4}, Source::Location{27, 8}}},
      members, decos);
  EXPECT_EQ(s->members().size(), 1u);
  ASSERT_EQ(s->decorations().size(), 1u);
  EXPECT_TRUE(s->decorations()[0]->Is<StructBlockDecoration>());
  EXPECT_EQ(s->source().range.begin.line, 27u);
  EXPECT_EQ(s->source().range.begin.column, 4u);
  EXPECT_EQ(s->source().range.end.line, 27u);
  EXPECT_EQ(s->source().range.end.column, 8u);
}

TEST_F(StructTest, IsValid) {
  auto* s = create<Struct>(StructMemberList{}, StructDecorationList{});
  EXPECT_TRUE(s->IsValid());
}

TEST_F(StructTest, IsValid_Null_StructMember) {
  StructMemberList members;
  members.push_back(create<StructMember>(mod->RegisterSymbol("a"), "a", ty.i32,
                                         StructMemberDecorationList{}));
  members.push_back(nullptr);

  auto* s = create<Struct>(members, ast::StructDecorationList{});
  EXPECT_FALSE(s->IsValid());
}  // namespace ast

TEST_F(StructTest, IsValid_Invalid_StructMember) {
  StructMemberList members;
  members.push_back(create<StructMember>(mod->RegisterSymbol(""), "", ty.i32,
                                         StructMemberDecorationList{}));

  auto* s = create<Struct>(members, ast::StructDecorationList{});
  EXPECT_FALSE(s->IsValid());
}  // namespace tint

TEST_F(StructTest, ToStr) {
  StructMemberList members;
  members.emplace_back(create<StructMember>(
      mod->RegisterSymbol("a"), "a", ty.i32, StructMemberDecorationList{}));

  StructDecorationList decos;
  decos.push_back(create<StructBlockDecoration>());

  auto* s = create<Struct>(members, decos);

  std::ostringstream out;
  s->to_str(out, 2);
  EXPECT_EQ(demangle(out.str()), R"(Struct{
    [[block]]
    StructMember{a: __i32}
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
