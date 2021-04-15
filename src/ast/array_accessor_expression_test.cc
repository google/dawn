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

using ArrayAccessorExpressionTest = TestHelper;

TEST_F(ArrayAccessorExpressionTest, Create) {
  auto* ary = Expr("ary");
  auto* idx = Expr("idx");

  auto* exp = create<ArrayAccessorExpression>(ary, idx);
  ASSERT_EQ(exp->array(), ary);
  ASSERT_EQ(exp->idx_expr(), idx);
}

TEST_F(ArrayAccessorExpressionTest, CreateWithSource) {
  auto* ary = Expr("ary");
  auto* idx = Expr("idx");

  auto* exp = create<ArrayAccessorExpression>(Source{Source::Location{20, 2}},
                                              ary, idx);
  auto src = exp->source();
  EXPECT_EQ(src.range.begin.line, 20u);
  EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(ArrayAccessorExpressionTest, IsArrayAccessor) {
  auto* ary = Expr("ary");
  auto* idx = Expr("idx");

  auto* exp = create<ArrayAccessorExpression>(ary, idx);
  EXPECT_TRUE(exp->Is<ArrayAccessorExpression>());
}

TEST_F(ArrayAccessorExpressionTest, Assert_Null_Array) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b;
        b.create<ArrayAccessorExpression>(nullptr, b.Expr("idx"));
      },
      "internal compiler error");
}

TEST_F(ArrayAccessorExpressionTest, Assert_Null_Index) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b;
        b.create<ArrayAccessorExpression>(b.Expr("arr"), nullptr);
      },
      "internal compiler error");
}

TEST_F(ArrayAccessorExpressionTest, Assert_DifferentProgramID_Array) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b1;
        ProgramBuilder b2;
        b1.create<ArrayAccessorExpression>(b2.Expr("arr"), b1.Expr("idx"));
      },
      "internal compiler error");
}

TEST_F(ArrayAccessorExpressionTest, Assert_DifferentProgramID_Index) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b1;
        ProgramBuilder b2;
        b1.create<ArrayAccessorExpression>(b1.Expr("arr"), b2.Expr("idx"));
      },
      "internal compiler error");
}

TEST_F(ArrayAccessorExpressionTest, ToStr) {
  auto* ary = Expr("ary");
  auto* idx = Expr("idx");

  auto* exp = create<ArrayAccessorExpression>(ary, idx);
  EXPECT_EQ(str(exp), R"(ArrayAccessor[not set]{
  Identifier[not set]{ary}
  Identifier[not set]{idx}
}
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
