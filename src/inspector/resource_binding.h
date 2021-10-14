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

#ifndef SRC_INSPECTOR_RESOURCE_BINDING_H_
#define SRC_INSPECTOR_RESOURCE_BINDING_H_

#include <cstdint>

#include "src/ast/storage_texture.h"
#include "src/ast/texture.h"

namespace tint {
namespace inspector {

/// Container for information about how a resource is bound
struct ResourceBinding {
  /// The dimensionality of a texture
  enum class TextureDimension {
    /// Invalid texture
    kNone = -1,
    /// 1 dimensional texture
    k1d,
    /// 2 dimensional texture
    k2d,
    /// 2 dimensional array texture
    k2dArray,
    /// 3 dimensional texture
    k3d,
    /// cube texture
    kCube,
    /// cube array texture
    kCubeArray,
  };

  /// Component type of the texture's data. Same as the Sampled Type parameter
  /// in SPIR-V OpTypeImage.
  enum class SampledKind { kUnknown = -1, kFloat, kUInt, kSInt };

  /// Enumerator of texture image formats
  enum class ImageFormat {
    kNone = -1,
    kR8Unorm,
    kR8Snorm,
    kR8Uint,
    kR8Sint,
    kR16Uint,
    kR16Sint,
    kR16Float,
    kRg8Unorm,
    kRg8Snorm,
    kRg8Uint,
    kRg8Sint,
    kR32Uint,
    kR32Sint,
    kR32Float,
    kRg16Uint,
    kRg16Sint,
    kRg16Float,
    kRgba8Unorm,
    kRgba8UnormSrgb,
    kRgba8Snorm,
    kRgba8Uint,
    kRgba8Sint,
    kBgra8Unorm,
    kBgra8UnormSrgb,
    kRgb10A2Unorm,
    kRg11B10Float,
    kRg32Uint,
    kRg32Sint,
    kRg32Float,
    kRgba16Uint,
    kRgba16Sint,
    kRgba16Float,
    kRgba32Uint,
    kRgba32Sint,
    kRgba32Float,
  };

  /// kXXX maps to entries returned by GetXXXResourceBindings call.
  enum class ResourceType {
    kUniformBuffer,
    kStorageBuffer,
    kReadOnlyStorageBuffer,
    kSampler,
    kComparisonSampler,
    kSampledTexture,
    kMultisampledTexture,
    kWriteOnlyStorageTexture,
    kDepthTexture,
    kDepthMultisampledTexture,
    kExternalTexture
  };

  /// Type of resource that is bound.
  ResourceType resource_type;
  /// Bind group the binding belongs
  uint32_t bind_group;
  /// Identifier to identify this binding within the bind group
  uint32_t binding;
  /// Size for this binding, in bytes, if defined.
  uint64_t size;
  /// Size for this binding without trailing structure padding, in bytes, if
  /// defined.
  uint64_t size_no_padding;
  /// Dimensionality of this binding, if defined.
  TextureDimension dim;
  /// Kind of data being sampled, if defined.
  SampledKind sampled_kind;
  /// Format of data, if defined.
  ImageFormat image_format;
};

/// Convert from internal ast::TextureDimension to public
/// ResourceBinding::TextureDimension
/// @param type_dim internal value to convert from
/// @returns the publicly visible equivalent
ResourceBinding::TextureDimension
TypeTextureDimensionToResourceBindingTextureDimension(
    const ast::TextureDimension& type_dim);

/// Infer ResourceBinding::SampledKind for a given sem::Type
/// @param base_type internal type to infer from
/// @returns the publicly visible equivalent
ResourceBinding::SampledKind BaseTypeToSampledKind(const sem::Type* base_type);

/// Convert from internal ast::ImageFormat to public
/// ResourceBinding::ImageFormat
/// @param image_format internal value to convert from
/// @returns the publicly visible equivalent
ResourceBinding::ImageFormat TypeImageFormatToResourceBindingImageFormat(
    const ast::ImageFormat& image_format);

}  // namespace inspector
}  // namespace tint

#endif  // SRC_INSPECTOR_RESOURCE_BINDING_H_
