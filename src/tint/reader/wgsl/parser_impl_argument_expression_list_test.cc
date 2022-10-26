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

TEST_F(ParserImplTest, ArgumentExpressionList_Parses) {
    auto p = parser("(a)");
    auto e = p->expect_argument_expression_list("argument list");
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);

    ASSERT_EQ(e.value.Length(), 1u);
    ASSERT_TRUE(e.value[0]->Is<ast::IdentifierExpression>());
}

TEST_F(ParserImplTest, ArgumentExpressionList_ParsesEmptyList) {
    auto p = parser("()");
    auto e = p->expect_argument_expression_list("argument list");
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);

    ASSERT_EQ(e.value.Length(), 0u);
}

TEST_F(ParserImplTest, ArgumentExpressionList_ParsesMultiple) {
    auto p = parser("(a, 33, 1+2)");
    auto e = p->expect_argument_expression_list("argument list");
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);

    ASSERT_EQ(e.value.Length(), 3u);
    ASSERT_TRUE(e.value[0]->Is<ast::IdentifierExpression>());
    ASSERT_TRUE(e.value[1]->Is<ast::LiteralExpression>());
    ASSERT_TRUE(e.value[2]->Is<ast::BinaryExpression>());
}

TEST_F(ParserImplTest, ArgumentExpressionList_TrailingComma) {
    auto p = parser("(a, 42,)");
    auto e = p->expect_argument_expression_list("argument list");
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);

    ASSERT_EQ(e.value.Length(), 2u);
    ASSERT_TRUE(e.value[0]->Is<ast::IdentifierExpression>());
    ASSERT_TRUE(e.value[1]->Is<ast::LiteralExpression>());
}

TEST_F(ParserImplTest, ArgumentExpressionList_HandlesMissingLeftParen) {
    auto p = parser("a)");
    auto e = p->expect_argument_expression_list("argument list");
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    EXPECT_EQ(p->error(), "1:1: expected '(' for argument list");
}

TEST_F(ParserImplTest, ArgumentExpressionList_HandlesMissingRightParen) {
    auto p = parser("(a");
    auto e = p->expect_argument_expression_list("argument list");
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    EXPECT_EQ(p->error(), "1:3: expected ')' for argument list");
}

TEST_F(ParserImplTest, ArgumentExpressionList_HandlesMissingExpression_0) {
    auto p = parser("(,)");
    auto e = p->expect_argument_expression_list("argument list");
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    EXPECT_EQ(p->error(), "1:2: expected ')' for argument list");
}

TEST_F(ParserImplTest, ArgumentExpressionList_HandlesMissingExpression_1) {
    auto p = parser("(a, ,)");
    auto e = p->expect_argument_expression_list("argument list");
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    EXPECT_EQ(p->error(), "1:5: expected ')' for argument list");
}

TEST_F(ParserImplTest, ArgumentExpressionList_HandlesInvalidExpression) {
    auto p = parser("(if(a) {})");
    auto e = p->expect_argument_expression_list("argument list");
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    EXPECT_EQ(p->error(), "1:2: expected ')' for argument list");
}

}  // namespace
}  // namespace tint::reader::wgsl
