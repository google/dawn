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
#include "src/ast/call_statement.h"
#include "src/ast/intrinsic_texture_helper_test.h"
#include "src/ast/stage_decoration.h"
#include "src/writer/spirv/spv_dump.h"
#include "src/writer/spirv/test_helper.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

struct expected_texture_overload_spirv {
  std::string types;
  std::string instructions;
  std::string capabilities;
};

expected_texture_overload_spirv expected_texture_overload(
    ast::intrinsic::test::ValidTextureOverload overload) {
  using ValidTextureOverload = ast::intrinsic::test::ValidTextureOverload;
  switch (overload) {
    case ValidTextureOverload::kDimensions1d:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 1D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeInt 32 1
%11 = OpConstant %9 0
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageQuerySizeLod %9 %10 %11
)",
          R"(
OpCapability Sampled1D
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kDimensions2d:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeInt 32 1
%9 = OpTypeVector %10 2
%12 = OpConstant %10 0
)",
          R"(
%11 = OpLoad %3 %1
%8 = OpImageQuerySizeLod %9 %11 %12
)",
          R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kDimensions2dLevel:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeInt 32 1
%9 = OpTypeVector %10 2
%12 = OpConstant %10 1
)",
          R"(
%11 = OpLoad %3 %1
%8 = OpImageQuerySizeLod %9 %11 %12
)",
          R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kDimensions2dArray:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeInt 32 1
%9 = OpTypeVector %10 2
%12 = OpTypeVector %10 3
%14 = OpConstant %10 0
)",
          R"(
%13 = OpLoad %3 %1
%11 = OpImageQuerySizeLod %12 %13 %14
%8 = OpVectorShuffle %9 %11 %11 0 1
)",
          R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kDimensions2dArrayLevel:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeInt 32 1
%9 = OpTypeVector %10 2
%12 = OpTypeVector %10 3
%14 = OpConstant %10 1
)",
          R"(
%13 = OpLoad %3 %1
%11 = OpImageQuerySizeLod %12 %13 %14
%8 = OpVectorShuffle %9 %11 %11 0 1
)",
          R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kDimensions3d:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeInt 32 1
%9 = OpTypeVector %10 3
%12 = OpConstant %10 0
)",
          R"(
%11 = OpLoad %3 %1
%8 = OpImageQuerySizeLod %9 %11 %12
)",
          R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kDimensions3dLevel:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeInt 32 1
%9 = OpTypeVector %10 3
%12 = OpConstant %10 1
)",
          R"(
%11 = OpLoad %3 %1
%8 = OpImageQuerySizeLod %9 %11 %12
)",
          R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kDimensionsCube:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeInt 32 1
%9 = OpTypeVector %10 3
%12 = OpTypeVector %10 2
%14 = OpConstant %10 0
)",
          R"(
%13 = OpLoad %3 %1
%11 = OpImageQuerySizeLod %12 %13 %14
%8 = OpVectorShuffle %9 %11 %11 0 1 1
)",
          R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kDimensionsCubeLevel:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeInt 32 1
%9 = OpTypeVector %10 3
%12 = OpTypeVector %10 2
%14 = OpConstant %10 1
)",
          R"(
%13 = OpLoad %3 %1
%11 = OpImageQuerySizeLod %12 %13 %14
%8 = OpVectorShuffle %9 %11 %11 0 1 1
)",
          R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kDimensionsCubeArray:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeInt 32 1
%9 = OpTypeVector %10 3
%13 = OpConstant %10 0
)",
          R"(
%12 = OpLoad %3 %1
%11 = OpImageQuerySizeLod %9 %12 %13
%8 = OpVectorShuffle %9 %11 %11 0 1 1
)",
          R"(
OpCapability SampledCubeArray
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kDimensionsCubeArrayLevel:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeInt 32 1
%9 = OpTypeVector %10 3
%13 = OpConstant %10 1
)",
          R"(
%12 = OpLoad %3 %1
%11 = OpImageQuerySizeLod %9 %12 %13
%8 = OpVectorShuffle %9 %11 %11 0 1 1
)",
          R"(
OpCapability SampledCubeArray
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kDimensionsMultisampled2d:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 1 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeInt 32 1
%9 = OpTypeVector %10 2
)",
          R"(
%11 = OpLoad %3 %1
%8 = OpImageQuerySize %9 %11
)",
          R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kDimensionsDepth2d:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeInt 32 1
%9 = OpTypeVector %10 2
%12 = OpConstant %10 0
)",
          R"(
%11 = OpLoad %3 %1
%8 = OpImageQuerySizeLod %9 %11 %12
)",
          R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kDimensionsDepth2dLevel:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeInt 32 1
%9 = OpTypeVector %10 2
%12 = OpConstant %10 1
)",
          R"(
%11 = OpLoad %3 %1
%8 = OpImageQuerySizeLod %9 %11 %12
)",
          R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kDimensionsDepth2dArray:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeInt 32 1
%9 = OpTypeVector %10 2
%12 = OpTypeVector %10 3
%14 = OpConstant %10 0
)",
          R"(
%13 = OpLoad %3 %1
%11 = OpImageQuerySizeLod %12 %13 %14
%8 = OpVectorShuffle %9 %11 %11 0 1
)",
          R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kDimensionsDepth2dArrayLevel:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeInt 32 1
%9 = OpTypeVector %10 2
%12 = OpTypeVector %10 3
%14 = OpConstant %10 1
)",
          R"(
%13 = OpLoad %3 %1
%11 = OpImageQuerySizeLod %12 %13 %14
%8 = OpVectorShuffle %9 %11 %11 0 1
)",
          R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kDimensionsDepthCube:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 1 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeInt 32 1
%9 = OpTypeVector %10 3
%12 = OpTypeVector %10 2
%14 = OpConstant %10 0
)",
          R"(
%13 = OpLoad %3 %1
%11 = OpImageQuerySizeLod %12 %13 %14
%8 = OpVectorShuffle %9 %11 %11 0 1 1
)",
          R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kDimensionsDepthCubeLevel:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 1 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeInt 32 1
%9 = OpTypeVector %10 3
%12 = OpTypeVector %10 2
%14 = OpConstant %10 1
)",
          R"(
%13 = OpLoad %3 %1
%11 = OpImageQuerySizeLod %12 %13 %14
%8 = OpVectorShuffle %9 %11 %11 0 1 1
)",
          R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kDimensionsDepthCubeArray:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 1 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeInt 32 1
%9 = OpTypeVector %10 3
%13 = OpConstant %10 0
)",
          R"(
%12 = OpLoad %3 %1
%11 = OpImageQuerySizeLod %9 %12 %13
%8 = OpVectorShuffle %9 %11 %11 0 1 1
)",
          R"(
OpCapability SampledCubeArray
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kDimensionsDepthCubeArrayLevel:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 1 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeInt 32 1
%9 = OpTypeVector %10 3
%13 = OpConstant %10 1
)",
          R"(
%12 = OpLoad %3 %1
%11 = OpImageQuerySizeLod %9 %12 %13
%8 = OpVectorShuffle %9 %11 %11 0 1 1
)",
          R"(
OpCapability SampledCubeArray
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kDimensionsStorageRO1d:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 1D 0 0 0 2 Rgba32f
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeInt 32 1
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageQuerySize %9 %10
)",
          R"(
