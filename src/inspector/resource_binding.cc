// Copyright 2021 The Tint Authors.
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

#include "src/inspector/resource_binding.h"

#include "src/sem/array.h"
#include "src/sem/f32_type.h"
#include "src/sem/i32_type.h"
#include "src/sem/matrix_type.h"
#include "src/sem/type.h"
#include "src/sem/u32_type.h"
#include "src/sem/vector_type.h"

namespace tint {
namespace inspector {

ResourceBinding::TextureDimension
TypeTextureDimensionToResourceBindingTextureDimension(
    const ast::TextureDimension& type_dim) {
  switch (type_dim) {
    case ast::TextureDimension::k1d:
      return ResourceBinding::TextureDimension::k1d;
    case ast::TextureDimension::k2d:
      return ResourceBinding::TextureDimension::k2d;
    case ast::TextureDimension::k2dArray:
      return ResourceBinding::TextureDimension::k2dArray;
    case ast::TextureDimension::k3d:
      return ResourceBinding::TextureDimension::k3d;
    case ast::TextureDimension::kCube:
      return ResourceBinding::TextureDimension::kCube;
    case ast::TextureDimension::kCubeArray:
      return ResourceBinding::TextureDimension::kCubeArray;
    case ast::TextureDimension::kNone:
      return ResourceBinding::TextureDimension::kNone;
  }
  return ResourceBinding::TextureDimension::kNone;
}

ResourceBinding::SampledKind BaseTypeToSampledKind(const sem::Type* base_type) {
  if (!base_type) {
    return ResourceBinding::SampledKind::kUnknown;
  }

  if (auto* at = base_type->As<sem::Array>()) {
    base_type = at->ElemType();
  } else if (auto* mt = base_type->As<sem::Matrix>()) {
    base_type = mt->type();
  } else if (auto* vt = base_type->As<sem::Vector>()) {
    base_type = vt->type();
  }

  if (base_type->Is<sem::F32>()) {
    return ResourceBinding::SampledKind::kFloat;
  } else if (base_type->Is<sem::U32>()) {
    return ResourceBinding::SampledKind::kUInt;
  } else if (base_type->Is<sem::I32>()) {
    return ResourceBinding::SampledKind::kSInt;
  } else {
    return ResourceBinding::SampledKind::kUnknown;
  }
}

ResourceBinding::ImageFormat TypeImageFormatToResourceBindingImageFormat(
    const ast::ImageFormat& image_format) {
  switch (image_format) {
    case ast::ImageFormat::kR8Unorm:
      return ResourceBinding::ImageFormat::kR8Unorm;
    case ast::ImageFormat::kR8Snorm:
      return ResourceBinding::ImageFormat::kR8Snorm;
    case ast::ImageFormat::kR8Uint:
      return ResourceBinding::ImageFormat::kR8Uint;
    case ast::ImageFormat::kR8Sint:
      return ResourceBinding::ImageFormat::kR8Sint;
    case ast::ImageFormat::kR16Uint:
      return ResourceBinding::ImageFormat::kR16Uint;
    case ast::ImageFormat::kR16Sint:
      return ResourceBinding::ImageFormat::kR16Sint;
    case ast::ImageFormat::kR16Float:
      return ResourceBinding::ImageFormat::kR16Float;
    case ast::ImageFormat::kRg8Unorm:
      return ResourceBinding::ImageFormat::kRg8Unorm;
    case ast::ImageFormat::kRg8Snorm:
      return ResourceBinding::ImageFormat::kRg8Snorm;
    case ast::ImageFormat::kRg8Uint:
      return ResourceBinding::ImageFormat::kRg8Uint;
    case ast::ImageFormat::kRg8Sint:
      return ResourceBinding::ImageFormat::kRg8Sint;
    case ast::ImageFormat::kR32Uint:
      return ResourceBinding::ImageFormat::kR32Uint;
    case ast::ImageFormat::kR32Sint:
      return ResourceBinding::ImageFormat::kR32Sint;
    case ast::ImageFormat::kR32Float:
      return ResourceBinding::ImageFormat::kR32Float;
    case ast::ImageFormat::kRg16Uint:
      return ResourceBinding::ImageFormat::kRg16Uint;
    case ast::ImageFormat::kRg16Sint:
      return ResourceBinding::ImageFormat::kRg16Sint;
    case ast::ImageFormat::kRg16Float:
      return ResourceBinding::ImageFormat::kRg16Float;
    case ast::ImageFormat::kRgba8Unorm:
      return ResourceBinding::ImageFormat::kRgba8Unorm;
    case ast::ImageFormat::kRgba8UnormSrgb:
      return ResourceBinding::ImageFormat::kRgba8UnormSrgb;
    case ast::ImageFormat::kRgba8Snorm:
      return ResourceBinding::ImageFormat::kRgba8Snorm;
    case ast::ImageFormat::kRgba8Uint:
      return ResourceBinding::ImageFormat::kRgba8Uint;
    case ast::ImageFormat::kRgba8Sint:
      return ResourceBinding::ImageFormat::kRgba8Sint;
    case ast::ImageFormat::kBgra8Unorm:
      return ResourceBinding::ImageFormat::kBgra8Unorm;
    case ast::ImageFormat::kBgra8UnormSrgb:
      return ResourceBinding::ImageFormat::kBgra8UnormSrgb;
    case ast::ImageFormat::kRgb10A2Unorm:
      return ResourceBinding::ImageFormat::kRgb10A2Unorm;
    case ast::ImageFormat::kRg11B10Float:
      return ResourceBinding::ImageFormat::kRg11B10Float;
    case ast::ImageFormat::kRg32Uint:
      return ResourceBinding::ImageFormat::kRg32Uint;
    case ast::ImageFormat::kRg32Sint:
      return ResourceBinding::ImageFormat::kRg32Sint;
    case ast::ImageFormat::kRg32Float:
      return ResourceBinding::ImageFormat::kRg32Float;
    case ast::ImageFormat::kRgba16Uint:
      return ResourceBinding::ImageFormat::kRgba16Uint;
    case ast::ImageFormat::kRgba16Sint:
      return ResourceBinding::ImageFormat::kRgba16Sint;
    case ast::ImageFormat::kRgba16Float:
      return ResourceBinding::ImageFormat::kRgba16Float;
    case ast::ImageFormat::kRgba32Uint:
      return ResourceBinding::ImageFormat::kRgba32Uint;
    case ast::ImageFormat::kRgba32Sint:
      return ResourceBinding::ImageFormat::kRgba32Sint;
    case ast::ImageFormat::kRgba32Float:
      return ResourceBinding::ImageFormat::kRgba32Float;
    case ast::ImageFormat::kNone:
      return ResourceBinding::ImageFormat::kNone;
  }
  return ResourceBinding::ImageFormat::kNone;
}

}  // namespace inspector
}  // namespace tint
