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
#include "src/ast/continue_statement.h"
#include "src/ast/return_statement.h"
#include "src/ast/statement.h"
#include "src/reader/wgsl/parser_impl.h"

namespace tint {
namespace reader {
namespace wgsl {

using ParserImplTest = testing::Test;

TEST_F(ParserImplTest, ContinueStmt) {
  ParserImpl p{"continue"};
  auto e = p.continue_stmt();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsContinue());
  EXPECT_EQ(e->condition(), ast::StatementCondition::kNone);
  EXPECT_EQ(e->conditional(), nullptr);
}

TEST_F(ParserImplTest, ContinueStmt_WithIf) {
  ParserImpl p{"continue if (a == b)"};
  auto e = p.continue_stmt();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsContinue());
  EXPECT_EQ(e->condition(), ast::StatementCondition::kIf);
  ASSERT_NE(e->conditional(), nullptr);
  EXPECT_TRUE(e->conditional()->IsRelational());
}

TEST_F(ParserImplTest, ContinueStmt_WithUnless) {
  ParserImpl p{"continue unless (a == b)"};
  auto e = p.continue_stmt();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsContinue());
  EXPECT_EQ(e->condition(), ast::StatementCondition::kUnless);
  ASSERT_NE(e->conditional(), nullptr);
  EXPECT_TRUE(e->conditional()->IsRelational());
}

TEST_F(ParserImplTest, ContinueStmt_InvalidRHS) {
  ParserImpl p{"continue if (a = b)"};
  auto e = p.continue_stmt();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:16: expected )");
}

TEST_F(ParserImplTest, ContinueStmt_MissingRHS) {
  ParserImpl p{"continue if"};
  auto e = p.continue_stmt();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:12: expected (");
}

}  // namespace wgsl
}  // namespace reader
}  // namespace tint
