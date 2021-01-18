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
      return ast::StorageClass::kUniformConstant;
    case SpvStorageClassStorageBuffer:
      return ast::StorageClass::kStorageBuffer;
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
  return ast::StorageClass::kNone;
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
      return ast::Builtin::kFragCoord;
    case SpvBuiltInFragDepth:
      return ast::Builtin::kFragDepth;
    case SpvBuiltInLocalInvocationId:
      return ast::Builtin::kLocalInvocationId;
    case SpvBuiltInLocalInvocationIndex:
      return ast::Builtin::kLocalInvocationIndex;
    case SpvBuiltInGlobalInvocationId:
      return ast::Builtin::kGlobalInvocationId;
    default:
      break;
  }

  Fail() << "unknown SPIR-V builtin: " << uint32_t(b);
  return ast::Builtin::kNone;
}

ast::type::TextureDimension EnumConverter::ToDim(SpvDim dim, bool arrayed) {
  if (arrayed) {
    switch (dim) {
      case SpvDim1D:
        return ast::type::TextureDimension::k1dArray;
      case SpvDim2D:
        return ast::type::TextureDimension::k2dArray;
      case SpvDimCube:
        return ast::type::TextureDimension::kCubeArray;
      default:
        break;
    }
    Fail() << "arrayed dimension must be 1D, 2D, or Cube. Got " << int(dim);
    return ast::type::TextureDimension::kNone;
  }
  // Assume non-arrayed
  switch (dim) {
    case SpvDim1D:
      return ast::type::TextureDimension::k1d;
    case SpvDim2D:
      return ast::type::TextureDimension::k2d;
    case SpvDim3D:
      return ast::type::TextureDimension::k3d;
    case SpvDimCube:
      return ast::type::TextureDimension::kCube;
    default:
      break;
  }
  Fail() << "invalid dimension: " << int(dim);
  return ast::type::TextureDimension::kNone;
}

ast::type::ImageFormat EnumConverter::ToImageFormat(SpvImageFormat fmt) {
  switch (fmt) {
    case SpvImageFormatUnknown:
      return ast::type::ImageFormat::kNone;

    // 8 bit channels
    case SpvImageFormatRgba8:
      return ast::type::ImageFormat::kRgba8Unorm;
    case SpvImageFormatRgba8Snorm:
      return ast::type::ImageFormat::kRgba8Snorm;
    case SpvImageFormatRgba8ui:
      return ast::type::ImageFormat::kRgba8Uint;
    case SpvImageFormatRgba8i:
      return ast::type::ImageFormat::kRgba8Sint;

    // 16 bit channels
    case SpvImageFormatRgba16ui:
      return ast::type::ImageFormat::kRgba16Uint;
    case SpvImageFormatRgba16i:
      return ast::type::ImageFormat::kRgba16Sint;
    case SpvImageFormatRgba16f:
      return ast::type::ImageFormat::kRgba16Float;

    // 32 bit channels
    case SpvImageFormatR32ui:
      return ast::type::ImageFormat::kR32Uint;
    case SpvImageFormatR32i:
      return ast::type::ImageFormat::kR32Sint;
    case SpvImageFormatR32f:
      return ast::type::ImageFormat::kR32Float;
    case SpvImageFormatRg32ui:
      return ast::type::ImageFormat::kRg32Uint;
    case SpvImageFormatRg32i:
      return ast::type::ImageFormat::kRg32Sint;
    case SpvImageFormatRg32f:
      return ast::type::ImageFormat::kRg32Float;
    case SpvImageFormatRgba32ui:
      return ast::type::ImageFormat::kRgba32Uint;
    case SpvImageFormatRgba32i:
      return ast::type::ImageFormat::kRgba32Sint;
    case SpvImageFormatRgba32f:
      return ast::type::ImageFormat::kRgba32Float;
    default:
      break;
  }
  Fail() << "invalid image format: " << int(fmt);
  return ast::type::ImageFormat::kNone;
}

}  // namespace spirv
}  // namespace reader
}  // namespace tint