OpCapability Image1D
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kDimensionsStorageRO2d:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 2 Rgba32f
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeInt 32 1
%9 = OpTypeVector %10 2
)",
          R"(
%11 = OpLoad %3 %1
%8 = OpImageQuerySize %9 %11
)",
          R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kDimensionsStorageRO2dArray:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 2 Rgba32f
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeInt 32 1
%9 = OpTypeVector %10 2
%12 = OpTypeVector %10 3
)",
          R"(
%13 = OpLoad %3 %1
%11 = OpImageQuerySize %12 %13
%8 = OpVectorShuffle %9 %11 %11 0 1
)",
          R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kDimensionsStorageRO3d:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 2 Rgba32f
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeInt 32 1
%9 = OpTypeVector %10 3
)",
          R"(
%11 = OpLoad %3 %1
%8 = OpImageQuerySize %9 %11
)",
          R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kDimensionsStorageWO1d:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 1D 0 0 0 2 Rgba32f
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeInt 32 1
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageQuerySize %9 %10
)",
          R"(
OpCapability Image1D
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kDimensionsStorageWO2d:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 2 Rgba32f
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeInt 32 1
%9 = OpTypeVector %10 2
)",
          R"(
%11 = OpLoad %3 %1
%8 = OpImageQuerySize %9 %11
)",
          R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kDimensionsStorageWO2dArray:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 2 Rgba32f
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeInt 32 1
%9 = OpTypeVector %10 2
%12 = OpTypeVector %10 3
)",
          R"(
%13 = OpLoad %3 %1
%11 = OpImageQuerySize %12 %13
%8 = OpVectorShuffle %9 %11 %11 0 1
)",
          R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kDimensionsStorageWO3d:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 2 Rgba32f
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeInt 32 1
%9 = OpTypeVector %10 3
)",
          R"(
%11 = OpLoad %3 %1
%8 = OpImageQuerySize %9 %11
)",
          R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kNumLayers2dArray:
      return {R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeInt 32 1
%11 = OpTypeVector %9 3
%13 = OpConstant %9 0
)",
              R"(
%12 = OpLoad %3 %1
%10 = OpImageQuerySizeLod %11 %12 %13
%8 = OpCompositeExtract %9 %10 2
)",
              R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kNumLayersCubeArray:
      return {R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeInt 32 1
%11 = OpTypeVector %9 3
%13 = OpConstant %9 0
)",
              R"(
%12 = OpLoad %3 %1
%10 = OpImageQuerySizeLod %11 %12 %13
%8 = OpCompositeExtract %9 %10 2
)",
              R"(
OpCapability SampledCubeArray
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kNumLayersDepth2dArray:
      return {R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeInt 32 1
%11 = OpTypeVector %9 3
%13 = OpConstant %9 0
)",
              R"(
%12 = OpLoad %3 %1
%10 = OpImageQuerySizeLod %11 %12 %13
%8 = OpCompositeExtract %9 %10 2
)",
              R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kNumLayersDepthCubeArray:
      return {R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 1 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeInt 32 1
%11 = OpTypeVector %9 3
%13 = OpConstant %9 0
)",
              R"(
%12 = OpLoad %3 %1
%10 = OpImageQuerySizeLod %11 %12 %13
%8 = OpCompositeExtract %9 %10 2
)",
              R"(
OpCapability SampledCubeArray
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kNumLayersStorageWO2dArray:
      return {R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 2 Rgba32f
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeInt 32 1
%11 = OpTypeVector %9 3
)",
              R"(
%12 = OpLoad %3 %1
%10 = OpImageQuerySize %11 %12
%8 = OpCompositeExtract %9 %10 2
)",
              R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kNumLevels2d:
      return {R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeInt 32 1
)",
              R"(
%10 = OpLoad %3 %1
%8 = OpImageQueryLevels %9 %10
)",
              R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kNumLevels2dArray:
      return {R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeInt 32 1
)",
              R"(
%10 = OpLoad %3 %1
%8 = OpImageQueryLevels %9 %10
)",
              R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kNumLevels3d:
      return {R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeInt 32 1
)",
              R"(
%10 = OpLoad %3 %1
%8 = OpImageQueryLevels %9 %10
)",
              R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kNumLevelsCube:
      return {R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeInt 32 1
)",
              R"(
%10 = OpLoad %3 %1
%8 = OpImageQueryLevels %9 %10
)",
              R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kNumLevelsCubeArray:
      return {R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeInt 32 1
)",
              R"(
%10 = OpLoad %3 %1
%8 = OpImageQueryLevels %9 %10
)",
              R"(
OpCapability SampledCubeArray
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kNumLevelsDepth2d:
      return {R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeInt 32 1
)",
              R"(
%10 = OpLoad %3 %1
%8 = OpImageQueryLevels %9 %10
)",
              R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kNumLevelsDepth2dArray:
      return {R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeInt 32 1
)",
              R"(
%10 = OpLoad %3 %1
%8 = OpImageQueryLevels %9 %10
)",
              R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kNumLevelsDepthCube:
      return {R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 1 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeInt 32 1
)",
              R"(
%10 = OpLoad %3 %1
%8 = OpImageQueryLevels %9 %10
)",
              R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kNumLevelsDepthCubeArray:
      return {R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 1 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeInt 32 1
)",
              R"(
%10 = OpLoad %3 %1
%8 = OpImageQueryLevels %9 %10
)",
              R"(
OpCapability SampledCubeArray
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kNumSamplesMultisampled2d:
      return {R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 1 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeInt 32 1
)",
              R"(
%10 = OpLoad %3 %1
%8 = OpImageQuerySamples %9 %10
)",
              R"(
OpCapability ImageQuery
)"};
    case ValidTextureOverload::kSample1dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 1D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpConstant %4 1
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleImplicitLod %9 %13 %14
)",
          R"(
OpCapability Sampled1D
)"};
    case ValidTextureOverload::kSample2dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 2
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstantComposite %14 %15 %16
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleImplicitLod %9 %13 %17
)",
          R"(
)"};
    case ValidTextureOverload::kSample2dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 2
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstantComposite %14 %15 %16
%19 = OpTypeInt 32 1
%18 = OpTypeVector %19 2
%20 = OpConstant %19 3
%21 = OpConstant %19 4
%22 = OpConstantComposite %18 %20 %21
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleImplicitLod %9 %13 %17 ConstOffset %22
)",
          R"(
)"};
    case ValidTextureOverload::kSample2dArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%18 = OpTypeInt 32 1
%19 = OpConstant %18 3
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%17 = OpConvertSToF %4 %19
%20 = OpCompositeConstruct %14 %15 %16 %17
%8 = OpImageSampleImplicitLod %9 %13 %20
)",
          R"(
)"};
    case ValidTextureOverload::kSample2dArrayOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%18 = OpTypeInt 32 1
%19 = OpConstant %18 3
%21 = OpTypeVector %18 2
%22 = OpConstant %18 4
%23 = OpConstant %18 5
%24 = OpConstantComposite %21 %22 %23
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%17 = OpConvertSToF %4 %19
%20 = OpCompositeConstruct %14 %15 %16 %17
%8 = OpImageSampleImplicitLod %9 %13 %20 ConstOffset %24
)",
          R"(
)"};
    case ValidTextureOverload::kSample3dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleImplicitLod %9 %13 %18
)",
          R"(
)"};
    case ValidTextureOverload::kSample3dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
