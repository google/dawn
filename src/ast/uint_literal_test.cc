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
#include "src/ast/type/u32_type.h"

namespace tint {
namespace ast {
namespace {

using UintLiteralTest = testing::Test;

TEST_F(UintLiteralTest, Value) {
  ast::type::U32Type u32;
  UintLiteral u{&u32, 47};
  ASSERT_TRUE(u.IsUint());
  EXPECT_EQ(u.value(), 47u);
}

TEST_F(UintLiteralTest, Is) {
  ast::type::U32Type u32;
  UintLiteral u{&u32, 42};
  EXPECT_FALSE(u.IsBool());
  EXPECT_FALSE(u.IsSint());
  EXPECT_FALSE(u.IsFloat());
  EXPECT_TRUE(u.IsUint());
  EXPECT_FALSE(u.IsNull());
}

TEST_F(UintLiteralTest, ToStr) {
  ast::type::U32Type u32;
  UintLiteral i{&u32, 42};

  EXPECT_EQ(i.to_str(), "42");
}

}  // namespace
}  // namespace ast
}  // namespace tint
