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

#include "src/ast/uint_literal.h"

#include "gtest/gtest.h"

namespace tint {
namespace ast {

using UintLiteralTest = testing::Test;

TEST_F(UintLiteralTest, Value) {
  UintLiteral u{47};
  ASSERT_TRUE(u.IsUint());
  EXPECT_EQ(u.value(), 47);
}

TEST_F(UintLiteralTest, Is) {
  UintLiteral u{42};
  EXPECT_FALSE(u.IsBool());
  EXPECT_FALSE(u.IsInt());
  EXPECT_FALSE(u.IsFloat());
  EXPECT_TRUE(u.IsUint());
}

TEST_F(UintLiteralTest, ToStr) {
  UintLiteral i{42};

  EXPECT_EQ(i.to_str(), "42");
}

}  // namespace ast
}  // namespace tint
