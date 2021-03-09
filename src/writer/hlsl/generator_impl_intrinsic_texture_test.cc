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

#include "src/ast/call_statement.h"
#include "src/ast/intrinsic_texture_helper_test.h"
#include "src/ast/stage_decoration.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

struct ExpectedResult {
  ExpectedResult(const char* o) : out(o) {}  // NOLINT
  ExpectedResult(const char* p, const char* o) : pre(p), out(o) {}

  std::string pre;
  std::string out;
};

ExpectedResult expected_texture_overload(
    ast::intrinsic::test::ValidTextureOverload overload) {
  using ValidTextureOverload = ast::intrinsic::test::ValidTextureOverload;
  switch (overload) {
    case ValidTextureOverload::kDimensions1d:
    case ValidTextureOverload::kDimensionsStorageRO1d:
    case ValidTextureOverload::kDimensionsStorageWO1d:
      return {
          R"(int _tint_tmp;
texture_tint_0.GetDimensions(_tint_tmp);
)",
          "_tint_tmp",
      };
    case ValidTextureOverload::kDimensions2d:
    case ValidTextureOverload::kDimensionsDepth2d:
    case ValidTextureOverload::kDimensionsStorageRO2d:
    case ValidTextureOverload::kDimensionsStorageWO2d:
      return {
          R"(int2 _tint_tmp;
texture_tint_0.GetDimensions(_tint_tmp.x, _tint_tmp.y);
)",
          "_tint_tmp",
      };
    case ValidTextureOverload::kDimensionsMultisampled2d:
      return {
          R"(int3 _tint_tmp;
texture_tint_0.GetDimensions(_tint_tmp.x, _tint_tmp.y, _tint_tmp.z);
)",
          "_tint_tmp.xy",
      };

    case ValidTextureOverload::kDimensions2dArray:
    case ValidTextureOverload::kDimensionsDepth2dArray:
    case ValidTextureOverload::kDimensionsStorageRO2dArray:
    case ValidTextureOverload::kDimensionsStorageWO2dArray:
      return {
          R"(int3 _tint_tmp;
texture_tint_0.GetDimensions(_tint_tmp.x, _tint_tmp.y, _tint_tmp.z);
)",
          "_tint_tmp.xy",
      };
    case ValidTextureOverload::kDimensionsMultisampled2dArray:
      return {
          R"(int4 _tint_tmp;
texture_tint_0.GetDimensions(_tint_tmp.x, _tint_tmp.y, _tint_tmp.z, _tint_tmp.w);
)",
          "_tint_tmp.xy",
      };
    case ValidTextureOverload::kDimensions3d:
    case ValidTextureOverload::kDimensionsStorageRO3d:
    case ValidTextureOverload::kDimensionsStorageWO3d:
      return {
          R"(int3 _tint_tmp;
texture_tint_0.GetDimensions(_tint_tmp.x, _tint_tmp.y, _tint_tmp.z);
)",
          "_tint_tmp",
      };
    case ValidTextureOverload::kDimensionsCube:
    case ValidTextureOverload::kDimensionsDepthCube:
      return {
          "int2 _tint_tmp;\n"
          "texture_tint_0.GetDimensions(_tint_tmp.x, _tint_tmp.y);\n",
          "_tint_tmp.xyy",
      };
    case ValidTextureOverload::kDimensionsCubeArray:
    case ValidTextureOverload::kDimensionsDepthCubeArray:
      return {
          R"(int3 _tint_tmp;
texture_tint_0.GetDimensions(_tint_tmp.x, _tint_tmp.y, _tint_tmp.z);
)",
          "_tint_tmp.xyy",
      };
    case ValidTextureOverload::kDimensions2dLevel:
    case ValidTextureOverload::kDimensionsDepth2dLevel:
      return {
          R"(int3 _tint_tmp;
texture_tint_0.GetDimensions(1, _tint_tmp.x, _tint_tmp.y, _tint_tmp.z);
)",
          "_tint_tmp.xy",
      };
    case ValidTextureOverload::kDimensions2dArrayLevel:
    case ValidTextureOverload::kDimensionsDepth2dArrayLevel:
      return {
          R"(int4 _tint_tmp;
texture_tint_0.GetDimensions(1, _tint_tmp.x, _tint_tmp.y, _tint_tmp.z, _tint_tmp.w);
)",
          "_tint_tmp.xy",
      };
    case ValidTextureOverload::kDimensions3dLevel:
      return {
          R"(int4 _tint_tmp;
texture_tint_0.GetDimensions(1, _tint_tmp.x, _tint_tmp.y, _tint_tmp.z, _tint_tmp.w);
)",
          "_tint_tmp.xyz",
      };
    case ValidTextureOverload::kDimensionsCubeLevel:
    case ValidTextureOverload::kDimensionsDepthCubeLevel:
      return {
          R"(int3 _tint_tmp;
texture_tint_0.GetDimensions(1, _tint_tmp.x, _tint_tmp.y, _tint_tmp.z);
)",
          "_tint_tmp.xyy",
      };
    case ValidTextureOverload::kDimensionsCubeArrayLevel:
    case ValidTextureOverload::kDimensionsDepthCubeArrayLevel:
      return {
          R"(int4 _tint_tmp;
texture_tint_0.GetDimensions(1, _tint_tmp.x, _tint_tmp.y, _tint_tmp.z, _tint_tmp.w);
)",
          "_tint_tmp.xyy",
      };
    case ValidTextureOverload::kNumLayers2dArray:
    case ValidTextureOverload::kNumLayersDepth2dArray:
    case ValidTextureOverload::kNumLayersCubeArray:
    case ValidTextureOverload::kNumLayersDepthCubeArray:
    case ValidTextureOverload::kNumLayersStorageWO2dArray:
      return {
          R"(int3 _tint_tmp;
texture_tint_0.GetDimensions(_tint_tmp.x, _tint_tmp.y, _tint_tmp.z);
)",
          "_tint_tmp.z",
      };
    case ValidTextureOverload::kNumLayersMultisampled2dArray:
      return {
          R"(int4 _tint_tmp;
texture_tint_0.GetDimensions(_tint_tmp.x, _tint_tmp.y, _tint_tmp.z, _tint_tmp.w);
)",
          "_tint_tmp.z",
      };

    case ValidTextureOverload::kNumLevels2d:
    case ValidTextureOverload::kNumLevelsCube:
    case ValidTextureOverload::kNumLevelsDepth2d:
    case ValidTextureOverload::kNumLevelsDepthCube:
      return {
          R"(int3 _tint_tmp;
texture_tint_0.GetDimensions(0, _tint_tmp.x, _tint_tmp.y, _tint_tmp.z);
)",
          "_tint_tmp.z",
      };
    case ValidTextureOverload::kNumLevels2dArray:
    case ValidTextureOverload::kNumLevels3d:
    case ValidTextureOverload::kNumLevelsCubeArray:
    case ValidTextureOverload::kNumLevelsDepth2dArray:
    case ValidTextureOverload::kNumLevelsDepthCubeArray:
      return {
          R"(int4 _tint_tmp;
texture_tint_0.GetDimensions(0, _tint_tmp.x, _tint_tmp.y, _tint_tmp.z, _tint_tmp.w);
)",
          "_tint_tmp.w",
      };
    case ValidTextureOverload::kNumSamplesMultisampled2d:
      return {
          R"(int3 _tint_tmp;
texture_tint_0.GetDimensions(_tint_tmp.x, _tint_tmp.y, _tint_tmp.z);
)",
          "_tint_tmp.z",
      };
    case ValidTextureOverload::kNumSamplesMultisampled2dArray:
      return {
          R"(int4 _tint_tmp;
texture_tint_0.GetDimensions(_tint_tmp.x, _tint_tmp.y, _tint_tmp.z, _tint_tmp.w);
)",
          "_tint_tmp.w",
      };
    case ValidTextureOverload::kSample1dF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, 1.0f))";
    case ValidTextureOverload::kSample2dF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, float2(1.0f, 2.0f)))";
    case ValidTextureOverload::kSample2dOffsetF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, float2(1.0f, 2.0f), int2(3, 4)))";
    case ValidTextureOverload::kSample2dArrayF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, float3(1.0f, 2.0f, float(3))))";
    case ValidTextureOverload::kSample2dArrayOffsetF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, float3(1.0f, 2.0f, float(3)), int2(4, 5)))";
    case ValidTextureOverload::kSample3dF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, float3(1.0f, 2.0f, 3.0f)))";
    case ValidTextureOverload::kSample3dOffsetF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, float3(1.0f, 2.0f, 3.0f), int3(4, 5, 6)))";
    case ValidTextureOverload::kSampleCubeF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, float3(1.0f, 2.0f, 3.0f)))";
    case ValidTextureOverload::kSampleCubeArrayF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, float4(1.0f, 2.0f, 3.0f, float(4))))";
    case ValidTextureOverload::kSampleDepth2dF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, float2(1.0f, 2.0f)))";
    case ValidTextureOverload::kSampleDepth2dOffsetF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, float2(1.0f, 2.0f), int2(3, 4)))";
    case ValidTextureOverload::kSampleDepth2dArrayF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, float3(1.0f, 2.0f, float(3))))";
    case ValidTextureOverload::kSampleDepth2dArrayOffsetF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, float3(1.0f, 2.0f, float(3)), int2(4, 5)))";
    case ValidTextureOverload::kSampleDepthCubeF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, float3(1.0f, 2.0f, 3.0f)))";
    case ValidTextureOverload::kSampleDepthCubeArrayF32:
      return R"(texture_tint_0.Sample(sampler_tint_0, float4(1.0f, 2.0f, 3.0f, float(4))))";
    case ValidTextureOverload::kSampleBias2dF32:
      return R"(texture_tint_0.SampleBias(sampler_tint_0, float2(1.0f, 2.0f), 3.0f))";
    case ValidTextureOverload::kSampleBias2dOffsetF32:
      return R"(texture_tint_0.SampleBias(sampler_tint_0, float2(1.0f, 2.0f), 3.0f, int2(4, 5)))";
    case ValidTextureOverload::kSampleBias2dArrayF32:
      return R"(texture_tint_0.SampleBias(sampler_tint_0, float3(1.0f, 2.0f, float(4)), 3.0f))";
    case ValidTextureOverload::kSampleBias2dArrayOffsetF32:
      return R"(texture_tint_0.SampleBias(sampler_tint_0, float3(1.0f, 2.0f, float(3)), 4.0f, int2(5, 6)))";
    case ValidTextureOverload::kSampleBias3dF32:
      return R"(texture_tint_0.SampleBias(sampler_tint_0, float3(1.0f, 2.0f, 3.0f), 4.0f))";
    case ValidTextureOverload::kSampleBias3dOffsetF32:
      return R"(texture_tint_0.SampleBias(sampler_tint_0, float3(1.0f, 2.0f, 3.0f), 4.0f, int3(5, 6, 7)))";
    case ValidTextureOverload::kSampleBiasCubeF32:
      return R"(texture_tint_0.SampleBias(sampler_tint_0, float3(1.0f, 2.0f, 3.0f), 4.0f))";
    case ValidTextureOverload::kSampleBiasCubeArrayF32:
      return R"(texture_tint_0.SampleBias(sampler_tint_0, float4(1.0f, 2.0f, 3.0f, float(3)), 4.0f))";
    case ValidTextureOverload::kSampleLevel2dF32:
      return R"(texture_tint_0.SampleLevel(sampler_tint_0, float2(1.0f, 2.0f), 3.0f))";
    case ValidTextureOverload::kSampleLevel2dOffsetF32:
      return R"(texture_tint_0.SampleLevel(sampler_tint_0, float2(1.0f, 2.0f), 3.0f, int2(4, 5)))";
    case ValidTextureOverload::kSampleLevel2dArrayF32:
      return R"(texture_tint_0.SampleLevel(sampler_tint_0, float3(1.0f, 2.0f, float(3)), 4.0f))";
    case ValidTextureOverload::kSampleLevel2dArrayOffsetF32:
      return R"(texture_tint_0.SampleLevel(sampler_tint_0, float3(1.0f, 2.0f, float(3)), 4.0f, int2(5, 6)))";
    case ValidTextureOverload::kSampleLevel3dF32:
      return R"(texture_tint_0.SampleLevel(sampler_tint_0, float3(1.0f, 2.0f, 3.0f), 4.0f))";
    case ValidTextureOverload::kSampleLevel3dOffsetF32:
      return R"(texture_tint_0.SampleLevel(sampler_tint_0, float3(1.0f, 2.0f, 3.0f), 4.0f, int3(5, 6, 7)))";
    case ValidTextureOverload::kSampleLevelCubeF32:
      return R"(texture_tint_0.SampleLevel(sampler_tint_0, float3(1.0f, 2.0f, 3.0f), 4.0f))";
    case ValidTextureOverload::kSampleLevelCubeArrayF32:
      return R"(texture_tint_0.SampleLevel(sampler_tint_0, float4(1.0f, 2.0f, 3.0f, float(4)), 5.0f))";
    case ValidTextureOverload::kSampleLevelDepth2dF32:
      return R"(texture_tint_0.SampleLevel(sampler_tint_0, float2(1.0f, 2.0f), 3))";
    case ValidTextureOverload::kSampleLevelDepth2dOffsetF32:
      return R"(texture_tint_0.SampleLevel(sampler_tint_0, float2(1.0f, 2.0f), 3, int2(4, 5)))";
    case ValidTextureOverload::kSampleLevelDepth2dArrayF32:
      return R"(texture_tint_0.SampleLevel(sampler_tint_0, float3(1.0f, 2.0f, float(3)), 4))";
    case ValidTextureOverload::kSampleLevelDepth2dArrayOffsetF32:
      return R"(texture_tint_0.SampleLevel(sampler_tint_0, float3(1.0f, 2.0f, float(3)), 4, int2(5, 6)))";
    case ValidTextureOverload::kSampleLevelDepthCubeF32:
      return R"(texture_tint_0.SampleLevel(sampler_tint_0, float3(1.0f, 2.0f, 3.0f), 4))";
    case ValidTextureOverload::kSampleLevelDepthCubeArrayF32:
      return R"(texture_tint_0.SampleLevel(sampler_tint_0, float4(1.0f, 2.0f, 3.0f, float(4)), 5))";
    case ValidTextureOverload::kSampleGrad2dF32:
      return R"(texture_tint_0.SampleGrad(sampler_tint_0, float2(1.0f, 2.0f), float2(3.0f, 4.0f), float2(5.0f, 6.0f)))";
    case ValidTextureOverload::kSampleGrad2dOffsetF32:
      return R"(texture_tint_0.SampleGrad(sampler_tint_0, float2(1.0f, 2.0f), float2(3.0f, 4.0f), float2(5.0f, 6.0f), int2(7, 8)))";
    case ValidTextureOverload::kSampleGrad2dArrayF32:
      return R"(texture_tint_0.SampleGrad(sampler_tint_0, float3(1.0f, 2.0f, float(3)), float2(4.0f, 5.0f), float2(6.0f, 7.0f)))";
    case ValidTextureOverload::kSampleGrad2dArrayOffsetF32:
      return R"(texture_tint_0.SampleGrad(sampler_tint_0, float3(1.0f, 2.0f, float(3)), float2(4.0f, 5.0f), float2(6.0f, 7.0f), int2(8, 9)))";
    case ValidTextureOverload::kSampleGrad3dF32:
      return R"(texture_tint_0.SampleGrad(sampler_tint_0, float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f)))";
    case ValidTextureOverload::kSampleGrad3dOffsetF32:
      return R"(texture_tint_0.SampleGrad(sampler_tint_0, float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f), int3(10, 11, 12)))";
    case ValidTextureOverload::kSampleGradCubeF32:
      return R"(texture_tint_0.SampleGrad(sampler_tint_0, float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f)))";
    case ValidTextureOverload::kSampleGradCubeArrayF32:
      return R"(texture_tint_0.SampleGrad(sampler_tint_0, float4(1.0f, 2.0f, 3.0f, float(4)), float3(5.0f, 6.0f, 7.0f), float3(8.0f, 9.0f, 10.0f)))";
    case ValidTextureOverload::kSampleCompareDepth2dF32:
      return R"(texture_tint_0.SampleCmp(sampler_tint_0, float2(1.0f, 2.0f), 3.0f))";
    case ValidTextureOverload::kSampleCompareDepth2dOffsetF32:
      return R"(texture_tint_0.SampleCmp(sampler_tint_0, float2(1.0f, 2.0f), 3.0f, int2(4, 5)))";
    case ValidTextureOverload::kSampleCompareDepth2dArrayF32:
      return R"(texture_tint_0.SampleCmp(sampler_tint_0, float3(1.0f, 2.0f, float(4)), 3.0f))";
    case ValidTextureOverload::kSampleCompareDepth2dArrayOffsetF32:
      return R"(texture_tint_0.SampleCmp(sampler_tint_0, float3(1.0f, 2.0f, float(4)), 3.0f, int2(5, 6)))";
    case ValidTextureOverload::kSampleCompareDepthCubeF32:
      return R"(texture_tint_0.SampleCmp(sampler_tint_0, float3(1.0f, 2.0f, 3.0f), 4.0f))";
    case ValidTextureOverload::kSampleCompareDepthCubeArrayF32:
      return R"(texture_tint_0.SampleCmp(sampler_tint_0, float4(1.0f, 2.0f, 3.0f, float(4)), 5.0f))";
    case ValidTextureOverload::kLoad1dLevelF32:
      return R"(texture_tint_0.Load(int2(1, 0), 3))";
    case ValidTextureOverload::kLoad1dLevelU32:
      return R"(texture_tint_0.Load(int2(1, 0), 3))";
    case ValidTextureOverload::kLoad1dLevelI32:
      return R"(texture_tint_0.Load(int2(1, 0), 3))";
    case ValidTextureOverload::kLoad2dLevelF32:
      return R"(texture_tint_0.Load(int3(1, 2, 0), 3))";
    case ValidTextureOverload::kLoad2dLevelU32:
      return R"(texture_tint_0.Load(int3(1, 2, 0), 3))";
    case ValidTextureOverload::kLoad2dLevelI32:
      return R"(texture_tint_0.Load(int3(1, 2, 0), 3))";
    case ValidTextureOverload::kLoad2dArrayLevelF32:
      return R"(texture_tint_0.Load(int4(1, 2, 3, 0), 4))";
    case ValidTextureOverload::kLoad2dArrayLevelU32:
      return R"(texture_tint_0.Load(int4(1, 2, 3, 0), 4))";
    case ValidTextureOverload::kLoad2dArrayLevelI32:
      return R"(texture_tint_0.Load(int4(1, 2, 3, 0), 4))";
    case ValidTextureOverload::kLoad3dLevelF32:
      return R"(texture_tint_0.Load(int4(1, 2, 3, 0), 4))";
    case ValidTextureOverload::kLoad3dLevelU32:
      return R"(texture_tint_0.Load(int4(1, 2, 3, 0), 4))";
    case ValidTextureOverload::kLoad3dLevelI32:
      return R"(texture_tint_0.Load(int4(1, 2, 3, 0), 4))";
    case ValidTextureOverload::kLoadMultisampled2dF32:
      return R"(texture_tint_0.Load(int3(1, 2, 0), 3))";
    case ValidTextureOverload::kLoadMultisampled2dU32:
      return R"(texture_tint_0.Load(int3(1, 2, 0), 3))";
    case ValidTextureOverload::kLoadMultisampled2dI32:
      return R"(texture_tint_0.Load(int3(1, 2, 0), 3))";
    case ValidTextureOverload::kLoadMultisampled2dArrayF32:
      return R"(texture_tint_0.Load(int4(1, 2, 3, 0), 4))";
    case ValidTextureOverload::kLoadMultisampled2dArrayU32:
      return R"(texture_tint_0.Load(int4(1, 2, 3, 0), 4))";
    case ValidTextureOverload::kLoadMultisampled2dArrayI32:
      return R"(texture_tint_0.Load(int4(1, 2, 3, 0), 4))";
    case ValidTextureOverload::kLoadDepth2dLevelF32:
      return R"(texture_tint_0.Load(int3(1, 2, 0), 3))";
    case ValidTextureOverload::kLoadDepth2dArrayLevelF32:
      return R"(texture_tint_0.Load(int4(1, 2, 3, 0), 4))";
    case ValidTextureOverload::kLoadStorageRO1dRgba32float:
      return R"(texture_tint_0.Load(1))";
    case ValidTextureOverload::kLoadStorageRO2dRgba8unorm:
    case ValidTextureOverload::kLoadStorageRO2dRgba8snorm:
    case ValidTextureOverload::kLoadStorageRO2dRgba8uint:
    case ValidTextureOverload::kLoadStorageRO2dRgba8sint:
    case ValidTextureOverload::kLoadStorageRO2dRgba16uint:
    case ValidTextureOverload::kLoadStorageRO2dRgba16sint:
    case ValidTextureOverload::kLoadStorageRO2dRgba16float:
    case ValidTextureOverload::kLoadStorageRO2dR32uint:
    case ValidTextureOverload::kLoadStorageRO2dR32sint:
    case ValidTextureOverload::kLoadStorageRO2dR32float:
    case ValidTextureOverload::kLoadStorageRO2dRg32uint:
    case ValidTextureOverload::kLoadStorageRO2dRg32sint:
    case ValidTextureOverload::kLoadStorageRO2dRg32float:
    case ValidTextureOverload::kLoadStorageRO2dRgba32uint:
    case ValidTextureOverload::kLoadStorageRO2dRgba32sint:
    case ValidTextureOverload::kLoadStorageRO2dRgba32float:
      return R"(texture_tint_0.Load(int2(1, 2)))";
    case ValidTextureOverload::kLoadStorageRO2dArrayRgba32float:
      return R"(texture_tint_0.Load(int3(1, 2, 3)))";
    case ValidTextureOverload::kLoadStorageRO3dRgba32float:
      return R"(texture_tint_0.Load(int3(1, 2, 3)))";
    case ValidTextureOverload::kStoreWO1dRgba32float:
      return R"(texture_tint_0[1] = float4(2.0f, 3.0f, 4.0f, 5.0f))";
    case ValidTextureOverload::kStoreWO2dRgba32float:
      return R"(texture_tint_0[int2(1, 2)] = float4(3.0f, 4.0f, 5.0f, 6.0f))";
    case ValidTextureOverload::kStoreWO2dArrayRgba32float:
      return R"(texture_tint_0[int3(1, 2, 3)] = float4(4.0f, 5.0f, 6.0f, 7.0f))";
    case ValidTextureOverload::kStoreWO3dRgba32float:
      return R"(texture_tint_0[int3(1, 2, 3)] = float4(4.0f, 5.0f, 6.0f, 7.0f))";
  }
  return "<unmatched texture overload>";
}  // NOLINT - Ignore the length of this function

class HlslGeneratorIntrinsicTextureTest
    : public TestParamHelper<ast::intrinsic::test::TextureOverloadCase> {};

TEST_P(HlslGeneratorIntrinsicTextureTest, Call) {
  auto param = GetParam();

  param.buildTextureVariable(this);
  param.buildSamplerVariable(this);

  auto* call = Call(param.function, param.args(this));

  Func("main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::CallStatement>(call),
       },
       ast::FunctionDecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitExpression(pre, out, call)) << gen.error();

  auto expected = expected_texture_overload(param.overload);

  EXPECT_EQ(expected.pre, pre_result());
  EXPECT_EQ(expected.out, result());

  Validate();
}

INSTANTIATE_TEST_SUITE_P(
    HlslGeneratorIntrinsicTextureTest,
    HlslGeneratorIntrinsicTextureTest,
    testing::ValuesIn(ast::intrinsic::test::TextureOverloadCase::ValidCases()));

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
