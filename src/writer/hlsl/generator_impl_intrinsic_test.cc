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

#include "src/ast/call_expression.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/module.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_Intrinsic = TestHelper;

struct IntrinsicData {
  const char* name;
  const char* hlsl_name;
};
inline std::ostream& operator<<(std::ostream& out, IntrinsicData data) {
  out << data.name;
  return out;
}
using HlslIntrinsicTest = TestHelperBase<testing::TestWithParam<IntrinsicData>>;
TEST_P(HlslIntrinsicTest, Emit) {
  auto param = GetParam();
  EXPECT_EQ(gen().generate_intrinsic_name(param.name), param.hlsl_name);
}
INSTANTIATE_TEST_SUITE_P(
    HlslGeneratorImplTest_Intrinsic,
    HlslIntrinsicTest,
    testing::Values(IntrinsicData{"any", "any"},
                    IntrinsicData{"all", "all"},
                    IntrinsicData{"dot", "dot"},
                    IntrinsicData{"dpdx", "ddx"},
                    IntrinsicData{"dpdx_coarse", "ddx_coarse"},
                    IntrinsicData{"dpdx_fine", "ddx_fine"},
                    IntrinsicData{"dpdy", "ddy"},
                    IntrinsicData{"dpdy_coarse", "ddy_coarse"},
                    IntrinsicData{"dpdy_fine", "ddy_fine"},
                    IntrinsicData{"fwidth", "fwidth"},
                    IntrinsicData{"fwidth_coarse", "fwidth"},
                    IntrinsicData{"fwidth_fine", "fwidth"},
                    IntrinsicData{"is_finite", "isfinite"},
                    IntrinsicData{"is_inf", "isinf"},
                    IntrinsicData{"is_nan", "isnan"}));

TEST_F(HlslGeneratorImplTest_Intrinsic, DISABLED_Intrinsic_IsNormal) {
  FAIL();
}

TEST_F(HlslGeneratorImplTest_Intrinsic, DISABLED_Intrinsic_Select) {
  FAIL();
}

TEST_F(HlslGeneratorImplTest_Intrinsic, DISABLED_Intrinsic_OuterProduct) {
  ast::type::F32Type f32;
  ast::type::VectorType vec2(&f32, 2);
  ast::type::VectorType vec3(&f32, 3);

  auto a =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &vec2);
  auto b =
      std::make_unique<ast::Variable>("b", ast::StorageClass::kNone, &vec3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("a"));
  params.push_back(std::make_unique<ast::IdentifierExpression>("b"));

  ast::CallExpression call(
      std::make_unique<ast::IdentifierExpression>("outer_product"),
      std::move(params));

  td().RegisterVariableForTesting(a.get());
  td().RegisterVariableForTesting(b.get());

  mod()->AddGlobalVariable(std::move(a));
  mod()->AddGlobalVariable(std::move(b));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(td().DetermineResultType(&call)) << td().error();

  gen().increment_indent();
  ASSERT_TRUE(gen().EmitExpression(out(), &call)) << gen().error();
  EXPECT_EQ(result(), "  float3x2(a * b[0], a * b[1], a * b[2])");
}

TEST_F(HlslGeneratorImplTest_Intrinsic, Intrinsic_Bad_Name) {
  EXPECT_EQ(gen().generate_intrinsic_name("unknown name"), "");
}

TEST_F(HlslGeneratorImplTest_Intrinsic, Intrinsic_Call) {
  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("param1"));
  params.push_back(std::make_unique<ast::IdentifierExpression>("param2"));

  ast::CallExpression call(std::make_unique<ast::IdentifierExpression>("dot"),
                           std::move(params));

  gen().increment_indent();
  ASSERT_TRUE(gen().EmitExpression(out(), &call)) << gen().error();
  EXPECT_EQ(result(), "  dot(param1, param2)");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
