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
#include "src/ast/if_statement.h"
#include "src/reader/wgsl/parser_impl.h"

namespace tint {
namespace reader {
namespace wgsl {

using ParserImplTest = testing::Test;

TEST_F(ParserImplTest, IfStmt) {
  ParserImpl p{"if (a == 4) { a = b; c = d; }"};
  auto e = p.if_stmt();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);

  ASSERT_TRUE(e->IsIf());
  ASSERT_NE(e->condition(), nullptr);
  ASSERT_TRUE(e->condition()->IsRelational());
  EXPECT_EQ(e->body().size(), 2);
  EXPECT_EQ(e->else_statements().size(), 0);
  EXPECT_EQ(e->premerge().size(), 0);
}

TEST_F(ParserImplTest, IfStmt_WithElse) {
  ParserImpl p{"if (a == 4) { a = b; c = d; } elseif(c) { d = 2; } else {}"};
  auto e = p.if_stmt();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);

  ASSERT_TRUE(e->IsIf());
  ASSERT_NE(e->condition(), nullptr);
  ASSERT_TRUE(e->condition()->IsRelational());
  EXPECT_EQ(e->body().size(), 2);

  ASSERT_EQ(e->else_statements().size(), 2);
  ASSERT_NE(e->else_statements()[0]->condition(), nullptr);
  ASSERT_TRUE(e->else_statements()[0]->condition()->IsIdentifier());
  EXPECT_EQ(e->else_statements()[0]->body().size(), 1);

  ASSERT_EQ(e->else_statements()[1]->condition(), nullptr);
  EXPECT_EQ(e->else_statements()[1]->body().size(), 0);
}

TEST_F(ParserImplTest, IfStmt_WithPremerge) {
  ParserImpl p{R"(if (a == 4) {
  a = b;
  c = d;
} else {
  d = 2;
} premerge {
  a = 2;
})"};
  auto e = p.if_stmt();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);

  ASSERT_TRUE(e->IsIf());
  ASSERT_NE(e->condition(), nullptr);
  ASSERT_TRUE(e->condition()->IsRelational());
  EXPECT_EQ(e->body().size(), 2);

  ASSERT_EQ(e->else_statements().size(), 1);
  ASSERT_EQ(e->else_statements()[0]->condition(), nullptr);
  EXPECT_EQ(e->else_statements()[0]->body().size(), 1);

  ASSERT_EQ(e->premerge().size(), 1);
}

TEST_F(ParserImplTest, IfStmt_InvalidCondition) {
  ParserImpl p{"if (a = 3) {}"};
  auto e = p.if_stmt();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:7: expected )");
}

TEST_F(ParserImplTest, IfStmt_MissingCondition) {
  ParserImpl p{"if {}"};
  auto e = p.if_stmt();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:4: expected (");
}

TEST_F(ParserImplTest, IfStmt_InvalidBody) {
  ParserImpl p{"if (a) { fn main() -> void {}}"};
  auto e = p.if_stmt();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:10: missing }");
}

TEST_F(ParserImplTest, IfStmt_MissingBody) {
  ParserImpl p{"if (a)"};
  auto e = p.if_stmt();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:7: missing {");
}

TEST_F(ParserImplTest, IfStmt_InvalidElseif) {
  ParserImpl p{"if (a) {} elseif (a) { fn main() -> a{}}"};
  auto e = p.if_stmt();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:24: missing }");
}

TEST_F(ParserImplTest, IfStmt_InvalidElse) {
  ParserImpl p{"if (a) {} else { fn main() -> a{}}"};
  auto e = p.if_stmt();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:18: missing }");
}

TEST_F(ParserImplTest, IfStmt_InvalidPremerge) {
  ParserImpl p{"if (a) {} else {} premerge { fn main() -> a{}}"};
  auto e = p.if_stmt();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:30: missing }");
}

}  // namespace wgsl
}  // namespace reader
}  // namespace tint
