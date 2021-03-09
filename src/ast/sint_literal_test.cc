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
#include "src/ast/test_helper.h"

namespace tint {
namespace ast {
namespace {

using SintLiteralTest = TestHelper;

TEST_F(SintLiteralTest, Value) {
  auto* i = create<SintLiteral>(ty.i32(), 47);
  ASSERT_TRUE(i->Is<SintLiteral>());
  EXPECT_EQ(i->value(), 47);
}

TEST_F(SintLiteralTest, Is) {
  ast::Literal* l = create<SintLiteral>(ty.i32(), 42);
  EXPECT_FALSE(l->Is<BoolLiteral>());
  EXPECT_TRUE(l->Is<SintLiteral>());
  EXPECT_FALSE(l->Is<FloatLiteral>());
  EXPECT_FALSE(l->Is<UintLiteral>());
  EXPECT_FALSE(l->Is<NullLiteral>());
}

TEST_F(SintLiteralTest, ToStr) {
  auto* i = create<SintLiteral>(ty.i32(), -42);
  EXPECT_EQ(str(i), "-42");
}

TEST_F(SintLiteralTest, Name_I32) {
  auto* i = create<SintLiteral>(ty.i32(), 2);
  EXPECT_EQ("__sint__i32_2", i->name());
}

TEST_F(SintLiteralTest, Name_U32) {
  type::U32 u32;
  auto* i = create<SintLiteral>(&u32, 2);
  EXPECT_EQ("__sint__u32_2", i->name());
}

}  // namespace
}  // namespace ast
}  // namespace tint
