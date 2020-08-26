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

#include <memory>

#include "src/ast/as_expression.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/module.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/u32_type.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_As = TestHelper;

TEST_F(HlslGeneratorImplTest_As, EmitExpression_As_Float) {
  ast::type::F32Type f32;
  auto id = std::make_unique<ast::IdentifierExpression>("id");
  ast::AsExpression as(&f32, std::move(id));

  ASSERT_TRUE(gen().EmitExpression(out(), &as)) << gen().error();
  EXPECT_EQ(result(), "asfloat(id)");
}

TEST_F(HlslGeneratorImplTest_As, EmitExpression_As_Int) {
  ast::type::I32Type i32;
  auto id = std::make_unique<ast::IdentifierExpression>("id");
  ast::AsExpression as(&i32, std::move(id));

  ASSERT_TRUE(gen().EmitExpression(out(), &as)) << gen().error();
  EXPECT_EQ(result(), "asint(id)");
}

TEST_F(HlslGeneratorImplTest_As, EmitExpression_As_Uint) {
  ast::type::U32Type u32;
  auto id = std::make_unique<ast::IdentifierExpression>("id");
  ast::AsExpression as(&u32, std::move(id));

  ASSERT_TRUE(gen().EmitExpression(out(), &as)) << gen().error();
  EXPECT_EQ(result(), "asuint(id)");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
