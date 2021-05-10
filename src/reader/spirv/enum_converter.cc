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

#include "src/reader/spirv/enum_converter.h"

namespace tint {
namespace reader {
namespace spirv {

EnumConverter::EnumConverter(const FailStream& fs) : fail_stream_(fs) {}

EnumConverter::~EnumConverter() = default;

ast::PipelineStage EnumConverter::ToPipelineStage(SpvExecutionModel model) {
  switch (model) {
    case SpvExecutionModelVertex:
      return ast::PipelineStage::kVertex;
    case SpvExecutionModelFragment:
      return ast::PipelineStage::kFragment;
    case SpvExecutionModelGLCompute:
      return ast::PipelineStage::kCompute;
    default:
      break;
  }

  Fail() << "unknown SPIR-V execution model: " << uint32_t(model);
  return ast::PipelineStage::kNone;
}

ast::StorageClass EnumConverter::ToStorageClass(const SpvStorageClass sc) {
  switch (sc) {
    case SpvStorageClassInput:
      return ast::StorageClass::kInput;
    case SpvStorageClassOutput:
      return ast::StorageClass::kOutput;
    case SpvStorageClassUniform:
      return ast::StorageClass::kUniform;
    case SpvStorageClassWorkgroup:
      return ast::StorageClass::kWorkgroup;
    case SpvStorageClassUniformConstant:
      return ast::StorageClass::kNone;
    case SpvStorageClassStorageBuffer:
      return ast::StorageClass::kStorage;
    case SpvStorageClassImage:
      return ast::StorageClass::kImage;
    case SpvStorageClassPrivate:
      return ast::StorageClass::kPrivate;
    case SpvStorageClassFunction:
      return ast::StorageClass::kFunction;
    default:
      break;
  }

  Fail() << "unknown SPIR-V storage class: " << uint32_t(sc);
  return ast::StorageClass::kInvalid;
}

ast::Builtin EnumConverter::ToBuiltin(SpvBuiltIn b) {
  switch (b) {
    case SpvBuiltInPosition:
      return ast::Builtin::kPosition;
    case SpvBuiltInVertexIndex:
      return ast::Builtin::kVertexIndex;
    case SpvBuiltInInstanceIndex:
      return ast::Builtin::kInstanceIndex;
    case SpvBuiltInFrontFacing:
      return ast::Builtin::kFrontFacing;
    case SpvBuiltInFragCoord:
      return ast::Builtin::kPosition;
    case SpvBuiltInFragDepth:
      return ast::Builtin::kFragDepth;
    case SpvBuiltInLocalInvocationId:
      return ast::Builtin::kLocalInvocationId;
    case SpvBuiltInLocalInvocationIndex:
      return ast::Builtin::kLocalInvocationIndex;
    case SpvBuiltInGlobalInvocationId:
      return ast::Builtin::kGlobalInvocationId;
    case SpvBuiltInWorkgroupId:
      return ast::Builtin::kWorkgroupId;
    case SpvBuiltInSampleId:
      return ast::Builtin::kSampleIndex;
    case SpvBuiltInSampleMask:
      return ast::Builtin::kSampleMask;
    default:
      break;
  }

  Fail() << "unknown SPIR-V builtin: " << uint32_t(b);
  return ast::Builtin::kNone;
}

ast::TextureDimension EnumConverter::ToDim(SpvDim dim, bool arrayed) {
  if (arrayed) {
    switch (dim) {
      case SpvDim2D:
        return ast::TextureDimension::k2dArray;
      case SpvDimCube:
        return ast::TextureDimension::kCubeArray;
      default:
        break;
    }
    Fail() << "arrayed dimension must be 2D or Cube. Got " << int(dim);
    return ast::TextureDimension::kNone;
  }
  // Assume non-arrayed
  switch (dim) {
    case SpvDim1D:
      return ast::TextureDimension::k1d;
    case SpvDim2D:
      return ast::TextureDimension::k2d;
    case SpvDim3D:
      return ast::TextureDimension::k3d;
    case SpvDimCube:
      return ast::TextureDimension::kCube;
    default:
      break;
  }
  Fail() << "invalid dimension: " << int(dim);
  return ast::TextureDimension::kNone;
}

ast::ImageFormat EnumConverter::ToImageFormat(SpvImageFormat fmt) {
  switch (fmt) {
    case SpvImageFormatUnknown:
      return ast::ImageFormat::kNone;

    // 8 bit channels
    case SpvImageFormatRgba8:
      return ast::ImageFormat::kRgba8Unorm;
    case SpvImageFormatRgba8Snorm:
      return ast::ImageFormat::kRgba8Snorm;
    case SpvImageFormatRgba8ui:
      return ast::ImageFormat::kRgba8Uint;
    case SpvImageFormatRgba8i:
      return ast::ImageFormat::kRgba8Sint;

    // 16 bit channels
    case SpvImageFormatRgba16ui:
      return ast::ImageFormat::kRgba16Uint;
    case SpvImageFormatRgba16i:
      return ast::ImageFormat::kRgba16Sint;
    case SpvImageFormatRgba16f:
      return ast::ImageFormat::kRgba16Float;

    // 32 bit channels
    case SpvImageFormatR32ui:
      return ast::ImageFormat::kR32Uint;
    case SpvImageFormatR32i:
      return ast::ImageFormat::kR32Sint;
    case SpvImageFormatR32f:
      return ast::ImageFormat::kR32Float;
    case SpvImageFormatRg32ui:
      return ast::ImageFormat::kRg32Uint;
    case SpvImageFormatRg32i:
      return ast::ImageFormat::kRg32Sint;
    case SpvImageFormatRg32f:
      return ast::ImageFormat::kRg32Float;
    case SpvImageFormatRgba32ui:
      return ast::ImageFormat::kRgba32Uint;
    case SpvImageFormatRgba32i:
      return ast::ImageFormat::kRgba32Sint;
    case SpvImageFormatRgba32f:
      return ast::ImageFormat::kRgba32Float;
    default:
      break;
  }
  Fail() << "invalid image format: " << int(fmt);
  return ast::ImageFormat::kNone;
}

}  // namespace spirv
}  // namespace reader
}  // namespace tint
