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

#include "src/ast/fallthrough_statement.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, CaseBody_Empty) {
  auto p = parser("");
  auto e = p->case_body();
  ASSERT_FALSE(p->has_error()) << p->error();
  EXPECT_FALSE(e.errored);
  EXPECT_TRUE(e.matched);
  EXPECT_EQ(e->size(), 0u);
}

TEST_F(ParserImplTest, CaseBody_Statements) {
  auto p = parser(R"(
  var a: i32;
  a = 2;)");

  auto e = p->case_body();
  ASSERT_FALSE(p->has_error()) << p->error();
  EXPECT_FALSE(e.errored);
  EXPECT_TRUE(e.matched);
  ASSERT_EQ(e->size(), 2u);
  EXPECT_TRUE(e->get(0)->Is<ast::VariableDeclStatement>());
  EXPECT_TRUE(e->get(1)->Is<ast::AssignmentStatement>());
}

TEST_F(ParserImplTest, CaseBody_InvalidStatement) {
  auto p = parser("a =");
  auto e = p->case_body();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(e.errored);
  EXPECT_FALSE(e.matched);
  EXPECT_EQ(e.value, nullptr);
}

TEST_F(ParserImplTest, CaseBody_Fallthrough) {
  auto p = parser("fallthrough;");
  auto e = p->case_body();
  ASSERT_FALSE(p->has_error()) << p->error();
  EXPECT_FALSE(e.errored);
  EXPECT_TRUE(e.matched);
  ASSERT_EQ(e->size(), 1u);
  EXPECT_TRUE(e->get(0)->Is<ast::FallthroughStatement>());
}

TEST_F(ParserImplTest, CaseBody_Fallthrough_MissingSemicolon) {
  auto p = parser("fallthrough");
  auto e = p->case_body();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(e.errored);
  EXPECT_FALSE(e.matched);
  EXPECT_EQ(e.value, nullptr);
  EXPECT_EQ(p->error(), "1:12: expected ';' for fallthrough statement");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
