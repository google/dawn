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

#include "src/tint/ast/workgroup_attribute.h"
#include "src/tint/reader/wgsl/parser_impl_test_helper.h"

namespace tint::reader::wgsl {
namespace {

TEST_F(ParserImplTest, AttributeList_Parses) {
    auto p = parser("@workgroup_size(2) @compute");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(attrs.errored);
    EXPECT_TRUE(attrs.matched);
    ASSERT_EQ(attrs.value.Length(), 2u);

    auto* attr_0 = attrs.value[0]->As<ast::Attribute>();
    auto* attr_1 = attrs.value[1]->As<ast::Attribute>();
    ASSERT_NE(attr_0, nullptr);
    ASSERT_NE(attr_1, nullptr);

    ASSERT_TRUE(attr_0->Is<ast::WorkgroupAttribute>());
    const ast::Expression* x = attr_0->As<ast::WorkgroupAttribute>()->x;
    ASSERT_NE(x, nullptr);
    auto* x_literal = x->As<ast::LiteralExpression>();
    ASSERT_NE(x_literal, nullptr);
    ASSERT_TRUE(x_literal->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(x_literal->As<ast::IntLiteralExpression>()->value, 2);
    EXPECT_EQ(x_literal->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);
    ASSERT_TRUE(attr_1->Is<ast::StageAttribute>());
    EXPECT_EQ(attr_1->As<ast::StageAttribute>()->stage, ast::PipelineStage::kCompute);
}

TEST_F(ParserImplTest, AttributeList_Invalid) {
    auto p = parser("@invalid");
    auto attrs = p->attribute_list();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    EXPECT_TRUE(attrs.value.IsEmpty());
    EXPECT_EQ(p->error(), R"(1:2: expected attribute
Did you mean 'invariant'?
Possible values: 'align', 'binding', 'builtin', 'compute', 'diagnostic', 'fragment', 'group', 'id', 'interpolate', 'invariant', 'location', 'must_use', 'size', 'vertex', 'workgroup_size')");
}

}  // namespace
}  // namespace tint::reader::wgsl
