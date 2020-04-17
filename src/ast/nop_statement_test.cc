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

#include "src/ast/nop_statement.h"

#include <sstream>

#include "gtest/gtest.h"

namespace tint {
namespace ast {
namespace {

using NopStatementTest = testing::Test;

TEST_F(NopStatementTest, Creation) {
  NopStatement n;
  EXPECT_EQ(n.line(), 0u);
  EXPECT_EQ(n.column(), 0u);
}

TEST_F(NopStatementTest, Creation_WithSource) {
  NopStatement n(Source{20, 2});
  EXPECT_EQ(n.line(), 20u);
  EXPECT_EQ(n.column(), 2u);
}

TEST_F(NopStatementTest, IsNop) {
  NopStatement n;
  EXPECT_TRUE(n.IsNop());
}

TEST_F(NopStatementTest, IsValid) {
  NopStatement n;
  EXPECT_TRUE(n.IsValid());
}

TEST_F(NopStatementTest, ToStr) {
  NopStatement n;
  std::ostringstream out;
  n.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Nop{}
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
