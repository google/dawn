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

#include "src/ast/bool_literal.h"
#include "src/ast/float_literal.h"
#include "src/ast/null_literal.h"
#include "src/ast/sint_literal.h"
#include "src/ast/test_helper.h"
#include "src/ast/uint_literal.h"
#include "src/type/i32_type.h"
#include "src/type/u32_type.h"

namespace tint {
namespace ast {
namespace {

using IntLiteralTest = TestHelper;

TEST_F(IntLiteralTest, Sint_IsInt) {
  auto* i = create<SintLiteral>(ty.i32, 47);
  ASSERT_TRUE(i->Is<IntLiteral>());
}

TEST_F(IntLiteralTest, Uint_IsInt) {
  auto* i = create<UintLiteral>(ty.i32, 42);
  EXPECT_TRUE(i->Is<IntLiteral>());
}

}  // namespace
}  // namespace ast
}  // namespace tint
