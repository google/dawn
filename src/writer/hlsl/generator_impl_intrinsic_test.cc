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
  auto* a = Var("a", ast::StorageClass::kNone, ty.vec2<f32>());
  auto* b = Var("b", ast::StorageClass::kNone, ty.vec3<f32>());

  auto* call = Call("outerProduct", "a", "b");

  td.RegisterVariableForTesting(a);
  td.RegisterVariableForTesting(b);

  mod->AddGlobalVariable(a);
  mod->AddGlobalVariable(b);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(td.DetermineResultType(call)) << td.error();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitExpression(pre, out, call)) << gen.error();
  EXPECT_EQ(result(), "  float3x2(a * b[0], a * b[1], a * b[2])");
}

TEST_F(HlslGeneratorImplTest_Intrinsic, Intrinsic_Bad_Name) {
  EXPECT_EQ(gen.generate_intrinsic_name(ast::Intrinsic::kNone), "");
}

TEST_F(HlslGeneratorImplTest_Intrinsic, Intrinsic_Call) {
  auto* call = Call("dot", "param1", "param2");

  auto* v1 = Var("param1", ast::StorageClass::kFunction, ty.vec3<f32>());
  auto* v2 = Var("param2", ast::StorageClass::kFunction, ty.vec3<f32>());

  td.RegisterVariableForTesting(v1);
  td.RegisterVariableForTesting(v2);

  ASSERT_TRUE(td.DetermineResultType(call)) << td.error();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitExpression(pre, out, call)) << gen.error();
  EXPECT_EQ(result(), "  dot(param1, param2)");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
