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

#include "src/tint/utils/string_stream.h"

namespace tint::ast {
namespace {

using IntLiteralExpressionTest = TestHelper;

TEST_F(IntLiteralExpressionTest, SuffixNone) {
    auto* i = create<IntLiteralExpression>(42, IntLiteralExpression::Suffix::kNone);
    ASSERT_TRUE(i->Is<IntLiteralExpression>());
    EXPECT_EQ(i->value, 42);
    EXPECT_EQ(i->suffix, IntLiteralExpression::Suffix::kNone);
}

TEST_F(IntLiteralExpressionTest, SuffixI) {
    auto* i = create<IntLiteralExpression>(42, IntLiteralExpression::Suffix::kI);
    ASSERT_TRUE(i->Is<IntLiteralExpression>());
    EXPECT_EQ(i->value, 42);
    EXPECT_EQ(i->suffix, IntLiteralExpression::Suffix::kI);
}

TEST_F(IntLiteralExpressionTest, SuffixU) {
    auto* i = create<IntLiteralExpression>(42, IntLiteralExpression::Suffix::kU);
    ASSERT_TRUE(i->Is<IntLiteralExpression>());
    EXPECT_EQ(i->value, 42);
    EXPECT_EQ(i->suffix, IntLiteralExpression::Suffix::kU);
}

TEST_F(IntLiteralExpressionTest, SuffixStringStream) {
    auto to_str = [](IntLiteralExpression::Suffix suffix) {
        utils::StringStream ss;
        ss << suffix;
        return ss.str();
    };

    EXPECT_EQ("", to_str(IntLiteralExpression::Suffix::kNone));
    EXPECT_EQ("i", to_str(IntLiteralExpression::Suffix::kI));
    EXPECT_EQ("u", to_str(IntLiteralExpression::Suffix::kU));
}

}  // namespace
}  // namespace tint::ast
