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

#include "src/ast/float_literal.h"

#include "gtest/gtest.h"

namespace tint {
namespace ast {

using FloatLiteralTest = testing::Test;

TEST_F(FloatLiteralTest, Value) {
  FloatLiteral f{47.2};
  ASSERT_TRUE(f.IsFloat());
  EXPECT_EQ(f.value(), 47.2);
}

TEST_F(FloatLiteralTest, Is) {
  FloatLiteral f{42};
  EXPECT_FALSE(f.IsBool());
  EXPECT_FALSE(f.IsInt());
  EXPECT_TRUE(f.IsFloat());
  EXPECT_FALSE(f.IsUint());
}

TEST_F(FloatLiteralTest, ToStr) {
  FloatLiteral f{42.1};

  EXPECT_EQ(f.to_str(), "42.1");
}

}  // namespace ast
}  // namespace tint
