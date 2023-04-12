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

#include "gmock/gmock.h"
#include "src/tint/ast/builtin_texture_helper_test.h"
#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/writer/hlsl/test_helper.h"

namespace tint::writer::hlsl {
namespace {

using ::testing::HasSubstr;

struct ExpectedResult {
    ExpectedResult(const char* o) : out(o) {}  // NOLINT
    ExpectedResult(const char* p, const char* o) : pre(p), out(o) {}

    std::string pre;
    std::string out;
};

ExpectedResult expected_texture_overload(ast::builtin::test::ValidTextureOverload overload) {
    using ValidTextureOverload = ast::builtin::test::ValidTextureOverload;
    switch (overload) {
        case ValidTextureOverload::kDimensions1d:
        case ValidTextureOverload::kDimensionsStorageWO1d:
            return {
                R"(int tint_tmp;
  tint_symbol.GetDimensions(tint_tmp);
)",
                "tint_tmp;",
            };
        case ValidTextureOverload::kDimensions2d:
        case ValidTextureOverload::kDimensionsDepth2d:
        case ValidTextureOverload::kDimensionsStorageWO2d:
            return {
                R"(int2 tint_tmp;
  tint_symbol.GetDimensions(tint_tmp.x, tint_tmp.y);
)",
                "tint_tmp;",
            };
        case ValidTextureOverload::kDimensionsDepthMultisampled2d:
        case ValidTextureOverload::kDimensionsMultisampled2d:
            return {
                R"(int3 tint_tmp;
  tint_symbol.GetDimensions(tint_tmp.x, tint_tmp.y, tint_tmp.z);
)",
                "tint_tmp.xy;",
            };

