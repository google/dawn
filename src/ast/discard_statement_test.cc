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

#include <sstream>

#include "gtest/gtest.h"

namespace tint {
namespace ast {
namespace {

using DiscardStatementTest = testing::Test;

TEST_F(DiscardStatementTest, Creation) {
  DiscardStatement stmt;
  EXPECT_EQ(stmt.line(), 0u);
  EXPECT_EQ(stmt.column(), 0u);
}

TEST_F(DiscardStatementTest, Creation_WithSource) {
  DiscardStatement stmt(Source{20, 2});
  EXPECT_EQ(stmt.line(), 20u);
  EXPECT_EQ(stmt.column(), 2u);
}

TEST_F(DiscardStatementTest, IsDiscard) {
  DiscardStatement stmt;
  EXPECT_TRUE(stmt.IsDiscard());
}

TEST_F(DiscardStatementTest, IsValid) {
  DiscardStatement stmt;
  EXPECT_TRUE(stmt.IsValid());
}

TEST_F(DiscardStatementTest, ToStr) {
  DiscardStatement stmt;
  std::ostringstream out;
  stmt.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Discard{}
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
