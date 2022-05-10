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

#include "src/tint/reader/wgsl/parser_impl_test_helper.h"

namespace tint::reader::wgsl {
namespace {

TEST_F(ParserImplTest, ConstExpr_TypeDecl) {
    auto p = parser("vec2<f32>(1., 2.)");
    auto e = p->expect_const_expr();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    ASSERT_TRUE(e->Is<ast::CallExpression>());

    auto* t = e->As<ast::CallExpression>();
    ASSERT_TRUE(t->target.type->Is<ast::Vector>());
    EXPECT_EQ(t->target.type->As<ast::Vector>()->width, 2u);

    ASSERT_EQ(t->args.size(), 2u);

    ASSERT_TRUE(t->args[0]->Is<ast::FloatLiteralExpression>());
    EXPECT_DOUBLE_EQ(t->args[0]->As<ast::FloatLiteralExpression>()->value, 1.);
    EXPECT_EQ(t->args[0]->As<ast::FloatLiteralExpression>()->suffix,
              ast::FloatLiteralExpression::Suffix::kNone);

    ASSERT_TRUE(t->args[1]->Is<ast::FloatLiteralExpression>());
    EXPECT_DOUBLE_EQ(t->args[1]->As<ast::FloatLiteralExpression>()->value, 2.);
    EXPECT_EQ(t->args[1]->As<ast::FloatLiteralExpression>()->suffix,
              ast::FloatLiteralExpression::Suffix::kNone);
}

TEST_F(ParserImplTest, ConstExpr_TypeDecl_Empty) {
    auto p = parser("vec2<f32>()");
    auto e = p->expect_const_expr();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    ASSERT_TRUE(e->Is<ast::CallExpression>());

    auto* t = e->As<ast::CallExpression>();
    ASSERT_TRUE(t->target.type->Is<ast::Vector>());
    EXPECT_EQ(t->target.type->As<ast::Vector>()->width, 2u);

    ASSERT_EQ(t->args.size(), 0u);
}

TEST_F(ParserImplTest, ConstExpr_TypeDecl_TrailingComma) {
    auto p = parser("vec2<f32>(1., 2.,)");
    auto e = p->expect_const_expr();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    ASSERT_TRUE(e->Is<ast::CallExpression>());

    auto* t = e->As<ast::CallExpression>();
    ASSERT_TRUE(t->target.type->Is<ast::Vector>());
    EXPECT_EQ(t->target.type->As<ast::Vector>()->width, 2u);

    ASSERT_EQ(t->args.size(), 2u);
    ASSERT_TRUE(t->args[0]->Is<ast::LiteralExpression>());
    ASSERT_TRUE(t->args[1]->Is<ast::LiteralExpression>());
}

TEST_F(ParserImplTest, ConstExpr_TypeDecl_MissingRightParen) {
    auto p = parser("vec2<f32>(1., 2.");
    auto e = p->expect_const_expr();
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    ASSERT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:17: expected ')' for type constructor");
}

TEST_F(ParserImplTest, ConstExpr_TypeDecl_MissingLeftParen) {
    auto p = parser("vec2<f32> 1., 2.)");
    auto e = p->expect_const_expr();
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    ASSERT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:11: expected '(' for type constructor");
}

TEST_F(ParserImplTest, ConstExpr_TypeDecl_MissingComma) {
    auto p = parser("vec2<f32>(1. 2.");
    auto e = p->expect_const_expr();
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    ASSERT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:14: expected ')' for type constructor");
}

TEST_F(ParserImplTest, ConstExpr_InvalidExpr) {
    auto p = parser("vec2<f32>(1., if(a) {})");
    auto e = p->expect_const_expr();
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    ASSERT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:15: invalid type for const_expr");
}

TEST_F(ParserImplTest, ConstExpr_ConstLiteral) {
    auto p = parser("true");
    auto e = p->expect_const_expr();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e.value->Is<ast::BoolLiteralExpression>());
    EXPECT_TRUE(e.value->As<ast::BoolLiteralExpression>()->value);
}

TEST_F(ParserImplTest, ConstExpr_ConstLiteral_Invalid) {
    auto p = parser("invalid");
    auto e = p->expect_const_expr();
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    ASSERT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:1: unable to parse const_expr");
}

TEST_F(ParserImplTest, ConstExpr_TypeConstructor) {
    auto p = parser("S(0)");

    auto e = p->expect_const_expr();
    ASSERT_FALSE(e.errored);
    ASSERT_TRUE(e->Is<ast::CallExpression>());
    ASSERT_NE(e->As<ast::CallExpression>()->target.type, nullptr);
    ASSERT_TRUE(e->As<ast::CallExpression>()->target.type->Is<ast::TypeName>());
    EXPECT_EQ(e->As<ast::CallExpression>()->target.type->As<ast::TypeName>()->name,
              p->builder().Symbols().Get("S"));
}

TEST_F(ParserImplTest, ConstExpr_Recursion) {
    std::stringstream out;
    for (size_t i = 0; i < 200; i++) {
        out << "f32(";
    }
    out << "1.0";
    for (size_t i = 0; i < 200; i++) {
        out << ")";
    }
    auto p = parser(out.str());
    auto e = p->expect_const_expr();
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    ASSERT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:517: maximum parser recursive depth reached");
}

TEST_F(ParserImplTest, UnaryOp_Recursion) {
    std::stringstream out;
    for (size_t i = 0; i < 200; i++) {
        out << "!";
    }
    out << "1.0";
    auto p = parser(out.str());
    auto e = p->unary_expression();
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    ASSERT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:130: maximum parser recursive depth reached");
}

}  // namespace
}  // namespace tint::reader::wgsl
