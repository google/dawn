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
#include "src/type/i32_type.h"

namespace tint {
namespace ast {
namespace {

using StructTest = TestHelper;

TEST_F(StructTest, Creation) {
  auto* s = create<Struct>(StructMemberList{Member("a", ty.i32())},
                           StructDecorationList{});
  EXPECT_EQ(s->members().size(), 1u);
  EXPECT_TRUE(s->decorations().empty());
  EXPECT_EQ(s->source().range.begin.line, 0u);
  EXPECT_EQ(s->source().range.begin.column, 0u);
  EXPECT_EQ(s->source().range.end.line, 0u);
  EXPECT_EQ(s->source().range.end.column, 0u);
}

TEST_F(StructTest, Creation_WithDecorations) {
  StructDecorationList decos;
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
  StructDecorationList decos;
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

TEST_F(StructTest, IsValid) {
  auto* s = create<Struct>(StructMemberList{}, StructDecorationList{});
  EXPECT_TRUE(s->IsValid());
}

TEST_F(StructTest, IsValid_Null_StructMember) {
  auto* s = create<Struct>(StructMemberList{Member("a", ty.i32()), nullptr},
                           StructDecorationList{});
  EXPECT_FALSE(s->IsValid());
}

TEST_F(StructTest, IsValid_Invalid_StructMember) {
  auto* s = create<Struct>(StructMemberList{Member("", ty.i32())},
                           ast::StructDecorationList{});
  EXPECT_FALSE(s->IsValid());
}

TEST_F(StructTest, ToStr) {
  StructDecorationList decos;
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
