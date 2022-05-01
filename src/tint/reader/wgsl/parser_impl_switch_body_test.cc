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

TEST_F(ParserImplTest, SwitchBody_Case) {
    auto p = parser("case 1 { a = 4; }");
    auto e = p->switch_body();
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::CaseStatement>());
    EXPECT_FALSE(e->IsDefault());
    auto* stmt = e->As<ast::CaseStatement>();
    ASSERT_EQ(stmt->selectors.size(), 1u);
    EXPECT_EQ(stmt->selectors[0]->ValueAsU32(), 1u);
    ASSERT_EQ(e->body->statements.size(), 1u);
    EXPECT_TRUE(e->body->statements[0]->Is<ast::AssignmentStatement>());
}

TEST_F(ParserImplTest, SwitchBody_Case_WithColon) {
    auto p = parser("case 1: { a = 4; }");
    auto e = p->switch_body();
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::CaseStatement>());
    EXPECT_FALSE(e->IsDefault());
    auto* stmt = e->As<ast::CaseStatement>();
    ASSERT_EQ(stmt->selectors.size(), 1u);
    EXPECT_EQ(stmt->selectors[0]->ValueAsU32(), 1u);
    ASSERT_EQ(e->body->statements.size(), 1u);
    EXPECT_TRUE(e->body->statements[0]->Is<ast::AssignmentStatement>());
}

TEST_F(ParserImplTest, SwitchBody_Case_TrailingComma) {
    auto p = parser("case 1, 2, { }");
    auto e = p->switch_body();
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::CaseStatement>());
    EXPECT_FALSE(e->IsDefault());
    auto* stmt = e->As<ast::CaseStatement>();
    ASSERT_EQ(stmt->selectors.size(), 2u);
    EXPECT_EQ(stmt->selectors[0]->ValueAsU32(), 1u);
    EXPECT_EQ(stmt->selectors[1]->ValueAsU32(), 2u);
}

TEST_F(ParserImplTest, SwitchBody_Case_TrailingComma_WithColon) {
    auto p = parser("case 1, 2,: { }");
    auto e = p->switch_body();
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::CaseStatement>());
    EXPECT_FALSE(e->IsDefault());
    auto* stmt = e->As<ast::CaseStatement>();
    ASSERT_EQ(stmt->selectors.size(), 2u);
    EXPECT_EQ(stmt->selectors[0]->ValueAsU32(), 1u);
    EXPECT_EQ(stmt->selectors[1]->ValueAsU32(), 2u);
}

TEST_F(ParserImplTest, SwitchBody_Case_InvalidConstLiteral) {
    auto p = parser("case a == 4: { a = 4; }");
    auto e = p->switch_body();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:6: unable to parse case selectors");
}

TEST_F(ParserImplTest, SwitchBody_Case_InvalidSelector_bool) {
    auto p = parser("case true: { a = 4; }");
    auto e = p->switch_body();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:6: invalid case selector must be an integer value");
}

TEST_F(ParserImplTest, SwitchBody_Case_MissingConstLiteral) {
    auto p = parser("case: { a = 4; }");
    auto e = p->switch_body();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:5: unable to parse case selectors");
}

TEST_F(ParserImplTest, SwitchBody_Case_MissingBracketLeft) {
    auto p = parser("case 1 a = 4; }");
    auto e = p->switch_body();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:8: expected '{' for case statement");
}

TEST_F(ParserImplTest, SwitchBody_Case_MissingBracketLeft_WithColon) {
    auto p = parser("case 1: a = 4; }");
    auto e = p->switch_body();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:9: expected '{' for case statement");
}

TEST_F(ParserImplTest, SwitchBody_Case_MissingBracketRight) {
    auto p = parser("case 1: { a = 4; ");
    auto e = p->switch_body();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:18: expected '}' for case statement");
}

TEST_F(ParserImplTest, SwitchBody_Case_InvalidCaseBody) {
    auto p = parser("case 1: { fn main() {} }");
    auto e = p->switch_body();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:11: expected '}' for case statement");
}

TEST_F(ParserImplTest, SwitchBody_Case_MultipleSelectors) {
    auto p = parser("case 1, 2 { }");
    auto e = p->switch_body();
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::CaseStatement>());
    EXPECT_FALSE(e->IsDefault());
    ASSERT_EQ(e->body->statements.size(), 0u);
    ASSERT_EQ(e->selectors.size(), 2u);
    ASSERT_EQ(e->selectors[0]->ValueAsI32(), 1);
    ASSERT_EQ(e->selectors[1]->ValueAsI32(), 2);
}

TEST_F(ParserImplTest, SwitchBody_Case_MultipleSelectors_WithColon) {
    auto p = parser("case 1, 2: { }");
    auto e = p->switch_body();
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::CaseStatement>());
    EXPECT_FALSE(e->IsDefault());
    ASSERT_EQ(e->body->statements.size(), 0u);
    ASSERT_EQ(e->selectors.size(), 2u);
    ASSERT_EQ(e->selectors[0]->ValueAsI32(), 1);
    ASSERT_EQ(e->selectors[1]->ValueAsI32(), 2);
}

TEST_F(ParserImplTest, SwitchBody_Case_MultipleSelectorsMissingComma) {
    auto p = parser("case 1 2: { }");
    auto e = p->switch_body();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:8: expected '{' for case statement");
}

TEST_F(ParserImplTest, SwitchBody_Case_MultipleSelectorsStartsWithComma) {
    auto p = parser("case , 1, 2: { }");
    auto e = p->switch_body();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:6: unable to parse case selectors");
}

TEST_F(ParserImplTest, SwitchBody_Default) {
    auto p = parser("default { a = 4; }");
    auto e = p->switch_body();
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::CaseStatement>());
    EXPECT_TRUE(e->IsDefault());
    ASSERT_EQ(e->body->statements.size(), 1u);
    EXPECT_TRUE(e->body->statements[0]->Is<ast::AssignmentStatement>());
}

TEST_F(ParserImplTest, SwitchBody_Default_WithColon) {
    auto p = parser("default: { a = 4; }");
    auto e = p->switch_body();
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::CaseStatement>());
    EXPECT_TRUE(e->IsDefault());
    ASSERT_EQ(e->body->statements.size(), 1u);
    EXPECT_TRUE(e->body->statements[0]->Is<ast::AssignmentStatement>());
}

TEST_F(ParserImplTest, SwitchBody_Default_MissingBracketLeft) {
    auto p = parser("default a = 4; }");
    auto e = p->switch_body();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:9: expected '{' for case statement");
}

TEST_F(ParserImplTest, SwitchBody_Default_MissingBracketLeft_WithColon) {
    auto p = parser("default: a = 4; }");
    auto e = p->switch_body();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:10: expected '{' for case statement");
}

TEST_F(ParserImplTest, SwitchBody_Default_MissingBracketRight) {
    auto p = parser("default: { a = 4; ");
    auto e = p->switch_body();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:19: expected '}' for case statement");
}

TEST_F(ParserImplTest, SwitchBody_Default_InvalidCaseBody) {
    auto p = parser("default: { fn main() {} }");
    auto e = p->switch_body();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:12: expected '}' for case statement");
}

}  // namespace
}  // namespace tint::reader::wgsl
