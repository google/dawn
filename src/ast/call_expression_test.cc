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

using CallExpressionTest = TestHelper;

TEST_F(CallExpressionTest, Creation) {
  auto* func = Expr("func");
  ExpressionList params;
  params.push_back(Expr("param1"));
  params.push_back(Expr("param2"));

  auto* stmt = create<CallExpression>(func, params);
  EXPECT_EQ(stmt->func(), func);

  const auto& vec = stmt->params();
  ASSERT_EQ(vec.size(), 2u);
  EXPECT_EQ(vec[0], params[0]);
  EXPECT_EQ(vec[1], params[1]);
}

TEST_F(CallExpressionTest, Creation_WithSource) {
  auto* func = Expr("func");
  auto* stmt = create<CallExpression>(Source{Source::Location{20, 2}}, func,
                                      ExpressionList{});
  auto src = stmt->source();
  EXPECT_EQ(src.range.begin.line, 20u);
  EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(CallExpressionTest, IsCall) {
  auto* func = Expr("func");
  auto* stmt = create<CallExpression>(func, ExpressionList{});
  EXPECT_TRUE(stmt->Is<CallExpression>());
}

TEST_F(CallExpressionTest, Assert_Null_Function) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b;
        b.create<CallExpression>(nullptr, ExpressionList{});
      },
      "internal compiler error");
}

TEST_F(CallExpressionTest, Assert_Null_Param) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b;
        ExpressionList params;
        params.push_back(b.Expr("param1"));
        params.push_back(nullptr);
        params.push_back(b.Expr("param2"));
        b.create<CallExpression>(b.Expr("func"), params);
      },
      "internal compiler error");
}

TEST_F(CallExpressionTest, Assert_DifferentProgramID_Function) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b1;
        ProgramBuilder b2;
        b1.create<CallExpression>(b2.Expr("func"), ExpressionList{});
      },
      "internal compiler error");
}

TEST_F(CallExpressionTest, Assert_DifferentProgramID_Param) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b1;
        ProgramBuilder b2;
        b1.create<CallExpression>(b1.Expr("func"),
                                  ExpressionList{b2.Expr("param1")});
      },
      "internal compiler error");
}

TEST_F(CallExpressionTest, ToStr_NoParams) {
  auto* func = Expr("func");
  auto* stmt = create<CallExpression>(func, ExpressionList{});
  EXPECT_EQ(str(stmt), R"(Call[not set]{
  Identifier[not set]{func}
  (
  )
}
)");
}

TEST_F(CallExpressionTest, ToStr_WithParams) {
  auto* func = Expr("func");
  ExpressionList params;
  params.push_back(Expr("param1"));
  params.push_back(Expr("param2"));

  auto* stmt = create<CallExpression>(func, params);
  EXPECT_EQ(str(stmt), R"(Call[not set]{
  Identifier[not set]{func}
  (
    Identifier[not set]{param1}
    Identifier[not set]{param2}
  )
}
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
