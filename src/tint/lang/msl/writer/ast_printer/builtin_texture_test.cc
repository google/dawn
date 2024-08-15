// Copyright 2020 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/msl/writer/ast_printer/helper_test.h"
#include "src/tint/lang/wgsl/ast/builtin_texture_helper_test.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::msl::writer {
namespace {

std::string expected_texture_overload(ast::test::ValidTextureOverload overload) {
    using ValidTextureOverload = ast::test::ValidTextureOverload;
    switch (overload) {
        case ValidTextureOverload::kDimensions1d:
        case ValidTextureOverload::kDimensionsStorageWO1d:
            return R"(Texture.get_width(0))";
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
            return R"(uint2(Texture.get_width(), Texture.get_height()))";
        case ValidTextureOverload::kDimensions3d:
        case ValidTextureOverload::kDimensionsStorageWO3d:
            return R"(uint3(Texture.get_width(), Texture.get_height(), Texture.get_depth()))";
        case ValidTextureOverload::kDimensions2dLevel:
        case ValidTextureOverload::kDimensionsCubeLevel:
        case ValidTextureOverload::kDimensionsCubeArrayLevel:
        case ValidTextureOverload::kDimensions2dArrayLevel:
        case ValidTextureOverload::kDimensionsDepth2dLevel:
        case ValidTextureOverload::kDimensionsDepth2dArrayLevel:
        case ValidTextureOverload::kDimensionsDepthCubeLevel:
        case ValidTextureOverload::kDimensionsDepthCubeArrayLevel:
            return R"(uint2(Texture.get_width(1), Texture.get_height(1)))";
        case ValidTextureOverload::kDimensions3dLevel:
            return R"(uint3(Texture.get_width(1), Texture.get_height(1), Texture.get_depth(1)))";
        case ValidTextureOverload::kGather2dF32:
            return R"(Texture.gather(Sampler, float2(1.0f, 2.0f), int2(0), component::x))";
        case ValidTextureOverload::kGather2dOffsetF32:
            return R"(Texture.gather(Sampler, float2(1.0f, 2.0f), int2(3, 4), component::x))";
        case ValidTextureOverload::kGather2dArrayF32:
            return R"(Texture.gather(Sampler, float2(1.0f, 2.0f), max(0, 3), int2(0), component::x))";
        case ValidTextureOverload::kGather2dArrayOffsetF32:
            return R"(Texture.gather(Sampler, float2(1.0f, 2.0f), 3u, int2(4, 5), component::x))";
        case ValidTextureOverload::kGatherCubeF32:
            return R"(Texture.gather(Sampler, float3(1.0f, 2.0f, 3.0f), component::x))";
        case ValidTextureOverload::kGatherCubeArrayF32:
            return R"(Texture.gather(Sampler, float3(1.0f, 2.0f, 3.0f), 4u, component::x))";
        case ValidTextureOverload::kGatherDepth2dF32:
            return R"(Texture.gather(Sampler, float2(1.0f, 2.0f)))";
        case ValidTextureOverload::kGatherDepth2dOffsetF32:
            return R"(Texture.gather(Sampler, float2(1.0f, 2.0f), int2(3, 4)))";
        case ValidTextureOverload::kGatherDepth2dArrayF32:
            return R"(Texture.gather(Sampler, float2(1.0f, 2.0f), 3u))";
        case ValidTextureOverload::kGatherDepth2dArrayOffsetF32:
            return R"(Texture.gather(Sampler, float2(1.0f, 2.0f), max(0, 3), int2(4, 5)))";
        case ValidTextureOverload::kGatherDepthCubeF32:
            return R"(Texture.gather(Sampler, float3(1.0f, 2.0f, 3.0f)))";
        case ValidTextureOverload::kGatherDepthCubeArrayF32:
            return R"(Texture.gather(Sampler, float3(1.0f, 2.0f, 3.0f), 4u))";
        case ValidTextureOverload::kGatherCompareDepth2dF32:
            return R"(Texture.gather_compare(Sampler, float2(1.0f, 2.0f), 3.0f))";
        case ValidTextureOverload::kGatherCompareDepth2dOffsetF32:
            return R"(Texture.gather_compare(Sampler, float2(1.0f, 2.0f), 3.0f, int2(4, 5)))";
        case ValidTextureOverload::kGatherCompareDepth2dArrayF32:
            return R"(Texture.gather_compare(Sampler, float2(1.0f, 2.0f), max(0, 3), 4.0f))";
        case ValidTextureOverload::kGatherCompareDepth2dArrayOffsetF32:
            return R"(Texture.gather_compare(Sampler, float2(1.0f, 2.0f), max(0, 3), 4.0f, int2(5, 6)))";
        case ValidTextureOverload::kGatherCompareDepthCubeF32:
            return R"(Texture.gather_compare(Sampler, float3(1.0f, 2.0f, 3.0f), 4.0f))";
        case ValidTextureOverload::kGatherCompareDepthCubeArrayF32:
            return R"(Texture.gather_compare(Sampler, float3(1.0f, 2.0f, 3.0f), 4u, 5.0f))";
        case ValidTextureOverload::kNumLayers2dArray:
        case ValidTextureOverload::kNumLayersCubeArray:
        case ValidTextureOverload::kNumLayersDepth2dArray:
        case ValidTextureOverload::kNumLayersDepthCubeArray:
        case ValidTextureOverload::kNumLayersStorageWO2dArray:
            return R"(Texture.get_array_size())";
        case ValidTextureOverload::kNumLevels2d:
        case ValidTextureOverload::kNumLevels2dArray:
        case ValidTextureOverload::kNumLevels3d:
        case ValidTextureOverload::kNumLevelsCube:
        case ValidTextureOverload::kNumLevelsCubeArray:
        case ValidTextureOverload::kNumLevelsDepth2d:
        case ValidTextureOverload::kNumLevelsDepth2dArray:
        case ValidTextureOverload::kNumLevelsDepthCube:
        case ValidTextureOverload::kNumLevelsDepthCubeArray:
            return R"(Texture.get_num_mip_levels())";
        case ValidTextureOverload::kNumSamplesDepthMultisampled2d:
        case ValidTextureOverload::kNumSamplesMultisampled2d:
            return R"(Texture.get_num_samples())";
        case ValidTextureOverload::kSample1dF32:
            return R"(Texture.sample(Sampler, 1.0f))";
        case ValidTextureOverload::kSample2dF32:
            return R"(Texture.sample(Sampler, float2(1.0f, 2.0f)))";
        case ValidTextureOverload::kSample2dOffsetF32:
            return R"(Texture.sample(Sampler, float2(1.0f, 2.0f), int2(3, 4)))";
        case ValidTextureOverload::kSample2dArrayF32:
            return R"(Texture.sample(Sampler, float2(1.0f, 2.0f), max(0, 3)))";
        case ValidTextureOverload::kSample2dArrayOffsetF32:
            return R"(Texture.sample(Sampler, float2(1.0f, 2.0f), 3u, int2(4, 5)))";
        case ValidTextureOverload::kSample3dF32:
            return R"(Texture.sample(Sampler, float3(1.0f, 2.0f, 3.0f)))";
        case ValidTextureOverload::kSample3dOffsetF32:
            return R"(Texture.sample(Sampler, float3(1.0f, 2.0f, 3.0f), int3(4, 5, 6)))";
        case ValidTextureOverload::kSampleCubeF32:
            return R"(Texture.sample(Sampler, float3(1.0f, 2.0f, 3.0f)))";
        case ValidTextureOverload::kSampleCubeArrayF32:
            return R"(Texture.sample(Sampler, float3(1.0f, 2.0f, 3.0f), max(0, 4)))";
        case ValidTextureOverload::kSampleDepth2dF32:
            return R"(Texture.sample(Sampler, float2(1.0f, 2.0f)))";
        case ValidTextureOverload::kSampleDepth2dOffsetF32:
            return R"(Texture.sample(Sampler, float2(1.0f, 2.0f), int2(3, 4)))";
        case ValidTextureOverload::kSampleDepth2dArrayF32:
            return R"(Texture.sample(Sampler, float2(1.0f, 2.0f), max(0, 3)))";
        case ValidTextureOverload::kSampleDepth2dArrayOffsetF32:
            return R"(Texture.sample(Sampler, float2(1.0f, 2.0f), max(0, 3), int2(4, 5)))";
        case ValidTextureOverload::kSampleDepthCubeF32:
            return R"(Texture.sample(Sampler, float3(1.0f, 2.0f, 3.0f)))";
        case ValidTextureOverload::kSampleDepthCubeArrayF32:
            return R"(Texture.sample(Sampler, float3(1.0f, 2.0f, 3.0f), 4u))";
        case ValidTextureOverload::kSampleBias2dF32:
            return R"(Texture.sample(Sampler, float2(1.0f, 2.0f), bias(3.0f)))";
        case ValidTextureOverload::kSampleBias2dOffsetF32:
            return R"(Texture.sample(Sampler, float2(1.0f, 2.0f), bias(3.0f), int2(4, 5)))";
        case ValidTextureOverload::kSampleBias2dArrayF32:
            return R"(Texture.sample(Sampler, float2(1.0f, 2.0f), 4u, bias(3.0f)))";
        case ValidTextureOverload::kSampleBias2dArrayOffsetF32:
            return R"(Texture.sample(Sampler, float2(1.0f, 2.0f), max(0, 3), bias(4.0f), int2(5, 6)))";
        case ValidTextureOverload::kSampleBias3dF32:
            return R"(Texture.sample(Sampler, float3(1.0f, 2.0f, 3.0f), bias(4.0f)))";
        case ValidTextureOverload::kSampleBias3dOffsetF32:
            return R"(Texture.sample(Sampler, float3(1.0f, 2.0f, 3.0f), bias(4.0f), int3(5, 6, 7)))";
        case ValidTextureOverload::kSampleBiasCubeF32:
            return R"(Texture.sample(Sampler, float3(1.0f, 2.0f, 3.0f), bias(4.0f)))";
        case ValidTextureOverload::kSampleBiasCubeArrayF32:
            return R"(Texture.sample(Sampler, float3(1.0f, 2.0f, 3.0f), max(0, 3), bias(4.0f)))";
        case ValidTextureOverload::kSampleLevel2dF32:
            return R"(Texture.sample(Sampler, float2(1.0f, 2.0f), level(3.0f)))";
        case ValidTextureOverload::kSampleLevel2dOffsetF32:
            return R"(Texture.sample(Sampler, float2(1.0f, 2.0f), level(3.0f), int2(4, 5)))";
        case ValidTextureOverload::kSampleLevel2dArrayF32:
            return R"(Texture.sample(Sampler, float2(1.0f, 2.0f), max(0, 3), level(4.0f)))";
        case ValidTextureOverload::kSampleLevel2dArrayOffsetF32:
            return R"(Texture.sample(Sampler, float2(1.0f, 2.0f), max(0, 3), level(4.0f), int2(5, 6)))";
        case ValidTextureOverload::kSampleLevel3dF32:
            return R"(Texture.sample(Sampler, float3(1.0f, 2.0f, 3.0f), level(4.0f)))";
        case ValidTextureOverload::kSampleLevel3dOffsetF32:
            return R"(Texture.sample(Sampler, float3(1.0f, 2.0f, 3.0f), level(4.0f), int3(5, 6, 7)))";
        case ValidTextureOverload::kSampleLevelCubeF32:
            return R"(Texture.sample(Sampler, float3(1.0f, 2.0f, 3.0f), level(4.0f)))";
        case ValidTextureOverload::kSampleLevelCubeArrayF32:
            return R"(Texture.sample(Sampler, float3(1.0f, 2.0f, 3.0f), max(0, 4), level(5.0f)))";
        case ValidTextureOverload::kSampleLevelDepth2dF32:
            return R"(Texture.sample(Sampler, float2(1.0f, 2.0f), level(3u)))";
        case ValidTextureOverload::kSampleLevelDepth2dOffsetF32:
            return R"(Texture.sample(Sampler, float2(1.0f, 2.0f), level(3), int2(4, 5)))";
        case ValidTextureOverload::kSampleLevelDepth2dArrayF32:
            return R"(Texture.sample(Sampler, float2(1.0f, 2.0f), 3u, level(4u)))";
        case ValidTextureOverload::kSampleLevelDepth2dArrayOffsetF32:
            return R"(Texture.sample(Sampler, float2(1.0f, 2.0f), 3u, level(4u), int2(5, 6)))";
        case ValidTextureOverload::kSampleLevelDepthCubeF32:
            return R"(Texture.sample(Sampler, float3(1.0f, 2.0f, 3.0f), level(4)))";
        case ValidTextureOverload::kSampleLevelDepthCubeArrayF32:
            return R"(Texture.sample(Sampler, float3(1.0f, 2.0f, 3.0f), max(0, 4), level(5)))";
        case ValidTextureOverload::kSampleGrad2dF32:
            return R"(Texture.sample(Sampler, float2(1.0f, 2.0f), gradient2d(float2(3.0f, 4.0f), float2(5.0f, 6.0f))))";
        case ValidTextureOverload::kSampleGrad2dOffsetF32:
            return R"(Texture.sample(Sampler, float2(1.0f, 2.0f), gradient2d(float2(3.0f, 4.0f), float2(5.0f, 6.0f)), int2(7)))";
        case ValidTextureOverload::kSampleGrad2dArrayF32:
            return R"(Texture.sample(Sampler, float2(1.0f, 2.0f), max(0, 3), gradient2d(float2(4.0f, 5.0f), float2(6.0f, 7.0f))))";
        case ValidTextureOverload::kSampleGrad2dArrayOffsetF32:
            return R"(Texture.sample(Sampler, float2(1.0f, 2.0f), 3u, gradient2d(float2(4.0f, 5.0f), float2(6.0f, 7.0f)), int2(6, 7)))";
        case ValidTextureOverload::kSampleGrad3dF32:
            return R"(Texture.sample(Sampler, float3(1.0f, 2.0f, 3.0f), gradient3d(float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f))))";
        case ValidTextureOverload::kSampleGrad3dOffsetF32:
            return R"(Texture.sample(Sampler, float3(1.0f, 2.0f, 3.0f), gradient3d(float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f)), int3(0, 1, 2)))";
        case ValidTextureOverload::kSampleGradCubeF32:
            return R"(Texture.sample(Sampler, float3(1.0f, 2.0f, 3.0f), gradientcube(float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f))))";
        case ValidTextureOverload::kSampleGradCubeArrayF32:
            return R"(Texture.sample(Sampler, float3(1.0f, 2.0f, 3.0f), 4u, gradientcube(float3(5.0f, 6.0f, 7.0f), float3(8.0f, 9.0f, 10.0f))))";
        case ValidTextureOverload::kSampleCompareDepth2dF32:
            return R"(Texture.sample_compare(Sampler, float2(1.0f, 2.0f), 3.0f))";
        case ValidTextureOverload::kSampleCompareDepth2dOffsetF32:
            return R"(Texture.sample_compare(Sampler, float2(1.0f, 2.0f), 3.0f, int2(4, 5)))";
        case ValidTextureOverload::kSampleCompareDepth2dArrayF32:
            return R"(Texture.sample_compare(Sampler, float2(1.0f, 2.0f), max(0, 4), 3.0f))";
        case ValidTextureOverload::kSampleCompareDepth2dArrayOffsetF32:
            return R"(Texture.sample_compare(Sampler, float2(1.0f, 2.0f), 4u, 3.0f, int2(5, 6)))";
        case ValidTextureOverload::kSampleCompareDepthCubeF32:
            return R"(Texture.sample_compare(Sampler, float3(1.0f, 2.0f, 3.0f), 4.0f))";
        case ValidTextureOverload::kSampleCompareDepthCubeArrayF32:
            return R"(Texture.sample_compare(Sampler, float3(1.0f, 2.0f, 3.0f), max(0, 4), 5.0f))";
        case ValidTextureOverload::kSampleCompareLevelDepth2dF32:
            return R"(Texture.sample_compare(Sampler, float2(1.0f, 2.0f), 3.0f, level(0)))";
        case ValidTextureOverload::kSampleCompareLevelDepth2dOffsetF32:
            return R"(Texture.sample_compare(Sampler, float2(1.0f, 2.0f), 3.0f, level(0), int2(4, 5)))";
        case ValidTextureOverload::kSampleCompareLevelDepth2dArrayF32:
            return R"(Texture.sample_compare(Sampler, float2(1.0f, 2.0f), max(0, 3), 4.0f, level(0)))";
        case ValidTextureOverload::kSampleCompareLevelDepth2dArrayOffsetF32:
            return R"(Texture.sample_compare(Sampler, float2(1.0f, 2.0f), max(0, 3), 4.0f, level(0), int2(5, 6)))";
        case ValidTextureOverload::kSampleCompareLevelDepthCubeF32:
            return R"(Texture.sample_compare(Sampler, float3(1.0f, 2.0f, 3.0f), 4.0f, level(0)))";
        case ValidTextureOverload::kSampleCompareLevelDepthCubeArrayF32:
            return R"(Texture.sample_compare(Sampler, float3(1.0f, 2.0f, 3.0f), max(0, 4), 5.0f, level(0)))";
        case ValidTextureOverload::kLoad1dLevelF32:
            return R"(Texture.read(uint(1u), 0))";
        case ValidTextureOverload::kLoad1dLevelU32:
            return R"(Texture.read(uint(1), 0))";
        case ValidTextureOverload::kLoad1dLevelI32:
            return R"(Texture.read(uint(1), 0))";
        case ValidTextureOverload::kLoad2dLevelF32:
            return R"(Texture.read(uint2(uint2(1u, 2u)), 3u))";
        case ValidTextureOverload::kLoad2dLevelU32:
            return R"(Texture.read(uint2(int2(1, 2)), 3))";
        case ValidTextureOverload::kLoad2dArrayLevelF32:
            return R"(Texture.read(uint2(int2(1, 2)), 3, 4))";
        case ValidTextureOverload::kLoad2dArrayLevelU32:
            return R"(Texture.read(uint2(int2(1, 2)), 3, 4))";
        case ValidTextureOverload::kLoad2dArrayLevelI32:
            return R"(Texture.read(uint2(uint2(1u, 2u)), 3u, 4u))";
        case ValidTextureOverload::kLoad3dLevelF32:
            return R"(Texture.read(uint3(int3(1, 2, 3)), 4))";
        case ValidTextureOverload::kLoad3dLevelU32:
            return R"(Texture.read(uint3(int3(1, 2, 3)), 4))";
        case ValidTextureOverload::kLoad3dLevelI32:
            return R"(Texture.read(uint3(uint3(1u, 2u, 3u)), 4u))";
        case ValidTextureOverload::kLoadMultisampled2dF32:
        case ValidTextureOverload::kLoadMultisampled2dU32:
            return R"(Texture.read(uint2(int2(1, 2)), 3))";
        case ValidTextureOverload::kLoad2dLevelI32:
        case ValidTextureOverload::kLoadMultisampled2dI32:
            return R"(Texture.read(uint2(uint2(1u, 2u)), 3u))";
        case ValidTextureOverload::kLoadDepth2dLevelF32:
            return R"(Texture.read(uint2(int2(1, 2)), 3))";
        case ValidTextureOverload::kLoadDepth2dArrayLevelF32:
            return R"(Texture.read(uint2(uint2(1u, 2u)), 3u, 4u))";
        case ValidTextureOverload::kLoadDepthMultisampled2dF32:
            return R"(Texture.read(uint2(uint2(1u, 2u)), 3u))";
        case ValidTextureOverload::kStoreWO1dRgba32float:
            return R"(Texture.write(float4(2.0f, 3.0f, 4.0f, 5.0f), uint(1)))";
        case ValidTextureOverload::kStoreWO2dRgba32float:
            return R"(Texture.write(float4(3.0f, 4.0f, 5.0f, 6.0f), uint2(int2(1, 2))))";
        case ValidTextureOverload::kStoreWO2dArrayRgba32float:
            return R"(Texture.write(float4(4.0f, 5.0f, 6.0f, 7.0f), uint2(uint2(1u, 2u)), 3u))";
        case ValidTextureOverload::kStoreWO3dRgba32float:
            return R"(Texture.write(float4(4.0f, 5.0f, 6.0f, 7.0f), uint3(uint3(1u, 2u, 3u))))";
    }
    return "<unmatched texture overload>";
}  // NOLINT - Ignore the length of this function

class MslGeneratorBuiltinTextureTest : public TestParamHelper<ast::test::TextureOverloadCase> {};

TEST_P(MslGeneratorBuiltinTextureTest, Call) {
    auto param = GetParam();

    param.BuildTextureVariable(this);
    param.BuildSamplerVariable(this);

    auto* call = Call(param.function, param.args(this));
    auto* stmt = param.returns_value ? static_cast<const ast::Statement*>(Assign(Phony(), call))
                                     : static_cast<const ast::Statement*>(CallStmt(call));

    Func("main", tint::Empty, ty.void_(), Vector{stmt},
         Vector{Stage(ast::PipelineStage::kFragment)});

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.Diagnostics();

    auto expected = expected_texture_overload(param.overload);
    EXPECT_EQ(expected, out.str());
}

INSTANTIATE_TEST_SUITE_P(MslGeneratorBuiltinTextureTest,
                         MslGeneratorBuiltinTextureTest,
                         testing::ValuesIn(ast::test::TextureOverloadCase::ValidCases()));

}  // namespace
}  // namespace tint::msl::writer
