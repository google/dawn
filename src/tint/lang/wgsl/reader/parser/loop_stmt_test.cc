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

#include "src/tint/lang/wgsl/ast/discard_statement.h"
#include "src/tint/lang/wgsl/reader/parser/helper_test.h"

namespace tint::wgsl::reader {
namespace {

TEST_F(WGSLParserTest, LoopStmt_BodyNoContinuing) {
    auto p = parser("loop { discard; }");
    Parser::AttributeList attrs;
    auto e = p->loop_statement(attrs);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    EXPECT_EQ(e->body->source.range.begin.line, 1u);
    EXPECT_EQ(e->body->source.range.begin.column, 6u);
    EXPECT_EQ(e->body->source.range.end.line, 1u);
    EXPECT_EQ(e->body->source.range.end.column, 18u);

    ASSERT_EQ(e->body->statements.Length(), 1u);
    EXPECT_TRUE(e->body->statements[0]->Is<ast::DiscardStatement>());

    EXPECT_EQ(e->continuing->statements.Length(), 0u);
}

TEST_F(WGSLParserTest, LoopStmt_BodyWithContinuing) {
    auto p = parser("loop { discard; continuing { discard; }}");
    Parser::AttributeList attrs;
    auto e = p->loop_statement(attrs);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    EXPECT_EQ(e->body->source.range.begin.line, 1u);
    EXPECT_EQ(e->body->source.range.begin.column, 6u);
    EXPECT_EQ(e->body->source.range.end.line, 1u);
    EXPECT_EQ(e->body->source.range.end.column, 41u);

    ASSERT_EQ(e->body->statements.Length(), 1u);
    EXPECT_TRUE(e->body->statements[0]->Is<ast::DiscardStatement>());

    EXPECT_EQ(e->continuing->statements.Length(), 1u);
    EXPECT_TRUE(e->continuing->statements[0]->Is<ast::DiscardStatement>());

    EXPECT_EQ(e->continuing->source.range.begin.line, 1u);
    EXPECT_EQ(e->continuing->source.range.begin.column, 28u);
    EXPECT_EQ(e->continuing->source.range.end.line, 1u);
    EXPECT_EQ(e->continuing->source.range.end.column, 40u);
}

TEST_F(WGSLParserTest, LoopStmt_NoBodyNoContinuing) {
    auto p = parser("loop { }");
    Parser::AttributeList attrs;
    auto e = p->loop_statement(attrs);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_EQ(e->body->statements.Length(), 0u);
    ASSERT_EQ(e->continuing->statements.Length(), 0u);
}

TEST_F(WGSLParserTest, LoopStmt_NoBodyWithContinuing) {
    auto p = parser("loop { continuing { discard; }}");
    Parser::AttributeList attrs;
    auto e = p->loop_statement(attrs);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_EQ(e->body->statements.Length(), 0u);
    ASSERT_EQ(e->continuing->statements.Length(), 1u);
    EXPECT_TRUE(e->continuing->statements[0]->Is<ast::DiscardStatement>());
}

TEST_F(WGSLParserTest, LoopStmt_StmtAttributes) {
    auto p = parser("@diagnostic(off, derivative_uniformity) loop { }");
    auto attrs = p->attribute_list();
    auto l = p->loop_statement(attrs.value);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(l.errored);
    ASSERT_TRUE(l.matched);

    EXPECT_TRUE(attrs->IsEmpty());
    ASSERT_EQ(l->attributes.Length(), 1u);
    EXPECT_TRUE(l->attributes[0]->Is<ast::DiagnosticAttribute>());
}

TEST_F(WGSLParserTest, LoopStmt_BodyAttributes) {
    auto p = parser("loop @diagnostic(off, derivative_uniformity) { }");
    Parser::AttributeList attrs;
    auto e = p->loop_statement(attrs);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_EQ(e->body->attributes.Length(), 1u);
    EXPECT_TRUE(e->body->attributes[0]->Is<ast::DiagnosticAttribute>());
}

TEST_F(WGSLParserTest, LoopStmt_MissingBracketLeft) {
    auto p = parser("loop discard; }");
    Parser::AttributeList attrs;
    auto e = p->loop_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:6: expected '{' for loop");
}

TEST_F(WGSLParserTest, LoopStmt_MissingBracketRight) {
    auto p = parser("loop { discard; ");
    Parser::AttributeList attrs;
    auto e = p->loop_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:17: expected '}' for loop");
}

TEST_F(WGSLParserTest, LoopStmt_InvalidStatements) {
    auto p = parser("loop { discard }");
    Parser::AttributeList attrs;
    auto e = p->loop_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:16: expected ';' for discard statement");
}

TEST_F(WGSLParserTest, LoopStmt_InvalidContinuing) {
    auto p = parser("loop { continuing { discard }}");
    Parser::AttributeList attrs;
    auto e = p->loop_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:29: expected ';' for discard statement");
}

TEST_F(WGSLParserTest, LoopStmt_Continuing_BreakIf) {
    auto p = parser("loop { continuing { break if 1 + 2 < 5; }}");
    Parser::AttributeList attrs;
    auto e = p->loop_statement(attrs);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_EQ(e->body->statements.Length(), 0u);
    ASSERT_EQ(e->continuing->statements.Length(), 1u);
    EXPECT_TRUE(e->continuing->statements[0]->Is<ast::BreakIfStatement>());
}

TEST_F(WGSLParserTest, LoopStmt_Continuing_BreakIf_MissingExpr) {
    auto p = parser("loop { continuing { break if; }}");
    Parser::AttributeList attrs;
    auto e = p->loop_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:21: expected expression for `break-if`");
}

TEST_F(WGSLParserTest, LoopStmt_Continuing_BreakIf_InvalidExpr) {
    auto p = parser("loop { continuing { break if switch; }}");
    Parser::AttributeList attrs;
    auto e = p->loop_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:21: expected expression for `break-if`");
}

TEST_F(WGSLParserTest, LoopStmt_NoContinuing_BreakIf) {
    auto p = parser("loop { break if true; }");
    Parser::AttributeList attrs;
    auto e = p->loop_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:14: expected ';' for break statement");
}

TEST_F(WGSLParserTest, LoopStmt_Continuing_BreakIf_MissingSemicolon) {
    auto p = parser("loop { continuing { break if 1 + 2 < 5 }}");
    Parser::AttributeList attrs;
    auto e = p->loop_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:40: expected ';' for `break-if` statement");
}

}  // namespace
}  // namespace tint::wgsl::reader
