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

#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, SwitchStmt_WithoutDefault) {
  auto p = parser(R"(switch(a) {
  case 1: {}
  case 2: {}
})");
  auto e = p->switch_stmt();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);
  ASSERT_TRUE(e->Is<ast::SwitchStatement>());
  ASSERT_EQ(e->body().size(), 2u);
  EXPECT_FALSE(e->body()[0]->IsDefault());
  EXPECT_FALSE(e->body()[1]->IsDefault());
}

TEST_F(ParserImplTest, SwitchStmt_Empty) {
  auto p = parser("switch(a) { }");
  auto e = p->switch_stmt();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);
  ASSERT_TRUE(e->Is<ast::SwitchStatement>());
  ASSERT_EQ(e->body().size(), 0u);
}

TEST_F(ParserImplTest, SwitchStmt_DefaultInMiddle) {
  auto p = parser(R"(switch(a) {
  case 1: {}
  default: {}
  case 2: {}
})");
  auto e = p->switch_stmt();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);
  ASSERT_TRUE(e->Is<ast::SwitchStatement>());

  ASSERT_EQ(e->body().size(), 3u);
  ASSERT_FALSE(e->body()[0]->IsDefault());
  ASSERT_TRUE(e->body()[1]->IsDefault());
  ASSERT_FALSE(e->body()[2]->IsDefault());
}

TEST_F(ParserImplTest, SwitchStmt_InvalidExpression) {
  auto p = parser("switch(a=b) {}");
  auto e = p->switch_stmt();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:9: expected ')'");
}

TEST_F(ParserImplTest, SwitchStmt_MissingExpression) {
  auto p = parser("switch {}");
  auto e = p->switch_stmt();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:8: expected '('");
}

TEST_F(ParserImplTest, SwitchStmt_MissingBracketLeft) {
  auto p = parser("switch(a) }");
  auto e = p->switch_stmt();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:11: expected '{' for switch statement");
}

TEST_F(ParserImplTest, SwitchStmt_MissingBracketRight) {
  auto p = parser("switch(a) {");
  auto e = p->switch_stmt();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:12: expected '}' for switch statement");
}

TEST_F(ParserImplTest, SwitchStmt_InvalidBody) {
  auto p = parser(R"(switch(a) {
  case: {}
})");
  auto e = p->switch_stmt();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "2:7: unable to parse case selectors");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
