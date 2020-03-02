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

#include "src/ast/int_literal.h"

#include "gtest/gtest.h"

namespace tint {
namespace ast {

using IntLiteralTest = testing::Test;

TEST_F(IntLiteralTest, Value) {
  IntLiteral i{47};
  ASSERT_TRUE(i.IsInt());
  EXPECT_EQ(i.value(), 47);
}

TEST_F(IntLiteralTest, Is) {
  IntLiteral i{42};
  EXPECT_FALSE(i.IsBool());
  EXPECT_TRUE(i.IsInt());
  EXPECT_FALSE(i.IsFloat());
  EXPECT_FALSE(i.IsUint());
}

TEST_F(IntLiteralTest, ToStr) {
  IntLiteral i{-42};

  EXPECT_EQ(i.to_str(), "-42");
}

}  // namespace ast
}  // namespace tint
