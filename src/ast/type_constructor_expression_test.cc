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

#include "gtest/gtest-spi.h"
#include "src/ast/test_helper.h"

namespace tint {
namespace ast {
namespace {

using TypeConstructorExpressionTest = TestHelper;

TEST_F(TypeConstructorExpressionTest, Creation) {
  ExpressionList expr;
  expr.push_back(Expr("expr"));

  auto* t = create<TypeConstructorExpression>(ty.f32(), expr);
  EXPECT_TRUE(t->type()->Is<ast::F32>());
  ASSERT_EQ(t->values().size(), 1u);
  EXPECT_EQ(t->values()[0], expr[0]);
}

TEST_F(TypeConstructorExpressionTest, Creation_WithSource) {
  ExpressionList expr;
  expr.push_back(Expr("expr"));

  auto* t = create<TypeConstructorExpression>(Source{Source::Location{20, 2}},
                                              ty.f32(), expr);
  auto src = t->source();
  EXPECT_EQ(src.range.begin.line, 20u);
  EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(TypeConstructorExpressionTest, IsTypeConstructor) {
  ExpressionList expr;
  expr.push_back(Expr("expr"));

  auto* t = create<TypeConstructorExpression>(ty.f32(), expr);
  EXPECT_TRUE(t->Is<TypeConstructorExpression>());
}

TEST_F(TypeConstructorExpressionTest, Assert_Null_Type) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b;
        b.create<TypeConstructorExpression>(nullptr, ExpressionList{b.Expr(1)});
      },
      "internal compiler error");
}

TEST_F(TypeConstructorExpressionTest, Assert_Null_Value) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b;
        b.create<TypeConstructorExpression>(b.ty.i32(),
                                            ExpressionList{nullptr});
      },
      "internal compiler error");
}

TEST_F(TypeConstructorExpressionTest, Assert_DifferentProgramID_Value) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b1;
        ProgramBuilder b2;
        b1.create<TypeConstructorExpression>(b1.ty.i32(),
                                             ExpressionList{b2.Expr(1)});
      },
      "internal compiler error");
}

TEST_F(TypeConstructorExpressionTest, ToStr) {
  ExpressionList expr;
  expr.push_back(Expr("expr_1"));
  expr.push_back(Expr("expr_2"));
  expr.push_back(Expr("expr_3"));

  auto* t = create<TypeConstructorExpression>(ty.vec3<f32>(), expr);
  EXPECT_EQ(str(t), R"(TypeConstructor[not set]{
  __vec_3__f32
  Identifier[not set]{expr_1}
  Identifier[not set]{expr_2}
  Identifier[not set]{expr_3}
}
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
