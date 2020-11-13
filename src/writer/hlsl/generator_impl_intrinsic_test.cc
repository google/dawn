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
  ast::Intrinsic intrinsic;
  const char* hlsl_name;
};
inline std::ostream& operator<<(std::ostream& out, IntrinsicData data) {
  out << data.hlsl_name;
  return out;
}
using HlslIntrinsicTest = TestParamHelper<IntrinsicData>;
TEST_P(HlslIntrinsicTest, Emit) {
  auto param = GetParam();
  EXPECT_EQ(gen.generate_intrinsic_name(param.intrinsic), param.hlsl_name);
}
INSTANTIATE_TEST_SUITE_P(
    HlslGeneratorImplTest_Intrinsic,
    HlslIntrinsicTest,
    testing::Values(IntrinsicData{ast::Intrinsic::kAny, "any"},
                    IntrinsicData{ast::Intrinsic::kAll, "all"},
                    IntrinsicData{ast::Intrinsic::kCountOneBits, "countbits"},
                    IntrinsicData{ast::Intrinsic::kDot, "dot"},
                    IntrinsicData{ast::Intrinsic::kDpdx, "ddx"},
                    IntrinsicData{ast::Intrinsic::kDpdxCoarse, "ddx_coarse"},
                    IntrinsicData{ast::Intrinsic::kDpdxFine, "ddx_fine"},
                    IntrinsicData{ast::Intrinsic::kDpdy, "ddy"},
                    IntrinsicData{ast::Intrinsic::kDpdyCoarse, "ddy_coarse"},
                    IntrinsicData{ast::Intrinsic::kDpdyFine, "ddy_fine"},
                    IntrinsicData{ast::Intrinsic::kFwidth, "fwidth"},
                    IntrinsicData{ast::Intrinsic::kFwidthCoarse, "fwidth"},
                    IntrinsicData{ast::Intrinsic::kFwidthFine, "fwidth"},
                    IntrinsicData{ast::Intrinsic::kIsFinite, "isfinite"},
                    IntrinsicData{ast::Intrinsic::kIsInf, "isinf"},
                    IntrinsicData{ast::Intrinsic::kIsNan, "isnan"},
                    IntrinsicData{ast::Intrinsic::kReverseBits,
                                  "reversebits"}));

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

  auto a = create<ast::Variable>("a", ast::StorageClass::kNone, &vec2);
  auto b = create<ast::Variable>("b", ast::StorageClass::kNone, &vec3);

  ast::ExpressionList params;
  params.push_back(create<ast::IdentifierExpression>("a"));
  params.push_back(create<ast::IdentifierExpression>("b"));

  ast::CallExpression call(create<ast::IdentifierExpression>("outer_product"),
                           std::move(params));

  td.RegisterVariableForTesting(a.get());
  td.RegisterVariableForTesting(b.get());

  mod.AddGlobalVariable(std::move(a));
  mod.AddGlobalVariable(std::move(b));

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(&call)) << td.error();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitExpression(pre, out, &call)) << gen.error();
  EXPECT_EQ(result(), "  float3x2(a * b[0], a * b[1], a * b[2])");
}

TEST_F(HlslGeneratorImplTest_Intrinsic, Intrinsic_Bad_Name) {
  EXPECT_EQ(gen.generate_intrinsic_name(ast::Intrinsic::kNone), "");
}

TEST_F(HlslGeneratorImplTest_Intrinsic, Intrinsic_Call) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList params;
  params.push_back(create<ast::IdentifierExpression>("param1"));
  params.push_back(create<ast::IdentifierExpression>("param2"));

  ast::CallExpression call(create<ast::IdentifierExpression>("dot"),
                           std::move(params));

  ast::Variable v1("param1", ast::StorageClass::kFunction, &vec);
  ast::Variable v2("param2", ast::StorageClass::kFunction, &vec);

  td.RegisterVariableForTesting(&v1);
  td.RegisterVariableForTesting(&v2);

  ASSERT_TRUE(td.DetermineResultType(&call)) << td.error();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitExpression(pre, out, &call)) << gen.error();
  EXPECT_EQ(result(), "  dot(param1, param2)");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
