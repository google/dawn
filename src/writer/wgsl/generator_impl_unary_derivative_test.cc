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
#include "src/ast/identifier_expression.h"
#include "src/ast/unary_derivative_expression.h"
#include "src/writer/wgsl/generator_impl.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

struct UnaryDerivativeData {
  const char* name;
  ast::UnaryDerivative derivative;
};
inline std::ostream& operator<<(std::ostream& out, UnaryDerivativeData data) {
  out << data.derivative;
  return out;
}
using UnaryDerivativeTest = testing::TestWithParam<UnaryDerivativeData>;
TEST_P(UnaryDerivativeTest, Emit_ModifierNone) {
  auto params = GetParam();

  auto expr = std::make_unique<ast::IdentifierExpression>("expr");

  ast::UnaryDerivativeExpression derivative(
      params.derivative, ast::DerivativeModifier::kNone, std::move(expr));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitExpression(&derivative)) << g.error();
  EXPECT_EQ(g.result(), std::string(params.name) + "(expr)");
}
TEST_P(UnaryDerivativeTest, Emit_ModifierFine) {
  auto params = GetParam();

  auto expr = std::make_unique<ast::IdentifierExpression>("expr");

  ast::UnaryDerivativeExpression derivative(
      params.derivative, ast::DerivativeModifier::kFine, std::move(expr));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitExpression(&derivative)) << g.error();
  EXPECT_EQ(g.result(), std::string(params.name) + "<fine>(expr)");
}
TEST_P(UnaryDerivativeTest, Emit_ModifierCoarse) {
  auto params = GetParam();

  auto expr = std::make_unique<ast::IdentifierExpression>("expr");

  ast::UnaryDerivativeExpression derivative(
      params.derivative, ast::DerivativeModifier::kCoarse, std::move(expr));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitExpression(&derivative)) << g.error();
  EXPECT_EQ(g.result(), std::string(params.name) + "<coarse>(expr)");
}
INSTANTIATE_TEST_SUITE_P(
    GeneratorImplTest,
    UnaryDerivativeTest,
    testing::Values(UnaryDerivativeData{"dpdx", ast::UnaryDerivative::kDpdx},
                    UnaryDerivativeData{"dpdy", ast::UnaryDerivative::kDpdy},
                    UnaryDerivativeData{"fwidth",
                                        ast::UnaryDerivative::kFwidth}));

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
