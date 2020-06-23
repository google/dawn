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

#include "gtest/gtest.h"
#include "src/ast/array_accessor_expression.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/type/i32_type.h"
#include "src/writer/wgsl/generator_impl.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using WgslGeneratorImplTest = testing::Test;

TEST_F(WgslGeneratorImplTest, EmitExpression_ArrayAccessor) {
  ast::type::I32Type i32;
  auto lit = std::make_unique<ast::SintLiteral>(&i32, 5);
  auto idx = std::make_unique<ast::ScalarConstructorExpression>(std::move(lit));
  auto ary = std::make_unique<ast::IdentifierExpression>("ary");

  ast::ArrayAccessorExpression expr(std::move(ary), std::move(idx));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitExpression(&expr)) << g.error();
  EXPECT_EQ(g.result(), "ary[5]");
}

TEST_F(WgslGeneratorImplTest, EmitArrayAccessor) {
  auto ary = std::make_unique<ast::IdentifierExpression>("ary");
  auto idx = std::make_unique<ast::IdentifierExpression>("idx");

  ast::ArrayAccessorExpression expr(std::move(ary), std::move(idx));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitArrayAccessor(&expr)) << g.error();
  EXPECT_EQ(g.result(), "ary[idx]");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
