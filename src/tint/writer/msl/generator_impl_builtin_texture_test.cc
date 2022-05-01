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

#include "src/tint/ast/builtin_texture_helper_test.h"
#include "src/tint/ast/call_statement.h"
#include "src/tint/writer/msl/test_helper.h"

namespace tint::writer::msl {
namespace {

std::string expected_texture_overload(ast::builtin::test::ValidTextureOverload overload) {
    using ValidTextureOverload = ast::builtin::test::ValidTextureOverload;
    switch (overload) {
        case ValidTextureOverload::kDimensions1d:
        case ValidTextureOverload::kDimensionsStorageWO1d:
            return R"(int(texture.get_width(0)))";
        case ValidTextureOverload::kDimensions2d:
        case ValidTextureOverload::kDimensions2dArray:
        case ValidTextureOverload::kDimensionsCube:
        case ValidTextureOverload::kDimensionsCubeArray:
        case ValidTextureOverload::kDimensionsMultisampled2d:
        case ValidTextureOverload::kDimensionsDepth2d:
        case ValidTextureOverload::kDimensionsDepth2dArray:
        case ValidTextureOverload::kDimensionsDepthCube:
        case ValidTextureOverload::kDimensionsDepthCubeArray:
        case ValidTextureOverload::kDimensionsDepthMultisampled2d:
        case ValidTextureOverload::kDimensionsStorageWO2d:
        case ValidTextureOverload::kDimensionsStorageWO2dArray:
            return R"(int2(texture.get_width(), texture.get_height()))";
        case ValidTextureOverload::kDimensions3d:
        case ValidTextureOverload::kDimensionsStorageWO3d:
            return R"(int3(texture.get_width(), texture.get_height(), texture.get_depth()))";
        case ValidTextureOverload::kDimensions2dLevel:
        case ValidTextureOverload::kDimensionsCubeLevel:
        case ValidTextureOverload::kDimensionsCubeArrayLevel:
        case ValidTextureOverload::kDimensions2dArrayLevel:
        case ValidTextureOverload::kDimensionsDepth2dLevel:
        case ValidTextureOverload::kDimensionsDepth2dArrayLevel:
        case ValidTextureOverload::kDimensionsDepthCubeLevel:
        case ValidTextureOverload::kDimensionsDepthCubeArrayLevel:
            return R"(int2(texture.get_width(1), texture.get_height(1)))";
        case ValidTextureOverload::kDimensions3dLevel:
            return R"(int3(texture.get_width(1), texture.get_height(1), texture.get_depth(1)))";
        case ValidTextureOverload::kGather2dF32:
            return R"(texture.gather(sampler, float2(1.0f, 2.0f), int2(0), component::x))";
        case ValidTextureOverload::kGather2dOffsetF32:
            return R"(texture.gather(sampler, float2(1.0f, 2.0f), int2(3, 4), component::x))";
        case ValidTextureOverload::kGather2dArrayF32:
            return R"(texture.gather(sampler, float2(1.0f, 2.0f), 3, int2(0), component::x))";
        case ValidTextureOverload::kGather2dArrayOffsetF32:
            return R"(texture.gather(sampler, float2(1.0f, 2.0f), 3, int2(4, 5), component::x))";
        case ValidTextureOverload::kGatherCubeF32:
            return R"(texture.gather(sampler, float3(1.0f, 2.0f, 3.0f), component::x))";
        case ValidTextureOverload::kGatherCubeArrayF32:
            return R"(texture.gather(sampler, float3(1.0f, 2.0f, 3.0f), 4, component::x))";
        case ValidTextureOverload::kGatherDepth2dF32:
            return R"(texture.gather(sampler, float2(1.0f, 2.0f)))";
        case ValidTextureOverload::kGatherDepth2dOffsetF32:
            return R"(texture.gather(sampler, float2(1.0f, 2.0f), int2(3, 4)))";
        case ValidTextureOverload::kGatherDepth2dArrayF32:
            return R"(texture.gather(sampler, float2(1.0f, 2.0f), 3))";
        case ValidTextureOverload::kGatherDepth2dArrayOffsetF32:
            return R"(texture.gather(sampler, float2(1.0f, 2.0f), 3, int2(4, 5)))";
        case ValidTextureOverload::kGatherDepthCubeF32:
            return R"(texture.gather(sampler, float3(1.0f, 2.0f, 3.0f)))";
        case ValidTextureOverload::kGatherDepthCubeArrayF32:
            return R"(texture.gather(sampler, float3(1.0f, 2.0f, 3.0f), 4))";
        case ValidTextureOverload::kGatherCompareDepth2dF32:
            return R"(texture.gather_compare(sampler, float2(1.0f, 2.0f), 3.0f))";
        case ValidTextureOverload::kGatherCompareDepth2dOffsetF32:
            return R"(texture.gather_compare(sampler, float2(1.0f, 2.0f), 3.0f, int2(4, 5)))";
        case ValidTextureOverload::kGatherCompareDepth2dArrayF32:
            return R"(texture.gather_compare(sampler, float2(1.0f, 2.0f), 3, 4.0f))";
        case ValidTextureOverload::kGatherCompareDepth2dArrayOffsetF32:
            return R"(texture.gather_compare(sampler, float2(1.0f, 2.0f), 3, 4.0f, int2(5, 6)))";
        case ValidTextureOverload::kGatherCompareDepthCubeF32:
            return R"(texture.gather_compare(sampler, float3(1.0f, 2.0f, 3.0f), 4.0f))";
        case ValidTextureOverload::kGatherCompareDepthCubeArrayF32:
            return R"(texture.gather_compare(sampler, float3(1.0f, 2.0f, 3.0f), 4, 5.0f))";
        case ValidTextureOverload::kNumLayers2dArray:
        case ValidTextureOverload::kNumLayersCubeArray:
        case ValidTextureOverload::kNumLayersDepth2dArray:
        case ValidTextureOverload::kNumLayersDepthCubeArray:
        case ValidTextureOverload::kNumLayersStorageWO2dArray:
            return R"(int(texture.get_array_size()))";
        case ValidTextureOverload::kNumLevels2d:
        case ValidTextureOverload::kNumLevels2dArray:
        case ValidTextureOverload::kNumLevels3d:
        case ValidTextureOverload::kNumLevelsCube:
        case ValidTextureOverload::kNumLevelsCubeArray:
        case ValidTextureOverload::kNumLevelsDepth2d:
        case ValidTextureOverload::kNumLevelsDepth2dArray:
        case ValidTextureOverload::kNumLevelsDepthCube:
        case ValidTextureOverload::kNumLevelsDepthCubeArray:
            return R"(int(texture.get_num_mip_levels()))";
        case ValidTextureOverload::kNumSamplesDepthMultisampled2d:
        case ValidTextureOverload::kNumSamplesMultisampled2d:
            return R"(int(texture.get_num_samples()))";
        case ValidTextureOverload::kSample1dF32:
            return R"(texture.sample(sampler, 1.0f))";
        case ValidTextureOverload::kSample2dF32:
            return R"(texture.sample(sampler, float2(1.0f, 2.0f)))";
        case ValidTextureOverload::kSample2dOffsetF32:
            return R"(texture.sample(sampler, float2(1.0f, 2.0f), int2(3, 4)))";
        case ValidTextureOverload::kSample2dArrayF32:
            return R"(texture.sample(sampler, float2(1.0f, 2.0f), 3))";
        case ValidTextureOverload::kSample2dArrayOffsetF32:
            return R"(texture.sample(sampler, float2(1.0f, 2.0f), 3, int2(4, 5)))";
        case ValidTextureOverload::kSample3dF32:
            return R"(texture.sample(sampler, float3(1.0f, 2.0f, 3.0f)))";
        case ValidTextureOverload::kSample3dOffsetF32:
            return R"(texture.sample(sampler, float3(1.0f, 2.0f, 3.0f), int3(4, 5, 6)))";
        case ValidTextureOverload::kSampleCubeF32:
            return R"(texture.sample(sampler, float3(1.0f, 2.0f, 3.0f)))";
        case ValidTextureOverload::kSampleCubeArrayF32:
            return R"(texture.sample(sampler, float3(1.0f, 2.0f, 3.0f), 4))";
        case ValidTextureOverload::kSampleDepth2dF32:
            return R"(texture.sample(sampler, float2(1.0f, 2.0f)))";
        case ValidTextureOverload::kSampleDepth2dOffsetF32:
            return R"(texture.sample(sampler, float2(1.0f, 2.0f), int2(3, 4)))";
        case ValidTextureOverload::kSampleDepth2dArrayF32:
            return R"(texture.sample(sampler, float2(1.0f, 2.0f), 3))";
        case ValidTextureOverload::kSampleDepth2dArrayOffsetF32:
            return R"(texture.sample(sampler, float2(1.0f, 2.0f), 3, int2(4, 5)))";
        case ValidTextureOverload::kSampleDepthCubeF32:
            return R"(texture.sample(sampler, float3(1.0f, 2.0f, 3.0f)))";
        case ValidTextureOverload::kSampleDepthCubeArrayF32:
            return R"(texture.sample(sampler, float3(1.0f, 2.0f, 3.0f), 4))";
        case ValidTextureOverload::kSampleBias2dF32:
            return R"(texture.sample(sampler, float2(1.0f, 2.0f), bias(3.0f)))";
        case ValidTextureOverload::kSampleBias2dOffsetF32:
            return R"(texture.sample(sampler, float2(1.0f, 2.0f), bias(3.0f), int2(4, 5)))";
        case ValidTextureOverload::kSampleBias2dArrayF32:
            return R"(texture.sample(sampler, float2(1.0f, 2.0f), 4, bias(3.0f)))";
        case ValidTextureOverload::kSampleBias2dArrayOffsetF32:
            return R"(texture.sample(sampler, float2(1.0f, 2.0f), 3, bias(4.0f), int2(5, 6)))";
        case ValidTextureOverload::kSampleBias3dF32:
            return R"(texture.sample(sampler, float3(1.0f, 2.0f, 3.0f), bias(4.0f)))";
        case ValidTextureOverload::kSampleBias3dOffsetF32:
            return R"(texture.sample(sampler, float3(1.0f, 2.0f, 3.0f), bias(4.0f), int3(5, 6, 7)))";
        case ValidTextureOverload::kSampleBiasCubeF32:
            return R"(texture.sample(sampler, float3(1.0f, 2.0f, 3.0f), bias(4.0f)))";
        case ValidTextureOverload::kSampleBiasCubeArrayF32:
            return R"(texture.sample(sampler, float3(1.0f, 2.0f, 3.0f), 3, bias(4.0f)))";
        case ValidTextureOverload::kSampleLevel2dF32:
            return R"(texture.sample(sampler, float2(1.0f, 2.0f), level(3.0f)))";
        case ValidTextureOverload::kSampleLevel2dOffsetF32:
            return R"(texture.sample(sampler, float2(1.0f, 2.0f), level(3.0f), int2(4, 5)))";
        case ValidTextureOverload::kSampleLevel2dArrayF32:
            return R"(texture.sample(sampler, float2(1.0f, 2.0f), 3, level(4.0f)))";
        case ValidTextureOverload::kSampleLevel2dArrayOffsetF32:
            return R"(texture.sample(sampler, float2(1.0f, 2.0f), 3, level(4.0f), int2(5, 6)))";
        case ValidTextureOverload::kSampleLevel3dF32:
            return R"(texture.sample(sampler, float3(1.0f, 2.0f, 3.0f), level(4.0f)))";
        case ValidTextureOverload::kSampleLevel3dOffsetF32:
            return R"(texture.sample(sampler, float3(1.0f, 2.0f, 3.0f), level(4.0f), int3(5, 6, 7)))";
        case ValidTextureOverload::kSampleLevelCubeF32:
            return R"(texture.sample(sampler, float3(1.0f, 2.0f, 3.0f), level(4.0f)))";
        case ValidTextureOverload::kSampleLevelCubeArrayF32:
            return R"(texture.sample(sampler, float3(1.0f, 2.0f, 3.0f), 4, level(5.0f)))";
        case ValidTextureOverload::kSampleLevelDepth2dF32:
            return R"(texture.sample(sampler, float2(1.0f, 2.0f), level(3)))";
        case ValidTextureOverload::kSampleLevelDepth2dOffsetF32:
            return R"(texture.sample(sampler, float2(1.0f, 2.0f), level(3), int2(4, 5)))";
        case ValidTextureOverload::kSampleLevelDepth2dArrayF32:
            return R"(texture.sample(sampler, float2(1.0f, 2.0f), 3, level(4)))";
        case ValidTextureOverload::kSampleLevelDepth2dArrayOffsetF32:
            return R"(texture.sample(sampler, float2(1.0f, 2.0f), 3, level(4), int2(5, 6)))";
        case ValidTextureOverload::kSampleLevelDepthCubeF32:
            return R"(texture.sample(sampler, float3(1.0f, 2.0f, 3.0f), level(4)))";
        case ValidTextureOverload::kSampleLevelDepthCubeArrayF32:
            return R"(texture.sample(sampler, float3(1.0f, 2.0f, 3.0f), 4, level(5)))";
        case ValidTextureOverload::kSampleGrad2dF32:
            return R"(texture.sample(sampler, float2(1.0f, 2.0f), gradient2d(float2(3.0f, 4.0f), float2(5.0f, 6.0f))))";
        case ValidTextureOverload::kSampleGrad2dOffsetF32:
            return R"(texture.sample(sampler, float2(1.0f, 2.0f), gradient2d(float2(3.0f, 4.0f), float2(5.0f, 6.0f)), int2(7, 7)))";
        case ValidTextureOverload::kSampleGrad2dArrayF32:
            return R"(texture.sample(sampler, float2(1.0f, 2.0f), 3, gradient2d(float2(4.0f, 5.0f), float2(6.0f, 7.0f))))";
        case ValidTextureOverload::kSampleGrad2dArrayOffsetF32:
            return R"(texture.sample(sampler, float2(1.0f, 2.0f), 3, gradient2d(float2(4.0f, 5.0f), float2(6.0f, 7.0f)), int2(6, 7)))";
        case ValidTextureOverload::kSampleGrad3dF32:
            return R"(texture.sample(sampler, float3(1.0f, 2.0f, 3.0f), gradient3d(float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f))))";
        case ValidTextureOverload::kSampleGrad3dOffsetF32:
            return R"(texture.sample(sampler, float3(1.0f, 2.0f, 3.0f), gradient3d(float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f)), int3(0, 1, 2)))";
        case ValidTextureOverload::kSampleGradCubeF32:
            return R"(texture.sample(sampler, float3(1.0f, 2.0f, 3.0f), gradientcube(float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f))))";
        case ValidTextureOverload::kSampleGradCubeArrayF32:
            return R"(texture.sample(sampler, float3(1.0f, 2.0f, 3.0f), 4, gradientcube(float3(5.0f, 6.0f, 7.0f), float3(8.0f, 9.0f, 10.0f))))";
        case ValidTextureOverload::kSampleCompareDepth2dF32:
            return R"(texture.sample_compare(sampler, float2(1.0f, 2.0f), 3.0f))";
        case ValidTextureOverload::kSampleCompareDepth2dOffsetF32:
            return R"(texture.sample_compare(sampler, float2(1.0f, 2.0f), 3.0f, int2(4, 5)))";
        case ValidTextureOverload::kSampleCompareDepth2dArrayF32:
            return R"(texture.sample_compare(sampler, float2(1.0f, 2.0f), 4, 3.0f))";
        case ValidTextureOverload::kSampleCompareDepth2dArrayOffsetF32:
            return R"(texture.sample_compare(sampler, float2(1.0f, 2.0f), 4, 3.0f, int2(5, 6)))";
        case ValidTextureOverload::kSampleCompareDepthCubeF32:
            return R"(texture.sample_compare(sampler, float3(1.0f, 2.0f, 3.0f), 4.0f))";
        case ValidTextureOverload::kSampleCompareDepthCubeArrayF32:
            return R"(texture.sample_compare(sampler, float3(1.0f, 2.0f, 3.0f), 4, 5.0f))";
        case ValidTextureOverload::kSampleCompareLevelDepth2dF32:
            return R"(texture.sample_compare(sampler, float2(1.0f, 2.0f), 3.0f))";
        case ValidTextureOverload::kSampleCompareLevelDepth2dOffsetF32:
            return R"(texture.sample_compare(sampler, float2(1.0f, 2.0f), 3.0f, int2(4, 5)))";
        case ValidTextureOverload::kSampleCompareLevelDepth2dArrayF32:
            return R"(texture.sample_compare(sampler, float2(1.0f, 2.0f), 4, 3.0f))";
        case ValidTextureOverload::kSampleCompareLevelDepth2dArrayOffsetF32:
            return R"(texture.sample_compare(sampler, float2(1.0f, 2.0f), 4, 3.0f, int2(5, 6)))";
        case ValidTextureOverload::kSampleCompareLevelDepthCubeF32:
            return R"(texture.sample_compare(sampler, float3(1.0f, 2.0f, 3.0f), 4.0f))";
        case ValidTextureOverload::kSampleCompareLevelDepthCubeArrayF32:
            return R"(texture.sample_compare(sampler, float3(1.0f, 2.0f, 3.0f), 4, 5.0f))";
        case ValidTextureOverload::kLoad1dLevelF32:
            return R"(texture.read(uint(1), 0))";
        case ValidTextureOverload::kLoad1dLevelU32:
            return R"(texture.read(uint(1), 0))";
        case ValidTextureOverload::kLoad1dLevelI32:
            return R"(texture.read(uint(1), 0))";
        case ValidTextureOverload::kLoad2dLevelF32:
            return R"(texture.read(uint2(int2(1, 2)), 3))";
        case ValidTextureOverload::kLoad2dLevelU32:
            return R"(texture.read(uint2(int2(1, 2)), 3))";
        case ValidTextureOverload::kLoad2dLevelI32:
            return R"(texture.read(uint2(int2(1, 2)), 3))";
        case ValidTextureOverload::kLoad2dArrayLevelF32:
            return R"(texture.read(uint2(int2(1, 2)), 3, 4))";
        case ValidTextureOverload::kLoad2dArrayLevelU32:
            return R"(texture.read(uint2(int2(1, 2)), 3, 4))";
        case ValidTextureOverload::kLoad2dArrayLevelI32:
            return R"(texture.read(uint2(int2(1, 2)), 3, 4))";
        case ValidTextureOverload::kLoad3dLevelF32:
            return R"(texture.read(uint3(int3(1, 2, 3)), 4))";
        case ValidTextureOverload::kLoad3dLevelU32:
            return R"(texture.read(uint3(int3(1, 2, 3)), 4))";
        case ValidTextureOverload::kLoad3dLevelI32:
            return R"(texture.read(uint3(int3(1, 2, 3)), 4))";
        case ValidTextureOverload::kLoadMultisampled2dF32:
            return R"(texture.read(uint2(int2(1, 2)), 3))";
        case ValidTextureOverload::kLoadMultisampled2dU32:
            return R"(texture.read(uint2(int2(1, 2)), 3))";
        case ValidTextureOverload::kLoadMultisampled2dI32:
            return R"(texture.read(uint2(int2(1, 2)), 3))";
        case ValidTextureOverload::kLoadDepth2dLevelF32:
        case ValidTextureOverload::kLoadDepthMultisampled2dF32:
            return R"(texture.read(uint2(int2(1, 2)), 3))";
        case ValidTextureOverload::kLoadDepth2dArrayLevelF32:
            return R"(texture.read(uint2(int2(1, 2)), 3, 4))";
        case ValidTextureOverload::kStoreWO1dRgba32float:
            return R"(texture.write(float4(2.0f, 3.0f, 4.0f, 5.0f), uint(1)))";
        case ValidTextureOverload::kStoreWO2dRgba32float:
            return R"(texture.write(float4(3.0f, 4.0f, 5.0f, 6.0f), uint2(int2(1, 2))))";
        case ValidTextureOverload::kStoreWO2dArrayRgba32float:
            return R"(texture.write(float4(4.0f, 5.0f, 6.0f, 7.0f), uint2(int2(1, 2)), 3))";
        case ValidTextureOverload::kStoreWO3dRgba32float:
            return R"(texture.write(float4(4.0f, 5.0f, 6.0f, 7.0f), uint3(int3(1, 2, 3))))";
    }
    return "<unmatched texture overload>";
}  // NOLINT - Ignore the length of this function

class MslGeneratorBuiltinTextureTest
    : public TestParamHelper<ast::builtin::test::TextureOverloadCase> {};

TEST_P(MslGeneratorBuiltinTextureTest, Call) {
    auto param = GetParam();

    param.BuildTextureVariable(this);
    param.BuildSamplerVariable(this);

    auto* call = Call(Expr(param.function), param.args(this));
    auto* stmt = CallStmt(call);

    Func("main", {}, ty.void_(), {stmt}, {Stage(ast::PipelineStage::kFragment)});

    GeneratorImpl& gen = Build();

    std::stringstream out;
    ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.error();

    auto expected = expected_texture_overload(param.overload);
    EXPECT_EQ(expected, out.str());
}

INSTANTIATE_TEST_SUITE_P(MslGeneratorBuiltinTextureTest,
                         MslGeneratorBuiltinTextureTest,
                         testing::ValuesIn(ast::builtin::test::TextureOverloadCase::ValidCases()));

}  // namespace
}  // namespace tint::writer::msl
