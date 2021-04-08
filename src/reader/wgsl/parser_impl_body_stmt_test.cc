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

#include "src/ast/discard_statement.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, BodyStmt) {
  auto p = parser(R"({
  discard;
  return 1 + b / 2;
})");
  auto e = p->expect_body_stmt();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(e.errored);
  ASSERT_EQ(e->size(), 2u);
  EXPECT_TRUE(e->get(0)->Is<ast::DiscardStatement>());
  EXPECT_TRUE(e->get(1)->Is<ast::ReturnStatement>());
}

TEST_F(ParserImplTest, BodyStmt_Empty) {
  auto p = parser("{}");
  auto e = p->expect_body_stmt();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(e.errored);
  EXPECT_EQ(e->size(), 0u);
}

TEST_F(ParserImplTest, BodyStmt_InvalidStmt) {
  auto p = parser("{fn main() {}}");
  auto e = p->expect_body_stmt();
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(e.errored);
  EXPECT_EQ(p->error(), "1:2: expected '}'");
}

TEST_F(ParserImplTest, BodyStmt_MissingRightParen) {
  auto p = parser("{return;");
  auto e = p->expect_body_stmt();
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(e.errored);
  EXPECT_EQ(p->error(), "1:9: expected '}'");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
