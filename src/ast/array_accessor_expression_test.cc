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

#include "src/ast/array_accessor_expression.h"

#include "src/ast/identifier_expression.h"
#include "src/ast/test_helper.h"

namespace tint {
namespace ast {
namespace {

using ArrayAccessorExpressionTest = TestHelper;

TEST_F(ArrayAccessorExpressionTest, Create) {
  auto* ary =
      create<IdentifierExpression>(Source{}, mod.RegisterSymbol("ary"), "ary");
  auto* idx =
      create<IdentifierExpression>(Source{}, mod.RegisterSymbol("idx"), "idx");

  ArrayAccessorExpression exp(Source{}, ary, idx);
  ASSERT_EQ(exp.array(), ary);
  ASSERT_EQ(exp.idx_expr(), idx);
}

TEST_F(ArrayAccessorExpressionTest, CreateWithSource) {
  auto* ary =
      create<IdentifierExpression>(Source{}, mod.RegisterSymbol("ary"), "ary");
  auto* idx =
      create<IdentifierExpression>(Source{}, mod.RegisterSymbol("idx"), "idx");

  ArrayAccessorExpression exp(Source{Source::Location{20, 2}}, ary, idx);
  auto src = exp.source();
  EXPECT_EQ(src.range.begin.line, 20u);
  EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(ArrayAccessorExpressionTest, IsArrayAccessor) {
  auto* ary =
      create<IdentifierExpression>(Source{}, mod.RegisterSymbol("ary"), "ary");
  auto* idx =
      create<IdentifierExpression>(Source{}, mod.RegisterSymbol("idx"), "idx");

  ArrayAccessorExpression exp(Source{}, ary, idx);
  EXPECT_TRUE(exp.Is<ArrayAccessorExpression>());
}

TEST_F(ArrayAccessorExpressionTest, IsValid) {
  auto* ary =
      create<IdentifierExpression>(Source{}, mod.RegisterSymbol("ary"), "ary");
  auto* idx =
      create<IdentifierExpression>(Source{}, mod.RegisterSymbol("idx"), "idx");

  ArrayAccessorExpression exp(Source{}, ary, idx);
  EXPECT_TRUE(exp.IsValid());
}

TEST_F(ArrayAccessorExpressionTest, IsValid_MissingArray) {
  auto* idx =
      create<IdentifierExpression>(Source{}, mod.RegisterSymbol("idx"), "idx");

  ArrayAccessorExpression exp(Source{}, nullptr, idx);
  EXPECT_FALSE(exp.IsValid());
}

TEST_F(ArrayAccessorExpressionTest, IsValid_MissingIndex) {
  auto* ary =
      create<IdentifierExpression>(Source{}, mod.RegisterSymbol("ary"), "ary");

  ArrayAccessorExpression exp(Source{}, ary, nullptr);
  EXPECT_FALSE(exp.IsValid());
}

TEST_F(ArrayAccessorExpressionTest, IsValid_InvalidArray) {
  auto* ary =
      create<IdentifierExpression>(Source{}, mod.RegisterSymbol(""), "");
  auto* idx =
      create<IdentifierExpression>(Source{}, mod.RegisterSymbol("idx"), "idx");
  ArrayAccessorExpression exp(Source{}, ary, idx);
  EXPECT_FALSE(exp.IsValid());
}

TEST_F(ArrayAccessorExpressionTest, IsValid_InvalidIndex) {
  auto* ary =
      create<IdentifierExpression>(Source{}, mod.RegisterSymbol("ary"), "ary");
  auto* idx =
      create<IdentifierExpression>(Source{}, mod.RegisterSymbol(""), "");
  ArrayAccessorExpression exp(Source{}, ary, idx);
  EXPECT_FALSE(exp.IsValid());
}

TEST_F(ArrayAccessorExpressionTest, ToStr) {
  auto* ary =
      create<IdentifierExpression>(Source{}, mod.RegisterSymbol("ary"), "ary");
  auto* idx =
      create<IdentifierExpression>(Source{}, mod.RegisterSymbol("idx"), "idx");

  ArrayAccessorExpression exp(Source{}, ary, idx);
  std::ostringstream out;
  exp.to_str(out, 2);

  EXPECT_EQ(demangle(out.str()), R"(  ArrayAccessor[not set]{
    Identifier[not set]{ary}
    Identifier[not set]{idx}
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
