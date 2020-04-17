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
struct {
  a : i32;
  [[offset 4 ]] b : f32;
})");
  auto s = p->struct_decl();
  ASSERT_FALSE(p->has_error());
  ASSERT_NE(s, nullptr);
  ASSERT_EQ(s->impl()->members().size(), 2u);
  EXPECT_EQ(s->impl()->members()[0]->name(), "a");
  EXPECT_EQ(s->impl()->members()[1]->name(), "b");
}

TEST_F(ParserImplTest, StructDecl_ParsesWithDecoration) {
  auto* p = parser(R"(
[[block]] struct {
  a : f32;
  b : f32;
})");
  auto s = p->struct_decl();
  ASSERT_FALSE(p->has_error());
  ASSERT_NE(s, nullptr);
  ASSERT_EQ(s->impl()->members().size(), 2u);
  EXPECT_EQ(s->impl()->members()[0]->name(), "a");
  EXPECT_EQ(s->impl()->members()[1]->name(), "b");
}

TEST_F(ParserImplTest, StructDecl_EmptyMembers) {
  auto* p = parser("struct {}");
  auto s = p->struct_decl();
  ASSERT_FALSE(p->has_error());
  ASSERT_NE(s, nullptr);
  ASSERT_EQ(s->impl()->members().size(), 0u);
}

TEST_F(ParserImplTest, StructDecl_MissingBracketLeft) {
  auto* p = parser("struct }");
  auto s = p->struct_decl();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(s, nullptr);
  EXPECT_EQ(p->error(), "1:8: missing { for struct declaration");
}

TEST_F(ParserImplTest, StructDecl_InvalidStructBody) {
  auto* p = parser("struct { a : B; }");
  auto s = p->struct_decl();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(s, nullptr);
  EXPECT_EQ(p->error(), "1:14: unknown type alias 'B'");
}

TEST_F(ParserImplTest, StructDecl_InvalidStructDecorationDecl) {
  auto* p = parser("[[block struct { a : i32; }");
  auto s = p->struct_decl();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(s, nullptr);
  EXPECT_EQ(p->error(), "1:9: missing ]] for struct decoration");
}

TEST_F(ParserImplTest, StructDecl_MissingStruct) {
  auto* p = parser("[[block]] {}");
  auto s = p->struct_decl();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(s, nullptr);
  EXPECT_EQ(p->error(), "1:11: missing struct declaration");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
