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

#include "src/ast/bool_literal.h"
#include "src/ast/float_literal.h"
#include "src/ast/sint_literal.h"
#include "src/ast/test_helper.h"
#include "src/ast/uint_literal.h"
#include "src/type/i32_type.h"

namespace tint {
namespace ast {
namespace {

using NullLiteralTest = TestHelper;

TEST_F(NullLiteralTest, Is) {
  ast::Literal* l = create<NullLiteral>(ty.i32());
  EXPECT_FALSE(l->Is<BoolLiteral>());
  EXPECT_FALSE(l->Is<SintLiteral>());
  EXPECT_FALSE(l->Is<FloatLiteral>());
  EXPECT_FALSE(l->Is<UintLiteral>());
  EXPECT_FALSE(l->Is<IntLiteral>());
  EXPECT_TRUE(l->Is<NullLiteral>());
}

TEST_F(NullLiteralTest, ToStr) {
  auto* i = create<NullLiteral>(ty.i32());

  EXPECT_EQ(i->to_str(), "null __i32");
}

TEST_F(NullLiteralTest, Name_I32) {
  auto* i = create<NullLiteral>(ty.i32());
  EXPECT_EQ("__null__i32", i->name());
}

}  // namespace
}  // namespace ast
}  // namespace tint
