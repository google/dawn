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
  EXPECT_EQ(decl->type->source().range, (Source::Range{{1u, 10u}, {1u, 13u}}));
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

TEST_F(ParserImplTest, VariableIdentDecl_InvalidType) {
  auto p = parser("my_var : invalid");
  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(decl.errored);
  ASSERT_EQ(p->error(), "1:10: unknown constructed type 'invalid'");
}

TEST_F(ParserImplTest, VariableIdentDecl_ParsesWithTextureAccessDeco_Read) {
  auto p = parser("my_var : [[access(read)]] texture_storage_1d<r32float>");

  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(decl.errored);
  ASSERT_EQ(decl->name, "my_var");
  ASSERT_NE(decl->type, nullptr);
  ASSERT_TRUE(decl->type->Is<ast::AccessControl>());
  EXPECT_TRUE(decl->type->As<ast::AccessControl>()->IsReadOnly());
  ASSERT_TRUE(
      decl->type->As<ast::AccessControl>()->type()->Is<ast::StorageTexture>());
}

TEST_F(ParserImplTest, VariableIdentDecl_ParsesWithTextureAccessDeco_Write) {
  auto p = parser("my_var : [[access(write)]] texture_storage_1d<r32float>");

  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(decl.errored);
  ASSERT_EQ(decl->name, "my_var");
  ASSERT_NE(decl->type, nullptr);
  ASSERT_TRUE(decl->type->Is<ast::AccessControl>());
  EXPECT_TRUE(decl->type->As<ast::AccessControl>()->IsWriteOnly());
  ASSERT_TRUE(
      decl->type->As<ast::AccessControl>()->type()->Is<ast::StorageTexture>());
}

TEST_F(ParserImplTest, VariableIdentDecl_ParsesWithAccessDeco_Read) {
  auto p = parser("my_var : [[access(read)]] S");

  auto* mem = Member("a", ty.i32(), ast::DecorationList{});
  ast::StructMemberList members;
  members.push_back(mem);

  auto* block_deco = create<ast::StructBlockDecoration>();
  ast::DecorationList decos;
  decos.push_back(block_deco);

  auto* s = Structure(Sym("S"), members, decos);

  p->register_constructed("S", s);

  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(decl.errored);
  ASSERT_EQ(decl->name, "my_var");
  ASSERT_NE(decl->type, nullptr);
  ASSERT_TRUE(decl->type->Is<ast::AccessControl>());
  EXPECT_TRUE(decl->type->As<ast::AccessControl>()->IsReadOnly());
}

TEST_F(ParserImplTest, VariableIdentDecl_ParsesWithAccessDeco_ReadWrite) {
  auto p = parser("my_var : [[access(read_write)]] S");

  auto* mem = Member("a", ty.i32(), ast::DecorationList{});
  ast::StructMemberList members;
  members.push_back(mem);

  auto* block_deco = create<ast::StructBlockDecoration>();
  ast::DecorationList decos;
  decos.push_back(block_deco);

  auto* s = Structure(Sym("S"), members, decos);

  p->register_constructed("S", s);

  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(decl.errored);
  ASSERT_EQ(decl->name, "my_var");
  ASSERT_NE(decl->type, nullptr);
  ASSERT_TRUE(decl->type->Is<ast::AccessControl>());
  EXPECT_TRUE(decl->type->As<ast::AccessControl>()->IsReadWrite());
}

TEST_F(ParserImplTest, VariableIdentDecl_MultipleAccessDecoFail) {
  auto p = parser("my_var : [[access(read), access(read_write)]] S");

  auto* mem = Member("a", ty.i32(), ast::DecorationList{});
  ast::StructMemberList members;
  members.push_back(mem);

  auto* block_deco = create<ast::StructBlockDecoration>();
  ast::DecorationList decos;
  decos.push_back(block_deco);

  auto* s = Structure(Sym("S"), members, decos);

  p->register_constructed("S", s);

  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(decl.errored);
  ASSERT_EQ(p->error(), "1:1: multiple access decorations not allowed");
}

TEST_F(ParserImplTest, VariableIdentDecl_MultipleAccessDeco_MultiBlock_Fail) {
  auto p = parser("my_var : [[access(read)]][[access(read_write)]] S");

  auto* mem = Member("a", ty.i32(), ast::DecorationList{});
  ast::StructMemberList members;
  members.push_back(mem);

  auto* block_deco = create<ast::StructBlockDecoration>();
  ast::DecorationList decos;
  decos.push_back(block_deco);

  auto* s = Structure(Sym("S"), members, decos);

  p->register_constructed("S", s);

  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(decl.errored);
  ASSERT_EQ(p->error(), "1:1: multiple access decorations not allowed");
}

TEST_F(ParserImplTest, VariableIdentDecl_AccessDecoBadValue) {
  auto p = parser("my_var : [[access(unknown)]] S");
  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(decl.errored);
  ASSERT_EQ(p->error(), "1:19: invalid value for access decoration");
}

TEST_F(ParserImplTest, VariableIdentDecl_AccessDecoIllegalValue) {
  auto p = parser("my_var : [[access(1)]] S");
  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(decl.errored);
  ASSERT_EQ(p->error(), "1:19: expected identifier for access_type");
}

TEST_F(ParserImplTest, VariableIdentDecl_NonAccessDecoFail) {
  auto p = parser("my_var : [[stride(1)]] S");

  auto* mem = Member("a", ty.i32(), ast::DecorationList{});
  ast::StructMemberList members;
  members.push_back(mem);

  auto* block_deco = create<ast::StructBlockDecoration>();
  ast::DecorationList decos;
  decos.push_back(block_deco);

  auto* s = Structure(Sym("S"), members, decos);

  p->register_constructed("S", s);

  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(decl.errored);
  ASSERT_EQ(p->error(), "1:12: unexpected decorations");
}

TEST_F(ParserImplTest, VariableIdentDecl_DecorationMissingRightBlock) {
  auto p = parser("my_var : [[access(read) S");
  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(decl.errored);
  ASSERT_EQ(p->error(), "1:25: expected ']]' for decoration list");
}

TEST_F(ParserImplTest, VariableIdentDecl_DecorationMissingRightParen) {
  auto p = parser("my_var : [[access(read]] S");
  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(decl.errored);
  ASSERT_EQ(p->error(), "1:23: expected ')' for access decoration");
}

TEST_F(ParserImplTest, VariableIdentDecl_DecorationMissingLeftParen) {
  auto p = parser("my_var : [[access read)]] S");
  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(decl.errored);
  ASSERT_EQ(p->error(), "1:19: expected '(' for access decoration");
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
