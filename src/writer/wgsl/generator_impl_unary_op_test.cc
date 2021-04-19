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

#include "src/writer/wgsl/test_helper.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

struct UnaryOpData {
  const char* name;
  ast::UnaryOp op;
};
inline std::ostream& operator<<(std::ostream& out, UnaryOpData data) {
  out << data.op;
  return out;
}
using WgslUnaryOpTest = TestParamHelper<UnaryOpData>;
TEST_P(WgslUnaryOpTest, Emit) {
  auto params = GetParam();

  auto* type = (params.op == ast::UnaryOp::kNot)
                   ? static_cast<type::Type*>(ty.bool_())
                   : static_cast<type::Type*>(ty.i32());
  Global("expr", type, ast::StorageClass::kPrivate);

  auto* op = create<ast::UnaryOpExpression>(params.op, Expr("expr"));
  WrapInFunction(op);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitExpression(op)) << gen.error();
  EXPECT_EQ(gen.result(), std::string(params.name) + "(expr)");
}
INSTANTIATE_TEST_SUITE_P(WgslGeneratorImplTest,
                         WgslUnaryOpTest,
                         testing::Values(UnaryOpData{"!", ast::UnaryOp::kNot},
                                         UnaryOpData{"-",
                                                     ast::UnaryOp::kNegation}));

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
