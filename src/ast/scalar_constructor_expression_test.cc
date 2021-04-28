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

using ScalarConstructorExpressionTest = TestHelper;

TEST_F(ScalarConstructorExpressionTest, Creation) {
  auto* b = create<BoolLiteral>(true);
  auto* c = create<ScalarConstructorExpression>(b);
  EXPECT_EQ(c->literal(), b);
}

TEST_F(ScalarConstructorExpressionTest, Creation_WithSource) {
  SetSource(Source{Source::Location{20, 2}});
  auto src = Expr(true)->source();
  EXPECT_EQ(src.range.begin.line, 20u);
  EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(ScalarConstructorExpressionTest, ToStr) {
  auto* c = Expr(true);
  EXPECT_EQ(str(c), R"(ScalarConstructor[not set]{true}
)");
}

TEST_F(ScalarConstructorExpressionTest, Assert_DifferentProgramID_Literal) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b1;
        ProgramBuilder b2;
        b1.create<ScalarConstructorExpression>(b2.create<BoolLiteral>(true));
      },
      "internal compiler error");
}

}  // namespace
}  // namespace ast
}  // namespace tint
