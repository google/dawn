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

using BoolLiteralTest = TestHelper;

TEST_F(BoolLiteralTest, True) {
  auto* b = create<BoolLiteral>(true);
  ASSERT_TRUE(b->Is<BoolLiteral>());
  ASSERT_TRUE(b->IsTrue());
  ASSERT_FALSE(b->IsFalse());
}

TEST_F(BoolLiteralTest, False) {
  auto* b = create<BoolLiteral>(false);
  ASSERT_TRUE(b->Is<BoolLiteral>());
  ASSERT_FALSE(b->IsTrue());
  ASSERT_TRUE(b->IsFalse());
}

TEST_F(BoolLiteralTest, ToStr) {
  auto* t = create<BoolLiteral>(true);
  auto* f = create<BoolLiteral>(false);

  EXPECT_EQ(str(t), "true");
  EXPECT_EQ(str(f), "false");
}

}  // namespace
}  // namespace ast
}  // namespace tint
