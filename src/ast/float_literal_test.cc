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

#include "src/ast/bool_literal.h"
#include "src/ast/null_literal.h"
#include "src/ast/sint_literal.h"
#include "src/ast/test_helper.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/uint_literal.h"

namespace tint {
namespace ast {
namespace {

using FloatLiteralTest = TestHelper;

TEST_F(FloatLiteralTest, Value) {
  type::F32 f32;
  FloatLiteral f{&f32, 47.2f};
  ASSERT_TRUE(f.Is<FloatLiteral>());
  EXPECT_EQ(f.value(), 47.2f);
}

TEST_F(FloatLiteralTest, Is) {
  type::F32 f32;
  FloatLiteral f{&f32, 42.f};
  Literal* l = &f;
  EXPECT_FALSE(l->Is<BoolLiteral>());
  EXPECT_FALSE(l->Is<SintLiteral>());
  EXPECT_FALSE(l->Is<IntLiteral>());
  EXPECT_TRUE(l->Is<FloatLiteral>());
  EXPECT_FALSE(l->Is<UintLiteral>());
  EXPECT_FALSE(l->Is<NullLiteral>());
}

TEST_F(FloatLiteralTest, ToStr) {
  type::F32 f32;
  FloatLiteral f{&f32, 42.1f};

  EXPECT_EQ(f.to_str(), "42.099998");
}

TEST_F(FloatLiteralTest, ToName) {
  type::F32 f32;
  FloatLiteral f{&f32, 42.1f};
  EXPECT_EQ(f.name(), "__float42.0999985");
}

}  // namespace
}  // namespace ast
}  // namespace tint
