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
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, StructMemberDecorationDecl_EmptyStr) {
  auto* p = parser("");
  ast::StructMemberDecorationList decos;
  ASSERT_TRUE(p->struct_member_decoration_decl(decos));
  ASSERT_FALSE(p->has_error());
  EXPECT_EQ(decos.size(), 0u);
}

TEST_F(ParserImplTest, StructMemberDecorationDecl_EmptyBlock) {
  auto* p = parser("[[]]");
  ast::StructMemberDecorationList decos;
  ASSERT_FALSE(p->struct_member_decoration_decl(decos));
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:3: empty struct member decoration found");
}

TEST_F(ParserImplTest, StructMemberDecorationDecl_Single) {
  auto* p = parser("[[offset(4)]]");
  ast::StructMemberDecorationList decos;
  ASSERT_TRUE(p->struct_member_decoration_decl(decos));
  ASSERT_FALSE(p->has_error());
  ASSERT_EQ(decos.size(), 1u);
  EXPECT_TRUE(decos[0]->IsOffset());
}

TEST_F(ParserImplTest, StructMemberDecorationDecl_InvalidDecoration) {
  auto* p = parser("[[offset(nan)]]");
  ast::StructMemberDecorationList decos;
  ASSERT_FALSE(p->struct_member_decoration_decl(decos));
  ASSERT_TRUE(p->has_error()) << p->error();
  EXPECT_EQ(p->error(), "1:10: invalid value for offset decoration");
}

TEST_F(ParserImplTest, StructMemberDecorationDecl_MissingClose) {
  auto* p = parser("[[offset(4)");
  ast::StructMemberDecorationList decos;
  ASSERT_FALSE(p->struct_member_decoration_decl(decos));
  ASSERT_TRUE(p->has_error()) << p->error();
  EXPECT_EQ(p->error(), "1:12: missing ]] for struct member decoration");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
