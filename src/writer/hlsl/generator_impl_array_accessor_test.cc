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

#include "src/ast/array_accessor_expression.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/module.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/type/i32_type.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_Expression = TestHelper;

TEST_F(HlslGeneratorImplTest_Expression, EmitExpression_ArrayAccessor) {
  ast::type::I32Type i32;
  auto lit = std::make_unique<ast::SintLiteral>(&i32, 5);
  auto idx = std::make_unique<ast::ScalarConstructorExpression>(std::move(lit));
  auto ary = std::make_unique<ast::IdentifierExpression>("ary");

  ast::ArrayAccessorExpression expr(std::move(ary), std::move(idx));

  ASSERT_TRUE(gen().EmitExpression(out(), &expr)) << gen().error();
  EXPECT_EQ(result(), "ary[5]");
}

TEST_F(HlslGeneratorImplTest_Expression, EmitArrayAccessor) {
  auto ary = std::make_unique<ast::IdentifierExpression>("ary");
  auto idx = std::make_unique<ast::IdentifierExpression>("idx");

  ast::ArrayAccessorExpression expr(std::move(ary), std::move(idx));

  ASSERT_TRUE(gen().EmitExpression(out(), &expr)) << gen().error();
  EXPECT_EQ(result(), "ary[idx]");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
