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

TEST_F(ParserImplTest, Attribute_Size) {
    auto p = parser("size(4)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    ASSERT_FALSE(p->has_error()) << p->error();

    auto* member_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(member_attr, nullptr);
    ASSERT_TRUE(member_attr->Is<ast::StructMemberSizeAttribute>());

    auto* o = member_attr->As<ast::StructMemberSizeAttribute>();
    ASSERT_TRUE(o->expr->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(o->expr->As<ast::IntLiteralExpression>()->value, 4u);
}

TEST_F(ParserImplTest, Attribute_Size_Expression) {
    auto p = parser("size(4 + 5)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    ASSERT_FALSE(p->has_error()) << p->error();

    auto* member_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(member_attr, nullptr);
    ASSERT_TRUE(member_attr->Is<ast::StructMemberSizeAttribute>());

    auto* o = member_attr->As<ast::StructMemberSizeAttribute>();
    ASSERT_TRUE(o->expr->Is<ast::BinaryExpression>());
    auto* expr = o->expr->As<ast::BinaryExpression>();

    EXPECT_EQ(ast::BinaryOp::kAdd, expr->op);
    auto* v = expr->lhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 4u);

    v = expr->rhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 5u);
}

TEST_F(ParserImplTest, Attribute_Size_TrailingComma) {
    auto p = parser("size(4,)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    ASSERT_FALSE(p->has_error()) << p->error();

    auto* member_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(member_attr, nullptr);
    ASSERT_TRUE(member_attr->Is<ast::StructMemberSizeAttribute>());

    auto* o = member_attr->As<ast::StructMemberSizeAttribute>();
    ASSERT_TRUE(o->expr->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(o->expr->As<ast::IntLiteralExpression>()->value, 4u);
}

TEST_F(ParserImplTest, Attribute_Size_MissingLeftParen) {
    auto p = parser("size 4)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:6: expected '(' for size attribute");
}

TEST_F(ParserImplTest, Attribute_Size_MissingRightParen) {
    auto p = parser("size(4");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:7: expected ')' for size attribute");
}

TEST_F(ParserImplTest, Attribute_Size_MissingValue) {
    auto p = parser("size()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:1: size expects 1 argument");
}

TEST_F(ParserImplTest, Attribute_Size_MissingInvalid) {
    auto p = parser("size(if)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:6: expected expression for size");
}

TEST_F(ParserImplTest, Attribute_Align) {
    auto p = parser("align(4)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    ASSERT_FALSE(p->has_error());

    auto* member_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(member_attr, nullptr);
    ASSERT_TRUE(member_attr->Is<ast::StructMemberAlignAttribute>());

    auto* o = member_attr->As<ast::StructMemberAlignAttribute>();
    ASSERT_TRUE(o->expr->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(o->expr->As<ast::IntLiteralExpression>()->value, 4);
    EXPECT_EQ(o->expr->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(ParserImplTest, Attribute_Align_Expression) {
    auto p = parser("align(4 + 5)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(attr.value, nullptr);

    auto* member_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(member_attr, nullptr);
    ASSERT_TRUE(member_attr->Is<ast::StructMemberAlignAttribute>());

    auto* o = member_attr->As<ast::StructMemberAlignAttribute>();
    ASSERT_TRUE(o->expr->Is<ast::BinaryExpression>());
    auto* expr = o->expr->As<ast::BinaryExpression>();

    EXPECT_EQ(ast::BinaryOp::kAdd, expr->op);
    auto* v = expr->lhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 4u);

    v = expr->rhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 5u);
}

TEST_F(ParserImplTest, Attribute_Align_TrailingComma) {
    auto p = parser("align(4,)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    ASSERT_FALSE(p->has_error()) << p->error();

    auto* member_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(member_attr, nullptr);
    ASSERT_TRUE(member_attr->Is<ast::StructMemberAlignAttribute>());

    auto* o = member_attr->As<ast::StructMemberAlignAttribute>();
    ASSERT_TRUE(o->expr->Is<ast::IntLiteralExpression>());

    auto* expr = o->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(expr->value, 4);
    EXPECT_EQ(expr->suffix, ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(ParserImplTest, Attribute_Align_MissingLeftParen) {
    auto p = parser("align 4)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:7: expected '(' for align attribute");
}

TEST_F(ParserImplTest, Attribute_Align_MissingRightParen) {
    auto p = parser("align(4");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:8: expected ')' for align attribute");
}

TEST_F(ParserImplTest, Attribute_Align_MissingValue) {
    auto p = parser("align()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:1: align expects 1 argument");
}

TEST_F(ParserImplTest, Attribute_Align_ExpressionInvalid) {
    auto p = parser("align(4 + 5 << 6)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());

    EXPECT_EQ(p->error(), "1:9: mixing '+' and '<<' requires parenthesis");
}

}  // namespace
}  // namespace tint::reader::wgsl
