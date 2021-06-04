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

TEST_F(ParserImplTest, VariableIdentDecl_InvalidType) {
  auto p = parser("my_var : invalid");
  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(decl.errored);
  ASSERT_EQ(p->error(), "1:10: unknown constructed type 'invalid'");
}

// TODO(crbug.com/tint/846): Remove
TEST_F(ParserImplTest,
       VariableIdentDecl_ParsesWithTextureAccessDeco_Read_DEPRECATED) {
  auto p = parser("my_var : [[access(read)]] texture_storage_1d<r32float>");

  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(decl.errored);
  ASSERT_EQ(decl->name, "my_var");
  ASSERT_NE(decl->type, nullptr);
  ASSERT_TRUE(decl->type->Is<ast::StorageTexture>());
  EXPECT_TRUE(decl->type->As<ast::StorageTexture>()->is_read_only());

  EXPECT_EQ(p->error(),
            "1:54: use of deprecated language feature: access control is "
            "expected as last parameter of storage textures");
}

// TODO(crbug.com/tint/846): Remove
TEST_F(ParserImplTest,
       VariableIdentDecl_ParsesWithTextureAccessDeco_Write_DEPRECATED) {
  auto p = parser("my_var : [[access(write)]] texture_storage_1d<r32float>");

  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(decl.errored);
  ASSERT_EQ(decl->name, "my_var");
  ASSERT_NE(decl->type, nullptr);
  ASSERT_TRUE(decl->type->Is<ast::StorageTexture>());
  EXPECT_TRUE(decl->type->As<ast::StorageTexture>()->is_write_only());

  EXPECT_EQ(p->error(),
            "1:55: use of deprecated language feature: access control is "
            "expected as last parameter of storage textures");
}

// TODO(crbug.com/tint/846): Remove
TEST_F(ParserImplTest, VariableIdentDecl_ParsesWithAccessDeco_Read_DEPRECATED) {
  auto p = parser("var my_var : [[access(read)]] S;");

  auto* mem = Member("a", ty.i32(), ast::DecorationList{});
  ast::StructMemberList members;
  members.push_back(mem);

  auto* block_deco = create<ast::StructBlockDecoration>();
  ast::DecorationList decos;
  decos.push_back(block_deco);

  auto* s = Structure(Sym("S"), members, decos);

  p->register_constructed("S", s);

  auto res = p->expect_global_decl();
  ASSERT_FALSE(res.errored) << p->error();
  ASSERT_NE(p->builder().AST().GlobalVariables().size(), 0u);
  auto* decl = p->builder().AST().GlobalVariables()[0];
  ASSERT_NE(decl, nullptr);
  ASSERT_EQ(decl->symbol(), p->builder().Symbols().Get("my_var"));
  ASSERT_NE(decl->type(), nullptr);
  EXPECT_TRUE(decl->type()->Is<ast::TypeName>());
  EXPECT_EQ(decl->declared_access(), ast::Access::kRead);

  EXPECT_EQ(p->error(),
            "1:1: use of deprecated language feature: declare access with "
            "var<none, read> instead of using [[access]] decoration");
}

// TODO(crbug.com/tint/846): Remove
TEST_F(ParserImplTest,
       VariableIdentDecl_ParsesWithAccessDeco_ReadWrite_DEPRECATED) {
  auto p = parser("var my_var : [[access(read_write)]] S;");

  auto* mem = Member("a", ty.i32(), ast::DecorationList{});
  ast::StructMemberList members;
  members.push_back(mem);

  auto* block_deco = create<ast::StructBlockDecoration>();
  ast::DecorationList decos;
  decos.push_back(block_deco);

  auto* s = Structure(Sym("S"), members, decos);

  p->register_constructed("S", s);

  auto res = p->expect_global_decl();
  ASSERT_FALSE(res.errored) << p->error();
  ASSERT_NE(p->builder().AST().GlobalVariables().size(), 0u);
  auto* decl = p->builder().AST().GlobalVariables()[0];
  ASSERT_NE(decl, nullptr);
  ASSERT_EQ(decl->symbol(), p->builder().Symbols().Get("my_var"));
  ASSERT_NE(decl->type(), nullptr);
  EXPECT_TRUE(decl->type()->Is<ast::TypeName>());
  EXPECT_EQ(decl->declared_access(), ast::Access::kReadWrite);

  EXPECT_EQ(p->error(),
            "1:1: use of deprecated language feature: declare access with "
            "var<none, read_write> instead of using [[access]] decoration");
}

// TODO(crbug.com/tint/846): Remove
TEST_F(ParserImplTest, VariableIdentDecl_MultipleAccessDecoFail_DEPRECATED) {
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

// TODO(crbug.com/tint/846): Remove
TEST_F(ParserImplTest,
       VariableIdentDecl_MultipleAccessDeco_MultiBlock_Fail_DEPRECATED) {
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

// TODO(crbug.com/tint/846): Remove
TEST_F(ParserImplTest, VariableIdentDecl_AccessDecoBadValue_DEPRECATED) {
  auto p = parser("my_var : [[access(unknown)]] S");
  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(decl.errored);
  ASSERT_EQ(p->error(), "1:19: invalid value for access control");
}

// TODO(crbug.com/tint/846): Remove
TEST_F(ParserImplTest, VariableIdentDecl_AccessDecoIllegalValue_DEPRECATED) {
  auto p = parser("my_var : [[access(1)]] S");
  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(decl.errored);
  ASSERT_EQ(p->error(), "1:19: expected identifier for access control");
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
