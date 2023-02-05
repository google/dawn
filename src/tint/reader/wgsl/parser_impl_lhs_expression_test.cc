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

TEST_F(ParserImplTest, LHSExpression_NoPrefix) {
    auto p = parser("a");
    auto e = p->lhs_expression();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    EXPECT_TRUE(e.matched);
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::IdentifierExpression>());
}

TEST_F(ParserImplTest, LHSExpression_NoMatch) {
    auto p = parser("123");
    auto e = p->lhs_expression();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    EXPECT_FALSE(e.matched);
    ASSERT_EQ(e.value, nullptr);
}

TEST_F(ParserImplTest, LHSExpression_And) {
    auto p = parser("&a");
    auto e = p->lhs_expression();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    EXPECT_TRUE(e.matched);
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::UnaryOpExpression>());

    auto* u = e->As<ast::UnaryOpExpression>();
    EXPECT_EQ(u->op, ast::UnaryOp::kAddressOf);
    EXPECT_TRUE(u->expr->Is<ast::IdentifierExpression>());
}

TEST_F(ParserImplTest, LHSExpression_Star) {
    auto p = parser("*a");
    auto e = p->lhs_expression();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    EXPECT_TRUE(e.matched);
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::UnaryOpExpression>());

    auto* u = e->As<ast::UnaryOpExpression>();
    EXPECT_EQ(u->op, ast::UnaryOp::kIndirection);
    EXPECT_TRUE(u->expr->Is<ast::IdentifierExpression>());
}

TEST_F(ParserImplTest, LHSExpression_InvalidCoreLHSExpr) {
    auto p = parser("*123");
    auto e = p->lhs_expression();
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    ASSERT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:2: missing expression");
}

TEST_F(ParserImplTest, LHSExpression_Multiple) {
    auto p = parser("*&********&&&&&&*a");
    auto e = p->lhs_expression();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    EXPECT_TRUE(e.matched);
    ASSERT_NE(e.value, nullptr);

    std::vector<ast::UnaryOp> results = {
        ast::UnaryOp::kIndirection, ast::UnaryOp::kAddressOf,   ast::UnaryOp::kIndirection,
        ast::UnaryOp::kIndirection, ast::UnaryOp::kIndirection, ast::UnaryOp::kIndirection,
        ast::UnaryOp::kIndirection, ast::UnaryOp::kIndirection, ast::UnaryOp::kIndirection,
        ast::UnaryOp::kIndirection, ast::UnaryOp::kAddressOf,   ast::UnaryOp::kAddressOf,
        ast::UnaryOp::kAddressOf,   ast::UnaryOp::kAddressOf,   ast::UnaryOp::kAddressOf,
        ast::UnaryOp::kAddressOf,   ast::UnaryOp::kIndirection};

    auto* expr = e.value;
    for (auto op : results) {
        ASSERT_TRUE(expr->Is<ast::UnaryOpExpression>());

        auto* u = expr->As<ast::UnaryOpExpression>();
        EXPECT_EQ(u->op, op);

        expr = u->expr;
    }

    EXPECT_TRUE(expr->Is<ast::IdentifierExpression>());
}

TEST_F(ParserImplTest, LHSExpression_PostfixExpression_Array) {
    auto p = parser("*a[0]");
    auto e = p->lhs_expression();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    EXPECT_TRUE(e.matched);
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::UnaryOpExpression>());

    auto* u = e->As<ast::UnaryOpExpression>();
    EXPECT_EQ(u->op, ast::UnaryOp::kIndirection);

    ASSERT_TRUE(u->expr->Is<ast::IndexAccessorExpression>());

    auto* access = u->expr->As<ast::IndexAccessorExpression>();
    ASSERT_TRUE(access->object->Is<ast::IdentifierExpression>());

    auto* obj = access->object->As<ast::IdentifierExpression>();
    EXPECT_EQ(obj->identifier->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(access->index->Is<ast::IntLiteralExpression>());
    auto* idx = access->index->As<ast::IntLiteralExpression>();
    EXPECT_EQ(0, idx->value);
}

TEST_F(ParserImplTest, LHSExpression_PostfixExpression) {
    auto p = parser("*a.foo");
    auto e = p->lhs_expression();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    EXPECT_TRUE(e.matched);
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::UnaryOpExpression>());

    auto* u = e->As<ast::UnaryOpExpression>();
    EXPECT_EQ(u->op, ast::UnaryOp::kIndirection);

    ASSERT_TRUE(u->expr->Is<ast::MemberAccessorExpression>());

    auto* access = u->expr->As<ast::MemberAccessorExpression>();
    ASSERT_TRUE(access->object->Is<ast::IdentifierExpression>());

    auto* struct_ident = access->object->As<ast::IdentifierExpression>();
    EXPECT_EQ(struct_ident->identifier->symbol, p->builder().Symbols().Get("a"));
    EXPECT_EQ(access->member->symbol, p->builder().Symbols().Get("foo"));
}

TEST_F(ParserImplTest, LHSExpression_InvalidPostfixExpression) {
    auto p = parser("*a.if");
    auto e = p->lhs_expression();
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    ASSERT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:4: expected identifier for member accessor");
}

}  // namespace
}  // namespace tint::reader::wgsl
