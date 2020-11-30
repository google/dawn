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
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

struct expected_texture_overload_spirv {
  std::string types;
  std::string instructions;
};

expected_texture_overload_spirv expected_texture_overload(
    ast::intrinsic::test::ValidTextureOverload overload) {
  using ValidTextureOverload = ast::intrinsic::test::ValidTextureOverload;
  switch (overload) {
    case ValidTextureOverload::kSample1dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 1D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpConstant %4 1
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleImplicitLod %9 %13 %14
)"};
    case ValidTextureOverload::kSample1dArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 1D 0 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 2
%15 = OpConstant %4 1
%17 = OpTypeInt 32 0
%18 = OpConstant %17 2
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%16 = OpConvertUToF %4 %18
%19 = OpCompositeConstruct %14 %15 %16
%8 = OpImageSampleImplicitLod %9 %13 %19
)"};
    case ValidTextureOverload::kSample2dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
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
)"};
    case ValidTextureOverload::kSample2dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
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
%8 = OpImageSampleImplicitLod %9 %13 %17 Offset %22
)"};
    case ValidTextureOverload::kSample2dArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%18 = OpTypeInt 32 0
%19 = OpConstant %18 3
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%17 = OpConvertUToF %4 %19
%20 = OpCompositeConstruct %14 %15 %16 %17
%8 = OpImageSampleImplicitLod %9 %13 %20
)"};
    case ValidTextureOverload::kSample2dArrayOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%18 = OpTypeInt 32 0
%19 = OpConstant %18 3
%22 = OpTypeInt 32 1
%21 = OpTypeVector %22 2
%23 = OpConstant %22 4
%24 = OpConstant %22 5
%25 = OpConstantComposite %21 %23 %24
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%17 = OpConvertUToF %4 %19
%20 = OpCompositeConstruct %14 %15 %16 %17
%8 = OpImageSampleImplicitLod %9 %13 %20 Offset %25
)"};
    case ValidTextureOverload::kSample3dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
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
)"};
    case ValidTextureOverload::kSample3dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
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
%8 = OpImageSampleImplicitLod %9 %13 %18 Offset %24
)"};
    case ValidTextureOverload::kSampleCubeF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
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
)"};
    case ValidTextureOverload::kSampleCubeArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstant %4 3
%18 = OpTypeInt 32 0
%19 = OpConstant %18 4
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%17 = OpConvertUToF %4 %19
%20 = OpCompositeConstruct %9 %14 %15 %16 %17
%8 = OpImageSampleImplicitLod %9 %13 %20
)"};
    case ValidTextureOverload::kSampleDepth2dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 2
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstantComposite %13 %14 %15
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%8 = OpImageSampleImplicitLod %4 %12 %16
)"};
    case ValidTextureOverload::kSampleDepth2dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 2
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstantComposite %13 %14 %15
%18 = OpTypeInt 32 1
%17 = OpTypeVector %18 2
%19 = OpConstant %18 3
%20 = OpConstant %18 4
%21 = OpConstantComposite %17 %19 %20
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%8 = OpImageSampleImplicitLod %4 %12 %16 Offset %21
)"};
    case ValidTextureOverload::kSampleDepth2dArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 3
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%17 = OpTypeInt 32 0
%18 = OpConstant %17 3
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%16 = OpConvertUToF %4 %18
%19 = OpCompositeConstruct %13 %14 %15 %16
%8 = OpImageSampleImplicitLod %4 %12 %19
)"};
    case ValidTextureOverload::kSampleDepth2dArrayOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 3
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%17 = OpTypeInt 32 0
%18 = OpConstant %17 3
%21 = OpTypeInt 32 1
%20 = OpTypeVector %21 2
%22 = OpConstant %21 4
%23 = OpConstant %21 5
%24 = OpConstantComposite %20 %22 %23
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%16 = OpConvertUToF %4 %18
%19 = OpCompositeConstruct %13 %14 %15 %16
%8 = OpImageSampleImplicitLod %4 %12 %19 Offset %24
)"};
    case ValidTextureOverload::kSampleDepthCubeF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 1 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 3
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstant %4 3
%17 = OpConstantComposite %13 %14 %15 %16
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%8 = OpImageSampleImplicitLod %4 %12 %17
)"};
    case ValidTextureOverload::kSampleDepthCubeArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 1 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 4
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstant %4 3
%18 = OpTypeInt 32 0
%19 = OpConstant %18 4
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%17 = OpConvertUToF %4 %19
%20 = OpCompositeConstruct %13 %14 %15 %16 %17
%8 = OpImageSampleImplicitLod %4 %12 %20
)"};
    case ValidTextureOverload::kSampleBias2dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
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
)"};
    case ValidTextureOverload::kSampleBias2dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
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
%8 = OpImageSampleImplicitLod %9 %13 %17 Bias|Offset %18 %23
)"};
    case ValidTextureOverload::kSampleBias2dArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%18 = OpTypeInt 32 0