%20 = OpTypeInt 32 1
%19 = OpTypeVector %20 3
%21 = OpConstant %20 4
%22 = OpConstant %20 5
%23 = OpConstant %20 6
%24 = OpConstantComposite %19 %21 %22 %23
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleImplicitLod %9 %13 %18 ConstOffset %24
)",
          R"(
)"};
    case ValidTextureOverload::kSampleCubeF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleImplicitLod %9 %13 %18
)",
          R"(
)"};
    case ValidTextureOverload::kSampleCubeArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstant %4 3
%18 = OpTypeInt 32 1
%19 = OpConstant %18 4
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%17 = OpConvertSToF %4 %19
%20 = OpCompositeConstruct %9 %14 %15 %16 %17
%8 = OpImageSampleImplicitLod %9 %13 %20
)",
          R"(
OpCapability SampledCubeArray
)"};
    case ValidTextureOverload::kSampleDepth2dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeVector %4 4
%13 = OpTypeSampledImage %3
%15 = OpTypeVector %4 2
%16 = OpConstant %4 1
%17 = OpConstant %4 2
%18 = OpConstantComposite %15 %16 %17
)",
          R"(
%11 = OpLoad %7 %5
%12 = OpLoad %3 %1
%14 = OpSampledImage %13 %12 %11
%9 = OpImageSampleImplicitLod %10 %14 %18
%8 = OpCompositeExtract %4 %9 0
)",
          R"(
)"};
    case ValidTextureOverload::kSampleDepth2dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeVector %4 4
%13 = OpTypeSampledImage %3
%15 = OpTypeVector %4 2
%16 = OpConstant %4 1
%17 = OpConstant %4 2
%18 = OpConstantComposite %15 %16 %17
%20 = OpTypeInt 32 1
%19 = OpTypeVector %20 2
%21 = OpConstant %20 3
%22 = OpConstant %20 4
%23 = OpConstantComposite %19 %21 %22
)",
          R"(
%11 = OpLoad %7 %5
%12 = OpLoad %3 %1
%14 = OpSampledImage %13 %12 %11
%9 = OpImageSampleImplicitLod %10 %14 %18 ConstOffset %23
%8 = OpCompositeExtract %4 %9 0
)",
          R"(
)"};
    case ValidTextureOverload::kSampleDepth2dArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeVector %4 4
%13 = OpTypeSampledImage %3
%15 = OpTypeVector %4 3
%16 = OpConstant %4 1
%17 = OpConstant %4 2
%19 = OpTypeInt 32 1
%20 = OpConstant %19 3
)",
          R"(
%11 = OpLoad %7 %5
%12 = OpLoad %3 %1
%14 = OpSampledImage %13 %12 %11
%18 = OpConvertSToF %4 %20
%21 = OpCompositeConstruct %15 %16 %17 %18
%9 = OpImageSampleImplicitLod %10 %14 %21
%8 = OpCompositeExtract %4 %9 0
)",
          R"(
)"};
    case ValidTextureOverload::kSampleDepth2dArrayOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeVector %4 4
%13 = OpTypeSampledImage %3
%15 = OpTypeVector %4 3
%16 = OpConstant %4 1
%17 = OpConstant %4 2
%19 = OpTypeInt 32 1
%20 = OpConstant %19 3
%22 = OpTypeVector %19 2
%23 = OpConstant %19 4
%24 = OpConstant %19 5
%25 = OpConstantComposite %22 %23 %24
)",
          R"(
%11 = OpLoad %7 %5
%12 = OpLoad %3 %1
%14 = OpSampledImage %13 %12 %11
%18 = OpConvertSToF %4 %20
%21 = OpCompositeConstruct %15 %16 %17 %18
%9 = OpImageSampleImplicitLod %10 %14 %21 ConstOffset %25
%8 = OpCompositeExtract %4 %9 0
)",
          R"(
)"};
    case ValidTextureOverload::kSampleDepthCubeF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 1 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeVector %4 4
%13 = OpTypeSampledImage %3
%15 = OpTypeVector %4 3
%16 = OpConstant %4 1
%17 = OpConstant %4 2
%18 = OpConstant %4 3
%19 = OpConstantComposite %15 %16 %17 %18
)",
          R"(
%11 = OpLoad %7 %5
%12 = OpLoad %3 %1
%14 = OpSampledImage %13 %12 %11
%9 = OpImageSampleImplicitLod %10 %14 %19
%8 = OpCompositeExtract %4 %9 0
)",
          R"(
)"};
    case ValidTextureOverload::kSampleDepthCubeArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 1 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeVector %4 4
%13 = OpTypeSampledImage %3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%19 = OpTypeInt 32 1
%20 = OpConstant %19 4
)",
          R"(
%11 = OpLoad %7 %5
%12 = OpLoad %3 %1
%14 = OpSampledImage %13 %12 %11
%18 = OpConvertSToF %4 %20
%21 = OpCompositeConstruct %10 %15 %16 %17 %18
%9 = OpImageSampleImplicitLod %10 %14 %21
%8 = OpCompositeExtract %4 %9 0
)",
          R"(
OpCapability SampledCubeArray
)"};
    case ValidTextureOverload::kSampleBias2dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 2
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstantComposite %14 %15 %16
%18 = OpConstant %4 3
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleImplicitLod %9 %13 %17 Bias %18
)",
          R"(
)"};
    case ValidTextureOverload::kSampleBias2dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 2
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstantComposite %14 %15 %16
%18 = OpConstant %4 3
%20 = OpTypeInt 32 1
%19 = OpTypeVector %20 2
%21 = OpConstant %20 4
%22 = OpConstant %20 5
%23 = OpConstantComposite %19 %21 %22
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleImplicitLod %9 %13 %17 Bias|ConstOffset %18 %23
)",
          R"(
)"};
    case ValidTextureOverload::kSampleBias2dArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%18 = OpTypeInt 32 1
%19 = OpConstant %18 4
%21 = OpConstant %4 3
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%17 = OpConvertSToF %4 %19
%20 = OpCompositeConstruct %14 %15 %16 %17
%8 = OpImageSampleImplicitLod %9 %13 %20 Bias %21
)",
          R"(
)"};
    case ValidTextureOverload::kSampleBias2dArrayOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%18 = OpTypeInt 32 1
%19 = OpConstant %18 3
%21 = OpConstant %4 4
%22 = OpTypeVector %18 2
%23 = OpConstant %18 5
%24 = OpConstant %18 6
%25 = OpConstantComposite %22 %23 %24
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%17 = OpConvertSToF %4 %19
%20 = OpCompositeConstruct %14 %15 %16 %17
%8 = OpImageSampleImplicitLod %9 %13 %20 Bias|ConstOffset %21 %25
)",
          R"(
)"};
    case ValidTextureOverload::kSampleBias3dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
%19 = OpConstant %4 4
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleImplicitLod %9 %13 %18 Bias %19
)",
          R"(
)"};
    case ValidTextureOverload::kSampleBias3dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
%19 = OpConstant %4 4
%21 = OpTypeInt 32 1
%20 = OpTypeVector %21 3
%22 = OpConstant %21 5
%23 = OpConstant %21 6
%24 = OpConstant %21 7
%25 = OpConstantComposite %20 %22 %23 %24
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleImplicitLod %9 %13 %18 Bias|ConstOffset %19 %25
)",
          R"(
)"};
    case ValidTextureOverload::kSampleBiasCubeF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
