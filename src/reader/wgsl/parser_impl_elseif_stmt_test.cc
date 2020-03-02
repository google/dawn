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

namespace tint {
namespace reader {
namespace wgsl {

using ParserImplTest = testing::Test;

TEST_F(ParserImplTest, ElseIfStmt) {
  ParserImpl p{"elseif (a == 4) { a = b; c = d; }"};
  auto e = p.elseif_stmt();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_EQ(e.size(), 1);

  ASSERT_TRUE(e[0]->IsElse());
  ASSERT_NE(e[0]->condition(), nullptr);
  ASSERT_TRUE(e[0]->condition()->IsRelational());
  EXPECT_EQ(e[0]->body().size(), 2);
}

TEST_F(ParserImplTest, ElseIfStmt_Multiple) {
  ParserImpl p{"elseif (a == 4) { a = b; c = d; } elseif(c) { d = 2; }"};
  auto e = p.elseif_stmt();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_EQ(e.size(), 2);

  ASSERT_TRUE(e[0]->IsElse());
  ASSERT_NE(e[0]->condition(), nullptr);
  ASSERT_TRUE(e[0]->condition()->IsRelational());
  EXPECT_EQ(e[0]->body().size(), 2);

  ASSERT_TRUE(e[1]->IsElse());
  ASSERT_NE(e[1]->condition(), nullptr);
  ASSERT_TRUE(e[1]->condition()->IsIdentifier());
  EXPECT_EQ(e[1]->body().size(), 1);
}

TEST_F(ParserImplTest, ElseIfStmt_InvalidBody) {
  ParserImpl p{"elseif (true) { fn main() -> void {}}"};
  auto e = p.elseif_stmt();
  ASSERT_TRUE(p.has_error());
  EXPECT_EQ(p.error(), "1:17: missing }");
}

TEST_F(ParserImplTest, ElseIfStmt_MissingBody) {
  ParserImpl p{"elseif (true)"};
  auto e = p.elseif_stmt();
  ASSERT_TRUE(p.has_error());
  EXPECT_EQ(p.error(), "1:14: missing {");
}

}  // namespace wgsl
}  // namespace reader
}  // namespace tint
