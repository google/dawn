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

#include "src/ast/bitcast_expression.h"

#include "src/ast/identifier_expression.h"
#include "src/ast/test_helper.h"
#include "src/type/f32_type.h"

namespace tint {
namespace ast {
namespace {

using BitcastExpressionTest = TestHelper;

TEST_F(BitcastExpressionTest, Create) {
  auto* expr = Expr("expr");

  auto* exp = create<BitcastExpression>(ty.f32(), expr);
  ASSERT_EQ(exp->type(), ty.f32());
  ASSERT_EQ(exp->expr(), expr);
}

TEST_F(BitcastExpressionTest, CreateWithSource) {
  auto* expr = Expr("expr");

  auto* exp = create<BitcastExpression>(Source{Source::Location{20, 2}},
                                        ty.f32(), expr);
  auto src = exp->source();
  EXPECT_EQ(src.range.begin.line, 20u);
  EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(BitcastExpressionTest, IsBitcast) {
  auto* expr = Expr("expr");

  auto* exp = create<BitcastExpression>(ty.f32(), expr);
  EXPECT_TRUE(exp->Is<BitcastExpression>());
}

TEST_F(BitcastExpressionTest, IsValid) {
  auto* expr = Expr("expr");

  auto* exp = create<BitcastExpression>(ty.f32(), expr);
  EXPECT_TRUE(exp->IsValid());
}

TEST_F(BitcastExpressionTest, IsValid_MissingType) {
  auto* expr = Expr("expr");

  auto* exp = create<BitcastExpression>(nullptr, expr);
  EXPECT_FALSE(exp->IsValid());
}

TEST_F(BitcastExpressionTest, IsValid_MissingExpr) {
  auto* exp = create<BitcastExpression>(ty.f32(), nullptr);
  EXPECT_FALSE(exp->IsValid());
}

TEST_F(BitcastExpressionTest, IsValid_InvalidExpr) {
  auto* expr = Expr("");
  auto* e = create<BitcastExpression>(ty.f32(), expr);
  EXPECT_FALSE(e->IsValid());
}

TEST_F(BitcastExpressionTest, ToStr) {
  auto* expr = Expr("expr");

  auto* exp = create<BitcastExpression>(ty.f32(), expr);
  std::ostringstream out;
  exp->to_str(Sem(), out, 2);

  EXPECT_EQ(demangle(out.str()), R"(  Bitcast[not set]<__f32>{
    Identifier[not set]{expr}
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
