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

#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_Bitcast = TestHelper;

TEST_F(HlslGeneratorImplTest_Bitcast, EmitExpression_Bitcast_Float) {
  auto* bitcast = create<ast::BitcastExpression>(ty.f32(), Expr(1));
  WrapInFunction(bitcast);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitExpression(pre, out, bitcast)) << gen.error();
  EXPECT_EQ(result(), "asfloat(1)");
}

TEST_F(HlslGeneratorImplTest_Bitcast, EmitExpression_Bitcast_Int) {
  auto* bitcast = create<ast::BitcastExpression>(ty.i32(), Expr(1u));
  WrapInFunction(bitcast);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitExpression(pre, out, bitcast)) << gen.error();
  EXPECT_EQ(result(), "asint(1u)");
}

TEST_F(HlslGeneratorImplTest_Bitcast, EmitExpression_Bitcast_Uint) {
  auto* bitcast = create<ast::BitcastExpression>(ty.u32(), Expr(1));
  WrapInFunction(bitcast);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitExpression(pre, out, bitcast)) << gen.error();
  EXPECT_EQ(result(), "asuint(1)");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
