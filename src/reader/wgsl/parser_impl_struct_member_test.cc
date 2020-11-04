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

#include "gtest/gtest.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/type/i32_type.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"
#include "src/type_manager.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, StructMember_Parses) {
  auto* i32 = tm()->Get(std::make_unique<ast::type::I32Type>());

  auto* p = parser("a : i32;");
  auto decos = p->decoration_list();
  EXPECT_EQ(decos.size(), 0u);
  auto m = p->struct_member(decos);
  ASSERT_FALSE(p->has_error());
  ASSERT_NE(m, nullptr);

  EXPECT_EQ(m->name(), "a");
  EXPECT_EQ(m->type(), i32);
  EXPECT_EQ(m->decorations().size(), 0u);

  ASSERT_EQ(m->source().range.begin.line, 1u);
  ASSERT_EQ(m->source().range.begin.column, 1u);
  ASSERT_EQ(m->source().range.end.line, 1u);
  ASSERT_EQ(m->source().range.end.column, 2u);
}

TEST_F(ParserImplTest, StructMember_ParsesWithDecoration) {
  auto* i32 = tm()->Get(std::make_unique<ast::type::I32Type>());

  auto* p = parser("[[offset(2)]] a : i32;");
  auto decos = p->decoration_list();
  EXPECT_EQ(decos.size(), 1u);
  auto m = p->struct_member(decos);
  ASSERT_FALSE(p->has_error());
  ASSERT_NE(m, nullptr);

  EXPECT_EQ(m->name(), "a");
  EXPECT_EQ(m->type(), i32);
  EXPECT_EQ(m->decorations().size(), 1u);
  EXPECT_TRUE(m->decorations()[0]->IsOffset());
  EXPECT_EQ(m->decorations()[0]->AsOffset()->offset(), 2u);

  ASSERT_EQ(m->source().range.begin.line, 1u);
  ASSERT_EQ(m->source().range.begin.column, 15u);
  ASSERT_EQ(m->source().range.end.line, 1u);
  ASSERT_EQ(m->source().range.end.column, 16u);
}

TEST_F(ParserImplTest, StructMember_ParsesWithMultipleDecorations) {
  auto* i32 = tm()->Get(std::make_unique<ast::type::I32Type>());

  auto* p = parser(R"([[offset(2)]]
[[offset(4)]] a : i32;)");
  auto decos = p->decoration_list();
  EXPECT_EQ(decos.size(), 2u);
  auto m = p->struct_member(decos);
  ASSERT_FALSE(p->has_error());
  ASSERT_NE(m, nullptr);

  EXPECT_EQ(m->name(), "a");
  EXPECT_EQ(m->type(), i32);
  EXPECT_EQ(m->decorations().size(), 2u);
  EXPECT_TRUE(m->decorations()[0]->IsOffset());
  EXPECT_EQ(m->decorations()[0]->AsOffset()->offset(), 2u);
  EXPECT_TRUE(m->decorations()[1]->IsOffset());
  EXPECT_EQ(m->decorations()[1]->AsOffset()->offset(), 4u);

  ASSERT_EQ(m->source().range.begin.line, 2u);
  ASSERT_EQ(m->source().range.begin.column, 15u);
  ASSERT_EQ(m->source().range.end.line, 2u);
  ASSERT_EQ(m->source().range.end.column, 16u);
}

TEST_F(ParserImplTest, StructMember_InvalidDecoration) {
  auto* p = parser("[[offset(nan)]] a : i32;");
  auto decos = p->decoration_list();
  auto m = p->struct_member(decos);
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(m, nullptr);
  EXPECT_EQ(p->error(),
            "1:10: expected signed integer literal for offset decoration");
}

TEST_F(ParserImplTest, StructMember_InvalidVariable) {
  auto* p = parser("[[offset(4)]] a : B;");
  auto decos = p->decoration_list();
  auto m = p->struct_member(decos);
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(m, nullptr);
  EXPECT_EQ(p->error(), "1:19: unknown constructed type 'B'");
}

TEST_F(ParserImplTest, StructMember_MissingSemicolon) {
  auto* p = parser("a : i32");
  auto decos = p->decoration_list();
  auto m = p->struct_member(decos);
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(m, nullptr);
  EXPECT_EQ(p->error(), "1:8: expected ';' for struct member");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
