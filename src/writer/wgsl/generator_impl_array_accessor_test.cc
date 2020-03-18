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
#include "src/ast/const_initializer_expression.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/int_literal.h"
#include "src/writer/wgsl/generator_impl.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using GeneratorImplTest = testing::Test;

TEST_F(GeneratorImplTest, EmitExpression_ArrayAccessor) {
  auto lit = std::make_unique<ast::IntLiteral>(5);
  auto idx = std::make_unique<ast::ConstInitializerExpression>(std::move(lit));
  auto ary = std::make_unique<ast::IdentifierExpression>("ary");

  ast::ArrayAccessorExpression expr(std::move(ary), std::move(idx));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitExpression(&expr)) << g.error();
  EXPECT_EQ(g.result(), "ary[5]");
}

TEST_F(GeneratorImplTest, EmitArrayAccessor) {
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
