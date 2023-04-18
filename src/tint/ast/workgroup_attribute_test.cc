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

using namespace tint::number_suffixes;  // NOLINT

namespace tint::ast {
namespace {

using WorkgroupAttributeTest = TestHelper;

TEST_F(WorkgroupAttributeTest, Creation_1param) {
    auto* d = WorkgroupSize(2_i);
    auto values = d->Values();

    ASSERT_TRUE(values[0]->Is<IntLiteralExpression>());
    EXPECT_EQ(values[0]->As<IntLiteralExpression>()->value, 2);

    EXPECT_EQ(values[1], nullptr);
    EXPECT_EQ(values[2], nullptr);
}
TEST_F(WorkgroupAttributeTest, Creation_2param) {
    auto* d = WorkgroupSize(2_i, 4_i);
    auto values = d->Values();

    ASSERT_TRUE(values[0]->Is<IntLiteralExpression>());
    EXPECT_EQ(values[0]->As<IntLiteralExpression>()->value, 2);

    ASSERT_TRUE(values[1]->Is<IntLiteralExpression>());
    EXPECT_EQ(values[1]->As<IntLiteralExpression>()->value, 4);

    EXPECT_EQ(values[2], nullptr);
}

TEST_F(WorkgroupAttributeTest, Creation_3param) {
    auto* d = WorkgroupSize(2_i, 4_i, 6_i);
    auto values = d->Values();

    ASSERT_TRUE(values[0]->Is<IntLiteralExpression>());
    EXPECT_EQ(values[0]->As<IntLiteralExpression>()->value, 2);

    ASSERT_TRUE(values[1]->Is<IntLiteralExpression>());
    EXPECT_EQ(values[1]->As<IntLiteralExpression>()->value, 4);

    ASSERT_TRUE(values[2]->Is<IntLiteralExpression>());
    EXPECT_EQ(values[2]->As<IntLiteralExpression>()->value, 6);
}

TEST_F(WorkgroupAttributeTest, Creation_WithIdentifier) {
    auto* d = WorkgroupSize(2_i, 4_i, "depth");
    auto values = d->Values();

    ASSERT_TRUE(values[0]->Is<IntLiteralExpression>());
    EXPECT_EQ(values[0]->As<IntLiteralExpression>()->value, 2);

    ASSERT_TRUE(values[1]->Is<IntLiteralExpression>());
    EXPECT_EQ(values[1]->As<IntLiteralExpression>()->value, 4);

    auto* z_ident = As<IdentifierExpression>(values[2]);
    ASSERT_TRUE(z_ident);
    EXPECT_EQ(z_ident->identifier->symbol.Name(), "depth");
}

}  // namespace
}  // namespace tint::ast