%19 = OpConstant %18 4
%21 = OpConstant %4 3
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%17 = OpConvertUToF %4 %19
%20 = OpCompositeConstruct %14 %15 %16 %17
%8 = OpImageSampleImplicitLod %9 %13 %20 Bias %21
)"};
    case ValidTextureOverload::kSampleBias2dArrayOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%18 = OpTypeInt 32 0
%19 = OpConstant %18 3
%21 = OpConstant %4 4
%23 = OpTypeInt 32 1
%22 = OpTypeVector %23 2
%24 = OpConstant %23 5
%25 = OpConstant %23 6
%26 = OpConstantComposite %22 %24 %25
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%17 = OpConvertUToF %4 %19
%20 = OpCompositeConstruct %14 %15 %16 %17
%8 = OpImageSampleImplicitLod %9 %13 %20 Bias|Offset %21 %26
)"};
    case ValidTextureOverload::kSampleBias3dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
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
)"};
    case ValidTextureOverload::kSampleBias3dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
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
%8 = OpImageSampleImplicitLod %9 %13 %18 Bias|Offset %19 %25
)"};
    case ValidTextureOverload::kSampleBiasCubeF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
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
)"};
    case ValidTextureOverload::kSampleBiasCubeArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstant %4 3
%18 = OpTypeInt 32 0
%19 = OpConstant %18 3
%21 = OpConstant %4 4
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%17 = OpConvertUToF %4 %19
%20 = OpCompositeConstruct %9 %14 %15 %16 %17
%8 = OpImageSampleImplicitLod %9 %13 %20 Bias %21
)"};
    case ValidTextureOverload::kSampleLevel2dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
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
)"};
    case ValidTextureOverload::kSampleLevel2dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
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
%8 = OpImageSampleExplicitLod %9 %13 %17 Lod|Offset %18 %23
)"};
    case ValidTextureOverload::kSampleLevel2dArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%18 = OpTypeInt 32 0
%19 = OpConstant %18 3
%21 = OpConstant %4 4
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%17 = OpConvertUToF %4 %19
%20 = OpCompositeConstruct %14 %15 %16 %17
%8 = OpImageSampleExplicitLod %9 %13 %20 Lod %21
)"};
    case ValidTextureOverload::kSampleLevel2dArrayOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%18 = OpTypeInt 32 0
%19 = OpConstant %18 3
%21 = OpConstant %4 4
%23 = OpTypeInt 32 1
%22 = OpTypeVector %23 2
%24 = OpConstant %23 5
%25 = OpConstant %23 6
%26 = OpConstantComposite %22 %24 %25
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%17 = OpConvertUToF %4 %19
%20 = OpCompositeConstruct %14 %15 %16 %17
%8 = OpImageSampleExplicitLod %9 %13 %20 Lod|Offset %21 %26
)"};
    case ValidTextureOverload::kSampleLevel3dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
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
)"};
    case ValidTextureOverload::kSampleLevel3dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
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
%8 = OpImageSampleExplicitLod %9 %13 %18 Lod|Offset %19 %25
)"};
    case ValidTextureOverload::kSampleLevelCubeF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
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
)"};
    case ValidTextureOverload::kSampleLevelCubeArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstant %4 3
