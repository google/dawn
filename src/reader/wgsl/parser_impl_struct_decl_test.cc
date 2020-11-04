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
#include "src/ast/type/struct_type.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, StructDecl_Parses) {
  auto* p = parser(R"(
struct S {
  a : i32;
  [[offset(4)]] b : f32;
})");
  auto s = p->struct_decl();
  ASSERT_FALSE(p->has_error());
  ASSERT_NE(s, nullptr);
  ASSERT_EQ(s->name(), "S");
  ASSERT_EQ(s->impl()->members().size(), 2u);
  EXPECT_EQ(s->impl()->members()[0]->name(), "a");
  EXPECT_EQ(s->impl()->members()[1]->name(), "b");
}

TEST_F(ParserImplTest, StructDecl_ParsesWithDecoration) {
  auto* p = parser(R"(
[[block]] struct B {
  a : f32;
  b : f32;
})");
  auto s = p->struct_decl();
  ASSERT_FALSE(p->has_error());
  ASSERT_NE(s, nullptr);
  ASSERT_EQ(s->name(), "B");
  ASSERT_EQ(s->impl()->members().size(), 2u);
  EXPECT_EQ(s->impl()->members()[0]->name(), "a");
  EXPECT_EQ(s->impl()->members()[1]->name(), "b");
  ASSERT_EQ(s->impl()->decorations().size(), 1u);
  EXPECT_TRUE(s->impl()->decorations()[0]->IsBlock());
}

TEST_F(ParserImplTest, StructDecl_ParsesWithMultipleDecoration) {
  auto* p = parser(R"(
[[block]]
[[block]] struct S {
  a : f32;
  b : f32;
})");
  auto s = p->struct_decl();
  ASSERT_FALSE(p->has_error());
  ASSERT_NE(s, nullptr);
  ASSERT_EQ(s->name(), "S");
  ASSERT_EQ(s->impl()->members().size(), 2u);
  EXPECT_EQ(s->impl()->members()[0]->name(), "a");
  EXPECT_EQ(s->impl()->members()[1]->name(), "b");
  ASSERT_EQ(s->impl()->decorations().size(), 2u);
  EXPECT_TRUE(s->impl()->decorations()[0]->IsBlock());
  EXPECT_TRUE(s->impl()->decorations()[1]->IsBlock());
}

TEST_F(ParserImplTest, StructDecl_EmptyMembers) {
  auto* p = parser("struct S {}");
  auto s = p->struct_decl();
  ASSERT_FALSE(p->has_error());
  ASSERT_NE(s, nullptr);
  ASSERT_EQ(s->impl()->members().size(), 0u);
}

TEST_F(ParserImplTest, StructDecl_MissingIdent) {
  auto* p = parser("struct {}");
  auto s = p->struct_decl();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(s, nullptr);
  EXPECT_EQ(p->error(), "1:8: expected identifier for struct declaration");
}

TEST_F(ParserImplTest, StructDecl_MissingBracketLeft) {
  auto* p = parser("struct S }");
  auto s = p->struct_decl();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(s, nullptr);
  EXPECT_EQ(p->error(), "1:10: missing { for struct declaration");
}

TEST_F(ParserImplTest, StructDecl_InvalidStructBody) {
  auto* p = parser("struct S { a : B; }");
  auto s = p->struct_decl();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(s, nullptr);
  EXPECT_EQ(p->error(), "1:16: unknown constructed type 'B'");
}

TEST_F(ParserImplTest, StructDecl_InvalidStructDecorationDecl) {
  auto* p = parser("[[block struct S { a : i32; }");
  auto s = p->struct_decl();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(s, nullptr);
  EXPECT_EQ(p->error(), "1:9: missing ]] for struct decoration");
}

TEST_F(ParserImplTest, StructDecl_MissingStruct) {
  auto* p = parser("[[block]] S {}");
  auto s = p->struct_decl();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(s, nullptr);
  EXPECT_EQ(p->error(), "1:11: missing struct declaration");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