%19 = OpConstant %4 4
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleImplicitLod %9 %13 %18 Bias %19
)",
          R"(
)"};
    case ValidTextureOverload::kSampleBiasCubeArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstant %4 3
%18 = OpTypeInt 32 1
%19 = OpConstant %18 3
%21 = OpConstant %4 4
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%17 = OpConvertSToF %4 %19
%20 = OpCompositeConstruct %9 %14 %15 %16 %17
%8 = OpImageSampleImplicitLod %9 %13 %20 Bias %21
)",
          R"(
OpCapability SampledCubeArray
)"};
    case ValidTextureOverload::kSampleLevel2dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 2
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstantComposite %14 %15 %16
%18 = OpConstant %4 3
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleExplicitLod %9 %13 %17 Lod %18
)",
          R"(
)"};
    case ValidTextureOverload::kSampleLevel2dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 2
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstantComposite %14 %15 %16
%18 = OpConstant %4 3
%20 = OpTypeInt 32 1
%19 = OpTypeVector %20 2
%21 = OpConstant %20 4
%22 = OpConstant %20 5
%23 = OpConstantComposite %19 %21 %22
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleExplicitLod %9 %13 %17 Lod|ConstOffset %18 %23
)",
          R"(
)"};
    case ValidTextureOverload::kSampleLevel2dArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%18 = OpTypeInt 32 1
%19 = OpConstant %18 3
%21 = OpConstant %4 4
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%17 = OpConvertSToF %4 %19
%20 = OpCompositeConstruct %14 %15 %16 %17
%8 = OpImageSampleExplicitLod %9 %13 %20 Lod %21
)",
          R"(
)"};
    case ValidTextureOverload::kSampleLevel2dArrayOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%18 = OpTypeInt 32 1
%19 = OpConstant %18 3
%21 = OpConstant %4 4
%22 = OpTypeVector %18 2
%23 = OpConstant %18 5
%24 = OpConstant %18 6
%25 = OpConstantComposite %22 %23 %24
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%17 = OpConvertSToF %4 %19
%20 = OpCompositeConstruct %14 %15 %16 %17
%8 = OpImageSampleExplicitLod %9 %13 %20 Lod|ConstOffset %21 %25
)",
          R"(
)"};
    case ValidTextureOverload::kSampleLevel3dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
%19 = OpConstant %4 4
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleExplicitLod %9 %13 %18 Lod %19
)",
          R"(
)"};
    case ValidTextureOverload::kSampleLevel3dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
%19 = OpConstant %4 4
%21 = OpTypeInt 32 1
%20 = OpTypeVector %21 3
%22 = OpConstant %21 5
%23 = OpConstant %21 6
%24 = OpConstant %21 7
%25 = OpConstantComposite %20 %22 %23 %24
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleExplicitLod %9 %13 %18 Lod|ConstOffset %19 %25
)",
          R"(
)"};
    case ValidTextureOverload::kSampleLevelCubeF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
%19 = OpConstant %4 4
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleExplicitLod %9 %13 %18 Lod %19
)",
          R"(
)"};
    case ValidTextureOverload::kSampleLevelCubeArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstant %4 3
%18 = OpTypeInt 32 1
%19 = OpConstant %18 4
%21 = OpConstant %4 5
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%17 = OpConvertSToF %4 %19
%20 = OpCompositeConstruct %9 %14 %15 %16 %17
%8 = OpImageSampleExplicitLod %9 %13 %20 Lod %21
)",
          R"(
OpCapability SampledCubeArray
)"};
    case ValidTextureOverload::kSampleLevelDepth2dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeVector %4 4
%13 = OpTypeSampledImage %3
%15 = OpTypeVector %4 2
%16 = OpConstant %4 1
%17 = OpConstant %4 2
%18 = OpConstantComposite %15 %16 %17
%20 = OpTypeInt 32 1
%21 = OpConstant %20 3
)",
          R"(
%11 = OpLoad %7 %5
%12 = OpLoad %3 %1
%14 = OpSampledImage %13 %12 %11
%19 = OpConvertSToF %4 %21
%9 = OpImageSampleExplicitLod %10 %14 %18 Lod %19
%8 = OpCompositeExtract %4 %9 0
)",
          R"(
)"};
    case ValidTextureOverload::kSampleLevelDepth2dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeVector %4 4
%13 = OpTypeSampledImage %3
%15 = OpTypeVector %4 2
%16 = OpConstant %4 1
%17 = OpConstant %4 2
%18 = OpConstantComposite %15 %16 %17
%20 = OpTypeInt 32 1
%21 = OpConstant %20 3
%22 = OpTypeVector %20 2
%23 = OpConstant %20 4
%24 = OpConstant %20 5
%25 = OpConstantComposite %22 %23 %24
)",
          R"(
%11 = OpLoad %7 %5
%12 = OpLoad %3 %1
%14 = OpSampledImage %13 %12 %11
%19 = OpConvertSToF %4 %21
%9 = OpImageSampleExplicitLod %10 %14 %18 Lod|ConstOffset %19 %25
%8 = OpCompositeExtract %4 %9 0
)",
          R"(
)"};
    case ValidTextureOverload::kSampleLevelDepth2dArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeVector %4 4
%13 = OpTypeSampledImage %3
%15 = OpTypeVector %4 3
%16 = OpConstant %4 1
%17 = OpConstant %4 2
%19 = OpTypeInt 32 1
%20 = OpConstant %19 3
%23 = OpConstant %19 4
)",
          R"(
%11 = OpLoad %7 %5
%12 = OpLoad %3 %1
%14 = OpSampledImage %13 %12 %11
%18 = OpConvertSToF %4 %20
%21 = OpCompositeConstruct %15 %16 %17 %18
%22 = OpConvertSToF %4 %23
%9 = OpImageSampleExplicitLod %10 %14 %21 Lod %22
%8 = OpCompositeExtract %4 %9 0
)",
          R"(
)"};
    case ValidTextureOverload::kSampleLevelDepth2dArrayOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeVector %4 4
%13 = OpTypeSampledImage %3
%15 = OpTypeVector %4 3
%16 = OpConstant %4 1
%17 = OpConstant %4 2
%19 = OpTypeInt 32 1
%20 = OpConstant %19 3
%23 = OpConstant %19 4
%24 = OpTypeVector %19 2
%25 = OpConstant %19 5
%26 = OpConstant %19 6
%27 = OpConstantComposite %24 %25 %26
)",
          R"(
%11 = OpLoad %7 %5
%12 = OpLoad %3 %1
%14 = OpSampledImage %13 %12 %11
%18 = OpConvertSToF %4 %20
%21 = OpCompositeConstruct %15 %16 %17 %18
%22 = OpConvertSToF %4 %23
%9 = OpImageSampleExplicitLod %10 %14 %21 Lod|ConstOffset %22 %27
%8 = OpCompositeExtract %4 %9 0
)",
          R"(
)"};
    case ValidTextureOverload::kSampleLevelDepthCubeF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 1 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeVector %4 4
%13 = OpTypeSampledImage %3
%15 = OpTypeVector %4 3
%16 = OpConstant %4 1
%17 = OpConstant %4 2
%18 = OpConstant %4 3
%19 = OpConstantComposite %15 %16 %17 %18
%21 = OpTypeInt 32 1
%22 = OpConstant %21 4
)",
          R"(
