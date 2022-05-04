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

#include "src/tint/ast/stage_attribute.h"
#include "src/tint/ast/workgroup_attribute.h"
#include "src/tint/reader/wgsl/parser_impl_test_helper.h"

namespace tint::reader::wgsl {
namespace {

TEST_F(ParserImplTest, Attribute_Workgroup) {
    auto p = parser("workgroup_size(4)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    auto* func_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(func_attr, nullptr);
    ASSERT_TRUE(func_attr->Is<ast::WorkgroupAttribute>());

    auto values = func_attr->As<ast::WorkgroupAttribute>()->Values();

    ASSERT_TRUE(values[0]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->value, 4);
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    EXPECT_EQ(values[1], nullptr);
    EXPECT_EQ(values[2], nullptr);
}

TEST_F(ParserImplTest, Attribute_Workgroup_2Param) {
    auto p = parser("workgroup_size(4, 5)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    auto* func_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(func_attr, nullptr) << p->error();
    ASSERT_TRUE(func_attr->Is<ast::WorkgroupAttribute>());

    auto values = func_attr->As<ast::WorkgroupAttribute>()->Values();

    ASSERT_TRUE(values[0]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->value, 4);
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    ASSERT_TRUE(values[1]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[1]->As<ast::IntLiteralExpression>()->value, 5);
    EXPECT_EQ(values[1]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    EXPECT_EQ(values[2], nullptr);
}

TEST_F(ParserImplTest, Attribute_Workgroup_3Param) {
    auto p = parser("workgroup_size(4, 5, 6)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    auto* func_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(func_attr, nullptr);
    ASSERT_TRUE(func_attr->Is<ast::WorkgroupAttribute>());

    auto values = func_attr->As<ast::WorkgroupAttribute>()->Values();

    ASSERT_TRUE(values[0]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->value, 4);
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    ASSERT_TRUE(values[1]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[1]->As<ast::IntLiteralExpression>()->value, 5);
    EXPECT_EQ(values[1]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    ASSERT_TRUE(values[2]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[2]->As<ast::IntLiteralExpression>()->value, 6);
    EXPECT_EQ(values[2]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(ParserImplTest, Attribute_Workgroup_WithIdent) {
    auto p = parser("workgroup_size(4, height)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    auto* func_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(func_attr, nullptr);
    ASSERT_TRUE(func_attr->Is<ast::WorkgroupAttribute>());

    auto values = func_attr->As<ast::WorkgroupAttribute>()->Values();

    ASSERT_TRUE(values[0]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->value, 4);
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    ASSERT_NE(values[1], nullptr);
    auto* y_ident = values[1]->As<ast::IdentifierExpression>();
    ASSERT_NE(y_ident, nullptr);
    EXPECT_EQ(p->builder().Symbols().NameFor(y_ident->symbol), "height");

    ASSERT_EQ(values[2], nullptr);
}

TEST_F(ParserImplTest, Attribute_Workgroup_TooManyValues) {
    auto p = parser("workgroup_size(1, 2, 3, 4)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:23: expected ')' for workgroup_size attribute");
}

TEST_F(ParserImplTest, Attribute_Workgroup_MissingLeftParam) {
    auto p = parser("workgroup_size 4, 5, 6)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:16: expected '(' for workgroup_size attribute");
}

TEST_F(ParserImplTest, Attribute_Workgroup_MissingRightParam) {
    auto p = parser("workgroup_size(4, 5, 6");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:23: expected ')' for workgroup_size attribute");
}

TEST_F(ParserImplTest, Attribute_Workgroup_MissingValues) {
    auto p = parser("workgroup_size()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:16: expected workgroup_size x parameter");
}

TEST_F(ParserImplTest, Attribute_Workgroup_Missing_X_Value) {
    auto p = parser("workgroup_size(, 2, 3)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:16: expected workgroup_size x parameter");
}

TEST_F(ParserImplTest, Attribute_Workgroup_Missing_Y_Comma) {
    auto p = parser("workgroup_size(1 2, 3)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:18: expected ')' for workgroup_size attribute");
}

TEST_F(ParserImplTest, Attribute_Workgroup_Missing_Y_Value) {
    auto p = parser("workgroup_size(1, , 3)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:19: expected workgroup_size y parameter");
}

TEST_F(ParserImplTest, Attribute_Workgroup_Missing_Z_Comma) {
    auto p = parser("workgroup_size(1, 2 3)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:21: expected ')' for workgroup_size attribute");
}

TEST_F(ParserImplTest, Attribute_Workgroup_Missing_Z_Value) {
    auto p = parser("workgroup_size(1, 2, )");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:22: expected workgroup_size z parameter");
}

TEST_F(ParserImplTest, Attribute_Stage) {
    auto p = parser("stage(compute)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    auto* func_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(func_attr, nullptr);
    ASSERT_TRUE(func_attr->Is<ast::StageAttribute>());
    EXPECT_EQ(func_attr->As<ast::StageAttribute>()->stage, ast::PipelineStage::kCompute);
}

TEST_F(ParserImplTest, Attribute_Stage_MissingValue) {
    auto p = parser("stage()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:7: invalid value for stage attribute");
}

TEST_F(ParserImplTest, Attribute_Stage_MissingInvalid) {
    auto p = parser("stage(nan)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:7: invalid value for stage attribute");
}

TEST_F(ParserImplTest, Attribute_Stage_MissingLeftParen) {
    auto p = parser("stage compute)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:7: expected '(' for stage attribute");
}

TEST_F(ParserImplTest, Attribute_Stage_MissingRightParen) {
    auto p = parser("stage(compute");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:14: expected ')' for stage attribute");
}

}  // namespace
}  // namespace tint::reader::wgsl
