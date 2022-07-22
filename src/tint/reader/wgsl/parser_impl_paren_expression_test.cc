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

TEST_F(ParserImplTest, ParenRhsStmt) {
    auto p = parser("(a + b)");
    auto e = p->expect_paren_expression();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
}

TEST_F(ParserImplTest, ParenRhsStmt_MissingLeftParen) {
    auto p = parser("true)");
    auto e = p->expect_paren_expression();
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    ASSERT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:1: expected '('");
}

TEST_F(ParserImplTest, ParenRhsStmt_MissingRightParen) {
    auto p = parser("(true");
    auto e = p->expect_paren_expression();
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    ASSERT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:6: expected ')'");
}

TEST_F(ParserImplTest, ParenRhsStmt_InvalidExpression) {
    auto p = parser("(if (a() {})");
    auto e = p->expect_paren_expression();
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    ASSERT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:2: unable to parse expression");
}

TEST_F(ParserImplTest, ParenRhsStmt_MissingExpression) {
    auto p = parser("()");
    auto e = p->expect_paren_expression();
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    ASSERT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:2: unable to parse expression");
}

}  // namespace
}  // namespace tint::reader::wgsl
