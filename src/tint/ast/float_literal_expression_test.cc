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

using FloatLiteralExpressionTest = TestHelper;

TEST_F(FloatLiteralExpressionTest, SuffixNone) {
    auto* i = create<FloatLiteralExpression>(42.0, FloatLiteralExpression::Suffix::kNone);
    ASSERT_TRUE(i->Is<FloatLiteralExpression>());
    EXPECT_EQ(i->value, 42);
    EXPECT_EQ(i->suffix, FloatLiteralExpression::Suffix::kNone);
}

TEST_F(FloatLiteralExpressionTest, SuffixF) {
    auto* i = create<FloatLiteralExpression>(42.0, FloatLiteralExpression::Suffix::kF);
    ASSERT_TRUE(i->Is<FloatLiteralExpression>());
    EXPECT_EQ(i->value, 42);
    EXPECT_EQ(i->suffix, FloatLiteralExpression::Suffix::kF);
}

TEST_F(FloatLiteralExpressionTest, SuffixH) {
    auto* i = create<FloatLiteralExpression>(42.0, FloatLiteralExpression::Suffix::kH);
    ASSERT_TRUE(i->Is<FloatLiteralExpression>());
    EXPECT_EQ(i->value, 42);
    EXPECT_EQ(i->suffix, FloatLiteralExpression::Suffix::kH);
}

TEST_F(FloatLiteralExpressionTest, SuffixStringStream) {
    auto to_str = [](FloatLiteralExpression::Suffix suffix) {
        utils::StringStream ss;
        ss << suffix;
        return ss.str();
    };

    EXPECT_EQ("", to_str(FloatLiteralExpression::Suffix::kNone));
    EXPECT_EQ("f", to_str(FloatLiteralExpression::Suffix::kF));
    EXPECT_EQ("h", to_str(FloatLiteralExpression::Suffix::kH));
}

}  // namespace
}  // namespace tint::ast
