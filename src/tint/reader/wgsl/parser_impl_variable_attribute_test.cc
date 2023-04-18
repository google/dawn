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
#include "src/tint/builtin/builtin_value.h"
#include "src/tint/reader/wgsl/parser_impl_test_helper.h"

namespace tint::reader::wgsl {
namespace {

TEST_F(ParserImplTest, Attribute_Id) {
    auto p = parser("id(4)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::IdAttribute>());

    auto* loc = var_attr->As<ast::IdAttribute>();
    ASSERT_TRUE(loc->expr->Is<ast::IntLiteralExpression>());
    auto* exp = loc->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(exp->value, 4u);
}

TEST_F(ParserImplTest, Attribute_Id_Expression) {
    auto p = parser("id(4 + 5)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::IdAttribute>());

    auto* loc = var_attr->As<ast::IdAttribute>();
    ASSERT_TRUE(loc->expr->Is<ast::BinaryExpression>());
    auto* expr = loc->expr->As<ast::BinaryExpression>();

    EXPECT_EQ(ast::BinaryOp::kAdd, expr->op);
    auto* v = expr->lhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 4u);

    v = expr->rhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 5u);
}

TEST_F(ParserImplTest, Attribute_Id_TrailingComma) {
    auto p = parser("id(4,)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::IdAttribute>());

    auto* loc = var_attr->As<ast::IdAttribute>();
    ASSERT_TRUE(loc->expr->Is<ast::IntLiteralExpression>());
    auto* exp = loc->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(exp->value, 4u);
}

TEST_F(ParserImplTest, Attribute_Id_MissingLeftParen) {
    auto p = parser("id 4)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:4: expected '(' for id attribute");
}

TEST_F(ParserImplTest, Attribute_Id_MissingRightParen) {
    auto p = parser("id(4");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:5: expected ')' for id attribute");
}

TEST_F(ParserImplTest, Attribute_Id_MissingValue) {
    auto p = parser("id()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:1: id expects 1 argument");
}

TEST_F(ParserImplTest, Attribute_Id_MissingInvalid) {
    auto p = parser("id(if)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:4: expected expression for id");
}

TEST_F(ParserImplTest, Attribute_Location) {
    auto p = parser("location(4)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::LocationAttribute>());

    auto* loc = var_attr->As<ast::LocationAttribute>();
    ASSERT_TRUE(loc->expr->Is<ast::IntLiteralExpression>());
    auto* exp = loc->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(exp->value, 4u);
}

TEST_F(ParserImplTest, Attribute_Location_Expression) {
    auto p = parser("location(4 + 5)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::LocationAttribute>());

    auto* loc = var_attr->As<ast::LocationAttribute>();
    ASSERT_TRUE(loc->expr->Is<ast::BinaryExpression>());
    auto* expr = loc->expr->As<ast::BinaryExpression>();

    EXPECT_EQ(ast::BinaryOp::kAdd, expr->op);
    auto* v = expr->lhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 4u);

    v = expr->rhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 5u);
}

TEST_F(ParserImplTest, Attribute_Location_TrailingComma) {
    auto p = parser("location(4,)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::LocationAttribute>());

    auto* loc = var_attr->As<ast::LocationAttribute>();
    ASSERT_TRUE(loc->expr->Is<ast::IntLiteralExpression>());
    auto* exp = loc->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(exp->value, 4u);
}

TEST_F(ParserImplTest, Attribute_Location_MissingLeftParen) {
    auto p = parser("location 4)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:10: expected '(' for location attribute");
}

TEST_F(ParserImplTest, Attribute_Location_MissingRightParen) {
    auto p = parser("location(4");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:11: expected ')' for location attribute");
}

TEST_F(ParserImplTest, Attribute_Location_MissingValue) {
    auto p = parser("location()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:1: location expects 1 argument");
}

TEST_F(ParserImplTest, Attribute_Location_MissingInvalid) {
    auto p = parser("location(if)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:10: expected expression for location");
}

class BuiltinTest : public ParserImplTestWithParam<builtin::BuiltinValue> {};

TEST_P(BuiltinTest, Attribute_Builtin) {
    auto str = utils::ToString(GetParam());
    auto p = parser("builtin(" + str + ")");

    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_TRUE(var_attr->Is<ast::BuiltinAttribute>());

    auto* builtin = var_attr->As<ast::BuiltinAttribute>();
    ast::CheckIdentifier(builtin->builtin, str);
}
TEST_P(BuiltinTest, Attribute_Builtin_TrailingComma) {
    auto str = utils::ToString(GetParam());
    auto p = parser("builtin(" + str + ",)");

    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_TRUE(var_attr->Is<ast::BuiltinAttribute>());

    auto* builtin = var_attr->As<ast::BuiltinAttribute>();
    ast::CheckIdentifier(builtin->builtin, str);
}
INSTANTIATE_TEST_SUITE_P(ParserImplTest,
                         BuiltinTest,
                         testing::Values(builtin::BuiltinValue::kPosition,
                                         builtin::BuiltinValue::kVertexIndex,
                                         builtin::BuiltinValue::kInstanceIndex,
                                         builtin::BuiltinValue::kFrontFacing,
                                         builtin::BuiltinValue::kFragDepth,
                                         builtin::BuiltinValue::kLocalInvocationId,
                                         builtin::BuiltinValue::kLocalInvocationIndex,
                                         builtin::BuiltinValue::kGlobalInvocationId,
                                         builtin::BuiltinValue::kWorkgroupId,
                                         builtin::BuiltinValue::kNumWorkgroups,
                                         builtin::BuiltinValue::kSampleIndex,
                                         builtin::BuiltinValue::kSampleMask));

TEST_F(ParserImplTest, Attribute_Builtin_MissingLeftParen) {
    auto p = parser("builtin position)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:9: expected '(' for builtin attribute");
}

TEST_F(ParserImplTest, Attribute_Builtin_MissingRightParen) {
    auto p = parser("builtin(position");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:17: expected ')' for builtin attribute");
}

TEST_F(ParserImplTest, Attribute_Builtin_MissingValue) {
    auto p = parser("builtin()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:1: builtin expects 1 argument");
}

TEST_F(ParserImplTest, Attribute_Interpolate_Flat) {
    auto p = parser("interpolate(flat)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::InterpolateAttribute>());

    auto* interp = var_attr->As<ast::InterpolateAttribute>();
    ast::CheckIdentifier(interp->type, "flat");
    EXPECT_EQ(interp->sampling, nullptr);
}

TEST_F(ParserImplTest, Attribute_Interpolate_Single_TrailingComma) {
    auto p = parser("interpolate(flat,)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::InterpolateAttribute>());

    auto* interp = var_attr->As<ast::InterpolateAttribute>();
    ast::CheckIdentifier(interp->type, "flat");
    EXPECT_EQ(interp->sampling, nullptr);
}

TEST_F(ParserImplTest, Attribute_Interpolate_Single_DoubleTrailingComma) {
    auto p = parser("interpolate(flat,,)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:18: expected expression for interpolate");
}

TEST_F(ParserImplTest, Attribute_Interpolate_Perspective_Center) {
    auto p = parser("interpolate(perspective, center)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::InterpolateAttribute>());

    auto* interp = var_attr->As<ast::InterpolateAttribute>();
    ast::CheckIdentifier(interp->type, "perspective");
    ast::CheckIdentifier(interp->sampling, "center");
}

TEST_F(ParserImplTest, Attribute_Interpolate_Double_TrailingComma) {
    auto p = parser("interpolate(perspective, center,)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::InterpolateAttribute>());

    auto* interp = var_attr->As<ast::InterpolateAttribute>();
    ast::CheckIdentifier(interp->type, "perspective");
    ast::CheckIdentifier(interp->sampling, "center");
}

TEST_F(ParserImplTest, Attribute_Interpolate_Perspective_Centroid) {
    auto p = parser("interpolate(perspective, centroid)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::InterpolateAttribute>());

    auto* interp = var_attr->As<ast::InterpolateAttribute>();
    ast::CheckIdentifier(interp->type, "perspective");
    ast::CheckIdentifier(interp->sampling, "centroid");
}

TEST_F(ParserImplTest, Attribute_Interpolate_Linear_Sample) {
    auto p = parser("interpolate(linear, sample)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::InterpolateAttribute>());

    auto* interp = var_attr->As<ast::InterpolateAttribute>();
    ast::CheckIdentifier(interp->type, "linear");
    ast::CheckIdentifier(interp->sampling, "sample");
}

TEST_F(ParserImplTest, Attribute_Interpolate_MissingLeftParen) {
    auto p = parser("interpolate flat)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:13: expected '(' for interpolate attribute");
}

TEST_F(ParserImplTest, Attribute_Interpolate_MissingRightParen) {
    auto p = parser("interpolate(flat");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:17: expected ')' for interpolate attribute");
}

TEST_F(ParserImplTest, Attribute_Interpolate_MissingFirstValue) {
    auto p = parser("interpolate()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:1: interpolate expects at least 1 argument");
}

TEST_F(ParserImplTest, Attribute_Binding) {
    auto p = parser("binding(4)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::BindingAttribute>());

    auto* binding = var_attr->As<ast::BindingAttribute>();
    ASSERT_TRUE(binding->expr->Is<ast::IntLiteralExpression>());
    auto* expr = binding->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(expr->value, 4);
    EXPECT_EQ(expr->suffix, ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(ParserImplTest, Attribute_Binding_Expression) {
    auto p = parser("binding(4 + 5)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::BindingAttribute>());

    auto* binding = var_attr->As<ast::BindingAttribute>();
    ASSERT_TRUE(binding->expr->Is<ast::BinaryExpression>());
    auto* expr = binding->expr->As<ast::BinaryExpression>();

    EXPECT_EQ(ast::BinaryOp::kAdd, expr->op);
    auto* v = expr->lhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 4u);

    v = expr->rhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 5u);
}

TEST_F(ParserImplTest, Attribute_Binding_TrailingComma) {
    auto p = parser("binding(4,)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::BindingAttribute>());

    auto* binding = var_attr->As<ast::BindingAttribute>();
    ASSERT_TRUE(binding->expr->Is<ast::IntLiteralExpression>());
    auto* expr = binding->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(expr->value, 4);
    EXPECT_EQ(expr->suffix, ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(ParserImplTest, Attribute_Binding_MissingLeftParen) {
    auto p = parser("binding 4)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:9: expected '(' for binding attribute");
}

TEST_F(ParserImplTest, Attribute_Binding_MissingRightParen) {
    auto p = parser("binding(4");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:10: expected ')' for binding attribute");
}

TEST_F(ParserImplTest, Attribute_Binding_MissingValue) {
    auto p = parser("binding()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:1: binding expects 1 argument");
}

TEST_F(ParserImplTest, Attribute_Binding_MissingInvalid) {
    auto p = parser("binding(if)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:9: expected expression for binding");
}

TEST_F(ParserImplTest, Attribute_group) {
    auto p = parser("group(4)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_FALSE(p->has_error());
    ASSERT_NE(var_attr, nullptr);
    ASSERT_TRUE(var_attr->Is<ast::GroupAttribute>());

    auto* group = var_attr->As<ast::GroupAttribute>();
    ASSERT_TRUE(group->expr->Is<ast::IntLiteralExpression>());
    auto* expr = group->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(expr->value, 4);
    EXPECT_EQ(expr->suffix, ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(ParserImplTest, Attribute_group_expression) {
    auto p = parser("group(4 + 5)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_FALSE(p->has_error());
    ASSERT_NE(var_attr, nullptr);
    ASSERT_TRUE(var_attr->Is<ast::GroupAttribute>());

    auto* group = var_attr->As<ast::GroupAttribute>();
    ASSERT_TRUE(group->expr->Is<ast::BinaryExpression>());
    auto* expr = group->expr->As<ast::BinaryExpression>();

    EXPECT_EQ(ast::BinaryOp::kAdd, expr->op);
    auto* v = expr->lhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 4u);

    v = expr->rhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 5u);
}

TEST_F(ParserImplTest, Attribute_group_TrailingComma) {
    auto p = parser("group(4,)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_FALSE(p->has_error());
    ASSERT_NE(var_attr, nullptr);
    ASSERT_TRUE(var_attr->Is<ast::GroupAttribute>());

    auto* group = var_attr->As<ast::GroupAttribute>();
    ASSERT_TRUE(group->expr->Is<ast::IntLiteralExpression>());
    auto* expr = group->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(expr->value, 4);
    EXPECT_EQ(expr->suffix, ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(ParserImplTest, Attribute_Group_MissingLeftParen) {
    auto p = parser("group 2)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:7: expected '(' for group attribute");
}

TEST_F(ParserImplTest, Attribute_Group_MissingRightParen) {
    auto p = parser("group(2");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:8: expected ')' for group attribute");
}

TEST_F(ParserImplTest, Attribute_Group_MissingValue) {
    auto p = parser("group()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:1: group expects 1 argument");
}

TEST_F(ParserImplTest, Attribute_Group_MissingInvalid) {
    auto p = parser("group(if)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:7: expected expression for group");
}

}  // namespace
}  // namespace tint::reader::wgsl