        case ValidTextureOverload::kDimensions2dArray:
        case ValidTextureOverload::kDimensionsDepth2dArray:
        case ValidTextureOverload::kDimensionsStorageWO2dArray:
            return {
                R"(int3 tint_tmp;
  tint_symbol.GetDimensions(tint_tmp.x, tint_tmp.y, tint_tmp.z);
)",
                "tint_tmp.xy;",
            };
        case ValidTextureOverload::kDimensions3d:
        case ValidTextureOverload::kDimensionsStorageWO3d:
            return {
                R"(int3 tint_tmp;
  tint_symbol.GetDimensions(tint_tmp.x, tint_tmp.y, tint_tmp.z);
)",
                "tint_tmp;",
            };
        case ValidTextureOverload::kDimensionsCube:
        case ValidTextureOverload::kDimensionsDepthCube:
            return {
                R"(int2 tint_tmp;
  tint_symbol.GetDimensions(tint_tmp.x, tint_tmp.y);
)",
                "tint_tmp;",
            };
        case ValidTextureOverload::kDimensionsCubeArray:
        case ValidTextureOverload::kDimensionsDepthCubeArray:
            return {
                R"(int3 tint_tmp;
  tint_symbol.GetDimensions(tint_tmp.x, tint_tmp.y, tint_tmp.z);
)",
                "tint_tmp.xy;",
            };
        case ValidTextureOverload::kDimensions2dLevel:
        case ValidTextureOverload::kDimensionsDepth2dLevel:
            return {
                R"(int3 tint_tmp;
  tint_symbol.GetDimensions(1, tint_tmp.x, tint_tmp.y, tint_tmp.z);
)",
                "tint_tmp.xy;",
            };
        case ValidTextureOverload::kDimensions2dArrayLevel:
        case ValidTextureOverload::kDimensionsDepth2dArrayLevel:
            return {
                R"(int4 tint_tmp;
  tint_symbol.GetDimensions(1, tint_tmp.x, tint_tmp.y, tint_tmp.z, tint_tmp.w);
)",
                "tint_tmp.xy;",
            };
        case ValidTextureOverload::kDimensions3dLevel:
            return {
                R"(int4 tint_tmp;
  tint_symbol.GetDimensions(1, tint_tmp.x, tint_tmp.y, tint_tmp.z, tint_tmp.w);
)",
                "tint_tmp.xyz;",
            };
        case ValidTextureOverload::kDimensionsCubeLevel:
        case ValidTextureOverload::kDimensionsDepthCubeLevel:
            return {
                R"(int3 tint_tmp;
  tint_symbol.GetDimensions(1, tint_tmp.x, tint_tmp.y, tint_tmp.z);
)",
                "tint_tmp.xy;",
            };
        case ValidTextureOverload::kDimensionsCubeArrayLevel:
        case ValidTextureOverload::kDimensionsDepthCubeArrayLevel:
            return {
                R"(int4 tint_tmp;
  tint_symbol.GetDimensions(1, tint_tmp.x, tint_tmp.y, tint_tmp.z, tint_tmp.w);
)",
                "tint_tmp.xy;",
            };
        case ValidTextureOverload::kGather2dF32:
            return R"(tint_symbol.GatherRed(tint_symbol_1, float2(1.0f, 2.0f)))";
        case ValidTextureOverload::kGather2dOffsetF32:
            return R"(tint_symbol.GatherRed(tint_symbol_1, float2(1.0f, 2.0f), int2(3, 4)))";
        case ValidTextureOverload::kGather2dArrayF32:
            return R"(tint_symbol.GatherRed(tint_symbol_1, float3(1.0f, 2.0f, float(3))))";
        case ValidTextureOverload::kGather2dArrayOffsetF32:
            return R"(tint_symbol.GatherRed(tint_symbol_1, float3(1.0f, 2.0f, float(3u)), int2(4, 5)))";
        case ValidTextureOverload::kGatherCubeF32:
            return R"(tint_symbol.GatherRed(tint_symbol_1, float3(1.0f, 2.0f, 3.0f)))";
        case ValidTextureOverload::kGatherCubeArrayF32:
            return R"(tint_symbol.GatherRed(tint_symbol_1, float4(1.0f, 2.0f, 3.0f, float(4u))))";
        case ValidTextureOverload::kGatherDepth2dF32:
            return R"(tint_symbol.Gather(tint_symbol_1, float2(1.0f, 2.0f)))";
        case ValidTextureOverload::kGatherDepth2dOffsetF32:
            return R"(tint_symbol.Gather(tint_symbol_1, float2(1.0f, 2.0f), int2(3, 4)))";
        case ValidTextureOverload::kGatherDepth2dArrayF32:
            return R"(tint_symbol.Gather(tint_symbol_1, float3(1.0f, 2.0f, float(3u))))";
        case ValidTextureOverload::kGatherDepth2dArrayOffsetF32:
            return R"(tint_symbol.Gather(tint_symbol_1, float3(1.0f, 2.0f, float(3)), int2(4, 5)))";
        case ValidTextureOverload::kGatherDepthCubeF32:
            return R"(tint_symbol.Gather(tint_symbol_1, float3(1.0f, 2.0f, 3.0f)))";
        case ValidTextureOverload::kGatherDepthCubeArrayF32:
            return R"(tint_symbol.Gather(tint_symbol_1, float4(1.0f, 2.0f, 3.0f, float(4u))))";
        case ValidTextureOverload::kGatherCompareDepth2dF32:
            return R"(tint_symbol.GatherCmp(tint_symbol_1, float2(1.0f, 2.0f), 3.0f))";
        case ValidTextureOverload::kGatherCompareDepth2dOffsetF32:
            return R"(tint_symbol.GatherCmp(tint_symbol_1, float2(1.0f, 2.0f), 3.0f, int2(4, 5)))";
        case ValidTextureOverload::kGatherCompareDepth2dArrayF32:
            return R"(tint_symbol.GatherCmp(tint_symbol_1, float3(1.0f, 2.0f, float(3)), 4.0f))";
        case ValidTextureOverload::kGatherCompareDepth2dArrayOffsetF32:
            return R"(tint_symbol.GatherCmp(tint_symbol_1, float3(1.0f, 2.0f, float(3)), 4.0f, int2(5, 6)))";
        case ValidTextureOverload::kGatherCompareDepthCubeF32:
            return R"(tint_symbol.GatherCmp(tint_symbol_1, float3(1.0f, 2.0f, 3.0f), 4.0f))";
        case ValidTextureOverload::kGatherCompareDepthCubeArrayF32:
            return R"(tint_symbol.GatherCmp(tint_symbol_1, float4(1.0f, 2.0f, 3.0f, float(4u)), 5.0f))";
        case ValidTextureOverload::kNumLayers2dArray:
        case ValidTextureOverload::kNumLayersDepth2dArray:
        case ValidTextureOverload::kNumLayersCubeArray:
        case ValidTextureOverload::kNumLayersDepthCubeArray:
        case ValidTextureOverload::kNumLayersStorageWO2dArray:
            return {
                R"(int3 tint_tmp;
  tint_symbol.GetDimensions(tint_tmp.x, tint_tmp.y, tint_tmp.z);
)",
                "tint_tmp.z;",
            };
        case ValidTextureOverload::kNumLevels2d:
        case ValidTextureOverload::kNumLevelsCube:
        case ValidTextureOverload::kNumLevelsDepth2d:
        case ValidTextureOverload::kNumLevelsDepthCube:
            return {
                R"(int3 tint_tmp;
  tint_symbol.GetDimensions(0, tint_tmp.x, tint_tmp.y, tint_tmp.z);
)",
                "tint_tmp.z;",
            };
        case ValidTextureOverload::kNumLevels2dArray:
        case ValidTextureOverload::kNumLevels3d:
        case ValidTextureOverload::kNumLevelsCubeArray:
        case ValidTextureOverload::kNumLevelsDepth2dArray:
        case ValidTextureOverload::kNumLevelsDepthCubeArray:
            return {
                R"(int4 tint_tmp;
  tint_symbol.GetDimensions(0, tint_tmp.x, tint_tmp.y, tint_tmp.z, tint_tmp.w);
)",
                "tint_tmp.w;",
            };
        case ValidTextureOverload::kNumSamplesDepthMultisampled2d:
        case ValidTextureOverload::kNumSamplesMultisampled2d:
            return {
                R"(int3 tint_tmp;
  tint_symbol.GetDimensions(tint_tmp.x, tint_tmp.y, tint_tmp.z);
)",
                "tint_tmp.z;",
            };
        case ValidTextureOverload::kSample1dF32:
            return R"(tint_symbol.Sample(tint_symbol_1, 1.0f);)";
        case ValidTextureOverload::kSample2dF32:
            return R"(tint_symbol.Sample(tint_symbol_1, float2(1.0f, 2.0f));)";
        case ValidTextureOverload::kSample2dOffsetF32:
            return R"(tint_symbol.Sample(tint_symbol_1, float2(1.0f, 2.0f), int2(3, 4));)";
        case ValidTextureOverload::kSample2dArrayF32:
            return R"(tint_symbol.Sample(tint_symbol_1, float3(1.0f, 2.0f, float(3)));)";
        case ValidTextureOverload::kSample2dArrayOffsetF32:
            return R"(tint_symbol.Sample(tint_symbol_1, float3(1.0f, 2.0f, float(3u)), int2(4, 5));)";
        case ValidTextureOverload::kSample3dF32:
            return R"(tint_symbol.Sample(tint_symbol_1, float3(1.0f, 2.0f, 3.0f));)";
        case ValidTextureOverload::kSample3dOffsetF32:
            return R"(tint_symbol.Sample(tint_symbol_1, float3(1.0f, 2.0f, 3.0f), int3(4, 5, 6));)";
        case ValidTextureOverload::kSampleCubeF32:
            return R"(tint_symbol.Sample(tint_symbol_1, float3(1.0f, 2.0f, 3.0f));)";
        case ValidTextureOverload::kSampleCubeArrayF32:
            return R"(tint_symbol.Sample(tint_symbol_1, float4(1.0f, 2.0f, 3.0f, float(4)));)";
        case ValidTextureOverload::kSampleDepth2dF32:
            return R"(tint_symbol.Sample(tint_symbol_1, float2(1.0f, 2.0f)).x;)";
        case ValidTextureOverload::kSampleDepth2dOffsetF32:
            return R"(tint_symbol.Sample(tint_symbol_1, float2(1.0f, 2.0f), int2(3, 4)).x;)";
        case ValidTextureOverload::kSampleDepth2dArrayF32:
            return R"(tint_symbol.Sample(tint_symbol_1, float3(1.0f, 2.0f, float(3))).x;)";
        case ValidTextureOverload::kSampleDepth2dArrayOffsetF32:
            return R"(tint_symbol.Sample(tint_symbol_1, float3(1.0f, 2.0f, float(3)), int2(4, 5)).x;)";
        case ValidTextureOverload::kSampleDepthCubeF32:
            return R"(tint_symbol.Sample(tint_symbol_1, float3(1.0f, 2.0f, 3.0f)).x;)";
        case ValidTextureOverload::kSampleDepthCubeArrayF32:
            return R"(tint_symbol.Sample(tint_symbol_1, float4(1.0f, 2.0f, 3.0f, float(4u))).x;)";
        case ValidTextureOverload::kSampleBias2dF32:
            return R"(tint_symbol.SampleBias(tint_symbol_1, float2(1.0f, 2.0f), 3.0f);)";
        case ValidTextureOverload::kSampleBias2dOffsetF32:
            return R"(tint_symbol.SampleBias(tint_symbol_1, float2(1.0f, 2.0f), 3.0f, int2(4, 5));)";
        case ValidTextureOverload::kSampleBias2dArrayF32:
            return R"(tint_symbol.SampleBias(tint_symbol_1, float3(1.0f, 2.0f, float(4u)), 3.0f);)";
        case ValidTextureOverload::kSampleBias2dArrayOffsetF32:
            return R"(tint_symbol.SampleBias(tint_symbol_1, float3(1.0f, 2.0f, float(3)), 4.0f, int2(5, 6));)";
        case ValidTextureOverload::kSampleBias3dF32:
            return R"(tint_symbol.SampleBias(tint_symbol_1, float3(1.0f, 2.0f, 3.0f), 4.0f);)";
        case ValidTextureOverload::kSampleBias3dOffsetF32:
            return R"(tint_symbol.SampleBias(tint_symbol_1, float3(1.0f, 2.0f, 3.0f), 4.0f, int3(5, 6, 7));)";
        case ValidTextureOverload::kSampleBiasCubeF32:
            return R"(tint_symbol.SampleBias(tint_symbol_1, float3(1.0f, 2.0f, 3.0f), 4.0f);)";
        case ValidTextureOverload::kSampleBiasCubeArrayF32:
            return R"(tint_symbol.SampleBias(tint_symbol_1, float4(1.0f, 2.0f, 3.0f, float(3)), 4.0f);)";
        case ValidTextureOverload::kSampleLevel2dF32:
            return R"(tint_symbol.SampleLevel(tint_symbol_1, float2(1.0f, 2.0f), 3.0f);)";
        case ValidTextureOverload::kSampleLevel2dOffsetF32:
            return R"(tint_symbol.SampleLevel(tint_symbol_1, float2(1.0f, 2.0f), 3.0f, int2(4, 5));)";
        case ValidTextureOverload::kSampleLevel2dArrayF32:
            return R"(tint_symbol.SampleLevel(tint_symbol_1, float3(1.0f, 2.0f, float(3)), 4.0f);)";
        case ValidTextureOverload::kSampleLevel2dArrayOffsetF32:
            return R"(tint_symbol.SampleLevel(tint_symbol_1, float3(1.0f, 2.0f, float(3)), 4.0f, int2(5, 6));)";
        case ValidTextureOverload::kSampleLevel3dF32:
            return R"(tint_symbol.SampleLevel(tint_symbol_1, float3(1.0f, 2.0f, 3.0f), 4.0f);)";
        case ValidTextureOverload::kSampleLevel3dOffsetF32:
            return R"(tint_symbol.SampleLevel(tint_symbol_1, float3(1.0f, 2.0f, 3.0f), 4.0f, int3(5, 6, 7));)";
        case ValidTextureOverload::kSampleLevelCubeF32:
            return R"(tint_symbol.SampleLevel(tint_symbol_1, float3(1.0f, 2.0f, 3.0f), 4.0f);)";
        case ValidTextureOverload::kSampleLevelCubeArrayF32:
            return R"(tint_symbol.SampleLevel(tint_symbol_1, float4(1.0f, 2.0f, 3.0f, float(4)), 5.0f);)";
        case ValidTextureOverload::kSampleLevelDepth2dF32:
            return R"(tint_symbol.SampleLevel(tint_symbol_1, float2(1.0f, 2.0f), 3u).x;)";
        case ValidTextureOverload::kSampleLevelDepth2dOffsetF32:
            return R"(tint_symbol.SampleLevel(tint_symbol_1, float2(1.0f, 2.0f), 3, int2(4, 5)).x;)";
        case ValidTextureOverload::kSampleLevelDepth2dArrayF32:
            return R"(tint_symbol.SampleLevel(tint_symbol_1, float3(1.0f, 2.0f, float(3u)), 4u).x;)";
        case ValidTextureOverload::kSampleLevelDepth2dArrayOffsetF32:
            return R"(tint_symbol.SampleLevel(tint_symbol_1, float3(1.0f, 2.0f, float(3u)), 4u, int2(5, 6)).x;)";
        case ValidTextureOverload::kSampleLevelDepthCubeF32:
            return R"(tint_symbol.SampleLevel(tint_symbol_1, float3(1.0f, 2.0f, 3.0f), 4).x;)";
        case ValidTextureOverload::kSampleLevelDepthCubeArrayF32:
            return R"(tint_symbol.SampleLevel(tint_symbol_1, float4(1.0f, 2.0f, 3.0f, float(4)), 5).x;)";
        case ValidTextureOverload::kSampleGrad2dF32:
            return R"(tint_symbol.SampleGrad(tint_symbol_1, float2(1.0f, 2.0f), float2(3.0f, 4.0f), float2(5.0f, 6.0f));)";
        case ValidTextureOverload::kSampleGrad2dOffsetF32:
            return R"(tint_symbol.SampleGrad(tint_symbol_1, float2(1.0f, 2.0f), float2(3.0f, 4.0f), float2(5.0f, 6.0f), (7).xx);)";
        case ValidTextureOverload::kSampleGrad2dArrayF32:
            return R"(tint_symbol.SampleGrad(tint_symbol_1, float3(1.0f, 2.0f, float(3)), float2(4.0f, 5.0f), float2(6.0f, 7.0f));)";
        case ValidTextureOverload::kSampleGrad2dArrayOffsetF32:
            return R"(tint_symbol.SampleGrad(tint_symbol_1, float3(1.0f, 2.0f, float(3u)), float2(4.0f, 5.0f), float2(6.0f, 7.0f), int2(6, 7));)";
        case ValidTextureOverload::kSampleGrad3dF32:
            return R"(tint_symbol.SampleGrad(tint_symbol_1, float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f));)";
        case ValidTextureOverload::kSampleGrad3dOffsetF32:
            return R"(tint_symbol.SampleGrad(tint_symbol_1, float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f), int3(0, 1, 2));)";
        case ValidTextureOverload::kSampleGradCubeF32:
            return R"(tint_symbol.SampleGrad(tint_symbol_1, float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f));)";
        case ValidTextureOverload::kSampleGradCubeArrayF32:
            return R"(tint_symbol.SampleGrad(tint_symbol_1, float4(1.0f, 2.0f, 3.0f, float(4u)), float3(5.0f, 6.0f, 7.0f), float3(8.0f, 9.0f, 10.0f));)";
        case ValidTextureOverload::kSampleCompareDepth2dF32:
            return R"(tint_symbol.SampleCmp(tint_symbol_1, float2(1.0f, 2.0f), 3.0f);)";
        case ValidTextureOverload::kSampleCompareDepth2dOffsetF32:
            return R"(tint_symbol.SampleCmp(tint_symbol_1, float2(1.0f, 2.0f), 3.0f, int2(4, 5));)";
        case ValidTextureOverload::kSampleCompareDepth2dArrayF32:
            return R"(tint_symbol.SampleCmp(tint_symbol_1, float3(1.0f, 2.0f, float(4)), 3.0f);)";
        case ValidTextureOverload::kSampleCompareDepth2dArrayOffsetF32:
            return R"(tint_symbol.SampleCmp(tint_symbol_1, float3(1.0f, 2.0f, float(4u)), 3.0f, int2(5, 6));)";
        case ValidTextureOverload::kSampleCompareDepthCubeF32:
            return R"(tint_symbol.SampleCmp(tint_symbol_1, float3(1.0f, 2.0f, 3.0f), 4.0f);)";
        case ValidTextureOverload::kSampleCompareDepthCubeArrayF32:
            return R"(tint_symbol.SampleCmp(tint_symbol_1, float4(1.0f, 2.0f, 3.0f, float(4)), 5.0f);)";
        case ValidTextureOverload::kSampleCompareLevelDepth2dF32:
            return R"(tint_symbol.SampleCmpLevelZero(tint_symbol_1, float2(1.0f, 2.0f), 3.0f);)";
        case ValidTextureOverload::kSampleCompareLevelDepth2dOffsetF32:
            return R"(tint_symbol.SampleCmpLevelZero(tint_symbol_1, float2(1.0f, 2.0f), 3.0f, int2(4, 5));)";
        case ValidTextureOverload::kSampleCompareLevelDepth2dArrayF32:
            return R"(tint_symbol.SampleCmpLevelZero(tint_symbol_1, float3(1.0f, 2.0f, float(3)), 4.0f);)";
        case ValidTextureOverload::kSampleCompareLevelDepth2dArrayOffsetF32:
            return R"(tint_symbol.SampleCmpLevelZero(tint_symbol_1, float3(1.0f, 2.0f, float(3)), 4.0f, int2(5, 6));)";
        case ValidTextureOverload::kSampleCompareLevelDepthCubeF32:
            return R"(tint_symbol.SampleCmpLevelZero(tint_symbol_1, float3(1.0f, 2.0f, 3.0f), 4.0f);)";
        case ValidTextureOverload::kSampleCompareLevelDepthCubeArrayF32:
            return R"(tint_symbol.SampleCmpLevelZero(tint_symbol_1, float4(1.0f, 2.0f, 3.0f, float(4)), 5.0f);)";
        case ValidTextureOverload::kLoad1dLevelF32:
            return R"(tint_symbol.Load(uint2(1u, 3u));)";
        case ValidTextureOverload::kLoad1dLevelU32:
        case ValidTextureOverload::kLoad1dLevelI32:
            return R"(tint_symbol.Load(int2(1, 3));)";
        case ValidTextureOverload::kLoad2dLevelU32:
            return R"(tint_symbol.Load(int3(1, 2, 3));)";
        case ValidTextureOverload::kLoad2dLevelF32:
        case ValidTextureOverload::kLoad2dLevelI32:
            return R"(tint_symbol.Load(uint3(1u, 2u, 3u));)";
        case ValidTextureOverload::kLoad2dArrayLevelF32:
        case ValidTextureOverload::kLoad2dArrayLevelU32:
        case ValidTextureOverload::kLoad3dLevelF32:
        case ValidTextureOverload::kLoad3dLevelU32:
            return R"(tint_symbol.Load(int4(1, 2, 3, 4));)";
        case ValidTextureOverload::kLoad2dArrayLevelI32:
        case ValidTextureOverload::kLoad3dLevelI32:
            return R"(tint_symbol.Load(uint4(1u, 2u, 3u, 4u));)";
        case ValidTextureOverload::kLoadMultisampled2dF32:
        case ValidTextureOverload::kLoadMultisampled2dU32:
            return R"(tint_symbol.Load(int2(1, 2), 3);)";
        case ValidTextureOverload::kLoadMultisampled2dI32:
            return R"(tint_symbol.Load(uint2(1u, 2u), 3u);)";
        case ValidTextureOverload::kLoadDepth2dLevelF32:
            return R"(tint_symbol.Load(int3(1, 2, 3)).x;)";
        case ValidTextureOverload::kLoadDepth2dArrayLevelF32:
            return R"(tint_symbol.Load(uint4(1u, 2u, 3u, 4u)).x;)";
        case ValidTextureOverload::kLoadDepthMultisampled2dF32:
            return R"(tint_symbol.Load(uint3(1u, 2u, uint(0)), 3u).x;)";
        case ValidTextureOverload::kStoreWO1dRgba32float:
            return R"(tint_symbol[1] = float4(2.0f, 3.0f, 4.0f, 5.0f);)";
        case ValidTextureOverload::kStoreWO2dRgba32float:
            return R"(tint_symbol[int2(1, 2)] = float4(3.0f, 4.0f, 5.0f, 6.0f);)";
        case ValidTextureOverload::kStoreWO2dArrayRgba32float:
            return R"(tint_symbol[uint3(1u, 2u, 3u)] = float4(4.0f, 5.0f, 6.0f, 7.0f);)";
        case ValidTextureOverload::kStoreWO3dRgba32float:
            return R"(tint_symbol[uint3(1u, 2u, 3u)] = float4(4.0f, 5.0f, 6.0f, 7.0f);)";
    }
    return "<unmatched texture overload>";
}  // NOLINT - Ignore the length of this function

class HlslGeneratorBuiltinTextureTest
    : public TestParamHelper<ast::builtin::test::TextureOverloadCase> {};

TEST_P(HlslGeneratorBuiltinTextureTest, Call) {
    auto param = GetParam();

    param.BuildTextureVariable(this);
    param.BuildSamplerVariable(this);

    auto* call = Call(param.function, param.args(this));
    auto* stmt = param.returns_value ? static_cast<const ast::Statement*>(Decl(Var("v", call)))
                                     : static_cast<const ast::Statement*>(CallStmt(call));

    Func("main", utils::Empty, ty.void_(), utils::Vector{stmt},
         utils::Vector{Stage(ast::PipelineStage::kFragment)});

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    auto expected = expected_texture_overload(param.overload);

    EXPECT_THAT(gen.result(), HasSubstr(expected.pre));
    EXPECT_THAT(gen.result(), HasSubstr(expected.out));
}

INSTANTIATE_TEST_SUITE_P(HlslGeneratorBuiltinTextureTest,
                         HlslGeneratorBuiltinTextureTest,
                         testing::ValuesIn(ast::builtin::test::TextureOverloadCase::ValidCases()));

}  // namespace
}  // namespace tint::writer::hlsl
