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

#include "src/tint/ast/stage_attribute.h"
#include "src/tint/ast/test_helper.h"

namespace tint::ast {
namespace {

using WorkgroupAttributeTest = TestHelper;

TEST_F(WorkgroupAttributeTest, Creation_1param) {
    auto* d = WorkgroupSize(2);
    auto values = d->Values();

    ASSERT_TRUE(values[0]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->value, 2);

    EXPECT_EQ(values[1], nullptr);
    EXPECT_EQ(values[2], nullptr);
}
TEST_F(WorkgroupAttributeTest, Creation_2param) {
    auto* d = WorkgroupSize(2, 4);
    auto values = d->Values();

    ASSERT_TRUE(values[0]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->value, 2);

    ASSERT_TRUE(values[1]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[1]->As<ast::IntLiteralExpression>()->value, 4);

    EXPECT_EQ(values[2], nullptr);
}

TEST_F(WorkgroupAttributeTest, Creation_3param) {
    auto* d = WorkgroupSize(2, 4, 6);
    auto values = d->Values();

    ASSERT_TRUE(values[0]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->value, 2);

    ASSERT_TRUE(values[1]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[1]->As<ast::IntLiteralExpression>()->value, 4);

    ASSERT_TRUE(values[2]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[2]->As<ast::IntLiteralExpression>()->value, 6);
}

TEST_F(WorkgroupAttributeTest, Creation_WithIdentifier) {
    auto* d = WorkgroupSize(2, 4, "depth");
    auto values = d->Values();

    ASSERT_TRUE(values[0]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->value, 2);

    ASSERT_TRUE(values[1]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[1]->As<ast::IntLiteralExpression>()->value, 4);

    auto* z_ident = As<ast::IdentifierExpression>(values[2]);
    ASSERT_TRUE(z_ident);
    EXPECT_EQ(Symbols().NameFor(z_ident->symbol), "depth");
}

}  // namespace
}  // namespace tint::ast
