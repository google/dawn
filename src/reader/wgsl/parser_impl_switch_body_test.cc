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

#include "gtest/gtest.h"
#include "src/ast/case_statement.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, SwitchBody_Case) {
  auto* p = parser("case 1: { a = 4; }");
  auto e = p->switch_body();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsCase());
  EXPECT_FALSE(e->IsDefault());
  ASSERT_EQ(e->body()->size(), 1u);
  EXPECT_TRUE(e->body()->get(0)->IsAssign());
}

TEST_F(ParserImplTest, SwitchBody_Case_InvalidConstLiteral) {
  auto* p = parser("case a == 4: { a = 4; }");
  auto e = p->switch_body();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:6: unable to parse case selectors");
}

TEST_F(ParserImplTest, SwitchBody_Case_InvalidSelector_bool) {
  auto* p = parser("case true: { a = 4; }");
  auto e = p->switch_body();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:6: invalid case selector must be an integer value");
}

TEST_F(ParserImplTest, SwitchBody_Case_MissingConstLiteral) {
  auto* p = parser("case: { a = 4; }");
  auto e = p->switch_body();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:5: unable to parse case selectors");
}

TEST_F(ParserImplTest, SwitchBody_Case_MissingColon) {
  auto* p = parser("case 1 { a = 4; }");
  auto e = p->switch_body();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:8: missing : for case statement");
}

TEST_F(ParserImplTest, SwitchBody_Case_MissingBracketLeft) {
  auto* p = parser("case 1: a = 4; }");
  auto e = p->switch_body();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:9: missing { for case statement");
}

TEST_F(ParserImplTest, SwitchBody_Case_MissingBracketRight) {
  auto* p = parser("case 1: { a = 4; ");
  auto e = p->switch_body();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:18: missing } for case statement");
}

TEST_F(ParserImplTest, SwitchBody_Case_InvalidCaseBody) {
  auto* p = parser("case 1: { fn main() -> void {} }");
  auto e = p->switch_body();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:11: missing } for case statement");
}

TEST_F(ParserImplTest, SwitchBody_Default) {
  auto* p = parser("default: { a = 4; }");
  auto e = p->switch_body();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsCase());
  EXPECT_TRUE(e->IsDefault());
  ASSERT_EQ(e->body()->size(), 1u);
  EXPECT_TRUE(e->body()->get(0)->IsAssign());
}

TEST_F(ParserImplTest, SwitchBody_Default_MissingColon) {
  auto* p = parser("default { a = 4; }");
  auto e = p->switch_body();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:9: missing : for case statement");
}

TEST_F(ParserImplTest, SwitchBody_Default_MissingBracketLeft) {
  auto* p = parser("default: a = 4; }");
  auto e = p->switch_body();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:10: missing { for case statement");
}

TEST_F(ParserImplTest, SwitchBody_Default_MissingBracketRight) {
  auto* p = parser("default: { a = 4; ");
  auto e = p->switch_body();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:19: missing } for case statement");
}

TEST_F(ParserImplTest, SwitchBody_Default_InvalidCaseBody) {
  auto* p = parser("default: { fn main() -> void {} }");
  auto e = p->switch_body();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:12: missing } for case statement");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
