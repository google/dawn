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

#include "src/ast/bool_literal.h"

#include "gtest/gtest.h"
#include "src/ast/type/bool_type.h"

namespace tint {
namespace ast {
namespace {

using BoolLiteralTest = testing::Test;

TEST_F(BoolLiteralTest, True) {
  ast::type::BoolType bool_type;
  BoolLiteral b{&bool_type, true};
  ASSERT_TRUE(b.IsBool());
  ASSERT_TRUE(b.IsTrue());
  ASSERT_FALSE(b.IsFalse());
}

TEST_F(BoolLiteralTest, False) {
  ast::type::BoolType bool_type;
  BoolLiteral b{&bool_type, false};
  ASSERT_TRUE(b.IsBool());
  ASSERT_FALSE(b.IsTrue());
  ASSERT_TRUE(b.IsFalse());
}

TEST_F(BoolLiteralTest, Is) {
  ast::type::BoolType bool_type;
  BoolLiteral b{&bool_type, false};
  EXPECT_TRUE(b.IsBool());
  EXPECT_FALSE(b.IsSint());
  EXPECT_FALSE(b.IsFloat());
  EXPECT_FALSE(b.IsUint());
  EXPECT_FALSE(b.IsInt());
  EXPECT_FALSE(b.IsNull());
}

TEST_F(BoolLiteralTest, ToStr) {
  ast::type::BoolType bool_type;
  BoolLiteral t{&bool_type, true};
  BoolLiteral f{&bool_type, false};

  EXPECT_EQ(t.to_str(), "true");
  EXPECT_EQ(f.to_str(), "false");
}

}  // namespace
}  // namespace ast
}  // namespace tint
