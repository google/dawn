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

#include "src/ast/unary_derivative_expression.h"

#include <sstream>

#include "gtest/gtest.h"
#include "src/ast/identifier_expression.h"

namespace tint {
namespace ast {
namespace {

using UnaryDerivativeExpressionTest = testing::Test;

TEST_F(UnaryDerivativeExpressionTest, Creation) {
  auto ident = std::make_unique<IdentifierExpression>("ident");
  auto* ident_ptr = ident.get();

  UnaryDerivativeExpression d(UnaryDerivative::kDpdy,
                              DerivativeModifier::kCoarse, std::move(ident));
  EXPECT_EQ(d.param(), ident_ptr);
  EXPECT_EQ(d.modifier(), DerivativeModifier::kCoarse);
  EXPECT_EQ(d.op(), UnaryDerivative::kDpdy);
}

TEST_F(UnaryDerivativeExpressionTest, Creation_WithSource) {
  auto ident = std::make_unique<IdentifierExpression>("ident");

  UnaryDerivativeExpression d(Source{20, 2}, UnaryDerivative::kDpdy,
                              DerivativeModifier::kCoarse, std::move(ident));
  auto src = d.source();
  EXPECT_EQ(src.line, 20u);
  EXPECT_EQ(src.column, 2u);
}

TEST_F(UnaryDerivativeExpressionTest, IsUnaryDerivative) {
  UnaryDerivativeExpression d;
  EXPECT_TRUE(d.IsUnaryDerivative());
}

TEST_F(UnaryDerivativeExpressionTest, IsValid) {
  auto ident = std::make_unique<IdentifierExpression>("ident");
  UnaryDerivativeExpression d(UnaryDerivative::kDpdy,
                              DerivativeModifier::kCoarse, std::move(ident));
  EXPECT_TRUE(d.IsValid());
}

TEST_F(UnaryDerivativeExpressionTest, IsValid_NullParam) {
  UnaryDerivativeExpression d(UnaryDerivative::kDpdy,
                              DerivativeModifier::kCoarse, nullptr);
  EXPECT_FALSE(d.IsValid());
}

TEST_F(UnaryDerivativeExpressionTest, IsValid_InvalidParam) {
  auto ident = std::make_unique<IdentifierExpression>("");
  UnaryDerivativeExpression d(UnaryDerivative::kDpdy,
                              DerivativeModifier::kCoarse, std::move(ident));
  EXPECT_FALSE(d.IsValid());
}

TEST_F(UnaryDerivativeExpressionTest, ToStr) {
  auto ident = std::make_unique<IdentifierExpression>("ident");
  UnaryDerivativeExpression d(UnaryDerivative::kDpdy,
                              DerivativeModifier::kCoarse, std::move(ident));
  std::ostringstream out;
  d.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  UnaryDerivative{
    dpdy
    coarse
    Identifier{ident}
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
