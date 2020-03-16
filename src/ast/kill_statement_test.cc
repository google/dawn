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

#include "src/ast/kill_statement.h"

#include <sstream>

#include "gtest/gtest.h"

namespace tint {
namespace ast {

using KillStatementTest = testing::Test;

TEST_F(KillStatementTest, Creation) {
  KillStatement k;
  EXPECT_EQ(k.line(), 0);
  EXPECT_EQ(k.column(), 0);
}

TEST_F(KillStatementTest, Creation_WithSource) {
  KillStatement k(Source{20, 2});
  EXPECT_EQ(k.line(), 20);
  EXPECT_EQ(k.column(), 2);
}

TEST_F(KillStatementTest, IsKill) {
  KillStatement k;
  EXPECT_TRUE(k.IsKill());
}

TEST_F(KillStatementTest, IsValid) {
  KillStatement k;
  EXPECT_TRUE(k.IsValid());
}

TEST_F(KillStatementTest, ToStr) {
  KillStatement k;
  std::ostringstream out;
  k.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Kill{}
)");
}

}  // namespace ast
}  // namespace tint

