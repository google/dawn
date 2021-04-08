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

TEST_F(ParserImplTest, ElseIfStmt) {
  auto p = parser("elseif (a == 4) { a = b; c = d; }");
  auto e = p->elseif_stmt();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_EQ(e.value.size(), 1u);

  ASSERT_TRUE(e.value[0]->Is<ast::ElseStatement>());
  ASSERT_NE(e.value[0]->condition(), nullptr);
  ASSERT_TRUE(e.value[0]->condition()->Is<ast::BinaryExpression>());
  EXPECT_EQ(e.value[0]->body()->size(), 2u);
}

TEST_F(ParserImplTest, ElseIfStmt_Multiple) {
  auto p = parser("elseif (a == 4) { a = b; c = d; } elseif(c) { d = 2; }");
  auto e = p->elseif_stmt();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_EQ(e.value.size(), 2u);

  ASSERT_TRUE(e.value[0]->Is<ast::ElseStatement>());
  ASSERT_NE(e.value[0]->condition(), nullptr);
  ASSERT_TRUE(e.value[0]->condition()->Is<ast::BinaryExpression>());
  EXPECT_EQ(e.value[0]->body()->size(), 2u);

  ASSERT_TRUE(e.value[1]->Is<ast::ElseStatement>());
  ASSERT_NE(e.value[1]->condition(), nullptr);
  ASSERT_TRUE(e.value[1]->condition()->Is<ast::IdentifierExpression>());
  EXPECT_EQ(e.value[1]->body()->size(), 1u);
}

TEST_F(ParserImplTest, ElseIfStmt_InvalidBody) {
  auto p = parser("elseif (true) { fn main() {}}");
  auto e = p->elseif_stmt();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:17: expected '}'");
}

TEST_F(ParserImplTest, ElseIfStmt_MissingBody) {
  auto p = parser("elseif (true)");
  auto e = p->elseif_stmt();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:14: expected '{'");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
