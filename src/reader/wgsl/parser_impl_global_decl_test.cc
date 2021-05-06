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

#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, GlobalDecl_Semicolon) {
  auto p = parser(";");
  p->expect_global_decl();
  ASSERT_FALSE(p->has_error()) << p->error();
}

TEST_F(ParserImplTest, GlobalDecl_GlobalVariable) {
  auto p = parser("var<private> a : vec2<i32> = vec2<i32>(1, 2);");
  p->expect_global_decl();
  ASSERT_FALSE(p->has_error()) << p->error();

  auto program = p->program();
  ASSERT_EQ(program.AST().GlobalVariables().size(), 1u);

  auto* v = program.AST().GlobalVariables()[0];
  EXPECT_EQ(v->symbol(), program.Symbols().Get("a"));
}

TEST_F(ParserImplTest, GlobalDecl_GlobalVariable_Invalid) {
  auto p = parser("var<private> a : vec2<invalid>;");
  p->expect_global_decl();
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:23: unknown constructed type 'invalid'");
}

TEST_F(ParserImplTest, GlobalDecl_GlobalVariable_MissingSemicolon) {
  auto p = parser("var<private> a : vec2<i32>");
  p->expect_global_decl();
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:27: expected ';' for variable declaration");
}

TEST_F(ParserImplTest, GlobalDecl_GlobalConstant) {
  auto p = parser("let a : i32 = 2;");
  p->expect_global_decl();
  ASSERT_FALSE(p->has_error()) << p->error();

  auto program = p->program();
  ASSERT_EQ(program.AST().GlobalVariables().size(), 1u);

  auto* v = program.AST().GlobalVariables()[0];
  EXPECT_EQ(v->symbol(), program.Symbols().Get("a"));
}

TEST_F(ParserImplTest, GlobalDecl_GlobalConstant_Invalid) {
  auto p = parser("let a : vec2<i32> 1.0;");
  p->expect_global_decl();
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:19: expected ';' for let declaration");
}

TEST_F(ParserImplTest, GlobalDecl_GlobalConstant_MissingSemicolon) {
  auto p = parser("let a : vec2<i32> = vec2<i32>(1, 2)");
  p->expect_global_decl();
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:36: expected ';' for let declaration");
}

TEST_F(ParserImplTest, GlobalDecl_TypeAlias) {
  auto p = parser("type A = i32;");
  p->expect_global_decl();
  ASSERT_FALSE(p->has_error()) << p->error();

  auto program = p->program();
  ASSERT_EQ(program.AST().ConstructedTypes().size(), 1u);
  ASSERT_TRUE(program.AST().ConstructedTypes()[0]->Is<ast::Alias>());
  EXPECT_EQ(
      program.Symbols().NameFor(
          program.AST().ConstructedTypes()[0]->As<ast::Alias>()->symbol()),
      "A");
}

TEST_F(ParserImplTest, GlobalDecl_TypeAlias_StructIdent) {
  auto p = parser(R"(struct A {
  a : f32;
};
type B = A;)");
  p->expect_global_decl();
  p->expect_global_decl();
  ASSERT_FALSE(p->has_error()) << p->error();

  auto program = p->program();
  ASSERT_EQ(program.AST().ConstructedTypes().size(), 2u);
  ASSERT_TRUE(program.AST().ConstructedTypes()[0]->Is<ast::Struct>());
  auto* str = program.AST().ConstructedTypes()[0]->As<ast::Struct>();
  EXPECT_EQ(str->name(), program.Symbols().Get("A"));

  ASSERT_TRUE(program.AST().ConstructedTypes()[1]->Is<ast::Alias>());
  auto* alias = program.AST().ConstructedTypes()[1]->As<ast::Alias>();
  EXPECT_EQ(alias->symbol(), program.Symbols().Get("B"));
  auto* tn = alias->type()->As<ast::TypeName>();
  EXPECT_NE(tn, nullptr);
  EXPECT_EQ(tn->name(), str->name());
}

TEST_F(ParserImplTest, GlobalDecl_TypeAlias_Invalid) {
  auto p = parser("type A = invalid;");
  p->expect_global_decl();
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:10: unknown constructed type 'invalid'");
}

TEST_F(ParserImplTest, GlobalDecl_TypeAlias_MissingSemicolon) {
  auto p = parser("type A = i32");
  p->expect_global_decl();
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:13: expected ';' for type alias");
}

