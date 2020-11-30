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

#include "src/ast/sint_literal.h"

#include "src/ast/bool_literal.h"
#include "src/ast/float_literal.h"
#include "src/ast/null_literal.h"
#include "src/ast/test_helper.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/uint_literal.h"

namespace tint {
namespace ast {
namespace {

using SintLiteralTest = TestHelper;

TEST_F(SintLiteralTest, Value) {
  type::I32 i32;
  SintLiteral i{&i32, 47};
  ASSERT_TRUE(i.Is<SintLiteral>());
  EXPECT_EQ(i.value(), 47);
}

TEST_F(SintLiteralTest, Is) {
  type::I32 i32;
  SintLiteral i{&i32, 42};
  Literal* l = &i;
  EXPECT_FALSE(l->Is<BoolLiteral>());
  EXPECT_TRUE(l->Is<SintLiteral>());
  EXPECT_FALSE(l->Is<FloatLiteral>());
  EXPECT_FALSE(l->Is<UintLiteral>());
  EXPECT_FALSE(l->Is<NullLiteral>());
}

TEST_F(SintLiteralTest, ToStr) {
  type::I32 i32;
  SintLiteral i{&i32, -42};

  EXPECT_EQ(i.to_str(), "-42");
}

TEST_F(SintLiteralTest, Name_I32) {
  type::I32 i32;
  SintLiteral i{&i32, 2};
  EXPECT_EQ("__sint__i32_2", i.name());
}

TEST_F(SintLiteralTest, Name_U32) {
  type::U32 u32;
  SintLiteral i{&u32, 2};
  EXPECT_EQ("__sint__u32_2", i.name());
}

}  // namespace
}  // namespace ast
}  // namespace tint
