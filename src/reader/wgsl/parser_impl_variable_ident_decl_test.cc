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
#include "src/ast/struct.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/type/access_control_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/struct_type.h"
#include "src/reader/wgsl/parser_impl.h"
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
  ASSERT_TRUE(decl->type->IsF32());

  ASSERT_EQ(decl->source.range.begin.line, 1u);
  ASSERT_EQ(decl->source.range.begin.column, 1u);
  ASSERT_EQ(decl->source.range.end.line, 1u);
  ASSERT_EQ(decl->source.range.end.column, 7u);
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

TEST_F(ParserImplTest, VariableIdentDecl_ParsesWithAccessDeco_Read) {
  ast::type::I32Type i32;

  ast::StructMember mem("a", &i32, ast::StructMemberDecorationList{});
  ast::StructMemberList members;
  members.push_back(&mem);

  ast::StructBlockDecoration block_deco(Source{});
  ast::StructDecorationList decos;
  decos.push_back(&block_deco);

  ast::Struct str(decos, members);
  ast::type::StructType s("S", &str);

  auto p = parser("my_var : [[access(read)]] S");
  p->register_constructed("S", &s);

  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(decl.errored);
  ASSERT_EQ(decl->name, "my_var");
  ASSERT_NE(decl->type, nullptr);
  ASSERT_TRUE(decl->type->Is<ast::type::AccessControlType>());
  EXPECT_TRUE(decl->type->As<ast::type::AccessControlType>()->IsReadOnly());
}

TEST_F(ParserImplTest, VariableIdentDecl_ParsesWithAccessDeco_ReadWrite) {
  ast::type::I32Type i32;

  ast::StructMember mem("a", &i32, ast::StructMemberDecorationList{});
  ast::StructMemberList members;
  members.push_back(&mem);

  ast::StructBlockDecoration block_deco(Source{});
  ast::StructDecorationList decos;
  decos.push_back(&block_deco);

  ast::Struct str(decos, members);
  ast::type::StructType s("S", &str);

  auto p = parser("my_var : [[access(read_write)]] S");
  p->register_constructed("S", &s);

  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(decl.errored);
  ASSERT_EQ(decl->name, "my_var");
  ASSERT_NE(decl->type, nullptr);
  ASSERT_TRUE(decl->type->Is<ast::type::AccessControlType>());
  EXPECT_TRUE(decl->type->As<ast::type::AccessControlType>()->IsReadWrite());
}

TEST_F(ParserImplTest, VariableIdentDecl_MultipleAccessDecoFail) {
  ast::type::I32Type i32;

  ast::StructMember mem("a", &i32, ast::StructMemberDecorationList{});
  ast::StructMemberList members;
  members.push_back(&mem);

  ast::StructBlockDecoration block_deco(Source{});
  ast::StructDecorationList decos;
  decos.push_back(&block_deco);

  ast::Struct str(decos, members);
  ast::type::StructType s("S", &str);

  auto p = parser("my_var : [[access(read), access(read_write)]] S");
  p->register_constructed("S", &s);

  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(decl.errored);
  ASSERT_EQ(p->error(), "1:1: multiple access decorations not allowed");
}

TEST_F(ParserImplTest, VariableIdentDecl_MultipleAccessDeco_MultiBlock_Fail) {
  ast::type::I32Type i32;

  ast::StructMember mem("a", &i32, ast::StructMemberDecorationList{});
  ast::StructMemberList members;
  members.push_back(&mem);

  ast::StructBlockDecoration block_deco(Source{});
  ast::StructDecorationList decos;
  decos.push_back(&block_deco);

  ast::Struct str(decos, members);
  ast::type::StructType s("S", &str);

  auto p = parser("my_var : [[access(read)]][[access(read_write)]] S");
  p->register_constructed("S", &s);

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
  ast::type::I32Type i32;

  ast::StructMember mem("a", &i32, ast::StructMemberDecorationList{});
  ast::StructMemberList members;
  members.push_back(&mem);

  ast::StructBlockDecoration block_deco(Source{});
  ast::StructDecorationList decos;
  decos.push_back(&block_deco);

  ast::Struct str(decos, members);
  ast::type::StructType s("S", &str);

  auto p = parser("my_var : [[stride(1)]] S");
  p->register_constructed("S", &s);

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