%18 = OpTypeInt 32 0
%19 = OpConstant %18 4
%21 = OpConstant %4 5
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%17 = OpConvertUToF %4 %19
%20 = OpCompositeConstruct %9 %14 %15 %16 %17
%8 = OpImageSampleExplicitLod %9 %13 %20 Lod %21
)"};
    case ValidTextureOverload::kSampleLevelDepth2dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 2
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstantComposite %13 %14 %15
%17 = OpTypeInt 32 0
%18 = OpConstant %17 3
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%8 = OpImageSampleExplicitLod %4 %12 %16 Lod %18
)"};
    case ValidTextureOverload::kSampleLevelDepth2dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 2
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstantComposite %13 %14 %15
%17 = OpTypeInt 32 0
%18 = OpConstant %17 3
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
%8 = OpImageSampleExplicitLod %4 %12 %16 Lod|Offset %18 %23
)"};
    case ValidTextureOverload::kSampleLevelDepth2dArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 3
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%17 = OpTypeInt 32 0
%18 = OpConstant %17 3
%20 = OpConstant %17 4
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%16 = OpConvertUToF %4 %18
%19 = OpCompositeConstruct %13 %14 %15 %16
%8 = OpImageSampleExplicitLod %4 %12 %19 Lod %20
)"};
    case ValidTextureOverload::kSampleLevelDepth2dArrayOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 3
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%17 = OpTypeInt 32 0
%18 = OpConstant %17 3
%20 = OpConstant %17 4
%22 = OpTypeInt 32 1
%21 = OpTypeVector %22 2
%23 = OpConstant %22 5
%24 = OpConstant %22 6
%25 = OpConstantComposite %21 %23 %24
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%16 = OpConvertUToF %4 %18
%19 = OpCompositeConstruct %13 %14 %15 %16
%8 = OpImageSampleExplicitLod %4 %12 %19 Lod|Offset %20 %25
)"};
    case ValidTextureOverload::kSampleLevelDepthCubeF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 1 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 3
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstant %4 3
%17 = OpConstantComposite %13 %14 %15 %16
%18 = OpTypeInt 32 0
%19 = OpConstant %18 4
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%8 = OpImageSampleExplicitLod %4 %12 %17 Lod %19
)"};
    case ValidTextureOverload::kSampleLevelDepthCubeArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 1 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 4
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstant %4 3
%18 = OpTypeInt 32 0
%19 = OpConstant %18 4
%21 = OpConstant %18 5
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%17 = OpConvertUToF %4 %19
%20 = OpCompositeConstruct %13 %14 %15 %16 %17
%8 = OpImageSampleExplicitLod %4 %12 %20 Lod %21
)"};
    case ValidTextureOverload::kSampleGrad2dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
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
)"};
    case ValidTextureOverload::kSampleGrad2dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
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
%8 = OpImageSampleExplicitLod %9 %13 %17 Grad|Offset %20 %23 %28
)"};
    case ValidTextureOverload::kSampleGrad2dArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%18 = OpTypeInt 32 0
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
%17 = OpConvertUToF %4 %19
%20 = OpCompositeConstruct %14 %15 %16 %17
%8 = OpImageSampleExplicitLod %9 %13 %20 Grad %24 %27
)"};
    case ValidTextureOverload::kSampleGrad2dArrayOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%18 = OpTypeInt 32 0
%19 = OpConstant %18 3
%21 = OpTypeVector %4 2
%22 = OpConstant %4 4
%23 = OpConstant %4 5
%24 = OpConstantComposite %21 %22 %23
%25 = OpConstant %4 6
%26 = OpConstant %4 7
%27 = OpConstantComposite %21 %25 %26
%29 = OpTypeInt 32 1
%28 = OpTypeVector %29 2
%30 = OpConstant %29 8
%31 = OpConstant %29 9
%32 = OpConstantComposite %28 %30 %31
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%17 = OpConvertUToF %4 %19
%20 = OpCompositeConstruct %14 %15 %16 %17
%8 = OpImageSampleExplicitLod %9 %13 %20 Grad|Offset %24 %27 %32
)"};
    case ValidTextureOverload::kSampleGrad3dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
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
)"};
    case ValidTextureOverload::kSampleGrad3dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
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
%8 = OpImageSampleExplicitLod %9 %13 %18 Grad|Offset %22 %26 %32
)"};
    case ValidTextureOverload::kSampleGradCubeF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
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
)"};
    case ValidTextureOverload::kSampleGradCubeArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstant %4 3
