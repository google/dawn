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

TEST_F(ParserImplTest, VariableIdentDecl_Parses) {
  auto p = parser("my_var : f32");
  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(decl.errored);
  ASSERT_EQ(decl->name, "my_var");
  ASSERT_NE(decl->type, nullptr);
  ASSERT_TRUE(decl->type->Is<ast::F32>());

  EXPECT_EQ(decl->source.range, (Source::Range{{1u, 1u}, {1u, 7u}}));
  EXPECT_EQ(decl->type->source.range, (Source::Range{{1u, 10u}, {1u, 13u}}));
}

TEST_F(ParserImplTest, VariableIdentDecl_Inferred_Parses) {
  auto p = parser("my_var = 1.0");
  auto decl = p->expect_variable_ident_decl("test", /*allow_inferred = */ true);
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(decl.errored);
  ASSERT_EQ(decl->name, "my_var");
  ASSERT_EQ(decl->type, nullptr);

  EXPECT_EQ(decl->source.range, (Source::Range{{1u, 1u}, {1u, 7u}}));
}

TEST_F(ParserImplTest, VariableIdentDecl_MissingIdent) {
  auto p = parser(": f32");
  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(decl.errored);
  ASSERT_EQ(p->error(), "1:1: expected identifier for test");
}

TEST_F(ParserImplTest, VariableIdentDecl_MissingColon) {
  auto p = parser("my_var f32");
  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(decl.errored);
  ASSERT_EQ(p->error(), "1:8: expected ':' for test");
}

TEST_F(ParserImplTest, VariableIdentDecl_MissingType) {
  auto p = parser("my_var :");
  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(decl.errored);
  ASSERT_EQ(p->error(), "1:9: invalid type for test");
}

TEST_F(ParserImplTest, VariableIdentDecl_InvalidIdent) {
  auto p = parser("123 : f32");
  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(decl.errored);
  ASSERT_EQ(p->error(), "1:1: expected identifier for test");
}

TEST_F(ParserImplTest, VariableIdentDecl_NonAccessDecoFail) {
  auto p = parser("my_var : [[stride(1)]] S");

  auto* mem = Member("a", ty.i32(), ast::DecorationList{});
  ast::StructMemberList members;
  members.push_back(mem);

  auto* block_deco = create<ast::StructBlockDecoration>();
  ast::DecorationList decos;
  decos.push_back(block_deco);

  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(decl.errored);
  ASSERT_EQ(p->error(), "1:12: unexpected decorations");
}

TEST_F(ParserImplTest, VariableIdentDecl_DecorationMissingRightBlock) {
  auto p = parser("my_var : [[stride(4) S");
  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(decl.errored);
  ASSERT_EQ(p->error(), "1:22: expected ']]' for decoration list");
}

TEST_F(ParserImplTest, VariableIdentDecl_DecorationMissingRightParen) {
  auto p = parser("my_var : [[stride(4]] S");
  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(decl.errored);
  ASSERT_EQ(p->error(), "1:20: expected ')' for stride decoration");
}

TEST_F(ParserImplTest, VariableIdentDecl_DecorationMissingLeftParen) {
  auto p = parser("my_var : [[stride 4)]] S");
  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(decl.errored);
  ASSERT_EQ(p->error(), "1:19: expected '(' for stride decoration");
}

TEST_F(ParserImplTest, VariableIdentDecl_DecorationEmpty) {
  auto p = parser("my_var : [[]] S");
  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(decl.errored);
  ASSERT_EQ(p->error(), "1:12: empty decoration list");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
