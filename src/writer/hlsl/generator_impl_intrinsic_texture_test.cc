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
#include "src/ast/builder.h"
#include "src/ast/intrinsic_texture_helper_test.h"
#include "src/ast/type/depth_texture_type.h"
#include "src/ast/type/sampled_texture_type.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/hlsl/generator_impl.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

std::string expected_texture_overload(
    ast::intrinsic::test::ValidTextureOverload overload) {
  using ValidTextureOverload = ast::intrinsic::test::ValidTextureOverload;
  switch (overload) {
    case ValidTextureOverload::kSample1dF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, 1.0f))";
    case ValidTextureOverload::kSample1dArrayF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, float2(1.0f, float(2u))))";
    case ValidTextureOverload::kSample2dF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, float2(1.0f, 2.0f)))";
    case ValidTextureOverload::kSample2dOffsetF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, float2(1.0f, 2.0f), int2(3, 4)))";
    case ValidTextureOverload::kSample2dArrayF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, float3(1.0f, 2.0f, float(3u))))";
    case ValidTextureOverload::kSample2dArrayOffsetF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, float3(1.0f, 2.0f, float(3u)), int2(4, 5)))";
    case ValidTextureOverload::kSample3dF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, float3(1.0f, 2.0f, 3.0f)))";
    case ValidTextureOverload::kSample3dOffsetF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, float3(1.0f, 2.0f, 3.0f), int3(4, 5, 6)))";
    case ValidTextureOverload::kSampleCubeF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, float3(1.0f, 2.0f, 3.0f)))";
    case ValidTextureOverload::kSampleCubeArrayF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, float4(1.0f, 2.0f, 3.0f, float(4u))))";
    case ValidTextureOverload::kSampleDepth2dF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, float2(1.0f, 2.0f)))";
    case ValidTextureOverload::kSampleDepth2dOffsetF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, float2(1.0f, 2.0f), int2(3, 4)))";
    case ValidTextureOverload::kSampleDepth2dArrayF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, float3(1.0f, 2.0f, float(3u))))";
    case ValidTextureOverload::kSampleDepth2dArrayOffsetF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, float3(1.0f, 2.0f, float(3u)), int2(4, 5)))";
    case ValidTextureOverload::kSampleDepthCubeF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, float3(1.0f, 2.0f, 3.0f)))";
    case ValidTextureOverload::kSampleDepthCubeArrayF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, float4(1.0f, 2.0f, 3.0f, float(4u))))";
    case ValidTextureOverload::kSampleBias2dF32:
      return R"(texture_tint_0.SampleBias(sampler_tint_0, float2(1.0f, 2.0f), 3.0f))";
    case ValidTextureOverload::kSampleBias2dOffsetF32:
      return R"(texture_tint_0.SampleBias(sampler_tint_0, float2(1.0f, 2.0f), 3.0f, int2(4, 5)))";
    case ValidTextureOverload::kSampleBias2dArrayF32:
      return R"(texture_tint_0.SampleBias(sampler_tint_0, float3(1.0f, 2.0f, float(4u)), 3.0f))";
    case ValidTextureOverload::kSampleBias2dArrayOffsetF32:
      return R"(texture_tint_0.SampleBias(sampler_tint_0, float3(1.0f, 2.0f, float(3u)), 4.0f, int2(5, 6)))";
    case ValidTextureOverload::kSampleBias3dF32:
      return R"(texture_tint_0.SampleBias(sampler_tint_0, float3(1.0f, 2.0f, 3.0f), 4.0f))";
    case ValidTextureOverload::kSampleBias3dOffsetF32:
      return R"(texture_tint_0.SampleBias(sampler_tint_0, float3(1.0f, 2.0f, 3.0f), 4.0f, int3(5, 6, 7)))";
    case ValidTextureOverload::kSampleBiasCubeF32:
      return R"(texture_tint_0.SampleBias(sampler_tint_0, float3(1.0f, 2.0f, 3.0f), 4.0f))";
    case ValidTextureOverload::kSampleBiasCubeArrayF32:
      return R"(texture_tint_0.SampleBias(sampler_tint_0, float4(1.0f, 2.0f, 3.0f, float(3u)), 4.0f))";
    case ValidTextureOverload::kSampleLevel2dF32:
      return R"(texture_tint_0.SampleLevel(sampler_tint_0, float2(1.0f, 2.0f), 3.0f))";
    case ValidTextureOverload::kSampleLevel2dOffsetF32:
      return R"(texture_tint_0.SampleLevel(sampler_tint_0, float2(1.0f, 2.0f), 3.0f, int2(4, 5)))";
    case ValidTextureOverload::kSampleLevel2dArrayF32:
      return R"(texture_tint_0.SampleLevel(sampler_tint_0, float3(1.0f, 2.0f, float(3u)), 4.0f))";
    case ValidTextureOverload::kSampleLevel2dArrayOffsetF32:
      return R"(texture_tint_0.SampleLevel(sampler_tint_0, float3(1.0f, 2.0f, float(3u)), 4.0f, int2(5, 6)))";
    case ValidTextureOverload::kSampleLevel3dF32:
      return R"(texture_tint_0.SampleLevel(sampler_tint_0, float3(1.0f, 2.0f, 3.0f), 4.0f))";
    case ValidTextureOverload::kSampleLevel3dOffsetF32:
      return R"(texture_tint_0.SampleLevel(sampler_tint_0, float3(1.0f, 2.0f, 3.0f), 4.0f, int3(5, 6, 7)))";
    case ValidTextureOverload::kSampleLevelCubeF32:
      return R"(texture_tint_0.SampleLevel(sampler_tint_0, float3(1.0f, 2.0f, 3.0f), 4.0f))";
    case ValidTextureOverload::kSampleLevelCubeArrayF32:
      return R"(texture_tint_0.SampleLevel(sampler_tint_0, float4(1.0f, 2.0f, 3.0f, float(4u)), 5.0f))";
    case ValidTextureOverload::kSampleLevelDepth2dF32:
      return R"(texture_tint_0.SampleLevel(sampler_tint_0, float2(1.0f, 2.0f), 3u))";
    case ValidTextureOverload::kSampleLevelDepth2dOffsetF32:
      return R"(texture_tint_0.SampleLevel(sampler_tint_0, float2(1.0f, 2.0f), 3u, int2(4, 5)))";
    case ValidTextureOverload::kSampleLevelDepth2dArrayF32:
      return R"(texture_tint_0.SampleLevel(sampler_tint_0, float3(1.0f, 2.0f, float(3u)), 4u))";
    case ValidTextureOverload::kSampleLevelDepth2dArrayOffsetF32:
      return R"(texture_tint_0.SampleLevel(sampler_tint_0, float3(1.0f, 2.0f, float(3u)), 4u, int2(5, 6)))";
    case ValidTextureOverload::kSampleLevelDepthCubeF32:
      return R"(texture_tint_0.SampleLevel(sampler_tint_0, float3(1.0f, 2.0f, 3.0f), 4u))";
    case ValidTextureOverload::kSampleLevelDepthCubeArrayF32:
      return R"(texture_tint_0.SampleLevel(sampler_tint_0, float4(1.0f, 2.0f, 3.0f, float(4u)), 5u))";
    case ValidTextureOverload::kSampleGrad2dF32:
      return R"(texture_tint_0.SampleGrad(sampler_tint_0, float2(1.0f, 2.0f), float2(3.0f, 4.0f), float2(5.0f, 6.0f)))";
    case ValidTextureOverload::kSampleGrad2dOffsetF32:
      return R"(texture_tint_0.SampleGrad(sampler_tint_0, float2(1.0f, 2.0f), float2(3.0f, 4.0f), float2(5.0f, 6.0f), int2(7, 8)))";
    case ValidTextureOverload::kSampleGrad2dArrayF32:
      return R"(texture_tint_0.SampleGrad(sampler_tint_0, float3(1.0f, 2.0f, float(3u)), float2(4.0f, 5.0f), float2(6.0f, 7.0f)))";
    case ValidTextureOverload::kSampleGrad2dArrayOffsetF32:
      return R"(texture_tint_0.SampleGrad(sampler_tint_0, float3(1.0f, 2.0f, float(3u)), float2(4.0f, 5.0f), float2(6.0f, 7.0f), int2(8, 9)))";
    case ValidTextureOverload::kSampleGrad3dF32:
      return R"(texture_tint_0.SampleGrad(sampler_tint_0, float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f)))";
    case ValidTextureOverload::kSampleGrad3dOffsetF32:
      return R"(texture_tint_0.SampleGrad(sampler_tint_0, float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f), int3(10, 11, 12)))";
    case ValidTextureOverload::kSampleGradCubeF32:
      return R"(texture_tint_0.SampleGrad(sampler_tint_0, float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f)))";
    case ValidTextureOverload::kSampleGradCubeArrayF32:
      return R"(texture_tint_0.SampleGrad(sampler_tint_0, float4(1.0f, 2.0f, 3.0f, float(4u)), float3(5.0f, 6.0f, 7.0f), float3(8.0f, 9.0f, 10.0f)))";
    case ValidTextureOverload::kSampleGradDepth2dF32:
      return R"(texture_tint_0.SampleCmp(sampler_tint_0, float2(1.0f, 2.0f), 3.0f))";
    case ValidTextureOverload::kSampleGradDepth2dOffsetF32:
      return R"(texture_tint_0.SampleCmp(sampler_tint_0, float2(1.0f, 2.0f), 3.0f, int2(4, 5)))";
    case ValidTextureOverload::kSampleGradDepth2dArrayF32:
      return R"(texture_tint_0.SampleCmp(sampler_tint_0, float3(1.0f, 2.0f, float(4u)), 3.0f))";
    case ValidTextureOverload::kSampleGradDepth2dArrayOffsetF32:
      return R"(texture_tint_0.SampleCmp(sampler_tint_0, float3(1.0f, 2.0f, float(4u)), 3.0f, int2(5, 6)))";
    case ValidTextureOverload::kSampleGradDepthCubeF32:
      return R"(texture_tint_0.SampleCmp(sampler_tint_0, float3(1.0f, 2.0f, 3.0f), 4.0f))";
    case ValidTextureOverload::kSampleGradDepthCubeArrayF32:
      return R"(texture_tint_0.SampleCmp(sampler_tint_0, float4(1.0f, 2.0f, 3.0f, float(4u)), 5.0f))";
  }
  return "<unmatched texture overload>";
}  // LINT - Ignore the length of this function

