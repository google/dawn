// Copyright 2022 The Tint Authors.
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

#include "src/tint/ast/discard_statement.h"

namespace tint::reader::wgsl {
namespace {

using WhileStmtTest = ParserImplTest;

// Test an empty while loop.
TEST_F(WhileStmtTest, Empty) {
    auto p = parser("while true { }");
    ParserImpl::AttributeList attrs;
    auto wl = p->while_statement(attrs);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(wl.errored);
    ASSERT_TRUE(wl.matched);
    EXPECT_TRUE(Is<ast::Expression>(wl->condition));
    EXPECT_TRUE(wl->body->Empty());
}

// Test an empty while loop with parentheses.
TEST_F(WhileStmtTest, EmptyWithParentheses) {
    auto p = parser("while (true) { }");
    ParserImpl::AttributeList attrs;
    auto wl = p->while_statement(attrs);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(wl.errored);
    ASSERT_TRUE(wl.matched);
    EXPECT_TRUE(Is<ast::Expression>(wl->condition));
    EXPECT_TRUE(wl->body->Empty());
}

// Test a while loop with non-empty body.
TEST_F(WhileStmtTest, Body) {
    auto p = parser("while (true) { discard; }");
    ParserImpl::AttributeList attrs;
    auto wl = p->while_statement(attrs);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(wl.errored);
    ASSERT_TRUE(wl.matched);
    EXPECT_TRUE(Is<ast::Expression>(wl->condition));
    ASSERT_EQ(wl->body->statements.Length(), 1u);
    EXPECT_TRUE(wl->body->statements[0]->Is<ast::DiscardStatement>());
}

// Test a while loop with complex condition.
TEST_F(WhileStmtTest, ComplexCondition) {
    auto p = parser("while (a + 1 - 2) == 3 { }");
    ParserImpl::AttributeList attrs;
    auto wl = p->while_statement(attrs);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(wl.errored);
    ASSERT_TRUE(wl.matched);
    EXPECT_TRUE(Is<ast::BinaryExpression>(wl->condition));
    EXPECT_TRUE(wl->body->Empty());
}

// Test a while loop with complex condition, with parentheses.
TEST_F(WhileStmtTest, ComplexConditionWithParentheses) {
    auto p = parser("while ((a + 1 - 2) == 3) { }");
    ParserImpl::AttributeList attrs;
    auto wl = p->while_statement(attrs);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(wl.errored);
    ASSERT_TRUE(wl.matched);
    EXPECT_TRUE(Is<ast::Expression>(wl->condition));
    EXPECT_TRUE(wl->body->Empty());
}

// Test a while loop with attributes.
TEST_F(WhileStmtTest, WithAttributes) {
    auto p = parser("@diagnostic(off, derivative_uniformity) while true { }");
    auto attrs = p->attribute_list();
    auto wl = p->while_statement(attrs.value);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(wl.errored);
    ASSERT_TRUE(wl.matched);

    EXPECT_TRUE(attrs->IsEmpty());
    ASSERT_EQ(wl->attributes.Length(), 1u);
    EXPECT_TRUE(wl->attributes[0]->Is<ast::DiagnosticAttribute>());
}

class WhileStmtErrorTest : public ParserImplTest {
  public:
    void TestWhileWithError(std::string for_str, std::string error_str) {
        auto p_for = parser(for_str);
        ParserImpl::AttributeList attrs;
        auto e_for = p_for->while_statement(attrs);

        EXPECT_FALSE(e_for.matched);
        EXPECT_TRUE(e_for.errored);
        EXPECT_TRUE(p_for->has_error());
        ASSERT_EQ(e_for.value, nullptr);
        EXPECT_EQ(p_for->error(), error_str);
    }
};

// Test a while loop with missing left parenthesis is invalid.
TEST_F(WhileStmtErrorTest, MissingLeftParen) {
    std::string while_str = "while bool) { }";
    std::string error_str = "1:11: expected '{' for while loop";

    TestWhileWithError(while_str, error_str);
}

// Test a while loop with missing condition is invalid.
TEST_F(WhileStmtErrorTest, MissingFirstSemicolon) {
    std::string while_str = "while () {}";
    std::string error_str = "1:8: unable to parse expression";

    TestWhileWithError(while_str, error_str);
}

// Test a while loop with missing right parenthesis is invalid.
TEST_F(WhileStmtErrorTest, MissingRightParen) {
    std::string while_str = "while (true {}";
    std::string error_str = "1:13: expected ')'";

    TestWhileWithError(while_str, error_str);
}

// Test a while loop with missing left brace is invalid.
TEST_F(WhileStmtErrorTest, MissingLeftBrace) {
    std::string while_str = "while (true) }";
    std::string error_str = "1:14: expected '{' for while loop";

    TestWhileWithError(while_str, error_str);
}

// Test a for loop with missing right brace is invalid.
TEST_F(WhileStmtErrorTest, MissingRightBrace) {
    std::string while_str = "while (true) {";
    std::string error_str = "1:15: expected '}' for while loop";

    TestWhileWithError(while_str, error_str);
}

// Test a while loop with an invalid break condition.
TEST_F(WhileStmtErrorTest, InvalidBreakConditionAsExpression) {
    std::string while_str = "while ((0 == 1) { }";
    std::string error_str = "1:17: expected ')'";

    TestWhileWithError(while_str, error_str);
}

// Test a while loop with a break condition not matching
// logical_or_expression.
TEST_F(WhileStmtErrorTest, InvalidBreakConditionMatch) {
    std::string while_str = "while (var i: i32 = 0) { }";
    std::string error_str = "1:8: unable to parse expression";

    TestWhileWithError(while_str, error_str);
}

// Test a while loop with an invalid body.
TEST_F(WhileStmtErrorTest, InvalidBody) {
    std::string while_str = "while (true) { let x: i32; }";
    std::string error_str = "1:26: expected '=' for 'let' declaration";

    TestWhileWithError(while_str, error_str);
}

// Test a for loop with a body not matching statements
TEST_F(WhileStmtErrorTest, InvalidBodyMatch) {
    std::string while_str = "while (true) { fn main() {} }";
    std::string error_str = "1:16: expected '}' for while loop";

    TestWhileWithError(while_str, error_str);
}

}  // namespace
}  // namespace tint::reader::wgsl