TEST_F(ParserImplTest, GlobalDecl_Function) {
  auto p = parser("fn main() { return; }");
  p->expect_global_decl();
  ASSERT_FALSE(p->has_error()) << p->error();

  auto program = p->program();
  ASSERT_EQ(program.AST().Functions().size(), 1u);
  EXPECT_EQ(program.Symbols().NameFor(program.AST().Functions()[0]->symbol()),
            "main");
}

TEST_F(ParserImplTest, GlobalDecl_Function_WithDecoration) {
  auto p = parser("[[workgroup_size(2)]] fn main() { return; }");
  p->expect_global_decl();
  ASSERT_FALSE(p->has_error()) << p->error();

  auto program = p->program();
  ASSERT_EQ(program.AST().Functions().size(), 1u);
  EXPECT_EQ(program.Symbols().NameFor(program.AST().Functions()[0]->symbol()),
            "main");
}

TEST_F(ParserImplTest, GlobalDecl_Function_Invalid) {
  auto p = parser("fn main() -> { return; }");
  p->expect_global_decl();
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:14: unable to determine function return type");
}

TEST_F(ParserImplTest, GlobalDecl_ParsesStruct) {
  auto p = parser("struct A { b: i32; c: f32;};");
  p->expect_global_decl();
  ASSERT_FALSE(p->has_error()) << p->error();

  auto program = p->program();
  ASSERT_EQ(program.AST().ConstructedTypes().size(), 1u);

  auto* t = program.AST().ConstructedTypes()[0];
  ASSERT_NE(t, nullptr);
  ASSERT_TRUE(t->Is<ast::Struct>());

  auto* str = t->As<ast::Struct>();
  EXPECT_EQ(str->name(), program.Symbols().Get("A"));
  EXPECT_EQ(str->members().size(), 2u);
}

TEST_F(ParserImplTest, GlobalDecl_Struct_WithStride) {
  auto p = parser("struct A { data: [[stride(4)]] array<f32>; };");

  p->expect_global_decl();
  ASSERT_FALSE(p->has_error()) << p->error();

  auto program = p->program();
  ASSERT_EQ(program.AST().ConstructedTypes().size(), 1u);

  auto* t = program.AST().ConstructedTypes()[0];
  ASSERT_NE(t, nullptr);
  ASSERT_TRUE(t->Is<ast::Struct>());

  auto* str = t->As<ast::Struct>();
  EXPECT_EQ(str->name(), program.Symbols().Get("A"));
  EXPECT_EQ(str->members().size(), 1u);
  EXPECT_FALSE(str->IsBlockDecorated());

  const auto* ty = str->members()[0]->type();
  ASSERT_TRUE(ty->Is<ast::Array>());
  const auto* arr = ty->As<ast::Array>();

  ASSERT_EQ(arr->decorations().size(), 1u);
  auto* stride = arr->decorations()[0];
  ASSERT_TRUE(stride->Is<ast::StrideDecoration>());
  ASSERT_EQ(stride->As<ast::StrideDecoration>()->stride(), 4u);
}

TEST_F(ParserImplTest, GlobalDecl_Struct_WithDecoration) {
  auto p = parser("[[block]] struct A { data: f32; };");
  p->expect_global_decl();
  ASSERT_FALSE(p->has_error()) << p->error();

  auto program = p->program();
  ASSERT_EQ(program.AST().ConstructedTypes().size(), 1u);

  auto* t = program.AST().ConstructedTypes()[0];
  ASSERT_NE(t, nullptr);
  ASSERT_TRUE(t->Is<ast::Struct>());

  auto* str = t->As<ast::Struct>();
  EXPECT_EQ(str->name(), program.Symbols().Get("A"));
  EXPECT_EQ(str->members().size(), 1u);
  EXPECT_TRUE(str->IsBlockDecorated());
}

TEST_F(ParserImplTest, GlobalDecl_Struct_Invalid) {
  auto p = parser("[[block]] A {};");
  p->expect_global_decl();
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:11: expected declaration after decorations");
}

TEST_F(ParserImplTest, GlobalDecl_StructMissing_Semi) {
  auto p = parser("[[block]] struct A {}");
  p->expect_global_decl();
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:22: expected ';' for struct declaration");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