%11 = OpLoad %7 %5
%12 = OpLoad %3 %1
%14 = OpSampledImage %13 %12 %11
%20 = OpConvertSToF %4 %22
%9 = OpImageSampleExplicitLod %10 %14 %19 Lod %20
%8 = OpCompositeExtract %4 %9 0
)",
          R"(
)"};
    case ValidTextureOverload::kSampleLevelDepthCubeArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 1 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeVector %4 4
%13 = OpTypeSampledImage %3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%19 = OpTypeInt 32 1
%20 = OpConstant %19 4
%23 = OpConstant %19 5
)",
          R"(
%11 = OpLoad %7 %5
%12 = OpLoad %3 %1
%14 = OpSampledImage %13 %12 %11
%18 = OpConvertSToF %4 %20
%21 = OpCompositeConstruct %10 %15 %16 %17 %18
%22 = OpConvertSToF %4 %23
%9 = OpImageSampleExplicitLod %10 %14 %21 Lod %22
%8 = OpCompositeExtract %4 %9 0
)",
          R"(
OpCapability SampledCubeArray
)"};
    case ValidTextureOverload::kSampleGrad2dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 2
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstantComposite %14 %15 %16
%18 = OpConstant %4 3
%19 = OpConstant %4 4
%20 = OpConstantComposite %14 %18 %19
%21 = OpConstant %4 5
%22 = OpConstant %4 6
%23 = OpConstantComposite %14 %21 %22
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleExplicitLod %9 %13 %17 Grad %20 %23
)",
          R"(
)"};
    case ValidTextureOverload::kSampleGrad2dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 2
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstantComposite %14 %15 %16
%18 = OpConstant %4 3
%19 = OpConstant %4 4
%20 = OpConstantComposite %14 %18 %19
%21 = OpConstant %4 5
%22 = OpConstant %4 6
%23 = OpConstantComposite %14 %21 %22
%25 = OpTypeInt 32 1
%24 = OpTypeVector %25 2
%26 = OpConstant %25 7
%27 = OpConstant %25 8
%28 = OpConstantComposite %24 %26 %27
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleExplicitLod %9 %13 %17 Grad|ConstOffset %20 %23 %28
)",
          R"(
)"};
    case ValidTextureOverload::kSampleGrad2dArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%18 = OpTypeInt 32 1
%19 = OpConstant %18 3
%21 = OpTypeVector %4 2
%22 = OpConstant %4 4
%23 = OpConstant %4 5
%24 = OpConstantComposite %21 %22 %23
%25 = OpConstant %4 6
%26 = OpConstant %4 7
%27 = OpConstantComposite %21 %25 %26
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%17 = OpConvertSToF %4 %19
%20 = OpCompositeConstruct %14 %15 %16 %17
%8 = OpImageSampleExplicitLod %9 %13 %20 Grad %24 %27
)",
          R"(
)"};
    case ValidTextureOverload::kSampleGrad2dArrayOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%18 = OpTypeInt 32 1
%19 = OpConstant %18 3
%21 = OpTypeVector %4 2
%22 = OpConstant %4 4
%23 = OpConstant %4 5
%24 = OpConstantComposite %21 %22 %23
%25 = OpConstant %4 6
%26 = OpConstant %4 7
%27 = OpConstantComposite %21 %25 %26
%28 = OpTypeVector %18 2
%29 = OpConstant %18 8
%30 = OpConstant %18 9
%31 = OpConstantComposite %28 %29 %30
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%17 = OpConvertSToF %4 %19
%20 = OpCompositeConstruct %14 %15 %16 %17
%8 = OpImageSampleExplicitLod %9 %13 %20 Grad|ConstOffset %24 %27 %31
)",
          R"(
)"};
    case ValidTextureOverload::kSampleGrad3dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
%19 = OpConstant %4 4
%20 = OpConstant %4 5
%21 = OpConstant %4 6
%22 = OpConstantComposite %14 %19 %20 %21
%23 = OpConstant %4 7
%24 = OpConstant %4 8
%25 = OpConstant %4 9
%26 = OpConstantComposite %14 %23 %24 %25
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleExplicitLod %9 %13 %18 Grad %22 %26
)",
          R"(
)"};
    case ValidTextureOverload::kSampleGrad3dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
%19 = OpConstant %4 4
%20 = OpConstant %4 5
%21 = OpConstant %4 6
%22 = OpConstantComposite %14 %19 %20 %21
%23 = OpConstant %4 7
%24 = OpConstant %4 8
%25 = OpConstant %4 9
%26 = OpConstantComposite %14 %23 %24 %25
%28 = OpTypeInt 32 1
%27 = OpTypeVector %28 3
%29 = OpConstant %28 10
%30 = OpConstant %28 11
%31 = OpConstant %28 12
%32 = OpConstantComposite %27 %29 %30 %31
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleExplicitLod %9 %13 %18 Grad|ConstOffset %22 %26 %32
)",
          R"(
)"};
    case ValidTextureOverload::kSampleGradCubeF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
%19 = OpConstant %4 4
%20 = OpConstant %4 5
%21 = OpConstant %4 6
%22 = OpConstantComposite %14 %19 %20 %21
%23 = OpConstant %4 7
%24 = OpConstant %4 8
%25 = OpConstant %4 9
%26 = OpConstantComposite %14 %23 %24 %25
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleExplicitLod %9 %13 %18 Grad %22 %26
)",
          R"(
)"};
    case ValidTextureOverload::kSampleGradCubeArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstant %4 3
%18 = OpTypeInt 32 1
%19 = OpConstant %18 4
%21 = OpTypeVector %4 3
%22 = OpConstant %4 5
%23 = OpConstant %4 6
%24 = OpConstant %4 7
%25 = OpConstantComposite %21 %22 %23 %24
%26 = OpConstant %4 8
%27 = OpConstant %4 9
%28 = OpConstant %4 10
%29 = OpConstantComposite %21 %26 %27 %28
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%17 = OpConvertSToF %4 %19
%20 = OpCompositeConstruct %9 %14 %15 %16 %17
%8 = OpImageSampleExplicitLod %9 %13 %20 Grad %25 %29
)",
          R"(
