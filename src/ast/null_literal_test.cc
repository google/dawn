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

#include "src/ast/null_literal.h"

#include "gtest/gtest.h"
#include "src/ast/type/i32_type.h"

namespace tint {
namespace ast {
namespace {

using NullLiteralTest = testing::Test;

TEST_F(NullLiteralTest, Is) {
  ast::type::I32Type i32;
  NullLiteral i{&i32};
  EXPECT_FALSE(i.IsBool());
  EXPECT_FALSE(i.IsSint());
  EXPECT_FALSE(i.IsFloat());
  EXPECT_FALSE(i.IsUint());
  EXPECT_FALSE(i.IsInt());
  EXPECT_TRUE(i.IsNull());
}

TEST_F(NullLiteralTest, ToStr) {
  ast::type::I32Type i32;
  NullLiteral i{&i32};

  EXPECT_EQ(i.to_str(), "null __i32");
}

TEST_F(NullLiteralTest, Name_I32) {
  ast::type::I32Type i32;
  NullLiteral i{&i32};
  EXPECT_EQ("__null__i32", i.name());
}

}  // namespace
}  // namespace ast
}  // namespace tint
