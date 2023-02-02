// Copyright 2022 The Tint Authors.
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

#include "src/tint/reader/wgsl/parser_impl_test_helper.h"

namespace tint::reader::wgsl {
namespace {

TEST_F(ParserImplTest, CoreLHS_NoMatch) {
    auto p = parser("123");
    auto e = p->core_lhs_expression();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    EXPECT_FALSE(e.matched);
}

TEST_F(ParserImplTest, CoreLHS_Ident) {
    auto p = parser("identifier");
    auto e = p->core_lhs_expression();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    EXPECT_TRUE(e.matched);
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::IdentifierExpression>());

    auto* ident_expr = e->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("identifier"));
}

TEST_F(ParserImplTest, CoreLHS_ParenStmt) {
    auto p = parser("(a)");
    auto e = p->core_lhs_expression();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    EXPECT_TRUE(e.matched);
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::IdentifierExpression>());

    auto* ident_expr = e->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));
}

TEST_F(ParserImplTest, CoreLHS_MissingRightParen) {
    auto p = parser("(a");
    auto e = p->core_lhs_expression();
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    ASSERT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:3: expected ')'");
}

TEST_F(ParserImplTest, CoreLHS_InvalidLHSExpression) {
    auto p = parser("(if (a() {})");
    auto e = p->core_lhs_expression();
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    ASSERT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:1: invalid expression");
}

TEST_F(ParserImplTest, CoreLHS_MissingLHSExpression) {
    auto p = parser("()");
    auto e = p->core_lhs_expression();
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    ASSERT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:1: invalid expression");
}

TEST_F(ParserImplTest, CoreLHS_Invalid) {
    auto p = parser("1234");
    auto e = p->core_lhs_expression();
    ASSERT_FALSE(p->has_error());
    ASSERT_FALSE(e.errored);
    EXPECT_FALSE(e.matched);
}

}  // namespace
}  // namespace tint::reader::wgsl