OpCapability SampledCubeArray
)"};
    case ValidTextureOverload::kSampleCompareDepth2dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 2
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstantComposite %13 %14 %15
%17 = OpConstant %4 3
%18 = OpConstant %4 0
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%8 = OpImageSampleDrefExplicitLod %4 %12 %16 %17 Lod %18
)",
          R"(
)"};
    case ValidTextureOverload::kSampleCompareDepth2dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 2
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstantComposite %13 %14 %15
%17 = OpConstant %4 3
%18 = OpConstant %4 0
%20 = OpTypeInt 32 1
%19 = OpTypeVector %20 2
%21 = OpConstant %20 4
%22 = OpConstant %20 5
%23 = OpConstantComposite %19 %21 %22
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%8 = OpImageSampleDrefExplicitLod %4 %12 %16 %17 Lod|ConstOffset %18 %23
)",
          R"(
)"};
    case ValidTextureOverload::kSampleCompareDepth2dArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 3
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%17 = OpTypeInt 32 1
%18 = OpConstant %17 4
%20 = OpConstant %4 3
%21 = OpConstant %4 0
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%16 = OpConvertSToF %4 %18
%19 = OpCompositeConstruct %13 %14 %15 %16
%8 = OpImageSampleDrefExplicitLod %4 %12 %19 %20 Lod %21
)",
          R"(
)"};
    case ValidTextureOverload::kSampleCompareDepth2dArrayOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 3
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%17 = OpTypeInt 32 1
%18 = OpConstant %17 4
%20 = OpConstant %4 3
%21 = OpConstant %4 0
%22 = OpTypeVector %17 2
%23 = OpConstant %17 5
%24 = OpConstant %17 6
%25 = OpConstantComposite %22 %23 %24
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%16 = OpConvertSToF %4 %18
%19 = OpCompositeConstruct %13 %14 %15 %16
%8 = OpImageSampleDrefExplicitLod %4 %12 %19 %20 Lod|ConstOffset %21 %25
)",
          R"(
)"};
    case ValidTextureOverload::kSampleCompareDepthCubeF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 1 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 3
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstant %4 3
%17 = OpConstantComposite %13 %14 %15 %16
%18 = OpConstant %4 4
%19 = OpConstant %4 0
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%8 = OpImageSampleDrefExplicitLod %4 %12 %17 %18 Lod %19
)",
          R"(
)"};
    case ValidTextureOverload::kSampleCompareDepthCubeArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 1 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 4
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstant %4 3
%18 = OpTypeInt 32 1
%19 = OpConstant %18 4
%21 = OpConstant %4 5
%22 = OpConstant %4 0
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%17 = OpConvertSToF %4 %19
%20 = OpCompositeConstruct %13 %14 %15 %16 %17
%8 = OpImageSampleDrefExplicitLod %4 %12 %20 %21 Lod %22
)",
          R"(
OpCapability SampledCubeArray
)"};
    case ValidTextureOverload::kLoad1dLevelF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 1D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%11 = OpTypeInt 32 1
%12 = OpConstant %11 1
%13 = OpConstant %11 3
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageFetch %9 %10 %12 Lod %13
)",
          R"(
OpCapability Sampled1D
)"};
    case ValidTextureOverload::kLoad1dLevelU32:
      return {
          R"(
%4 = OpTypeInt 32 0
%3 = OpTypeImage %4 1D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%11 = OpTypeInt 32 1
%12 = OpConstant %11 1
%13 = OpConstant %11 3
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageFetch %9 %10 %12 Lod %13
)",
          R"(
OpCapability Sampled1D
)"};
    case ValidTextureOverload::kLoad1dLevelI32:
      return {
          R"(
%4 = OpTypeInt 32 1
%3 = OpTypeImage %4 1D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%11 = OpConstant %4 1
%12 = OpConstant %4 3
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageFetch %9 %10 %11 Lod %12
)",
          R"(
OpCapability Sampled1D
)"};
    case ValidTextureOverload::kLoad2dLevelF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeInt 32 1
%11 = OpTypeVector %12 2
%13 = OpConstant %12 1
%14 = OpConstant %12 2
%15 = OpConstantComposite %11 %13 %14
%16 = OpConstant %12 3
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageFetch %9 %10 %15 Lod %16
)",
          R"(
)"};
    case ValidTextureOverload::kLoad2dLevelU32:
      return {
          R"(
%4 = OpTypeInt 32 0
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeInt 32 1
%11 = OpTypeVector %12 2
%13 = OpConstant %12 1
%14 = OpConstant %12 2
%15 = OpConstantComposite %11 %13 %14
%16 = OpConstant %12 3
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageFetch %9 %10 %15 Lod %16
)",
          R"(
)"};
    case ValidTextureOverload::kLoad2dLevelI32:
      return {
          R"(
%4 = OpTypeInt 32 1
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%11 = OpTypeVector %4 2
%12 = OpConstant %4 1
%13 = OpConstant %4 2
%14 = OpConstantComposite %11 %12 %13
%15 = OpConstant %4 3
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageFetch %9 %10 %14 Lod %15
)",
          R"(
)"};
    case ValidTextureOverload::kLoad2dArrayLevelF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeInt 32 1
%11 = OpTypeVector %12 3
%13 = OpConstant %12 1
%14 = OpConstant %12 2
%15 = OpConstant %12 3
%16 = OpConstantComposite %11 %13 %14 %15
%17 = OpConstant %12 4
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageFetch %9 %10 %16 Lod %17
)",
          R"(
)"};
    case ValidTextureOverload::kLoad2dArrayLevelU32:
      return {
          R"(
%4 = OpTypeInt 32 0
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeInt 32 1
%11 = OpTypeVector %12 3
%13 = OpConstant %12 1
%14 = OpConstant %12 2
%15 = OpConstant %12 3
%16 = OpConstantComposite %11 %13 %14 %15
%17 = OpConstant %12 4
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageFetch %9 %10 %16 Lod %17
)",
          R"(
)"};
    case ValidTextureOverload::kLoad2dArrayLevelI32:
      return {
          R"(
%4 = OpTypeInt 32 1
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%11 = OpTypeVector %4 3
%12 = OpConstant %4 1
%13 = OpConstant %4 2
%14 = OpConstant %4 3
%15 = OpConstantComposite %11 %12 %13 %14
%16 = OpConstant %4 4
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageFetch %9 %10 %15 Lod %16
)",
          R"(
)"};
    case ValidTextureOverload::kLoad3dLevelF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeInt 32 1
%11 = OpTypeVector %12 3
%13 = OpConstant %12 1
%14 = OpConstant %12 2
%15 = OpConstant %12 3
%16 = OpConstantComposite %11 %13 %14 %15
%17 = OpConstant %12 4
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageFetch %9 %10 %16 Lod %17
)",
          R"(
)"};
    case ValidTextureOverload::kLoad3dLevelU32:
      return {
          R"(
%4 = OpTypeInt 32 0
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeInt 32 1
%11 = OpTypeVector %12 3
%13 = OpConstant %12 1
%14 = OpConstant %12 2
%15 = OpConstant %12 3
%16 = OpConstantComposite %11 %13 %14 %15
%17 = OpConstant %12 4
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageFetch %9 %10 %16 Lod %17
)",
          R"(
)"};
    case ValidTextureOverload::kLoad3dLevelI32:
      return {
          R"(
%4 = OpTypeInt 32 1
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%11 = OpTypeVector %4 3
%12 = OpConstant %4 1
%13 = OpConstant %4 2
%14 = OpConstant %4 3
%15 = OpConstantComposite %11 %12 %13 %14
%16 = OpConstant %4 4
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageFetch %9 %10 %15 Lod %16
)",
          R"(
)"};
    case ValidTextureOverload::kLoadMultisampled2dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 1 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeInt 32 1
%11 = OpTypeVector %12 2
%13 = OpConstant %12 1
%14 = OpConstant %12 2
%15 = OpConstantComposite %11 %13 %14
%16 = OpConstant %12 3
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageFetch %9 %10 %15 Sample %16
)",
          R"(
)"};
    case ValidTextureOverload::kLoadMultisampled2dU32:
      return {
          R"(
%4 = OpTypeInt 32 0
%3 = OpTypeImage %4 2D 0 0 1 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeInt 32 1
%11 = OpTypeVector %12 2
%13 = OpConstant %12 1
%14 = OpConstant %12 2
%15 = OpConstantComposite %11 %13 %14
%16 = OpConstant %12 3
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageFetch %9 %10 %15 Sample %16
)",
          R"(
)"};
    case ValidTextureOverload::kLoadMultisampled2dI32:
      return {
          R"(
%4 = OpTypeInt 32 1
%3 = OpTypeImage %4 2D 0 0 1 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%11 = OpTypeVector %4 2
%12 = OpConstant %4 1
%13 = OpConstant %4 2
%14 = OpConstantComposite %11 %12 %13
%15 = OpConstant %4 3
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageFetch %9 %10 %14 Sample %15
)",
          R"(
)"};
    case ValidTextureOverload::kLoadDepth2dLevelF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeVector %4 4
