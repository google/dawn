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

TEST_F(ParserImplTest, VariableStmt_VariableDecl) {
    auto p = parser("var a : i32;");
    auto e = p->variable_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::VariableDeclStatement>());
    ASSERT_NE(e->variable, nullptr);
    EXPECT_EQ(e->variable->name->symbol, p->builder().Symbols().Get("a"));

    EXPECT_EQ(e->source.range.begin.line, 1u);
    EXPECT_EQ(e->source.range.begin.column, 1u);
    EXPECT_EQ(e->source.range.end.line, 1u);
    EXPECT_EQ(e->source.range.end.column, 12u);

    EXPECT_EQ(e->variable->initializer, nullptr);
}

TEST_F(ParserImplTest, VariableStmt_VariableDecl_WithInit) {
    auto p = parser("var a : i32 = 1;");
    auto e = p->variable_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::VariableDeclStatement>());
    ASSERT_NE(e->variable, nullptr);
    EXPECT_EQ(e->variable->name->symbol, p->builder().Symbols().Get("a"));

    EXPECT_EQ(e->source.range.begin.line, 1u);
    EXPECT_EQ(e->source.range.begin.column, 1u);
    EXPECT_EQ(e->source.range.end.line, 1u);
    EXPECT_EQ(e->source.range.end.column, 12u);

    ASSERT_NE(e->variable->initializer, nullptr);
    EXPECT_TRUE(e->variable->initializer->Is<ast::LiteralExpression>());
}

TEST_F(ParserImplTest, VariableStmt_VariableDecl_InitializerInvalid) {
    auto p = parser("var a : i32 = if(a) {}");
    auto e = p->variable_statement();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:15: missing initializer for 'var' declaration");
}

TEST_F(ParserImplTest, VariableStmt_VariableDecl_ArrayInit) {
    auto p = parser("var a : array<i32> = array<i32>();");
    auto e = p->variable_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::VariableDeclStatement>());
    ASSERT_NE(e->variable, nullptr);
    EXPECT_EQ(e->variable->name->symbol, p->builder().Symbols().Get("a"));

    ASSERT_NE(e->variable->initializer, nullptr);
    auto* call = e->variable->initializer->As<ast::CallExpression>();
    ASSERT_NE(call, nullptr);
    ast::CheckIdentifier(call->target, ast::Template("array", "i32"));
}

TEST_F(ParserImplTest, VariableStmt_VariableDecl_ArrayInit_NoSpace) {
    auto p = parser("var a : array<i32>=array<i32>();");
    auto e = p->variable_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::VariableDeclStatement>());
    ASSERT_NE(e->variable, nullptr);
    EXPECT_EQ(e->variable->name->symbol, p->builder().Symbols().Get("a"));

    ASSERT_NE(e->variable->initializer, nullptr);
    auto* call = e->variable->initializer->As<ast::CallExpression>();
    ASSERT_NE(call, nullptr);
    ast::CheckIdentifier(call->target, ast::Template("array", "i32"));
}

TEST_F(ParserImplTest, VariableStmt_VariableDecl_VecInit) {
    auto p = parser("var a : vec2<i32> = vec2<i32>();");
    auto e = p->variable_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::VariableDeclStatement>());
    ASSERT_NE(e->variable, nullptr);
    EXPECT_EQ(e->variable->name->symbol, p->builder().Symbols().Get("a"));

    ASSERT_NE(e->variable->initializer, nullptr);
    auto* call = e->variable->initializer->As<ast::CallExpression>();
    ast::CheckIdentifier(call->target, ast::Template("vec2", "i32"));
}

TEST_F(ParserImplTest, VariableStmt_VariableDecl_VecInit_NoSpace) {
    auto p = parser("var a : vec2<i32>=vec2<i32>();");
    auto e = p->variable_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::VariableDeclStatement>());
    ASSERT_NE(e->variable, nullptr);
    EXPECT_EQ(e->variable->name->symbol, p->builder().Symbols().Get("a"));

    ASSERT_NE(e->variable->initializer, nullptr);
    auto* call = e->variable->initializer->As<ast::CallExpression>();
    ASSERT_NE(call, nullptr);
    ast::CheckIdentifier(call->target, ast::Template("vec2", "i32"));
}

TEST_F(ParserImplTest, VariableStmt_Let) {
    auto p = parser("let a : i32 = 1");
    auto e = p->variable_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::VariableDeclStatement>());

    EXPECT_EQ(e->source.range.begin.line, 1u);
    EXPECT_EQ(e->source.range.begin.column, 1u);
    EXPECT_EQ(e->source.range.end.line, 1u);
    EXPECT_EQ(e->source.range.end.column, 12u);
}

TEST_F(ParserImplTest, VariableStmt_Let_ComplexExpression) {
    auto p = parser("let x = collide + collide_1;");
    // Parse as `statement` to validate the `;` at the end so we know we parsed the whole expression
    auto e = p->statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::VariableDeclStatement>());

    auto* decl = e->As<ast::VariableDeclStatement>();
    ASSERT_NE(decl->variable->initializer, nullptr);

    ASSERT_TRUE(decl->variable->initializer->Is<ast::BinaryExpression>());
    auto* expr = decl->variable->initializer->As<ast::BinaryExpression>();
    EXPECT_EQ(expr->op, ast::BinaryOp::kAdd);

    ASSERT_TRUE(expr->lhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = expr->lhs->As<ast::IdentifierExpression>();
    ast::CheckIdentifier(ident_expr->identifier, "collide");

    ASSERT_TRUE(expr->rhs->Is<ast::IdentifierExpression>());
    ident_expr = expr->rhs->As<ast::IdentifierExpression>();
    ast::CheckIdentifier(ident_expr->identifier, "collide_1");
}

TEST_F(ParserImplTest, VariableStmt_Let_MissingEqual) {
    auto p = parser("let a : i32 1");
    auto e = p->variable_statement();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:13: expected '=' for 'let' declaration");
}

TEST_F(ParserImplTest, VariableStmt_Let_MissingInitializer) {
    auto p = parser("let a : i32 =");
    auto e = p->variable_statement();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:14: missing initializer for 'let' declaration");
}

TEST_F(ParserImplTest, VariableStmt_Let_InvalidInitializer) {
    auto p = parser("let a : i32 = if (a) {}");
    auto e = p->variable_statement();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:15: missing initializer for 'let' declaration");
}

}  // namespace
}  // namespace tint::reader::wgsl
