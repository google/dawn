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

#include "src/ast/scalar_constructor_expression.h"

#include "src/ast/bool_literal.h"
#include "src/ast/test_helper.h"
#include "src/type/bool_type.h"

namespace tint {
namespace ast {
namespace {

using ScalarConstructorExpressionTest = TestHelper;

TEST_F(ScalarConstructorExpressionTest, Creation) {
  auto* b = create<BoolLiteral>(ty.bool_(), true);
  auto* c = create<ScalarConstructorExpression>(b);
  EXPECT_EQ(c->literal(), b);
}

TEST_F(ScalarConstructorExpressionTest, Creation_WithSource) {
  SetSource(Source{Source::Location{20, 2}});
  auto src = Expr(true)->source();
  EXPECT_EQ(src.range.begin.line, 20u);
  EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(ScalarConstructorExpressionTest, IsValid) {
  auto* c = Expr(true);
  EXPECT_TRUE(c->IsValid());
}

TEST_F(ScalarConstructorExpressionTest, IsValid_MissingLiteral) {
  auto* c = create<ScalarConstructorExpression>(nullptr);
  EXPECT_FALSE(c->IsValid());
}

TEST_F(ScalarConstructorExpressionTest, ToStr) {
  auto* c = Expr(true);
  std::ostringstream out;
  c->to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  ScalarConstructor[not set]{true}
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