%13 = OpTypeInt 32 1
%12 = OpTypeVector %13 2
%14 = OpConstant %13 1
%15 = OpConstant %13 2
%16 = OpConstantComposite %12 %14 %15
%17 = OpConstant %13 3
)",
          R"(
%11 = OpLoad %3 %1
%9 = OpImageFetch %10 %11 %16 Lod %17
%8 = OpCompositeExtract %4 %9 0
)",
          R"(
)"};
    case ValidTextureOverload::kLoadDepth2dArrayLevelF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 1 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%10 = OpTypeVector %4 4
%13 = OpTypeInt 32 1
%12 = OpTypeVector %13 3
%14 = OpConstant %13 1
%15 = OpConstant %13 2
%16 = OpConstant %13 3
%17 = OpConstantComposite %12 %14 %15 %16
%18 = OpConstant %13 4
)",
          R"(
%11 = OpLoad %3 %1
%9 = OpImageFetch %10 %11 %17 Lod %18
%8 = OpCompositeExtract %4 %9 0
)",
          R"(
)"};
    case ValidTextureOverload::kLoadStorageRO1dRgba32float:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 1D 0 0 0 2 Rgba32f
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%11 = OpTypeInt 32 1
%12 = OpConstant %11 1
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageRead %9 %10 %12
)",
          R"(
OpCapability Image1D
)"};
    case ValidTextureOverload::kLoadStorageRO2dRgba8unorm:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 2 Rgba8
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeInt 32 1
%11 = OpTypeVector %12 2
%13 = OpConstant %12 1
%14 = OpConstant %12 2
%15 = OpConstantComposite %11 %13 %14
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageRead %9 %10 %15
)",
          R"(
)"};
    case ValidTextureOverload::kLoadStorageRO2dRgba8snorm:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 2 Rgba8Snorm
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeInt 32 1
%11 = OpTypeVector %12 2
%13 = OpConstant %12 1
%14 = OpConstant %12 2
%15 = OpConstantComposite %11 %13 %14
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageRead %9 %10 %15
)",
          R"(
)"};
    case ValidTextureOverload::kLoadStorageRO2dRgba8uint:
      return {
          R"(
%4 = OpTypeInt 32 0
%3 = OpTypeImage %4 2D 0 0 0 2 Rgba8ui
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeInt 32 1
%11 = OpTypeVector %12 2
%13 = OpConstant %12 1
%14 = OpConstant %12 2
%15 = OpConstantComposite %11 %13 %14
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageRead %9 %10 %15
)",
          R"(
)"};
    case ValidTextureOverload::kLoadStorageRO2dRgba8sint:
      return {
          R"(
%4 = OpTypeInt 32 1
%3 = OpTypeImage %4 2D 0 0 0 2 Rgba8i
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%11 = OpTypeVector %4 2
%12 = OpConstant %4 1
%13 = OpConstant %4 2
%14 = OpConstantComposite %11 %12 %13
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageRead %9 %10 %14
)",
          R"(
)"};
    case ValidTextureOverload::kLoadStorageRO2dRgba16uint:
      return {
          R"(
%4 = OpTypeInt 32 0
%3 = OpTypeImage %4 2D 0 0 0 2 Rgba16ui
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeInt 32 1
%11 = OpTypeVector %12 2
%13 = OpConstant %12 1
%14 = OpConstant %12 2
%15 = OpConstantComposite %11 %13 %14
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageRead %9 %10 %15
)",
          R"(
)"};
    case ValidTextureOverload::kLoadStorageRO2dRgba16sint:
      return {
          R"(
%4 = OpTypeInt 32 1
%3 = OpTypeImage %4 2D 0 0 0 2 Rgba16i
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%11 = OpTypeVector %4 2
%12 = OpConstant %4 1
%13 = OpConstant %4 2
%14 = OpConstantComposite %11 %12 %13
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageRead %9 %10 %14
)",
          R"(
)"};
    case ValidTextureOverload::kLoadStorageRO2dRgba16float:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 2 Rgba16f
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeInt 32 1
%11 = OpTypeVector %12 2
%13 = OpConstant %12 1
%14 = OpConstant %12 2
%15 = OpConstantComposite %11 %13 %14
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageRead %9 %10 %15
)",
          R"(
)"};
    case ValidTextureOverload::kLoadStorageRO2dR32uint:
      return {
          R"(
%4 = OpTypeInt 32 0
%3 = OpTypeImage %4 2D 0 0 0 2 R32ui
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeInt 32 1
%11 = OpTypeVector %12 2
%13 = OpConstant %12 1
%14 = OpConstant %12 2
%15 = OpConstantComposite %11 %13 %14
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageRead %9 %10 %15
)",
          R"(
)"};
    case ValidTextureOverload::kLoadStorageRO2dR32sint:
      return {
          R"(
%4 = OpTypeInt 32 1
%3 = OpTypeImage %4 2D 0 0 0 2 R32i
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%11 = OpTypeVector %4 2
%12 = OpConstant %4 1
%13 = OpConstant %4 2
%14 = OpConstantComposite %11 %12 %13
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageRead %9 %10 %14
)",
          R"(
)"};
    case ValidTextureOverload::kLoadStorageRO2dR32float:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 2 R32f
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeInt 32 1
%11 = OpTypeVector %12 2
%13 = OpConstant %12 1
%14 = OpConstant %12 2
%15 = OpConstantComposite %11 %13 %14
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageRead %9 %10 %15
)",
          R"(
)"};
    case ValidTextureOverload::kLoadStorageRO2dRg32uint:
      return {
          R"(
%4 = OpTypeInt 32 0
%3 = OpTypeImage %4 2D 0 0 0 2 Rg32ui
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeInt 32 1
%11 = OpTypeVector %12 2
%13 = OpConstant %12 1
%14 = OpConstant %12 2
%15 = OpConstantComposite %11 %13 %14
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageRead %9 %10 %15
)",
          R"(
OpCapability StorageImageExtendedFormats
)"};
    case ValidTextureOverload::kLoadStorageRO2dRg32sint:
      return {
          R"(
%4 = OpTypeInt 32 1
%3 = OpTypeImage %4 2D 0 0 0 2 Rg32i
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%11 = OpTypeVector %4 2
%12 = OpConstant %4 1
%13 = OpConstant %4 2
%14 = OpConstantComposite %11 %12 %13
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageRead %9 %10 %14
)",
          R"(
OpCapability StorageImageExtendedFormats
)"};
    case ValidTextureOverload::kLoadStorageRO2dRg32float:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 2 Rg32f
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeInt 32 1
%11 = OpTypeVector %12 2
%13 = OpConstant %12 1
%14 = OpConstant %12 2
%15 = OpConstantComposite %11 %13 %14
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageRead %9 %10 %15
)",
          R"(
