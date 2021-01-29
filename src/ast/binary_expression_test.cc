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

#include "src/ast/binary_expression.h"

#include <sstream>

#include "src/ast/identifier_expression.h"
#include "src/ast/test_helper.h"

namespace tint {
namespace ast {
namespace {

using BinaryExpressionTest = TestHelper;

TEST_F(BinaryExpressionTest, Creation) {
  auto* lhs = Expr("lhs");
  auto* rhs = Expr("rhs");

  auto* r = create<BinaryExpression>(BinaryOp::kEqual, lhs, rhs);
  EXPECT_EQ(r->lhs(), lhs);
  EXPECT_EQ(r->rhs(), rhs);
  EXPECT_EQ(r->op(), BinaryOp::kEqual);
}

TEST_F(BinaryExpressionTest, Creation_WithSource) {
  auto* lhs = Expr("lhs");
  auto* rhs = Expr("rhs");

  auto* r = create<BinaryExpression>(Source{Source::Location{20, 2}},
                                     BinaryOp::kEqual, lhs, rhs);
  auto src = r->source();
  EXPECT_EQ(src.range.begin.line, 20u);
  EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(BinaryExpressionTest, IsBinary) {
  auto* lhs = Expr("lhs");
  auto* rhs = Expr("rhs");

  auto* r = create<BinaryExpression>(BinaryOp::kEqual, lhs, rhs);
  EXPECT_TRUE(r->Is<BinaryExpression>());
}

TEST_F(BinaryExpressionTest, IsValid) {
  auto* lhs = Expr("lhs");
  auto* rhs = Expr("rhs");

  auto* r = create<BinaryExpression>(BinaryOp::kEqual, lhs, rhs);
  EXPECT_TRUE(r->IsValid());
}

TEST_F(BinaryExpressionTest, IsValid_Null_LHS) {
  auto* rhs = Expr("rhs");

  auto* r = create<BinaryExpression>(BinaryOp::kEqual, nullptr, rhs);
  EXPECT_FALSE(r->IsValid());
}

TEST_F(BinaryExpressionTest, IsValid_Invalid_LHS) {
  auto* lhs = Expr("");
  auto* rhs = Expr("rhs");

  auto* r = create<BinaryExpression>(BinaryOp::kEqual, lhs, rhs);
  EXPECT_FALSE(r->IsValid());
}

TEST_F(BinaryExpressionTest, IsValid_Null_RHS) {
  auto* lhs = Expr("lhs");

  auto* r = create<BinaryExpression>(BinaryOp::kEqual, lhs, nullptr);
  EXPECT_FALSE(r->IsValid());
}

TEST_F(BinaryExpressionTest, IsValid_Invalid_RHS) {
  auto* lhs = Expr("lhs");
  auto* rhs = Expr("");

  auto* r = create<BinaryExpression>(BinaryOp::kEqual, lhs, rhs);
  EXPECT_FALSE(r->IsValid());
}

TEST_F(BinaryExpressionTest, IsValid_Binary_None) {
  auto* lhs = Expr("lhs");
  auto* rhs = Expr("rhs");

  auto* r = create<BinaryExpression>(BinaryOp::kNone, lhs, rhs);
  EXPECT_FALSE(r->IsValid());
}

TEST_F(BinaryExpressionTest, ToStr) {
  auto* lhs = Expr("lhs");
  auto* rhs = Expr("rhs");

  auto* r = create<BinaryExpression>(BinaryOp::kEqual, lhs, rhs);
  std::ostringstream out;
  r->to_str(Sem(), out, 2);
  EXPECT_EQ(demangle(out.str()), R"(  Binary[not set]{
    Identifier[not set]{lhs}
    equal
    Identifier[not set]{rhs}
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
