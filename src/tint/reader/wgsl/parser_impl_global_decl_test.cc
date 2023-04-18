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

#include "src/tint/ast/test_helper.h"
#include "src/tint/reader/wgsl/parser_impl_test_helper.h"

namespace tint::reader::wgsl {
namespace {

TEST_F(ParserImplTest, GlobalDecl_Semicolon) {
    auto p = parser(";");
    p->global_decl();
    ASSERT_FALSE(p->has_error()) << p->error();
}

TEST_F(ParserImplTest, GlobalDecl_GlobalVariable) {
    auto p = parser("var<private> a : vec2<i32> = vec2<i32>(1, 2);");
    p->global_decl();
    ASSERT_FALSE(p->has_error()) << p->error();

    auto program = p->program();
    ASSERT_EQ(program.AST().GlobalVariables().Length(), 1u);

    auto* v = program.AST().GlobalVariables()[0];
    EXPECT_EQ(v->name->symbol, program.Symbols().Get("a"));
    ast::CheckIdentifier(v->type, ast::Template("vec2", "i32"));
}

TEST_F(ParserImplTest, GlobalDecl_GlobalVariable_Inferred) {
    auto p = parser("var<private> a = vec2<i32>(1, 2);");
    p->global_decl();
    ASSERT_FALSE(p->has_error()) << p->error();

    auto program = p->program();
    ASSERT_EQ(program.AST().GlobalVariables().Length(), 1u);

    auto* v = program.AST().GlobalVariables()[0];
    EXPECT_EQ(v->name->symbol, program.Symbols().Get("a"));
    EXPECT_EQ(v->type, nullptr);
}

TEST_F(ParserImplTest, GlobalDecl_GlobalVariable_MissingSemicolon) {
    auto p = parser("var<private> a : vec2<i32>");
    p->global_decl();
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:27: expected ';' for variable declaration");
}

TEST_F(ParserImplTest, GlobalDecl_GlobalLet) {
    auto p = parser("let a : i32 = 2;");
    auto e = p->global_decl();
    EXPECT_TRUE(p->has_error());
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(p->error(), "1:1: module-scope 'let' is invalid, use 'const'");
}

TEST_F(ParserImplTest, GlobalDecl_GlobalConst) {
    auto p = parser("const a : i32 = 2;");
    p->global_decl();
    ASSERT_FALSE(p->has_error()) << p->error();

    auto program = p->program();
    ASSERT_EQ(program.AST().GlobalVariables().Length(), 1u);

    auto* v = program.AST().GlobalVariables()[0];
    EXPECT_EQ(v->name->symbol, program.Symbols().Get("a"));
}

TEST_F(ParserImplTest, GlobalDecl_GlobalConst_MissingInitializer) {
    auto p = parser("const a : vec2<i32>;");
    p->global_decl();
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:20: expected '=' for 'const' declaration");
}

TEST_F(ParserImplTest, GlobalDecl_GlobalConst_Invalid) {
    auto p = parser("const a : vec2<i32> 1.0;");
    p->global_decl();
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:21: expected '=' for 'const' declaration");
}

TEST_F(ParserImplTest, GlobalDecl_GlobalConst_MissingSemicolon) {
    auto p = parser("const a : vec2<i32> = vec2<i32>(1, 2)");
    p->global_decl();
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:38: expected ';' for 'const' declaration");
}

TEST_F(ParserImplTest, GlobalDecl_TypeAlias) {
    auto p = parser("alias A = i32;");
    p->global_decl();
    ASSERT_FALSE(p->has_error()) << p->error();

    auto program = p->program();
    ASSERT_EQ(program.AST().TypeDecls().Length(), 1u);
    ASSERT_TRUE(program.AST().TypeDecls()[0]->Is<ast::Alias>());
    ast::CheckIdentifier(program.AST().TypeDecls()[0]->As<ast::Alias>()->name, "A");
}

TEST_F(ParserImplTest, GlobalDecl_TypeAlias_StructIdent) {
    auto p = parser(R"(struct A {
  a : f32,
}
alias B = A;)");
    p->global_decl();
    p->global_decl();
    ASSERT_FALSE(p->has_error()) << p->error();

    auto program = p->program();
    ASSERT_EQ(program.AST().TypeDecls().Length(), 2u);
    ASSERT_TRUE(program.AST().TypeDecls()[0]->Is<ast::Struct>());
    auto* str = program.AST().TypeDecls()[0]->As<ast::Struct>();
    EXPECT_EQ(str->name->symbol, program.Symbols().Get("A"));

