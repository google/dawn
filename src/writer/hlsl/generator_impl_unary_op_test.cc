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
#include <vector>

#include "src/ast/identifier_expression.h"
#include "src/ast/module.h"
#include "src/ast/unary_op_expression.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

struct UnaryOpData {
  const char* name;
  ast::UnaryOp op;
};
inline std::ostream& operator<<(std::ostream& out, UnaryOpData data) {
  out << data.op;
  return out;
}
using HlslUnaryOpTest = TestHelperBase<testing::TestWithParam<UnaryOpData>>;
TEST_P(HlslUnaryOpTest, Emit) {
  auto params = GetParam();

  auto expr = std::make_unique<ast::IdentifierExpression>("expr");
  ast::UnaryOpExpression op(params.op, std::move(expr));

  ASSERT_TRUE(gen().EmitExpression(out(), &op)) << gen().error();
  EXPECT_EQ(result(), std::string(params.name) + "(expr)");
}
INSTANTIATE_TEST_SUITE_P(HlslGeneratorImplTest_UnaryOp,
                         HlslUnaryOpTest,
                         testing::Values(UnaryOpData{"!", ast::UnaryOp::kNot},
                                         UnaryOpData{"-",
                                                     ast::UnaryOp::kNegation}));

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
