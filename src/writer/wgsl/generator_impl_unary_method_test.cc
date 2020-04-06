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
#include "src/ast/unary_method_expression.h"
#include "src/writer/wgsl/generator_impl.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

struct UnaryMethodData {
  const char* name;
  ast::UnaryMethod method;
};
inline std::ostream& operator<<(std::ostream& out, UnaryMethodData data) {
  out << data.method;
  return out;
}
using UnaryMethodTest = testing::TestWithParam<UnaryMethodData>;
TEST_P(UnaryMethodTest, Emit) {
  auto params = GetParam();

  auto expr = std::make_unique<ast::IdentifierExpression>("expr");
  ast::ExpressionList ops;
  ops.push_back(std::move(expr));

  ast::UnaryMethodExpression method(params.method, std::move(ops));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitExpression(&method)) << g.error();
  EXPECT_EQ(g.result(), std::string(params.name) + "(expr)");
}
INSTANTIATE_TEST_SUITE_P(
    GeneratorImplTest,
    UnaryMethodTest,
    testing::Values(UnaryMethodData{"any", ast::UnaryMethod::kAny},
                    UnaryMethodData{"all", ast::UnaryMethod::kAll},
                    UnaryMethodData{"is_nan", ast::UnaryMethod::kIsNan},
                    UnaryMethodData{"is_finite", ast::UnaryMethod::kIsFinite},
                    UnaryMethodData{"is_normal", ast::UnaryMethod::kIsNormal}));

using UnaryMethodTest_MultiParam = testing::TestWithParam<UnaryMethodData>;
TEST_P(UnaryMethodTest_MultiParam, Emit) {
  auto params = GetParam();

  auto expr1 = std::make_unique<ast::IdentifierExpression>("expr1");
  auto expr2 = std::make_unique<ast::IdentifierExpression>("expr2");
  ast::ExpressionList ops;
  ops.push_back(std::move(expr1));
  ops.push_back(std::move(expr2));

  ast::UnaryMethodExpression method(params.method, std::move(ops));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitExpression(&method)) << g.error();
  EXPECT_EQ(g.result(), std::string(params.name) + "(expr1, expr2)");
}
INSTANTIATE_TEST_SUITE_P(
    GeneratorImplTest,
    UnaryMethodTest_MultiParam,
    testing::Values(UnaryMethodData{"dot", ast::UnaryMethod::kDot},
                    UnaryMethodData{"outer_product",
                                    ast::UnaryMethod::kOuterProduct}));

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