    ASSERT_TRUE(program.AST().TypeDecls()[1]->Is<ast::Alias>());
    auto* alias = program.AST().TypeDecls()[1]->As<ast::Alias>();
    EXPECT_EQ(alias->name->symbol, program.Symbols().Get("B"));
    ast::CheckIdentifier(alias->type, "A");
}

TEST_F(ParserImplTest, GlobalDecl_TypeAlias_MissingSemicolon) {
    auto p = parser("alias A = i32");
    p->global_decl();
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:14: expected ';' for type alias");
}

TEST_F(ParserImplTest, GlobalDecl_Function) {
    auto p = parser("fn main() { return; }");
    p->global_decl();
    ASSERT_FALSE(p->has_error()) << p->error();

    auto program = p->program();
    ASSERT_EQ(program.AST().Functions().Length(), 1u);
    ast::CheckIdentifier(program.AST().Functions()[0]->name, "main");
}

TEST_F(ParserImplTest, GlobalDecl_Function_WithAttribute) {
    auto p = parser("@workgroup_size(2) fn main() { return; }");
    p->global_decl();
    ASSERT_FALSE(p->has_error()) << p->error();

    auto program = p->program();
    ASSERT_EQ(program.AST().Functions().Length(), 1u);
    ast::CheckIdentifier(program.AST().Functions()[0]->name, "main");
}

TEST_F(ParserImplTest, GlobalDecl_Function_Invalid) {
    auto p = parser("fn main() -> { return; }");
    p->global_decl();
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:14: unable to determine function return type");
}

TEST_F(ParserImplTest, GlobalDecl_ParsesStruct) {
    auto p = parser("struct A { b: i32, c: f32}");
    p->global_decl();
    ASSERT_FALSE(p->has_error()) << p->error();

    auto program = p->program();
    ASSERT_EQ(program.AST().TypeDecls().Length(), 1u);

    auto* t = program.AST().TypeDecls()[0];
    ASSERT_NE(t, nullptr);
    ASSERT_TRUE(t->Is<ast::Struct>());

    auto* str = t->As<ast::Struct>();
    EXPECT_EQ(str->name->symbol, program.Symbols().Get("A"));
    EXPECT_EQ(str->members.Length(), 2u);
}

TEST_F(ParserImplTest, GlobalDecl_Struct_Invalid) {
    {
        auto p = parser("A {}");
        auto decl = p->global_decl();
        // global_decl will result in a no match.
        ASSERT_FALSE(p->has_error()) << p->error();
        ASSERT_TRUE(!decl.matched && !decl.errored);
    }
    {
        auto p = parser("A {}");
        p->translation_unit();
        // translation_unit will result in a general error.
        ASSERT_TRUE(p->has_error());
        EXPECT_EQ(p->error(), "1:1: unexpected token");
    }
}

TEST_F(ParserImplTest, GlobalDecl_Struct_UnexpectedAttribute) {
    auto p = parser("@vertex struct S { i : i32 }");

    auto s = p->global_decl();
    EXPECT_TRUE(s.errored);
    EXPECT_FALSE(s.matched);

    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:2: unexpected attributes");
}

TEST_F(ParserImplTest, GlobalDecl_ConstAssert_WithParen) {
    auto p = parser("const_assert(true);");
    p->global_decl();
    ASSERT_FALSE(p->has_error()) << p->error();

    auto program = p->program();
    ASSERT_EQ(program.AST().ConstAsserts().Length(), 1u);
    auto* sa = program.AST().ConstAsserts()[0];
    EXPECT_EQ(sa->source.range.begin.line, 1u);
    EXPECT_EQ(sa->source.range.begin.column, 1u);
    EXPECT_EQ(sa->source.range.end.line, 1u);
    EXPECT_EQ(sa->source.range.end.column, 19u);

    EXPECT_TRUE(sa->condition->Is<ast::BoolLiteralExpression>());
    EXPECT_EQ(sa->condition->source.range.begin.line, 1u);
    EXPECT_EQ(sa->condition->source.range.begin.column, 14u);
    EXPECT_EQ(sa->condition->source.range.end.line, 1u);
    EXPECT_EQ(sa->condition->source.range.end.column, 18u);
}

TEST_F(ParserImplTest, GlobalDecl_ConstAssert_WithoutParen) {
    auto p = parser("const_assert  true;");
    p->global_decl();
    ASSERT_FALSE(p->has_error()) << p->error();

    auto program = p->program();
    ASSERT_EQ(program.AST().ConstAsserts().Length(), 1u);
    auto* sa = program.AST().ConstAsserts()[0];
    EXPECT_TRUE(sa->condition->Is<ast::BoolLiteralExpression>());

    EXPECT_EQ(sa->source.range.begin.line, 1u);
    EXPECT_EQ(sa->source.range.begin.column, 1u);
    EXPECT_EQ(sa->source.range.end.line, 1u);
    EXPECT_EQ(sa->source.range.end.column, 19u);

    EXPECT_TRUE(sa->condition->Is<ast::BoolLiteralExpression>());
    EXPECT_EQ(sa->condition->source.range.begin.line, 1u);
    EXPECT_EQ(sa->condition->source.range.begin.column, 15u);
    EXPECT_EQ(sa->condition->source.range.end.line, 1u);
    EXPECT_EQ(sa->condition->source.range.end.column, 19u);
}

}  // namespace
}  // namespace tint::reader::wgsl
