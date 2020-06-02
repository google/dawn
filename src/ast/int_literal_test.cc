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
#include "src/ast/sint_literal.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/uint_literal.h"

namespace tint {
namespace ast {
namespace {

using IntLiteralTest = testing::Test;

TEST_F(IntLiteralTest, Sint_IsInt) {
  ast::type::I32Type i32;
  SintLiteral i{&i32, 47};
  ASSERT_TRUE(i.IsInt());
}

TEST_F(IntLiteralTest, Uint_IsInt) {
  ast::type::I32Type i32;
  UintLiteral i{&i32, 42};
  EXPECT_TRUE(i.IsInt());
}

}  // namespace
}  // namespace ast
}  // namespace tint