class HlslGeneratorIntrinsicTextureTest
    : public ast::BuilderWithContextAndModule,
      public testing::TestWithParam<ast::intrinsic::test::TextureOverloadCase> {
 protected:
  void OnVariableBuilt(ast::Variable* var) override {
    td.RegisterVariableForTesting(var);
  }

  /// @returns the result string
  std::string result() const { return out.str(); }
  /// @returns the pre result string
  std::string pre_result() const { return pre.str(); }

  /// The type determiner
  TypeDeterminer td{ctx, mod};
  /// The generator
  GeneratorImpl gen{ctx, mod};
  /// The output stream
  std::ostringstream out;
  /// The pre-output stream
  std::ostringstream pre;
};

TEST_P(HlslGeneratorIntrinsicTextureTest, Call) {
  auto param = GetParam();

  ast::type::Type* datatype = nullptr;
  switch (param.texture_data_type) {
    case ast::intrinsic::test::TextureDataType::kF32:
      datatype = ty.f32;
      break;
    case ast::intrinsic::test::TextureDataType::kU32:
      datatype = ty.u32;
      break;
    case ast::intrinsic::test::TextureDataType::kI32:
      datatype = ty.i32;
      break;
  }

  ast::type::SamplerType sampler_type{param.sampler_kind};
  switch (param.texture_kind) {
    case ast::intrinsic::test::TextureKind::kRegular:
      Var("texture", ast::StorageClass::kNone,
          ctx->type_mgr().Get<ast::type::SampledTextureType>(
              param.texture_dimension, datatype));
      break;

    case ast::intrinsic::test::TextureKind::kDepth:
      Var("texture", ast::StorageClass::kNone,
          ctx->type_mgr().Get<ast::type::DepthTextureType>(
              param.texture_dimension));
      break;
  }

  Var("sampler", ast::StorageClass::kNone, &sampler_type);

  ast::CallExpression call{Expr(param.function), param.args(this)};

  EXPECT_TRUE(td.DetermineResultType(&call)) << td.error();

  ASSERT_TRUE(gen.EmitExpression(pre, out, &call)) << gen.error();

  EXPECT_TRUE(pre_result().empty());

  auto expected = expected_texture_overload(param.overload);
  EXPECT_EQ(result(), expected);
}

INSTANTIATE_TEST_SUITE_P(
    HlslGeneratorIntrinsicTextureTest,
    HlslGeneratorIntrinsicTextureTest,
    testing::ValuesIn(ast::intrinsic::test::TextureOverloadCase::ValidCases()));

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
