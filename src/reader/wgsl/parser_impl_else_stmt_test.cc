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
#include "src/ast/else_statement.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, ElseStmt) {
  auto* p = parser("else { a = b; c = d; }");
  auto e = p->else_stmt();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsElse());
  ASSERT_EQ(e->condition(), nullptr);
  EXPECT_EQ(e->body()->size(), 2u);
}

TEST_F(ParserImplTest, ElseStmt_InvalidBody) {
  auto* p = parser("else { fn main() -> void {}}");
  auto e = p->else_stmt();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:8: missing }");
}

TEST_F(ParserImplTest, ElseStmt_MissingBody) {
  auto* p = parser("else");
  auto e = p->else_stmt();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:5: missing {");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
