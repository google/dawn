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

#include "src/ast/continue_statement.h"

#include "gtest/gtest.h"

namespace tint {
namespace ast {
namespace {

using ContinueStatementTest = testing::Test;

TEST_F(ContinueStatementTest, Creation_WithSource) {
  ContinueStatement stmt(Source{20, 2});
  auto src = stmt.source();
  EXPECT_EQ(src.line, 20u);
  EXPECT_EQ(src.column, 2u);
}

TEST_F(ContinueStatementTest, IsContinue) {
  ContinueStatement stmt;
  EXPECT_TRUE(stmt.IsContinue());
}

TEST_F(ContinueStatementTest, IsValid) {
  ContinueStatement stmt;
  EXPECT_TRUE(stmt.IsValid());
}

TEST_F(ContinueStatementTest, ToStr) {
  ContinueStatement stmt;
  std::ostringstream out;
  stmt.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Continue{}
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
