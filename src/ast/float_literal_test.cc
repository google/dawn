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
#include "src/ast/type/f32_type.h"

namespace tint {
namespace ast {
namespace {

using FloatLiteralTest = testing::Test;

TEST_F(FloatLiteralTest, Value) {
  ast::type::F32Type f32;
  FloatLiteral f{&f32, 47.2f};
  ASSERT_TRUE(f.IsFloat());
  EXPECT_EQ(f.value(), 47.2f);
}

TEST_F(FloatLiteralTest, Is) {
  ast::type::F32Type f32;
  FloatLiteral f{&f32, 42.f};
  EXPECT_FALSE(f.IsBool());
  EXPECT_FALSE(f.IsSint());
  EXPECT_FALSE(f.IsInt());
  EXPECT_TRUE(f.IsFloat());
  EXPECT_FALSE(f.IsUint());
  EXPECT_FALSE(f.IsNull());
}

TEST_F(FloatLiteralTest, ToStr) {
  ast::type::F32Type f32;
  FloatLiteral f{&f32, 42.1f};

  EXPECT_EQ(f.to_str(), "42.099998");
}

TEST_F(FloatLiteralTest, ToName) {
  ast::type::F32Type f32;
  FloatLiteral f{&f32, 42.1f};
  EXPECT_EQ(f.name(), "__float42.0999985");
}

}  // namespace
}  // namespace ast
}  // namespace tint
