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

TEST_F(ParserImplTest, SwitchStmt_WithoutDefault) {
    auto p = parser(R"(switch a {
  case 1: {}
  case 2: {}
})");
    ParserImpl::AttributeList attrs;
    auto e = p->switch_statement(attrs);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::SwitchStatement>());
    ASSERT_EQ(e->body.Length(), 2u);
    EXPECT_FALSE(e->body[0]->ContainsDefault());
    EXPECT_FALSE(e->body[1]->ContainsDefault());
}

TEST_F(ParserImplTest, SwitchStmt_Empty) {
    auto p = parser("switch a { }");
    ParserImpl::AttributeList attrs;
    auto e = p->switch_statement(attrs);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::SwitchStatement>());
    ASSERT_EQ(e->body.Length(), 0u);
}

TEST_F(ParserImplTest, SwitchStmt_DefaultInMiddle) {
    auto p = parser(R"(switch a {
  case 1: {}
  default: {}
  case 2: {}
})");
    ParserImpl::AttributeList attrs;
    auto e = p->switch_statement(attrs);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::SwitchStatement>());

    ASSERT_EQ(e->body.Length(), 3u);
    ASSERT_FALSE(e->body[0]->ContainsDefault());
    ASSERT_TRUE(e->body[1]->ContainsDefault());
    ASSERT_FALSE(e->body[2]->ContainsDefault());
}

TEST_F(ParserImplTest, SwitchStmt_Default_Mixed) {
    auto p = parser(R"(switch a {
  case 1, default, 2: {}
})");
    ParserImpl::AttributeList attrs;
    auto e = p->switch_statement(attrs);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::SwitchStatement>());

    ASSERT_EQ(e->body.Length(), 1u);
    ASSERT_TRUE(e->body[0]->ContainsDefault());
}

TEST_F(ParserImplTest, SwitchStmt_WithParens) {
    auto p = parser("switch(a+b) { }");
    ParserImpl::AttributeList attrs;
    auto e = p->switch_statement(attrs);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::SwitchStatement>());
    ASSERT_EQ(e->body.Length(), 0u);
}

TEST_F(ParserImplTest, SwitchStmt_WithAttributes) {
    auto p = parser("@diagnostic(off, derivative_uniformity) switch a { default{} }");
    auto a = p->attribute_list();
    auto e = p->switch_statement(a.value);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::SwitchStatement>());

    EXPECT_TRUE(a->IsEmpty());
    ASSERT_EQ(e->attributes.Length(), 1u);
    EXPECT_TRUE(e->attributes[0]->Is<ast::DiagnosticAttribute>());
}

TEST_F(ParserImplTest, SwitchStmt_WithBodyAttributes) {
    auto p = parser("switch a @diagnostic(off, derivative_uniformity) { default{} }");
    ParserImpl::AttributeList attrs;
    auto e = p->switch_statement(attrs);
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::SwitchStatement>());

    EXPECT_TRUE(e->attributes.IsEmpty());
    ASSERT_EQ(e->body_attributes.Length(), 1u);
    EXPECT_TRUE(e->body_attributes[0]->Is<ast::DiagnosticAttribute>());
}

TEST_F(ParserImplTest, SwitchStmt_InvalidExpression) {
    auto p = parser("switch a=b {}");
    ParserImpl::AttributeList attrs;
    auto e = p->switch_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:9: expected '{' for switch statement");
}

TEST_F(ParserImplTest, SwitchStmt_MissingExpression) {
    auto p = parser("switch {}");
    ParserImpl::AttributeList attrs;
    auto e = p->switch_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:8: unable to parse selector expression");
}

TEST_F(ParserImplTest, SwitchStmt_MissingBracketLeft) {
    auto p = parser("switch a }");
    ParserImpl::AttributeList attrs;
    auto e = p->switch_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:10: expected '{' for switch statement");
}

TEST_F(ParserImplTest, SwitchStmt_MissingBracketRight) {
    auto p = parser("switch a {");
    ParserImpl::AttributeList attrs;
    auto e = p->switch_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:11: expected '}' for switch statement");
}

TEST_F(ParserImplTest, SwitchStmt_InvalidBody) {
    auto p = parser(R"(switch a {
  case: {}
})");
    ParserImpl::AttributeList attrs;
    auto e = p->switch_statement(attrs);
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "2:7: expected case selector expression or `default`");
}

}  // namespace
}  // namespace tint::reader::wgsl