OpCapability StorageImageExtendedFormats
)"};
    case ValidTextureOverload::kLoadStorageRO2dRgba32uint:
      return {
          R"(
%4 = OpTypeInt 32 0
%3 = OpTypeImage %4 2D 0 0 0 2 Rgba32ui
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeInt 32 1
%11 = OpTypeVector %12 2
%13 = OpConstant %12 1
%14 = OpConstant %12 2
%15 = OpConstantComposite %11 %13 %14
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageRead %9 %10 %15
)",
          R"(
)"};
    case ValidTextureOverload::kLoadStorageRO2dRgba32sint:
      return {
          R"(
%4 = OpTypeInt 32 1
%3 = OpTypeImage %4 2D 0 0 0 2 Rgba32i
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%11 = OpTypeVector %4 2
%12 = OpConstant %4 1
%13 = OpConstant %4 2
%14 = OpConstantComposite %11 %12 %13
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageRead %9 %10 %14
)",
          R"(
)"};
    case ValidTextureOverload::kLoadStorageRO2dRgba32float:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 2 Rgba32f
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeInt 32 1
%11 = OpTypeVector %12 2
%13 = OpConstant %12 1
%14 = OpConstant %12 2
%15 = OpConstantComposite %11 %13 %14
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageRead %9 %10 %15
)",
          R"(
)"};
    case ValidTextureOverload::kLoadStorageRO2dArrayRgba32float:

      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 2 Rgba32f
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeInt 32 1
%11 = OpTypeVector %12 3
%13 = OpConstant %12 1
%14 = OpConstant %12 2
%15 = OpConstant %12 3
%16 = OpConstantComposite %11 %13 %14 %15
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageRead %9 %10 %16
)",
          R"(
)"};
    case ValidTextureOverload::kLoadStorageRO3dRgba32float:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 2 Rgba32f
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVector %4 4
%12 = OpTypeInt 32 1
%11 = OpTypeVector %12 3
%13 = OpConstant %12 1
%14 = OpConstant %12 2
%15 = OpConstant %12 3
%16 = OpConstantComposite %11 %13 %14 %15
)",
          R"(
%10 = OpLoad %3 %1
%8 = OpImageRead %9 %10 %16
)",
          R"(
)"};
    case ValidTextureOverload::kStoreWO1dRgba32float:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 1D 0 0 0 2 Rgba32f
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVoid
%11 = OpTypeInt 32 1
%12 = OpConstant %11 1
%13 = OpTypeVector %4 4
%14 = OpConstant %4 2
%15 = OpConstant %4 3
%16 = OpConstant %4 4
%17 = OpConstant %4 5
%18 = OpConstantComposite %13 %14 %15 %16 %17
)",
          R"(
%10 = OpLoad %3 %1
OpImageWrite %10 %12 %18
)",
          R"(
OpCapability Image1D
)"};
    case ValidTextureOverload::kStoreWO2dRgba32float:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 2 Rgba32f
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVoid
%12 = OpTypeInt 32 1
%11 = OpTypeVector %12 2
%13 = OpConstant %12 1
%14 = OpConstant %12 2
%15 = OpConstantComposite %11 %13 %14
%16 = OpTypeVector %4 4
%17 = OpConstant %4 3
%18 = OpConstant %4 4
%19 = OpConstant %4 5
%20 = OpConstant %4 6
%21 = OpConstantComposite %16 %17 %18 %19 %20
)",
          R"(
%10 = OpLoad %3 %1
OpImageWrite %10 %15 %21
)",
          R"(
)"};
    case ValidTextureOverload::kStoreWO2dArrayRgba32float:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 2 Rgba32f
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVoid
%12 = OpTypeInt 32 1
%11 = OpTypeVector %12 3
%13 = OpConstant %12 1
%14 = OpConstant %12 2
%15 = OpConstant %12 3
%16 = OpConstantComposite %11 %13 %14 %15
%17 = OpTypeVector %4 4
%18 = OpConstant %4 4
%19 = OpConstant %4 5
%20 = OpConstant %4 6
%21 = OpConstant %4 7
%22 = OpConstantComposite %17 %18 %19 %20 %21
)",
          R"(
%10 = OpLoad %3 %1
OpImageWrite %10 %16 %22
)",
          R"(
)"};
    case ValidTextureOverload::kStoreWO3dRgba32float:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 2 Rgba32f
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%9 = OpTypeVoid
%12 = OpTypeInt 32 1
%11 = OpTypeVector %12 3
%13 = OpConstant %12 1
%14 = OpConstant %12 2
%15 = OpConstant %12 3
%16 = OpConstantComposite %11 %13 %14 %15
%17 = OpTypeVector %4 4
%18 = OpConstant %4 4
%19 = OpConstant %4 5
%20 = OpConstant %4 6
%21 = OpConstant %4 7
%22 = OpConstantComposite %17 %18 %19 %20 %21
)",
          R"(
%10 = OpLoad %3 %1
OpImageWrite %10 %16 %22
)",
          R"(
)"};
  }

  return {"<unmatched texture overload>", "<unmatched texture overload>",
          "<unmatched texture overload>"};
}  // NOLINT - Ignore the length of this function

using IntrinsicTextureTest =
    TestParamHelper<ast::intrinsic::test::TextureOverloadCase>;

INSTANTIATE_TEST_SUITE_P(
    IntrinsicTextureTest,
    IntrinsicTextureTest,
    testing::ValuesIn(ast::intrinsic::test::TextureOverloadCase::ValidCases()));

TEST_P(IntrinsicTextureTest, Call) {
  auto param = GetParam();

  auto* texture = param.buildTextureVariable(this);
  auto* sampler = param.buildSamplerVariable(this);

  auto* call =
      create<ast::CallExpression>(Expr(param.function), param.args(this));
  WrapInFunction(call);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(texture)) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(sampler)) << b.error();

  EXPECT_EQ(b.GenerateExpression(call), 8u) << b.error();

  auto expected = expected_texture_overload(param.overload);
  EXPECT_EQ(expected.types, "\n" + DumpInstructions(b.types()));
  EXPECT_EQ(expected.instructions,
            "\n" + DumpInstructions(b.functions()[0].instructions()));
  EXPECT_EQ(expected.capabilities, "\n" + DumpInstructions(b.capabilities()));
}

// Check the SPIRV generated passes validation
TEST_P(IntrinsicTextureTest, ValidateSPIRV) {
  auto param = GetParam();

  param.buildTextureVariable(this);
  param.buildSamplerVariable(this);

  auto* call =
      create<ast::CallExpression>(Expr(param.function), param.args(this));

  Func("main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::CallStatement>(call),
       },
       ast::DecorationList{
           Stage(ast::PipelineStage::kFragment),
       });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.Build()) << b.error();

  Validate(b);
}

TEST_P(IntrinsicTextureTest, OutsideFunction_IsError) {
  auto param = GetParam();

  // The point of this test is to try to generate the texture
  // intrinsic call outside a function.

  auto* texture = param.buildTextureVariable(this);
  auto* sampler = param.buildSamplerVariable(this);

  auto* call =
      create<ast::CallExpression>(Expr(param.function), param.args(this));
  WrapInFunction(call);

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(texture)) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(sampler)) << b.error();
  EXPECT_EQ(b.GenerateExpression(call), 0u);
  EXPECT_THAT(b.error(),
              ::testing::StartsWith(
                  "Internal error: trying to add SPIR-V instruction "));
  EXPECT_THAT(b.error(), ::testing::EndsWith(" outside a function"));
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