%18 = OpTypeInt 32 0
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
%17 = OpConvertUToF %4 %19
%20 = OpCompositeConstruct %9 %14 %15 %16 %17
%8 = OpImageSampleExplicitLod %9 %13 %20 Grad %25 %29
)"};
    case ValidTextureOverload::kSampleGradDepth2dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
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
)"};
    case ValidTextureOverload::kSampleGradDepth2dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
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
%8 = OpImageSampleDrefExplicitLod %4 %12 %16 %17 Lod|Offset %18 %23
)"};
    case ValidTextureOverload::kSampleGradDepth2dArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 3
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%17 = OpTypeInt 32 0
%18 = OpConstant %17 4
%20 = OpConstant %4 3
%21 = OpConstant %4 0
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%16 = OpConvertUToF %4 %18
%19 = OpCompositeConstruct %13 %14 %15 %16
%8 = OpImageSampleDrefExplicitLod %4 %12 %19 %20 Lod %21
)"};
    case ValidTextureOverload::kSampleGradDepth2dArrayOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 3
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%17 = OpTypeInt 32 0
%18 = OpConstant %17 4
%20 = OpConstant %4 3
%21 = OpConstant %4 0
%23 = OpTypeInt 32 1
%22 = OpTypeVector %23 2
%24 = OpConstant %23 5
%25 = OpConstant %23 6
%26 = OpConstantComposite %22 %24 %25
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%16 = OpConvertUToF %4 %18
%19 = OpCompositeConstruct %13 %14 %15 %16
%8 = OpImageSampleDrefExplicitLod %4 %12 %19 %20 Lod|Offset %21 %26
)"};
    case ValidTextureOverload::kSampleGradDepthCubeF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 1 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
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
)"};
    case ValidTextureOverload::kSampleGradDepthCubeArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 1 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 4
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstant %4 3
%18 = OpTypeInt 32 0
%19 = OpConstant %18 4
%21 = OpConstant %4 5
%22 = OpConstant %4 0
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%17 = OpConvertUToF %4 %19
%20 = OpCompositeConstruct %13 %14 %15 %16 %17
%8 = OpImageSampleDrefExplicitLod %4 %12 %20 %21 Lod %22
)"};
  }
  return {"<unmatched texture overload>", "<unmatched texture overload>"};
}  // NOLINT - Ignore the length of this function

class IntrinsicTextureTest
    : public ast::BuilderWithContextAndModule,
      public testing::TestWithParam<ast::intrinsic::test::TextureOverloadCase> {
 protected:
  void OnVariableBuilt(ast::Variable* var) override {
    td.RegisterVariableForTesting(var);
  }

  TypeDeterminer td{ctx, mod};
  spirv::Builder b{ctx, mod};
};

INSTANTIATE_TEST_SUITE_P(
    IntrinsicTextureTest,
    IntrinsicTextureTest,
    testing::ValuesIn(ast::intrinsic::test::TextureOverloadCase::ValidCases()));

TEST_P(IntrinsicTextureTest, Call) {
  auto param = GetParam();

  b.push_function(Function{});

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

  ast::type::Sampler sampler_type{param.sampler_kind};
  ast::Variable* tex = nullptr;
  switch (param.texture_kind) {
    case ast::intrinsic::test::TextureKind::kRegular:
      tex = Var("texture", ast::StorageClass::kNone,
                mod->create<ast::type::SampledTexture>(param.texture_dimension,
                                                       datatype));
      break;

    case ast::intrinsic::test::TextureKind::kDepth:
      tex = Var("texture", ast::StorageClass::kNone,
                mod->create<ast::type::DepthTexture>(param.texture_dimension));
      break;
  }

  auto* sampler = Var("sampler", ast::StorageClass::kNone, &sampler_type);

  ASSERT_TRUE(b.GenerateGlobalVariable(tex)) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(sampler)) << b.error();

  ast::CallExpression call{Expr(param.function), param.args(this)};

  EXPECT_TRUE(td.DetermineResultType(&call)) << td.error();
  EXPECT_EQ(b.GenerateExpression(&call), 8u) << b.error();

  auto expected = expected_texture_overload(param.overload);
  EXPECT_EQ(expected.types, "\n" + DumpInstructions(b.types()));
  EXPECT_EQ(expected.instructions,
            "\n" + DumpInstructions(b.functions()[0].instructions()));
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
