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
#include "src/ast/relational_expression.h"
#include "src/writer/wgsl/generator_impl.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

struct RelationData {
  const char* result;
  ast::Relation relation;
};
inline std::ostream& operator<<(std::ostream& out, RelationData data) {
  out << data.relation;
  return out;
}
using RelationTest = testing::TestWithParam<RelationData>;
TEST_P(RelationTest, Emit) {
  auto params = GetParam();

  auto left = std::make_unique<ast::IdentifierExpression>("left");
  auto right = std::make_unique<ast::IdentifierExpression>("right");

  ast::RelationalExpression expr(params.relation, std::move(left),
                                 std::move(right));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitExpression(&expr)) << g.error();
  EXPECT_EQ(g.result(), params.result);
}
INSTANTIATE_TEST_SUITE_P(
    GeneratorImplTest,
    RelationTest,
    testing::Values(
        RelationData{"(left & right)", ast::Relation::kAnd},
        RelationData{"(left | right)", ast::Relation::kOr},
        RelationData{"(left ^ right)", ast::Relation::kXor},
        RelationData{"(left && right)", ast::Relation::kLogicalAnd},
        RelationData{"(left || right)", ast::Relation::kLogicalOr},
        RelationData{"(left == right)", ast::Relation::kEqual},
        RelationData{"(left != right)", ast::Relation::kNotEqual},
        RelationData{"(left < right)", ast::Relation::kLessThan},
        RelationData{"(left > right)", ast::Relation::kGreaterThan},
        RelationData{"(left <= right)", ast::Relation::kLessThanEqual},
        RelationData{"(left >= right)", ast::Relation::kGreaterThanEqual},
        RelationData{"(left << right)", ast::Relation::kShiftLeft},
        RelationData{"(left >> right)", ast::Relation::kShiftRight},
        RelationData{"(left >>> right)", ast::Relation::kShiftRightArith},
        RelationData{"(left + right)", ast::Relation::kAdd},
        RelationData{"(left - right)", ast::Relation::kSubtract},
        RelationData{"(left * right)", ast::Relation::kMultiply},
        RelationData{"(left / right)", ast::Relation::kDivide},
        RelationData{"(left % right)", ast::Relation::kModulo}));

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
