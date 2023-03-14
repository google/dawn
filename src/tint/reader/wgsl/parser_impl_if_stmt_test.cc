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

TEST_F(ParserImplTest, IfStmt) {
    auto p = parser("if a == 4 { a = b; c = d; }");
    ParserImpl::AttributeList attrs;
    auto e = p->if_statement(attrs);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::IfStatement>());
    ASSERT_NE(e->condition, nullptr);
    ASSERT_TRUE(e->condition->Is<ast::BinaryExpression>());
    EXPECT_EQ(e->body->statements.Length(), 2u);
    EXPECT_EQ(e->else_statement, nullptr);
}

TEST_F(ParserImplTest, IfStmt_WithElse) {
    auto p = parser("if a == 4 { a = b; c = d; } else if(c) { d = 2; } else {}");
    ParserImpl::AttributeList attrs;
    auto e = p->if_statement(attrs);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::IfStatement>());
    ASSERT_NE(e->condition, nullptr);
    ASSERT_TRUE(e->condition->Is<ast::BinaryExpression>());
    EXPECT_EQ(e->body->statements.Length(), 2u);

    auto* elseif = As<ast::IfStatement>(e->else_statement);
    ASSERT_NE(elseif, nullptr);
    ASSERT_TRUE(elseif->condition->Is<ast::IdentifierExpression>());
    EXPECT_EQ(elseif->body->statements.Length(), 1u);

    auto* el = As<ast::BlockStatement>(elseif->else_statement);
    ASSERT_NE(el, nullptr);
    EXPECT_EQ(el->statements.Length(), 0u);
}

TEST_F(ParserImplTest, IfStmt_WithElse_WithParens) {
    auto p = parser("if(a==4) { a = b; c = d; } else if(c) { d = 2; } else {}");
    ParserImpl::AttributeList attrs;
    auto e = p->if_statement(attrs);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::IfStatement>());
    ASSERT_NE(e->condition, nullptr);
    ASSERT_TRUE(e->condition->Is<ast::BinaryExpression>());
    EXPECT_EQ(e->body->statements.Length(), 2u);

    auto* elseif = As<ast::IfStatement>(e->else_statement);
    ASSERT_NE(elseif, nullptr);
    ASSERT_TRUE(elseif->condition->Is<ast::IdentifierExpression>());
    EXPECT_EQ(elseif->body->statements.Length(), 1u);

    auto* el = As<ast::BlockStatement>(elseif->else_statement);
    ASSERT_NE(el, nullptr);
    EXPECT_EQ(el->statements.Length(), 0u);
}

TEST_F(ParserImplTest, IfStmt_WithAttributes) {
    auto p = parser(R"(@diagnostic(off, derivative_uniformity) if true { })");
    auto a = p->attribute_list();
    auto e = p->if_statement(a.value);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::IfStatement>());

    EXPECT_TRUE(a->IsEmpty());
    ASSERT_EQ(e->attributes.Length(), 1u);
    EXPECT_TRUE(e->attributes[0]->Is<ast::DiagnosticAttribute>());
}

TEST_F(ParserImplTest, IfStmt_InvalidCondition) {
    auto p = parser("if a = 3 {}");
    ParserImpl::AttributeList attrs;
    auto e = p->if_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:6: expected '{' for if statement");
}

TEST_F(ParserImplTest, IfStmt_MissingCondition) {
    auto p = parser("if {}");
    ParserImpl::AttributeList attrs;
    auto e = p->if_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:4: unable to parse condition expression");
}

TEST_F(ParserImplTest, IfStmt_InvalidBody) {
    auto p = parser("if a { fn main() {}}");
    ParserImpl::AttributeList attrs;
    auto e = p->if_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:8: expected '}' for if statement");
}

TEST_F(ParserImplTest, IfStmt_MissingBody) {
    auto p = parser("if a");
    ParserImpl::AttributeList attrs;
    auto e = p->if_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:5: expected '{' for if statement");
}

TEST_F(ParserImplTest, IfStmt_InvalidElseif) {
    auto p = parser("if a {} else if a { fn main() -> a{}}");
    ParserImpl::AttributeList attrs;
    auto e = p->if_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:21: expected '}' for if statement");
}

TEST_F(ParserImplTest, IfStmt_InvalidElse) {
    auto p = parser("if a {} else { fn main() -> a{}}");
    ParserImpl::AttributeList attrs;
    auto e = p->if_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:16: expected '}' for else statement");
}

}  // namespace
}  // namespace tint::reader::wgsl
