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

#include "src/ast/test_helper.h"

namespace tint {
namespace ast {
namespace {

using UintLiteralTest = TestHelper;

TEST_F(UintLiteralTest, Value) {
  auto* u = create<UintLiteral>(ty.u32(), 47);
  ASSERT_TRUE(u->Is<UintLiteral>());
  EXPECT_EQ(u->value(), 47u);
}

TEST_F(UintLiteralTest, Is) {
  ast::Literal* l = create<UintLiteral>(ty.u32(), 42);
  EXPECT_FALSE(l->Is<BoolLiteral>());
  EXPECT_FALSE(l->Is<SintLiteral>());
  EXPECT_FALSE(l->Is<FloatLiteral>());
  EXPECT_TRUE(l->Is<UintLiteral>());
}

TEST_F(UintLiteralTest, ToStr) {
  auto* u = create<UintLiteral>(ty.u32(), 42u);
  EXPECT_EQ(str(u), "42u");
}

}  // namespace
}  // namespace ast
}  // namespace tint
