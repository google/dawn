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

#include "gtest/gtest.h"

namespace tint {
namespace ast {
namespace {

using FallthroughStatementTest = testing::Test;

TEST_F(FallthroughStatementTest, Creation) {
  FallthroughStatement stmt;
  EXPECT_EQ(stmt.line(), 0u);
  EXPECT_EQ(stmt.column(), 0u);
}

TEST_F(FallthroughStatementTest, Creation_WithSource) {
  FallthroughStatement stmt(Source{20, 2});
  auto src = stmt.source();
  EXPECT_EQ(src.line, 20u);
  EXPECT_EQ(src.column, 2u);
}

TEST_F(FallthroughStatementTest, IsFallthrough) {
  FallthroughStatement stmt;
  EXPECT_TRUE(stmt.IsFallthrough());
}

TEST_F(FallthroughStatementTest, IsValid) {
  FallthroughStatement stmt;
  EXPECT_TRUE(stmt.IsValid());
}

TEST_F(FallthroughStatementTest, ToStr) {
  FallthroughStatement stmt;
  std::ostringstream out;
  stmt.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Fallthrough{}
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
