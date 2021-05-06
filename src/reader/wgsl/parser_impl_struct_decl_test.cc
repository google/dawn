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

#include "src/ast/struct_block_decoration.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, StructDecl_Parses) {
  auto p = parser(R"(
struct S {
  a : i32;
  b : f32;
})");
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_FALSE(decos.matched);
  ASSERT_EQ(decos.value.size(), 0u);

  auto s = p->struct_decl(decos.value);
  EXPECT_FALSE(p->has_error());
  EXPECT_FALSE(s.errored);
  EXPECT_TRUE(s.matched);
  ASSERT_NE(s.value, nullptr);
  ASSERT_EQ(s->name(), p->builder().Symbols().Register("S"));
  ASSERT_EQ(s->members().size(), 2u);
  EXPECT_EQ(s->members()[0]->symbol(), p->builder().Symbols().Register("a"));
  EXPECT_EQ(s->members()[1]->symbol(), p->builder().Symbols().Register("b"));
}

TEST_F(ParserImplTest, StructDecl_ParsesWithDecoration) {
  auto p = parser(R"(
[[block]] struct B {
  a : f32;
  b : f32;
})");
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_TRUE(decos.matched);
  ASSERT_EQ(decos.value.size(), 1u);

  auto s = p->struct_decl(decos.value);
  EXPECT_FALSE(p->has_error());
  EXPECT_FALSE(s.errored);
  EXPECT_TRUE(s.matched);
  ASSERT_NE(s.value, nullptr);
  ASSERT_EQ(s->name(), p->builder().Symbols().Register("B"));
  ASSERT_EQ(s->members().size(), 2u);
  EXPECT_EQ(s->members()[0]->symbol(), p->builder().Symbols().Register("a"));
  EXPECT_EQ(s->members()[1]->symbol(), p->builder().Symbols().Register("b"));
  ASSERT_EQ(s->decorations().size(), 1u);
  EXPECT_TRUE(s->decorations()[0]->Is<ast::StructBlockDecoration>());
}

TEST_F(ParserImplTest, StructDecl_ParsesWithMultipleDecoration) {
  auto p = parser(R"(
[[block]]
[[block]] struct S {
  a : f32;
  b : f32;
})");
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_TRUE(decos.matched);
  ASSERT_EQ(decos.value.size(), 2u);

  auto s = p->struct_decl(decos.value);
  EXPECT_FALSE(p->has_error());
  EXPECT_FALSE(s.errored);
  EXPECT_TRUE(s.matched);
  ASSERT_NE(s.value, nullptr);
  ASSERT_EQ(s->name(), p->builder().Symbols().Register("S"));
  ASSERT_EQ(s->members().size(), 2u);
  EXPECT_EQ(s->members()[0]->symbol(), p->builder().Symbols().Register("a"));
  EXPECT_EQ(s->members()[1]->symbol(), p->builder().Symbols().Register("b"));
  ASSERT_EQ(s->decorations().size(), 2u);
  EXPECT_TRUE(s->decorations()[0]->Is<ast::StructBlockDecoration>());
  EXPECT_TRUE(s->decorations()[1]->Is<ast::StructBlockDecoration>());
}

TEST_F(ParserImplTest, StructDecl_EmptyMembers) {
  auto p = parser("struct S {}");
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_FALSE(decos.matched);
  ASSERT_EQ(decos.value.size(), 0u);

  auto s = p->struct_decl(decos.value);
  EXPECT_FALSE(p->has_error());
  EXPECT_FALSE(s.errored);
  EXPECT_TRUE(s.matched);
  ASSERT_NE(s.value, nullptr);
  ASSERT_EQ(s->members().size(), 0u);
}

TEST_F(ParserImplTest, StructDecl_MissingIdent) {
  auto p = parser("struct {}");
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_FALSE(decos.matched);
  ASSERT_EQ(decos.value.size(), 0u);

  auto s = p->struct_decl(decos.value);
  EXPECT_TRUE(s.errored);
  EXPECT_FALSE(s.matched);
  EXPECT_EQ(s.value, nullptr);

  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:8: expected identifier for struct declaration");
}

TEST_F(ParserImplTest, StructDecl_MissingBracketLeft) {
  auto p = parser("struct S }");
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_FALSE(decos.matched);
  ASSERT_EQ(decos.value.size(), 0u);

  auto s = p->struct_decl(decos.value);
  EXPECT_TRUE(s.errored);
  EXPECT_FALSE(s.matched);
  EXPECT_EQ(s.value, nullptr);

  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:10: expected '{' for struct declaration");
}

TEST_F(ParserImplTest, StructDecl_InvalidStructBody) {
  auto p = parser("struct S { a : B; }");
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_FALSE(decos.matched);
  ASSERT_EQ(decos.value.size(), 0u);

  auto s = p->struct_decl(decos.value);
  EXPECT_TRUE(s.errored);
  EXPECT_FALSE(s.matched);
  EXPECT_EQ(s.value, nullptr);

  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:16: unknown constructed type 'B'");
}

TEST_F(ParserImplTest, StructDecl_InvalidDecorationDecl) {
  auto p = parser("[[block struct S { a : i32; }");
  auto decos = p->decoration_list();
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);

  auto s = p->struct_decl(decos.value);
  EXPECT_FALSE(s.errored);
  EXPECT_TRUE(s.matched);
  EXPECT_NE(s.value, nullptr);

  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:9: expected ']]' for decoration list");
}

TEST_F(ParserImplTest, StructDecl_MissingStruct) {
  auto p = parser("[[block]] S {}");
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_TRUE(decos.matched);
  ASSERT_EQ(decos.value.size(), 1u);

  auto s = p->struct_decl(decos.value);
  EXPECT_FALSE(s.errored);
  EXPECT_FALSE(s.matched);
  EXPECT_EQ(s.value, nullptr);

  EXPECT_FALSE(p->has_error());
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
